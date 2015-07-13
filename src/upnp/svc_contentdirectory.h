//
// Created by Pontus Karlsson on 13/07/15.
//

#ifndef MINIDLNA_SVC_CONTENTDIRECTORY_H
#define MINIDLNA_SVC_CONTENTDIRECTORY_H

#include "svcdesc.h"

#define CONTENTDIRECTORY_PATH			"/ContentDir.xml"
#define CONTENTDIRECTORY_CONTROLURL		"/ctl/ContentDir"
#define CONTENTDIRECTORY_EVENTURL		"/evt/ContentDir"

char * genContentDirectory(int * len);

char * getVarsContentDirectory(int * len);

#endif //MINIDLNA_SVC_CONTENTDIRECTORY_H
