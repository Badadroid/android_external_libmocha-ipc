/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2012 KB <kbjetdroid@gmail.com>
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
 */

#include <stdlib.h>

#include <radio.h>
#include <tapi.h>
#include <tapi_at.h>

#define LOG_TAG "RIL-Mocha-TAPI-AT"
#include <utils/Log.h>

/*
 * All the TAPI AT handling will be done here
 *
 */

void tapi_at_parser(uint16_t tapiAtType, uint32_t tapiAtLength, uint8_t *tapiAtData)
{
	struct tapiPacket tx_packet;

	struct modem_io request;
    uint8_t *frame;
    uint8_t *payload;
    uint32_t frame_length;

    switch(tapiAtType)
    {
    default:
		DEBUG_I("TapiAT Packet type 0x%X is not yet handled, len = 0x%x", tapiAtType, tapiAtLength);
		hex_dump(tapiAtData, tapiAtLength);
    	break;
    }
}

void tapi_at_init(void)
{
	struct tapiPacket pkt;
	pkt.header.len = 0;
	pkt.header.tapiService = TAPI_TYPE_AT;	
	pkt.header.tapiServiceFunction = TAPI_AT_INIT;
	pkt.buf = NULL;
	
	tapi_send_packet(&pkt);
}
