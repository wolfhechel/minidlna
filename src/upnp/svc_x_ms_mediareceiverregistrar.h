//
// Created by Pontus Karlsson on 13/07/15.
//

#ifndef MINIDLNA_SVC_X_MS_MEDIARECEIVERREGISTRAR_H
#define MINIDLNA_SVC_X_MS_MEDIARECEIVERREGISTRAR_H

#include "svcdesc.h"

#define X_MS_MEDIARECEIVERREGISTRAR_PATH	    "/X_MS_MediaReceiverRegistrar.xml"
#define X_MS_MEDIARECEIVERREGISTRAR_CONTROLURL	"/ctl/X_MS_MediaReceiverRegistrar"
#define X_MS_MEDIARECEIVERREGISTRAR_EVENTURL	"/evt/X_MS_MediaReceiverRegistrar"

char * genX_MS_MediaReceiverRegistrar(int * len);

char * getVarsX_MS_MediaReceiverRegistrar(int * len);

#endif //MINIDLNA_SVC_X_MS_MEDIARECEIVERREGISTRAR_H
