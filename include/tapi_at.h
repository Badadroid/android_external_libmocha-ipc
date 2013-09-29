/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2012 KB <kbjetdroid@gmail.com>
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

#ifndef __AT_H__
#define __AT_H__

#if defined(DEVICE_JET)
#include "device/jet/tapi_at.h"
#elif defined(DEVICE_WAVE)
#include "device/wave/tapi_at.h"
#endif

/**
 * All the TAPI AT context structures generic to all Mocha devices will be defined here
 */

struct tapiAT {
};

void tapi_at_parser(uint16_t tapiAtType, uint32_t tapiAtLength, uint8_t *tapiAtData);

void tapi_at_init(void);
#endif
