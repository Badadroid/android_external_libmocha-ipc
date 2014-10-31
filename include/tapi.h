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

#ifndef __TAPI_PACKET_H__
#define __TAPI_PACKET_H__

#include <radio.h>

#define TAPI_TYPE_CALL 		0 //53 subtypes
#define TAPI_TYPE_NETTEXT 	1 //around 10 subtypes
#define TAPI_TYPE_NETWORK 	2 //23 subtypes
#define TAPI_TYPE_SS 		3 //48 subtypes
#define TAPI_TYPE_AT 		4 //34 subtypes
#define TAPI_TYPE_DMH 		5 //n subtypes, called API_IDs (must be nonzero)
#define TAPI_TYPE_CONFIG 	6 //n subtypes, called API_IDs (must be nonzero)

struct tapiPacketHeader {
	uint16_t tapiService;
	uint16_t tapiServiceFunction;
	uint32_t len;
} __attribute__((__packed__));

struct tapiPacket {
	struct tapiPacketHeader header;
	uint8_t *buf;
} __attribute__((__packed__));

void ipc_parse_tapi(struct ipc_client* client, struct modem_io *ipc_frame);
void tapi_send_packet(struct tapiPacket* tapiReq);
void tapi_init(void);

#endif
