/**
 * This file is part of mocha-ril.
 *
 * Copyright (C) 2013 Paul Kocialkowski <contact@paulk.fr>
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
#include "sim.h"
#include <sim.h>
#include <tapi_network.h>
#include <misc.h>

void ril_sim_init(void)
{
	sim_atk_open();	
	sim_open_to_modem(4);
}

void ipc_sim_open(void *data)
{
	uint8_t* buf = (uint8_t*)(data);
	int size_delta;
	simEventPacketHeader* simEvent = (simEventPacketHeader*)(data);
	if (simEvent->eventStatus == SIM_CARD_NOT_PRESENT) {
		ALOGE("SIM_ABSENT");
		ipc_sim_status((void*)SIM_STATE_ABSENT);
	}
	if (simEvent->eventStatus == SIM_OK) {
		ALOGE("SIM_READY");
		ipc_sim_status((void*)SIM_STATE_READY);
	}
	size_delta = 0x473 - simEvent->bufLen; /* 0x473 is event buffer length in "modern" packets. Packets on some SW versions are shorter. */
	/* copying IMSI in BCD format from sim packet */
	int imsi_len = buf[0xAE - size_delta];
	int i = 0;
	for (i = 0; i < imsi_len; i++)
		ril_data.cached_bcd_imsi[i] = buf[i+0xB2 - size_delta];
	/*Converting IMSI out of dat stuff to ASCII*/
	imsi_bcd2ascii(ril_data.cached_imsi, ril_data.cached_bcd_imsi, imsi_len);
	/* Clean print of IMSI*/
	memset(buf + 0xB2 - size_delta, 0xFF, imsi_len);
	ril_tokens_check();
	ipc_boot8_mode(1);
}

void ipc_sim_status(void *data)
{
	ALOGE("%s: test me!", __func__);

	ril_sim_state sim_state;
	sim_state = (int) data;

	/* Update radio state based on SIM state */
	ril_state_update(sim_state);

	if (sim_state == SIM_STATE_PIN)
		ril_data.state.bPinLock = 1;

	if (sim_state == SIM_STATE_READY && ril_data.smsc_number[0] == 0)
		//request SMSC number
		sim_get_file_info(4, 0x6f42);

	ril_request_unsolicited(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0);	
}

