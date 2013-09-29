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

#ifndef __BT_PACKET_H__
#define __BT_PACKET_H__

#include <radio.h>

#if defined(DEVICE_WAVE)
#include "device/wave/bt.h"
#endif

typedef struct 
{
	uint16_t type;
	uint16_t length;
} __attribute__((__packed__)) btPacketHeader;

void ipc_parse_bt(struct ipc_client* client, struct modem_io *ipc_frame);
void bt_addr_info(uint8_t *data);

#endif
