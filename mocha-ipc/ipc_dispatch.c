/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 	2011-2013 KB <kbjetdroid@gmail.com>
 * 					2011-2013 Dominik Marszk <dmarszk@gmail.com>
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

#include <drv.h>
#include <tapi.h>
#include <fm.h>
#include <sim.h>
#include <radio.h>
#include <misc.h>
#include <proto.h>
#include <sound.h>
#include <bt.h>

#define LOG_TAG "RIL-Mocha-IPC-PARSER"
#include <utils/Log.h>

void ipc_dispatch(struct ipc_client *client, struct modem_io *ipc_frame)
{
	switch(ipc_frame->cmd)
    {
		case FIFO_PKT_SIM:
			ipc_parse_sim(client, ipc_frame);
		break;
		case FIFO_PKT_PROTO:
			ipc_parse_proto(client, ipc_frame);
		break;
        case FIFO_PKT_TAPI:
        	ipc_parse_tapi(client, ipc_frame);
        break;
        case FIFO_PKT_FILE:
        	ipc_parse_fm(client, ipc_frame);
			/*
        	if (ret)
        	{
        		modem_send_tapi_init(ipc_frame);
        		sim_atk_open(0);
        		sim_open_to_modem(0);
        	}*/
        break;
		case FIFO_PKT_SOUND:
            ipc_parse_sound(client, ipc_frame);
		break;
        case FIFO_PKT_DVB_H_DebugLevel:
            ipc_parse_dbg_level(client, ipc_frame);
        break;
        case FIFO_PKT_BOOT:
            ipc_parse_boot(client, ipc_frame);
        break;
		case FIFO_PKT_SYSTEM:
			ipc_parse_system(client, ipc_frame);
		break;
        case FIFO_PKT_DRV:
        	ipc_parse_drv(client, ipc_frame);
        break;
        case FIFO_PKT_DEBUG:
        	ipc_parse_dbg(client, ipc_frame);
	        break;
        case FIFO_PKT_BLUETOOTH:
        	ipc_parse_bt(client, ipc_frame);
 	       break;
        default :
        	DEBUG_I("Packet type 0x%x not yet handled\n", ipc_frame->cmd);
        	DEBUG_I("Frame header = 0x%x\n Frame type = 0x%x\n Frame length = 0x%x\n", 
			ipc_frame->magic, ipc_frame->cmd, ipc_frame->datasize);
        	ipc_hex_dump(client, ipc_frame->data, ipc_frame->datasize);

    }
}
