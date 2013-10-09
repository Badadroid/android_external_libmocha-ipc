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

#define MAX_CONNECTIONS 2

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
	PROTO_PACKET_DS_NETWORK_RESP = 0x11,
	PROTO_PACKET_RECEIVE_MODEM_SERVICE_IND = 0x12,
	PROTO_PACKET_SOME_UNLOAD_FUNCTION = 0x13,
	PROTO_PACKET_STOP_RRC_CONNECTION = 0x15,
	PROTO_PACKET_MODEM_RRC_CONNECTION_IND = 0x16,
	PROTO_PACKET_DEACTIVATE_EXT = 0x37	
};

enum PROTO_OPMODES
{
	PROTO_OPMODE_NONE = 0,
	PROTO_OPMODE_PS = 1,
	PROTO_OPMODE_CS = 2,
	PROTO_OPMODE_WLAN = 3,
	PROTO_OPMODE_BT = 4,
	PROTO_OPMODE_NDIS = 5
};

enum PROTO_TYPES
{
	PROTO_TYPE_NONE = 0,
	PROTO_TYPE_IP = 1,
	PROTO_TYPE_PPP = 2,
	PROTO_TYPE_USER = 5,
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

typedef struct {
	uint8_t ipConfigType; //this name is my imagination
	uint8_t align_1[3];
	uint32_t ip;
	uint32_t dns1;
	uint32_t dns2;
	uint32_t subnet;
	uint32_t gatewayIp;
	uint8_t unknown1[110];
	uint32_t bStaticIp;
	uint32_t bValidLocalAddr;
	uint32_t localAddr;
	uint32_t unknown2;
	uint32_t bValidDnsAddr1;
	uint32_t dnsAddr1;
	uint32_t bValidDnsAddr2;
	uint32_t dnsAddr2;
	uint8_t unknown3[165];
} __attribute__((__packed__)) protoStartNetworkInfo;

typedef struct {
	uint16_t opMode;
	uint16_t protoType;
	uint32_t contextId; //id for this connection
	uint16_t error; //errorcode, if non-zero there's been an error
	protoStartNetworkInfo netInfo;
} __attribute__((__packed__)) protoStartNetworkCnf;

typedef struct {
	uint16_t opMode;
	uint16_t protoType;
	uint32_t contextId; //id for this connection
} __attribute__((__packed__)) protoStopNetwork;

typedef struct {
	uint16_t opMode;
	uint16_t protoType;
	uint32_t contextId; //id for this connection
	uint16_t error; //errorcode, if non-zero there's been an error
} __attribute__((__packed__)) protoStopNetworkCnf;

typedef struct {
	uint16_t opMode;
	uint16_t protoType;
	uint32_t contextId;
	uint32_t netBufLen;
	uint8_t netBuf[0];
} __attribute__((__packed__)) protoTransferDataBuf;

void ipc_parse_proto(struct ipc_client* client, struct modem_io *ipc_frame);

void proto_send_packet(struct protoPacket* protoReq);
void proto_startup(void);
void proto_start_network(protoStartNetwork* startNetwork);
void proto_stop_network(protoStopNetwork* stopNetwork);
void proto_ds_network_resp(uint8_t* buf);
void proto_some_unload_function(uint32_t buf);
void proto_send_data(uint16_t opMode, uint16_t protoType, uint32_t contextId, uint32_t netBufLen, uint8_t *netBuf);

#endif
