//
// Created by Pontus Karlsson on 13/07/15.
//

#ifndef MINIDLNA_SVC_CONNECTIONMGR_H
#define MINIDLNA_SVC_CONNECTIONMGR_H

#include "svcdesc.h"

#define CONNECTIONMGR_PATH			    "/ConnectionMgr.xml"
#define CONNECTIONMGR_CONTROLURL		"/ctl/ConnectionMgr"
#define CONNECTIONMGR_EVENTURL			"/evt/ConnectionMgr"

char * genConnectionManager(int * len);

char * getVarsConnectionManager(int * len);

#endif //MINIDLNA_SVC_CONNECTIONMGR_H
