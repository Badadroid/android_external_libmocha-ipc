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
 
#define LOG_TAG "RIL-Mocha-NETWORK"
#include <time.h>
#include <utils/Log.h>

#include "mocha-ril.h"
#include "util.h"
#include <tapi_network.h>
#include <tapi_nettext.h>
#include <sim.h>
#include <proto.h>

int ril_net_select_register(char *plmn, tapiNetSearchCnf net_select_entry)
{
	struct ril_net_select *net_select;
	struct list_head *list_end;
	struct list_head *list;

	net_select = calloc(1, sizeof(struct ril_net_select));
	if (net_select == NULL)
		return -1;

	net_select->plmn = plmn;
	net_select->net_select_entry = net_select_entry;

	ALOGE("%s: added plmn %s", __func__, net_select->plmn);

	list_end = ril_data.net_select_list;
	while (list_end != NULL && list_end->next != NULL)
		list_end = list_end->next;

	list = list_head_alloc((void *) net_select, list_end, NULL);

	if (ril_data.net_select_list == NULL)
		ril_data.net_select_list = list;

	return 0;
}

void ril_net_select_unregister(void)
{
	struct list_head *list;

	list = ril_data.net_select_list;
	while (list != NULL) {
		free(list->data);
		ril_data.net_select_list = list->next;
		list_head_free(list);
		list = ril_data.net_select_list;
	}
}

struct ril_net_select *ril_net_select_find_plmn(char *plmn)
{
	struct ril_net_select *net_select;
	struct list_head *list;

	list = ril_data.net_select_list;
	while (list != NULL) {
		net_select = (struct ril_net_select *) list->data;
		if (net_select == NULL)
			goto list_continue;

		if (strcmp(net_select->plmn,plmn) == 0)
			return net_select;

list_continue:
		list = list->next;
	}

	return NULL;
}

int ipc2ril_net_mode(uint32_t mode)
{
	switch (mode) {
		case TAPI_NETWORK_MODE_GSM_900_1800:
			return 1;
		case TAPI_NETWORK_MODE_AUTOMATIC:
		default:
			return 0;
	} 
}

uint32_t ril2ipc_net_mode(int mode)
{
	switch (mode) {
		case 1: // GSM 900/1800
			return TAPI_NETWORK_MODE_GSM_900_1800;
		case 0:
		default: // automatic
			return TAPI_NETWORK_MODE_AUTOMATIC;
	}
}
 
void ipc_network_radio_info(void* data)
{
	tapiRadioInfo* radioInfo = (tapiRadioInfo*)(data);
	RIL_SignalStrength_v6 ss;
	int rssi,asu;

	/* Don't consider this if modem isn't in normal power mode. */
	if(ril_data.state.power_state < POWER_STATE_NORMAL)
		return;

	rssi = (uint32_t)radioInfo->rxLevel;

	switch (rssi) {

	case 10:
		asu = 1;
		break;
	case 25:
		asu = 3;
		break;
	case 45:
		asu = 5;
		break;
	case 60:
		asu = 8;
		break;
	case 75:
		asu = 12;
		break;
	case 90:
		asu = 15;
		break;
	default:
		asu = 1;
		break;
	}

	ALOGD("Signal Strength is %d\n", asu);

	memset(&ss, 0, sizeof(ss));
	memset(&ss.LTE_SignalStrength, -1, sizeof(ss.LTE_SignalStrength));

	ss.GW_SignalStrength.signalStrength = asu;
	ss.GW_SignalStrength.bitErrorRate = 99;

	ril_request_unsolicited(RIL_UNSOL_SIGNAL_STRENGTH, &ss, sizeof(ss));
}

