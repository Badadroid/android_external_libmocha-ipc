/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2011 Paul Kocialkowski <contact@paulk.fr>
 *                    Joerie de Gram <j.de.gram@gmail.com>
 *                    Simon Busch <morphis@gravedo.de>
 *
 * Modified for Jet & Wave - 	KB <kbjetdroid@gmail.com>
 *								Dominik Marszk <dmarszk@gmail.com>
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
 *
 */
#undef DEBUG

#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <radio.h>

#include "ipc_private.h"
#include "wave_ipc.h"

#define LOG_TAG "RIL-Mocha_WaveIPC"
#include <utils/Log.h>

#ifdef DEBUG
int32_t log_fd;
#define LOG_PATH "/data/radio/ipc_log.txt"

void hexdump_byte(uint8_t byte, int fd)
{
	char a;
	a = (byte >> 4) & 0xF;
	if(a < 0xA)
		a += 0x30;
	else
		a += 0x37;
	write(fd, &a, 1);
	a = (byte >> 0) & 0xF;
	if(a < 0xA)
		a += 0x30;
	else
		a += 0x37;	
	write(fd, &a, 1);
	write(fd, " ", 1);
}

void log_write(int type, struct modem_io* mio, int size)
{
	int i;
	char a;
#ifndef FM_DEBUG
	if(mio->cmd == 0x06)
		return;
#endif

	if(type == 0)
	{
		write(log_fd, "tx_frame: ", 10); 
	}
	else if(type == 1)
	{
		write(log_fd, "rx_frame: ", 10); 
	}
	else 
	{
		write(log_fd, "unknown: ", 9); 
	}
	for(i = 0; i < 0xC; i++)
	{
		hexdump_byte(((uint8_t*)mio)[i], log_fd);
	}
	for(i = 0; i < mio->datasize; i++)
	{
		hexdump_byte(mio->data[i], log_fd);
	}
	write(log_fd, "\n", 1);
}
#endif

int32_t wave_modem_bootstrap(struct ipc_client *client)
{
    int32_t modemctl_fd = -1;
	int32_t status;

    DEBUG_I("open %s\n", MODEMCTL_PATH);

    modemctl_fd = open(MODEMCTL_PATH, O_RDWR | O_NDELAY);

    if(modemctl_fd < 0) {
    	DEBUG_I("failed to open %s\n", MODEMCTL_PATH);
    	return 1;
    }
	
	ioctl(modemctl_fd, IOCTL_MODEM_GET_STATUS, &status);
	if(status != 0)		
		DEBUG_I("Modem already booted, bootstraping again not supported\n");
	else
	{	
		DEBUG_I("send amss_run_request\n");
		
		/* force status to POWER_ON as it's been done by FOTA */
		ioctl(modemctl_fd, IOCTL_MODEM_SET_STATUS, 3);
		ioctl(modemctl_fd, IOCTL_MODEM_AMSSRUNREQ);
	}
    DEBUG_I("closing %s\n", MODEMCTL_PATH);

    close(modemctl_fd);

    DEBUG_I("exit\n");

    return 0;
}

int32_t wave_ipc_open(void *data, uint32_t size, void *io_data)
{
    int32_t fd = -1;
#ifdef DEBUG
	char buf[50];
	sprintf(buf, "LOG START! Timestamp: %d\n", (int)time(NULL));
#endif

    if(io_data == NULL)
        return -1;

    fd = *((int32_t *) io_data);

    fd = open(MODEMPACKET_PATH, O_RDWR);
#ifdef DEBUG
	log_fd = open(LOG_PATH, O_RDWR | O_CREAT | O_APPEND, 0660);
    DEBUG_I("IPC dump log filename=%s fd = 0x%x\n", LOG_PATH, log_fd);
	write(log_fd, buf, strlen(buf));
#endif

    DEBUG_I("IO filename=%s fd = 0x%x\n", MODEMPACKET_PATH, fd);

    if(fd < 0) {
        return 1;
    }

    memcpy(io_data, &fd, sizeof(int32_t));

    return 0;
}

int32_t wave_ipc_close(void *data, uint32_t size, void *io_data)
{
    int32_t fd = -1;

    if(io_data == NULL)
        return -1;

    fd = *((int32_t *) io_data);

    if(fd) {
        return close(fd);
    }

    return 0;
}

int32_t wave_ipc_power_on(void *data)
{
    int32_t modemctl_fd = -1;

    DEBUG_I("open %s\n", MODEMCTL_PATH);

    modemctl_fd = open(MODEMCTL_PATH, O_RDWR | O_NDELAY);

    if(modemctl_fd < 0) {
    	DEBUG_I("failed to open %s\n", MODEMCTL_PATH);
    	return 1;
    }

    DEBUG_I("send IOCTL_MODEM_ON\n");

    ioctl(modemctl_fd, IOCTL_MODEM_ON);

    DEBUG_I("closing %s\n", MODEMCTL_PATH);

    close(modemctl_fd);

    DEBUG_I("exit\n");

    return 0;
}

int32_t wave_ipc_power_off(void *data)
{
    int32_t modemctl_fd = -1;

    DEBUG_I("open %s\n", MODEMCTL_PATH);

    modemctl_fd = open(MODEMCTL_PATH, O_RDWR | O_NDELAY);

    if(modemctl_fd < 0) {
    	DEBUG_I("failed to open %s\n", MODEMCTL_PATH);
    	return 1;
    }

    DEBUG_I("send IOCTL_MODEM_OFF\n");

    ioctl(modemctl_fd, IOCTL_MODEM_OFF);

    DEBUG_I("closing %s\n", MODEMCTL_PATH);

    close(modemctl_fd);

    DEBUG_I("exit\n");

    return 0;
}

