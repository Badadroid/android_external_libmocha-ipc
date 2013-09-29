/**
 * This file is part of mocha-ril.
 *
 * Copyright (C) 2012 Paul Kocialkowski <contact@paulk.fr>
 * Copyright (C) 2013 Dominik Marszk <dmarszk@gmail.com>
 * Copyright (C) 2013 Nikolay Volkov <volk204@mail.ru>
 *
 * mocha-ril is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mocha-ril is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mocha-ril.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define LOG_TAG "RIL-Mocha-SS"
#include <utils/Log.h>

#include "mocha-ril.h"
#include "util.h"
#include <tapi_ss.h>


void ipc_ss_ussd_response(void* data)
{
	ALOGE("%s: test me!", __func__);

	char *message[2];
	unsigned int i,data_dec_len = 0;; 
	int result = 0;
	char *data_dec = NULL;
	tapiSsCallback* ssResp = (tapiSsCallback*)(data);

	memset(message, 0, sizeof(message));

	data_dec_len = ssResp->strLen;

	/* receiving "DONE!" from AMSS*/
	if (strncmp("DONE!", (char*)ssResp->ussdStr, 5) == 0)
	{
		ALOGD("receiving DONE!, ignore it");
		return;
	} 

	asprintf(&message[0], "%d", ssResp->SsType);
	ril_data.state.ussd_state = ssResp->SsType;

	switch (ssResp->dcs)
	{
		case 0x48:
	
			ALOGD("USSD Rx encoding is UCS2");

			message[1] = malloc(data_dec_len * 4 + 1);
			char *ucs2 = (char*)ssResp->ussdStr;

			for (i = 0; i < data_dec_len; i += 2) {
				int c = (ucs2[i] << 8) | ucs2[1 + i];
				result += utf8_write(message[1], result, c);
			}
			message[1][result] = '\0';
			break;
		case 0xF:

			ALOGD("USSD Rx encoding is GSM7, but we receive it in ASCII from AMSS");

			asprintf(&message[1], "%s", (char*)ssResp->ussdStr);
			DEBUG_I("%s: message -  %s ", __func__, (char*)ssResp->ussdStr);
			message[1][data_dec_len] = '\0';
			break;

		default:

			ALOGD("USSD Rx encoding %x is unknown, assuming ASCII",ssResp->dcs);

			asprintf(&message[1], "%s", (char*)ssResp->ussdStr);
			DEBUG_I("%s: message -  %s ", __func__, (char*)ssResp->ussdStr);
			message[1][data_dec_len] = '\0';

			break;

	}

	ril_request_unsolicited(RIL_UNSOL_ON_USSD, message, sizeof(message));

	for (i = 0; i < sizeof(message) / sizeof(char *); i++) {
		if (message[i] != NULL)
			free(message[i]);
	}
}

void ipc_ss_error_response(void* data)
{
	ALOGE("%s: test me!", __func__);
	char *message[2];
	unsigned int i,data_dec_len = 0;; 
	char *data_dec = NULL;

	memset(message, 0, sizeof(message));

	asprintf(&message[0], "%d", USSD_TERMINATED_BY_NET);
	asprintf(&message[1], "%d", (int)(data));

	ril_data.state.ussd_state = USSD_TERMINATED_BY_NET;

	ril_request_unsolicited(RIL_UNSOL_ON_USSD, message, sizeof(message));

	for (i = 0; i < sizeof(message) / sizeof(char *); i++) {
		if (message[i] != NULL)
			free(message[i]);
	}

}

void ril_request_send_ussd(RIL_Token t, void *data, size_t datalen)
{
	ALOGE("%s: test me!", __func__);
	tapiSsSendUssd *ussd_req;
	tapiSsResponse *ss_resp;

	DEBUG_I("%s: message - %s ", __func__, (char *)data);

	switch(ril_data.state.ussd_state) {
		case USSD_ACTION_REQUIRE:
			ss_resp = malloc(sizeof(tapiSsResponse));
			memset(ss_resp, 0, sizeof(tapiSsResponse));

			if (strlen((char *)data) > 0)
			{
				ss_resp->rspType = 2;
				ss_resp->indType = USSD_ACTION_REQUIRE; 
				ss_resp->dcs = 0xF;
				ss_resp->strLen = strlen((char *)data);
				memcpy(ss_resp->rspString, data, strlen((char *)data));
			}
			else
			{
				DEBUG_I("%s: USSD message is empty, aborting", __func__);
				
				ss_resp->rspType = 3;
				ss_resp->indType = USSD_ACTION_REQUIRE; 
				ril_data.state.ussd_state = USSD_NOT_SUPPORT;
			}
			tapi_ss_ussd_resp(ss_resp);

			if (ss_resp != NULL)
				free(ss_resp);
			break;
		case USSD_NO_ACTION_REQUIRE:
		case USSD_TERMINATED_BY_NET:
		case USSD_OTHER_CLIENT:
		case USSD_NOT_SUPPORT:
		case USSD_TIME_OUT:
		default:
			if (strlen((char *)data) > 0)
			{	

				ussd_req = malloc(sizeof(tapiSsSendUssd));
				memset(ussd_req, 0, sizeof(tapiSsSendUssd));	

				ussd_req->bUnknown = 0;
				ussd_req->dcs = 0xF;
				ussd_req->strLen = strlen((char *)data);
				memcpy(ussd_req->ussdStr, data, strlen((char *)data));

				tapi_ss_send_ussd_string_request(ussd_req);

				if (ussd_req != NULL)
					free(ussd_req);
			}
			break;

		}

	if (strlen((char *)data) > 0)
		ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);
	else
		ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);

}

void ril_request_cancel_ussd(RIL_Token t, void *data, size_t datalen)
{
	ALOGE("%s: test me!", __func__);
	ril_data.state.ussd_state = USSD_TERMINATED_BY_NET;
	tapiSsResponse *ss_resp;

	ss_resp = malloc(sizeof(tapiSsResponse));
	memset(ss_resp, 0, sizeof(tapiSsResponse));

	ss_resp->rspType = 3;
	ss_resp->indType = USSD_ACTION_REQUIRE; 

	tapi_ss_ussd_resp(ss_resp);

	ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);

	if (ss_resp != NULL)
		free(ss_resp); 
}


