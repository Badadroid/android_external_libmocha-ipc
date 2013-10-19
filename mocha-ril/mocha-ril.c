/**
 * This file is part of mocha-ril.
 *
 * Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
 * Copyright (C) 2011-2012 Paul Kocialkowski <contact@oaulk.fr>
 * Copyright (C) 2012 KB <kbjetdroid@gmail.com>
 * Copyright (C) 2012-2013 Dominik Marszk <dmarszk@gmail.com>
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

#define LOG_TAG "RIL-Mocha"

#include <time.h>
#include <pthread.h>

#include <utils/Log.h>
#include <telephony/ril.h>

#include "mocha-ril.h"
#include "util.h"

#include <fm.h>
#include <drv.h>
#include <tapi.h>
#include <proto.h>
#include <sim.h>
#include <lbs.h>
#include <samsung-ril-socket.h>

/**
 * Samsung-RIL TODO:
 * Everything? :]
 */

/**
 * RIL data
 */

struct ril_data ril_data;

/**
 * RIL requests
 */

int ril_request_id_get(void)
{
	ril_data.request_id++;
	ril_data.request_id %= 0xff;

	return ril_data.request_id;
}

int ril_request_id_set(int id)
{
	id %= 0xff;

	while(ril_data.request_id < id) {
		ril_data.request_id++;
		ril_data.request_id %= 0xff;
	}

	return ril_data.request_id;
}

int ril_request_register(RIL_Token t, int id)
{
	struct ril_request_info *request;
	struct list_head *list_end;
	struct list_head *list;

	request = calloc(1, sizeof(struct ril_request_info));
	if(request == NULL)
		return -1;

	request->token = t;
	request->id = id;
	request->canceled = 0;

	list_end = ril_data.requests;
	while(list_end != NULL && list_end->next != NULL)
		list_end = list_end->next;

	list = list_head_alloc((void *) request, list_end, NULL);

	if(ril_data.requests == NULL)
		ril_data.requests = list;

	return 0;
}

void ril_request_unregister(struct ril_request_info *request)
{
	struct list_head *list;

	if(request == NULL)
		return;

	list = ril_data.requests;
	while(list != NULL) {
		if(list->data == (void *) request) {
			memset(request, 0, sizeof(struct ril_request_info));
			free(request);

			if(list == ril_data.requests)
				ril_data.requests = list->next;

			list_head_free(list);

			break;
		}
list_continue:
		list = list->next;
	}
}

struct ril_request_info *ril_request_info_find_id(int id)
{
	struct ril_request_info *request;
	struct list_head *list;

	list = ril_data.requests;
	while(list != NULL) {
		request = (struct ril_request_info *) list->data;
		if(request == NULL)
			goto list_continue;

		if(request->id == id)
			return request;

list_continue:
		list = list->next;
	}

	return NULL;
}

struct ril_request_info *ril_request_info_find_token(RIL_Token t)
{
	struct ril_request_info *request;
	struct list_head *list;

	list = ril_data.requests;
	while(list != NULL) {
		request = (struct ril_request_info *) list->data;
		if(request == NULL)
			goto list_continue;

		if(request->token == t)
			return request;

list_continue:
		list = list->next;
	}

	return NULL;
}

int ril_request_set_canceled(RIL_Token t, int canceled)
{
	struct ril_request_info *request;

	request = ril_request_info_find_token(t);
	if(request == NULL)
		return -1;

	request->canceled = canceled ? 1 : 0;

	return 0;
}

int ril_request_get_canceled(RIL_Token t)
{
	struct ril_request_info *request;

	request = ril_request_info_find_token(t);
	if(request == NULL)
		return -1;

	return request->canceled;
}

RIL_Token ril_request_get_token(int id)
{
	struct ril_request_info *request;

	request = ril_request_info_find_id(id);
	if(request == NULL)
		return (RIL_Token) 0x00;

	return request->token;
}

int ril_request_get_id(RIL_Token t)
{
	struct ril_request_info *request;
	int id, rc;

	request = ril_request_info_find_token(t);
	if(request != NULL)
		return request->id;

	id = ril_request_id_get();
	rc = ril_request_register(t, id);
	if(rc < 0)
		return -1;

	return id;	
}

void ril_request_complete(RIL_Token t, RIL_Errno e, void *data, size_t length)
{
	struct ril_request_info *request;
	int canceled = 0;

	request = ril_request_info_find_token(t);
	if(request == NULL)
		goto complete;

	canceled = ril_request_get_canceled(t);
	ril_request_unregister(request);

	if(canceled)
		return;

complete:
	ril_data.env->OnRequestComplete(t, e, data, length);
}

