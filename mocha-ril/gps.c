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
#include <lbs.h>

void ipc_lbs_get_position_ind(void* data)
{
	lbsGetPositionInd* get_pos = (lbsGetPositionInd*)(data);
	ALOGD("%s: latitude = %f, longitude = %f", __func__, get_pos->latitude, get_pos->longitude);
	ALOGD("%s: altitude = %f, speed = %f, timestamp = %d", __func__, get_pos->altitude, get_pos->speed, get_pos->timestamp);
	ALOGD("%s: lbsPositionDataType = %d, numOfSatInView = %d, numOfSatToFix = %d", __func__, get_pos->lbsPositionDataType, get_pos->numOfSatInView, get_pos->numOfSatToFix);
	//FIXME: send information in GPS HAL
}

void srs_gps_navigation(struct srs_message *message)
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
