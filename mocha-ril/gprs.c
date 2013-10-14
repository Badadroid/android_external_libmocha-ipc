/**
 * This file is part of mocha-ril.
 *
 * Copyright (C) 2013 Nikolay Volkov <volk204@mail.ru>
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

#include <pthread.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <errno.h>

#define LOG_TAG "RIL-Mocha-GPRS"
#include <utils/Log.h>
#include <cutils/properties.h>
#include <netutils/ifc.h>

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

struct in_addr htoina(uint32_t addr)
{
	struct in_addr ret;
	ret.s_addr = htonl(addr);
	return ret;
}

char* proto_type_to_data_call_type(uint8_t type)
{
	switch(type)
	{
		case PROTO_TYPE_IP:
			return "IP";
		case PROTO_TYPE_PPP:
			return "PPP";
		default:
			return "IP";
	}
}

uint8_t data_call_type_to_proto_type(char* type)
{
	if(!strcmp(type, "IP") || !strcmp(type, "IPV6") || !strcmp(type, "IPV4V6") )
		return PROTO_TYPE_IP;
	else if(!strcmp(type, "PPP"))
		return PROTO_TYPE_PPP;
	else
		return PROTO_TYPE_NONE;
}

void *gprs_tunneling_thread(void *data)
{
	int n;
	uint8_t buf[1500]; //MTU is 1500
	ril_gprs_connection *gprs_connection = (ril_gprs_connection *)data;
    fd_set fds;
	struct timeval select_timeout;

	ALOGD("%s: Thread initialized for connection cid %d, contextId %d on %s", __func__, 
		gprs_connection->cid, gprs_connection->contextId, gprs_connection->ifname);
	while(1)
	{		
		pthread_mutex_lock(&gprs_connection->mutex);
		FD_ZERO(&fds);
		FD_SET(gprs_connection->iface, &fds);
		select_timeout.tv_sec = 0;
		select_timeout.tv_usec = 300000; //300ms select timeout
		select(gprs_connection->iface+1, &fds, NULL, NULL, &select_timeout);
		if(gprs_connection->thread_state == 2)
		{			
			pthread_mutex_unlock(&gprs_connection->mutex);
			pthread_exit(NULL);
			return NULL;
		}
		if(FD_ISSET(gprs_connection->iface, &fds)) {
			n = read(gprs_connection->iface, buf, 1500);
			RIL_LOCK();
			ALOGD("%s: Tunneling %d bytes of the net frame from %s to CP", __func__, n, gprs_connection->ifname);
			proto_send_data(PROTO_OPMODE_PS, gprs_connection->type, gprs_connection->contextId, n, buf);
			RIL_UNLOCK();
		}		
		pthread_mutex_unlock(&gprs_connection->mutex);
	}
}

int gprs_start_tunneling_thread(struct ril_gprs_connection *gprs_connection)
{
	pthread_attr_t attr;
	int rd;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	rd = pthread_create(&gprs_connection->thread, &attr, gprs_tunneling_thread, (void *) gprs_connection);
	if(rd == 0)
	{
		gprs_connection->thread_state = 1;
	}
	return rd;
}

int gprs_stop_tunneling_thread(struct ril_gprs_connection *gprs_connection)
{
	if(gprs_connection->thread_state == 1)
	{
		gprs_connection->thread_state = 2;
		pthread_mutex_lock(&gprs_connection->mutex);
		pthread_mutex_unlock(&gprs_connection->mutex);
		return 0;
	}
	return -1;
}

int ril_gprs_connection_register(int cid)
{
	struct ril_gprs_connection *gprs_connection;
	struct list_head *list_end;
	struct list_head *list;

	gprs_connection = calloc(1, sizeof(struct ril_gprs_connection));
	if (gprs_connection == NULL)
		return -1;

	gprs_connection->cid = cid;
	gprs_connection->iface = -1;
	pthread_mutex_init(&gprs_connection->mutex, NULL);

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

	gprs_stop_tunneling_thread(gprs_connection);
	pthread_mutex_destroy(&gprs_connection->mutex);
	memset(gprs_connection, 0, sizeof(struct ril_gprs_connection));
	free(gprs_connection);
	
	list = ril_data.gprs_connections;
	while (list != NULL) {
		if (list->data == (void *) gprs_connection) {
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

struct ril_gprs_connection *ril_gprs_connection_find_contextId(uint32_t contextId)
{
	struct ril_gprs_connection *gprs_connection;
	struct list_head *list;

	list = ril_data.gprs_connections;
	while (list != NULL) {
		gprs_connection = (struct ril_gprs_connection *) list->data;
		if (gprs_connection == NULL)
			goto list_continue;

		if (gprs_connection->contextId == contextId)
			return gprs_connection;

list_continue:
		list = list->next;
	}

	return NULL;
}

struct ril_gprs_connection *ril_gprs_connection_start(void)
{
	struct ril_gprs_connection *gprs_connection;
	struct list_head *list;
	int cid;
	int rc;
	int i;

	for (i = 0 ; i < MAX_CONNECTIONS ; i++) {
		cid = i + 1;
		list = ril_data.gprs_connections;
		while (list != NULL) {
			if (list->data == NULL)
				goto list_continue;
			gprs_connection = (struct ril_gprs_connection *) list->data;
			if (gprs_connection->cid == cid) {
				cid = 0;
				break;
			}
list_continue:
			list = list->next;
		}
		if (cid > 0)
			break;
	}

	if (cid <= 0) {
		ALOGE("Unable to find an unused cid, aborting");
		return NULL;
	}

	ALOGD("Using GPRS connection cid: %d", cid);
	rc = ril_gprs_connection_register(cid);
	if (rc < 0)
		return NULL;

	gprs_connection = ril_gprs_connection_find_cid(cid);
	asprintf(&gprs_connection->ifname, "tun%d", cid - 1);
	gprs_connection->iface = tun_alloc(gprs_connection->ifname, IFF_TUN | IFF_NO_PI);
	if(gprs_connection->iface < 0)
	{
		ALOGE("Couldn't create interface %s, errno: %d", gprs_connection->ifname, errno);
		if (gprs_connection->ifname != NULL)
			free(gprs_connection->ifname);
		ril_gprs_connection_unregister(gprs_connection);
		return NULL;
	}
	
	return gprs_connection;
}

void ril_gprs_connection_stop(struct ril_gprs_connection *gprs_connection)
{
	ALOGD("Destroying GPRS connection with cid: %d", gprs_connection->cid);
	if (gprs_connection == NULL)
		return;

	if (gprs_connection->iface >= 0)
		close(gprs_connection->iface);
	if (gprs_connection->ifname != NULL)
		free(gprs_connection->ifname);

	ril_gprs_connection_unregister(gprs_connection);
}

#define IN_ADDR_FMT(ip) ((uint8_t*)&ip.s_addr)[0], ((uint8_t*)&ip.s_addr)[1], ((uint8_t*)&ip.s_addr)[2], ((uint8_t*)&ip.s_addr)[3] 

void ipc_proto_start_network_cnf(void* data)
{
	ALOGE("%s: Test me!", __func__);

	struct ril_gprs_connection *gprs_connection;
	protoStartNetworkCnf* netCnf = (protoStartNetworkCnf*)(data);
	RIL_Data_Call_Response_v6 *setup_data_call_response;

	gprs_connection = ril_gprs_connection_find_contextId(0xFFFFFFFF);

	if (!gprs_connection) {
		ALOGE("Unable to find GPRS connection, aborting");
		return;
	}
	gprs_connection->contextId = netCnf->contextId;

	if (netCnf->error != 0)
	{
		//FIXME: add conversion for error
		ALOGE("There was an error, aborting port list complete");
		gprs_connection->fail_cause = PDP_FAIL_ERROR_UNSPECIFIED;
		ril_data.state.gprs_last_failed_cid = gprs_connection->cid;
		ril_request_complete(gprs_connection->token, RIL_E_GENERIC_FAILURE, NULL, 0);
		gprs_connection->token = 0;
		ril_gprs_connection_stop(gprs_connection); /* We can't rely on RILJ calling last_fail_cause */
		return;
	}
	gprs_connection->ip = htoina(netCnf->netInfo.localAddr);
	gprs_connection->gateway = htoina(netCnf->netInfo.localAddr);	
	gprs_connection->dns1 = htoina(netCnf->netInfo.dnsAddr1);	
	gprs_connection->dns2 = htoina(netCnf->netInfo.dnsAddr2);
	// FIXME: subnet isn't reliable!
	gprs_connection->prefix_len = 32;

	if(gprs_start_tunneling_thread(gprs_connection) != 0)
	{
		//TODO: Close proto on CP side
		ALOGE("Couldn't start the tunneling thread");
		gprs_connection->fail_cause = PDP_FAIL_ERROR_UNSPECIFIED;
		ril_data.state.gprs_last_failed_cid = gprs_connection->cid;
		ril_request_complete(gprs_connection->token, RIL_E_GENERIC_FAILURE, NULL, 0);
		gprs_connection->token = 0;
		ril_gprs_connection_stop(gprs_connection); /* We can't rely on RILJ calling last_fail_cause */
		return;
	}

	if(ifc_configure(gprs_connection->ifname, gprs_connection->ip.s_addr,
		gprs_connection->prefix_len, gprs_connection->gateway.s_addr, 
		gprs_connection->dns1.s_addr, gprs_connection->dns2.s_addr) < 0)
	{
		//TODO: Close proto on CP side
		ALOGE("Couldn't ifc_configure on %s, errno: %d (check system logcat)", gprs_connection->ifname, errno);
		gprs_connection->fail_cause = PDP_FAIL_ERROR_UNSPECIFIED;
		ril_data.state.gprs_last_failed_cid = gprs_connection->cid;
		ril_request_complete(gprs_connection->token, RIL_E_GENERIC_FAILURE, NULL, 0);
		gprs_connection->token = 0;
		ril_gprs_connection_stop(gprs_connection); /* We can't rely on RILJ calling last_fail_cause */
		return;
	}
	
	setup_data_call_response = calloc(1, sizeof(RIL_Data_Call_Response_v6));
	asprintf(&setup_data_call_response->addresses, "%d.%d.%d.%d/%d",
		IN_ADDR_FMT(gprs_connection->ip),
		gprs_connection->prefix_len);
	asprintf(&setup_data_call_response->dnses, "%d.%d.%d.%d %d.%d.%d.%d",
		IN_ADDR_FMT(gprs_connection->dns1), IN_ADDR_FMT(gprs_connection->dns2));
	asprintf(&setup_data_call_response->gateways, "%d.%d.%d.%d",
		IN_ADDR_FMT(gprs_connection->gateway));
	/*asprintf(&setup_data_call_response->gateways, "%d.%d.%d.%d", 
		((netCnf->netInfo.gatewayIp >> 24) & 0xFF), ((netCnf->netInfo.gatewayIp >> 16) & 0xFF), 
		((netCnf->netInfo.gatewayIp >> 8) & 0xFF), ((netCnf->netInfo.gatewayIp >> 0) & 0xFF));	
	*/

	setup_data_call_response->status = PDP_FAIL_NONE;
	setup_data_call_response->cid = gprs_connection->cid;
	setup_data_call_response->ifname = gprs_connection->ifname;
	setup_data_call_response->active = 2;
	setup_data_call_response->type = proto_type_to_data_call_type(gprs_connection->type);
	
	ALOGD("GPRS configuration: cid: %d, type: %s, iface: %s, ip: %s, gateway: %s, dnses: %s", 
		setup_data_call_response->cid, setup_data_call_response->type, 
		setup_data_call_response->ifname, setup_data_call_response->addresses, 
		setup_data_call_response->gateways, setup_data_call_response->dnses);



	ril_request_complete(gprs_connection->token, RIL_E_SUCCESS, setup_data_call_response, sizeof(RIL_Data_Call_Response_v6));

	ril_data.data_call_count++;
	ril_unsol_data_call_list_changed(0);
	
	if(setup_data_call_response->addresses)
		free(setup_data_call_response->addresses);
	if(setup_data_call_response->dnses)
		free(setup_data_call_response->dnses);
	if(setup_data_call_response->gateways)
		free(setup_data_call_response->gateways);
	free(setup_data_call_response);
}

