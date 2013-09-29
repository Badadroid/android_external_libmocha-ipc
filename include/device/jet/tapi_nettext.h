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
 *
 */

#ifndef __DEVICE_JET_NETTEXT_H__
#define __DEVICE_JET_NETTEXT_H__

/**
 * All the TAPI Nettext (sms) context structures specific to Jet will be defined here
 */
enum TAPI_NETTEXT_TYPE
{
	TAPI_NETTEXT_SETMEMAVAIL 		= 0x3A, //uint8_t bMemAvail = bool probably
	TAPI_NETTEXT_SETPREFERREDMEM 	= 0x3B, //uint8_t memType = sim/phone probably, stored in TapiSmsConfig.ini as variable called "DEF_STORAGE"
	TAPI_NETTEXT_SETNETBURST		= 0x3D, //uint32_t bEnable = 0/1, depending on NvInt id=469 and on some SmsSvc state
	TAPI_NETTEXT_SETCBSETTING 		= 0x3E, //size of buf is 0x64
};

#endif
