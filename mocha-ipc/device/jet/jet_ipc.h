/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
 *
 * Modified for Jet - KB <kbjetdroid@gmail.com>
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
 
#ifndef _JET_IPC_H_
#define _JET_IPC_H_

#include <radio.h>

#define DPRAM_TTY			"/dev/dpram0"

#define IOCTL_PHONE_ON			0x68d0
#define IOCTL_PHONE_OFF			0x68d1
#define IOCTL_PHONE_GETSTATUS		0x68d2
#define IOCTL_PHONE_RESET		0x68d3
#define IOCTL_PHONE_RAMDUMP		0x68d4
#define IOCTL_PHONE_BOOTTYPE		0x68d5
#define IOCTL_MEM_RW			0x68d6
#define IOCTL_WAKEUP			0x68d7
#define IOCTL_SILENT_RESET		0x68d8

struct multiPacketHeader {
	uint32_t command;
	uint32_t packtLen;
	uint32_t packetType;
};

#endif
