/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2012 Dominik Marszk <dmarszk@gmail.com>
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

#ifndef __DEVICE_WAVE_NETTEXT_H__
#define __DEVICE_WAVE_NETTEXT_H__

/**
 * All the TAPI Nettext (sms) context structures specific to Wave will be defined here
 */
enum TAPI_NETTEXT_TYPE
{
	TAPI_NETTEXT_SEND				= 0x37, //len=0x138
	TAPI_NETTEXT_DTC				= 0x38, //len=4 uin32_t = 0/1, CONNECT_DTC or DISCONNECT_DTC
	TAPI_NETTEXT_SET_AUTO_DISCONNECT_DTC = 0x39, //len=4 uin32_t = 0
	TAPI_NETTEXT_SET_MEM_AVAIL 		= 0x3A, //len=4 uint32_t bMemAvail = bool probably
	TAPI_NETTEXT_SET_PREFERRED_MEM 	= 0x3B, //uint8_t memType = sim/phone probably, stored in TapiSmsConfig.ini as variable called "DEF_STORAGE"
	TAPI_NETTEXT_SET_BURST			= 0x3D, //uint32_t bEnable = 0/1, depending on NvInt id=469 and on some SmsSvc state
	TAPI_NETTEXT_SET_CB_SETTING 	= 0x3E, //size of buf is 0x64
	TAPI_NETTEXT_DELETE_CB 			= 0x3F,
	TAPI_NETTEXT_SEND_CALLBACK		= 0x40,
	//0x41 UNKNOWN sub_1D23E9A0
	TAPI_NETTEXT_INCOMING			= 0x42,
	TAPI_NETTEXT_CB_INCOMING		= 0x43,
	TAPI_NETTEXT_MEMORY_STATUS_IND	= 0x44,
};

#endif
