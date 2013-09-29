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

#ifndef __TAPI_NETTEXT_H__
#define __TAPI_NETTEXT_H__

#if defined(DEVICE_JET)
#include "device/jet/tapi_nettext.h"
#elif defined(DEVICE_WAVE)
#include "device/wave/tapi_nettext.h"
#endif

/**
 * All the TAPI Nettext (sms) context structures generic to all Mocha devices will be defined here
 */

struct tapiNetText {
	uint32_t handle;
};

typedef struct {
	uint32_t ext_cb;
	uint32_t ext_cb_enable;
	uint32_t enable_all_combined_cb_channels;
	uint32_t combined_language_type;
	uint32_t number_of_combined_cbmi;
	uint16_t cb_info[40];
} __attribute__((__packed__)) tapi_nettext_cb_settings;

typedef struct {
	uint8_t msgType;
	uint8_t bearer;
	uint8_t unknown0[2];
	/* Here starts 0x8C bytes block memcopied from bada internal struct from offset 0x10 */

	/* Info about sending number */
	uint8_t msgRef; // 00
	uint8_t NPI_FromNumber; /* Numbering Plan Identification */
	uint8_t TON_FromNumber; /* Type Of Number */
	uint8_t lengthFromNumber; /* Length of Number */
	char szFromNumber[21];
	
	/* Info about receiving number */
	uint8_t NPI_ToNumber; /* Numbering Plan Identification */
	uint8_t TON_ToNumber; /* Type Of Number */
	uint8_t lengthToNumber; /* Length of Number */
	char szToNumber [24]; 

	uint32_t scTime; /*  TP-Service-Centre-Time-Stamp (TP-SCTS) */
	uint8_t priority; // 00
	uint8_t NPI_SMSC; /* Numbering Plan Identification */
	uint8_t TON_SMSC; /* Type Of Number */
	uint8_t lengthSMSC; /* Length of Number */
	char SMSC [24];
	uint32_t bReject;
	uint32_t replyPath; /* TP-Reply-Path (TP-RP) */
	uint32_t bUDHI; /*TP-User-Data-Header-Indicator (TP-UDHI) */
	uint32_t bSRR; /* TP-Status-Report-Request (TP-SRR)*/
	uint32_t unknown3; 
	uint32_t dischargeTime; /* used only in SMS-STATUS-REPORT PDU  */ 
	uint32_t statusReport;
	uint8_t validityType; /*TP-Validity-Period-Format (TP-VPF) */
	uint8_t validityRelativeType;
	uint8_t unknown4[2]; /* Padding probably */
	uint32_t validityValue; /* TP-Validy-Period TP-VP) */
	uint32_t bMsgClassSet;
	uint32_t bFlash;
	uint32_t bMsgIndActive;
	uint8_t codingGroupType;
	uint8_t waitingType;
	uint8_t alphabetType; /* TP-Data-Coding-Scheme (TP-DCS) */
	uint8_t classType; //04
	uint32_t time_zone; /* part of TP-SCTS */	
	/* end of 0x8C memcpied block */
	uint32_t hNetTextInfo; /* Value from nettextinfo offset 0x4 */
	uint8_t nUDH;
	uint8_t messageLength;
	uint8_t messageBody[160]; 
} __attribute__((__packed__)) tapiNettextInfo;

typedef struct {
	uint32_t unknown1; 
	uint32_t status;
} __attribute__((__packed__)) tapiNettextCallBack;


void tapi_nettext_parser(uint16_t tapiNettextType, uint32_t tapiNettextLength, uint8_t *tapiNettextData);

void tapi_nettext_set_mem_available(uint32_t bMemAvail);
void tapi_nettext_set_preferred_memory(uint8_t preferredMemory);
void tapi_nettext_set_net_burst(uint32_t bNetBurstEnabled);
void tapi_nettext_set_cb_settings(tapi_nettext_cb_settings* cb_sett_buf);
void tapi_nettext_incoming(uint32_t tapiNettextLength, uint8_t *tapiNettextData);
void tapi_nettext_send(uint8_t* tapiNettextOutgoingMessage);
void tapi_nettext_send_callback(uint8_t *callBack);

#endif
