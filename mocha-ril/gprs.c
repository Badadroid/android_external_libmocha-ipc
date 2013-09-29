/**
 * This file is part of mocha-ril.
 *
 * Copyright (C) 2011 Denis 'GNUtoo' Carikli <GNUtoo@no-log.org>
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

#include <netinet/in.h>
#include <arpa/inet.h>

#define LOG_TAG "RIL-Mocha-GPRS"
#include <utils/Log.h>
#include <cutils/properties.h>

#include "mocha-ril.h"
#include "util.h"

/* FIXME: we don't want these hardcoded */
#define INTERFACE		"rmnet0"
#define IP_STRING_SIZE		15 + 1

// TODO: Put this in libmocha-ipc headers
extern void ipc_gprs_define_pdp_context_setup(struct ipc_gprs_define_pdp_context *message, char *apn);

// libnetutils missing prototype
extern int ifc_configure(const char *ifname,
	in_addr_t address,
	in_addr_t netmask,
	in_addr_t gateway,
	in_addr_t dns1,
	in_addr_t dns2);

void ipc_gprs_pdp_context_complete(struct ipc_message_info *info)
{
	struct ipc_gen_phone_res *phone_res = (struct ipc_gen_phone_res *) info->data;
	int rc;
	int aseq;

	rc = ipc_gen_phone_res_check(phone_res);
	if(rc < 0) {
		ril_request_complete(reqGetToken(info->aseq), RIL_E_GENERIC_FAILURE, NULL, 0);
		ALOGE("There was an error, aborting PDP context complete");
		return;
	}

	/* We need to get a clean new aseq here */
	aseq = ril_request_reg_id(reqGetToken(info->aseq));

	/* activate the connection */
	ipc_send(IPC_GPRS_PDP_CONTEXT, IPC_TYPE_SET, 
			(void *) &(ril_state.gprs_context), sizeof(struct ipc_gprs_pdp_context), aseq);

	ipc_gen_phone_res_expect_to_abort(aseq, IPC_GPRS_PDP_CONTEXT);
	// TODO: if this aborts, last fail cause will be: PDP_FAIL_ERROR_UNSPECIFIED
}

void ril_request_setup_data_call(RIL_Token t, void *data, int length)
{
	char *username = NULL;
	char *password = NULL;
	char *apn = NULL;

	struct ipc_gprs_define_pdp_context setup_apn_message;
	struct ipc_gprs_pdp_context activate_message;

	/* get the apn, username and password */
	apn = ((char **) data)[2];
	username = ((char **) data)[3];

	if(username != NULL) {
		if(strlen(username) < 2)
			username = "dummy";
	} else {
		username = "dummy";
	}

	password = ((char **) data)[4];

	if(password != NULL) {
		if(strlen(password) < 2)
			password = "dummy";
	} else {
		password = "dummy";
	}

	ALOGD("Requesting data connection to APN '%s'\n", apn);

	/* create the structs with the apn */
	ipc_gprs_define_pdp_context_setup(&setup_apn_message, apn);

	/* create the structs with the username/password tuple */
	ipc_gprs_pdp_context_setup(&(ril_state.gprs_context), username, password);

	/* send the struct to the modem */
	ipc_send(IPC_GPRS_DEFINE_PDP_CONTEXT, IPC_TYPE_SET, 
			(void *) &setup_apn_message, sizeof(struct ipc_gprs_define_pdp_context), reqGetId(t));

	ipc_gen_phone_res_expect_to_func(reqGetId(t), IPC_GPRS_DEFINE_PDP_CONTEXT,
		ipc_gprs_pdp_context_complete);
}

void ipc_gprs_ip_configuration(struct ipc_message_info *info)
{
	/* Quick and dirty configuration, TODO: Handle that better */

        struct ipc_gprs_ip_configuration *ip_config = (struct ipc_gprs_ip_configuration *) info->data;

	char local_ip[IP_STRING_SIZE];
	char gateway[IP_STRING_SIZE];
	char subnet_mask[IP_STRING_SIZE];
	char dns1[IP_STRING_SIZE];
	char dns2[IP_STRING_SIZE];

	char dns_prop_name[PROPERTY_KEY_MAX];
	char gw_prop_name[PROPERTY_KEY_MAX];

	char *response[3];
	int rc;

	/* TODO: transform that into some macros */
	snprintf(local_ip, IP_STRING_SIZE, "%i.%i.%i.%i",(ip_config->ip)[0],(ip_config->ip)[1],
						(ip_config->ip)[2],(ip_config->ip)[3]);

        snprintf(gateway, IP_STRING_SIZE, "%i.%i.%i.%i",(ip_config->ip)[0],(ip_config->ip)[1],
                                                (ip_config->ip)[2],(ip_config->ip)[3]);

        snprintf(subnet_mask, IP_STRING_SIZE, "255.255.255.255");

        snprintf(dns1, IP_STRING_SIZE, "%i.%i.%i.%i",(ip_config->dns1)[0],(ip_config->dns1)[1],
                                                (ip_config->dns1)[2],(ip_config->dns1)[3]);

        snprintf(dns2, IP_STRING_SIZE , "%i.%i.%i.%i",(ip_config->dns2)[0],(ip_config->dns2)[1],
                                                (ip_config->dns2)[2],(ip_config->dns2)[3]);

        ALOGD("GPRS configuration: ip:%s, gateway:%s, subnet_mask:%s, dns1:%s, dns2:%s",
							local_ip, gateway, subnet_mask ,dns1, dns2);
	rc = ifc_configure(INTERFACE, 
			inet_addr(local_ip),
			inet_addr(subnet_mask),
			inet_addr(gateway),
			inet_addr(dns1),
			inet_addr(dns2));
        ALOGD("ifc_configure: %d",rc);

	snprintf(dns_prop_name, sizeof(dns_prop_name), "net.%s.dns1", INTERFACE);
	property_set(dns_prop_name, dns1);
	snprintf(dns_prop_name, sizeof(dns_prop_name), "net.%s.dns2", INTERFACE);
	property_set(dns_prop_name, dns2);
	snprintf(gw_prop_name, sizeof(gw_prop_name), "net.%s.gw", INTERFACE);
	property_set(dns_prop_name, gateway);

	response[0] = "0"; //FIXME: connection id
	response[1] = INTERFACE;
	response[2] = local_ip;

	ril_request_complete(reqGetToken(info->aseq), RIL_E_SUCCESS, response, sizeof(response));
}

void ril_request_deactivate_data_call(RIL_Token t, void *data, int length)
{
	struct ipc_gprs_pdp_context deactivate_message;
	memset(&deactivate_message, 0, sizeof(deactivate_message));
	deactivate_message.unk0[1]=1;

	/* send the struct to the modem */
	ipc_send(IPC_GPRS_PDP_CONTEXT, IPC_TYPE_SET, 
			(void *) &deactivate_message, sizeof(struct ipc_gprs_pdp_context), reqGetId(t));

	ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);
}
