/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2013 Dominik Marszk <dmarszk@gmail.com>
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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <radio.h>
#include <sound.h>
#include <samsung-ril-socket.h>

#define LOG_TAG "RIL-Mocha-SOUND"
#include <utils/Log.h>

void ipc_parse_sound(struct ipc_client* client, struct modem_io *ipc_frame)
{
	soundPacket *packet;

    packet = (soundPacket*)(ipc_frame->data);
	DEBUG_I("Sound packet type = 0x%x\n  Total packet length = 0x%X", packet->buffer[0], ipc_frame->datasize);
	ipc_hex_dump(client, ipc_frame->data, ipc_frame->datasize);	
}

void sound_send_packet(uint8_t *data, int32_t data_size)
{
	struct modem_io request;
	request.data = data;
	request.magic = 0xCAFECAFE;
	request.cmd = FIFO_PKT_SOUND;
	request.datasize = sizeof(soundPacket);
	ipc_send(&request);
}

void sound_send_set_volume(uint16_t outDevice, uint8_t inDeviceMuted, uint8_t outDeviceMuted, uint16_t soundType, uint16_t oemVolume)
{	
	soundPacket snd_packet;
	soundChannelSetupPacket *set_mute;
	set_mute = (soundChannelSetupPacket *)&snd_packet;

	set_mute->packetType = SOUND_PACKET_SET_VOLUME; 
	/* inDevice is not set for this packet */
	set_mute->outDevice = outDevice;
	set_mute->inDeviceMuted = inDeviceMuted;
	set_mute->outDeviceMuted = outDeviceMuted;
	set_mute->soundType = soundType;
	set_mute->oemVolume = oemVolume;

	sound_send_packet((uint8_t *)&snd_packet, sizeof(soundPacket));
}

void sound_send_set_mute(uint16_t inDevice, uint16_t outDevice, uint8_t inDeviceMuted,
							uint8_t outDeviceMuted, uint16_t soundType)
{
	soundPacket snd_packet;
	soundChannelSetupPacket *set_mute;
	set_mute = (soundChannelSetupPacket *)&snd_packet;

	if(soundType == SND_TYPE_VOICE)
	{
		if(outDevice == SND_OUTPUT_4) //Bluetooth headset perhaps
			snd_packet.buffer[0] = SOUND_PACKET_NS_EC_OFF;
		else
			snd_packet.buffer[0] = SOUND_PACKET_NS_EC_ON;
		sound_send_packet((uint8_t *)&snd_packet, sizeof(soundPacket));
	}

	set_mute->packetType = SOUND_PACKET_SET_MUTE; 
	set_mute->inDevice = inDevice;
	set_mute->outDevice = outDevice;
	set_mute->inDeviceMuted = inDeviceMuted;
	set_mute->outDeviceMuted = outDeviceMuted;
	set_mute->soundType = soundType;
	set_mute->oemVolume = 6; /* always 6 for mute packet */

	sound_send_packet((uint8_t *)&snd_packet, sizeof(soundPacket));
}

void sound_send_set_path(uint16_t inDevice, uint16_t outDevice, uint8_t inDeviceMuted,
							uint8_t outDeviceMuted, uint16_t soundType, uint16_t oemVolume)
{
	soundPacket snd_packet;
	soundChannelSetupPacket *set_mute;
	set_mute = (soundChannelSetupPacket *)&snd_packet;

	if(soundType == SND_TYPE_VOICE)
	{
		if(outDevice == SND_OUTPUT_4) //Bluetooth headset perhaps
			snd_packet.buffer[0] = SOUND_PACKET_NS_EC_OFF;
		else
			snd_packet.buffer[0] = SOUND_PACKET_NS_EC_ON;
		sound_send_packet((uint8_t *)&snd_packet, sizeof(soundPacket));
	}

	set_mute->packetType = SOUND_PACKET_SET_SND_PATH; 
	set_mute->inDevice = inDevice;
	set_mute->outDevice = outDevice;
	set_mute->inDeviceMuted = inDeviceMuted;
	set_mute->outDeviceMuted = outDeviceMuted;
	set_mute->soundType = soundType;
	set_mute->oemVolume = oemVolume;

	sound_send_packet((uint8_t *)&snd_packet, sizeof(soundPacket));
}

void sound_send_1mic_ns_ctrl(uint8_t enabled)
{
	soundPacket snd_packet;
	if(enabled)
		snd_packet.buffer[0] = SOUND_PACKET_1MIC_NS_ON;
	else
		snd_packet.buffer[0] = SOUND_PACKET_1MIC_NS_OFF;

	sound_send_packet((uint8_t *)&snd_packet, sizeof(soundPacket));
}

void sound_send_pcm_if_ctrl(uint8_t enabled)
{
	soundPacket snd_packet;
	if(enabled)
		snd_packet.buffer[0] = SOUND_PACKET_CALL_PCM_IF_EN_ON;
	else
		snd_packet.buffer[0] = SOUND_PACKET_CALL_PCM_IF_EN_OFF;

	sound_send_packet((uint8_t *)&snd_packet, sizeof(soundPacket));
}