void ipc_lock_status(void* data)
{
	ALOGD("%s: test me!", __func__);
	simEventPacketHeader* simEvent = (simEventPacketHeader*)(data);
	if (simEvent->eventStatus == SIM_OK)
	{
		uint8_t *buf = (uint8_t *)data + sizeof(simEventPacketHeader);
		lockStatus* pinSt = (lockStatus*)(buf);
		int attempts = -1;
		switch(pinSt->status)
		{
			case 0:
				ALOGD("%s : Correct password ", __func__);
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
					sim_send_oem_data(0x4, SIM_OEM_REQUEST_GET_OPEN_DATA, NULL, 0);
				}
				else if(ril_data.tokens.puk_status != 0)
				{
					ril_request_complete(ril_data.tokens.puk_status, RIL_E_SUCCESS, &attempts, sizeof(attempts));
					ril_data.tokens.puk_status = 0;
					sim_send_oem_data(0x4, SIM_OEM_REQUEST_GET_OPEN_DATA, NULL, 0);
				}
				else
				{
					ALOGE("%s: Received correct password info but no CHV token is 	active.", __func__);
				}
				return;
			case 1:
				ALOGE("%s : Wrong password ", __func__);
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
					ALOGE("%s: Received wrong password info but no CHV token is 	active.", __func__);
				}
				return;
			case 2:
				ALOGE("%s : Wrong password and no attempts left!", __func__);
				attempts = 0;
				if (ril_data.tokens.set_facility_lock != 0)
				{
					ril_request_complete(ril_data.tokens.set_facility_lock, 	RIL_E_PASSWORD_INCORRECT, &attempts, sizeof(attempts));
					ril_data.tokens.set_facility_lock = 0;
				}
				else if (ril_data.tokens.change_sim_pin != 0)
				{
					ril_request_complete(ril_data.tokens.change_sim_pin, 	RIL_E_PASSWORD_INCORRECT, &attempts, sizeof(attempts));
					ril_data.tokens.change_sim_pin = 0;
				}
				else if (ril_data.tokens.pin_status != 0)
				{
					ril_request_complete(ril_data.tokens.pin_status, 	RIL_E_PASSWORD_INCORRECT, &attempts, sizeof(attempts));
					ipc_sim_status((void*)SIM_STATE_PUK);
					ril_data.tokens.pin_status = 0;
				}
				else if(ril_data.tokens.puk_status != 0)
				{
					ril_request_complete(ril_data.tokens.puk_status, 	RIL_E_PASSWORD_INCORRECT, &attempts, sizeof(attempts));
					ipc_sim_status((void*)SIM_STATE_BLOCKED);
					ril_data.tokens.puk_status = 0;
				}
				else
				{
					ALOGE("%s: Received no attempts left info but no CHV token is 	active.", __func__);
				}
				return;
		}
	}
	else
	{
		if (ril_data.tokens.set_facility_lock != 0)
		{
			ril_request_complete(ril_data.tokens.set_facility_lock, 	RIL_E_GENERIC_FAILURE, NULL, 0);
			ril_data.tokens.set_facility_lock = 0;
		}
		else if (ril_data.tokens.change_sim_pin != 0)
		{
			ril_request_complete(ril_data.tokens.change_sim_pin, RIL_E_GENERIC_FAILURE, NULL, 0);
			ril_data.tokens.change_sim_pin = 0;
		}
		else if (ril_data.tokens.pin_status != 0)
		{
			ril_request_complete(ril_data.tokens.pin_status, RIL_E_GENERIC_FAILURE, NULL, 0);
			ril_data.tokens.pin_status = 0;
		}
		else if(ril_data.tokens.puk_status != 0)
		{
			ril_request_complete(ril_data.tokens.puk_status, RIL_E_GENERIC_FAILURE, NULL, 0);
			ril_data.tokens.puk_status = 0;
		}
		else
		{
			ALOGE("%s: General Error but no CHV token is active.", __func__);
		}
	}
}

void ipc_sim_smsc_number(void* data)
{
	char tmp[3];
	uint8_t *buf;
	int i, dataLen;

	simDataResponse* simResponse = (simDataResponse*)((uint8_t *)data + sizeof(simEventPacketHeader));

	buf = (uint8_t *)data + sizeof(simEventPacketHeader) + sizeof(simDataResponse);
	dataLen = simResponse->bufLen;

	if  (ril_data.smsc_number[0] == 0)
	{
		for (i = dataLen - 15; i < dataLen + (int)buf[dataLen - 15] - 14; i++)
			{
				sprintf(tmp, "%02x", buf[i]);
				strcat(ril_data.smsc_number, tmp);
			}
		ALOGD("%s : SMSC number: %s", __func__, ril_data.smsc_number);
	}
}

