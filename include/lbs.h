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

enum  LBS_PKT_TYPE
{
	LBS_PKT_INIT				= 0x0,
	LBS_PKT_GET_POSITION			= 0x1,
	LBS_PKT_CANCEL_POSITION			= 0x2,
	LBS_PKT_SET_PARAMETER			= 0x3,
	LBS_PKT_GET_PARAMETER			= 0x4,
	LBS_PKT_DELETE_GPS_DATA			= 0x5,
	LBS_PKT_REQ_RSSI			= 0x6,
	LBS_PKT_SUPL_RESPONSE_MT		= 0x7,
	LBS_PKT_SUPL_RECEIVE_WAPPUSH		= 0x8,
	LBS_PKT_SUPL_SEND_COMPLETE		= 0x9,
	LBS_PKT_SUPL_DATA_RECEIVED		= 0xA,
	LBS_PKT_SUPL_NETWORK_ERROR		= 0xB,
	LBS_PKT_SUPL_CHANGE_NETWORK_STATE	= 0xC,
	LBS_PKT_XTRA_INJECT_TIME_INFO		= 0xD,
	LBS_PKT_XTRA_INITIATE_DOWNLOAD		= 0xE,
	LBS_PKT_XTRA_INJECT_DATA		= 0xF,
	LBS_PKT_XTRA_INJECT_DATA_ERROR		= 0x10,
	LBS_PKT_XTRA_QUERY_DATA_VILIDITY	= 0x11,
	LBS_PKT_XTRA_SET_AUTO_DOWN_INTERVAL	= 0x12,
	LBS_PKT_BASEBAND_GET_LOCATION_ID	= 0x13,
	LBS_PKT_INIT_IND			= 0x15,
	LBS_PKT_GET_POSITION_IND		= 0x16,
	LBS_PKT_CANCEL_POSITION_IND		= 0x17,
	LBS_PKT_SET_PARAMETER_IND		= 0x18,
	LBS_PKT_GET_PARAMETER_IND		= 0x19,
	LBS_PKT_RECEIVED_MT_LR			= 0x1A,
	LBS_PKT_ERROR_IND			= 0x1B,
	LBS_PKT_NMEA_DATA_IND			= 0x1C,
	LBS_PKT_STATE_IND			= 0x1D,
	LBS_PKT_RADIO_INFO_IND			= 0x1E,
	LBS_PKT_SUPL_HASH_IND			= 0x1F,
	LBS_PKT_SUPL_OPEN_CONN_IND		= 0x20,
	LBS_PKT_SUPL_SEND_DATA_IND		= 0x21,
	LBS_PKT_SUPL_RECEIVE_COMPLETE		= 0x22,
	LBS_PKT_SUPL_CLOSE_CONN_IND		= 0x23,
	LBS_PKT_XTRA_OPERATION_IND		= 0x24,
	LBS_PKT_XTRA_QUERY_DATA_VALIDITY_IND	= 0x25,
	LBS_PKT_XTRA_INJECT_TIME_INFO_IND	= 0x26,
	LBS_PKT_XTRA_INJECT_DATA_IND		= 0x27,
};

struct lbsPacketHeader {
	uint32_t type;
	uint32_t size;
	uint32_t subType; //seems to be always 1
} __attribute__((__packed__));

typedef struct {
	uint32_t arg;
	uint32_t gps_mode;
	uint32_t gps_option;
	uint32_t unknown1[2];
	uint32_t gps_h_accuracy;
	uint32_t gps_v_accuracy;
	uint32_t device_type;
	uint32_t unknown2;
} __attribute__((__packed__)) lbsGetPosition;

void lbs_init(void);
void lbs_send_init(uint32_t var);
void lbs_delete_gps_data(void);
void lbs_send_packet(uint32_t type, uint32_t size, uint32_t subType, void* buf);

#endif
