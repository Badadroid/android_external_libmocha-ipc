/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2012 Dominik Marszk <dmarszk@gmail.com>
 *
 *
 * libmocha-ipc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libmocha-ipc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libmocha-ipc.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <getopt.h>

#include <radio.h>
#include <tm.h>
#include <drv.h>

#define LOG_TAG "RIL-Mocha-TM"
#include <utils/Log.h>

void ipc_parse_tm(struct ipc_client* client, struct modem_io *ipc_frame)
{
	if (ipc_frame->datasize == 0x8c)
		tm_bat_info((struct tm_battery_info *)(ipc_frame->data));
	else
	{
		DEBUG_I("Test_Mode Parser");
		DEBUG_I("Frame type = 0x%x\n Frame length = 0x%x\n", ipc_frame->cmd, ipc_frame->datasize);
		ipc_hex_dump(client, ipc_frame->data, ipc_frame->datasize);
	}
}

void tm_send_packet(uint8_t group, uint8_t type, uint8_t *data, int32_t data_size)
{
	struct modem_io pkt;
	pkt.data = malloc(data_size + sizeof(struct tm_tx_packet_header));	
	pkt.data[0] = group;
	pkt.data[1] = type;
	if(data_size > 0)
		memcpy(pkt.data + 2, data, data_size);
	pkt.magic = 0xCAFECAFE;
	pkt.cmd = FIFO_PKT_TESTMODE;
	pkt.datasize = data_size + 2;
	ipc_send(&pkt);
	free(pkt.data);
}

void tm_bat_info(struct tm_battery_info *bat_info)
{
	//DEBUG_I("Battery info: \n ADC_val = %d raw_volt= %d\n raw_soc= %f%% adj_soc=%f%% max_soc=%f%%", bat_info->ADC_val, bat_info->raw_volt, bat_info->raw_soc,bat_info->adj_soc, bat_info->max_soc);
	char buf[20];
	int32_t len;

	sprintf(buf, "%d", bat_info->ADC_val);
	len = strlen(buf);
	if(write(fd_temp, buf, strlen(buf)) != len)
		DEBUG_E("%s: Failed to write battery ADC_val, error: %s", __func__, strerror(errno));

	sprintf(buf, "%d", bat_info->raw_volt * 1000);
	len = strlen(buf);
	if(write(fd_volt, buf, strlen(buf)) != len)
		DEBUG_E("%s: Failed to write battery raw_volt, error: %s", __func__, strerror(errno));

	handleFuelGaugeStatus(bat_info->bat_gauge);
}