void ipc_sim_io_response(void* data)
{
	struct ril_request_sim_io_info *sim_io_info;
	struct sim_file_response sim_file_response;
	char *sim_response = NULL;
	char tmp[3];
	uint8_t buffer[15];
	uint8_t *buf;
	int i;
	RIL_SIM_IO_Response sim_io_response;

	sim_io_info = ril_request_sim_io_info_find_token(ril_data.tokens.sim_io);
	if (sim_io_info == NULL) {
		ALOGE("%s : Unable to find SIM I/O in the list!", __func__);

		// Send the next SIM I/O in the list
		ril_request_sim_io_next();

		return;
	}

	simEventPacketHeader* simEvent = (simEventPacketHeader*)(data);
	memset(&sim_io_response, 0, sizeof(sim_io_response));

	switch(simEvent->eventType)
	{
		case SIM_EVENT_FILE_INFO:
			buf = (uint8_t *)data + sizeof(simEventPacketHeader);
			fileInfoEvent* fileInfo = (fileInfoEvent*)(buf);

			memset(&sim_file_response, 0, sizeof(sim_file_response));

			sim_file_response.file_id[0] = ((fileInfo->fileId >> 8) & 0xFF);
			sim_file_response.file_id[1] = ((fileInfo->fileId >> 0) & 0xFF);

			sim_file_response.file_size[0] = ((fileInfo->fileSize >> 8) & 0xFF);
			sim_file_response.file_size[1] = ((fileInfo->fileSize >> 0) & 0xFF);

			// Fallback to EF
			sim_file_response.file_type = SIM_FILE_TYPE_EF;
			for (i = 0 ; i < sim_file_ids_count ; i++) {
				if (sim_io_info->fileid == sim_file_ids[i].file_id) {
					sim_file_response.file_type = sim_file_ids[i].type;
					break;
				}
			}

			sim_file_response.access_condition[0] = 0x00;
			sim_file_response.access_condition[1] = 0xff;
			sim_file_response.access_condition[2] = 0xff;

			sim_file_response.file_status = 0x01;
			sim_file_response.file_length = 0x02;

			sim_file_response.file_structure = SIM_FILE_STRUCTURE_LINEAR_FIXED;

			if (fileInfo->fileId == 0x2FE2 || fileInfo->fileId == 0x6F46)
				sim_file_response.file_structure = SIM_FILE_STRUCTURE_TRANSPARENT;

			sim_file_response.record_length = fileInfo->recordSize;

			sim_response = (char *) malloc(sizeof(struct sim_file_response) * 2 + 1);

			bin2hex((void *) &sim_file_response, sizeof(struct sim_file_response), sim_response);
			sim_io_response.simResponse = sim_response;

			break;
		case SIM_EVENT_READ_FILE:
			buf = (uint8_t *)data + sizeof(simEventPacketHeader) + sizeof(simDataResponse);

			// Copy the data as-is
			sim_response = calloc(1,sim_io_info->p3 * 2 + 1);
			for (i = 0; i <= sim_io_info->p3; i++)
			{
				sprintf(tmp, "%02X", buf[i]);
				strcat(sim_response, tmp);
			}
			sim_response[sim_io_info->p3 * 2] = '\0';
			sim_io_response.simResponse = sim_response;
			break;
		default:
			sim_io_response.simResponse = NULL;
			break;
	}

	switch (simEvent->eventStatus)
	{
		case SIM_OK:
			sim_io_response.sw1 = 0x90;
			sim_io_response.sw2 = 0x00;
			break;
		case SIM_FILE_NOT_FOUND:
			sim_io_response.sw1 = 0x94;
			sim_io_response.sw2 = 0x04;
			break;
		default:
			sim_io_response.sw1 = 0x6F;
			sim_io_response.sw2 = 0x00;
			break;
	}

	ril_request_complete(ril_data.tokens.sim_io, RIL_E_SUCCESS, &sim_io_response, sizeof(sim_io_response));

	if (sim_io_response.simResponse != NULL) {
		ALOGD("%s: SIM response: %s", __func__, sim_io_response.simResponse);
		free(sim_io_response.simResponse);
	}

	ril_request_sim_io_unregister(sim_io_info);
	// Send the next SIM I/O in the list
	ril_request_sim_io_next();
	return;
error:
	ril_request_complete(ril_data.tokens.sim_io, RIL_E_GENERIC_FAILURE, NULL, 0);

}

