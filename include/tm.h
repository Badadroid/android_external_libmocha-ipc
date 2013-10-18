/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2012 Dominik Marszk <dmarszk@gmail.com>
 *
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

#ifndef __TM_H__
#define __TM_H__

#include <radio.h>

struct tm_tx_packet_header {
	uint8_t group;
	uint8_t type;
} __attribute__((__packed__));

struct tm_tx_packet {
	struct tm_tx_packet_header;
	uint8_t* buf;
} __attribute__((__packed__));

struct tm_rx_packet_header {
	uint16_t len; //not sure
} __attribute__((__packed__));

struct tm_rx_packet {
	struct tm_rx_packet_header;
	uint8_t* buf;
} __attribute__((__packed__));

void ipc_parse_tm(struct ipc_client* client, struct modem_io *ipc_frame);
void tm_send_packet(uint8_t group, uint8_t type, uint8_t *data, int32_t data_size);
void ipc_send_rcv_tm();
#endif
