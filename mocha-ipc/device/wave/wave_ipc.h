/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
 *
 * Modified for Jet & Wave - 	KB <kbjetdroid@gmail.com>
 *								Dominik Marszk <dmarszk@gmail.com>
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
 
#ifndef _WAVE_IPC_H_
#define _WAVE_IPC_H_

#include <radio.h>

#define MODEMCTL_PATH			"/dev/modem_ctl"
#define MODEMPACKET_PATH			"/dev/modem_packet"

struct multiPacketHeader {
	uint32_t command;
	uint32_t packtLen;
	uint32_t packetType;
};


#endif

