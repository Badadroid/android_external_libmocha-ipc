/**
 * This file is part of mocha-ril.
 *
 * Copyright (C) 2011 Paul Kocialkowski <contact@oaulk.fr>
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

#define LOG_TAG "RIL-Mocha-SND"
#include <utils/Log.h>

#include "mocha-ril.h"
#include "util.h"
#include <sound.h>

void ril_request_set_mute(RIL_Token t, void *data, size_t datalen)
{
	unsigned char mute_data = ((int *)data)[0] > 0 ? 1 : 0;
	ALOGD("%s - mute data is %d\n", __func__, mute_data);
	sound_send_set_mute(ril_data.inDevice, ril_data.outDevice, mute_data, 0, SND_TYPE_VOICE);
	ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);
}

void srs_snd_set_volume(struct srs_message *message)
{
	struct srs_snd_set_volume_packet *volume = (struct srs_snd_set_volume_packet *) message->data;

	ALOGD("%s for: 0x%x type = 0x%x vol = 0x%x\n", __func__, volume->outDevice, volume->soundType, volume->volume);
	sound_send_set_volume(volume->outDevice, 0, 0, volume->soundType, volume->volume);
}

void srs_snd_set_audio_path(struct srs_message *message)
{
	struct srs_snd_set_path_packet *set_path = (struct srs_snd_set_path_packet *) message->data;

	ALOGD("%s - sndType: %d, indev: %d, outdev: %d\n", __func__, set_path->soundType, set_path->inDevice, set_path->outDevice);
	ril_data.inDevice = set_path->inDevice;
	ril_data.outDevice = set_path->outDevice;
	sound_send_set_path(set_path->inDevice, set_path->outDevice, 0, 0, set_path->soundType, 6 /* dummy volume */);
	sound_send_set_mute(set_path->inDevice, set_path->outDevice, 0, 0, set_path->soundType);
}

void srs_snd_1mic_ns_ctrl(struct srs_message *message)
{
	struct srs_snd_enable_disable_packet *en_dis_data = (struct srs_snd_enable_disable_packet *) message->data;
	ALOGD("%s - enabled: %d \n", __func__, en_dis_data->enabled);
	sound_send_1mic_ns_ctrl(en_dis_data->enabled);
}

void srs_snd_pcm_if_ctrl(struct srs_message *message)
{
	struct srs_snd_enable_disable_packet *en_dis_data = (struct srs_snd_enable_disable_packet *) message->data;
	ALOGD("%s - enabled: %d \n", __func__, en_dis_data->enabled);
	sound_send_pcm_if_ctrl(en_dis_data->enabled);
}