int32_t send_packet(struct ipc_client *client, struct modem_io *ipc_frame)
{
	return client->handlers->write((void*) ipc_frame, 0, client->handlers->write_data);
}

int32_t wave_ipc_send(struct ipc_client *client, struct modem_io *ipc_frame)
{
	int32_t left_data;
	struct modem_io multi_packet;
	struct multiPacketHeader *multiHeader;

	if (ipc_frame->datasize > MAX_SINGLE_FRAME_DATA)
	{
		DEBUG_I("packet to send is larger than 0x1000\n");

		multi_packet.magic = 0xCAFECAFE;
		multi_packet.cmd = FIFO_PKT_FIFO_INTERNAL;
		multi_packet.datasize = 0x0C;

		multiHeader = (struct multiPacketHeader *)malloc(sizeof(struct multiPacketHeader));

		multiHeader->command = 0x02;
		multiHeader->packtLen = ipc_frame->datasize;
		multiHeader->packetType = ipc_frame->cmd;

		multi_packet.data = (uint8_t *)multiHeader;
		send_packet(client, &multi_packet);
		free(multiHeader);

		left_data = ipc_frame->datasize;

		multi_packet.data = ipc_frame->data;

		while (left_data > 0)
		{
			if (left_data > MAX_SINGLE_FRAME_DATA)
			{
				multi_packet.datasize = MAX_SINGLE_FRAME_DATA;
			}
			else
			{
				multi_packet.datasize = left_data;
			}

			send_packet(client, &multi_packet);

			multi_packet.data += MAX_SINGLE_FRAME_DATA;

			left_data -= MAX_SINGLE_FRAME_DATA;
		}
	}
	else
	{
		send_packet(client, ipc_frame);
	}

	return 0;
}

int32_t wave_ipc_recv(struct ipc_client *client, struct modem_io *ipc_frame)
{
	ipc_frame->data = (uint8_t*)malloc(SIZ_PACKET_BUFSIZE);
    return client->handlers->read((void*)ipc_frame, 0, client->handlers->read_data);
}

int32_t wave_ipc_read(void *data, unsigned int size, void *io_data)
{
    int fd = -1;
    int rc;

    if(io_data == NULL)
        return -1;

    if(data == NULL)
        return -1;

    fd = *((int *) io_data);

    if(fd < 0)
        return -1;

    rc = ioctl(fd, IOCTL_MODEM_RECV, data);

    if(rc < 0)
        return -1;
	
#ifdef DEBUG
	log_write(1, data, size);
#endif

    return 0;
}

int32_t wave_ipc_write(void *data, unsigned int size, void *io_data)
{
    int fd = -1;
    int rc;

    if(io_data == NULL)
        return -1;

    fd = *((int *) io_data);

    if(fd < 0)
        return -1;

    rc = ioctl(fd, IOCTL_MODEM_SEND, data);

    if(rc < 0)
        return -1;
		
#ifdef DEBUG
	log_write(0, data, size);
#endif

    return 0;
}

int32_t wave_modem_operations(struct ipc_client *client, void *data, uint32_t cmd)
{
	int32_t ret;
    int32_t modemctl_fd = -1;

    DEBUG_I("open %s\n", MODEMCTL_PATH);

    modemctl_fd = open(MODEMCTL_PATH, O_RDWR | O_NDELAY);

    if(modemctl_fd < 0) {
    	DEBUG_I("failed to open %s\n", MODEMCTL_PATH);
    	return 1;
    }

    DEBUG_I("send ioctl = 0x%x\n", cmd);

    ret = ioctl(modemctl_fd, cmd, data);

    DEBUG_I("closing %s\n", MODEMCTL_PATH);

    close(modemctl_fd);

    DEBUG_I("exit\n");

    return ret;
}

void *wave_ipc_common_data_create(void)
{
    void *io_data;
    int io_data_len;

    io_data_len = sizeof(int);
    io_data = malloc(io_data_len);

    if(io_data == NULL)
        return NULL;

    memset(io_data, 0, io_data_len);

    return io_data;
}

int wave_ipc_common_data_destroy(void *io_data)
{
    // This was already done, not an error but we need to return
    if(io_data == NULL)
        return 0;

    free(io_data);

    return 0;
}

int wave_ipc_common_data_set_fd(void *io_data, int fd)
{
    int *common_data;

    if(io_data == NULL)
        return -1;

    common_data = (int *) io_data;
    common_data = &fd;

    return 0;
}

int wave_ipc_common_data_get_fd(void *io_data)
{
    int *common_data;

    if(io_data == NULL)
        return -1;

    common_data = (int *) io_data;

    return (int) *(common_data);
}

struct ipc_handlers wave_default_handlers = {
    .open = wave_ipc_open,
    .close = wave_ipc_close,
    .power_on = wave_ipc_power_on,
    .power_off = wave_ipc_power_off,
    .read = wave_ipc_read,
    .write = wave_ipc_write,
    .common_data = NULL,
    .common_data_create = wave_ipc_common_data_create,
    .common_data_destroy = wave_ipc_common_data_destroy,
    .common_data_set_fd = wave_ipc_common_data_set_fd,
    .common_data_get_fd = wave_ipc_common_data_get_fd,
};

struct ipc_ops wave_ops = {
    .send = wave_ipc_send,
    .recv = wave_ipc_recv,
    .bootstrap = wave_modem_bootstrap,
    .modem_operations = wave_modem_operations,
};

void wave_ipc_register(void)
{
    ipc_register_device_client_handlers(IPC_DEVICE_WAVE, &wave_ops, &wave_default_handlers);
}
