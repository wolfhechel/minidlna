//
// Created by Pontus Karlsson on 13/07/15.
//

#ifndef MINIDLNA_SVCDESC_H
#define MINIDLNA_SVCDESC_H

#include <stddef.h>

/* for service description */
struct serviceDesc {
    const struct action * actionList;
    const struct stateVar * serviceStateTable;
};

struct action {
    const char * name;
    const struct argument * args;
};

struct argument {
    const char * name;		/* the name of the argument */
    unsigned char dir;		/* 1 = in, 2 = out */
    unsigned char relatedVar;	/* index of the related variable */
};

#define EVENTED 1<<7
struct stateVar {
    const char * name;
    unsigned char itype;	/* MSB: sendEvent flag, 7 LSB: index in upnptypes */
    unsigned char idefault;	/* default value */
    unsigned char iallowedlist;	/* index in allowed values list */
    unsigned char ieventvalue;	/* fixed value returned or magical values */
};

char * genServiceDesc(int * len, const struct serviceDesc * s);

char * genEventVars(int * len, const struct serviceDesc * s, const char * servns);

#endif //MINIDLNA_SVCDESC_H
