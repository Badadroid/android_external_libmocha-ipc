/**
 * This file is part of mocha-ril.
 *
 * Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
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

#ifndef _SAMSUNG_RIL_IPC_H_
#define _SAMSUNG_RIL_IPC_H_

#include "mocha-ril.h"

#define ipc_send_get(command, mseq) \
	ipc_send(command, IPC_TYPE_GET, NULL, 0, mseq)

#define ipc_send_set(command, mseq, data, length) \
	ipc_send(command, IPC_TYPE_SET, data, length, mseq)

#define ipc_send_exec(command, mseq) \
	ipc_send(command, IPC_TYPE_EXEC, NULL, 0, mseq)

struct ipc_client_data {
	struct ipc_client *ipc_client;
	int ipc_client_fd;
};

extern struct ril_client_funcs ipc_client_funcs;

void ipc_send(struct modem_io *request);

int ipc_modem_io(void *data, uint32_t cmd);

#endif
