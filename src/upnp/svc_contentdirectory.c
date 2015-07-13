//
// Created by Pontus Karlsson on 13/07/15.
//

#include "svc_contentdirectory.h"


static const struct argument GetSearchCapabilitiesArgs[] =
        {
                {"SearchCaps", 2, 10},
                {0, 0}
        };

static const struct argument GetSortCapabilitiesArgs[] =
        {
                {"SortCaps", 2, 11},
                {0, 0}
        };

static const struct argument GetSystemUpdateIDArgs[] =
        {
                {"Id", 2, 12},
                {0, 0}
        };

static const struct argument BrowseArgs[] =
        {
                {"ObjectID", 1, 1},
                {"BrowseFlag", 1, 4},
                {"Filter", 1, 5},
                {"StartingIndex", 1, 7},
                {"RequestedCount", 1, 8},
                {"SortCriteria", 1, 6},
                {"Result", 2, 2},
                {"NumberReturned", 2, 8},
                {"TotalMatches", 2, 8},
                {"UpdateID", 2, 9},
                {0, 0}
        };

static const struct argument SearchArgs[] =
        {
                {"ContainerID", 1, 1},
                {"SearchCriteria", 1, 3},
                {"Filter", 1, 5},
                {"StartingIndex", 1, 7},
                {"RequestedCount", 1, 8},
                {"SortCriteria", 1, 6},
                {"Result", 2, 2},
                {"NumberReturned", 2, 8},
                {"TotalMatches", 2, 8},
                {"UpdateID", 2, 9},
                {0, 0}
        };

static const struct action ContentDirectoryActions[] =
        {
                {"GetSearchCapabilities", GetSearchCapabilitiesArgs}, /* R */
                {"GetSortCapabilities", GetSortCapabilitiesArgs}, /* R */
                {"GetSystemUpdateID", GetSystemUpdateIDArgs}, /* R */
                {"Browse", BrowseArgs}, /* R */
                {"Search", SearchArgs}, /* O */
                {0, 0}
        };

static const struct stateVar ContentDirectoryVars[] =
        {
                {"TransferIDs", 0|EVENTED, 0, 0, 48}, /* 0 */
                {"A_ARG_TYPE_ObjectID", 0, 0},
                {"A_ARG_TYPE_Result", 0, 0},
                {"A_ARG_TYPE_SearchCriteria", 0, 0},
                {"A_ARG_TYPE_BrowseFlag", 0, 0, 36},
                /* Allowed Values : BrowseMetadata / BrowseDirectChildren */
                {"A_ARG_TYPE_Filter", 0, 0}, /* 5 */
                {"A_ARG_TYPE_SortCriteria", 0, 0},
                {"A_ARG_TYPE_Index", 3, 0},
                {"A_ARG_TYPE_Count", 3, 0},
                {"A_ARG_TYPE_UpdateID", 3, 0},
                {"SearchCapabilities", 0, 0},
                {"SortCapabilities", 0, 0},
                {"SystemUpdateID", 3|EVENTED, 0, 0, 255},
                {0, 0}
        };

static const struct serviceDesc scpdContentDirectory =
        { ContentDirectoryActions, ContentDirectoryVars };


/* genContentDirectory() :
 * Generate the ContentDirectory xml description */
char *
genContentDirectory(int * len)
{
    return genServiceDesc(len, &scpdContentDirectory);
}


char *
getVarsContentDirectory(int * l)
{
    return genEventVars(l,
                        &scpdContentDirectory,
                        "urn:schemas-upnp-org:service:ContentDirectory:1");
}