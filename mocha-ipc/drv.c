/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2011 KB <kbjetdroid@gmail.com>
 * Copyright (C) 2012 Dominik Marszk <dmarszk@gmail.com>
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
#include <drv.h>
#include <errno.h>
#include <tm.h>

#define LOG_TAG "RIL-Mocha-IPC_DRV"
#include <utils/Log.h>

/*
 * TODO: Implement handling of all the IpcDrv packets
 *
 */

#ifdef DEVICE_JET
#include <device/jet/sound_data.h>
char *nvm_file_path = "/efs/bml4";
char *power_dev_path = "/sys/devices/platform/i2c-gpio.6/i2c-6/6-0066/max8998-charger/power_supply/"; 
char *fake_apps_version = "S800MPOJB1";
#elif defined(DEVICE_WAVE)
#include <device/wave/sound_data.h>
char *nvm_file_path = "/dev/block/mtdblock0";
char *power_dev_path = "/sys/devices/platform/i2c-gpio.6/i2c-6/6-0066/max8998-charger/power_supply/"; 
char *fake_apps_version = "S8530JPKA1";
#endif

/*
 * TODO: Read sound config data from file
 */
int32_t get_nvm_data(void *data, uint32_t size)
{
	int32_t fd, retval;
	fd = open(nvm_file_path, O_RDONLY);

	DEBUG_I("file %s open status = %d", nvm_file_path, fd);
	if(fd < 0)
	{
		DEBUG_I("%s: error! %s", __func__, strerror(errno));
		return -1;
	}
	
	retval = read(fd, data, size);
	DEBUG_I("file %s read status = %d", nvm_file_path, retval);
	if(retval < 0)
	{
		DEBUG_I("%s: error! %s", __func__, strerror(errno));
		if (fd > 0)
			close(fd);
		return retval;
	}
		
	if (fd > 0)
		close(fd);
	return 0;
}

void handleReadNvRequest(struct drvNvPacket* rxNvPacket)
{
	struct modem_io request;

	DEBUG_I("size = 0x%x", rxNvPacket->size);

	request.data = malloc((rxNvPacket->size) + sizeof(struct drvPacketHeader));
	request.data[0] = NV_BACKUP_DATA;	
	get_nvm_data(request.data + sizeof(struct drvPacketHeader), rxNvPacket->size);

	request.magic = 0xCAFECAFE;
	request.cmd = FIFO_PKT_DRV;
	request.datasize = rxNvPacket->size + sizeof(struct drvPacketHeader);

	ipc_send(&request);
	free(request.data);
}

#if defined(DEVICE_JET)
void handleJetPmicRequest(struct modem_io *ipc_frame)
{
    uint8_t *payload;
	int32_t retval;
	uint8_t params[3];
	struct drvPMICPacket *pmic_packet;

	pmic_packet = (struct drvPMICPacket *)(ipc_frame->data);

	DEBUG_I("PMIC value = 0x%x", pmic_packet->value);

	params[0] = 1;
	params[1] = 0x9B; //SIMLTTV;
	if (pmic_packet->value >= 0x7D0)
		params[2] = 0x2D;
	else
		params[2] = 0x15;

	retval = ipc_modem_io(params, IOCTL_MODEM_PMIC);

	DEBUG_I("ioctl return value = 0x%x", retval);

	ipc_send(ipc_frame);
}
#endif

void handleSystemInfoRequest()
{
    uint8_t payload[0x14];
	struct drvRequest tx_packet;
	struct modem_io request;
	
	/* TODO: for WAVE add USB TA info sending if there's USB connected (it shouldn't be sent if USB is disconnected) */
	tm_send_packet(0x0,0x0D, (uint8_t*)RCV_MSM_Data, sizeof(RCV_MSM_Data));
	tm_send_packet(0x0,0x12, (uint8_t*)SPK_MSM_Data, sizeof(SPK_MSM_Data));
	tm_send_packet(0x0,0x10, (uint8_t*)EAR_MSM_Data, sizeof(EAR_MSM_Data));
	tm_send_packet(0x0,0x14, (uint8_t*)BTH_MSM_Data, sizeof(BTH_MSM_Data));
	drv_send_packet(SOUND_CONFIG, (uint8_t*)RCV_MSM_Data, sizeof(RCV_MSM_Data));
	drv_send_packet(SOUND_CONFIG, (uint8_t*)EAR_MSM_Data, sizeof(EAR_MSM_Data));
	drv_send_packet(SOUND_CONFIG, (uint8_t*)SPK_MSM_Data, sizeof(SPK_MSM_Data));
	drv_send_packet(SOUND_CONFIG, (uint8_t*)BTH_MSM_Data, sizeof(BTH_MSM_Data));
	memset(payload,0, sizeof(payload));
	memcpy(payload, fake_apps_version, strlen(fake_apps_version));
	drv_send_packet(HIDDEN_SW_VER, payload, 0x14);

	DEBUG_I("Sent all the sound packages");
}

