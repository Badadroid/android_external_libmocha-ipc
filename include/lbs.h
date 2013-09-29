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

#ifndef __LBS_H__
#define __LBS_H__

#include <types.h>
#include <radio.h>

struct lbsPacketHeader {
	uint32_t type;
	uint32_t size;
	uint32_t subType; //seems to be always 1
} __attribute__((__packed__));

void lbs_init(void);
void lbs_send_init(uint32_t var);
void lbs_send_packet(uint32_t type, uint32_t size, uint32_t subType, void* buf);

#endif
