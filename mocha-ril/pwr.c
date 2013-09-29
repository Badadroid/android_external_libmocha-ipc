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
	/*  
	* It's not ON yet but AMSS is able to serve most of IPC request already
	*/
	ril_data.state.power_state = POWER_STATE_LPM;
	ril_data.state.radio_state = RADIO_STATE_OFF;

	tapi_init();
	proto_startup();
	lbs_init();
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
		ril_data.state.power_state = POWER_STATE_NORMAL;
		ril_data.state.radio_state = RADIO_STATE_ON;
		network_start();
	}
	
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED, NULL, 0);
	return;
#if 0
	if(power_state > 0) {
		ALOGD("Request power to NORMAL");
		power_data = IPC_PWR_PHONE_STATE_NORMAL;
		ipc_send(IPC_PWR_PHONE_STATE, IPC_TYPE_EXEC, (void *) &power_data, sizeof(power_data), reqGetId(t));

		ril_data.state.radio_power = t;

		/* Don't tell the RIL we're not off anymore: wait for the message */
	} else {
		ALOGD("Request power to LPM");
		power_data = IPC_PWR_PHONE_STATE_LPM;
		ipc_send(IPC_PWR_PHONE_STATE, IPC_TYPE_EXEC, (void *) &power_data, sizeof(power_data), reqGetId(t));

		ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);

		/* We're not going to get any message to make sure we're in LPM so tell RILJ we're off anyway */
		ril_globals_init();
		ril_data.state_lpm();
		ril_request_unsolicited(RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED, NULL, 0);
	}
#endif
}
