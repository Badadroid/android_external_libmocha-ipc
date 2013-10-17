/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2012 Dominik Marszk <dmarszk@gmail.com>
 *                    KB <kbjetdroid@gmail.com>
 *
 * Implemented as per the Mocha AP-CP protocol analysis done by Dominik Marszk
 *
 * libmocha-ipc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libmocha-ipc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libmocha-ipc.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <radio.h>
#include <tapi.h>
#include <tapi_network.h>

#define LOG_TAG "RIL-Mocha-TAPI-NET"
#include <utils/Log.h>

/*
 * All the TAPI Network handling will be done here
 *
 */


void tapi_network_parser(uint16_t tapiNetType, uint32_t tapiNetLength, uint8_t *tapiNetData)
{
	struct tapiPacket tx_packet;
	struct modem_io request;

	uint8_t *frame;
	uint8_t *payload;
	uint32_t frame_length;

	switch(tapiNetType)
	{
		case TAPI_NETWORK_SET_SUBSCRIPTION_MODE:
			tapi_network_set_subscription_mode(tapiNetLength, tapiNetData);
			break;
		case TAPI_NETWORK_SELECT_IND:
			tapi_network_network_select_ind(tapiNetLength, tapiNetData);
			break;
		case TAPI_NETWORK_RADIO_INFO:
			tapi_network_radio_info(tapiNetLength, tapiNetData);
			break;
		case TAPI_NETWORK_COMMON_ERROR:
			tapi_network_common_error(tapiNetLength, tapiNetData);
			break;
		case TAPI_NETWORK_CELL_INFO:
			tapi_network_cell_info(tapiNetLength, tapiNetData);
			break;
		case TAPI_NETWORK_NITZ_INFO_IND:
			tapi_network_nitz_info_ind(tapiNetLength, tapiNetData);
			break;
		case TAPI_NETWORK_SEARCH_CNF:
			ipc_invoke_ril_cb(NETWORK_SEARCH_CNF, (void*)tapiNetData);
			break;
		case TAPI_NETWORK_SELECT_CNF:
			ipc_invoke_ril_cb(NETWORK_SELECT_CNF, (void*)tapiNetData);
			break;
		default:
			DEBUG_I("TapiNetwork packet type 0x%X is not yet handled, len = 0x%x", tapiNetType, tapiNetLength);
			break;
	}
	DEBUG_I("tapi_network_parser");
	hex_dump(tapiNetData, tapiNetLength);
}

void tapi_network_init(void)
{
	struct tapiPacket pkt;
	pkt.header.len = 0;
	pkt.header.tapiService = TAPI_TYPE_NETWORK;	
	pkt.header.tapiServiceFunction = TAPI_NETWORK_INIT;
	pkt.buf = NULL;
	tapi_send_packet(&pkt);
}

void tapi_network_startup(tapiStartupNetworkInfo* network_startup_info)
{
	struct tapiPacket pkt;
	pkt.header.len = sizeof(tapiStartupNetworkInfo);
	pkt.header.tapiService = TAPI_TYPE_NETWORK;	
	pkt.header.tapiServiceFunction = TAPI_NETWORK_STARTUP;	
	pkt.buf = (uint8_t*)(network_startup_info);
	DEBUG_I("tapi_network_startup - AutoSelection:%d,bPoweronGprsAttach:%d,networkMode=0x%X,networkOrder:%d,serviceDomain:%d,subscriptionMode:%d,bFlightMode=%d", network_startup_info->bAutoSelection, network_startup_info->bPoweronGprsAttach, network_startup_info->networkMode, network_startup_info->networkOrder, network_startup_info->serviceDomain, network_startup_info->subscriptionMode, network_startup_info->bFlightMode);
	tapi_send_packet(&pkt);
}

void tapi_network_shutdown(uint8_t mode)
{
	struct tapiPacket pkt;
	pkt.header.len = 0;
	pkt.header.tapiService = TAPI_TYPE_NETWORK;
	pkt.header.tapiServiceFunction = TAPI_NETWORK_SHUTDOWN;
	pkt.buf = NULL;
	tapi_send_packet(&pkt);
}

void tapi_set_offline_mode(uint8_t mode)
{
	struct tapiPacket pkt;
	pkt.header.len = 1;
	pkt.header.tapiService = TAPI_TYPE_NETWORK;	
	pkt.header.tapiServiceFunction = TAPI_NETWORK_SET_OFFLINE_MODE;
	pkt.buf = &mode;
	tapi_send_packet(&pkt);
}


void tapi_network_select(tapiNetSearchCnf* net_select)
{
	struct tapiPacket pkt;
	pkt.header.len = sizeof(tapiNetSearchCnf);
	pkt.header.tapiService = TAPI_TYPE_NETWORK;
	pkt.header.tapiServiceFunction = TAPI_NETWORK_SELECT;
	pkt.buf = (uint8_t*)(net_select);
	hex_dump(pkt.buf , pkt.header.len);
	tapi_send_packet(&pkt);
}

void tapi_network_reselect(uint8_t mode)
{
	struct tapiPacket pkt;
	pkt.header.len = 1;
	pkt.header.tapiService = TAPI_TYPE_NETWORK;
	pkt.header.tapiServiceFunction = TAPI_NETWORK_RESELECT;
	pkt.buf = &mode;
	hex_dump(pkt.buf , pkt.header.len);
	tapi_send_packet(&pkt);
}

