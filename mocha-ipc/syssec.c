/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2012 Dominik Marszk <dmarszk@gmail.com>
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

#include <radio.h>
#include <syssec.h>

#define LOG_TAG "RIL-Mocha-SYSSEC"
#include <utils/Log.h>

#include <drv.h>

const uint8_t fake_imei[] = {0x08, 0x1A, 0x32, 0x54, 0x76, 0x98, 0x12, 0x34, 0x56};

void ipc_parse_syssec(struct ipc_client* client, struct modem_io *ipc_frame)
{
	DEBUG_I("Entering");

	struct sysSecPacketHeader *rx_header;

    rx_header = (struct sysSecPacketHeader*)(ipc_frame->data);

	DEBUG_I("Syssec packet type = 0x%x\n  Syssec packet unk1 = 0x%X\n  packet length = 0x%X, unk2= 0x%X", rx_header->type, rx_header->unknown1, rx_header->bufLen, rx_header->unknown2);
	ipc_hex_dump(client, ipc_frame->data, rx_header->bufLen);
	DEBUG_I("Exiting");
}

void load_sec_data()
{
	uint8_t data[150];
	uint8_t real_imei[9]; 	
	int i;
	DEBUG_I("Loading dat stuff.");
	get_nvm_data(data, 150);

	if (data[137] == 0x08)
	{
		DEBUG_I("Real IMEI exist in nv_data");
		for (i = 0; i < 9; i++) {
			real_imei[i] = data[i+137];}
		memcpy(cached_bcd_imei, real_imei, 9);

		
	}else{
		DEBUG_I("We are using fake IMEI");
		memcpy(cached_bcd_imei, fake_imei, 9);
	}
	
	DEBUG_I("Converting IMEI out of dat stuff to ASCII.");
	imei_bcd2ascii(cached_imei, cached_bcd_imei);
}

void syssec_send_imei(void)
{
	uint8_t buffer[40];
	int ret;
	struct sysSecPacketHeader* pkt_hdr;
	struct modem_io request;
	
	if(cached_bcd_imei[0] == 0x00)
		load_sec_data();
	
	memset(buffer, 0, 40);
	
	pkt_hdr = (struct sysSecPacketHeader*) &buffer;

	pkt_hdr->type = SYS_SEC_SETIMEI;
	pkt_hdr->bufLen = 17;
	memcpy(buffer + sizeof(struct sysSecPacketHeader) + 8, cached_bcd_imei, 9);	
	request.magic = 0xCAFECAFE;
	request.cmd = FIFO_PKT_SECUREBOOT;
	request.datasize = pkt_hdr->bufLen + sizeof(struct sysSecPacketHeader); // 17+16=33
	request.data = buffer;
	
	ipc_send(&request);
}
