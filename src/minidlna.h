//
// Created by Pontus Karlsson on 12/07/15.
//

#ifndef MINIDLNA_MINIDLNA_H
#define MINIDLNA_MINIDLNA_H

#include <time.h>

#include <sqlite3.h>

#include "minidlnatypes.h"
#include "clients.h"
#include "config.h"

#include "getifaddr.h"
struct runtime_vars_s {
	unsigned short port;	/* HTTP Port */
	unsigned int notify_interval;	/* seconds between SSDP announces */
	unsigned int max_connections;	/* max number of simultaneous conenctions */
	const char *root_container;	/* root ObjectID (instead of "0") */
	const char *ifaces[MAX_LAN_ADDR];	/* list of configured network interfaces */
#ifdef ENABLE_VIDEO_THUMB
	int thumb_width;	/* Video thumbnail width */
#endif
	int mta;
};

#define SERVER_NAME "MiniDLNA"

#define DB_VERSION 11

#ifdef ENABLE_NLS
#	define _(string) gettext(string)
#else
#	define _(string) (string)
#endif

#define THISORNUL(s) (s ? s : "")

#define RESOURCE_PROTOCOL_INFO_VALUES \
	"http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_TN," \
	"http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM," \
	"http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_MED," \
	"http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_LRG," \
	"http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HD_50_AC3_ISO," \
	"http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HD_60_AC3_ISO," \
	"http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HP_HD_AC3_ISO," \
	"http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_MULT5_ISO," \
	"http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_AC3_ISO," \
	"http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_MPEG1_L3_ISO," \
	"http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_MULT5_ISO," \
	"http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_AC3_ISO," \
	"http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_MPEG1_L3_ISO," \
	"http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_NTSC," \
	"http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_PAL," \
	"http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_NA_ISO," \
	"http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_NA_ISO," \
	"http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_EU_ISO," \
	"http-get:*:video/mpeg:DLNA.ORG_PN=MPEG1," \
	"http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_AAC_MULT5," \
	"http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_AC3," \
	"http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_AAC_520," \
	"http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_AAC_940," \
	"http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L31_HD_AAC," \
	"http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L32_HD_AAC," \
	"http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L3L_SD_AAC," \
	"http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_HP_HD_AAC," \
	"http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_HD_1080i_AAC," \
	"http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_HD_720p_AAC," \
	"http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_AAC," \
	"http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_VGA_AAC," \
	"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_50_AC3," \
	"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_50_AC3_T," \
	"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_60_AC3," \
	"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_60_AC3_T," \
	"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HP_HD_AC3_T," \
	"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_MULT5," \
	"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_MULT5_T," \
	"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AC3," \
	"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AC3_T," \
	"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_MPEG1_L3," \
	"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_MPEG1_L3_T," \
	"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_MULT5," \
	"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_MULT5_T," \
	"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AC3," \
	"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AC3_T," \
	"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_MPEG1_L3," \
	"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_MPEG1_L3_T," \
	"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_NA," \
	"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_NA_T," \
	"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_EU," \
	"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_EU_T," \
	"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_NA," \
	"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_NA_T," \
	"http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPLL_BASE," \
	"http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPML_BASE," \
	"http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPML_MP3," \
	"http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_BASE," \
	"http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_FULL," \
	"http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_PRO," \
	"http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVHIGH_FULL," \
	"http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVHIGH_PRO," \
	"http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_P2_3GPP_SP_L0B_AAC," \
	"http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_P2_3GPP_SP_L0B_AMR," \
	"http-get:*:audio/mpeg:DLNA.ORG_PN=MP3," \
	"http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMABASE," \
	"http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMAFULL," \
	"http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMAPRO," \
	"http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMALSL," \
	"http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMALSL_MULT5," \
	"http-get:*:audio/mp4:DLNA.ORG_PN=AAC_ISO_320," \
	"http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_ISO_320," \
	"http-get:*:audio/mp4:DLNA.ORG_PN=AAC_ISO," \
	"http-get:*:audio/mp4:DLNA.ORG_PN=AAC_MULT5_ISO," \
	"http-get:*:audio/L16;rate=44100;channels=2:DLNA.ORG_PN=LPCM," \
	"http-get:*:image/jpeg:*," \
	"http-get:*:video/avi:*," \
	"http-get:*:video/divx:*," \
	"http-get:*:video/x-matroska:*," \
	"http-get:*:video/mpeg:*," \
	"http-get:*:video/mp4:*," \
	"http-get:*:video/x-ms-wmv:*," \
	"http-get:*:video/x-msvideo:*," \
	"http-get:*:video/x-flv:*," \
	"http-get:*:video/x-tivo-mpeg:*," \
	"http-get:*:video/quicktime:*," \
	"http-get:*:audio/mp4:*," \
	"http-get:*:audio/x-wav:*," \
	"http-get:*:audio/x-flac:*," \
	"http-get:*:application/ogg:*"

#define DLNA_FLAG_DLNA_V1_5      0x00100000
#define DLNA_FLAG_HTTP_STALLING  0x00200000
#define DLNA_FLAG_TM_B           0x00400000
#define DLNA_FLAG_TM_I           0x00800000
#define DLNA_FLAG_TM_S           0x01000000
#define DLNA_FLAG_LOP_BYTES      0x20000000
#define DLNA_FLAG_LOP_NPT        0x40000000

extern struct runtime_vars_s runtime_vars;
/* runtime boolean flags */
extern uint32_t runtime_flags;
#define INOTIFY_MASK          0x0001
#define DLNA_STRICT_MASK      0x0004
#define NO_PLAYLIST_MASK      0x0008
#define SYSTEMD_MASK          0x0010
#define MERGE_MEDIA_DIRS_MASK 0x0020
#ifdef ENABLE_VIDEO_THUMB
#define THUMB_MASK            0x0080
#endif

#define SETFLAG(mask)	runtime_flags |= mask
#define GETFLAG(mask)	(runtime_flags & mask)
#define CLEARFLAG(mask)	runtime_flags &= ~mask

extern const char *pidfilename;

extern char uuidvalue[];

#define MODELNAME_MAX_LEN 64
extern char modelname[];

#define MODELNUMBER_MAX_LEN 16
extern char modelnumber[];

#define SERIALNUMBER_MAX_LEN 16
extern char serialnumber[];

#define PRESENTATIONURL_MAX_LEN 64
extern char presentationurl[];

#define FRIENDLYNAME_MAX_LEN 64
extern char friendly_name[];

/* lan addresses */
extern int n_lan_addr;
extern struct lan_addr_s lan_addr[];

/* UPnP-A/V [DLNA] */
extern sqlite3 *db;
extern char db_path[];
extern char log_path[];
extern struct media_dir_s *media_dirs;
extern struct linked_names_s *album_art_names;
extern struct linked_names_s *ignore_paths;
extern short int scanning;
extern volatile short int quitting;
extern volatile uint32_t updateID;
extern const char *force_sort_criteria;
#endif //MINIDLNA_MINIDLNA_H
