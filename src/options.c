/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * author: Ryan Wagoner
 *
 * Copyright (c) 2006, Thomas Bernard
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * The name of the author may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "options.h"
#include "utils.h"
#include "minidlna.h"

struct option * ary_options = NULL;
int num_options = 0;

static const struct {
	enum upnpconfigoptions id;
	const char * name;
} optionids[] = {
	{ UPNPIFNAME, "network_interface" },
	{ UPNPPORT, "port" },
	{ UPNPNOTIFY_INTERVAL, "notify_interval" },
	{ UPNPUUID, "uuid"},
	{ UPNPSERIAL, "serial"},
	{ UPNPMODEL_NAME, "model_name"},
	{ UPNPMODEL_NUMBER, "model_number"},
	{ UPNPFRIENDLYNAME, "friendly_name"},
	{ UPNPMEDIADIR, "media_dir"},
	{ UPNPALBUMART_NAMES, "album_art_names"},
	{ SCANNER_IGNORE, "ignore_paths"},
	{ UPNPINOTIFY, "inotify" },
	{ UPNPDBDIR, "db_dir" },
	{ UPNPLOGDIR, "log_dir" },
	{ UPNPLOGLEVEL, "log_level" },
	{ ENABLE_DLNA_STRICT, "strict_dlna" },
	{ ROOT_CONTAINER, "root_container" },
	{ USER_ACCOUNT, "user" },
	{ FORCE_SORT_CRITERIA, "force_sort_criteria" },
	{ MAX_CONNECTIONS, "max_connections" },
#ifdef ENABLE_VIDEO_THUMB
	{ ENABLE_THUMB, "enable_thumbnail" },
	{ THUMB_WIDTH, "thumbnail_width" },
#endif
	{ ENABLE_MTA, "enable_mta" },
	{ MERGE_MEDIA_DIRS, "merge_media_dirs" }
};

int
readoptionsfile(const char * fname)
{
	FILE *hfile = NULL;
	char buffer[1024];
	char *equals;
	char *name;
	char *value;
	char *t;
	int linenum = 0;
	int i;
	enum upnpconfigoptions id;

	if(!fname || *fname == '\0')
		return -1;

	memset(buffer, 0, sizeof(buffer));

#ifdef DEBUG
	printf("Reading configuration from file %s\n", fname);
#endif

	if(!(hfile = fopen(fname, "r")))
		return -1;

	while(fgets(buffer, sizeof(buffer), hfile))
	{
		linenum++;
		t = strchr(buffer, '\n'); 
		if(t)
		{
			*t = '\0';
			t--;
			while((t >= buffer) && isspace(*t))
			{
				*t = '\0';
				t--;
			}
		}

		/* skip leading whitespaces */
		name = buffer;
		while(isspace(*name))
			name++;

		/* check for comments or empty lines */
		if(name[0] == '#' || name[0] == '\0') continue;

		if(!(equals = strchr(name, '=')))
		{
			fprintf(stderr, "parsing error file %s line %d : %s\n",
			        fname, linenum, name);
			continue;
		}

		/* remove ending whitespaces */
		for(t=equals-1; t>name && isspace(*t); t--)
			*t = '\0';

		*equals = '\0';
		value = equals+1;

		/* skip leading whitespaces */
		while(isspace(*value))
			value++;

		id = UPNP_INVALID;
		for(i=0; i<sizeof(optionids)/sizeof(optionids[0]); i++)
		{
			/*printf("%2d %2d %s %s\n", i, optionids[i].id, name,
			       optionids[i].name); */

			if(0 == strcmp(name, optionids[i].name))
			{
				id = optionids[i].id;
				break;
			}
		}

		if(id == UPNP_INVALID)
		{
			if (strcmp(name, "include") == 0)
				readoptionsfile(value);
			else
				fprintf(stderr, "parsing error file %s line %d : %s=%s\n",
				        fname, linenum, name, value);
		}
		else
		{
			num_options++;
			t = realloc(ary_options, num_options * sizeof(struct option));
			if(!t)
			{
				fprintf(stderr, "memory allocation error: %s=%s\n",
					name, value);
				num_options--;
				continue;
			}
			else
				ary_options = (struct option *)t;

			ary_options[num_options-1].id = id;
			strncpyt(ary_options[num_options-1].value, value, MAX_OPTION_VALUE_LEN);
		}

	}
	
	fclose(hfile);
	
	return 0;
}

void free_linked_names_s(struct linked_names_s *names)
{
	while (names)
	{
		struct linked_names_s *last_name = names;
		free(last_name->name);
		names = names->next;
		free(last_name);
	}
}

void
freeoptions(void)
{
	struct media_dir_s *media_path, *last_path;
	
	media_path = media_dirs;
	while (media_path)
	{
		free(media_path->path);
		last_path = media_path;
		media_path = media_path->next;
		free(last_path);
	}

	free_linked_names_s(album_art_names);
	free_linked_names_s(ignore_paths);

	if(ary_options)
	{
		free(ary_options);
		ary_options = NULL;
		num_options = 0;
	}
}

int strtobool(const char *str)
{
	return ((strcasecmp(str, "yes") == 0) ||
			(strcasecmp(str, "true") == 0) ||
			(atoi(str) == 1));
}

struct linked_names_s * parse_delimited_list_of_options(char * input, const char * delimiter) {
	struct linked_names_s * linked_entry = NULL, * return_value;
	char * word;
	for (return_value = NULL; (word = strtok(input, delimiter)); input = NULL) {
		struct linked_names_s * entry = calloc(1, sizeof(struct linked_names_s));
		int len = strlen(word);
		if (word[len - 1] == '*')
		{
			word[len - 1] = '\0';
			entry->wildcard = 1;
		}
		entry->name = strdup(word);
		if (return_value) linked_entry->next = entry;
		else return_value = entry;

		linked_entry = entry;
	}
	return return_value;
}

void
add_element_to_linked_list(void **root, void* entry)
{
	while (*root != NULL) root = (void**)&(*(char*)*root); // make use of the fact that the next pointer is the first element
	*root = entry;
}

