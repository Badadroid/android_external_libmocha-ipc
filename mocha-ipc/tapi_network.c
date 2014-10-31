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
			DEBUG_I("tapi_network_set_subscription_mode mode:%d\n", (uint8_t)tapiNetData[0]);
			//TODO: bounce-back packet to CP, with the same type, subtype and mode
			ipc_invoke_ril_cb(NETWORK_SET_SUBSCRIPTION_MODE, (void*)tapiNetData);
			break;
		case TAPI_NETWORK_SELECT_IND:
			ipc_invoke_ril_cb(NETWORK_SELECT, (void*)tapiNetData);
			break;
		case TAPI_NETWORK_RADIO_INFO:
			ipc_invoke_ril_cb(NETWORK_RADIO_INFO, (void*)tapiNetData);
			break;
		case TAPI_NETWORK_COMMON_ERROR:
			DEBUG_I("networkOptError: %d", (uint8_t)tapiNetData[0]);
			ipc_invoke_ril_cb(NETWORK_OPT_ERROR, (void*)tapiNetData);
			break;
		case TAPI_NETWORK_CELL_INFO:
			ipc_invoke_ril_cb(NETWORK_CELL_INFO, (void*)tapiNetData);
			break;
		case TAPI_NETWORK_NITZ_INFO_IND:
			ipc_invoke_ril_cb(NETWORK_NITZ_INFO_IND, (void*)tapiNetData);
			break;
		case TAPI_NETWORK_SEARCH_CNF:
			ipc_invoke_ril_cb(NETWORK_SEARCH_CNF, (void*)tapiNetData);
			break;
		case TAPI_NETWORK_SELECT_CNF:
			ipc_invoke_ril_cb(NETWORK_SELECT_CNF, (void*)tapiNetData);
			break;
		default:
			DEBUG_I("TapiNetwork packet type 0x%X is not yet handled, len = 0x%x", tapiNetType, tapiNetLength);
			hex_dump(tapiNetData, tapiNetLength);
			break;
	}
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
