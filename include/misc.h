/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2012 Dominik Marszk <dmarszk@gmail.com>
 *
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

#ifndef __MISC_H__
#define __MISC_H__

#include <types.h>
#include <radio.h>

void ipc_send_lpm_mode(int lpmEnabled);
void ipc_power_mode(int mode);
void ipc_parse_boot(struct ipc_client *client, struct modem_io *ipc_frame);
void ipc_parse_dbg_level(struct ipc_client *client, struct modem_io *ipc_frame);
void ipc_parse_system(struct ipc_client *client, struct modem_io *ipc_frame);
void ipc_parse_dbg(struct ipc_client *client, struct modem_io *ipc_frame);

#endif