void ril_request_unsolicited(int request, void *data, size_t length)
{
	ril_data.env->OnUnsolicitedResponse(request, data, length);
}

void ril_request_timed_callback(RIL_TimedCallback callback, void *data, const struct timeval *time)
{
	ril_data.env->RequestTimedCallback(callback, data, time);
}

/**
 * RIL tokens
 * this should be called after syssec data loading and after receiving system info from modem
 */

void ril_tokens_check(void)
{
	if(ril_data.tokens.baseband_version != 0) {
		if(ril_data.state.radio_state != RADIO_STATE_OFF) {
			ril_request_baseband_version(ril_data.tokens.baseband_version);
			ril_data.tokens.baseband_version = 0;
		}
	}
	if(ril_data.tokens.get_imei != 0) {
		if(cached_imei[0] != 0x00) {
			ril_request_get_imei(ril_data.tokens.get_imei);
			ril_data.tokens.get_imei = 0;
		}
	}
	if(ril_data.tokens.get_imsi != 0) {
		if (ril_data.cached_imsi[0] != 0x00) {
			ril_request_get_imsi(ril_data.tokens.get_imsi);
			ril_data.tokens.get_imsi = 0;
		}
	}
}

void srs_dispatch(struct srs_message *message)
{
	if(message == NULL)
		return;
		
	RIL_LOCK();
	
	switch(message->command) {
		case SRS_CONTROL_PING:
			srs_control_ping(message);
			break;
		case SRS_SND_SET_VOLUME:
			srs_snd_set_volume(message);
			break;
		case SRS_SND_SET_AUDIO_PATH:
			srs_snd_set_audio_path(message);
			break;
		case SRS_SND_1MIC_NS_CTRL:
			srs_snd_1mic_ns_ctrl(message);
			break;
		case SRS_SND_PCM_IF_CTRL:
			srs_snd_pcm_if_ctrl(message);
			break;
		default:
			ALOGD("Unhandled command: (%04x)", message->command);
			break;
	}
	
	RIL_UNLOCK();
}

int ril_modem_check(void)
{
	if(ril_data.ipc_packet_client == NULL)
		return -1;

	if(ril_data.ipc_packet_client->state != RIL_CLIENT_READY)
		return -2;
		
	return 0;
}