void ipc_network_select(void* data)
{	
	tapiNetworkInfo* netInfo = (tapiNetworkInfo*)(data);
	
	/* Converts IPC network registration status to Android RIL format */
	switch(netInfo->serviceLevel) {
		case TAPI_SERVICE_LEVEL_NONE:
			ril_data.state.reg_state = 0;//0 - Not registered, MT is not currently searching a new operator to register
			break;
		case TAPI_SERVICE_LEVEL_EMERGENCY:
			ril_data.state.reg_state = 12;//12 - Same as 2, but indicates that emergency calls are enabled.
			break;
		case TAPI_SERVICE_LEVEL_FULL:
			ril_data.state.reg_state = 1; //1 - Registered, home network
			break;
		case TAPI_SERVICE_LEVEL_SEARCHING:
			ril_data.state.reg_state = 2;//2 - Not registered, but MT is currently searching a new operator to register
			break;
		default:
			ril_data.state.reg_state = 0;
			break;
	}

	switch(netInfo->psServiceType) {
		case 0:
			ril_data.state.act = RADIO_TECH_UNKNOWN;
			break;
		case 1: /* GPRS */
			ril_data.state.act = RADIO_TECH_GPRS;
			break;
		case 2: /* EDGE */
			ril_data.state.act = RADIO_TECH_EDGE;
			break;
		case 3: /* 3G */
			ril_data.state.act = RADIO_TECH_UMTS;
			break;
		case 4: /* 3G+ */
			ril_data.state.act = RADIO_TECH_HSDPA;
			break;
		default:
			ril_data.state.reg_state = RADIO_TECH_UNKNOWN;
			break;
	}

	strcpy(ril_data.state.SPN, netInfo->spn);
	strcpy(ril_data.state.name, netInfo->name);

	if (ril_data.tokens.network_selection != 0)
	{
		if (netInfo->serviceLevel == TAPI_SERVICE_LEVEL_FULL)
		{
			ril_request_complete(ril_data.tokens.network_selection, RIL_E_SUCCESS, NULL, 0);
			ril_net_select_unregister();
		}
		else
			ril_request_complete(ril_data.tokens.network_selection, RIL_E_ILLEGAL_SIM_OR_ME, NULL, 0);	
		ril_data.tokens.network_selection = 0;
	}

	ril_request_unsolicited(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);
}

void ipc_cell_info(void* data)
{
	tapiCellInfo* cellInfo = (tapiCellInfo*)(data);

	if(cellInfo->bCellChanged)
	{
		ril_data.state.cell_id = (uint32_t)cellInfo->cellId[0] << 24 | (uint32_t)cellInfo->cellId[1] << 16 | (uint32_t)cellInfo->cellId[2] << 8 | (uint32_t)cellInfo->cellId[3];
	}
	if(cellInfo->bRACChanged)
	{
		ril_data.state.rac_id = cellInfo->racId;
	}
	if(cellInfo->bLACChanged)
	{
		ril_data.state.lac_id = (uint16_t)cellInfo->lacId[0] << 8 | (uint16_t)cellInfo->lacId[1];
	}
	if(cellInfo->bPLMNChanged)
	{
		uint16_t mcc = ((cellInfo->plmnId[0] & 0xF) * 100) + (((cellInfo->plmnId[0] >> 4) & 0xF) * 10) + (((cellInfo->plmnId[1]) & 0xF) * 1);
		uint16_t mnc = ((cellInfo->plmnId[2] & 0xF) * 10) + (((cellInfo->plmnId[2] >> 4) & 0xF) * 1);
		sprintf(ril_data.state.proper_plmn, "%3u%2u", mcc, mnc);
	}
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);
}


void ipc_network_nitz_info(void* data)
{
	tapiNitzInfo* nitz;
	char str[128];
	
	ALOGD("Received NITZ... dumping");
	hex_dump(data, 0x70);
	
	nitz = (tapiNitzInfo*) data;

	if (nitz->bNetworkTimeAvail == 0)
		return;

	sprintf(str, "%02u/%02u/%02u,%02u:%02u:%02u+%02d,%02d",
		nitz->year, nitz->month, nitz->day, nitz->hour, nitz->minute, nitz->second, nitz->tz, nitz->dls);

	ril_request_unsolicited(RIL_UNSOL_NITZ_TIME_RECEIVED, str, strlen(str) + 1);
}

