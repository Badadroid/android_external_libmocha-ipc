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
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <getopt.h>

#include <radio.h>
#include <proto.h>

#define LOG_TAG "RIL-Mocha-PRO"
#include <utils/Log.h>

/*
 * TODO: Implement handling of all the Proto packets
 *
 */

void ipc_parse_proto(struct ipc_client* client, struct modem_io *ipc_frame)
{
	struct protoPacketHeader *rx_header;

	rx_header = (struct protoPacketHeader *)(ipc_frame->data);
	switch (rx_header->type)
	{
		case PROTO_PACKET_STARTING_NETWORK_IND:
			DEBUG_I("PROTO_PACKET_STARTING_NETWORK_IND packet received");
			break;
		case PROTO_PACKET_START_NETWORK_CNF:
			DEBUG_I("PROTO_PACKET_START_NETWORK_CNF packet received");
			ipc_invoke_ril_cb(PROTO_START_NETWORK_CNF, (void*)(ipc_frame->data + sizeof(struct protoPacketHeader)));	
			break;
		case PROTO_PACKET_START_NETWORK_IND:
			DEBUG_I("PROTO_PACKET_START_NETWORK_IND packet received");
			break;
		case PROTO_PACKET_STOP_NETWORK_CNF:
			DEBUG_I("PROTO_PACKET_STOP_NETWORK_CNF packet received");
			ipc_invoke_ril_cb(PROTO_STOP_NETWORK_CNF, (void*)(ipc_frame->data + sizeof(struct protoPacketHeader)));
			break;
		case PROTO_PACKET_STOP_NETWORK_IND:
			DEBUG_I("PROTO_PACKET_STOP_NETWORK_IND packet received");
			break;
		case PROTO_PACKET_SUSPEND_NETWORK_IND:
			DEBUG_I("PROTO_PACKET_SUSPEND_NETWORK_IND packet received");
			break;
		case PROTO_PACKET_RESUME_NETWORK_IND:
			DEBUG_I("PROTO_PACKET_RESUME_NETWORK_IND packet received");
			break;
		case PROTO_PACKET_UPDATE_NETWORK_STATUS_IND:
			DEBUG_I("PROTO_PACKET_UPDATE_NETWORK_STATUS_IND packet received");
			break;
		case PROTO_PACKET_RECEIVE_DATA_IND:
			DEBUG_I("PROTO_PACKET_RECEIVE_DATA_IND packet received");
			ipc_invoke_ril_cb(PROTO_RECEIVE_DATA_IND, (void*)(ipc_frame->data + sizeof(struct protoPacketHeader)));
			return;
			break;
		case PROTO_PACKET_DS_NETWORK_IND:
			DEBUG_I("PROTO_PACKET_DS_NETWORK_IND packet received");
			proto_ds_network_resp(ipc_frame->data + sizeof(struct protoPacketHeader));
			break;
		case PROTO_PACKET_RECEIVE_MODEM_SERVICE_IND:
			DEBUG_I("PROTO_PACKET_RECEIVE_MODEM_SERVICE_IND packet received");
			break;
		case PROTO_PACKET_MODEM_RRC_CONNECTION_IND:
			DEBUG_I("PROTO_PACKET_MODEM_RRC_CONNECTION_IND packet received");
			break;
		default :
			DEBUG_I("Proto Packet type = 0x%x is not yet handled, len = 0x%x", rx_header->type, ipc_frame->datasize - sizeof(struct protoPacketHeader));
			break;
		}
	hex_dump(ipc_frame->data + sizeof(struct protoPacketHeader), ipc_frame->datasize - sizeof(struct protoPacketHeader));
}

void proto_send_packet(struct protoPacket* protoReq)
{
	struct modem_io request;
	
	uint32_t bufLen = protoReq->header.len + sizeof(struct protoPacketHeader);
	uint8_t* fifobuf = malloc(bufLen);
	memcpy(fifobuf, protoReq, sizeof(struct protoPacketHeader));
	if(protoReq->header.len)
		memcpy(fifobuf + sizeof(struct protoPacketHeader), protoReq->buf, protoReq->header.len);

	request.magic = 0xCAFECAFE;
	request.cmd = FIFO_PKT_PROTO;
	request.datasize = bufLen;

	request.data = fifobuf;

	ipc_send(&request);

	free(fifobuf);
}

void proto_startup(void)
{
	struct protoPacket pkt;
	pkt.header.type = PROTO_PACKET_STARTUP;
	pkt.header.len = 0;
	pkt.buf = NULL;
	proto_send_packet(&pkt);
}

void proto_start_network(protoStartNetwork* startNetwork)
{
	struct protoPacket pkt;
	pkt.header.type = PROTO_PACKET_START_NETWORK;
	pkt.header.len = sizeof(protoStartNetwork);
	pkt.buf = (uint8_t*)(startNetwork);
	DEBUG_I("proto_start_network: size = %x; APN = %s; UserName = %s; Pass = %s", pkt.header.len, startNetwork->napAddr, startNetwork->userId, startNetwork->userPasswd);
	hex_dump(pkt.buf , pkt.header.len);
	proto_send_packet(&pkt);
}

void proto_stop_network(protoStopNetwork* stopNetwork)
{
	struct protoPacket pkt;
	pkt.header.type = PROTO_PACKET_STOP_NETWORK;
	pkt.header.len = sizeof(protoStopNetwork);
	pkt.buf = (uint8_t*)(stopNetwork);
	hex_dump(pkt.buf , pkt.header.len);
	proto_send_packet(&pkt);
}

void proto_ds_network_resp(uint8_t* buf)
{
	struct protoPacket pkt;
	pkt.header.type = PROTO_PACKET_DS_NETWORK_RESP;
	pkt.header.len = 8;
	pkt.buf = buf;
	DEBUG_I("proto_ds_network_resp");
	hex_dump(buf, 8);
	proto_send_packet(&pkt);
}

void proto_some_unload_function(uint32_t buf)
{
	struct protoPacket pkt;
	pkt.header.type = PROTO_PACKET_SOME_UNLOAD_FUNCTION;
	pkt.header.len = 4;
	pkt.buf = (uint8_t*)&buf;
	hex_dump(&buf, 4);
	proto_send_packet(&pkt);
}

void proto_send_data(uint16_t opMode, uint16_t protoType, uint32_t contextId, uint32_t netBufLen, uint8_t *netBuf)
{
	protoTransferDataBuf* send_hdr;
	struct protoPacket pkt;
	pkt.header.type = PROTO_PACKET_SEND_DATA;
	pkt.header.len = sizeof(protoTransferDataBuf) + netBufLen;
	pkt.buf = malloc(sizeof(protoTransferDataBuf) + netBufLen);
	send_hdr = (protoTransferDataBuf*)pkt.buf;
	send_hdr->opMode = opMode;
	send_hdr->protoType = protoType;
	send_hdr->contextId = contextId;
	send_hdr->netBufLen = netBufLen;
	memcpy(send_hdr->netBuf, netBuf, netBufLen);
	proto_send_packet(&pkt);
	free(pkt.buf);
}
