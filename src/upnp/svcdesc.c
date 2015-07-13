//
// Created by Pontus Karlsson on 13/07/15.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "svcdesc.h"
#include "minidlna.h"
#include "utils.h"
#include "xml.h"


static const char root_service[] =
        "scpd xmlns=\"urn:schemas-upnp-org:service-1-0\"";

static const char * const upnptypes[] =
        {
                "string",
                "boolean",
                "ui2",
                "ui4",
                "i4",
                "uri",
                "int",
                "bin.base64"
        };

static const char * const upnpdefaultvalues[] =
        {
                0,
                "Unconfigured"
        };

static const char * const upnpallowedvalues[] =
        {
                0,			/* 0 */
                "DSL",			/* 1 */
                "POTS",
                "Cable",
                "Ethernet",
                0,
                "Up",			/* 6 */
                "Down",
                "Initializing",
                "Unavailable",
                0,
                "TCP",			/* 11 */
                "UDP",
                0,
                "Unconfigured",		/* 14 */
                "IP_Routed",
                "IP_Bridged",
                0,
                "Unconfigured",		/* 18 */
                "Connecting",
                "Connected",
                "PendingDisconnect",
                "Disconnecting",
                "Disconnected",
                0,
                "ERROR_NONE",		/* 25 */
                0,
                "OK",			/* 27 */
                "ContentFormatMismatch",
                "InsufficientBandwidth",
                "UnreliableChannel",
                "Unknown",
                0,
                "Input",		/* 33 */
                "Output",
                0,
                "BrowseMetadata",	/* 36 */
                "BrowseDirectChildren",
                0,
                "COMPLETED",		/* 39 */
                "ERROR",
                "IN_PROGRESS",
                "STOPPED",
                0,
                RESOURCE_PROTOCOL_INFO_VALUES,		/* 44 */
                0,
                "0",			/* 46 */
                0,
                "",			/* 48 */
                0
        };



/* genServiceDesc() :
 * Generate service description with allowed methods and
 * related variables. */
char * genServiceDesc(int * len, const struct serviceDesc * s)
{
    int i, j;
    const struct action * acts;
    const struct stateVar * vars;
    const struct argument * args;
    const char * p;
    char * str;
    int tmplen;
    tmplen = 2048;
    str = (char *)malloc(tmplen);
    if(str == NULL)
        return NULL;
    *len = strlen(xmlver);
    memcpy(str, xmlver, *len + 1);

    acts = s->actionList;
    vars = s->serviceStateTable;

    str = strcat_char(str, len, &tmplen, '<');
    str = strcat_str(str, len, &tmplen, root_service);
    str = strcat_char(str, len, &tmplen, '>');

    str = strcat_str(str, len, &tmplen,
                     "<specVersion><major>1</major><minor>0</minor></specVersion>");

    i = 0;
    str = strcat_str(str, len, &tmplen, "<actionList>");
    while(acts[i].name)
    {
        str = strcat_str(str, len, &tmplen, "<action><name>");
        str = strcat_str(str, len, &tmplen, acts[i].name);
        str = strcat_str(str, len, &tmplen, "</name>");
        /* argument List */
        args = acts[i].args;
        if(args)
        {
            str = strcat_str(str, len, &tmplen, "<argumentList>");
            j = 0;
            while(args[j].dir)
            {
                str = strcat_str(str, len, &tmplen, "<argument><name>");
                p = vars[args[j].relatedVar].name;
                str = strcat_str(str, len, &tmplen, (args[j].name ? args[j].name : p));
                str = strcat_str(str, len, &tmplen, "</name><direction>");
                str = strcat_str(str, len, &tmplen, args[j].dir==1?"in":"out");
                str = strcat_str(str, len, &tmplen,
                                 "</direction><relatedStateVariable>");
                str = strcat_str(str, len, &tmplen, p);
                str = strcat_str(str, len, &tmplen,
                                 "</relatedStateVariable></argument>");
                j++;
            }
            str = strcat_str(str, len, &tmplen,"</argumentList>");
        }
        str = strcat_str(str, len, &tmplen, "</action>");
        i++;
    }
    str = strcat_str(str, len, &tmplen, "</actionList><serviceStateTable>");
    i = 0;
    while(vars[i].name)
    {
        str = strcat_str(str, len, &tmplen,
                         "<stateVariable sendEvents=\"");
        str = strcat_str(str, len, &tmplen, (vars[i].itype & EVENTED)?"yes":"no");
        str = strcat_str(str, len, &tmplen, "\"><name>");
        str = strcat_str(str, len, &tmplen, vars[i].name);
        str = strcat_str(str, len, &tmplen, "</name><dataType>");
        str = strcat_str(str, len, &tmplen, upnptypes[vars[i].itype & 0x0f]);
        str = strcat_str(str, len, &tmplen, "</dataType>");
        if(vars[i].iallowedlist)
        {
            str = strcat_str(str, len, &tmplen, "<allowedValueList>");
            for(j=vars[i].iallowedlist; upnpallowedvalues[j]; j++)
            {
                str = strcat_str(str, len, &tmplen, "<allowedValue>");
                str = strcat_str(str, len, &tmplen, upnpallowedvalues[j]);
                str = strcat_str(str, len, &tmplen, "</allowedValue>");
            }
            str = strcat_str(str, len, &tmplen, "</allowedValueList>");
        }

        if(vars[i].idefault)
        {
            str = strcat_str(str, len, &tmplen, "<defaultValue>");
            str = strcat_str(str, len, &tmplen, upnpdefaultvalues[vars[i].idefault]);
            str = strcat_str(str, len, &tmplen, "</defaultValue>");
        }
        str = strcat_str(str, len, &tmplen, "</stateVariable>");
        i++;
    }
    str = strcat_str(str, len, &tmplen, "</serviceStateTable></scpd>");
    str[*len] = '\0';
    return str;
}

char * genEventVars(int * len, const struct serviceDesc * s, const char * servns)
{
    const struct stateVar * v;
    char * str;
    int tmplen;
    char buf[512];
    tmplen = 512;
    str = (char *)malloc(tmplen);
    if(str == NULL)
        return NULL;
    *len = 0;
    v = s->serviceStateTable;
    snprintf(buf, sizeof(buf), "<e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\" xmlns:s=\"%s\">", servns);
    str = strcat_str(str, len, &tmplen, buf);
    while(v->name) {
        if(v->itype & EVENTED) {
            snprintf(buf, sizeof(buf), "<e:property><%s>", v->name);
            str = strcat_str(str, len, &tmplen, buf);
            switch(v->ieventvalue) {
                case 0:
                    break;
                case 255:	/* Magical values should go around here */
                    if( strcmp(v->name, "SystemUpdateID") == 0 )
                    {
                        snprintf(buf, sizeof(buf), "%d", updateID);
                        str = strcat_str(str, len, &tmplen, buf);
                    }
                    break;
                default:
                    str = strcat_str(str, len, &tmplen, upnpallowedvalues[v->ieventvalue]);
            }
            snprintf(buf, sizeof(buf), "</%s></e:property>", v->name);
            str = strcat_str(str, len, &tmplen, buf);
        }
        v++;
    }
    str = strcat_str(str, len, &tmplen, "</e:propertyset>");
    str[*len] = '\0';
    return str;
}