void tapi_network_search(void)
{
	struct tapiPacket pkt;
	pkt.header.len = 0;
	pkt.header.tapiService = TAPI_TYPE_NETWORK;
	pkt.header.tapiServiceFunction = TAPI_NETWORK_SEARCH;
	pkt.buf = NULL;
	tapi_send_packet(&pkt);
}

void tapi_set_selection_mode(uint8_t mode)
{
	struct tapiPacket pkt;
	pkt.header.len = 1;
	pkt.header.tapiService = TAPI_TYPE_NETWORK;	
	pkt.header.tapiServiceFunction = TAPI_NETWORK_SET_SELECTION_MODE;
	pkt.buf = &mode;
	hex_dump(pkt.buf , pkt.header.len);
	tapi_send_packet(&pkt);
}

void tapi_network_set_mode(uint32_t mode)
{
	struct tapiPacket pkt;
	pkt.header.len = 4;
	pkt.header.tapiService = TAPI_TYPE_NETWORK;	
	pkt.header.tapiServiceFunction = TAPI_NETWORK_SET_MODE;
	pkt.buf = (uint8_t *)&mode;
	tapi_send_packet(&pkt);
}

void tapi_set_subscription_mode(uint8_t mode)
{
	struct tapiPacket pkt;
	pkt.header.len = 1;
	pkt.header.tapiService = TAPI_TYPE_NETWORK;
	pkt.header.tapiServiceFunction = TAPI_NETWORK_SET_SUBSCRIPTION_MODE;
	pkt.buf = &mode;
	tapi_send_packet(&pkt);
}

void tapi_network_set_subscription_mode(uint32_t tapiNetLength, uint8_t *tapiNetData)
{
	uint8_t subscriptionMode = (uint8_t)tapiNetData[0];	
	DEBUG_I("tapi_network_set_subscription_mode mode:%d\n", subscriptionMode);
	//TODO: bounce-back packet to CP, with the same type, subtype and mode
	ipc_invoke_ril_cb(NETWORK_SET_SUBSCRIPTION_MODE, (void*)tapiNetData);
}

void tapi_network_network_select_ind(uint32_t tapiNetLength, uint8_t *tapiNetData)
{	
	tapiNetworkInfo* netInfo = (tapiNetworkInfo*)(tapiNetData);	
	
	DEBUG_I("tapi_network_network_select_ind: serviceLevel=%d, serviceType=%d, psServiceType=%d, systemId.systemType=%d, sysIdFormat = %d, networkMode = 0x%X, systemId =%d, bForbidden = %d, bHome = %d, bEquivalent=%d, bRoaming=%d, name=%s, spn=%s, registrationFail.state=%d, registrationFail.cause=%d, bDisplayPplmn=%d, bDisplaySpn=%d", netInfo->serviceLevel, netInfo->serviceType, netInfo->psServiceType, netInfo->systemType, netInfo->sysIdFormat, netInfo->networkMode, netInfo->systemId, netInfo->bForbidden, netInfo->bHome, netInfo->bEquivalent, netInfo->bRoaming, netInfo->name, netInfo->spn, netInfo->registrationFail.state, netInfo->registrationFail.cause, netInfo->bDisplayPplmn, netInfo->bDisplaySpn);
	ipc_invoke_ril_cb(NETWORK_SELECT, (void*)netInfo);
}

void tapi_network_radio_info(uint32_t tapiNetLength, uint8_t *tapiNetData)
{
	tapiRadioInfo* radioInfo = (tapiRadioInfo*)(tapiNetData);
	DEBUG_I("tapi_network_radio_info: rxLevel=%d, rxQual=%d", radioInfo->rxLevel, radioInfo->rxQual);
	ipc_invoke_ril_cb(NETWORK_RADIO_INFO, (void*)radioInfo);
}

void tapi_network_common_error(uint32_t tapiNetLength, uint8_t *tapiNetData)
{	
	uint8_t networkOptError = (uint8_t)tapiNetData[0];
	DEBUG_I("networkOptError: %d", networkOptError);
	ipc_invoke_ril_cb(NETWORK_OPT_ERROR, (void*)&networkOptError);
}

void tapi_network_cell_info(uint32_t tapiNetLength, uint8_t *tapiNetData)
{	
	tapiCellInfo* cellInfo = (tapiCellInfo*)(tapiNetData);	
	DEBUG_I("tapi_network_cell_info: cbchStatus:%d, bCellChanged:%d, bRACChanged:%d, bLACChanged:%d, bPLMNChanged:%d", cellInfo->cbchStatus, cellInfo->bCellChanged, cellInfo->bRACChanged, cellInfo->bLACChanged, cellInfo->bPLMNChanged);
	DEBUG_I("tapi_network_cell_info: cellId:%x %x %x %x, racId:%x, ladId:%x %x, plmnId:%x %x %x\n", cellInfo->cellId[0], cellInfo->cellId[1], cellInfo->cellId[2], cellInfo->cellId[3], cellInfo->racId, cellInfo->lacId[0], cellInfo->lacId[1], cellInfo->plmnId[0], cellInfo->plmnId[1], cellInfo->plmnId[2]);
	ipc_invoke_ril_cb(NETWORK_CELL_INFO, (void*)cellInfo);
}

void tapi_network_nitz_info_ind(uint32_t tapiNetLength, uint8_t *tapiNetData)
{
	ipc_invoke_ril_cb(NETWORK_NITZ_INFO_IND, (void*)tapiNetData);
}
