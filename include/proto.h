/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2011 KB <kbjetdroid@gmail.com>
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

#ifndef __PROTO_H__
#define __PROTO_H__

#include <radio.h>

enum PROTO_PACKET_API_ID
{
	PROTO_PACKET_STARTUP = 1,
	PROTO_PACKET_CLEANUP = 2,
	PROTO_PACKET_START_NETWORK = 3,
	PROTO_PACKET_STOP_NETWORK = 4,
	PROTO_PACKET_STARTING_NETWORK_IND = 5,
	PROTO_PACKET_START_NETWORK_CNF = 6,
	PROTO_PACKET_START_NETWORK_IND = 7,
	PROTO_PACKET_STOP_NETWORK_CNF  = 8,
	PROTO_PACKET_STOP_NETWORK_IND  = 9,
	PROTO_PACKET_SUSPEND_NETWORK_IND = 0xA,
	PROTO_PACKET_RESUME_NETWORK_IND = 0xB,
	PROTO_PACKET_UPDATE_NETWORK_STATUS_IND = 0xC,
	PROTO_PACKET_SEND_DATA = 0xD,
	PROTO_PACKET_RECEIVE_DATA_IND = 0xE,
	PROTO_PACKET_UNKNOWN1 = 0xF,
	PROTO_PACKET_DS_NETWORK_IND = 0x10,
	PROTO_PACKET_UNKNOWN2 = 0x11,
	PROTO_PACKET_RECEIVE_MODEM_SERVICE_IND = 0x12,
	PROTO_PACKET_SOME_UNLOAD_FUNCTION = 0x13,
	PROTO_PACKET_STOP_RRC_CONNECTION = 0x15,
	PROTO_PACKET_MODEM_RRC_CONNECTION_IND = 0x16,
	PROTO_PACKET_DEACTIVATE_EXT = 0x37	
};

struct protoPacketHeader {
	uint16_t type;
	uint16_t len;
} __attribute__((__packed__));

struct protoPacket {
	struct protoPacketHeader header;
	uint8_t *buf;
} __attribute__((__packed__));


typedef struct {
	uint16_t opMode;
	uint16_t protoType;
	char napAddr[65];
	char dialNumber[65];
	uint32_t preferredAccountHandle;
	uint32_t bStaticIpAddr; //Or something related to IP
	uint32_t bValidLocalAddr; //Or something similiar, if(bStaticIpAddr == 1) bValidLocalAddr = 1; else bValidLocalAddr = 0;
	uint32_t localAddr; //In host byte order
	uint32_t bStaticDnsAddr;
	uint32_t bValidDnsAddr1; // same comments as for ip address
	uint32_t dnsAddr1;
	uint32_t bValidDnsAddr2;
	uint32_t dnsAddr2;
	uint32_t unknown1;
	uint32_t bHasPreferredAccount; // Or something
	uint32_t serviceOption;
	uint32_t unknown2[4];
	uint32_t trafficClass;
	uint8_t unknown3[42];
	uint8_t authType;
	char userId[45];
	char userPasswd[45];
} __attribute__((__packed__)) protoStartNetwork;

void ipc_parse_proto(struct ipc_client* client, struct modem_io *ipc_frame);

void proto_send_packet(struct protoPacket* protoReq);
void proto_startup(void);
void proto_start_network(protoStartNetwork* startNetwork);
void proto_unknown2(uint8_t* buf);
void proto_some_unload_function(uint32_t buf);

#endif
