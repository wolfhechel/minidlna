/* MiniDLNA media server
 * Copyright (C) 2008  Justin Maggard
 *
 * This file is part of MiniDLNA.
 *
 * MiniDLNA is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * MiniDLNA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MiniDLNA. If not, see <http://www.gnu.org/licenses/>.
 */
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <limits.h>
#include <libgen.h>
#include <setjmp.h>
#include <errno.h>

#include <jpeglib.h>

#include "minidlna.h"
#include "albumart.h"
#include "sql.h"
#include "utils.h"
#include "crypt.h"
#include "image_utils.h"
#include "libav.h"
#include "video_thumb.h"
#include "log.h"

static int
art_cache_exists(const char *orig_path, char **cache_file)
{
	if( xasprintf(cache_file, "%s/art_cache%s", db_path, orig_path) < 0 )
		return 0;

	strcpy(strchr(*cache_file, '\0')-4, ".jpg");

	return (!access(*cache_file, F_OK));
}

static char *
save_resized_album_art(image_s *imsrc, const char *path)
{
	int dstw, dsth;
	image_s *imdst;
	char *cache_file;
	char cache_dir[MAXPATHLEN];

	if( !imsrc )
		return NULL;

	if( art_cache_exists(path, &cache_file) )
		return cache_file;

	strncpyt(cache_dir, cache_file, sizeof(cache_dir));
	make_dir(dirname(cache_dir), S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);

	if( imsrc->width > imsrc->height )
	{
		dstw = 160;
		dsth = (imsrc->height<<8) / ((imsrc->width<<8)/160);
	}
	else
	{
		dstw = (imsrc->width<<8) / ((imsrc->height<<8)/160);
		dsth = 160;
	}
	imdst = image_resize(imsrc, dstw, dsth);
	if( !imdst )
	{
		free(cache_file);
		return NULL;
	}

	cache_file = image_save_to_jpeg_file(imdst, cache_file);
	image_free(imdst);
	
	return cache_file;
}

int
is_album_art(const char * name)
{
	struct linked_names_s * album_art_name;

	/* Check if this file name matches one of the default album art names */
	for( album_art_name = album_art_names; album_art_name; album_art_name = album_art_name->next )
	{
		if( album_art_name->wildcard )
		{
			if( strncmp(album_art_name->name, name, strlen(album_art_name->name)) == 0 )
				break;
		}
		else
		{
			if( strcmp(album_art_name->name, name) == 0 )
				break;
		}
	}

	return (album_art_name ? 1 : 0);
}

/* And our main album art functions */
void
update_if_album_art(const char *path)
{
	char *dir;
	char *match;
	char file[MAXPATHLEN];
	char fpath[MAXPATHLEN];
	char dpath[MAXPATHLEN];
	int ncmp = 0;
	int album_art;
	DIR *dh;
	struct dirent *dp;
	enum file_types type = TYPE_UNKNOWN;
	int64_t art_id = 0;
	int ret;

	strncpyt(fpath, path, sizeof(fpath));
	match = basename(fpath);
	/* Check if this file name matches a specific audio or video file */
	if( ends_with(match, ".cover.jpg") )
	{
		ncmp = strlen(match)-10;
	}
	else
	{
		ncmp = strrchr(match, '.') - match;
	}
	/* Check if this file name matches one of the default album art names */
	album_art = is_album_art(match);

	strncpyt(dpath, path, sizeof(dpath));
	dir = dirname(dpath);
	dh = opendir(dir);
	if( !dh )
		return;
	while ((dp = readdir(dh)) != NULL)
	{
		if (is_reg(dp) == 1)
		{
			type = TYPE_FILE;
		}
		else if (is_dir(dp) == 1)
		{
			type = TYPE_DIR;
		}
		else
		{
			snprintf(file, sizeof(file), "%s/%s", dir, dp->d_name);
			type = resolve_unknown_type(file, ALL_MEDIA);
		}
		if( type != TYPE_FILE )
			continue;
		if( (dp->d_name[0] != '.') &&
		    (is_video(dp->d_name) || is_audio(dp->d_name)) &&
		    (album_art || strncmp(dp->d_name, match, ncmp) == 0) )
		{
			DPRINTF(E_DEBUG, L_METADATA, "New file %s looks like cover art for %s\n", path, dp->d_name);
			snprintf(file, sizeof(file), "%s/%s", dir, dp->d_name);
			art_id = find_album_art(file, NULL, 0);
			ret = sql_exec(db, "UPDATE DETAILS set ALBUM_ART = %lld where PATH = '%q'", (long long)art_id, file);
			if( ret != SQLITE_OK )
				DPRINTF(E_WARN, L_METADATA, "Error setting %s as cover art for %s\n", match, dp->d_name);
		}
	}
	closedir(dh);
}

