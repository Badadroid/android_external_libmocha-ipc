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

#include <pthread.h>

#define LOG_TAG "RIL-Mocha-Client"
#include <utils/Log.h>

#include "mocha-ril.h"

/**
 * RIL client functions
 */

struct ril_client *ril_client_new(struct ril_client_funcs *client_funcs)
{
	struct ril_client *ril_client;
	int rc;

	ril_client = malloc(sizeof(struct ril_client));
	memset(ril_client, 0, sizeof(struct ril_client));

	if(client_funcs->create)
		ril_client->funcs.create = client_funcs->create;

	if(client_funcs->destroy)
		ril_client->funcs.destroy = client_funcs->destroy;

	if(client_funcs->read_loop)
		ril_client->funcs.read_loop = client_funcs->read_loop;

	pthread_mutex_init(&(ril_client->mutex), NULL);

	return ril_client;
}

int ril_client_free(struct ril_client *client)
{
	pthread_mutex_destroy(&(client->mutex));

	free(client);
	client = NULL;

	return 0;
}

int ril_client_create(struct ril_client *client)
{
	int rc;
	int c;

	for(c = 5 ; c > 0 ; c--) {
		ALOGD("Creating RIL client inners, try #%d", 6-c);

		rc = client->funcs.create(client);

		if(rc < 0)
			ALOGD("RIL client inners creation failed!");
		else
			break;

		usleep(500000);
	}

	if(c == 0) {
		client->state = RIL_CLIENT_ERROR;
		return -1;
	}

	client->state = RIL_CLIENT_CREATED;

	return 0;
}

int ril_client_destroy(struct ril_client *client)
{
	int rc;
	int c;

	for(c = 5 ; c > 0 ; c--) {
		ALOGD("Destroying RIL client inners, try #%d", 6-c);

		rc = client->funcs.destroy(client);

		if(rc < 0)
			ALOGD("RIL client inners destroying failed!");
		else
			break;

		usleep(500000);
	}

	if(c == 0) {
		client->state = RIL_CLIENT_ERROR;
		return -1;
	}

	client->state = RIL_CLIENT_DESTROYED;

	return 0;
}

void *ril_client_thread(void *data)
{
	struct ril_client *client;
	int rc;
	int c;

	if(data == NULL) {
		ALOGE("Data passed to thread is NULL!");

		return 0;
	}

	client = (struct ril_client *) data;

	for(c = 5 ; c > 0 ; c--) {
		client->state = RIL_CLIENT_READY;

		rc = client->funcs.read_loop(client);

		if(rc < 0) {
			client->state = RIL_CLIENT_ERROR;

			ALOGE("There was an error with the read loop! Trying to destroy and recreate client object");

			ril_client_destroy(client);
			ril_client_create(client);

			continue;
		} else {
			client->state = RIL_CLIENT_CREATED;

			ALOGD("read loop ended with no error!");
			break;
		}
	}

	if(c == 0) {
		ALOGE("FATAL: Main loop failed too many times.");
	}

	// We are destroying everything here

	rc = ril_client_destroy(client);
	if(rc < 0) {
		ALOGE("RIL client destroy failed!");
	}

	rc = ril_client_free(client);
	if(rc < 0) {
		ALOGE("RIL client free failed!");
	}

	return 0;
}

int ril_client_thread_start(struct ril_client *client)
{
	pthread_attr_t attr;
	int rc;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	
	rc = pthread_create(&(client->thread), &attr, ril_client_thread, (void *) client);

	if(rc != 0) {
		ALOGE("pthread creation failed");
		return -1;
	}

	return 0;
}
