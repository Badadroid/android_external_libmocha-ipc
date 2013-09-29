/**
 * This file is part of mocha-ril.
 *
 * Copyright (C) 2011-2012 Paul Kocialkowski <contact@paulk.fr>
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

#ifndef _SRS_H_
#define _SRS_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <samsung-ril-socket.h>

#define SRS_CLIENT_LOCK() pthread_mutex_lock(&client_data->mutex)
#define SRS_CLIENT_UNLOCK() pthread_mutex_unlock(&client_data->mutex)

struct srs_client_info {
	int fd;
};

struct srs_client_data {
	struct ril_client *client;

	int server_fd;
	int client_fd;

	struct list_head *clients;

	pthread_t thread;
	pthread_mutex_t mutex;
	int running;
};

extern struct ril_client_funcs srs_client_funcs;

int srs_send(unsigned short command, void *data, int length);
void srs_control_ping(struct srs_message *message);

#endif
