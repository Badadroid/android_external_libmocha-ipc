/**
 * This file is part of mocha-ril.
 *
 * Copyright (C) 2014 Nikolay Volkov <volk204@mail.ru>
 * Copyright (C) 2014 Dominik Marszk <dmarszk@gmail.com>
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
	ALOGD("%s: heading = %f", __func__, get_pos->heading);
	ALOGD("%s: lbsPositionDataType = %d, numOfSatInView = %d, numOfSatToFix = %d", __func__, get_pos->lbsPositionDataType, get_pos->numOfSatInView, get_pos->numOfSatToFix);
	ALOGD("%s: h_accuracy = %f, v_accuracy = %f", __func__, get_pos->h_accuracy, get_pos->v_accuracy);
	ALOGD("%s: PDOP = %d, HDOP = %d, VDOP = %d", __func__, get_pos->pdop, get_pos->hdop, get_pos->vdop);

	GpsSvStatus status;
	unsigned int i;

	memset(&status, 0, sizeof(GpsSvStatus));
	status.size = sizeof(GpsSvStatus);
	status.num_svs = get_pos->numOfSatInView;

	for (i = 0; i < get_pos->numOfSatInView; i++)
	{
		status.sv_list[i].size = sizeof(GpsSvInfo);
		status.sv_list[i].prn = get_pos->satInfo[i].prn;
		status.sv_list[i].snr = get_pos->satInfo[i].snr / 10;
		status.sv_list[i].elevation = get_pos->satInfo[i].elevation;
		status.sv_list[i].azimuth = get_pos->satInfo[i].azimuth;
	}

	for(i = 0; i < get_pos->numOfSatToFix; i++)
		status.used_in_fix_mask |= (1ul << (get_pos->satIdtoFix[i] - 1));

	srs_send(find_srs_gps_client(), SRS_GPS_SV_STATUS, &status, sizeof(GpsSvStatus));

	if (get_pos->lbsPositionDataType == LBS_POSITION_DATA_NEW)
	{
		GpsLocation location;

		memset(&location, 0, sizeof(location));

		location.size = sizeof(GpsLocation);

		location.timestamp = get_pos->timestamp;
		// convert gps time to epoch time ms
		location.timestamp += 315964800; // 1/1/1970 to 1/6/1980
		location.timestamp *= 1000; //ms

		location.flags |= GPS_LOCATION_HAS_LAT_LONG;
		location.latitude = get_pos->latitude;
		location.longitude = get_pos->longitude;

		location.flags |= GPS_LOCATION_HAS_ALTITUDE;
		location.altitude = get_pos->altitude;

		location.flags |= GPS_LOCATION_HAS_SPEED;
		location.speed = get_pos->speed / 3.6f; // convert kp/h to m/s

		location.flags |= GPS_LOCATION_HAS_BEARING;
		location.bearing = get_pos->heading;

		location.flags |= GPS_LOCATION_HAS_ACCURACY;
		location.accuracy = get_pos->h_accuracy;

		srs_send(find_srs_gps_client(), SRS_GPS_LOCATION, &location, sizeof(GpsLocation));
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

	srs_send(find_srs_gps_client(), SRS_GPS_STATE, &value, sizeof(GpsStatusValue));
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

struct srs_client_info *find_srs_gps_client(void)
{
	struct srs_client_data *client_data;

	if (ril_data.srs_client == NULL || ril_data.srs_client->data == NULL)
		return NULL;

	client_data = (struct srs_client_data *) ril_data.srs_client->data;
	return srs_client_info_find_type(client_data, SRS_CLIENT_TYPE_GPS);
}
