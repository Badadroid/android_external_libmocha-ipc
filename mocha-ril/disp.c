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

#define LOG_TAG "RIL-Mocha-DISP"
#include <utils/Log.h>

#include "mocha-ril.h"
#include "util.h"

void ipc_disp_icon_info(struct ipc_message_info *info)
{
	struct ipc_disp_icon_info *signal_info = (struct ipc_disp_icon_info *) info->data;
	RIL_SignalStrength ss;
	int rssi;

	/* Don't consider this if modem isn't in normal power mode. */
	if(ril_state.power_mode < POWER_MODE_NORMAL)
		return;

	memset(&ss, 0, sizeof(ss));

	/* Multiplying the number of bars by 3 yields
	 * an asu with an equal number of bars in Android
	 */
	rssi = (3 * signal_info->rssi);

	ss.GW_SignalStrength.signalStrength = rssi;
	ss.GW_SignalStrength.bitErrorRate = 99;

	/* Send CDMA and EVDO levels even in GSM mode */
	ss.CDMA_SignalStrength.dbm = rssi;
	ss.CDMA_SignalStrength.ecio = 200;

	ss.EVDO_SignalStrength.dbm = rssi;
	ss.EVDO_SignalStrength.ecio = 200;

	ALOGD("Signal Strength is %d\n", rssi);

	ril_request_unsolicited(RIL_UNSOL_SIGNAL_STRENGTH, &ss, sizeof(ss));
}

void ipc_disp_rssi_info(struct ipc_message_info *info)
{
	struct ipc_disp_rssi_info *rssi_info = (struct ipc_disp_rssi_info *) info->data;
	RIL_SignalStrength ss;
	int rssi;

	/* Don't consider this if modem isn't in normal power mode. */
	if(ril_state.power_mode < POWER_MODE_NORMAL)
		return;

	memset(&ss, 0, sizeof(ss));

	if(rssi_info->rssi > 0x6f) {
		rssi = 0;
	} else {
		rssi = (((rssi_info->rssi - 0x71) * -1) - ((rssi_info->rssi - 0x71) * -1) % 2) / 2;
		if(rssi > 31)
			rssi = 31;
	}

	/* Send CDMA and EVDO levels even in GSM mode */
	ss.GW_SignalStrength.signalStrength = rssi;
	ss.GW_SignalStrength.bitErrorRate = 99;

	ss.CDMA_SignalStrength.dbm = rssi;
	ss.CDMA_SignalStrength.ecio = 200;

	ss.EVDO_SignalStrength.dbm = rssi;
	ss.EVDO_SignalStrength.ecio = 200;

	ALOGD("Signal Strength is %d\n", rssi);

	ril_request_unsolicited(RIL_UNSOL_SIGNAL_STRENGTH, &ss, sizeof(ss));
}
