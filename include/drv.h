/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2011-2012 KB <kbjetdroid@gmail.com>
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

#ifndef __DRV_H__
#define __DRV_H__

#if defined(DEVICE_JET)
#include "device/jet/drv.h"
#elif defined(DEVICE_WAVE)
#include "device/wave/drv.h"
#endif

#include <radio.h>

struct drvPacketHeader {
	uint8_t drvPacketType;
} __attribute__((__packed__));

struct drvNvPacket {
	struct drvPacketHeader header;
	uint32_t size;
} __attribute__((__packed__));

struct drvPMICPacket {
	struct drvPacketHeader header;
	uint32_t unk1;
	uint32_t unk2;
	uint32_t value;
} __attribute__((__packed__));

struct drvRequest {
	struct drvPacketHeader header;
	uint8_t *respBuf;
} __attribute__((__packed__));

void ipc_parse_drv(struct ipc_client* client, struct modem_io *ipc_frame);
void drv_send_packet(uint8_t type, uint8_t *data, int32_t data_size);
int32_t get_nvm_data(void *data, uint32_t size);

#endif
