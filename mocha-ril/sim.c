/**
 * This file is part of mocha-ril.
 *
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

#define LOG_TAG "RIL-Mocha-SIM"
#include <utils/Log.h>

#include "mocha-ril.h"
#include "util.h"
#include <sim.h>
#include <tapi_network.h>

void ril_sim_init(void)
{
	sim_atk_open();	
	sim_open_to_modem(4);
}

void ipc_sim_open(void *data)
{
	uint8_t* buf = (uint8_t*)(data);
	simEventPacketHeader* simEvent = (simEventPacketHeader*)(data);
	if (simEvent->eventStatus == SIM_CARD_NOT_PRESENT) {
		DEBUG_I("SIM_ABSENT");
		ipc_sim_status((void*)SIM_STATE_ABSENT);
	}
	if (simEvent->eventStatus == SIM_OK) {
		DEBUG_I("SIM_READY");
		ipc_sim_status((void*)SIM_STATE_READY);
	}
	/* copying IMSI in BCD format from sim packet */
	int imsi_len = buf[0xAE];
	int i = 0;
	for (i = 0; i < imsi_len; i++)
		ril_data.cached_bcd_imsi[i] = buf[i+0xB2];
	/*Converting IMSI out of dat stuff to ASCII*/
	imsi_bcd2ascii(ril_data.cached_imsi, ril_data.cached_bcd_imsi, imsi_len);
	/* Clean print of IMSI*/
	memset(buf + 0xB2, 0xFF, imsi_len);
	ril_tokens_check();
}

void ipc_sim_status(void *data)
{
	ALOGE("%s: test me!", __func__);

	ril_sim_state sim_state;
	sim_state =(int) data;

	/* Update radio state based on SIM state */
	ril_state_update(sim_state);

	if (sim_state == SIM_STATE_PIN)
		ril_data.state.bPinLock = 1;

	if (sim_state == SIM_STATE_READY && ril_data.smsc_number[0] == 0)
		//request SMSC number
		sim_data_request_to_modem(4, 0x6f42);

	ril_request_unsolicited(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0);	

}

