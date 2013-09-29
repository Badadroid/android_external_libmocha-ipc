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

#ifndef __SOUND_H__
#define __SOUND_H__

#include <stdint.h>
#include <stdio.h>

#include "types.h"
#include "util.h"

#include <radio.h>

enum SOUND_PACKET_TYPE {
	SOUND_PACKET_SET_VOLUME = 0,
	SOUND_PACKET_SET_MUTE = 1, /* Generated for soundType 0xD, 0x10, 0x11, 0x12, 0x13*/
	SOUND_PACKET_SET_SND_PATH = 2, /* Generated for soundType 0xD, 0x10, 0x11, 0x12, 0x13*/
	SOUND_PACKET_TESTMODE = 6,
	SOUND_PACKET_DTMF = 7, /* Got some buffer, unknown for now */
	SOUND_PACKET_1MIC_NS_ON = 8, /* No buffer */
	SOUND_PACKET_1MIC_NS_OFF = 9, /* No buffer */
	SOUND_PACKET_CALL_PCM_IF_EN_ON = 0xB, /* No buffer */
	SOUND_PACKET_CALL_PCM_IF_EN_OFF = 0xC, /* No buffer */
	SOUND_PACKET_NS_EC_ON = 0xE, /* Generated for sndType 0x11 and outdevice != 0x10 ADDITIONALLY to SET_MUTE and SET_SNDPATH*/
	SOUND_PACKET_NS_EC_OFF = 0xF, /*  Generated for sndType 0x11 and outdevice == 0x10 ADDITIONALLY to SET_MUTE and SET_SNDPATH, related to bluetooth */
};

typedef struct {
	uint8_t buffer[56]; /*First byte is packet type */
} __attribute__((__packed__)) soundPacket;

typedef struct { /* structure valid for SET_VOLUME, SET_MUTE and SET_SND_PATH */
	uint8_t packetType; /* 0 or 1 or 2 */
	uint8_t align; /* random stuff */
	uint16_t inDevice; /* not set for volume packet */
	uint16_t outDevice;
	uint8_t inDeviceMuted;
	uint8_t outDeviceMuted;
	uint16_t soundType; /* SetMute packet is transmitted only for soundType 0x11 and 0xD */
	uint16_t oemVolume; /* always 6 for mute packet, its valid values are from 1 to 15, this should be checked!*/
} __attribute__((__packed__)) soundChannelSetupPacket;



void ipc_parse_sound(struct ipc_client* client, struct modem_io *ipc_frame);
void sound_send_packet(uint8_t *data, int32_t data_size);
void sound_send_set_volume(uint16_t outDevice, uint8_t inDeviceMuted, uint8_t outDeviceMuted, uint16_t soundType, uint16_t oemVolume);
void sound_send_set_mute(uint16_t inDevice, uint16_t outDevice, uint8_t inDeviceMuted,
							uint8_t outDeviceMuted, uint16_t soundType);
void sound_send_set_path(uint16_t inDevice, uint16_t outDevice, uint8_t inDeviceMuted,
							uint8_t outDeviceMuted, uint16_t soundType, uint16_t oemVolume);
void sound_send_1mic_ns_ctrl(uint8_t enabled);
void sound_send_pcm_if_ctrl(uint8_t enabled);

#endif
