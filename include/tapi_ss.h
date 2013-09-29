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

#ifndef __TAPI_SS_H__
#define __TAPI_SS_H__

#if defined(DEVICE_JET)
#include "device/jet/tapi_ss.h"
#elif defined(DEVICE_WAVE)
#include "device/wave/tapi_ss.h"
#endif

/**
 * All the TAPI SS context structures generic to all Mocha devices will be defined here
 */

struct tapiSS {
};

typedef struct {
	uint32_t bUnknown;
	uint8_t dcs;
	uint8_t align[3];
	uint32_t strLen;
	char ussdStr[0xB8];
} __attribute__((__packed__)) tapiSsSendUssd;

typedef struct {
	uint8_t SsType;
	uint8_t iEncoderType;
	uint8_t dcs;
	uint8_t unknown1;
	uint32_t strLen;
	char ussdStr[0xB8]; 
} __attribute__((__packed__)) tapiSsCallback; 

typedef struct {
	uint8_t rspType;
	uint8_t indType; //must match the type byte from IND or CNF received
	uint8_t align0[2];
	uint32_t requestBySat;
	uint8_t dcs;
	uint8_t align1[3];
	uint32_t strLen;
	char rspString[0xB8]; 
} __attribute__((__packed__)) tapiSsResponse;

void tapi_ss_parser(uint16_t tapiSsType, uint32_t tapiSsLength, uint8_t *tapiSsData);
void tapi_ss_send_ussd_string_request(tapiSsSendUssd* ussd_req);
void tapi_ss_ussd_resp(tapiSsResponse* ussd_req);
void tapi_ss_ussd_callback(uint8_t *response);
void tapi_ss_error(uint8_t *response);

#endif
