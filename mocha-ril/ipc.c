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

#define LOG_TAG "RIL-Mocha-IPC"
#include <utils/Log.h>

#include "mocha-ril.h"
#include <radio.h>

/**
 * IPC shared 
 */

void ipc_log_handler(const char *message, void *user_data)
{
	ALOGD("ipc: %s", message);
}

/**
 * IPC main frame type
 */

void ipc_send(struct modem_io *request)
{
	struct ipc_client *ipc_client;
	if(ril_data.ipc_packet_client == NULL) {
		ALOGE("ipc_packet_client is null, aborting!");
		return;
	}

	if(ril_data.ipc_packet_client->data == NULL) {
		ALOGE("ipc_packet_client data is null, aborting!");
		return;
	}

	ipc_client = ((struct ipc_client_data *) ril_data.ipc_packet_client->data)->ipc_client;

	RIL_CLIENT_LOCK(ril_data.ipc_packet_client);
	ipc_client_send(ipc_client, request);
	RIL_CLIENT_UNLOCK(ril_data.ipc_packet_client);
}

int ipc_modem_io(void *data, uint32_t cmd)
{
	int retval;
	struct ipc_client *ipc_client;
	if(ril_data.ipc_packet_client == NULL) {
		ALOGE("ipc_packet_client is null, aborting!");
		return -1;
	}

	if(ril_data.ipc_packet_client->data == NULL) {
		ALOGE("ipc_packet_client data is null, aborting!");
		return -1;
	}

	ipc_client = ((struct ipc_client_data *) ril_data.ipc_packet_client->data)->ipc_client;

	RIL_CLIENT_LOCK(ril_data.ipc_packet_client);
	retval = ipc_client_modem_operations(ipc_client, data, cmd);
	RIL_CLIENT_UNLOCK(ril_data.ipc_packet_client);

	return retval;
}

int ipc_read_loop(struct ril_client *client)
{
    struct modem_io resp;
	struct ipc_client *ipc_client;
	int ipc_client_fd;
	fd_set fds;

	if(client == NULL) {
		ALOGE("client is NULL, aborting!");
		return -1;
	}

	if(client->data == NULL) {
		ALOGE("client data is NULL, aborting!");
		return -1;
	}

	ipc_client = ((struct ipc_client_data *) client->data)->ipc_client;
	ipc_client_fd = ((struct ipc_client_data *) client->data)->ipc_client_fd;

	FD_ZERO(&fds);
	FD_SET(ipc_client_fd, &fds);

	ALOGI("Starting read loop, fd = %d", ipc_client_fd);

	while(1) {
		if(ipc_client_fd < 0) {
			ALOGE("IPC client fd is negative, aborting!");
			return -1;
		}

		select(FD_SETSIZE, &fds, NULL, NULL, NULL);

		if(FD_ISSET(ipc_client_fd, &fds)) {
			RIL_CLIENT_LOCK(client);
			if(ipc_client_recv(ipc_client, &resp) < 0) {
				RIL_CLIENT_UNLOCK(client);
				ALOGE("IPC recv failed, aborting!");
				return -1;
			}
			RIL_CLIENT_UNLOCK(client);
			
			RIL_LOCK();
			ipc_dispatch(ipc_client, &resp);			
			RIL_UNLOCK();
			
			if(resp.data != NULL)
				free(resp.data);
		}
	}
	ALOGI("Exiting read loop");

	return 0;
}

int ipc_create(struct ril_client *client)
{
	struct ipc_client_data *client_object;
	struct ipc_client *ipc_client;
	int ipc_client_fd;
	int rc;

	client_object = malloc(sizeof(struct ipc_client_data));
	memset(client_object, 0, sizeof(struct ipc_client_data));
	client_object->ipc_client_fd = -1;

	client->data = client_object;

	ipc_client = (struct ipc_client *) client_object->ipc_client;

	ALOGD("Creating new client");
	ipc_client = ipc_client_new();

	if(ipc_client == NULL) {
		ALOGE("Client creation failed!");
		return -1;
	}

	client_object->ipc_client = ipc_client;

	ALOGD("Setting log handler");
	rc = ipc_client_set_log_handler(ipc_client, ipc_log_handler, NULL);

	if(rc < 0) {
		ALOGE("Setting log handler failed!");
		return -1;
	}

	// ipc_client_set_handlers

	ALOGD("Creating handlers common data");
	rc = ipc_client_create_handlers_common_data(ipc_client);

	if(rc < 0) {
		ALOGE("Creating handlers common data failed!");
		return -1;
	}

	ALOGD("Starting modem bootstrap");
	rc = ipc_client_bootstrap_modem(ipc_client);

	if(rc < 0) {
		ALOGE("Modem bootstrap failed!");
		return -1;
	}

	ALOGD("Client open...");
	if(ipc_client_open(ipc_client)) {
		ALOGE("%s: failed to open ipc client", __FUNCTION__);
		return -1;
	}

	ALOGD("Obtaining ipc_client_fd");
	ipc_client_fd = ipc_client_get_handlers_common_data_fd(ipc_client);
	client_object->ipc_client_fd = ipc_client_fd;

	if(ipc_client_fd < 0) {
		ALOGE("%s: client_fd is negative, aborting", __FUNCTION__);
		return -1;
	}


	ALOGD("IPC client done");

	return 0;
}

int ipc_destroy(struct ril_client *client)
{
	struct ipc_client *ipc_client;
	int ipc_client_fd;
	int rc;

	ALOGD("Destroying ipc client");

	if(client == NULL) {
		ALOGE("client was already destroyed");
		return 0;
	}

	if(client->data == NULL) {
		ALOGE("client data was already destroyed");
		return 0;
	}

	ipc_client_fd = ((struct ipc_client_data *) client->data)->ipc_client_fd;

	if(ipc_client_fd)
		close(ipc_client_fd);

	ipc_client = ((struct ipc_client_data *) client->data)->ipc_client;

	if(ipc_client != NULL) {
		ipc_client_destroy_handlers_common_data(ipc_client);
		ipc_client_power_off(ipc_client);
		ipc_client_close(ipc_client);
		ipc_client_free(ipc_client);
	}

	free(client->data);

	return 0;
}


struct ril_client_funcs ipc_client_funcs = {
	.create = ipc_create,
	.destroy = ipc_destroy,
	.read_loop = ipc_read_loop,
};
