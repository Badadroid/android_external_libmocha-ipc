/**
 * This file is part of mocha-ril.
 *
 * Copyright (C) 2013 Paul Kocialkowski <contact@oaulk.fr>
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

#include <pthread.h>

#include <samsung-ril-socket.h>

#ifndef _SRS_CLIENT_H_
#define _SRS_CLIENT_H_

#define SRS_CLIENT_TIMEOUT	500000

#define SRS_CLIENT_LOCK(client) pthread_mutex_lock(&(client->mutex))
#define SRS_CLIENT_UNLOCK(client) pthread_mutex_unlock(&(client->mutex))

typedef void (*srs_client_thread_cb)(struct srs_message *message);

struct srs_client {
	int fd;

	pthread_mutex_t mutex;
	pthread_t thread;
	int thread_run;

	srs_client_thread_cb thread_cb;
};

int srs_client_recv_message(struct srs_client *client, struct srs_message *message);
int srs_client_send_message(struct srs_client *client, struct srs_message *message);
int srs_client_send(struct srs_client *client, unsigned short command, void *data, int length);

int srs_client_open(struct srs_client *client);
int srs_client_close(struct srs_client *client);
int srs_client_create(struct srs_client **client_p);
int srs_client_destroy(struct srs_client *client);

int srs_client_thread_start(struct srs_client *client,
	srs_client_thread_cb cb);
int srs_client_thread_stop(struct srs_client *client);

int srs_client_ping(struct srs_client *client);

#endif