void ril_on_request(int request, void *data, size_t datalen, RIL_Token t)
{
	int check;

	RIL_LOCK();
	ALOGV("Request from RILD ID - %d", request);
	check = ril_modem_check();
	if(check < 0)
	{
		ALOGE("ril_modem_check() returned %d => replying RIL_E_RADIO_NOT_AVAILABLE", check);
		ril_request_complete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
	}

	switch(request) {
		/* MISC */
		case RIL_REQUEST_GET_IMEI:
			ril_request_get_imei(t);
			break;
		case RIL_REQUEST_GET_IMSI:
			ril_request_get_imsi(t);
			break;
		/* PWR */
		case RIL_REQUEST_RADIO_POWER:
			ril_request_radio_power(t, data, datalen);
			break;
		case RIL_REQUEST_BASEBAND_VERSION:
			ril_request_baseband_version(t);
			break;
#if 0
		/* SAT */
		case RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE:
			requestSatSendTerminalResponse(t, data, datalen);
			break;
		case RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND:
			requestSatSendEnvelopeCommand(t, data, datalen);
			break;
		case RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM:
			ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);
			break;
#endif		/* SIM */
		case RIL_REQUEST_GET_SIM_STATUS:
			ril_request_get_sim_status(t);
			break;
#if 0
		case RIL_REQUEST_SIM_IO:
			ril_request_sim_io(t, data, datalen);
			break;
#endif
		case RIL_REQUEST_ENTER_SIM_PIN:
			ril_request_enter_sim_pin(t, data, datalen);
			break;
		case RIL_REQUEST_ENTER_SIM_PUK:
			ril_request_enter_sim_puk(t, data, datalen);
			break;
		case RIL_REQUEST_QUERY_FACILITY_LOCK:
			ril_request_query_facility_lock(t, data, datalen);
			break;
		case RIL_REQUEST_SET_FACILITY_LOCK:
			ril_request_set_facility_lock(t, data, datalen);
			break;
		case RIL_REQUEST_CHANGE_SIM_PIN:
			ril_request_change_sim_pin(t, data, datalen);
			break;
		/* NET */
		case RIL_REQUEST_OPERATOR:
			ril_request_operator(t);
			break;
		case RIL_REQUEST_VOICE_REGISTRATION_STATE:
			ril_request_voice_registration_state(t);
			break;
		case RIL_REQUEST_DATA_REGISTRATION_STATE:
			ril_request_data_registration_state(t);
			break;
		case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS:
			ril_request_query_available_networks(t);
			break;
		case RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC:
			ril_request_set_network_selection_automatic(t);
			break;
		case RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL:
			ril_request_set_network_selection_manual(t, data, datalen);
			break;
		case RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE:
			ril_request_query_network_selection_mode(t);
			break;
		case RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE:
			ril_request_get_preferred_network_type(t);
			break;
		case RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE:
			ril_request_set_preferred_network_type(t, data, datalen);
			break;
		/* SMS */
		case RIL_REQUEST_SEND_SMS:
			ril_request_send_sms(t, data, datalen);
			break;
		case RIL_REQUEST_SEND_SMS_EXPECT_MORE:
			ril_request_send_sms_expect_more(t, data, datalen);
			break;
		case RIL_REQUEST_SMS_ACKNOWLEDGE:
			/* implemented in AMMS */
			ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);
			break;
		/* CALL */
		case RIL_REQUEST_DIAL:
			ril_request_dial(t, data, datalen);
			break;
		case RIL_REQUEST_GET_CURRENT_CALLS:
			ril_request_get_current_calls(t);
			break;
		case RIL_REQUEST_HANGUP:
			ril_request_hangup(t, data, datalen);
			break;
		case RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND:
			ril_request_hangup_waiting_or_background(t);
			break;
		case RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND:
			ril_request_hangup_foreground_resume_background(t);
			break;
		case RIL_REQUEST_ANSWER:
			ril_request_answer(t);
			break;
		case RIL_REQUEST_LAST_CALL_FAIL_CAUSE:
			ril_request_last_call_fail_cause(t);
			break;
		case RIL_REQUEST_DTMF:
			ril_request_dtmf(t, data, datalen);
			break;
		case RIL_REQUEST_DTMF_START:
			ril_request_dtmf_start(t, data, datalen);
			break;
		case RIL_REQUEST_DTMF_STOP:
			ril_request_dtmf_stop(t);
			break;
		case RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE:
			ril_request_switch_waiting_or_holding_and_active(t);
			break;
		/* GPRS */
		case RIL_REQUEST_SETUP_DATA_CALL:
			ril_request_setup_data_call(t, data, datalen);
			break;
		case RIL_REQUEST_DEACTIVATE_DATA_CALL:
			ril_request_deactivate_data_call(t, data, datalen);
			break;
		case RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE:
			ril_request_last_data_call_fail_cause(t);
			break;
		case RIL_REQUEST_DATA_CALL_LIST:
			ril_request_data_call_list(t);
			break;
		/* SND */
		case RIL_REQUEST_SET_MUTE:
			ril_request_set_mute(t, data, datalen);
			break;
		/* SS */
		case RIL_REQUEST_SEND_USSD:
			ril_request_send_ussd(t, data, datalen);
			break;
		case RIL_REQUEST_CANCEL_USSD:
			ril_request_cancel_ussd(t, data, datalen);
			break;
		/* OTHER */
		case RIL_REQUEST_SCREEN_STATE:
			ril_request_screen_state(t, data, datalen);			
			break;

		default:
			ALOGE("Request not implemented: %d", request);
			ril_request_complete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
			break;
	}
	
	RIL_UNLOCK();
}

RIL_RadioState ril_on_state_request(void)
{
	return ril_data.state.radio_state;
}

int ril_on_supports(int request)
{
	return 1;
}

void ril_on_cancel(RIL_Token t)
{
	ril_request_set_canceled(t, 1);
}

const char *ril_get_version(void)
{
	return RIL_VERSION_STRING;
}

/**
 * RIL init function
 */

