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
#include <tm.h>

#define LOG_TAG "RIL-Mocha-TM"
#include <utils/Log.h>


void ipc_parse_tm(struct ipc_client* client, struct modem_io *ipc_frame)
{
	DEBUG_I("Entering");

    DEBUG_I("leaving");

}

void tm_send_packet(uint8_t group, uint8_t type, uint8_t *data, int32_t data_size)
{
	struct modem_io pkt;
	pkt.data = malloc(data_size + sizeof(struct tm_tx_packet_header));	
	pkt.data[0] = group;
	pkt.data[1] = type;
	memcpy(pkt.data + 2, data, data_size);
	pkt.magic = 0xCAFECAFE;
	pkt.cmd = FIFO_PKT_TESTMODE;
	pkt.datasize = data_size + 2;
	ipc_send(&pkt);
	free(pkt.data);
}

void ipc_send_rcv_tm()
{
}
