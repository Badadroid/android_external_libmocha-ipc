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
 *
 */

#ifndef __TAPI_CALL_H__
#define __TAPI_CALL_H__

#if defined(DEVICE_JET)
#include "device/jet/tapi_call.h"
#elif defined(DEVICE_WAVE)
#include "device/wave/tapi_call.h"
#endif

/**
 * All the TAPI call context structures generic to all Mocha devices will be defined here
 */

#define MAX_CALLS 6

typedef struct {
	uint32_t callId;
	uint8_t callType; 
	uint8_t nameMode;
	char phoneNumber[1];
} __attribute__((__packed__)) tapiCallInfo;

typedef struct {
	uint32_t callId;
	uint32_t cause;
} __attribute__((__packed__)) tapiCallEnd;

typedef struct {
	uint8_t callType;
	uint8_t align[3]; //to align to DWORD, uninitialized
	uint32_t callId;
} __attribute__((__packed__)) tapiCallAnswer;

typedef struct{
	/* Alignments aren't initialized in APPS */
	uint8_t callType;
	uint8_t align0[3];
	uint32_t callId; 
	uint8_t releaseCause;
	uint8_t align1[3];
} __attribute__((__packed__)) tapiCallRelease;

typedef struct {
	uint32_t callId;
	char tone[88];
} __attribute__((__packed__)) tapiDtmf;

typedef struct {
	uint32_t callId;
	uint32_t reason;
} __attribute__((__packed__)) tapiDtmfCnf;

typedef struct {
	uint32_t activeCallId;
	uint32_t holdingCallId;
} __attribute__((__packed__)) tapiSwap;

typedef struct {
	uint32_t activatedCallId;
	uint32_t heldCallId;
	uint32_t cause;
} __attribute__((__packed__)) tapiSwapCnf;

typedef struct {
	uint32_t callId;
	uint32_t error;
} __attribute__((__packed__)) tapiCallError;

typedef struct 
{
	uint32_t acm;
	uint32_t acmMax;
	uint32_t ccm;
} __attribute__((__packed__)) AocInfoStruct;

typedef struct{
	/* TapiCallContext */
	uint8_t contextType; //01
	uint8_t align1[3];
	uint32_t bUsed; //01
	uint32_t hCall; // 01 or 03
	uint32_t hClient; //5C 25 C8 41
	uint32_t callNo; // FF
	uint32_t bOriginated; //01
	uint8_t nameMode; //02
	char callNum1[48];
	uint8_t field_49[35];
	uint32_t startTime;
	uint32_t endTime;
	uint8_t callType1; // 00
	uint8_t callState; //01
	uint8_t activeState;
	uint8_t conferenceState;
	uint8_t currentLine;
	uint8_t align2[3];
	AocInfoStruct aocInfo_acm;
	uint8_t unknown9[64]; //00
	uint8_t unknown10; //01
	uint8_t unknown11[759]; //00
	uint32_t unknown12; //3A
	uint8_t unknown13[284]; //00
	uint8_t unknown14; //01
	uint8_t unknown15[267]; //00
	/*TapiSetupCallInfo */
	uint8_t callType2;
	char callNum2[48];
	uint8_t field_31[38];
	uint8_t szCalledPartySubaddress[32];
	uint8_t gap_77[9];
	uint8_t identityMode;
	uint8_t align3[3];
	uint32_t field_84;
	uint32_t field_88;
	uint16_t field_8C;
	uint16_t align4;
	uint32_t field_90;
	uint32_t field_94;
	uint8_t field_98[397];
	uint8_t field_225;
	uint8_t field_226;
	uint8_t emergencyCategory;
} __attribute__((__packed__)) tapiCallSetup;

void tapi_call_parser(uint16_t tapiCallType, uint32_t tapiCallLength, uint8_t *tapiCallData);
void tapi_call_release(uint8_t callType,uint32_t callId, uint8_t releaseCause);
void tapi_call_answer(uint8_t callType, uint32_t callId);
void tapi_call_setup(tapiCallSetup* callSetup);
void tapi_call_hold(uint32_t callId);
void tapi_call_activate(uint32_t callId);
void tapi_start_dtmf(uint32_t callId, char tone);
void tapi_stop_dtmf(uint32_t callId);
void tapi_calls_swap(uint32_t activeCallId, uint32_t holdingCallId);

#endif
