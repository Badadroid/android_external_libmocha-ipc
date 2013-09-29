/**
 * This file is part of mocha-ril.
 *
 * Copyright (C) 2011-2012 Paul Kocialkowski <contact@oaulk.fr>
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

/* Samsung RIL Socket protocol defines */

#ifndef _SAMSUNG_RIL_SOCKET_H_
#define _SAMSUNG_RIL_SOCKET_H_

#define SRS_COMMAND(f)  ((f->group << 8) | f->index)
#define SRS_GROUP(m)    (m >> 8)
#define SRS_INDEX(m)    (m & 0xff)

#define SRS_SOCKET_NAME			"samsung-ril-socket"
#define SRS_DATA_MAX_SIZE		0x1000

#define SRS_CONTROL			0x01
#define SRS_CONTROL_PING		0x0101

#define SRS_SND				0x02
#define SRS_SND_SET_VOLUME		0x0201
#define SRS_SND_SET_AUDIO_PATH	0x0202
#define SRS_SND_1MIC_NS_CTRL	0x0203
#define SRS_SND_PCM_IF_CTRL		0x0204

#define SRS_CONTROL_CAFFE		0xCAFFE

struct srs_header {
	unsigned int length;
	unsigned char group;
	unsigned char index;
} __attribute__((__packed__));

struct srs_message {
	unsigned short command;
	int length;
	void *data;
};

/**
 * Sound types.
 */
typedef enum {
	SND_TYPE_NONE = 0x0,
	SND_TYPE_PCM = 0x5,
	SND_TYPE_CALL_MELODY = 0x7, /* Notification of incoming call */
	SND_TYPE_SYSTEM = 0x8, /* Test sounds, media player sounds, system sounds like clicks etc. */
	SND_TYPE_UNKNOWN = 0xB, /* Set by AIOAudioEngine if it can't be determined */
	SND_TYPE_DTMF = 0xD,
	SND_TYPE_RECORDING = 0x10,
	SND_TYPE_VOICE = 0x11,
	SND_TYPE_VIDEOCALL = 0x12,
	SND_TYPE_FMRADIO = 0x14,
	SND_TYPE_LAST = 0x15 /* No idea what's it */
} SndType;

typedef enum {
	SND_OUTPUT_SPEAKER = 	0x1,
	SND_OUTPUT_EARPIECE = 	0x2,
	SND_OUTPUT_HEADSET = 	0x4,
	SND_OUTPUT_4 = 		0x10,
	SND_OUTPUT_5 = 		0x80,
	SND_OUTPUT_6 = 		0x100,
	SND_OUTPUT_7 = 		0x200,
} SndOutput;

typedef enum {
	SND_INPUT_MAIN_MIC = 	0x100,
	SND_INPUT_EAR_MIC = 	0x200, 
	SND_INPUT_BT_MIC = 	0x800 /* TODO: confirm */
} SndInput;

struct srs_snd_set_volume_packet {
	uint16_t outDevice;
	uint16_t soundType;
	int volume;
} __attribute__((__packed__));

struct srs_snd_set_path_packet {
	uint16_t inDevice;
	uint16_t outDevice;
	uint8_t inDeviceMuted;
	uint8_t outDeviceMuted;
	uint16_t soundType;
} __attribute__((__packed__));

struct srs_snd_enable_disable_packet {
	uint8_t enabled;
} __attribute__((__packed__));

struct srs_control_ping {
	int caffe;
} __attribute__((__packed__));


#endif