char *
check_embedded_art(const char *path, uint8_t *image_data, int image_size)
{
	int width = 0, height = 0;
	char *art_path = NULL;
	char *cache_dir;
	FILE *dstfile;
	image_s *imsrc;
	static char last_path[PATH_MAX];
	static unsigned int last_hash = 0;
	static int last_success = 0;
	unsigned int hash;

	if( !image_data || !image_size || !path )
	{
		return NULL;
	}
	/* If the embedded image matches the embedded image from the last file we
	 * checked, just make a hard link.  Better than storing it on the disk twice. */
	hash = DJBHash(image_data, image_size);
	if( hash == last_hash )
	{
		if( !last_success )
			return NULL;
		art_cache_exists(path, &art_path);
		if( link(last_path, art_path) == 0 )
		{
			return(art_path);
		}
		else
		{
			if( errno == ENOENT )
			{
				cache_dir = strdup(art_path);
				make_dir(dirname(cache_dir), S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
				free(cache_dir);
				if( link(last_path, art_path) == 0 )
					return(art_path);
			}
			DPRINTF(E_WARN, L_METADATA, "Linking %s to %s failed [%s]\n", art_path, last_path, strerror(errno));
			free(art_path);
			art_path = NULL;
		}
	}
	last_hash = hash;

	imsrc = image_new_from_jpeg(NULL, 0, image_data, image_size, 1, ROTATE_NONE);
	if( !imsrc )
	{
		last_success = 0;
		return NULL;
	}
	width = imsrc->width;
	height = imsrc->height;

	if( width > 160 || height > 160 )
	{
		art_path = save_resized_album_art(imsrc, path);
	}
	else if( width > 0 && height > 0 )
	{
		size_t nwritten;
		if( art_cache_exists(path, &art_path) )
			goto end_art;
		cache_dir = strdup(art_path);
		make_dir(dirname(cache_dir), S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
		free(cache_dir);
		dstfile = fopen(art_path, "w");
		if( !dstfile )
		{
			free(art_path);
			art_path = NULL;
			goto end_art;
		}
		nwritten = fwrite((void *)image_data, 1, image_size, dstfile);
		fclose(dstfile);
		if( nwritten != image_size )
		{
			DPRINTF(E_WARN, L_METADATA, "Embedded art error: wrote %lu/%d bytes\n",
				(unsigned long)nwritten, image_size);
			remove(art_path);
			free(art_path);
			art_path = NULL;
			goto end_art;
		}
	}
end_art:
	image_free(imsrc);
	if( !art_path )
	{
		DPRINTF(E_WARN, L_METADATA, "Invalid embedded album art in %s\n", basename((char *)path));
		last_success = 0;
		return NULL;
	}
	DPRINTF(E_DEBUG, L_METADATA, "Found new embedded album art in %s\n", basename((char *)path));
	last_success = 1;
	strcpy(last_path, art_path);

	return(art_path);
}

static char *
check_for_album_file(const char *path)
{
	char file[MAXPATHLEN];
	char mypath[MAXPATHLEN];
	struct linked_names_s *album_art_name;
	image_s *imsrc = NULL;
	int width=0, height=0;
	char *art_file, *p;
	const char *dir;
	struct stat st;
	int ret;

	if( stat(path, &st) != 0 )
		return NULL;

	if( S_ISDIR(st.st_mode) )
	{
		dir = path;
		goto check_dir;
	}
	strncpyt(mypath, path, sizeof(mypath));
	dir = dirname(mypath);

	/* First look for file-specific cover art */
	snprintf(file, sizeof(file), "%s.cover.jpg", path);
	ret = access(file, R_OK);
	if( ret != 0 )
	{
		strncpyt(file, path, sizeof(file));
		p = strrchr(file, '.');
		if( p )
		{
			strcpy(p, ".jpg");
			ret = access(file, R_OK);
		}
		if( ret != 0 )
		{
			p = strrchr(file, '/');
			if( p )
			{
				memmove(p+2, p+1, file+MAXPATHLEN-p-2);
				p[1] = '.';
				ret = access(file, R_OK);
			}
		}
	}
	if( ret == 0 )
	{
		if( art_cache_exists(file, &art_file) )
			goto existing_file;
		free(art_file);
		imsrc = image_new_from_jpeg(file, 1, NULL, 0, 1, ROTATE_NONE);
		if( imsrc )
			goto found_file;
	}
check_dir:
	/* Then fall back to possible generic cover art file names */
	for( album_art_name = album_art_names; album_art_name; album_art_name = album_art_name->next )
	{
		snprintf(file, sizeof(file), "%s/%s", dir, album_art_name->name);
		if( access(file, R_OK) == 0 )
		{
			if( art_cache_exists(file, &art_file) )
			{
existing_file:
				return art_file;
			}
			free(art_file);
			imsrc = image_new_from_jpeg(file, 1, NULL, 0, 1, ROTATE_NONE);
			if( !imsrc )
				continue;
found_file:
			width = imsrc->width;
			height = imsrc->height;
			if( width > 160 || height > 160 )
				art_file = save_resized_album_art(imsrc, file);
			else
				art_file = strdup(file);
			image_free(imsrc);
			return(art_file);
		}
	}
	return NULL;
}

#ifdef ENABLE_VIDEO_THUMB
char *
generate_thumbnail(const char * path)
{
	char *tfile = NULL;
	char cache_dir[MAXPATHLEN];

	if( art_cache_exists(path, &tfile) )
		return tfile;

	memset(&cache_dir, 0, sizeof(cache_dir));

	if ( is_video(path) )
	{
		strncpyt(cache_dir, tfile, sizeof(cache_dir)-1);
		if ( !make_dir(dirname(cache_dir), S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) &&
			!video_thumb_generate_tofile(path, tfile, 20, runtime_vars.thumb_width))
			return tfile;
	}
	free(tfile);

	return 0;
}
#endif

int64_t
find_album_art(const char *path, uint8_t *image_data, int image_size)
{
	char *album_art = NULL;
	int64_t ret = 0;

	if ( image_size )
		album_art = check_embedded_art(path, image_data, image_size);

	if ( !album_art )
		album_art = check_for_album_file(path);

#ifdef ENABLE_VIDEO_THUMB
	if ( !album_art && GETFLAG(THUMB_MASK) )
		album_art = generate_thumbnail(path);
#endif

	if( album_art )
	{
		ret = sql_get_int_field(db, "SELECT ID from ALBUM_ART where PATH = '%q'", album_art);
		if( !ret )
		{
			if( sql_exec(db, "INSERT into ALBUM_ART (PATH) VALUES ('%q')", album_art) == SQLITE_OK )
				ret = sqlite3_last_insert_rowid(db);
		}
	}
	free(album_art);

	return ret;
}
