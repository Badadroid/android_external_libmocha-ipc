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
#include <tapi.h>
#include <tapi_call.h>
#include <tapi_nettext.h>
#include <tapi_network.h>
#include <tapi_ss.h>
#include <tapi_at.h>
#include <tapi_dmh.h>
#include <tapi_config.h>

#define LOG_TAG "RIL-Mocha-TAPI-PACKET"
#include <utils/Log.h>

void ipc_parse_tapi(struct ipc_client* client, struct modem_io *ipc_frame)
{
	struct tapiPacketHeader *rx_header;
	struct tapiPacket tx_packet;
	uint8_t resp_buf[8];

	struct modem_io request;
    uint8_t *frame;
    uint8_t *payload;
    uint32_t frame_length;

    rx_header = (struct tapiPacketHeader *)(ipc_frame->data);

    switch (rx_header->tapiService)
    {
	case TAPI_TYPE_CALL:
		DEBUG_I("Tapi call/general packet received");
		tapi_call_parser(rx_header->tapiServiceFunction, rx_header->len, (ipc_frame->data + sizeof(struct tapiPacketHeader)));
		break;
	case TAPI_TYPE_NETTEXT:
		DEBUG_I("Tapi nettext packet received");
		tapi_nettext_parser(rx_header->tapiServiceFunction, rx_header->len, (ipc_frame->data + sizeof(struct tapiPacketHeader)));
		break;
	case TAPI_TYPE_NETWORK:
		DEBUG_I("Tapi network packet received");
		tapi_network_parser(rx_header->tapiServiceFunction, rx_header->len, (ipc_frame->data + sizeof(struct tapiPacketHeader)));
		break;
	case TAPI_TYPE_SS:
		DEBUG_I("Tapi SS packet received");
		tapi_ss_parser(rx_header->tapiServiceFunction, rx_header->len, (ipc_frame->data + sizeof(struct tapiPacketHeader)));
		break;
	case TAPI_TYPE_AT:
		DEBUG_I("Tapi AT packet received");
		tapi_at_parser(rx_header->tapiServiceFunction, rx_header->len, (ipc_frame->data + sizeof(struct tapiPacketHeader)));
		break;
	case TAPI_TYPE_DMH:
		DEBUG_I("Tapi DMH packet received");
		tapi_dmh_parser(rx_header->tapiServiceFunction, rx_header->len, (ipc_frame->data + sizeof(struct tapiPacketHeader)));
		break;
	case TAPI_TYPE_CONFIG:
		DEBUG_I("Tapi Config packet received");
		tapi_config_parser(rx_header->tapiServiceFunction, rx_header->len, (ipc_frame->data + sizeof(struct tapiPacketHeader)));
		break;
    default :
		DEBUG_I("Undefined TAPI Service 0x%x received", rx_header->tapiService);
		break;
    }
	if(rx_header->tapiService || rx_header->tapiServiceFunction)
	{
		*(uint32_t*)(resp_buf) = 0;
		*(uint32_t*)(resp_buf+4) = 1; /* return true */
		tx_packet.buf = resp_buf;
		tx_packet.header.tapiService = 0;
		tx_packet.header.tapiServiceFunction = 0;
		tx_packet.header.len = 8;
		tapi_send_packet(&tx_packet);
	}
}

void tapi_send_packet(struct tapiPacket* tapiReq)
{
	struct modem_io request;
	
	uint32_t bufLen = tapiReq->header.len + sizeof(struct tapiPacketHeader);
	uint8_t* fifobuf = malloc(bufLen);
	memcpy(fifobuf, tapiReq, sizeof(struct tapiPacketHeader));
	if(tapiReq->header.len)
		memcpy(fifobuf + sizeof(struct tapiPacketHeader), tapiReq->buf, tapiReq->header.len);

	request.magic = 0xCAFECAFE;
	request.cmd = FIFO_PKT_TAPI;
	request.datasize = bufLen;

	request.data = fifobuf;

	ipc_send(&request);

	free(fifobuf);
}

void tapi_init(void)
{
	DEBUG_I("enter tapi_init");
	/* TODO: prepare some contexts for Network, SS, NetText, AT here or in these functions? */
	tapi_network_init();
	tapi_nettext_set_mem_available(1);
	tapi_at_init();
	DEBUG_I("exit tapi_init");
}