void ipc_proto_stop_network_cnf(void* data)
{
	ALOGE("%s: Test me!", __func__);

	struct ril_gprs_connection *gprs_connection;
	protoStopNetworkCnf* netCnf = (protoStopNetworkCnf*)(data);

	gprs_connection = ril_gprs_connection_find_contextId(netCnf->contextId);

	if (!gprs_connection) {
		ALOGE("Unable to find GPRS connection, aborting");
		return;
	}

	if (netCnf->error != 0)
	{
		//FIXME: add conversion for error
		ALOGE("There was an error, aborting deactivate data call");
		ril_request_complete(gprs_connection->token, RIL_E_GENERIC_FAILURE, NULL, 0);
		gprs_connection->token = 0;
		return;
	}

	ril_request_complete(gprs_connection->token, RIL_E_SUCCESS, NULL, 0);
	ril_gprs_connection_stop(gprs_connection);

	ril_data.data_call_count--;
	ril_unsol_data_call_list_changed(0);
}

void ipc_proto_receive_data_ind(void* data)
{
	int n;
	struct ril_gprs_connection *gprs_connection = NULL;
	protoTransferDataBuf* rcvData = (protoTransferDataBuf*)data;
	gprs_connection = ril_gprs_connection_find_contextId(rcvData->contextId);
	if(!gprs_connection || gprs_connection->iface < 0)
	{
		ALOGE("%s: Couldn't find gprs_connection context or tun fd is invalid!", __func__);
		return;
	}
	n = write(gprs_connection->iface, rcvData->netBuf, rcvData->netBufLen);
	ALOGD("%s: Wrote %d/%d bytes of the net frame into %s", __func__, n, rcvData->netBufLen, gprs_connection->ifname);
}