void ril_request_get_sim_status(RIL_Token t)
{
	ALOGD("%s: test me!", __func__);

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
		ALOGE("%s: pin exceeds maximum length", __func__);
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
	ALOGD("%s: test me!", __func__);

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
			sim_enable_chv(0x4, 0x0, password);
		}
		else if (!strcmp(lock, "0"))
		{
		ALOGE("%s: Request for disabling PIN", __func__);
			//disable PIN Lock
			sim_disable_chv(0x4, 0x0, password);
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
	ALOGD("%s: test me!", __func__);
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


/*
* SIM I/O
*/
int ril_request_sim_io_register(RIL_Token t, int command, int fileid,
	int p1, int p2, int p3, char *data, int length,
	struct ril_request_sim_io_info **sim_io_p)
{
	ALOGD("%s: fileid 0x%x", __func__, fileid);

	struct ril_request_sim_io_info *sim_io;
	struct list_head *list_end;
	struct list_head *list;

	sim_io = calloc(1, sizeof(struct ril_request_sim_io_info));
	if (sim_io == NULL)
		return -1;

	sim_io->command = command;
	sim_io->fileid = fileid;
	sim_io->p1 = p1;
	sim_io->p2 = p2;
	sim_io->p3 = p3;
	sim_io->data = data;
	sim_io->length = length;
	sim_io->waiting = 1;
	sim_io->token = t;

	list_end = ril_data.sim_io;
	while (list_end != NULL && list_end->next != NULL)
		list_end = list_end->next;

	list = list_head_alloc((void *) sim_io, list_end, NULL);

	if (ril_data.sim_io == NULL)
		ril_data.sim_io = list;

	if (sim_io_p != NULL)
		*sim_io_p = sim_io;
	return 0;
}

void ril_request_sim_io_unregister(struct ril_request_sim_io_info *sim_io)
{
	struct list_head *list;

	if (sim_io == NULL)
		return;

	list = ril_data.sim_io;
	while (list != NULL) {
		if (list->data == (void *) sim_io) {
			memset(sim_io, 0, sizeof(struct ril_request_sim_io_info));
			free(sim_io);

			if (list == ril_data.sim_io)
				ril_data.sim_io = list->next;

			list_head_free(list);

			break;
		}
list_continue:
		list = list->next;
	}
}

struct ril_request_sim_io_info *ril_request_sim_io_info_find(void)
{
	struct ril_request_sim_io_info *sim_io;
	struct list_head *list;

	list = ril_data.sim_io;
	while (list != NULL) {
		sim_io = (struct ril_request_sim_io_info *) list->data;
		if (sim_io == NULL)
			goto list_continue;

		return sim_io;
list_continue:
		list = list->next;
	}
	return NULL;
}

struct ril_request_sim_io_info *ril_request_sim_io_info_find_token(RIL_Token t)
{
	struct ril_request_sim_io_info *sim_io;
	struct list_head *list;

	list = ril_data.sim_io;
	while (list != NULL) {
		sim_io = (struct ril_request_sim_io_info *) list->data;
		if (sim_io == NULL)
			goto list_continue;

		if (sim_io->token == t)
			return sim_io;

list_continue:
		list = list->next;
	}

	return NULL;
}

void ril_request_sim_io_next(void)
{
	struct ril_request_sim_io_info *sim_io;
	int rc;

	ril_data.tokens.sim_io = RIL_TOKEN_NULL;

	sim_io = ril_request_sim_io_info_find();
	if (sim_io == NULL)
		return;

	ALOGD("%s: fileid 0x%x", __func__, sim_io->fileid);

	sim_io->waiting = 0;
	ril_data.tokens.sim_io = sim_io->token;



	ril_request_sim_io_complete(sim_io->token, sim_io->command, sim_io->fileid,
		sim_io->p1, sim_io->p2, sim_io->p3, sim_io->data, sim_io->length);

	if (sim_io->data != NULL)
		free(sim_io->data);
	sim_io->data = NULL;
	sim_io->length = 0;
}

void ril_request_sim_io_complete(RIL_Token t, int command, int fileid,
int p1, int p2, int p3, char *data, int length)
{
	simDataRequest sim_data;
	int i;

	switch(command)
	{
		case SIM_COMMAND_GET_RESPONSE:
			ALOGD("%s: fileid 0x%x", __func__, fileid);
			sim_get_file_info(4, fileid);
			break;
		case SIM_COMMAND_READ_RECORD:
		case SIM_COMMAND_READ_BINARY:
			sim_data.fileId = fileid;
			sim_data.fileType = SIM_FILE_TYPE_MF;
			for (i = 0 ; i < sim_file_ids_count ; i++) {
				if (sim_data.fileId == sim_file_ids[i].file_id) {
					sim_data.fileType = sim_file_ids[i].type;
					break;
				}
			}
			sim_data.unk0 = 0x00;
			sim_data.recordIndex = p1;
			sim_data.unk1 = 0x00;
			sim_data.unk2 = 0x00;
			sim_data.simInd2 = 0x01;
			sim_data.size = p3;
			sim_data.unk3 = 0x00;
			sim_data.unk4 = 0x00;
			sim_data.unk5 = 0x00;
			sim_data.unk6 = 0x00;
			sim_data.unk7 = 0x00;

			DEBUG_I("%s: Reading Record: fileId = 0x%x, packet no. %d\n", __func__, sim_data.fileId, sim_data.recordIndex);
			if (command == SIM_COMMAND_READ_RECORD)
				sim_read_file_record(0x5, &sim_data);
			else
				sim_read_file_binary(0x5, &sim_data);
			break;
		case SIM_COMMAND_UPDATE_BINARY:
		case SIM_COMMAND_UPDATE_RECORD:
		case SIM_COMMAND_SEEK:
		default:
			ALOGE("%s: Unsupported SIM_IO comand 0x%x", __func__, command);
			break;
	}
}

void ril_request_sim_io(RIL_Token t, void *data, int length)
{

	struct ril_request_sim_io_info *sim_io_info = NULL;
	RIL_SIM_IO_v6 *sim_io = NULL;

	void *sim_io_data = NULL;
	int sim_io_data_length = 0;
	int rc;

	if (data == NULL || length < (int) sizeof(*sim_io))
		goto error;

	sim_io = (RIL_SIM_IO_v6 *) data;

	if (sim_io->command != SIM_COMMAND_GET_RESPONSE)
		if (sim_io->command != SIM_COMMAND_READ_RECORD)
			if (sim_io->command != SIM_COMMAND_READ_BINARY)
			{
				ALOGE("%s: Unsupported SIM_IO comand 0x%x", __func__, sim_io->command);
				goto error;
			}

	// SIM IO data should be a string if present
	if (sim_io->data != NULL) {
		sim_io_data_length = strlen(sim_io->data) / 2;
		if (sim_io_data_length > 0) {
			sim_io_data = calloc(1, sim_io_data_length);
			hex2bin(sim_io->data, sim_io_data_length * 2, sim_io_data);
		}
	}

	rc = ril_request_sim_io_register(t, sim_io->command, sim_io->fileid,
		sim_io->p1, sim_io->p2, sim_io->p3, sim_io_data, sim_io_data_length,
		&sim_io_info);

	if (rc < 0 || sim_io_info == NULL) {
		ALOGE("%s: Unable to add the request to the list", __func__);
		ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
		if (sim_io_data != NULL)
			free(sim_io_data);

		// Send the next SIM I/O in the list
		ril_request_sim_io_next();
	}

	if (ril_data.tokens.sim_io != RIL_TOKEN_NULL) {
		ALOGD("%s: Another SIM I/O is being processed, adding to the list", __func__);
		return;
	}

	sim_io_info->waiting = 0;
	ril_data.tokens.sim_io = t;

	ril_request_sim_io_complete(t, sim_io->command, sim_io->fileid,
		sim_io->p1, sim_io->p2, sim_io->p3, sim_io_data, sim_io_data_length);

	if (sim_io_data != NULL)
		free(sim_io_data);
	sim_io_info->data = NULL;
	sim_io_info->length = 0;

	return;
error:
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}
