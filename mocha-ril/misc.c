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

#define LOG_TAG "RIL-Mocha-MISC"
#include <utils/Log.h>

#include "mocha-ril.h"
#include "util.h"
#include <misc.h>
#include <drv.h>

void ril_request_get_imei(RIL_Token t)
{
	if(cached_imei[0] != 0x00) {
		ril_request_complete(t, RIL_E_SUCCESS, cached_imei, sizeof(cached_imei));
	} else {
		ALOGE("%s: Error!", __func__);
		ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	}
}


void ril_request_baseband_version(RIL_Token t)
{

	if(ril_data.state.radio_state != RADIO_STATE_OFF) {	
		ril_request_complete(t, RIL_E_SUCCESS, cached_sw_version, sizeof(cached_sw_version));
	} else {
		ALOGE("%s: Error!", __func__);
		ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	}
}


void ril_request_screen_state(RIL_Token t, void *data, size_t datalen)
{
	uint32_t status = 0;

	if (((int *)data)[0] == 0 )
		ipc_power_mode(0);
	else
		drv_send_packet(BATT_GAUGE_STATUS_REQ, (uint8_t*)&status, 4);
		ipc_power_mode(6);


	ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);

}


/**
 * In: RIL_REQUEST_GET_IMSI
 *   Get the SIM IMSI
 *   Only valid when radio state is "RADIO_STATE_SIM_READY"
 *
 * Out: IPC_MISC_ME_IMSI
 *   Requests ME's IMSI
 */
void ril_request_get_imsi(RIL_Token t)
{
	if (cached_imsi[0] != 0x00) {
		ril_request_complete(t, RIL_E_SUCCESS, cached_imsi, sizeof(cached_imsi));
	} else {
		ALOGE("%s: Error!", __func__);
		ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	}
}
