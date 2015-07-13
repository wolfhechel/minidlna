/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 *
 * Copyright (c) 2006-2008, Thomas Bernard
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
#include <stdlib.h>
#include <string.h>

#include "upnpdescgen.h"
#include "minidlnapath.h"
#include "minidlna.h"
#include "utils.h"
#include "xml.h"

#include "svc_connectionmgr.h"
#include "svc_contentdirectory.h"
#include "svc_x_ms_mediareceiverregistrar.h"

static const char root_device[] = 
	"root xmlns=\"urn:schemas-upnp-org:device-1-0\""
	" xmlns:pnpx=\"http://schemas.microsoft.com/windows/pnpx/2005/11\""
	" xmlns:df=\"http://schemas.microsoft.com/windows/2008/09/devicefoundation\""
	;

/* root Description of the UPnP Device */
static const struct XMLElt rootDesc[] =
{
	{root_device, INITHELPER(1,2)},

	{"specVersion", INITHELPER(3,2)},
	{"device", INITHELPER(5,(20))},

	{"/major", "1"},
	{"/minor", "0"},

	{"/deviceType", "urn:schemas-upnp-org:device:MediaServer:1"},
	{"/pnpx:X_hardwareId", "VEN_0000&amp;DEV_0000&amp;REV_01 VEN_0033&amp;DEV_0001&amp;REV_01"},
	{"/pnpx:X_compatibleId", "MS_DigitalMediaDeviceClass_DMS_V001"},
	{"/pnpx:X_deviceCategory", "MediaDevices"},
	{"/df:X_deviceCategory", "Multimedia.DMS"},
	{"/microsoft:magicPacketWakeSupported xmlns:microsoft=\"urn:schemas-microsoft-com:WMPNSS-1-0\"", "0"},
	{"/sec:ProductCap", "smi,DCM10,getMediaInfo.sec,getCaptionInfo.sec"},
	{"/sec:X_ProductCap","smi,DCM10,getMediaInfo.sec,getCaptionInfo.sec"},
	{"/friendlyName", friendly_name},	/* required */
	{"/manufacturer", ROOTDEV_MANUFACTURER},		/* required */
	{"/manufacturerURL", ROOTDEV_MANUFACTURERURL},	/* optional */
	{"/modelDescription", ROOTDEV_MODELDESCRIPTION}, /* recommended */
	{"/modelName", modelname},	/* required */
	{"/modelNumber", modelnumber},
	{"/modelURL", ROOTDEV_MODELURL},
	{"/serialNumber", serialnumber},
	{"/UDN", uuidvalue},	/* required */
	{"/dlna:X_DLNADOC xmlns:dlna=\"urn:schemas-dlna-org:device-1-0\"", "DMS-1.50"},

	{"iconList", INITHELPER((25),4)},
	{"serviceList", INITHELPER((48),3)},

	{"icon", INITHELPER(29,5)},
	{"icon", INITHELPER(34,5)},
	{"icon", INITHELPER(39,5)},
	{"icon", INITHELPER(44,5)},

	{"/mimetype", "image/png"},
	{"/width", "48"},
	{"/height", "48"},
	{"/depth", "24"},
	{"/url", "/icons/sm.png"},

	{"/mimetype", "image/png"},
	{"/width", "120"},
	{"/height", "120"},
	{"/depth", "24"},
	{"/url", "/icons/lrg.png"},

	{"/mimetype", "image/jpeg"},
	{"/width", "48"},
	{"/height", "48"},
	{"/depth", "24"},
	{"/url", "/icons/sm.jpg"},

	{"/mimetype", "image/jpeg"},
	{"/width", "120"},
	{"/height", "120"},
	{"/depth", "24"},
	{"/url", "/icons/lrg.jpg"},

	{"service", INITHELPER(52,5)},
	{"service", INITHELPER(57,5)},
	{"service", INITHELPER(62,5)},

	{"/serviceType", "urn:schemas-upnp-org:service:ContentDirectory:1"},
	{"/serviceId", "urn:upnp-org:serviceId:ContentDirectory"},
	{"/controlURL", CONTENTDIRECTORY_CONTROLURL},
	{"/eventSubURL", CONTENTDIRECTORY_EVENTURL},
	{"/SCPDURL", CONTENTDIRECTORY_PATH},

	{"/serviceType", "urn:schemas-upnp-org:service:ConnectionManager:1"},
	{"/serviceId", "urn:upnp-org:serviceId:ConnectionManager"},
	{"/controlURL", CONNECTIONMGR_CONTROLURL},
	{"/eventSubURL", CONNECTIONMGR_EVENTURL},
	{"/SCPDURL", CONNECTIONMGR_PATH},

	{"/serviceType", "urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1"},
	{"/serviceId", "urn:microsoft.com:serviceId:X_MS_MediaReceiverRegistrar"},
	{"/controlURL", X_MS_MEDIARECEIVERREGISTRAR_CONTROLURL},
	{"/eventSubURL", X_MS_MEDIARECEIVERREGISTRAR_EVENTURL},
	{"/SCPDURL", X_MS_MEDIARECEIVERREGISTRAR_PATH},

	{0, 0}
};