void ipc_network_search_cnf(void* data)
{
	struct ril_net_select *net_select;
	struct list_head *list;
	char **response;
	char *plmn;
	int length, count, index;
	unsigned int i, plmn_dec, num_entries;

	num_entries = ((uint8_t *)data)[0];

	ALOGD("%s: Packet with %d entries\n", __func__, num_entries);
	count = 0;

	if (ril_data.net_select_list != NULL)
	{
		ALOGD("%s: List is not empty, cleaning...\n", __func__);
		ril_net_select_unregister();
	}

	for (i = 0; i < num_entries; i++)
	{
		tapiNetSearchCnf *entry = (tapiNetSearchCnf *)(((uint8_t *)data) + (sizeof(tapiNetSearchCnf) * i + 4));

		if (entry->MNC > 99)
			plmn_dec = entry->MCC * 1000 + entry->MNC;
		else
			plmn_dec = entry->MCC * 100 + entry->MNC;

		asprintf(&plmn, "%d", plmn_dec);

		net_select = ril_net_select_find_plmn(plmn);

		if (net_select)
		{
			if (net_select->net_select_entry.systemType < entry->systemType)
				net_select->net_select_entry = *entry;
		}
		else
		{
			ril_net_select_register(plmn, *entry);
			count++;
		}
	}
	ALOGD("%s: List created with %d entries\n", __func__, count);

	length = sizeof(char *) * 4 * count;
	response = (char **) calloc(1, length);
	count = 0;

	list = ril_data.net_select_list;

	while (list != NULL) {
		net_select = (struct ril_net_select *) list->data;

		if (net_select == NULL)
			goto list_continue;

		index = count * 4;
		asprintf(&response[index], "%s", net_select->net_select_entry.name);
		asprintf(&response[index + 1], "%s", net_select->net_select_entry.name);
		asprintf(&response[index + 2], "%s", net_select->plmn);
		if (net_select->net_select_entry.bForbidden)
			response[index + 3] = strdup("forbidden");
		else if (net_select->net_select_entry.bCurrent)
			response[index + 3] = strdup("current");
		else if (net_select->net_select_entry.bAvailable)
			response[index + 3] = strdup("available");
		else
			response[index + 3] = strdup("unknown");
		count++;

list_continue:
		list = list->next;
	}

	ril_request_complete(ril_data.tokens.query_avail_networks, RIL_E_SUCCESS, response, length);

	if (plmn != NULL)
		free(plmn);

	for (i = 0; i < length / sizeof(char *); i++) {
		if (response[i] != NULL)
			free(response[i]);
	}
}

void ipc_network_select_cnf(void* data)
{
	tapiNetworkInfo* netInfo = (tapiNetworkInfo*)(data);
	if (netInfo->serviceLevel == TAPI_SERVICE_LEVEL_FULL)
	{
		ril_request_complete(ril_data.tokens.network_selection, RIL_E_SUCCESS, NULL, 0);
		ril_net_select_unregister();
	}
	else
		ril_request_complete(ril_data.tokens.network_selection, RIL_E_ILLEGAL_SIM_OR_ME, NULL, 0);
	ril_data.tokens.network_selection = 0;
}

void network_start(void)
{
	tapiStartupNetworkInfo start_info;
	start_info.bAutoSelection = 1;
	start_info.bPoweronGprsAttach = 1;
	start_info.networkOrder = 1;
	start_info.serviceDomain = 0;
	start_info.align1 = 0;
	start_info.networkMode = ril_data.state.net_mode;
	start_info.subscriptionMode = 0;
	start_info.bFlightMode = 0;	
	start_info.unknown = 0x02;
	start_info.align2 = 0;
	/* TODO: Check if it can be executed from tapi_init, or do we need to wait for network select or some other packet. */
	tapi_network_startup(&start_info);
	
	tapi_nettext_set_preferred_memory(1); /* let's hope it means phone, not sim */
	tapi_nettext_set_net_burst(0); /* disable */

	if (ril_data.state.sim_state == SIM_STATE_READY)
		sim_status(2);

}

void ril_request_operator(RIL_Token t)
{
	char *response[3];
	char *plmn;
	unsigned int mcc, mnc;
	int plmn_entries;
	unsigned int i;

	if (ril_data.state.reg_state == 1) {
		memset(response, 0, sizeof(response));

		if (ril_data.state.name[0] != 0)
			{
			asprintf(&response[0], "%s", ril_data.state.name);
			asprintf(&response[1], "%s", ril_data.state.name);
			}
		else if (ril_data.state.SPN[0] != 0)
			{
			asprintf(&response[0], "%s", ril_data.state.SPN);
			asprintf(&response[1], "%s", ril_data.state.SPN);
			}

		asprintf(&response[2], "%s", ril_data.state.proper_plmn);

		ril_request_complete(t, RIL_E_SUCCESS, response, sizeof(response));
		for (i = 0; i < sizeof(response) / sizeof(char *); i++) {
			if (response[i] != NULL)
				free(response[i]);
		}
	}
	else
	{
		ril_request_complete(t, RIL_E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW, NULL, 0);
	}
}