void ril_request_setup_data_call(RIL_Token t, void *data, int length)
{
	ALOGE("%s: Test me!", __func__);

	struct ril_gprs_connection *gprs_connection = NULL;
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

	gprs_connection = ril_gprs_connection_start();

	if (!gprs_connection) {
		ALOGE("Unable to create GPRS connection, aborting");
		ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
		return;
	}

	gprs_connection->type = data_call_type_to_proto_type(((char **) data)[6]);
	if(gprs_connection->type == PROTO_TYPE_NONE)
	{
		ALOGE("Unsupported data connection type %s", ((char **) data)[6]);
		ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
		ril_gprs_connection_stop(gprs_connection);
		return;
	}
	gprs_connection->token = t;
	gprs_connection->contextId = 0xFFFFFFFF;

	start_network = (protoStartNetwork *)calloc(1, sizeof(protoStartNetwork));

	start_network->opMode = PROTO_OPMODE_PS;
	start_network->protoType = gprs_connection->type;

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
	ALOGE("%s: Test me!", __func__);

	struct ril_gprs_connection *gprs_connection;
	char *cid;
	int rc;
	protoStopNetwork* stop_network;

	if (data == NULL || length < (int) sizeof(char *))
		goto error;

	cid = ((char **) data)[0];

	gprs_connection = ril_gprs_connection_find_cid(atoi(cid));

	if (!gprs_connection) {
		ALOGE("Unable to find GPRS connection, aborting");
		ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	return;
	}
	gprs_connection->token = t;

	stop_network = (protoStopNetwork *)calloc(1, sizeof(protoStopNetwork));

	stop_network->opMode = PROTO_OPMODE_PS;
	stop_network->protoType = gprs_connection->type;
	stop_network->contextId = gprs_connection->contextId;

	proto_stop_network(stop_network);

	return;
error:
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void ril_request_last_data_call_fail_cause(RIL_Token t)
{
	ALOGE("%s: Test me!", __func__);
	struct ril_gprs_connection *gprs_connection;
	int last_failed_cid;
	int fail_cause;

	last_failed_cid = ril_data.state.gprs_last_failed_cid;

	if (!last_failed_cid) {
		ALOGE("No GPRS connection was reported to have failed");
		goto fail_cause_unspecified;
	}

	gprs_connection = ril_gprs_connection_find_cid(last_failed_cid);

	if (!gprs_connection) {
		ALOGE("Unable to find GPRS connection");
		goto fail_cause_unspecified;
	}

	fail_cause = gprs_connection->fail_cause;

	ril_gprs_connection_stop(gprs_connection);

	goto fail_cause_return;

fail_cause_unspecified:
	fail_cause = PDP_FAIL_ERROR_UNSPECIFIED;

fail_cause_return:
	ril_data.state.gprs_last_failed_cid = 0;

	ril_request_complete(t, RIL_E_SUCCESS, &fail_cause, sizeof(fail_cause));
}

void ril_unsol_data_call_list_changed(RIL_Token t)
{
	ALOGE("%s: test me me!", __func__);
	struct ril_gprs_connection *gprs_connection;
	struct list_head *list;
	RIL_Data_Call_Response_v6 data_call_list[ril_data.data_call_count];
	int i = 0;

	memset(data_call_list, 0, sizeof(data_call_list));

	list = ril_data.gprs_connections;
	while (list != NULL) {
		gprs_connection = (struct ril_gprs_connection *) list->data;
		if (gprs_connection == NULL)
			goto list_continue;

		data_call_list[i].status = PDP_FAIL_NONE;
		data_call_list[i].cid = gprs_connection->cid;
		data_call_list[i].active = 2;
		data_call_list[i].type = proto_type_to_data_call_type(gprs_connection->type);
		data_call_list[i].ifname = gprs_connection->ifname;
		asprintf(&data_call_list[i].addresses, "%d.%d.%d.%d/%d",
			IN_ADDR_FMT(gprs_connection->ip),
			gprs_connection->prefix_len);
		asprintf(&data_call_list[i].dnses, "%d.%d.%d.%d %d.%d.%d.%d",
			IN_ADDR_FMT(gprs_connection->dns1), IN_ADDR_FMT(gprs_connection->dns2));
		asprintf(&data_call_list[i].gateways, "%d.%d.%d.%d",
			IN_ADDR_FMT(gprs_connection->gateway));
		i++;

list_continue:
		list = list->next;
	}

	if (t == 0)
		ril_request_unsolicited(RIL_UNSOL_DATA_CALL_LIST_CHANGED,
		&data_call_list, sizeof(data_call_list));
	else
		ril_request_complete(t, RIL_E_SUCCESS,
		&data_call_list, sizeof(data_call_list));

	for (i = 0; i < ril_data.data_call_count; i++) {
		if (data_call_list[i].addresses)
			free(data_call_list[i].addresses);
		if (data_call_list[i].dnses)
			free(data_call_list[i].dnses);
		if (data_call_list[i].gateways)
			free(data_call_list[i].gateways);
	}

}

void ril_request_data_call_list(RIL_Token t)
{
	ALOGE("%s: test me me!", __func__);
	ril_unsol_data_call_list_changed(t);
}

