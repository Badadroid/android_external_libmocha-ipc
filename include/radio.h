/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
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

#ifndef __RADIO_H__
#define __RADIO_H__

#include <stdint.h>
#include <stdio.h>

#include "types.h"
#include "util.h"

enum ipc_device_enum
{
	IPC_DEVICE_JET = 0,
	IPC_DEVICE_WAVE,
	IPC_DEVICE_LAST
};

struct fifoPacketHeader
{
	uint32_t magic;
	uint32_t cmd;
	uint32_t datasize;
};

struct modem_io {
	uint32_t magic;
	uint32_t cmd;
	uint32_t datasize;
	uint8_t *data;
};

enum ipc_ril_cb_type {
	NETWORK_SET_SUBSCRIPTION_MODE = 0,
	NETWORK_RADIO_INFO,
	NETWORK_SELECT,
	NETWORK_CELL_INFO,
	NETWORK_OPT_ERROR,
	NETWORK_NITZ_INFO_IND,
    NETWORK_SEARCH_CNF,
    NETWORK_SELECT_CNF,
	CP_SYSTEM_START,
    CALL_INCOMING_IND,
    CALL_END_IND,
    CALL_SETUP_IND,
	CALL_HOLD,
    CALL_SWAP,
	CALL_ACTIVATE,
    CALL_ALERT,
    CALL_CONNECTED,
    CALL_DTMF_START,
    CALL_DTMF_STOP,
    CALL_ERROR,
    NETTEXT_INCOMING,
    NETTEXT_SEND_CALLBACK,
    SIM_OPEN,
    SIM_STATUS,
    LOCK_STATUS,
    SIM_IO_RESPONSE,
    SS_USSD_CALLBACK,
    SS_ERROR,
    PROTO_START_NETWORK_CNF,
    PROTO_STOP_NETWORK_CNF,
    PROTO_RECEIVE_DATA_IND,
	IPC_RIL_CB_LAST
};

typedef enum {
	SIM_STATE_ABSENT			= 0,
	SIM_STATE_NOT_READY			= 1,
	SIM_STATE_READY				= 2,
	SIM_STATE_PIN				= 3,
	SIM_STATE_PUK				= 4,
	SIM_STATE_BLOCKED			= 5,
	SIM_STATE_NETWORK_PERSO 		= 6,
	SIM_STATE_NETWORK_SUBSET_PERSO		= 7,
	SIM_STATE_CORPORATE_PERSO		= 8,
	SIM_STATE_SERVICE_PROVIDER_PERSO	= 9,
} ril_sim_state;

extern uint8_t cached_bcd_imei[9];
extern char cached_imei[33];
extern char* fake_apps_version;

typedef void (*ipc_ril_cb)(void* data);
typedef void (*ipc_client_log_handler_cb)(const char *message, void *user_data);

typedef int (*ipc_io_handler_cb)(void *data, unsigned int size, void *io_data);
typedef int (*ipc_handler_cb)(void *data);

struct ipc_client;
struct ipc_handlers;

void ipc_dispatch(struct ipc_client* client, struct modem_io *resp);

void ipc_init(void);
void ipc_shutdown(void);

void ipc_register_ril_cb(int type, ipc_ril_cb cb);
void ipc_invoke_ril_cb(int type, void* data);

struct ipc_client* ipc_client_new();
struct ipc_client *ipc_client_new_for_device(int device_type);
int ipc_client_free(struct ipc_client *client);

int ipc_client_set_log_handler(struct ipc_client *client, ipc_client_log_handler_cb log_handler_cb, void *user_data);

int ipc_client_set_handlers(struct ipc_client *client, struct ipc_handlers *handlers);
int ipc_client_set_io_handlers(struct ipc_client *client, 
                               ipc_io_handler_cb read, void *read_data,
                               ipc_io_handler_cb write, void *write_data);
int ipc_client_set_handlers_common_data(struct ipc_client *client, void *data);
void *ipc_client_get_handlers_common_data(struct ipc_client *client);
int ipc_client_create_handlers_common_data(struct ipc_client *client);
int ipc_client_destroy_handlers_common_data(struct ipc_client *client);
int ipc_client_set_handlers_common_data_fd(struct ipc_client *client, int fd);
int ipc_client_get_handlers_common_data_fd(struct ipc_client *client);

int ipc_client_bootstrap_modem(struct ipc_client *client);
int ipc_client_modem_operations(struct ipc_client *client, void *data, unsigned int cmd);
int ipc_client_open(struct ipc_client *client);
int ipc_client_close(struct ipc_client *client);
int ipc_client_power_on(struct ipc_client *client);
int ipc_client_power_off(struct ipc_client *client);

int ipc_client_recv(struct ipc_client *client, struct modem_io *ipc_frame);

/* Convenience functions for ipc_send */
int ipc_client_send(struct ipc_client *client, struct modem_io *ipc_frame);
void ipc_client_send_get(struct ipc_client *client, const unsigned short command, unsigned char mseq);
void ipc_client_send_exec(struct ipc_client *client, const unsigned short command, unsigned char mseq);

/* Utility functions */
void imei_bcd2ascii(char* out, const uint8_t* in);
void imsi_bcd2ascii(char* out, const uint8_t* in, int len);
void bcd2ascii(char* out, const uint8_t* in, int size);
void ipc_hex_dump(struct ipc_client *client, void *data, int size);
void *ipc_mtd_read(struct ipc_client *client, char *mtd_name, int size, int block_size);
void *ipc_file_read(struct ipc_client *client, char *file_name, int size, int block_size);

#ifndef RIL_SHLIB

struct ipc_client *client;

static inline void *mtd_read(char *mtd_name, int size, int block_size)
{
	return ipc_mtd_read(client, mtd_name, size, block_size);
}

static inline void hex_dump(void *data, int size)
{
	ipc_hex_dump(client, data, size);
}

static inline void ipc_send(struct modem_io *ipc_frame)
{
	ipc_client_send(client, ipc_frame);
}

static inline int ipc_modem_io(void *data, uint32_t cmd)
{
	return ipc_client_modem_operations(client, data, cmd);
}

#else
	extern void hex_dump(void *data, int size);
	extern void ipc_send(struct modem_io *ipc_frame);
	extern int ipc_modem_io(void *data, uint32_t cmd);

#endif //RIL_SHLIB

#endif

// vim:ts=4:sw=4:expandtab