void ril_request_voice_registration_state(RIL_Token t)
{
	char *response[15];
	unsigned int i;

	memset(response, 0, sizeof(response));

	asprintf(&response[0], "%d", ril_data.state.reg_state);
	asprintf(&response[1], "%x", ril_data.state.lac_id);
	asprintf(&response[2], "%x", ril_data.state.cell_id);
	asprintf(&response[3], "%d", ril_data.state.act);
	
	if(ril_data.state.reg_state == 3) /* If registration failed */
		asprintf(&response[13], "%d", 0); /* Set "General" reason of failure - can we get real reason? Do we need to? */

	ril_request_complete(t, RIL_E_SUCCESS, response, sizeof(response));
	
	for (i = 0; i < sizeof(response) / sizeof(char *); i++) {
		if (response[i] != NULL)
			free(response[i]);
	}
}

void ril_request_data_registration_state(RIL_Token t)
{
	char *response[6];
	unsigned int i;

	memset(response, 0, sizeof(response));

	if (ril_data.state.act == RADIO_TECH_UNKNOWN)
		asprintf(&response[0], "%d", 0);
	else
		asprintf(&response[0], "%d", ril_data.state.reg_state);
	asprintf(&response[1], "%x", ril_data.state.lac_id);
	asprintf(&response[2], "%x", ril_data.state.cell_id);
	asprintf(&response[3], "%d", ril_data.state.act);
	if(ril_data.state.reg_state == 3) /* If registration failed */
		asprintf(&response[4], "%d", 7); /* Set "GPRS services not allowed" reason of failure - can we get real reason? Do we need to? */
	asprintf(&response[5], "%d", MAX_CONNECTIONS);

	ril_request_complete(t, RIL_E_SUCCESS, response, sizeof(response));

	for (i = 0; i < sizeof(response) / sizeof(char *); i++) {
		if (response[i] != NULL)
			free(response[i]);
	}
}


void ril_request_get_preferred_network_type(RIL_Token t)
{
	int ril_mode;
	
	ril_mode = ipc2ril_net_mode(ril_data.state.net_mode);

	ril_request_complete(t, RIL_E_SUCCESS, &ril_mode, sizeof(ril_mode));	
}

void ril_request_set_preferred_network_type(RIL_Token t, void *data, size_t datalen)
{
	int ril_mode;

	if (data == NULL || datalen < (int) sizeof(int))

		goto error;

	ril_mode = *((int *) data);

	ril_data.state.net_mode = ril2ipc_net_mode(ril_mode);

	if (ril_data.state.radio_state != RADIO_STATE_OFF) {

		tapi_network_set_mode(ril_data.state.net_mode);
		ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);
	}
	else
	{
		ril_request_complete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
	}

	return;
error:
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void ril_request_query_available_networks(RIL_Token t)
{
	tapi_set_selection_mode(1);
	tapi_network_search();
	ril_data.tokens.query_avail_networks = t;
}

void ril_request_set_network_selection_automatic(RIL_Token t)
{
	tapi_network_reselect(1);
	tapi_set_selection_mode(0);
	ril_data.tokens.network_selection = t;
}

void ril_request_set_network_selection_manual(RIL_Token t, void *data, size_t datalen)
{
	struct ril_net_select *net_select;

	if (data == NULL || datalen < (int) sizeof(int))

		goto error;


	net_select = ril_net_select_find_plmn((char *)data);

	if (!net_select) {
		ALOGE("Unable to find network entry, aborting");
		goto error;
	}

	ALOGE("%s: found plmn %s, type %d", __func__, net_select->plmn, net_select->net_select_entry.systemType);

	tapi_network_select(&net_select->net_select_entry);

	ril_data.tokens.network_selection = t;

	return;
error:
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}