void ipc_lock_status(void* data)
{
	ALOGE("%s: test me!", __func__);
	lockStatus* pinSt = (lockStatus*)(data);
	int attempts = -1;
	switch(pinSt->status){
		case 0:
			DEBUG_I("%s : Correct password ", __func__);
			if (ril_data.tokens.set_facility_lock != 0)
			{
				ril_request_complete(ril_data.tokens.set_facility_lock, RIL_E_SUCCESS, &attempts, sizeof(attempts));
				ril_data.tokens.set_facility_lock = 0;
			}
			else if (ril_data.tokens.change_sim_pin != 0)
			{
				ril_request_complete(ril_data.tokens.change_sim_pin, RIL_E_SUCCESS, &attempts, sizeof(attempts));
				ril_data.tokens.change_sim_pin = 0;
			}
			else if (ril_data.tokens.pin_status != 0)
			{
				ril_request_complete(ril_data.tokens.pin_status, RIL_E_SUCCESS, &attempts, sizeof(attempts));
				ril_data.tokens.pin_status = 0;
				sim_send_oem_data(0x4, SIM_EVENT_VERIFY_PIN1_IND, NULL, 0);
			}
			else if(ril_data.tokens.puk_status != 0)
			{
				ril_request_complete(ril_data.tokens.puk_status, RIL_E_SUCCESS, &attempts, sizeof(attempts));
				ril_data.tokens.puk_status = 0;
				sim_send_oem_data(0x4, SIM_EVENT_VERIFY_PIN1_IND, NULL, 0);
			}
			else
			{
				ALOGE("%s: Received correct password info but no CHV token is active.", __func__);
			}
			return;
		case 1:
			DEBUG_I("%s : Wrong password ", __func__);
			attempts = pinSt->attempts;
			if (ril_data.tokens.set_facility_lock != 0)
			{
				ril_request_complete(ril_data.tokens.set_facility_lock, RIL_E_PASSWORD_INCORRECT, &attempts, sizeof(attempts));
				ril_data.tokens.set_facility_lock = 0;
			}
			else if (ril_data.tokens.change_sim_pin != 0)
			{
				ril_request_complete(ril_data.tokens.change_sim_pin, RIL_E_PASSWORD_INCORRECT, &attempts, sizeof(attempts));
				ril_data.tokens.change_sim_pin = 0;
			}
			else if (ril_data.tokens.pin_status != 0)
			{
				ril_request_complete(ril_data.tokens.pin_status, RIL_E_PASSWORD_INCORRECT, &attempts, sizeof(attempts));
				ril_data.tokens.pin_status = 0;
			}
			else if(ril_data.tokens.puk_status != 0)
			{
				ril_request_complete(ril_data.tokens.puk_status, RIL_E_PASSWORD_INCORRECT, &attempts, sizeof(attempts));
				ril_data.tokens.puk_status = 0;
			}
			else
			{
				ALOGE("%s: Received wrong password info but no CHV token is active.", __func__);
			}
			return;
		case 2:
			DEBUG_I("%s : Wrong password and no attempts left!", __func__);
			attempts = 0;
			if (ril_data.tokens.set_facility_lock != 0)
			{
				ril_request_complete(ril_data.tokens.set_facility_lock, RIL_E_PASSWORD_INCORRECT, &attempts, sizeof(attempts));
				ril_data.tokens.set_facility_lock = 0;
			}
			else if (ril_data.tokens.change_sim_pin != 0)
			{
				ril_request_complete(ril_data.tokens.change_sim_pin, RIL_E_PASSWORD_INCORRECT, &attempts, sizeof(attempts));
				ril_data.tokens.change_sim_pin = 0;
			}
			else if (ril_data.tokens.pin_status != 0)
			{
				ril_request_complete(ril_data.tokens.pin_status, RIL_E_PASSWORD_INCORRECT, &attempts, sizeof(attempts));
				sim_status(SIM_STATE_PUK);
				ril_data.tokens.pin_status = 0;
			}
			else if(ril_data.tokens.puk_status != 0)
			{
				ril_request_complete(ril_data.tokens.puk_status, RIL_E_PASSWORD_INCORRECT, &attempts, sizeof(attempts));
				sim_status(SIM_STATE_BLOCKED);
				ril_data.tokens.puk_status = 0;
			}
			else
			{
				ALOGE("%s: Received no attempts left info but no CHV token is active.", __func__);
			}
			return;
	}
}

void ipc_sim_io_response(void* data)
{
	ALOGE("%s: test me!", __func__);

	char *response, *a;
	uint8_t *buf;
	int i, dataLen;
	RIL_SIM_IO_Response sim_io_response;
	

	sim_data_response* resp = (sim_data_response*)(data);

	ALOGE("%s: FileId = %X", __func__, resp->simDataType);
	ALOGE("%s: bufLen = %d", __func__, resp->bufLen);

	buf = (uint8_t *)data + sizeof(sim_data_response);

	dataLen = resp->bufLen;

	if (resp->simDataType == 0x6f42 && ril_data.smsc_number[0] == 0)
	{

		for (i = dataLen - 15; i < dataLen + (int)buf[dataLen - 15] - 14; i++)
		{
			asprintf(&a, "%02x", buf[i]);
			strcat(ril_data.smsc_number,a);
		}
		DEBUG_I("%s : SMSC number: %s", __func__, ril_data.smsc_number);
		return;				
	}
	
	response = malloc((dataLen * 2) + 1);
	memset(response, 0, (dataLen * 2) + 1);

	for (i = 0; i < dataLen; i++)
	{
		asprintf(&a, "%02x", buf[i]);
		strcat(response,a);
	}
	DEBUG_I("%s : SIM_IO_RESPONSE: %s", __func__, response);
	sim_io_response.sw1 = 144;
	sim_io_response.sw2 = 0;
	sim_io_response.simResponse = response;
//	ril_request_complete(token, RIL_E_SUCCESS, &sim_io_response, sizeof(sim_io_response));
	free(response);
}