void send_ta_info()
{
	char buf[200];
	struct drvRequest tx_packet;
	struct modem_io request;
	int32_t fd, len;
	uint16_t status = 0;
	
	sprintf(buf, "%s%s", power_dev_path, "usb/online");
	fd = open(buf, O_RDONLY);
	if(fd < 0)
		DEBUG_E("Couldn't open %s, %s", buf, strerror(errno));
	else {
		read(fd, buf, 1);
		if(buf[0] == '1')
			status = 5;
		close(fd);
	}
	if(status == 0) {
		sprintf(buf, "%s%s", power_dev_path, "ac/online");
		fd = open(buf, O_RDONLY);
		if(fd < 0)
			DEBUG_E("Couldn't open %s", buf);
		else {
			read(fd, buf, 1);
			if(buf[0] == '1')
				status = 5;
			close(fd);
		}
	}
	
	drv_send_packet(TA_INFO_RESP, (uint8_t*)&status, 2);
}

void handleFuelGaugeStatus(uint8_t percentage)
{
	char buf[200];
	int32_t fd, len;
	DEBUG_I("%s: Percentage: %d%%", __func__, percentage);
	sprintf(buf, "%s%s", power_dev_path, "battery/capacity");
	fd = open(buf, O_RDWR);
	if(fd < 0)
	{
		DEBUG_E("%s: Error: %s Failed to open %s.", __func__, strerror(errno), buf);
		return;
	}
	sprintf(buf, "%d", percentage);
	len = strlen(buf);
	if(write(fd, buf, strlen(buf)) != len)	
		DEBUG_E("%s: Failed to write battery capacity, error: %s", __func__, strerror(errno));
	close(fd);
}

void ipc_parse_drv(struct ipc_client* client, struct modem_io *ipc_frame)
{
	int32_t retval;
	struct drvPacketHeader *rx_header;

	rx_header = (struct drvPacketHeader *)(ipc_frame->data);
	
    switch (rx_header->drvPacketType) {
	case READ_NV_BACKUP:
		DEBUG_I("ReadNvBackup IpcDrv packet received");
		handleReadNvRequest((struct drvNvPacket *)(ipc_frame->data));
		break;
#if defined (DEVICE_JET)
	case PMIC_PACKET:
		DEBUG_I("PMIC IpcDrv packet received");
		handleJetPmicRequest(ipc_frame);
		break;
#endif
	case SYSTEM_INFO_REQ:
		DEBUG_I("SYSTEM_INFO_REQ IpcDrv packet received");
		handleSystemInfoRequest();
		break;
	case TA_INFO_REQ:
		DEBUG_I("TA_INFO requested");
		send_ta_info();
		break;
	case BATT_GAUGE_STATUS_CHANGE_IND:	
		DEBUG_I("BATT_GAUGE_STATUS_CHANGE_IND IpcDrv packet received");
		handleFuelGaugeStatus(*((uint8_t*)ipc_frame->data + 1));
		break;
	case BATT_GAUGE_STATUS_RESP:	
		DEBUG_I("BATT_GAUGE_STATUS_RESP IpcDrv packet received");
		handleFuelGaugeStatus(*((uint8_t*)ipc_frame->data + 1));
		break;
	default:
		DEBUG_I("IpcDrv Packet type 0x%X is not yet handled", rx_header->drvPacketType);
		DEBUG_I("Frame type = 0x%x\n Frame length = 0x%x", ipc_frame->cmd, ipc_frame->datasize);

		ipc_hex_dump(client, ipc_frame->data, ipc_frame->datasize);

		break;
    }

    DEBUG_I("DRV exit");
}

void drv_send_packet(uint8_t type, uint8_t *data, int32_t data_size)
{
	struct modem_io request;
	request.data = malloc(data_size + sizeof(struct drvPacketHeader));
	request.data[0] = type;
	memcpy(request.data + 1, data, data_size);
	request.magic = 0xCAFECAFE;
	request.cmd = FIFO_PKT_DRV;
	request.datasize = data_size + 1;
	ipc_send(&request);
	free(request.data);
}
