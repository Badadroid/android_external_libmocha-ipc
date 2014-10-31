/**
 * This file is part of mocha-ril.
 *
 * Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
 * Copyright (C) 2011 Paul Kocialkowski <contact@oaulk.fr>
 * Copyright (C) 2012-2013 Dominik Marszk <dmarszk@gmail.com>
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

#define LOG_TAG "RIL-Mocha-PWR"
#include <time.h>
#include <utils/Log.h>

#include <lbs.h>
#include <tapi.h>
#include <sim.h>
#include <proto.h>
#include <misc.h>

#include "mocha-ril.h"
#include "util.h"

void ipc_cp_system_start(void* data)
{
	struct modem_io *ipc_frame = (struct modem_io*) data; 
	uint32_t desc_size;
	int suffix_size;
	/*  
	* It's not ON yet but AMSS is able to serve most of IPC request already
	*/
	ril_data.state.power_state = POWER_STATE_LPM;
	ril_data.state.radio_state = RADIO_STATE_OFF;

	desc_size = strlen((const char*)ipc_frame->data);
	if(desc_size > 32 || desc_size > ipc_frame->datasize)
		DEBUG_E("too big desc_size: %d", desc_size);
	else
		memcpy(ril_data.cached_sw_version, ipc_frame->data, desc_size);
	ril_data.cached_sw_version[desc_size] = 0x00;
	suffix_size = ipc_frame->datasize - desc_size - 1;
	if(suffix_size > 0) {
		DEBUG_I("dumping rest of data from IPC_SYSTEM packet");
		hex_dump(ipc_frame->data+desc_size+1, suffix_size);
	}

	tapi_init();
	proto_startup();
	lbs_send_init(1);
	ril_sim_init();

	ril_request_unsolicited(RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED, NULL, 0);
	ril_tokens_check();
}

/**
 * In: RIL_REQUEST_RADIO_POWER
 *   Request ON or OFF radio power mode
 *   
 */
void ril_request_radio_power(RIL_Token t, void *data, size_t datalen)
{
	int power_state = *((int *)data);
	unsigned short power_data;
	/* TODO: fix it, implement LPM mode? */
	ALOGD("%s: requested power_state is %d", __func__, power_state);
	if(ril_data.state.power_state == POWER_STATE_OFF) {
		ALOGD("%s: current power state OFF, returning RADIO_NOT_AVAILABLE", __func__);
		ril_request_complete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
		return;
	}
	if(power_state <= 0) {
		ALOGD("Request power to OFF");
		ril_data.state.power_state = POWER_STATE_LPM;
		ril_data.state.radio_state = RADIO_STATE_OFF;
		tapi_set_offline_mode(TAPI_NETWORK_OFFLINE_MODE_ON);
		ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);
	} else {	
		ALOGD("Request power to NORMAL");
		tapi_set_offline_mode(TAPI_NETWORK_OFFLINE_MODE_OFF);
		/* This is an utterly ugly hack-around */
		ril_data.state.power_state = POWER_STATE_NORMAL;
		ril_data.state.radio_state = RADIO_STATE_ON;
		network_start();
		ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);
	}
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED, NULL, 0); 
}
