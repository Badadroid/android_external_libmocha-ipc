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
 *
 */
 
#ifndef __DEVICE_JET_DRV_PACKET_H__
#define __DEVICE_JET_DRV_PACKET_H__

//suffix AP/BP shows sender of the packet
enum IpcPacketType {
	READ_NV_BACKUP 		= 0x1,
	WRITE_NV_BACKUP 	= 0x2,
	NV_BACKUP_DATA 		= 0x3,
	SYSTEM_INFO_REQ 	= 0x17,
	SOUND_CONFIG 		= 0x1B,
	PMIC_PACKET 		= 0x1C,
	HIDDEN_SW_VER 		= 0x39,
	
};

#endif