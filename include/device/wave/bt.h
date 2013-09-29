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

#ifndef __DEVICE_WAVE_BT_H__
#define __DEVICE_WAVE_BT_H__

/**
 * All the TAPI Network context structures specific to Wave will be defined here
 */

enum BT_PACKET_TYPE
{
	BT_PACKET_API_RESULT 			= 0,
	BT_PACKET_INIT 				= 1,
	BT_PACKET_AG_EXTENDED_AT_RSP 		= 4,
	BT_PACKET_SAP_SIM_OPEN_RSP 		= 0x18, /* Seems unused */
	BT_PACKET_SAP_SEND_APDU_RSP 		= 0x1C,
	BT_PACKET_SAP_SIM_POWER_ON_RSP 		= 0x20,
	BT_PACKET_SAP_SIM_POWER_OFF_RSP 	= 0x22,
	BT_PACKET_SAP_GET_ATR_RSP 		= 0x1E,
	BT_PACKET_SAP_SIM_RESET_RSP 		= 0x24,
	BT_PACKET_SAP_SIM_CARD_READER_STATUS 	= 0x25,
	BT_PACKET_SAP_SIM_CARD_STATUS 		= 0x26,
	BT_PACKET_BT_ADDR_INFO 			= 0x29,
};


#endif
