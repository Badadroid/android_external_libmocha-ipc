/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
 *               2011 Simon Busch <morphis@gravedo.de>
 *				 2011-2012 KB <kbjetdroid@gmail.com>
 *				 2011-2012 Dominik Marszk <dmarszk@gmail.com>
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
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdbool.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <asm/types.h>

#include <radio.h>

#include "ipc_private.h"

#define LOG_TAG "RIL-Mocha-IPC-LIB"
#include <utils/Log.h>

struct ipc_device_desc devices[IPC_DEVICE_LAST];

ipc_ril_cb ipc_ril_cb_map[IPC_RIL_CB_LAST];

uint8_t cached_bcd_imei[9];
char cached_imei[33];

extern void jet_ipc_register();
extern void wave_ipc_register();

void ipc_init(void)
{
	int i;
	memset(cached_bcd_imei, 0, sizeof(cached_bcd_imei));
	memset(cached_imei, 0, sizeof(cached_imei));
#if defined(DEVICE_JET)
    jet_ipc_register();
#elif defined(DEVICE_WAVE)
    wave_ipc_register();
#endif

	for(i = 0; i < IPC_RIL_CB_LAST; i++)
	{
		ipc_ril_cb_map[i] = NULL;
	}
}

void ipc_shutdown(void)
{
}

void ipc_register_ril_cb(int type, ipc_ril_cb cb)
{
	if(ipc_ril_cb_map[type])
	{
		DEBUG_W("Registering more than one callback for type %d! Overriding it.", (int)type);
	}
	ipc_ril_cb_map[type] = cb;
}

void ipc_invoke_ril_cb(int type, void* data)
{
	//DEBUG_I("Invoking RIL callback of type %d", type);
	if(ipc_ril_cb_map[type])
	{
		ipc_ril_cb_map[type](data);
	}
	else
		DEBUG_W("Missing IPC RIL CB of type %d", (int) type);
}

void log_handler_default(const char *message, void *user_data)
{
    printf("%s\n", message);
}

void ipc_register_device_client_handlers(int device, struct ipc_ops *client_ops,
                                         struct ipc_handlers *handlers)
{
    devices[device].client_ops = client_ops;
    devices[device].handlers = handlers;
}

void ipc_client_log(struct ipc_client *client, const char *message, ...)
{
    assert(client->log_handler != NULL);

    va_list args;
    char buffer[4096];

    va_start(args, message);
    vsprintf(buffer, message, args);
    client->log_handler(buffer, client->log_data);
    va_end(args);
}

struct ipc_client* ipc_client_new()
{
    int device_type = -1, in_hardware = 0;
    char buf[4096];

    // gather device type from /proc/cpuinfo
    int fd = open("/proc/cpuinfo", O_RDONLY);
    int bytesread = read(fd, buf, 4096);
    close(fd);

    // match hardware name with our supported devices
    char *pch = strtok(buf, "\n");
    while (pch != NULL)
    {
        int rc;
        if ((rc = strncmp(pch, "Hardware", 9)) == 9)
        {
            if (strstr(pch, "GT-S8000") != NULL)
                device_type = IPC_DEVICE_JET;
            else if (strstr(pch, "wave") != NULL)
                device_type = IPC_DEVICE_WAVE;
        }
        pch = strtok(NULL, "\n");
    }

    // validate that we have found any supported device
    if (device_type == -1)
        return NULL;

    return ipc_client_new_for_device(device_type);
}

struct ipc_client* ipc_client_new_for_device(int device_type)
{
    struct ipc_client *client;

    if (device_type < 0 || device_type > IPC_DEVICE_LAST)
        return 0;

    client = (struct ipc_client*) malloc(sizeof(struct ipc_client));

	client->ops = devices[device_type].client_ops;


    client->handlers = (struct ipc_handlers *) malloc(sizeof(struct ipc_handlers));
    client->log_handler = log_handler_default;
    if (devices[device_type].handlers != 0)
        memcpy(client->handlers, devices[device_type].handlers , sizeof(struct ipc_handlers));

    return client;
}

int ipc_client_free(struct ipc_client *client)
{
    free(client->handlers);
    free(client);
    client = NULL;
    return 0;
}

int32_t ipc_client_set_log_handler(struct ipc_client *client, ipc_client_log_handler_cb log_handler_cb, void *user_data)
{
    if(client == NULL)
        return -1;

    client->log_handler = log_handler_cb;
    client->log_data = user_data;

    return 0;
}

int32_t ipc_client_set_handlers(struct ipc_client *client, struct ipc_handlers *handlers)
{
    if(client == NULL ||
       handlers == NULL)
        return -1;

    memcpy(client->handlers, handlers, sizeof(struct ipc_handlers));

    return 0;
}

