/**
 * This file is part of mocha-ril.
 *
 * Copyright (C) 2014 Nikolay Volkov <volk204@mail.ru>
 *
 * mocha-ril is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mocha-ril is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mocha-ril.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define LOG_TAG "RIL-Mocha-GPS"
#include <time.h>
#include <utils/Log.h>

#include "mocha-ril.h"
#include "util.h"
#include <hardware/gps.h>
#include <lbs.h>

void ipc_lbs_get_position_ind(void* data)
{
	lbsGetPositionInd* get_pos = (lbsGetPositionInd*)(data);
	ALOGD("%s: latitude = %f, longitude = %f", __func__, get_pos->latitude, get_pos->longitude);
	ALOGD("%s: altitude = %f, speed = %f, timestamp = %d", __func__, get_pos->altitude, get_pos->speed, get_pos->timestamp);
	ALOGD("%s: lbsPositionDataType = %d, numOfSatInView = %d, numOfSatToFix = %d", __func__, get_pos->lbsPositionDataType, get_pos->numOfSatInView, get_pos->numOfSatToFix);

	GpsSvStatus status;
	unsigned int i;

	memset(&status, 0, sizeof(GpsSvStatus));
	status.size = sizeof(GpsSvStatus);
	status.num_svs = get_pos->numOfSatInView;

	for (i = 0; i < get_pos->numOfSatInView; i++)
	{
		status.sv_list[i].size = sizeof(GpsSvInfo);
		status.sv_list[i].prn = get_pos->satInfo[i].prn;
		status.sv_list[i].snr = get_pos->satInfo[i].snr;
		status.sv_list[i].elevation = get_pos->satInfo[i].elevation;
		status.sv_list[i].azimuth = get_pos->satInfo[i].azimuth;
	}

	srs_send(SRS_GPS_SV_STATUS, sizeof(GpsSvStatus), &status);

	if (get_pos->lbsPositionDataType == LBS_POSITION_DATA_NEW)
	{
		GpsLocation location;

		memset(&location, 0, sizeof(location));

		location.size = sizeof(GpsLocation);

		location.timestamp = get_pos->timestamp;
		// convert gps time to epoch time ms
		location.timestamp += 315964800; // 1/1/1970 to 1/6/1980
		location.timestamp -= 15; // 15 leap seconds between 1980 and 2011
		location.timestamp *= 1000; //ms

		location.flags |= GPS_LOCATION_HAS_LAT_LONG;
		location.latitude = get_pos->latitude;
		location.longitude = get_pos->longitude;

		srs_send(SRS_GPS_LOCATION, sizeof(GpsLocation), &location);
	}
}

void ipc_lbs_state_ind(void* data)
{
	uint32_t lbs_state = *((uint32_t*)data);
	GpsStatusValue value;

	switch (lbs_state)
	{
		case 1:
			value = GPS_STATUS_SESSION_BEGIN;
			break;
		case 0:
			value = GPS_STATUS_SESSION_END;
			break;
		default:
			ALOGD("%s unknown lbs_state = %d", __func__, lbs_state);
			value = GPS_STATUS_SESSION_END;
			break;
	}

	ALOGD("%s GpsStatusValue = %d", __func__, value);
	//FIXME: Send GpsStatusValue in GPS_HAL to void update_gps_status(GpsStatusValue value)
}

void srs_gps_navigation_mode(struct srs_message *message)
{
	struct srs_snd_enable_disable_packet *data = (struct srs_snd_enable_disable_packet *) message->data;

	if (data->enabled == 1)
	{
		ALOGD("%s GPS navigation enabled", __func__);
		lbsGetPosition get_position;
		get_position.arg = 0x1;
		get_position.gps_mode = 0x1;
		get_position.gps_option = 0x4;
		get_position.unknown1[0] = 0x2;
		get_position.unknown1[1] = 0xff;
		get_position.gps_h_accuracy = 0x03E8;
		get_position.gps_v_accuracy = 0x01F4;
		get_position.device_type = 0x3;
		get_position.unknown2 = 0x1;
		lbs_send_packet(LBS_PKT_GET_POSITION, 0x24, 1, (void*)&get_position);
	}
	else
	{
		ALOGD("%s GPS navigation disabled", __func__);
		lbs_send_packet(LBS_PKT_CANCEL_POSITION, 0, 1, 0);
	}
}