void ril_install_ipc_callbacks(void)
{
	ipc_register_ril_cb(CP_SYSTEM_START, ipc_cp_system_start);
	ipc_register_ril_cb(NETWORK_RADIO_INFO, ipc_network_radio_info);
	ipc_register_ril_cb(NETWORK_SELECT, ipc_network_select);
	ipc_register_ril_cb(NETWORK_CELL_INFO, ipc_cell_info);
	ipc_register_ril_cb(NETWORK_NITZ_INFO_IND, ipc_network_nitz_info);
	ipc_register_ril_cb(NETWORK_SEARCH_CNF, ipc_network_search_cnf);
	ipc_register_ril_cb(NETWORK_SELECT_CNF, ipc_network_select_cnf);
	ipc_register_ril_cb(CALL_INCOMING_IND, ipc_call_incoming);
	ipc_register_ril_cb(CALL_END_IND, ipc_call_end);
	ipc_register_ril_cb(CALL_SETUP_IND, ipc_call_setup_ind);
	ipc_register_ril_cb(CALL_ALERT, ipc_call_alert);
	ipc_register_ril_cb(CALL_CONNECTED, ipc_call_connected);
	ipc_register_ril_cb(CALL_DTMF_START, ipc_call_dtmf_start);
	ipc_register_ril_cb(CALL_DTMF_STOP, ipc_call_dtmf_stop);
	ipc_register_ril_cb(CALL_HOLD, ipc_call_hold);
	ipc_register_ril_cb(CALL_SWAP, ipc_call_swap);
	ipc_register_ril_cb(CALL_ACTIVATE, ipc_call_activate);
	ipc_register_ril_cb(CALL_ERROR, ipc_call_error);
	ipc_register_ril_cb(SIM_OPEN, ipc_sim_open);
	ipc_register_ril_cb(SIM_STATUS, ipc_sim_status);
	ipc_register_ril_cb(LOCK_STATUS, ipc_lock_status);
	ipc_register_ril_cb(NETTEXT_INCOMING, ipc_incoming_sms);
	ipc_register_ril_cb(NETTEXT_SEND_CALLBACK, ipc_sms_send_status);
	ipc_register_ril_cb(SIM_IO_RESPONSE, ipc_sim_io_response);
	ipc_register_ril_cb(SS_USSD_CALLBACK, ipc_ss_ussd_response);
	ipc_register_ril_cb(SS_ERROR, ipc_ss_error_response);
	ipc_register_ril_cb(PROTO_START_NETWORK_CNF, ipc_proto_start_network_cnf);
	ipc_register_ril_cb(PROTO_STOP_NETWORK_CNF, ipc_proto_stop_network_cnf);
	ipc_register_ril_cb(PROTO_RECEIVE_DATA_IND, ipc_proto_receive_data_ind);
}
 
void ril_data_init(void)
{
	memset(&ril_data, 0, sizeof(ril_data));

	pthread_mutex_init(&ril_data.mutex, NULL);
	ril_data.state.sim_state = SIM_STATE_NOT_READY;
	ril_data.inDevice = SND_INPUT_MAIN_MIC;
	ril_data.outDevice = SND_OUTPUT_EARPIECE;
	load_ril_config();
}

/**
 * RIL interface
 */

static const RIL_RadioFunctions ril_ops = {
	SAMSUNG_RIL_VERSION,
	ril_on_request,
	ril_on_state_request,
	ril_on_supports,
	ril_on_cancel,
	ril_get_version
};

const RIL_RadioFunctions *RIL_Init(const struct RIL_Env *env, int argc, char **argv)
{
	struct ril_client *ipc_packet_client;
	struct ril_client *srs_client;
	int rc;
    pthread_t networkInit;

	if(env == NULL)
		return NULL;

	ril_data_init();
	ril_data.env = (struct RIL_Env *) env;

	RIL_LOCK();
	
	ipc_init();
	ril_install_ipc_callbacks();

	ALOGI("Creating IPC client");

	ipc_packet_client = ril_client_new(&ipc_client_funcs);
	rc = ril_client_create(ipc_packet_client);

	if(rc < 0) {
		ALOGE("IPC client creation failed.");
		goto srs;
	}

	rc = ril_client_thread_start(ipc_packet_client);

	if(rc < 0) {
		ALOGE("IPC thread creation failed.");
		goto srs;
	}

	ril_data.ipc_packet_client = ipc_packet_client;
	ALOGI("IPC client ready");
	
srs:
	ALOGD("Creating SRS client");

	srs_client = ril_client_new(&srs_client_funcs);
	rc = ril_client_create(srs_client);

	if(rc < 0) {
		ALOGE("SRS client creation failed.");
		goto end;
	}

	rc = ril_client_thread_start(srs_client);

	if(rc < 0) {
		ALOGE("SRS thread creation failed.");
		goto end;
	}

	ril_data.srs_client = srs_client;
	ALOGD("SRS client ready");

end:
	ril_data.state.radio_state = RADIO_STATE_OFF;
	ril_data.state.power_state = POWER_STATE_OFF;

	RIL_UNLOCK();

	return &ril_ops;
}
