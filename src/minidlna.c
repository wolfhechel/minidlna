/* MiniDLNA project
 *
 * http://sourceforge.net/projects/minidlna/
 *
 * MiniDLNA media server
 * Copyright (C) 2008-2012  Justin Maggard
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
 *
 * Portions of the code from the MiniUPnP project:
 *
 * Copyright (c) 2006-2007, Thomas Bernard
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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <pwd.h>

#ifdef ENABLE_NLS
#include <locale.h>
#include <libintl.h>

static void init_nls(void) {
	setlocale(LC_MESSAGES, "");
	setlocale(LC_CTYPE, "en_US.utf8");
	DPRINTF(E_DEBUG, L_GENERAL, "Using locale dir %s\n", bindtextdomain("minidlna", getenv("TEXTDOMAINDIR")));
	textdomain("minidlna");
}
#else
#define init_nls() ({})
#endif

#include "minidlna.h"
#include "upnp/upnphttp.h"
#include "upnp/ssdp.h"
#include "upnp/upnpevents.h"
#include "sql.h"
#include "getifaddr.h"
#include "options.h"
#include "utils.h"
#include "process.h"
#include "scanner.h"
#include "inotify.h"
#include "log.h"


struct runtime_vars_s runtime_vars;
uint32_t runtime_flags = INOTIFY_MASK;

char uuidvalue[] = "uuid:00000000-0000-0000-0000-000000000000";
char modelname[MODELNAME_MAX_LEN] = ROOTDEV_MODELNAME;
char modelnumber[MODELNUMBER_MAX_LEN] = PACKAGE_VERSION;
char serialnumber[SERIALNUMBER_MAX_LEN] = "00000000";
char friendly_name[FRIENDLYNAME_MAX_LEN];

int n_lan_addr = 0;
struct lan_addr_s lan_addr[MAX_LAN_ADDR];

/* UPnP-A/V [DLNA] */
sqlite3 *db;
char db_path[PATH_MAX] = {'\0'};
char log_path[PATH_MAX] = {'\0'};
struct media_dir_s * media_dirs = NULL;
struct linked_names_s * album_art_names = NULL;
struct linked_names_s * ignore_paths = NULL;
short int scanning = 0;
volatile short int quitting = 0;
volatile uint32_t updateID = 0;
const char *force_sort_criteria = NULL;

#if SQLITE_VERSION_NUMBER < 3005001
# warning "Your SQLite3 library appears to be too old!  Please use 3.5.1 or newer."
# define sqlite3_threadsafe() 0
#endif

/* Handler for the SIGTERM signal (kill) 
 * SIGINT is also handled */
static void
sigterm(int sig)
{
	signal(sig, SIG_IGN);	/* Ignore this signal while we are quitting */

	DPRINTF(E_WARN, L_GENERAL, "received signal %d, good-bye\n", sig);

	quitting = 1;
}

static void
sigusr1(int sig)
{
	signal(sig, sigusr1);
	DPRINTF(E_WARN, L_GENERAL, "received signal %d, clear cache\n", sig);

	memset(&clients, '\0', sizeof(clients));
}

static void
sighup(int sig)
{
	signal(sig, sighup);
	DPRINTF(E_WARN, L_GENERAL, "received signal %d, re-read\n", sig);

	reload_ifaces(1);
}

static void
getfriendlyname(char *buf, int len)
{
	char *p = NULL;
	char hn[256];
	int off;

	if (gethostname(hn, sizeof(hn)) == 0)
	{
		strncpyt(buf, hn, len);
		p = strchr(buf, '.');
		if (p)
			*p = '\0';
	}
	else
		strcpy(buf, "Unknown");

	off = strlen(buf);
	off += snprintf(buf+off, len-off, ": ");
	char * logname;
	logname = getenv("LOGNAME");
#ifndef STATIC // Disable for static linking
	if (!logname)
	{
		struct passwd * pwent;
		pwent = getpwuid(getuid());
		if (pwent)
			logname = pwent->pw_name;
	}
#endif
	snprintf(buf+off, len-off, "%s", logname?logname:"Unknown");
}

