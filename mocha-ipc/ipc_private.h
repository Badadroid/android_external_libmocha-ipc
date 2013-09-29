/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2011 Simon Busch <morphis@gravedo.de>
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

#ifndef __IPC_PRIVATE_H__
#define __IPC_PRIVATE_H__

#include <radio.h>

struct ipc_ops {
    int32_t (*bootstrap)(struct ipc_client *client);
    int32_t (*modem_operations)(struct ipc_client *client, void *data, uint32_t cmd);
    int32_t (*send)(struct ipc_client *client, struct modem_io *);
    int32_t (*recv)(struct ipc_client *client, struct modem_io *);
};

struct ipc_handlers {
    /* Transport handlers/data */
    ipc_io_handler_cb read;
    void *read_data;
    ipc_io_handler_cb write;
    void *write_data;
    ipc_io_handler_cb open;
    void *open_data;
    ipc_io_handler_cb close;
    void *close_data;

    /* Power handlers */
    ipc_handler_cb power_on;
    void *power_on_data;
    ipc_handler_cb power_off;
    void *power_off_data;

    /* Handlers common data*/
    void *common_data;

    void *(*common_data_create)(void);
    int (*common_data_destroy)(void *io_data);
    int (*common_data_set_fd)(void *io_data, int fd);
    int (*common_data_get_fd)(void *io_data);
};

struct ipc_client {
    ipc_client_log_handler_cb log_handler;
    void *log_data;

    struct ipc_ops *ops;
    struct ipc_handlers *handlers;
};

struct ipc_device_desc {
    struct ipc_ops *client_ops;
    struct ips_handlers *handlers;
};

void ipc_client_log(struct ipc_client *client, const char *message, ...);
void ipc_register_device_client_handlers(int device, struct ipc_ops *client_ops,
											struct ipc_handlers *handlers);

#endif

// vim:ts=4:sw=4:expandtab
