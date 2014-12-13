/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2012 KB <kbjetdroid@gmail.com>
 * Copyright (C) 2013 Dominik Marszk <dmarszk@gmail.com>
 * Copyright (C) 2013 Nikolay Volkov <volk204@mail.ru>
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

#include <radio.h>
#include <tapi.h>
#include <tapi_call.h>

#define LOG_TAG "RIL-Mocha-TAPI-CALL"
#include <utils/Log.h>

/*
 * All the TAPI call handling will be done here
 *
 */

void tapi_call_parser(uint16_t tapiCallType, uint32_t tapiCallLength, uint8_t *tapiCallData)
{
	struct tapiPacket tx_packet;

	struct modem_io request;
	uint8_t *frame;
	uint8_t *payload;
	uint32_t frame_length;

	switch(tapiCallType)
	{
		case TAPI_CALL_APIREQ:
			/* Confirmation of properly executed function, just drop it */
			break;
		case TAPI_CALL_INCOMING_IND:
			ipc_invoke_ril_cb(CALL_INCOMING_IND, (void*)tapiCallData);
			break;
		case TAPI_CALL_END_IND:
			ipc_invoke_ril_cb(CALL_END_IND, (void*)tapiCallData);
			break;
		case TAPI_CALL_SETUP_IND:
			ipc_invoke_ril_cb(CALL_SETUP_IND, (void*)tapiCallData);	
			break;
		case TAPI_CALL_ALERT_IND:
			ipc_invoke_ril_cb(CALL_ALERT, (void*)tapiCallData);
			break;
		case TAPI_CALL_CONNECTED_IND:
			ipc_invoke_ril_cb(CALL_CONNECTED, (void*)tapiCallData);	
			break;
		case TAPI_CALL_START_DTMF_CNF:
			ipc_invoke_ril_cb(CALL_DTMF_START, (void*)tapiCallData);
	 		break;
		case TAPI_CALL_STOP_DTMF_CNF:
			ipc_invoke_ril_cb(CALL_DTMF_STOP, (void*)tapiCallData);
			break;
		case TAPI_CALL_HOLD_CNF:
			ipc_invoke_ril_cb(CALL_HOLD, (void*)tapiCallData);
			break;
		case TAPI_CALL_SWAP_CNF:
			ipc_invoke_ril_cb(CALL_SWAP, (void*)tapiCallData);
			break;
		case TAPI_CALL_ACTIVATE_CNF:
			ipc_invoke_ril_cb(CALL_ACTIVATE, (void*)tapiCallData);
			break;
		case TAPI_CALL_ERROR_IND:
			ipc_invoke_ril_cb(CALL_ERROR, (void*)tapiCallData);
			break;
		case TAPI_CALL_CONNECTED_NUMBER_IND:
		case TAPI_CALL_SS_NOTIFY_IND:
		case TAPI_CALL_CONNECTING_IND:
			//unused packets
			break;
		default:	
			DEBUG_I("TapiCall Packet type 0x%X is not yet handled, len = 0x%x", tapiCallType, tapiCallLength);
			hex_dump(tapiCallData, tapiCallLength);
	    	break;
	}

}

void tapi_call_release(uint8_t callType,uint32_t callId, uint8_t releaseCause)
{
	struct tapiPacket tx_packet;
	tapiCallRelease callRelease;
	callRelease.callType = callType;
	callRelease.callId = callId;
	callRelease.releaseCause = releaseCause;
	tx_packet.buf = (uint8_t *)&callRelease;
	tx_packet.header.tapiService = 0;
	tx_packet.header.tapiServiceFunction = TAPI_CALL_RELEASE;
	tx_packet.header.len = sizeof(tapiCallRelease);
	tapi_send_packet(&tx_packet);
}

void tapi_call_answer(uint8_t callType, uint32_t callId)
{
	struct tapiPacket tx_packet;
	tapiCallAnswer callAnswer;
	callAnswer.callType = callType;
	callAnswer.callId = callId; 
	tx_packet.buf = (uint8_t *)&callAnswer;
	tx_packet.header.tapiService = 0;
	tx_packet.header.tapiServiceFunction = TAPI_CALL_ANSWER;
	tx_packet.header.len = 8;
	tapi_send_packet(&tx_packet);
}

void tapi_call_setup(tapiCallSetup* callSetup)
{
	struct tapiPacket tx_packet;
	tx_packet.buf = (uint8_t *)(callSetup);
	tx_packet.header.tapiService = 0;
	tx_packet.header.tapiServiceFunction = TAPI_CALL_SETUP;
	tx_packet.header.len = sizeof(tapiCallSetup);
	tapi_send_packet(&tx_packet);
}

void tapi_call_hold(uint32_t callId)
{
	struct tapiPacket tx_packet;
	tx_packet.buf = (uint8_t *)&callId;
	tx_packet.header.tapiService = 0;
	tx_packet.header.tapiServiceFunction = TAPI_CALL_HOLD;
	tx_packet.header.len = 4;
	tapi_send_packet(&tx_packet);
}

void tapi_call_activate(uint32_t callId)
{
	struct tapiPacket tx_packet;
	tx_packet.buf = (uint8_t *)&callId;
	tx_packet.header.tapiService = 0;
	tx_packet.header.tapiServiceFunction = TAPI_CALL_ACTIVATE;
	tx_packet.header.len = 4;
	tapi_send_packet(&tx_packet);
}

void tapi_start_dtmf(uint32_t callId, char tone)
{
	struct tapiPacket tx_packet;
	tapiDtmf dtmf;

	memset(&dtmf, 0x00, sizeof(tapiDtmf));

	dtmf.callId = callId;
	dtmf.tone[0] = tone; 

	tx_packet.buf = (uint8_t *)&dtmf;
	tx_packet.header.tapiService = 0;
	tx_packet.header.tapiServiceFunction = TAPI_CALL_START_DTMF;
	tx_packet.header.len = 0x5C;
	tapi_send_packet(&tx_packet);
}

void tapi_stop_dtmf(uint32_t callId)
{
	struct tapiPacket tx_packet;
	tapiDtmf dtmf;

	dtmf.callId = callId;

	tx_packet.buf = (uint8_t *)&dtmf;
	tx_packet.header.tapiService = 0;
	tx_packet.header.tapiServiceFunction = TAPI_CALL_STOP_DTMF;
	tx_packet.header.len = 0x5C;
	tapi_send_packet(&tx_packet);
}

void tapi_calls_swap(uint32_t activeCallId, uint32_t holdingCallId)
{
	struct tapiPacket tx_packet;
	tapiSwap swap;

	swap.activeCallId = activeCallId;
	swap.holdingCallId = holdingCallId;

	tx_packet.buf = (uint8_t *)&swap;
	tx_packet.header.tapiService = 0;
	tx_packet.header.tapiServiceFunction = TAPI_CALL_SWAP;
	tx_packet.header.len = sizeof(tapiSwap);
	tapi_send_packet(&tx_packet);
}
