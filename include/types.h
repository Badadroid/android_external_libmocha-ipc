/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
 *
 * Modified for Jet - KB <kbjetdroid@gmail.com>
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

#ifndef __TYPES_H__
#define __TYPES_H__

#if defined(DEVICE_JET)
#include "device/jet/jet_modem_ctl.h"
#elif defined(DEVICE_WAVE)
#include "device/wave/wave_modem_ctl.h"
#endif

#ifndef LOG_FILE
#define LOG_FILE stderr
#endif

#ifdef RIL_SHLIB
#define DEBUG_E(...) ALOGE(__VA_ARGS__)
#define DEBUG_W(...) ALOGW(__VA_ARGS__)
#define DEBUG_I(...) ALOGI(__VA_ARGS__)
#else

#ifdef DEBUG_ERROR
#define DEBUG_E(x, args ...) fprintf(LOG_FILE, "[%s:%u] ERROR: " \
x, __FUNCTION__ ,__LINE__, ## args)
#define DEBUG_W(x, args ...)
#define DEBUG_I(x, args ...)
#else
#ifdef DEBUG_WARNING
#define DEBUG_E(x, args ...) fprintf(LOG_FILE, "[%s:%u] ERROR: " \
x, __FUNCTION__ ,__LINE__, ## args)
#define DEBUG_W(x, args ...) fprintf(LOG_FILE, "[%s:%u] WARNING: " \
x, __FUNCTION__ ,__LINE__, ## args)
#define DEBUG_I(x, args ...)
#else
#ifdef DEBUG_INFO
#define DEBUG_E(x, args ...) fprintf(LOG_FILE, "[%s:%u] ERROR: " \
x, __FUNCTION__ ,__LINE__, ## args)
#define DEBUG_W(x, args ...) fprintf(LOG_FILE, "[%s:%u] WARNING: " \
x, __FUNCTION__ ,__LINE__, ## args)
#define DEBUG_I(x, args ...) fprintf(LOG_FILE, "[%s:%u] INFO: " \
x, __FUNCTION__ ,__LINE__, ## args)
#else
#define DEBUG_E(x, args ...)
#define DEBUG_W(x, args ...)
#define DEBUG_I(x, args ...)
#endif
#endif
#endif

#endif //RIL_SHLIB

#define MAX_SINGLE_FRAME_DATA 	0xFF4

/**
 * FIFO packet type list
 */
typedef enum
{
	FIFO_PKT_NONE = 0, 		// 0
	FIFO_PKT_KEY,			// 1
	FIFO_PKT_SIM,			// 2
	FIFO_PKT_PROTO,			// 3
	FIFO_PKT_TAPI,			// 4
	FIFO_PKT_PHONESTATUS,	// 5
	FIFO_PKT_FILE,			// 6
	FIFO_PKT_LCD,			// 7
	FIFO_PKT_LED,			// 8
	FIFO_PKT_SOUND,			// 9 Sound means voice here
	FIFO_PKT_SOUND_DATA,	// 10
	FIFO_PKT_H324M,			// 11
	FIFO_PKT_AMR_DATA,		// 12
	FIFO_PKT_AMR_CTRL,		// 13
	FIFO_PKT_CLOCK,			// 14
	FIFO_PKT_BOOT,			// 15
	FIFO_PKT_FLIP,			// 16
	FIFO_PKT_SYSTEM,		// 17
	FIFO_PKT_USBPROTO,		// 18
	FIFO_PKT_USBFILE,		// 19
	FIFO_PKT_USBDIAG,		// 20
	FIFO_PKT_IRDAPROTO,		// 21
	FIFO_PKT_IRDAFILE,		// 22
	FIFO_PKT_IRDADIAG,		// 23
	FIFO_PKT_TIMER,			// 24
	FIFO_PKT_DEBUG,			// 25
	FIFO_PKT_DIAGNOSE,		// 26
	FIFO_PKT_SPECIAL_BOOT,	// 27
	FIFO_PKT_CALL_TIME,		// 28
	FIFO_PKT_ALARM,			// 29
	FIFO_PKT_FIFO_INTERNAL, // 30
	FIFO_PKT_USBCRCPROTO,	// 31
	FIFO_PKT_USBCRCFILE,	// 32
	FIFO_PKT_USBCRCDIAG,	// 33
	FIFO_PKT_VIBRATOR,		// 34
	FIFO_PKT_AMLED,			// 35 AppMgr LED
	FIFO_PKT_AMVIB,			// 36 AppMgr Vibrator
	FIFO_PKT_AMLCD,			// 37 AppMgr LCD Backlight
	FIFO_PKT_DATA_PCSYNC,
	FIFO_PKT_CTRLCMD_PCSYNC,
    FIFO_PKT_DATA_WSSSYNC,
	FIFO_PKT_TIME,			// 41 TimeMgr
	FIFO_PKT_DVB_H_CAS_SIM,	// 42 DVB-H CAS SIM
	FIFO_PKT_DVB_H_CAS_TEST,// 43 DVB-H CAS Test module.
	FIFO_PKT_DVB_H_CAS,		// 44 DVB-H CAS Common usage.
	FIFO_PKT_DVB_H_CAS_IPS, // 45 DVB-H CAS IPS usage.
	FIFO_PKT_DVB_H_DebugLevel, //46 receive debug level from MSM
    FIFO_PKT_Forced_Assert, // 47
	FIFO_PKT_MEMORY,        // 48
	FIFO_PKT_NV,	        // 49		// NvMgrLite
	FIFO_PKT_LBS,			// 50 LBS
    FIFO_PKT_SIM_JSR177,    // 51 S8000_JSR177_kjseo

	FIFO_PKT_USER = 0x80,
	FIFO_PKT_DVBH = FIFO_PKT_USER + 0x06,
	FIFO_PKT_DVBH_SVC,
	FIFO_PKT_DVB_H_LAYER1,

	FIFO_PKT_DVB_PLAYER,
	FIFO_PKT_AV_PLAYER,

	FIFO_PKT_PH,			// BB -> MM : Protocol Handler FIFO Type
	FIFO_PKT_PH_LITE,		// MM -> BB : Protocol Handler Lite FIFO Type

	FIFO_PKT_FX = 0x90,

	FIFO_PKT_BLUETOOTH ,

	FIFO_PKT_TESTMODE,	// Testmode

	FIFO_PKT_DRV,	//  driver
	FIFO_PKT_AGENT,
	FIFO_PKT_DEVMGR,
	FIFO_PKT_SECUREBOOT,

/*
 * Add WAVE specific Packet types here
 */

#if defined(DEVICE_WAVE)
	FIFO_PKT_SYNC = 0x98,
	FIFO_PKT_DATA_SRVC = 0x9B,
	FIFO_PKT_DIAG_SRVC = 0x9D,
#endif

	FIFO_PKT_MAX

} FifoType;


#endif

// vim:ts=4:sw=4:expandtab
