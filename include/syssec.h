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

#ifndef __SYSSEC_H__
#define __SYSSEC_H__

#include <stdint.h>
#include <stdio.h>

#include "types.h"
#include "util.h"

#include <radio.h>

enum sysSecPacketTypes {
	SYS_SEC_SETIMEI = 2,
};

struct sysSecPacketHeader {
	uint32_t type;
	uint32_t unknown1;
	uint32_t bufLen;
	uint32_t unknown2;
} __attribute__((__packed__));

struct sysSecPacket {
	struct sysSecPacketHeader header;
	uint8_t *buf;
} __attribute__((__packed__));

void ipc_parse_syssec(struct ipc_client* client, struct modem_io *ipc_frame);
void syssec_send_imei(void);
#endif
