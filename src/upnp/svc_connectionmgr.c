//
// Created by Pontus Karlsson on 13/07/15.
//

#include "svc_connectionmgr.h"



/* For ConnectionManager */
static const struct argument GetProtocolInfoArgs[] =
        {
                {"Source", 2, 0},
                {"Sink", 2, 1},
                {NULL, 0, 0}
        };

static const struct argument PrepareForConnectionArgs[] =
        {
                {"RemoteProtocolInfo", 1, 6},
                {"PeerConnectionManager", 1, 4},
                {"PeerConnectionID", 1, 7},
                {"Direction", 1, 5},
                {"ConnectionID", 2, 7},
                {"AVTransportID", 2, 8},
                {"RcsID", 2, 9},
                {NULL, 0, 0}
        };

static const struct argument ConnectionCompleteArgs[] =
        {
                {"ConnectionID", 1, 7},
                {NULL, 0, 0}
        };

static const struct argument GetCurrentConnectionIDsArgs[] =
        {
                {"ConnectionIDs", 2, 2},
                {NULL, 0, 0}
        };

static const struct argument GetCurrentConnectionInfoArgs[] =
        {
                {"ConnectionID", 1, 7},
                {"RcsID", 2, 9},
                {"AVTransportID", 2, 8},
                {"ProtocolInfo", 2, 6},
                {"PeerConnectionManager", 2, 4},
                {"PeerConnectionID", 2, 7},
                {"Direction", 2, 5},
                {"Status", 2, 3},
                {NULL, 0, 0}
        };

static const struct action ConnectionManagerActions[] =
        {
                {"GetProtocolInfo", GetProtocolInfoArgs}, /* R */
                //OPTIONAL {"PrepareForConnection", PrepareForConnectionArgs}, /* R */
                //OPTIONAL {"ConnectionComplete", ConnectionCompleteArgs}, /* R */
                {"GetCurrentConnectionIDs", GetCurrentConnectionIDsArgs}, /* R */
                {"GetCurrentConnectionInfo", GetCurrentConnectionInfoArgs}, /* R */
                {0, 0}
        };

static const struct stateVar ConnectionManagerVars[] =
        {
                {"SourceProtocolInfo", 0|EVENTED, 0, 0, 44}, /* required */
                {"SinkProtocolInfo", 0|EVENTED, 0, 0, 48}, /* required */
                {"CurrentConnectionIDs", 0|EVENTED, 0, 0, 46}, /* required */
                {"A_ARG_TYPE_ConnectionStatus", 0, 0, 27}, /* required */
                {"A_ARG_TYPE_ConnectionManager", 0, 0}, /* required */
                {"A_ARG_TYPE_Direction", 0, 0, 33}, /* required */
                {"A_ARG_TYPE_ProtocolInfo", 0, 0}, /* required */
                {"A_ARG_TYPE_ConnectionID", 4, 0}, /* required */
                {"A_ARG_TYPE_AVTransportID", 4, 0}, /* required */
                {"A_ARG_TYPE_RcsID", 4, 0}, /* required */
                {0, 0}
        };

static const struct serviceDesc scpdConnectionManager =
        { ConnectionManagerActions, ConnectionManagerVars };

/* genConnectionManager() :
 * Generate the ConnectionManager xml description */
char *
genConnectionManager(int * len)
{
    return genServiceDesc(len, &scpdConnectionManager);
}

char *
getVarsConnectionManager(int * l)
{
    return genEventVars(l,
                        &scpdConnectionManager,
                        "urn:schemas-upnp-org:service:ConnectionManager:1");
}