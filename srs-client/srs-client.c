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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>

#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cutils/sockets.h>

#include <telephony/ril.h>

#include <samsung-ril-socket.h>
#include <srs-client.h>

/*
 * SRS Client fops
 */

int srs_client_recv_message(struct srs_client *client, struct srs_message *message)
{
	struct srs_header *header_p;
	struct srs_header header;
	void *data = NULL;
	int length = 0;

	struct timeval timeout;
	fd_set fds;
	int rc;

	if (client == NULL || message == NULL || client->fd < 0)
		return -EINVAL;

	memset(message, 0, sizeof(struct srs_message));
	memset(&header, 0, sizeof(header));

	timeout.tv_sec = (SRS_CLIENT_TIMEOUT - SRS_CLIENT_TIMEOUT % 1000000) / 1000000;
	timeout.tv_usec = SRS_CLIENT_TIMEOUT % 1000000;

	FD_ZERO(&fds);
	FD_SET(client->fd, &fds);

	rc = select(client->fd + 1, &fds, NULL, NULL, &timeout);
	if (rc == 0) {
		rc = 0;
		goto done;
	} else if (rc < 0 || !FD_ISSET(client->fd, &fds))
		goto error;

	SRS_CLIENT_LOCK(client);
	rc = read(client->fd, &header, sizeof(header));
	SRS_CLIENT_UNLOCK(client);

	if (rc != sizeof(header))
		goto error;

	header_p = &header;
	message->command = SRS_COMMAND(header_p);

	length = header.length - sizeof(header);
	if (length > 0) {
		data = calloc(1, length);
		if (data == NULL)
			goto error;

		FD_ZERO(&fds);
		FD_SET(client->fd, &fds);

		rc = select(client->fd + 1, &fds, NULL, NULL, &timeout);
		if (rc <= 0 || !FD_ISSET(client->fd, &fds))
			goto error;

		SRS_CLIENT_LOCK(client);
		rc = read(client->fd, data, length);
		SRS_CLIENT_UNLOCK(client);

		if (rc != length)
			goto error;

		message->data = data;
		message->length = length;
	}

	rc = header.length;
	goto done;

error:
	rc = -1;

	if (data != NULL)
		free(data);

done:
	return rc;
}

int srs_client_send_message(struct srs_client *client, struct srs_message *message)
{
	struct srs_header header;
	unsigned char *p = NULL;
	void *data = NULL;
	int length = 0;

	struct timeval timeout;
	fd_set fds;
	int rc;

	if (client == NULL || message == NULL || client->fd < 0)
		return -EINVAL;

	memset(&header, 0, sizeof(header));
	header.length = message->length + sizeof(header);
	header.group = SRS_GROUP(message->command);
	header.index = SRS_INDEX(message->command);

	length = header.length;
	data = calloc(1, length);
	if (data == NULL)
		goto error;

	p = (unsigned char *) data;
	memcpy(p, &header, sizeof(header));
	p += sizeof(header);
	if (message->data != NULL && message->length > 0) {
		memcpy(p, message->data, message->length);
		p += message->length;
	}

	timeout.tv_sec = (SRS_CLIENT_TIMEOUT - SRS_CLIENT_TIMEOUT % 1000000) / 1000000;
	timeout.tv_usec = SRS_CLIENT_TIMEOUT % 1000000;

	FD_ZERO(&fds);
	FD_SET(client->fd, &fds);

	rc = select(client->fd + 1, NULL, &fds, NULL, &timeout);
	if (rc <= 0 || !FD_ISSET(client->fd, &fds))
		goto error;

	SRS_CLIENT_LOCK(client);
	rc = write(client->fd, data, length);
	SRS_CLIENT_UNLOCK(client);

	if (rc != length)
		goto error;

	rc = length;
	goto done;

error:
	rc = -1;

done:
	if (data != NULL)
		free(data);

	return rc;
}

int srs_client_send(struct srs_client *client, unsigned short command, void *data, int length)
{
	struct srs_message message;

	memset(&message, 0, sizeof(message));
	message.command = command;
	message.data = data;
	message.length = length;

	return srs_client_send_message(client, &message);
}

int srs_client_open(struct srs_client *client)
{
	int fd;

	if (client == NULL)
		return -EINVAL;
#if RIL_VERSION >= 6
	fd = socket_local_client(SRS_SOCKET_NAME, ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
#else
	fd = socket_local_client(SRS_SOCKET_NAME, ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
#endif
	if (fd < 0) {
		client->fd = -1;
		return -1;
	}

	client->fd = fd;
	return 0;
}

int srs_client_close(struct srs_client *client)
{
	if (client == NULL || client->fd < 0)
		return -EINVAL;

	close(client->fd);
	client->fd = -1;

	return 0;
}

int srs_client_create(struct srs_client **client_p)
{
	struct srs_client *client;

	if (client_p == NULL)
		return -EINVAL;

	client = calloc(1, sizeof(struct srs_client));
	if (client == NULL) {
		*client_p = NULL;
		return -1;
	}

	client->fd = -1;
	pthread_mutex_init(&(client->mutex), NULL);

	*client_p = client;

	return 0;
}

int srs_client_destroy(struct srs_client *client)
{
	if (client == NULL)
		return -EINVAL;

	pthread_mutex_destroy(&(client->mutex));

	free(client);

	return 0;
}

/*
 * SRS Client thread
 */

void *srs_client_thread(void *data)
{
	struct srs_message message;
	struct srs_client *client;
	int rc;

	if (data == NULL)
		return NULL;

	client = (struct srs_client *) data;

	if (client->thread_cb == NULL)
		goto done;

	while (client->thread_run) {
		rc = srs_client_recv_message(client, &message);
		if (rc < 0)
			goto done;

		client->thread_cb(&message);
	}

done:
	client->thread_run = 0;

	return NULL;
}

int srs_client_thread_start(struct srs_client *client,
	srs_client_thread_cb cb)
{
	pthread_attr_t attr;
	int rc;

	if (client == NULL || cb == NULL)
		return -EINVAL;

	client->thread_cb = cb;
	client->thread_run = 1;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	rc = pthread_create(&(client->thread), &attr, srs_client_thread, (void *) client);
	if (rc != 0)
		return -1;

	return 0;
}

int srs_client_thread_stop(struct srs_client *client)
{
	if (client == NULL)
		return -EINVAL;

	client->thread_run = 0;

	return 0;
}

/*
 * SRS Client inline
 */

int srs_client_ping(struct srs_client *client)
{
	struct srs_message message;
	struct srs_control_ping ping;
	struct srs_control_ping *ping_p;
	int rc;

	if (client == NULL)
		return -1;

	memset(&message, 0, sizeof(message));

	ping.caffe = SRS_CONTROL_CAFFE;
	rc = srs_client_send(client, SRS_CONTROL_PING, &ping, sizeof(ping));
	if (rc < 0)
		goto error;

	rc = srs_client_recv_message(client, &message);
	if (rc < 0 || message.length <= 0 || message.data == NULL)
		goto error;

	ping_p = (struct srs_control_ping *) message.data;
	if (ping_p->caffe != SRS_CONTROL_CAFFE)
		goto error;

	rc = 0;
	goto done;

error:
	rc = -1;

done:
	if (message.data != NULL)
		free(message.data);

	return rc;
}
