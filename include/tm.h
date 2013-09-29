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