int
main(int argc, char **argv)
{
	int i;
	int pid;
	int debug_flag = 0;
	int verbose_flag = 0;
	int options_flag = 0;
	struct sigaction sa;
	const char * optionsfile = DEFAULT_CONF_PATH;
	const char *pidfilename = DEFAULT_RUN_PATH "/minidlna/minidlna.pid";

	char mac_str[13];
	char *string, *word;
	char *path;
	char buf[PATH_MAX];
	char log_str[75] = "general,artwork,database,inotify,scanner,metadata,http,ssdp=warn";
	char *log_level = NULL;
	int ifaces = 0;
	uid_t uid = 0;
	int ret;
	int shttpl = -1;
	int smonitor = -1;

	LIST_HEAD(httplisthead, upnphttp) upnphttphead;

	struct upnphttp * e = 0,
			        * next,
			        * tmp;

	fd_set readset,
		   writeset;	/* for select() */

	struct timeval timeout,
			       timeofday,
			       lastnotifytime = {0, 0};

	time_t lastupdatetime = 0;

	int max_fd = -1,
		last_changecnt = 0;

	pid_t scanner_pid = 0;

	pthread_t inotify_thread = 0;

	/* first check if "-f" option is used */
	for (i=2; i<argc; i++)
	{
		if (strcmp(argv[i-1], "-f") == 0)
		{
			optionsfile = argv[i];
			options_flag = 1;
			break;
		}
	}

	/* set up uuid based on mac address */
	if (getsyshwaddr(mac_str, sizeof(mac_str)) < 0)
	{
		DPRINTF(E_OFF, L_GENERAL, "No MAC address found.  Falling back to generic UUID.\n");
		strcpy(mac_str, "554e4b4e4f57");
	}
	strcpy(uuidvalue+5, "4d696e69-444c-164e-9d41-");
	strncat(uuidvalue, mac_str, 12);

	getfriendlyname(friendly_name, FRIENDLYNAME_MAX_LEN);

	runtime_vars.port = 8200;
	runtime_vars.notify_interval = 895;	/* seconds between SSDP announces */
	runtime_vars.max_connections = 50;
	runtime_vars.root_container = NULL;
	runtime_vars.ifaces[0] = NULL;

#ifdef ENABLE_VIDEO_THUMB
	runtime_vars.thumb_width = 160;
#endif
	runtime_vars.mta = 0;

	/* read options file first since
	 * command line arguments have final say */
	if (readoptionsfile(optionsfile) < 0)
	{
		/* only error if file exists or using -f */
		if(access(optionsfile, F_OK) == 0 || options_flag)
			DPRINTF(E_FATAL, L_GENERAL, "Error reading configuration file %s\n", optionsfile);
	}

	for (i=0; i<num_options; i++)
	{
		switch (ary_options[i].id)
		{
			case UPNPIFNAME:
				for (string = ary_options[i].value; (word = strtok(string, ",")); string = NULL)
				{
					if (ifaces >= MAX_LAN_ADDR)
					{
						DPRINTF(E_ERROR, L_GENERAL, "Too many interfaces (max: %d), ignoring %s\n",
								MAX_LAN_ADDR, word);
						break;
					}
					runtime_vars.ifaces[ifaces++] = word;
				}
				break;
			case UPNPPORT:
				runtime_vars.port = atoi(ary_options[i].value);
				break;
			case UPNPNOTIFY_INTERVAL:
				runtime_vars.notify_interval = atoi(ary_options[i].value);
				break;
			case UPNPSERIAL:
				strncpyt(serialnumber, ary_options[i].value, SERIALNUMBER_MAX_LEN);
				break;
			case UPNPMODEL_NAME:
				strncpyt(modelname, ary_options[i].value, MODELNAME_MAX_LEN);
				break;
			case UPNPMODEL_NUMBER:
				strncpyt(modelnumber, ary_options[i].value, MODELNUMBER_MAX_LEN);
				break;
			case UPNPFRIENDLYNAME:
				strncpyt(friendly_name, ary_options[i].value, FRIENDLYNAME_MAX_LEN);
				break;
			case UPNPMEDIADIR: {
				struct media_dir_s *media_dir;
				media_types types = ALL_MEDIA;
				path = ary_options[i].value;
				word = strchr(path, ',');
				if (word && (access(path, F_OK) != 0))
				{
					types = 0;
					while (*path)
					{
						if (*path == ',')
						{
							path++;
							break;
						}
						else if (*path == 'A' || *path == 'a')
							types |= TYPE_AUDIO;
						else if (*path == 'V' || *path == 'v')
							types |= TYPE_VIDEO;
						else if (*path == 'P' || *path == 'p')
							types |= TYPE_IMAGES;
						else
							DPRINTF(E_FATAL, L_GENERAL, "Media directory entry not understood [%s]\n",
									ary_options[i].value);
						path++;
					}
				}
				path = realpath(path, buf);
				if (!path || access(path, F_OK) != 0)
				{
					DPRINTF(E_ERROR, L_GENERAL, "Media directory \"%s\" not accessible [%s]\n",
							ary_options[i].value, strerror(errno));
					break;
				}
				media_dir = calloc(1, sizeof(struct media_dir_s));
				media_dir->path = strdup(path);
				media_dir->types = types;
				add_element_to_linked_list((void**)&media_dirs, media_dir);
			} break;
			case UPNPALBUMART_NAMES: {
				struct linked_names_s *album_art_name = parse_delimited_list_of_options(ary_options[i].value, "/");
				add_element_to_linked_list((void**)&album_art_names, album_art_name);
			} break;
			case SCANNER_IGNORE: {
				struct linked_names_s *ignore_path = parse_delimited_list_of_options(ary_options[i].value, "/");
				add_element_to_linked_list((void**)&ignore_paths, ignore_path);
			} break;
			case UPNPDBDIR:
				path = realpath(ary_options[i].value, buf);
				if (!path)
					path = (ary_options[i].value);
				make_dir(path, S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO);
				if (access(path, F_OK) != 0)
					DPRINTF(E_FATAL, L_GENERAL, "Database path not accessible! [%s]\n", path);
				strncpyt(db_path, path, PATH_MAX);
				break;
			case UPNPLOGDIR:
				path = realpath(ary_options[i].value, buf);
				if (!path)
					path = (ary_options[i].value);
				make_dir(path, S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO);
				if (access(path, F_OK) != 0)
					DPRINTF(E_FATAL, L_GENERAL, "Log path not accessible! [%s]\n", path);
				strncpyt(log_path, path, PATH_MAX);
				break;
			case UPNPLOGLEVEL:
				log_level = ary_options[i].value;
				break;
			case UPNPINOTIFY:
				if (!strtobool(ary_options[i].value))
					CLEARFLAG(INOTIFY_MASK);
				break;
			case ENABLE_DLNA_STRICT:
				if (strtobool(ary_options[i].value))
					SETFLAG(DLNA_STRICT_MASK);
				break;
			case ROOT_CONTAINER:
				switch (ary_options[i].value[0]) {
					case '.':
						runtime_vars.root_container = NULL;
						break;
					case 'B':
					case 'b':
						runtime_vars.root_container = BROWSEDIR_ID;
						break;
					case 'M':
					case 'm':
						runtime_vars.root_container = MUSIC_ID;
						break;
					case 'V':
					case 'v':
						runtime_vars.root_container = VIDEO_ID;
						break;
					case 'P':
					case 'p':
						runtime_vars.root_container = IMAGE_ID;
						break;
					default:
						runtime_vars.root_container = ary_options[i].value;
						DPRINTF(E_WARN, L_GENERAL, "Using arbitrary root container [%s]\n",
								ary_options[i].value);
						break;
				}
				break;
			case UPNPUUID:
				strcpy(uuidvalue+5, ary_options[i].value);
				break;
			case USER_ACCOUNT:
				uid = strtoul(ary_options[i].value, &string, 0);
				if (*string)
				{
					/* Symbolic username given, not UID. */
					struct passwd *entry = getpwnam(ary_options[i].value);
					if (!entry)
						DPRINTF(E_FATAL, L_GENERAL, "Bad user '%s'.\n", argv[i]);
					uid = entry->pw_uid;
				}
				break;
			case FORCE_SORT_CRITERIA:
				force_sort_criteria = ary_options[i].value;
				break;
			case MAX_CONNECTIONS:
				runtime_vars.max_connections = atoi(ary_options[i].value);
				break;
			case MERGE_MEDIA_DIRS:
				if (strtobool(ary_options[i].value))
					SETFLAG(MERGE_MEDIA_DIRS_MASK);
				break;
#ifdef ENABLE_VIDEO_THUMB
			case ENABLE_THUMB:
				if( (strcmp(ary_options[i].value, "yes") == 0) || atoi(ary_options[i].value) )
					SETFLAG(THUMB_MASK);
				break;
			case THUMB_WIDTH:
				runtime_vars.thumb_width = atoi(ary_options[i].value);
				break;
#endif
			case ENABLE_MTA:
				runtime_vars.mta = atoi(ary_options[i].value);
				break;
			default:
				DPRINTF(E_ERROR, L_GENERAL, "Unknown option in file %s\n",
						optionsfile);
		}
	}
	if (log_path[0] == '\0')
	{
		if (db_path[0] == '\0')
			strncpyt(log_path, DEFAULT_LOG_PATH, PATH_MAX);
		else
			strncpyt(log_path, db_path, PATH_MAX);
	}
	if (db_path[0] == '\0')
		strncpyt(db_path, DEFAULT_DB_PATH, PATH_MAX);

	/* command line arguments processing */
	for (i=1; i<argc; i++)
	{
		if (argv[i][0] != '-')
		{
			DPRINTF(E_FATAL, L_GENERAL, "Unknown option: %s\n", argv[i]);
		}
		else if (strcmp(argv[i], "--help") == 0)
		{
			runtime_vars.port = -1;
			break;
		}
		else switch(argv[i][1])
			{
				case 't':
					if (i+1 < argc)
						runtime_vars.notify_interval = atoi(argv[++i]);
					else
						DPRINTF(E_FATAL, L_GENERAL, "Option -%c takes one argument.\n", argv[i][1]);
					break;
				case 's':
					if (i+1 < argc)
						strncpyt(serialnumber, argv[++i], SERIALNUMBER_MAX_LEN);
					else
						DPRINTF(E_FATAL, L_GENERAL, "Option -%c takes one argument.\n", argv[i][1]);
					break;
				case 'm':
					if (i+1 < argc)
						strncpyt(modelnumber, argv[++i], MODELNUMBER_MAX_LEN);
					else
						DPRINTF(E_FATAL, L_GENERAL, "Option -%c takes one argument.\n", argv[i][1]);
					break;
				case 'p':
					if (i+1 < argc)
						runtime_vars.port = atoi(argv[++i]);
					else
						DPRINTF(E_FATAL, L_GENERAL, "Option -%c takes one argument.\n", argv[i][1]);
					break;
				case 'P':
					if (i+1 < argc)
					{
						if (argv[++i][0] != '/')
							DPRINTF(E_FATAL, L_GENERAL, "Option -%c requires an absolute filename.\n", argv[i-1][1]);
						else
							pidfilename = argv[i];
					}
					else
						DPRINTF(E_FATAL, L_GENERAL, "Option -%c takes one argument.\n", argv[i][1]);
					break;
				case 'd':
					debug_flag = 1;
				case 'v':
					verbose_flag = 1;
					break;
				case 'i':
					if (i+1 < argc)
					{
						i++;
						if (ifaces >= MAX_LAN_ADDR)
						{
							DPRINTF(E_ERROR, L_GENERAL, "Too many interfaces (max: %d), ignoring %s\n",
									MAX_LAN_ADDR, argv[i]);
							break;
						}
						runtime_vars.ifaces[ifaces++] = argv[i];
					}
					else
						DPRINTF(E_FATAL, L_GENERAL, "Option -%c takes one argument.\n", argv[i][1]);
					break;
				case 'f':
					i++;	/* discarding, the config file is already read */
					break;
				case 'h':
					runtime_vars.port = -1; // triggers help display
					break;
				case 'R':
					snprintf(buf, sizeof(buf), "rm -rf %s/files.db %s/art_cache", db_path, db_path);
					if (system(buf) != 0)
						DPRINTF(E_FATAL, L_GENERAL, "Failed to clean old file cache. EXITING\n");
					break;
				case 'u':
					if (i+1 != argc)
					{
						i++;
						uid = strtoul(argv[i], &string, 0);
						if (*string)
						{
							/* Symbolic username given, not UID. */
							struct passwd *entry = getpwnam(argv[i]);
							if (!entry)
								DPRINTF(E_FATAL, L_GENERAL, "Bad user '%s'.\n", argv[i]);
							uid = entry->pw_uid;
						}
					}
					else
						DPRINTF(E_FATAL, L_GENERAL, "Option -%c takes one argument.\n", argv[i][1]);
					break;
					break;
#ifdef __linux__
		case 'S':
			SETFLAG(SYSTEMD_MASK);
			break;
#endif
				case 'V':
					printf("Version " PACKAGE_VERSION "\n");
					exit(0);
					break;
				default:
					DPRINTF(E_ERROR, L_GENERAL, "Unknown option: %s\n", argv[i]);
					runtime_vars.port = -1; // triggers help display
			}
	}

	if (runtime_vars.port <= 0)
	{
		printf("Usage:\n\t"
					   "%s [-d] [-v] [-f config_file] [-p port]\n"
					   "\t\t[-i network_interface] [-u uid_to_run_as]\n"
					   "\t\t[-t notify_interval] [-P pid_filename]\n"
					   "\t\t[-s serial] [-m model_number]\n"
#ifdef __linux__
			"\t\t[-w url] [-R] [-L] [-S] [-V] [-h]\n"
#else
					   "\t\t[-R] [-V] [-h]\n"
#endif
					   "\nNotes:\n\tNotify interval is in seconds. Default is 895 seconds.\n"
					   "\tDefault pid file is %s.\n"
					   "\tWith -d minidlna will run in debug mode (not daemonize).\n"
					   "\t-v enables verbose output\n"
					   "\t-h displays this text\n"
					   "\t-R forces a full rescan\n"
#ifdef __linux__
			"\t-S changes behaviour for systemd\n"
#endif
					   "\t-V print the version number\n",
			   argv[0], pidfilename);
		return 1;
	}

	if (verbose_flag)
	{
		strcpy(log_str+65, "debug");
		log_level = log_str;
	}
	else if (!log_level)
		log_level = log_str;

	/* Set the default log file path to NULL (stdout) */
	path = NULL;
	if (debug_flag)
	{
		pid = getpid();
		strcpy(log_str+65, "maxdebug");
		log_level = log_str;
	}
	else if (GETFLAG(SYSTEMD_MASK))
	{
		pid = getpid();
	}
	else
	{
		pid = process_daemonize();

		if (access(db_path, F_OK) != 0)
			make_dir(db_path, S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO);

		snprintf(buf, sizeof(buf), "%s/minidlna.log", log_path);
		path = buf;
	}
	log_init(path, log_level);

	if (process_check_if_running(pidfilename) < 0)
	{
		DPRINTF(E_ERROR, L_GENERAL, SERVER_NAME " is already running. EXITING.\n");
		return 1;
	}

	/* set signal handlers */
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = sigterm;
	if (sigaction(SIGTERM, &sa, NULL))
		DPRINTF(E_FATAL, L_GENERAL, "Failed to set %s handler. EXITING.\n", "SIGTERM");
	if (sigaction(SIGINT, &sa, NULL))
		DPRINTF(E_FATAL, L_GENERAL, "Failed to set %s handler. EXITING.\n", "SIGINT");
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		DPRINTF(E_FATAL, L_GENERAL, "Failed to set %s handler. EXITING.\n", "SIGPIPE");
	if (signal(SIGHUP, &sighup) == SIG_ERR)
		DPRINTF(E_FATAL, L_GENERAL, "Failed to set %s handler. EXITING.\n", "SIGHUP");
	signal(SIGUSR1, &sigusr1);
	sa.sa_handler = process_handle_child_termination;
	if (sigaction(SIGCHLD, &sa, NULL))
		DPRINTF(E_FATAL, L_GENERAL, "Failed to set %s handler. EXITING.\n", "SIGCHLD");

	if (writepidfile(pidfilename, pid, uid) != 0)
		pidfilename = NULL;

	if (uid > 0)
	{
		struct stat st;
		if (stat(db_path, &st) == 0 && st.st_uid != uid && chown(db_path, uid, -1) != 0)
			DPRINTF(E_ERROR, L_GENERAL, "Unable to set db_path [%s] ownership to %d: %s\n",
					db_path, uid, strerror(errno));
	}

	if (uid > 0 && setuid(uid) == -1)
		DPRINTF(E_FATAL, L_GENERAL, "Failed to switch to uid '%d'. [%s] EXITING.\n",
				uid, strerror(errno));

	children = calloc(runtime_vars.max_connections, sizeof(struct child));
	if (!children)
	{
		DPRINTF(E_ERROR, L_GENERAL, "Allocation failed\n");
		return 1;
	}

	for (i = 0; i < L_MAX; i++)
		log_level[i] = E_WARN;

	init_nls();

	DPRINTF(E_WARN, L_GENERAL, "Starting " SERVER_NAME " version " PACKAGE_VERSION ".\n");
	if (sqlite3_libversion_number() < 3005001)
	{
		DPRINTF(E_WARN, L_GENERAL, "SQLite library is old.  Please use version 3.5.1 or newer.\n");
	}

	LIST_INIT(&upnphttphead);

	ret = open_db(NULL);
	if (ret == 0)
	{
		updateID = sql_get_int_field(db, "SELECT VALUE from SETTINGS where KEY = 'UPDATE_ID'");
		if (updateID == -1)
			ret = -1;
	}
	check_db(db, ret, &scanner_pid);
#ifdef HAVE_INOTIFY
	if( GETFLAG(INOTIFY_MASK) )
	{
		if (!sqlite3_threadsafe() || sqlite3_libversion_number() < 3005001)
			DPRINTF(E_ERROR, L_GENERAL, "SQLite library is not threadsafe!  "
			                            "Inotify will be disabled.\n");
		else if (pthread_create(&inotify_thread, NULL, start_inotify, NULL) != 0)
			DPRINTF(E_FATAL, L_GENERAL, "ERROR: pthread_create() failed for start_inotify. EXITING\n");
	}
#endif
	smonitor = OpenAndConfMonitorSocket();

	/* open socket for HTTP connections. */
	shttpl = OpenAndConfHTTPSocket(runtime_vars.port);
	if (shttpl < 0)
		DPRINTF(E_FATAL, L_GENERAL, "Failed to open socket for HTTP. EXITING\n");
	DPRINTF(E_WARN, L_GENERAL, "HTTP listening on port %d\n", runtime_vars.port);

	reload_ifaces(0);
	lastnotifytime.tv_sec = time(NULL) + runtime_vars.notify_interval;

	/* main loop */
	while (!quitting)
	{

		/* Check if we need to send SSDP NOTIFY messages and do it if
		 * needed */
		if (gettimeofday(&timeofday, 0) < 0)
		{
			DPRINTF(E_ERROR, L_GENERAL, "gettimeofday(): %s\n", strerror(errno));
			timeout.tv_sec = runtime_vars.notify_interval;
			timeout.tv_usec = 0;
		}
		else
		{
			/* the comparison is not very precise but who cares ? */
			if (timeofday.tv_sec >= (lastnotifytime.tv_sec + runtime_vars.notify_interval))
			{
				DPRINTF(E_DEBUG, L_SSDP, "Sending SSDP notifies\n");
				for (i = 0; i < n_lan_addr; i++)
				{
					SendSSDPNotifies(lan_addr[i].snotify, lan_addr[i].str,
						runtime_vars.port, runtime_vars.notify_interval);
				}
				memcpy(&lastnotifytime, &timeofday, sizeof(struct timeval));
				timeout.tv_sec = runtime_vars.notify_interval;
				timeout.tv_usec = 0;
			}
			else
			{
				timeout.tv_sec = lastnotifytime.tv_sec + runtime_vars.notify_interval
				                 - timeofday.tv_sec;
				if (timeofday.tv_usec > lastnotifytime.tv_usec)
				{
					timeout.tv_usec = 1000000 + lastnotifytime.tv_usec
					                  - timeofday.tv_usec;
					timeout.tv_sec--;
				}
				else
					timeout.tv_usec = lastnotifytime.tv_usec - timeofday.tv_usec;
			}
		}

		if (scanning)
		{
			if (!scanner_pid || kill(scanner_pid, 0) != 0)
			{
				scanning = 0;
				updateID++;
			}
		}

		/* select open sockets (SSDP, HTTP listen, and all HTTP soap sockets) */
		FD_ZERO(&readset);

		for (i = 0; i < n_lan_addr; i++)
		{
			FD_SET(lan_addr[i].snotify, &readset);
		}

		if (smonitor >= 0)
		{
			FD_SET(smonitor, &readset);
			max_fd = MAX(max_fd, smonitor);
		}
		
		if (shttpl >= 0) 
		{
			FD_SET(shttpl, &readset);
			max_fd = MAX(max_fd, shttpl);
		}

		i = 0;	/* active HTTP connections count */
		for (e = upnphttphead.lh_first; e != NULL; e = e->entries.le_next)
		{
			if ((e->socket >= 0) && (e->state <= 2))
			{
				FD_SET(e->socket, &readset);
				max_fd = MAX(max_fd, e->socket);
				i++;
			}
		}

		FD_ZERO(&writeset);
		upnpevents_selectfds(&readset, &writeset, &max_fd);

		ret = select(max_fd+1, &readset, &writeset, 0, &timeout);
		if (ret < 0)
		{
			if(quitting) goto shutdown;
			if(errno == EINTR) continue;
			DPRINTF(E_ERROR, L_GENERAL, "select(all): %s\n", strerror(errno));
			DPRINTF(E_FATAL, L_GENERAL, "Failed to select open sockets. EXITING\n");
		}

		upnpevents_processfds(&readset, &writeset);

		/* process SSDP packets */
		for (i = 0; i < n_lan_addr; i++)
		{
			if (FD_ISSET(lan_addr[i].snotify, &readset)) {
				ProcessSSDPRequest(lan_addr[i].snotify, runtime_vars.port);
			}
		}

		if (smonitor >= 0 && FD_ISSET(smonitor, &readset))
		{
			ProcessMonitorEvent(smonitor);
		}
		/* increment SystemUpdateID if the content database has changed,
		 * and if there is an active HTTP connection, at most once every 2 seconds */
		if (i && (timeofday.tv_sec >= (lastupdatetime + 2)))
		{
			if (scanning || sqlite3_total_changes(db) != last_changecnt)
			{
				updateID++;
				last_changecnt = sqlite3_total_changes(db);
				upnp_event_var_change_notify(EContentDirectory);
				lastupdatetime = timeofday.tv_sec;
			}
		}
		/* process active HTTP connections */
		for (e = upnphttphead.lh_first; e != NULL; e = e->entries.le_next)
		{
			if ((e->socket >= 0) && (e->state <= 2) && (FD_ISSET(e->socket, &readset)))
				Process_upnphttp(e);
		}
		/* process incoming HTTP connections */
		if (shttpl >= 0 && FD_ISSET(shttpl, &readset))
		{
			tmp = Accept_upnphttp(shttpl);

			if (tmp)
				LIST_INSERT_HEAD(&upnphttphead, tmp, entries);
		}
		/* delete finished HTTP connections */
		for (e = upnphttphead.lh_first; e != NULL; e = next)
		{
			next = e->entries.le_next;
			if(e->state >= 100)
			{
				LIST_REMOVE(e, entries);
				Delete_upnphttp(e);
			}
		}
	}

shutdown:
	/* kill the scanner */
	if (scanning && scanner_pid)
		kill(scanner_pid, SIGKILL);

	/* kill other child processes */
	process_reap_children();
	free(children);

	/* close out open sockets */
	while (upnphttphead.lh_first != NULL)
	{
		e = upnphttphead.lh_first;
		LIST_REMOVE(e, entries);
		Delete_upnphttp(e);
	}

	if (shttpl >= 0)
		close(shttpl);

	for (i = 0; i < n_lan_addr; i++)
	{
		SendSSDPGoodbyes(lan_addr[i].snotify);
		close(lan_addr[i].snotify);
	}

	if (inotify_thread)
		pthread_join(inotify_thread, NULL);

	sql_exec(db, "UPDATE SETTINGS set VALUE = '%u' where KEY = 'UPDATE_ID'", updateID);
	sqlite3_close(db);

	upnpevents_removeSubscribers();

	if (pidfilename && unlink(pidfilename) < 0)
		DPRINTF(E_ERROR, L_GENERAL, "Failed to remove pidfile %s: %s\n", pidfilename, strerror(errno));

	log_close();
	freeoptions();

	exit(EXIT_SUCCESS);
}