int32_t ipc_client_set_io_handlers(struct ipc_client *client,
                               ipc_io_handler_cb read, void *read_data,
                               ipc_io_handler_cb write, void *write_data)
{
    if(client == NULL ||
       client->handlers == NULL)
        return -1;

    if(read != NULL)
        client->handlers->read = read;
    if(read_data != NULL)
        client->handlers->read_data = read_data;
    if(write != NULL)
        client->handlers->write = write;
    if(write_data != NULL)
        client->handlers->write_data = write_data;

    return 0;
}

int ipc_client_set_handlers_common_data(struct ipc_client *client, void *data)
{
    void *common_data;

    if(client == NULL ||
       client->handlers == NULL ||
       data == NULL)
        return -1;

    common_data = data;
    client->handlers->common_data = common_data;

    client->handlers->read_data = common_data;
    client->handlers->write_data = common_data;
    client->handlers->open_data = common_data;
    client->handlers->close_data = common_data;
    client->handlers->power_on_data = common_data;
    client->handlers->power_off_data = common_data;

    return 0;
}

void *ipc_client_get_handlers_common_data(struct ipc_client *client)
{
    if(client == NULL ||
       client->handlers == NULL)
        return NULL;

    return client->handlers->common_data;
}

int ipc_client_create_handlers_common_data(struct ipc_client *client)
{
    void *common_data;

    if(client == NULL ||
       client->handlers == NULL)
        return -1;

    common_data = client->handlers->common_data_create();
    client->handlers->common_data = common_data;

    client->handlers->read_data = common_data;
    client->handlers->write_data = common_data;
    client->handlers->open_data = common_data;
    client->handlers->close_data = common_data;
    client->handlers->power_on_data = common_data;
    client->handlers->power_off_data = common_data;

    return 0;
}

int ipc_client_destroy_handlers_common_data(struct ipc_client *client)
{
    void *common_data;
    int rc;

    if(client == NULL ||
       client->handlers == NULL ||
       client->handlers->common_data_destroy == NULL)
        return -1;

    rc = client->handlers->common_data_destroy(client->handlers->common_data);

    if(rc < 0)
        return -1;

    common_data = NULL;
    client->handlers->common_data = common_data;

    client->handlers->read_data = common_data;
    client->handlers->write_data = common_data;
    client->handlers->open_data = common_data;
    client->handlers->close_data = common_data;
    client->handlers->power_on_data = common_data;
    client->handlers->power_off_data = common_data;

    return 0;
}

int ipc_client_set_handlers_common_data_fd(struct ipc_client *client, int fd)
{
    if(client == NULL ||
       client->handlers == NULL ||
       client->handlers->common_data_set_fd == NULL)
        return -1;

    return client->handlers->common_data_set_fd(client->handlers->common_data, fd);
}

int ipc_client_get_handlers_common_data_fd(struct ipc_client *client)
{
    if(client == NULL ||
       client->handlers == NULL ||
       client->handlers->common_data_get_fd == NULL)
        return -1;

    return client->handlers->common_data_get_fd(client->handlers->common_data);
}
int32_t ipc_client_bootstrap_modem(struct ipc_client *client)
{
    if (client == NULL ||
        client->ops == NULL ||
        client->ops->bootstrap == NULL)
        return -1;

    return client->ops->bootstrap(client);
}

int32_t ipc_client_modem_operations(struct ipc_client *client, void *data, uint32_t cmd)
{
    if (client == NULL ||
        client->ops == NULL ||
        client->ops->modem_operations == NULL)
        return -1;

    return client->ops->modem_operations(client, data, cmd);
}

int32_t ipc_client_open(struct ipc_client *client)
{
    int32_t fd;

    if (client == NULL ||
        client->handlers == NULL ||
        client->handlers->open == NULL)
        return -1;

    return client->handlers->open(NULL, 0, client->handlers->open_data);
}

int32_t ipc_client_close(struct ipc_client *client)
{
    if (client == NULL ||
        client->handlers == NULL ||
        client->handlers->close == NULL)
        return -1;

    return client->handlers->close(NULL, 0, client->handlers->close_data);
}

int32_t ipc_client_power_on(struct ipc_client *client)
{
	return 0; //for jet
    if (client == NULL ||
        client->handlers == NULL ||
        client->handlers->power_on == NULL)
        return -1;

    return client->handlers->power_on(client->handlers->power_on_data);
}

int32_t ipc_client_power_off(struct ipc_client *client)
{
	return 0; //for jet
	if (client == NULL ||
        client->handlers == NULL ||
        client->handlers->power_off == NULL)
        return -1;

    return client->handlers->power_off(client->handlers->power_off_data);
}

int32_t ipc_client_send(struct ipc_client *client, struct modem_io *ipc_frame)
{
    if (client == NULL ||
        client->ops == NULL ||
        client->ops->send == NULL)
        return -1;

    return client->ops->send(client, ipc_frame);
}

int32_t ipc_client_recv(struct ipc_client *client, struct modem_io *ipc_frame)
{
    if (client == NULL ||
        client->ops == NULL ||
        client->ops->recv == NULL)
        return -1;

    return client->ops->recv(client, ipc_frame);
}
