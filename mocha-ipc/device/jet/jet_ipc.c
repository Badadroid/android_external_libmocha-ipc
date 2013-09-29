/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2011 Paul Kocialkowski <contact@paulk.fr>
 *                    Joerie de Gram <j.de.gram@gmail.com>
 *                    Simon Busch <morphis@gravedo.de>
 *
 * Modified for Jet - KB <kbjetdroid@gmail.com>
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

#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <radio.h>

#include "ipc_private.h"
#include "jet_ipc.h"

#define LOG_TAG "RIL-Mocha_JetIPC"
#include <utils/Log.h>

int32_t jet_modem_bootstrap(struct ipc_client *client)
{

	int32_t rc = 0;

    int32_t dpram_fd = -1;

	int32_t fd, retval;

    DEBUG_I("jet_ipc_bootstrap: enter\n");

    DEBUG_I("jet_ipc_bootstrap: open modem_ctl\n");

    dpram_fd = open(DPRAM_TTY, O_RDWR | O_NDELAY);

    if(dpram_fd < 0) {
    	DEBUG_I("jet_ipc_bootstrap: failed to open dev/dpram0\n");
    	return 1;
    }

    DEBUG_I("jet_ipc_bootstrap: send amss_run_request\n");

    ioctl(dpram_fd, IOCTL_MODEM_AMSSRUNREQ);

    DEBUG_I("jet_ipc_bootstrap: closing dev/modem_ctl\n");

    close(dpram_fd);

    DEBUG_I("jet_ipc_bootstrap: exit\n");

    return 0;
}

int32_t jet_ipc_open(void *data, uint32_t size, void *io_data)
{
    struct termios termios;

    int32_t fd = -1;

    if(io_data == NULL)
        return -1;

    fd = *((int32_t *) io_data);

    fd = open(DPRAM_TTY, O_RDWR);

    DEBUG_I("dpram fd = 0x%x\n", fd);

    if(fd < 0) {
        return 1;
    }

    tcgetattr(fd, &termios);
    cfmakeraw(&termios);
    tcsetattr(fd, TCSANOW, &termios);

    memcpy(io_data, &fd, sizeof(int32_t));

    return 0;
}

int32_t jet_ipc_close(void *data, uint32_t size, void *io_data)
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

int32_t jet_ipc_power_on(void *data)
{
    int32_t fd = -1;

    if(data == NULL)
        return -1;

    fd = *((int32_t *) data);

    ioctl(fd, IOCTL_PHONE_ON);

    return 0;
}

int32_t jet_ipc_power_off(void *data)
{
    int32_t fd = -1;

    if(data == NULL)
        return -1;

    fd = *((int32_t *) data);

    ioctl(fd, IOCTL_PHONE_OFF);

    return 0;
}

int32_t send_packet(struct ipc_client *client, struct modem_io *ipc_frame)
{
	int32_t retval;
    struct fifoPacketHeader *ipc;
    uint8_t *frame;
    uint8_t *payload;
    int32_t frame_length;

    /* Frame length: FIFO header + payload length */
    frame_length = (sizeof(*ipc) + ipc_frame->datasize);

    frame = (uint8_t*)malloc(frame_length);

    /* FIFO header */
    ipc = (struct fifoPacketHeader*)(frame);

    ipc->magic = ipc_frame->magic;
    ipc->cmd = ipc_frame->cmd;
    ipc->datasize = ipc_frame->datasize;

    /* FIFO payload */
    payload = (frame + sizeof(*ipc));
    memcpy(payload, ipc_frame->data, ipc_frame->datasize);

	retval = client->handlers->write(frame, frame_length, client->handlers->write_data);

    free(frame);

    return 0;
}

int32_t jet_ipc_send(struct ipc_client *client, struct modem_io *ipc_frame)
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
		free(multiHeader);

		send_packet(client, &multi_packet);

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

int32_t jet_ipc_recv(struct ipc_client *client, struct modem_io *ipc_frame)
{
    uint8_t buf[SIZ_PACKET_HEADER];
    uint8_t *data;
    uint32_t frame_length;
    struct fifoPacketHeader *ipc;
    struct modem_io modem_packet;
    uint32_t num_read;
    uint32_t left;

    num_read = client->handlers->read((void*)buf, sizeof(buf), client->handlers->read_data);

    ipc = (struct fifoPacketHeader *)buf;

    if(num_read == sizeof(buf) && ipc->magic == 0xCAFECAFE) {

        frame_length = ipc->datasize;
        left = frame_length;

        data = (uint8_t*)malloc(left);
        num_read = client->handlers->read((void*)data, left, client->handlers->read_data);

        if(num_read == left) {
            ipc_frame->magic = ipc->magic;
            ipc_frame->cmd = ipc->cmd;
            ipc_frame->datasize = ipc->datasize;

            ipc_frame->data = (uint8_t*)malloc(left);
            memcpy(ipc_frame->data, data , ipc_frame->datasize);

            return 0;
        }
    }

    return 0;
}

int32_t jet_ipc_read(void *data, uint32_t size, void *io_data)
{
    int32_t fd = -1;

    if(io_data == NULL)
        return -1;

    fd = *((int32_t *) io_data);

    if(fd < 0)
        return -1;

    return read(fd, data, size);
}

int32_t jet_ipc_write(void *data, uint32_t size, void *io_data)
{
    int32_t fd = -1;

    if(io_data == NULL)
        return -1;

    fd = *((int32_t *) io_data);

    if(fd < 0)
        return -1;

    return write(fd, data, size);
}

int32_t jet_modem_operations(struct ipc_client *client, void *data, uint32_t cmd)
{
    int32_t fd = -1;

    fd = *((int32_t *)client->handlers->write_data);

    if(fd < 0)
        return -1;
    DEBUG_I("modem_operations ioctl cmd = 0x%x\n", cmd);

    return ioctl(fd, cmd, data);
}

void *jet_ipc_common_data_create(void)
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

int jet_ipc_common_data_destroy(void *io_data)
{
    // This was already done, not an error but we need to return
    if(io_data == NULL)
        return 0;

    free(io_data);

    return 0;
}

int jet_ipc_common_data_set_fd(void *io_data, int fd)
{
    int *common_data;

    if(io_data == NULL)
        return -1;

    common_data = (int *) io_data;
    common_data = &fd;

    return 0;
}

int jet_ipc_common_data_get_fd(void *io_data)
{
    int *common_data;

    if(io_data == NULL)
        return -1;

    common_data = (int *) io_data;

    return (int) *(common_data);
}

struct ipc_handlers jet_default_handlers = {
    .open = jet_ipc_open,
    .close = jet_ipc_close,
    .power_on = jet_ipc_power_on,
    .power_off = jet_ipc_power_off,
    .read = jet_ipc_read,
    .write = jet_ipc_write,
    .common_data = NULL,
    .common_data_create = jet_ipc_common_data_create,
    .common_data_destroy = jet_ipc_common_data_destroy,
    .common_data_set_fd = jet_ipc_common_data_set_fd,
    .common_data_get_fd = jet_ipc_common_data_get_fd,
};

struct ipc_ops jet_ops = {
    .send = jet_ipc_send,
    .recv = jet_ipc_recv,
    .bootstrap = jet_modem_bootstrap,
    .modem_operations = jet_modem_operations,
};

void jet_ipc_register(void)
{
    ipc_register_device_client_handlers(IPC_DEVICE_JET, &jet_ops, &jet_default_handlers);
}
