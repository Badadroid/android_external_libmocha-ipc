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

#ifndef __SIM_H__
#define __SIM_H__

#include <radio.h>

#if defined(DEVICE_JET)
#define SIM_SUBTYPE_DIFF 0x18
#elif defined(DEVICE_WAVE)
#define SIM_SUBTYPE_DIFF 0x1D
#endif

#define SIM_SESSION_COUNT 0x20
#define SIM_SESSION_START_ID 0
#define SIM_SESSION_END_ID (SIM_SESSION_START_ID+(SIM_SESSION_COUNT-1))

#define SIM_VALIDATE_SID(hSim) {if(!hSim) {DEBUG_E("SIM_VALIDATE_SID failure!\n"); return;}}

enum SIM_EVENT_TYPE
{
	SIM_EVENT_BEGIN = 0,
	SIM_EVENT_SIM_OPEN = 1,
	SIM_EVENT_VERIFY_PIN1_IND = 2,
	SIM_EVENT_GET_SIM_OPEN_DATA = 3,
	SIM_EVENT_SIM_CLOSE = 4,
	SIM_EVENT_FILE_INFO = 5,
	SIM_EVENT_CHV_INFO = 6,
	SIM_EVENT_READ_FILE = 7,
	SIM_EVENT_UPDATE_FILE = 8,
	SIM_EVENT_OP_CHV_ = 9,
	SIM_EVENT_VERIFY_CHV = 10,
	SIM_EVENT_CHANGE_CHV = 11,
	SIM_EVENT_DISABLE_CHV = 12,
	SIM_EVENT_ENABLE_CHV = 13,
	SIM_EVENT_UNBLOCK_CHV = 14,
	SIM_EVENT_DISABLE_FDN = 15,
	SIM_EVENT_ENABLE_FDN = 16,
	SIM_EVENT_READ_RECORD_FILE_ALL = 17,
	SIM_EVENT_OPEN_CHANNEL = 23,
	SIM_EVENT_CLOSE_CHANNEL = 24,
	SIM_EVENT_END = 25,	
};

enum SIM_EVENT_STATUS
{
	SIM_OK = 0,
	SIM_ERROR = 1,
	SIM_ACCESS_DENIED = 2,
	SIM_FILE_NOT_FOUND = 3,
	SIM_INCOMPATIBLE_PIN_OPERATION = 4,
	SIM_NOT_SUPPORTED = 5,
	SIM_CARD_NOT_PRESENT = 6,
	SIM_CARD_ERROR = 7,
	SIM_INIT_OK = 8,
	SIM_INIT_ERROR = 9,
	SIM_INCORRECT_PARAMS = 10,
	SIM_INSERT_DELAYED = 11,
};

struct simPacketHeader {
	uint32_t type;
	uint32_t subType;
	uint32_t bufLen;
} __attribute__((__packed__));

typedef struct  {
	uint32_t sid;
	uint8_t eventType;
	uint8_t eventStatus;
	uint8_t unused; //does always(appearantly) match subtype
	uint32_t bufLen;
} __attribute__((__packed__)) simEventPacketHeader;

struct simEventPacket {
	simEventPacketHeader* header;
	uint8_t *eventBuf;
} __attribute__((__packed__));

struct simPacket {
	struct simPacketHeader header;
	uint8_t *simBuf;
} __attribute__((__packed__));

struct oemSimPacketHeader{
	uint32_t hSim; //not sure if its really session_id
	uint8_t type; //equal to parent packet subtype
	uint32_t oemBufLen;
} __attribute__((__packed__));

struct oemSimPacket{
	struct oemSimPacketHeader header;
	uint8_t *oemBuf;
} __attribute__((__packed__));

typedef struct
{
	uint32_t 	atkType;
	uint32_t 	atkSubType;
	uint32_t 	atkBufLen;
} __attribute__((__packed__))  sim_atk_packet_header;

typedef struct
{
	uint8_t 	status;
	uint8_t 	attempts;
} __attribute__((__packed__))  lockStatus;

typedef struct
{
	uint16_t 	simDataType;
	uint8_t 	simInd1; //always 0x02
	uint32_t 	unk0; //always 0x00
	uint32_t 	dataCounter;
	uint32_t 	unk1; //always 0x00
	uint32_t 	unk2; //always 0x00
	uint8_t 	simInd2; //always 0x01
	uint16_t 	someType;
	uint32_t 	unk3; //always 0x00
	uint32_t 	unk4; //always 0x00
	uint32_t 	unk5; //always 0x00
	uint32_t 	unk6; //always 0x00
	uint32_t 	unk7; //always 0x00
} __attribute__((__packed__))  sim_data_request;

typedef struct
{
	uint16_t 	simDataType;
	uint32_t 	bufLen;
	//uint8_t		*buf;
} __attribute__((__packed__))  sim_data_response;


void ipc_parse_sim(struct ipc_client* client, struct modem_io *ipc_frame);
void sim_parse_event(uint8_t* buf, uint32_t bufLen);

void sim_send_oem_req(uint8_t* simBuf, uint8_t simBufLen);
void sim_send_oem_data(uint8_t hSim, uint8_t packetType, uint8_t* dataBuf, uint32_t oemBufLen);
void sim_atk_send_packet(uint32_t atkType, uint32_t atkSubType, uint32_t atkBufLen, uint8_t* atkBuf);

void sim_verify_chv(uint8_t hSim, uint8_t pinType, char* pin);
void sim_disable_chv(uint8_t hSim, uint8_t pinType, char* pin);
void sim_enable_chv(uint8_t hSim, uint8_t pinType, char* pin);
void sim_unblock_chv(uint8_t hSim, uint8_t pinType, char* puk, char* pin);
int sim_atk_open(void);
void sim_open_to_modem(uint8_t hSim);
void sim_status(int simCardStatus);
void lock_status(uint8_t *lockStatus);

void sim_get_data_from_modem(uint8_t hSim, sim_data_request *sim_data);
void sim_data_request_to_modem(uint8_t hSim, uint16_t simDataType);
void sim_io_response(uint8_t* buf);

#endif