void ril_request_get_sim_status(RIL_Token t)
{
	ALOGE("%s: test me!", __func__);

	RIL_CardStatus_v6 card_status;
	ril_sim_state sim_state;
	int app_status_array_length;
	int app_index;
	int i;

	static RIL_AppStatus app_status_array[] = {
		/* SIM_ABSENT = 0 */
		{ RIL_APPTYPE_UNKNOWN, RIL_APPSTATE_UNKNOWN, RIL_PERSOSUBSTATE_UNKNOWN,
		NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
		/* SIM_NOT_READY = 1 */
		{ RIL_APPTYPE_SIM, RIL_APPSTATE_DETECTED, RIL_PERSOSUBSTATE_UNKNOWN,
		NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
		/* SIM_READY = 2 */
		{ RIL_APPTYPE_SIM, RIL_APPSTATE_READY, RIL_PERSOSUBSTATE_READY,
		NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
		/* SIM_PIN = 3 */
		{ RIL_APPTYPE_SIM, RIL_APPSTATE_PIN, RIL_PERSOSUBSTATE_UNKNOWN,
		NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
		/* SIM_PUK = 4 */
		{ RIL_APPTYPE_SIM, RIL_APPSTATE_PUK, RIL_PERSOSUBSTATE_UNKNOWN,
		NULL, NULL, 0, RIL_PINSTATE_ENABLED_BLOCKED, RIL_PINSTATE_UNKNOWN },
		/* SIM_BLOCKED = 4 */
		{ RIL_APPTYPE_SIM, RIL_APPSTATE_PUK, RIL_PERSOSUBSTATE_UNKNOWN,
		NULL, NULL, 0, RIL_PINSTATE_ENABLED_PERM_BLOCKED, RIL_PINSTATE_UNKNOWN },
		/* SIM_NETWORK_PERSO = 6 */
		{ RIL_APPTYPE_SIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_NETWORK,
		NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
		/* SIM_NETWORK_SUBSET_PERSO = 7 */
		{ RIL_APPTYPE_SIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_NETWORK_SUBSET,
		NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
		/* SIM_CORPORATE_PERSO = 8 */
		{ RIL_APPTYPE_SIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_CORPORATE,
		NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
		/* SIM_SERVICE_PROVIDER_PERSO = 9 */
		{ RIL_APPTYPE_SIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_SERVICE_PROVIDER,
		NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
		};

	sim_state = ril_data.state.sim_state;

	/* Card is assumed to be present if not explicitly absent */
	if(ril_data.state.sim_state == SIM_STATE_ABSENT) {
		card_status.card_state = RIL_CARDSTATE_ABSENT;
	} else {
		card_status.card_state = RIL_CARDSTATE_PRESENT;
	}
		
	// Initialize the apps
	for (i = 0 ; i < RIL_CARD_MAX_APPS ; i++) {
		card_status.applications[i] = app_status_array[0];
	}

	// FIXME: How do we know that?
	card_status.universal_pin_state = RIL_PINSTATE_UNKNOWN;


	/* Initialize apps */
	for (i = 0; i < RIL_CARD_MAX_APPS; i++) {
	card_status.applications[i] = app_status_array[i];
	}

	// sim_state corresponds to the app index on the table
	card_status.gsm_umts_subscription_app_index = (int) sim_state;
	card_status.cdma_subscription_app_index = (int) sim_state;
	card_status.num_applications = RIL_CARD_MAX_APPS;

	ril_request_complete(t, RIL_E_SUCCESS, &card_status, sizeof(card_status));
	
}

void ril_state_update(ril_sim_state sim_state)
{
	RIL_RadioState radio_state;

	ril_data.state.sim_state = sim_state;

	/* If power mode isn't at least normal, don't update RIL state */
	if (ril_data.state.power_state != POWER_STATE_NORMAL)
		return;

	switch(sim_state) {
		case SIM_STATE_READY:
			radio_state = RADIO_STATE_SIM_READY;
			tapi_set_subscription_mode(0x1);
			nettext_cb_setup();
			break;
		case SIM_STATE_NOT_READY:
			radio_state = RADIO_STATE_SIM_NOT_READY;
			break;
		case SIM_STATE_ABSENT:
		case SIM_STATE_PIN:
		case SIM_STATE_PUK:
		case SIM_STATE_BLOCKED:
		case SIM_STATE_NETWORK_PERSO:
		case SIM_STATE_NETWORK_SUBSET_PERSO:
		case SIM_STATE_CORPORATE_PERSO:
		case SIM_STATE_SERVICE_PROVIDER_PERSO:
			radio_state = RADIO_STATE_SIM_LOCKED_OR_ABSENT;
			break;
		default:
			radio_state = RADIO_STATE_SIM_NOT_READY;
			break;
	}
	ril_data.state.radio_state = radio_state;
	ril_tokens_check();
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED, NULL, 0);
}

void ril_request_enter_sim_pin(RIL_Token t, void *data, size_t datalen)
{
	char *pin = ((char **) data)[0];

	if (strlen(data) > 16) {
		ALOGE("%s: pin exceeds maximum length", __FUNCTION__);
		ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	}
	sim_verify_chv(0x4, 0x0, pin);
	ril_data.tokens.pin_status = t;
}

void ril_request_enter_sim_puk(RIL_Token t, void *data, size_t datalen)
{
	char *puk;
	char *pin;

	if (data == NULL || datalen < (int) (2 * sizeof(char *)))
		goto error;

	puk = ((char **) data)[0];
	pin = ((char **) data)[1];

	sim_unblock_chv(0x4, 0x2, puk, pin);
	ril_data.tokens.puk_status = t;

	return;
error:
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void ril_request_query_facility_lock(RIL_Token t, void *data, size_t datalen)
{
	char *facility;

	if (data == NULL || datalen < sizeof(char *))
		goto error;

	facility = ((char **) data)[0];

	if (!strcmp(facility, "SC")) {
		ril_request_complete(t, RIL_E_SUCCESS, &ril_data.state.bPinLock, sizeof(int));
	} else {
	ALOGE("%s: unsupported facility: %s", __func__, facility);
		goto error;
	}
	return;
error:
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}
void ril_request_set_facility_lock(RIL_Token t, void *data, size_t datalen)
{
	ALOGE("%s: test me!", __func__);

	char *facility;
	char *lock;
	char *password;

	if (data == NULL || datalen < (int) (4 * sizeof(char *)))
		goto error;

	facility = ((char **) data)[0];
	lock = ((char **) data)[1];
	password = ((char **) data)[2];

	if (!strcmp(facility, "SC")) {
		if (!strcmp(lock, "1"))
		{
		ALOGE("%s: Request for enabling PIN", __func__);
			//enable PIN lock
			sim_disable_chv(0x4, 0x0, password);
		}
		else if (!strcmp(lock, "0"))
		{
		ALOGE("%s: Request for disabling PIN", __func__);
			//disable PIN Lock
			sim_enable_chv(0x4, 0x0, password);
		}
	}
	else
	{
		ALOGE("%s: unsupported facility: %s", __func__, facility);
		goto error;
	}
	ril_data.tokens.set_facility_lock = t;
	return;
error:
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void ril_request_change_sim_pin(RIL_Token t, void *data, size_t datalen)
{
	ALOGE("%s: test me!", __func__);
	char *password_old;
	char *password_new;

	if (data == NULL || datalen < (int) (2 * sizeof(char *)))
		goto error;

	password_old = ((char **) data)[0];
	password_new = ((char **) data)[1];

	sim_change_chv(0x4, 0x0, password_old, password_new);

	ril_data.tokens.change_sim_pin = t;

	return;
error:
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}
