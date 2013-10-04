/**
 * This file is part of mocha-ril.
 *
 * Copyright (C) 2013 Dominik Marszk <dmarszk@gmail.com>
 * Copyright (C) 2011-2013 Paul Kocialkowski <contact@paulk.fr>
 * Copyright (C) 2011 Denis 'GNUtoo' Carikli <GNUtoo@no-log.org>
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

#include <netinet/in.h>
#include <arpa/inet.h>

#define LOG_TAG "RIL-Mocha-GPRS"
#include <utils/Log.h>
#include <cutils/properties.h>

#include "mocha-ril.h"
#include "util.h"
#include <proto.h>

// libnetutils missing prototype
extern int ifc_configure(const char *ifname,
	in_addr_t address,
	in_addr_t netmask,
	in_addr_t gateway,
	in_addr_t dns1,
	in_addr_t dns2);


int ril_gprs_connection_register(int cid)
{
	struct ril_gprs_connection *gprs_connection;
	struct list_head *list_end;
	struct list_head *list;

	gprs_connection = calloc(1, sizeof(struct ril_gprs_connection));
	if (gprs_connection == NULL)
		return -1;

	gprs_connection->cid = cid;

	list_end = ril_data.gprs_connections;
	while (list_end != NULL && list_end->next != NULL)
		list_end = list_end->next;

	list = list_head_alloc((void *) gprs_connection, list_end, NULL);

	if (ril_data.gprs_connections == NULL)
		ril_data.gprs_connections = list;

	return 0;
}

void ril_gprs_connection_unregister(struct ril_gprs_connection *gprs_connection)
{
	struct list_head *list;

	if (gprs_connection == NULL)
		return;

	list = ril_data.gprs_connections;
	while (list != NULL) {
		if (list->data == (void *) gprs_connection) {
			memset(gprs_connection, 0, sizeof(struct ril_gprs_connection));
			free(gprs_connection);

			if (list == ril_data.gprs_connections)
				ril_data.gprs_connections = list->next;

			list_head_free(list);

			break;
		}
list_continue:
		list = list->next;
	}
}

struct ril_gprs_connection *ril_gprs_connection_find_cid(int cid)
{
	struct ril_gprs_connection *gprs_connection;
	struct list_head *list;

	list = ril_data.gprs_connections;
	while (list != NULL) {
		gprs_connection = (struct ril_gprs_connection *) list->data;
		if (gprs_connection == NULL)
			goto list_continue;

		if (gprs_connection->cid == cid)
			return gprs_connection;

list_continue:
		list = list->next;
	}

	return NULL;
}


void ril_request_setup_data_call(RIL_Token t, void *data, int length)
{
	char *username = NULL;
	char *password = NULL;
	char *apn = NULL;
	protoStartNetwork* start_network;



	if (data == NULL || length < (int) (4 * sizeof(char *)))
		goto error;

	apn = ((char **) data)[2];
	username = ((char **) data)[3];
	password = ((char **) data)[4];

	ALOGD("Requesting data connection to APN '%s'\n", apn);

	start_network = (protoStartNetwork *)malloc(sizeof(protoStartNetwork));
	memset(start_network, 0, sizeof(protoStartNetwork));

	start_network->opMode = 1;
	start_network->protoType = 1;

	unsigned int i = 0;

	if (apn != NULL)
	{
		while (i < strlen(apn))
		{
			start_network->napAddr[i] = apn[i];
			i = i + 1;
		}
	}

	start_network->preferredAccountHandle = 0x21;
	start_network->localAddr = 0xFFFFFFFF;
	start_network->dnsAddr1 = 0xFFFFFFFF;
	start_network->dnsAddr2 = 0xFFFFFFFF;
//	start_network->authType = 0x01; //PAP

	if (username != NULL)
	{
		i = 0;
		while (i < strlen(username))
		{
			start_network->userId[i] = username[i];
			i = i + 1;
		}
	}

	if (password != NULL)
	{
		i = 0;
		while (i < strlen(password))
		{
			start_network->userPasswd[i] = password[i];
			i = i + 1;
		}
	}

	proto_start_network(start_network);

	return;
error:
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);

}

void ril_request_deactivate_data_call(RIL_Token t, void *data, int length)
{
	ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);
}

