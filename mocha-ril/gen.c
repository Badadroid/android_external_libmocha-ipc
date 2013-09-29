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

#define LOG_TAG "RIL-Mocha-GEN"
#include <utils/Log.h>

#include "mocha-ril.h"
#include "util.h"

/**
 * GEN global vars
 */

struct ipc_gen_phone_res_expect ipc_gen_phone_res_expects[0x20];
int ipc_gen_phone_res_id = 0;

/**
 * IPC_GEN_PHONE_RES has shared aseq (in the header), group, index and type (in the data)
 * On this implementation, we just check aseq and command (group and index).
 * aseq permits to identify the queued request and do what's wanted.
 * It can be either call a function with the struct ipc_message_info,
 * complete the request to RILJ (with or without an error),
 * return to RILJ if there is an error in this response.
 * 
 * It would have been possible to deal with group, index and type only and use 
 * callback functions. Though, what is currently being used is more a "standard"
 * error catch system, that requires less code (no particular function), while
 * it also permits using custom functions, when IPC_GEN_PHONE_RES code is 
 * request-specific.
 * 
 * On a custom function, don't forget to get a clean new aseq if you're going to 
 * send some data to the modem, just liek this:
 * aseq = ril_request_reg_id(reqGetToken(info->aseq));
 */

/**
 * GEN expects functions
 */

void ipc_gen_phone_res_expects_init(void)
{
	memset(ipc_gen_phone_res_expects, 0, sizeof(struct ipc_gen_phone_res_expect) * 0x20);
}

int ipc_gen_phone_res_id_new(void)
{
	ipc_gen_phone_res_id++;
	ipc_gen_phone_res_id %= 0x20;
	return ipc_gen_phone_res_id;
}

int ipc_gen_phone_res_get_id(unsigned char aseq)
{
	int i;

	for(i=0 ; i < 0x20 ; i++)
		if(ipc_gen_phone_res_expects[i].aseq == aseq)
			return i;

	return -1;
}

void ipc_gen_phone_res_clean_id(int id)
{
	ipc_gen_phone_res_expects[id].aseq = 0;
	ipc_gen_phone_res_expects[id].command = 0;
	ipc_gen_phone_res_expects[id].func = NULL;
	ipc_gen_phone_res_expects[id].to_complete = 0;
	ipc_gen_phone_res_expects[id].to_abort = 0;
}

void ipc_gen_phone_res_expect_to_func(unsigned char aseq, unsigned short command,
		void (*func)(struct ipc_message_info *info))
{
	int id = ipc_gen_phone_res_id_new();

	ipc_gen_phone_res_expects[id].aseq = aseq;
	ipc_gen_phone_res_expects[id].command = command;
	ipc_gen_phone_res_expects[id].func = func;
	ipc_gen_phone_res_expects[id].to_complete = 0;
	ipc_gen_phone_res_expects[id].to_abort = 0;
}

void ipc_gen_phone_res_expect_to_complete(unsigned char aseq, unsigned short command)
{
	int id = ipc_gen_phone_res_id_new();

	ipc_gen_phone_res_expects[id].aseq = aseq;
	ipc_gen_phone_res_expects[id].command = command;
	ipc_gen_phone_res_expects[id].func = NULL;
	ipc_gen_phone_res_expects[id].to_complete = 1;
	ipc_gen_phone_res_expects[id].to_abort = 0;
}

void ipc_gen_phone_res_expect_to_abort(unsigned char aseq, unsigned short command)
{
	int id = ipc_gen_phone_res_id_new();

	ipc_gen_phone_res_expects[id].aseq = aseq;
	ipc_gen_phone_res_expects[id].command = command;
	ipc_gen_phone_res_expects[id].func = NULL;
	ipc_gen_phone_res_expects[id].to_complete = 0;
	ipc_gen_phone_res_expects[id].to_abort = 1;
}

/**
 * GEN dequeue function
 */

/**
 * In: IPC_GEN_PHONE_RES
 *   Check the ipc_gen_phone_res_expects queue and act accordingly
 */
void ipc_gen_phone_res(struct ipc_message_info *info)
{
	struct ipc_gen_phone_res *phone_res = (struct ipc_gen_phone_res *) info->data;
	int id = ipc_gen_phone_res_get_id(info->aseq);
	RIL_Errno e;
	int rc;

	// In this case, it can be a real error or we just didn't queue
	if(id < 0) {
		ALOGD("aseq: 0x%x not found in the IPC_GEN_PHONE_RES queue", info->aseq);
		return;
	}

	ALOGD("aseq: 0x%x found in the IPC_GEN_PHONE_RES queue!", info->aseq);

	if(ipc_gen_phone_res_expects[id].command != IPC_COMMAND(phone_res)) {
		ALOGE("IPC_GEN_PHONE_RES aseq (0x%x) doesn't match the queued one with command (0x%x)", 
				ipc_gen_phone_res_expects[id].aseq, ipc_gen_phone_res_expects[id].command);

		if(ipc_gen_phone_res_expects[id].func != NULL) {
			ALOGE("Not safe to run the custom function, reporting generic failure");
			ril_request_complete(reqGetToken(ipc_gen_phone_res_expects[id].aseq), RIL_E_GENERIC_FAILURE, NULL, 0);
		}
	}

	if(ipc_gen_phone_res_expects[id].func != NULL) {
		ipc_gen_phone_res_expects[id].func(info);
		ipc_gen_phone_res_clean_id(id);
		return;
	}

	rc = ipc_gen_phone_res_check(phone_res);
	if(rc < 0)
		e = RIL_E_GENERIC_FAILURE;
	else
		e = RIL_E_SUCCESS;

	if(ipc_gen_phone_res_expects[id].to_complete || (ipc_gen_phone_res_expects[id].to_abort && rc < 0)) {
		ril_request_complete(reqGetToken(ipc_gen_phone_res_expects[id].aseq), e, NULL, 0);
		ipc_gen_phone_res_clean_id(id);
		return;
	}

	ipc_gen_phone_res_clean_id(id);
}
