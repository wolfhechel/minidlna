//
// Created by Pontus Karlsson on 13/07/15.
//

#include "svc_x_ms_mediareceiverregistrar.h"


static const struct argument GetIsAuthorizedArgs[] =
        {
                {"DeviceID", 1, 0},
                {"Result", 2, 3},
                {NULL, 0, 0}
        };

static const struct argument GetIsValidatedArgs[] =
        {
                {"DeviceID", 1, 0},
                {"Result", 2, 3},
                {NULL, 0, 0}
        };

static const struct argument GetRegisterDeviceArgs[] =
        {
                {"RegistrationReqMsg", 1, 1},
                {"RegistrationRespMsg", 2, 2},
                {NULL, 0, 0}
        };

static const struct action X_MS_MediaReceiverRegistrarActions[] =
        {
                {"IsAuthorized", GetIsAuthorizedArgs}, /* R */
                {"IsValidated", GetIsValidatedArgs}, /* R */
                {"RegisterDevice", GetRegisterDeviceArgs},
                {0, 0}
        };

static const struct stateVar X_MS_MediaReceiverRegistrarVars[] =
        {
                {"A_ARG_TYPE_DeviceID", 0, 0},
                {"A_ARG_TYPE_RegistrationReqMsg", 7, 0},
                {"A_ARG_TYPE_RegistrationRespMsg", 7, 0},
                {"A_ARG_TYPE_Result", 6, 0},
                {"AuthorizationDeniedUpdateID", 3|EVENTED, 0},
                {"AuthorizationGrantedUpdateID", 3|EVENTED, 0},
                {"ValidationRevokedUpdateID", 3|EVENTED, 0},
                {"ValidationSucceededUpdateID", 3|EVENTED, 0},
                {0, 0}
        };


static const struct serviceDesc scpdX_MS_MediaReceiverRegistrar =
        { X_MS_MediaReceiverRegistrarActions, X_MS_MediaReceiverRegistrarVars };

/* genX_MS_MediaReceiverRegistrar() :
 * Generate the X_MS_MediaReceiverRegistrar xml description */
char *
genX_MS_MediaReceiverRegistrar(int * len)
{
    return genServiceDesc(len, &scpdX_MS_MediaReceiverRegistrar);
}

char *
getVarsX_MS_MediaReceiverRegistrar(int * l)
{
    return genEventVars(l,
                        &scpdX_MS_MediaReceiverRegistrar,
                        "urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1");
}
