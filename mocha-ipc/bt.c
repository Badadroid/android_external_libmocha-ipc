/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2013 Nikolay Volkov <volk204@mail.ru>
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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <radio.h>
#include <bt.h>

#define LOG_TAG "RIL-Mocha-BT"
#include <utils/Log.h>

#define RIL_BDADDR_PATH "/data/radio/bt.txt"

void ipc_parse_bt(struct ipc_client* client, struct modem_io *ipc_frame)
{
	btPacketHeader *rx_header;

	rx_header = (btPacketHeader *)(ipc_frame->data);

	switch (rx_header->type)
	{
		case BT_PACKET_BT_ADDR_INFO:
		bt_addr_info(ipc_frame->data + sizeof(btPacketHeader));
	    	break;
		default:
		DEBUG_I("BT packet type 0x%X is not yet handled, len = 0x%x", rx_header->type, rx_header->length);
	    	break;
	}
	DEBUG_I("bt_packet_parser");
	hex_dump(ipc_frame->data + sizeof(btPacketHeader), rx_header->length);
}

void bt_addr_info(uint8_t *data)
{
	char bdaddr[18];
	int fd;
	int i = 5;

	memset(bdaddr, 0, sizeof(bdaddr));
	sprintf(bdaddr, "%02x:%02x:%02x:%02x:%02x:%02x", data[5], data[4], data[3], data[2], data[1], data[0]);
	DEBUG_I("%s : BT MAC adress = %s", __func__, bdaddr);	

	fd = open(RIL_BDADDR_PATH, O_WRONLY|O_CREAT|O_TRUNC, 00600|00060|00006);
	if (fd < 0) {
		fprintf(stderr, "open(%s) failed\n", RIL_BDADDR_PATH);
		ALOGE("Can't open %s\n", RIL_BDADDR_PATH);
		return;
	}
	write(fd, bdaddr, 18);
	close(fd);
}