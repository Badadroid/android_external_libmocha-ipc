/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2012 KB <kbjetdroid@gmail.com>
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

#include <radio.h>
#include <tapi.h>
#include <tapi_ss.h>

#define LOG_TAG "RIL-Mocha-TAPI-SS"
#include <utils/Log.h>

/*
 * All the TAPI SS handling will be done here
 *
 */

void tapi_ss_parser(uint16_t tapiSsType, uint32_t tapiSsLength, uint8_t *tapiSsData)
{
	struct tapiPacket tx_packet;

	struct modem_io request;
	uint8_t *frame;
	uint8_t *payload;
	uint32_t frame_length;

	switch(tapiSsType)
	{
	case TAPI_SS_USSD_CNF:
		tapi_ss_ussd_callback(tapiSsData);
	    	break;
	case TAPI_SS_USSD_IND:
		tapi_ss_ussd_callback(tapiSsData);
	    	break;
	case TAPI_SS_COMMON_ERROR_IND:
		tapi_ss_error(tapiSsData);
	    	break;	
	default:
	    	break;
    }
	DEBUG_I("Tapi SS type = 0x%x\n tapiSsLength = 0x%x", tapiSsType, tapiSsLength);
	hex_dump(tapiSsData, tapiSsLength);
}

void tapi_ss_send_ussd_string_request(tapiSsSendUssd* ussd_req)
{
	struct tapiPacket pkt;
	pkt.header.len = sizeof(tapiSsSendUssd);
	pkt.header.tapiService = TAPI_TYPE_SS;	
	pkt.header.tapiServiceFunction = TAPI_SS_SEND_USSD_STRING_REQ;
	pkt.buf = (uint8_t*)ussd_req;
	
	tapi_send_packet(&pkt);

}

void tapi_ss_ussd_resp(tapiSsResponse* ussd_req)
{
	struct tapiPacket pkt;
	pkt.header.len = sizeof(tapiSsResponse);
	pkt.header.tapiService = TAPI_TYPE_SS;	
	pkt.header.tapiServiceFunction = TAPI_SS_USSD_RSP;
	pkt.buf = (uint8_t*)ussd_req;
	
	tapi_send_packet(&pkt);

}

void tapi_ss_ussd_callback(uint8_t *response)
{
	ipc_invoke_ril_cb(SS_USSD_CALLBACK, (void*)response);
}

void tapi_ss_error(uint8_t *response)
{
	ipc_invoke_ril_cb(SS_ERROR, (void*)response);
}