/* iterative subroutine using a small stack
 * This way, the progam stack usage is kept low */
static char *
genXML(char * str, int * len, int * tmplen,
                   const struct XMLElt * p)
{
	uint16_t i, j, k;
	int top;
	const char * eltname, *s;
	char c;
	char element[64];
	struct {
		unsigned short i;
		unsigned short j;
		const char * eltname;
	} pile[16]; /* stack */
	top = -1;
	i = 0;	/* current node */
	j = 1;	/* i + number of nodes*/
	for(;;)
	{
		eltname = p[i].eltname;
		if(!eltname)
			return str;
		if(eltname[0] == '/')
		{
			str = strcat_char(str, len, tmplen, '<');
			str = strcat_str(str, len, tmplen, eltname+1);
			str = strcat_char(str, len, tmplen, '>');
			str = strcat_str(str, len, tmplen, p[i].data);
			str = strcat_char(str, len, tmplen, '<');
			sscanf(eltname, "%s", element);
			str = strcat_str(str, len, tmplen, element);
			str = strcat_char(str, len, tmplen, '>');
			for(;;)
			{
				if(top < 0)
					return str;
				i = ++(pile[top].i);
				j = pile[top].j;

				if(i==j)
				{
					str = strcat_char(str, len, tmplen, '<');
					str = strcat_char(str, len, tmplen, '/');
					s = pile[top].eltname;
					for(c = *s; c > ' '; c = *(++s))
						str = strcat_char(str, len, tmplen, c);
					str = strcat_char(str, len, tmplen, '>');
					top--;
				}
				else
					break;
			}
		}
		else
		{
			str = strcat_char(str, len, tmplen, '<');
			str = strcat_str(str, len, tmplen, eltname);
			str = strcat_char(str, len, tmplen, '>');
			k = i;
			i = (unsigned long)p[k].data & 0xffff;
			j = i + ((unsigned long)p[k].data >> 16);
			top++;
			pile[top].i = i;
			pile[top].j = j;
			pile[top].eltname = eltname;
		}
	}
}

/* genRootDesc() :
 * - Generate the root description of the UPnP device.
 * - the len argument is used to return the length of
 *   the returned string. 
 * - tmp_uuid argument is used to build the uuid string */
char *
genRootDesc(int * len)
{
	char * str;
	int tmplen;
	tmplen = 2560;
	str = (char *)malloc(tmplen);
	if(str == NULL)
		return NULL;
	* len = strlen(xmlver);
	memcpy(str, xmlver, *len + 1);
	str = genXML(str, len, &tmplen, rootDesc);
	str[*len] = '\0';
	return str;
}
