/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2012 KB <kbjetdroid@gmail.com>
 * Copyright (C) 2013 Dominik Marszk <dmarszk@gmail.com>
 * Copyright (C) 2013 Nikolay Volkov <volk204@mail.ru>
 *
 * Implemented as per the Mocha AP-CP protocol analysis done by Dominik Marszk
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
#include <sim.h>

#define LOG_TAG "RIL-Mocha-SIM"
#include <utils/Log.h>

/*
 * TODO: Implement handling of all the SIM packets
 *
 */

void ipc_parse_sim(struct ipc_client* client, struct modem_io *ipc_frame)
{
	DEBUG_I("Entering ipc_parse_sim");
	int32_t retval, count;
	uint32_t diffedSubtype;
	struct simPacketHeader *simHeader;
	struct simPacket sim_packet;
	struct modem_io request;
	void *frame;
 	uint8_t *payload;
 	uint32_t frame_length;
	uint8_t buf[4];

	struct fifoPacketHeader *fifoHeader;

	DEBUG_I("Frame header = 0x%x\n Frame type = 0x%x\n Frame length = 0x%x", ipc_frame->magic, ipc_frame->cmd, ipc_frame->datasize);

    simHeader = (struct simPacketHeader *)(ipc_frame->data);
    sim_packet.simBuf = (uint8_t *)(ipc_frame->data + sizeof(struct simPacketHeader));

	DEBUG_I("Sim Packet type = 0x%x\n Sim Packet sub-type = 0x%x\n Sim Packet length = 0x%x", simHeader->type, simHeader->subType, simHeader->bufLen);

	if(simHeader->type != 0)
	{
		switch (simHeader->subType)
		{
		case 0x00:
			DEBUG_I("SIM_PACKET OemSimAtkInjectDisplayTextInd rcvd");

			/*struct oemSimPacketHeader *oem_header;
			struct oemSimPacket oem_packet;

			oem_header = (struct oemSimPacketHeader *)(sim_packet.respBuf);
			oem_packet.oemBuf = (uint8_t *)(sim_packet.respBuf + sizeof(struct oemSimPacketHeader));

			DEBUG_I("Sim oem type = 0x%x\n Sim Packet sub-type = 0x%x\n Oem length = 0x%x", oem_header->oemType, oem_header->packetSubType, oem_header->oemBufLen);

			ipc_hex_dump(oem_packet.oemBuf, oem_header->oemBufLen);
*/
			break;
		case 0x24:
			DEBUG_I("SIM_ATK_interface response");
			break;
		default :
			DEBUG_I("Unknown SIM subType %d, discarding the packet...", simHeader->subType);
			break;
		}
	}
	else
	{
		if(simHeader->subType >= SIM_SUBTYPE_DIFF)
		{
			diffedSubtype = simHeader->subType - SIM_SUBTYPE_DIFF;
			if(diffedSubtype >= SIM_SUBTYPE_DIFF)
			{
				//do_nothing
			}
			else
			{
				switch(diffedSubtype)
				{
					case 1:
					case 2: //in this subtype
						//TODO: these 2 subtypes are somewhat special - apps does switch some bool if they are used, not sure what way they are special.
						sim_parse_event(sim_packet.simBuf, simHeader->bufLen);
						buf[0]=0;
						buf[1]=0;		
						sim_atk_send_packet(0x1, 0x31, 0x2, buf);

						break;
					default:
						sim_parse_event(sim_packet.simBuf, simHeader->bufLen);
						break;
				}
			}
		}
		else if(simHeader->subType == 0)
		{
			/* Discard the packet */
		}
		else
		{
			sim_send_oem_req(sim_packet.simBuf, simHeader->bufLen); //bounceback packet
		}
	}
	ipc_hex_dump(client, ipc_frame->data, ipc_frame->datasize);
	DEBUG_I("Leaving ipc_parse_sim");
}
void sim_parse_event(uint8_t* buf, uint32_t bufLen)
{
	simEventPacketHeader* simEvent = (simEventPacketHeader*)(buf);
	simDataRequest sim_data;
	uint16_t simFileId;
	uint16_t simSize;
	switch(simEvent->eventType)
	{
		case SIM_EVENT_BEGIN:
			DEBUG_I("SIM_EVENT_BEGIN");
			break;
		case SIM_EVENT_SIM_OPEN:
		case SIM_EVENT_GET_SIM_OPEN_DATA:
			DEBUG_I("SIM_EVENT_OPEN");
			ipc_invoke_ril_cb(SIM_OPEN, (void*)buf);
			break;
		case SIM_EVENT_VERIFY_PIN1_IND:
			if (buf[sizeof(simEventPacketHeader)] == 3)
			{
				DEBUG_I("SIM_PUK");
				ipc_invoke_ril_cb(SIM_STATUS, (void*)SIM_STATE_PUK);
			} else {
				DEBUG_I("SIM_PIN");
				ipc_invoke_ril_cb(SIM_STATUS, (void*)SIM_STATE_PIN);
			}
			break;
		case SIM_EVENT_VERIFY_CHV:
			DEBUG_I("SIM_EVENT_VERIFY_CHV");
				ipc_invoke_ril_cb(LOCK_STATUS, (void*)buf);
			break;
		case SIM_EVENT_DISABLE_CHV:
			DEBUG_I("SIM_EVENT_DISABLE_CHV");
			ipc_invoke_ril_cb(LOCK_STATUS, (void*)buf);
			break;
		case SIM_EVENT_ENABLE_CHV:
			DEBUG_I("SIM_EVENT_ENABLE_CHV");
			ipc_invoke_ril_cb(LOCK_STATUS, (void*)buf);
			break;
		case SIM_EVENT_UNBLOCK_CHV:
			DEBUG_I("SIM_EVENT_UNBLOCK_CHV");
			ipc_invoke_ril_cb(LOCK_STATUS, (void*)buf);
			break;
		case SIM_EVENT_FILE_INFO:
			DEBUG_I("SIM_EVENT_FILE_INFO");	
			memcpy(&simFileId, (buf + 15), 2);
			/* work around for reading SMSC number */
			if (simFileId == 0x6F42) {
				DEBUG_I("Request for reading SMSC");
				memcpy(&simSize, (buf + 26), 2);
				sim_data.fileId = simFileId;
				sim_data.size = simSize;
				sim_data.fileType = 0x02;
				sim_data.simInd2 = 0x01;
				sim_data.unk0 = 0x00;
				sim_data.unk1 = 0x00;
				sim_data.unk2 = 0x00;
				sim_data.unk3 = 0x00;
				sim_data.unk4 = 0x00;
				sim_data.unk5 = 0x00;
				sim_data.unk6 = 0x00;
				sim_data.unk7 = 0x00;
				sim_data.recordIndex = 0x01;
				sim_read_file_record(0x5, &sim_data);
			}
			else
				ipc_invoke_ril_cb(SIM_IO_RESPONSE, (void*)buf);
			break;
		case SIM_EVENT_READ_FILE:
			DEBUG_I("SIM_EVENT_READ_FILE");
			memcpy(&simFileId, (buf + 11), 2);
			/* work around for reading SMSC number */
			if (simFileId == 0x6F42)
				ipc_invoke_ril_cb(SIM_SMSC_NUMBER, (void*)buf);
			else
				ipc_invoke_ril_cb(SIM_IO_RESPONSE, (void*)buf);
			break;
		case SIM_EVENT_CHANGE_CHV:
			DEBUG_I("SIM_EVENT_CHANGE_PIN");
			ipc_invoke_ril_cb(LOCK_STATUS, (void*)buf);
			break;
		default:
			DEBUG_I("SIM_EVENT_DEFAULT");
			break;

	}
	DEBUG_I("%s: sim event = %d, sim event status = %d",__func__,simEvent->eventType,simEvent->eventStatus);
}

void sim_send_oem_req(uint8_t* simBuf, uint8_t simBufLen)
{
	//simBuf is expected to contain full oemPacket structure
	struct modem_io request;
	struct simPacket sim_packet;
	sim_packet.header.type = 0;
	sim_packet.header.subType = ((struct oemSimPacketHeader *)(simBuf))->type;
	sim_packet.header.bufLen = simBufLen;
	sim_packet.simBuf = simBuf;

	uint32_t bufLen = sim_packet.header.bufLen + sizeof(struct simPacketHeader);
	uint8_t* fifobuf = malloc(bufLen);
	memcpy(fifobuf, &sim_packet.header, sizeof(struct simPacketHeader));
	memcpy(fifobuf + sizeof(struct simPacketHeader), sim_packet.simBuf, sim_packet.header.bufLen);

	request.magic = 0xCAFECAFE;
	request.cmd = FIFO_PKT_SIM;
	request.datasize = bufLen;

	request.data = fifobuf;
	hex_dump(fifobuf, bufLen);

	ipc_send(&request);

	free(fifobuf);
}

void sim_send_oem_data(uint8_t hSim, uint8_t packetType, uint8_t* dataBuf, uint32_t oemBufLen)
{
	SIM_VALIDATE_SID(hSim);

	struct oemSimPacketHeader oem_header;	
	oem_header.type = packetType;
	oem_header.hSim = hSim; //session id
	oem_header.oemBufLen = oemBufLen;
	
	uint32_t simBufLen = oemBufLen + sizeof(struct oemSimPacketHeader) + 1; /* Looks like bug in Bada, but there's always 1 redundant, zero byte */
	uint8_t* simBuf = malloc(simBufLen);
	memset(simBuf, 0x00, simBufLen);
	memcpy(simBuf, &(oem_header), sizeof(struct oemSimPacketHeader));
	if(oemBufLen)
		memcpy(simBuf + sizeof(struct oemSimPacketHeader), dataBuf, oemBufLen);
	
	sim_send_oem_req(simBuf, simBufLen);
	free(simBuf);
}

void sim_verify_chv(uint8_t hSim, uint8_t pinType, char* pin)
{
	uint8_t packetBuf[10];
	SIM_VALIDATE_SID(hSim);
	//TODO: obtain session context, check if session is busy, print exception if it is busy and return failure
	//TODO: if session is not busy, mark it busy
	memset(packetBuf, 0x00, 10);

	packetBuf[0] = pinType;
	memcpy(packetBuf+1, pin, strlen(pin)); //max pin len is 9 digits
	sim_send_oem_data(hSim, SIM_OEM_REQUEST_VERIFY_CHV, packetBuf, 10);
}

void sim_enable_chv(uint8_t hSim, uint8_t pinType, char* pin)
{
	uint8_t packetBuf[10];
	SIM_VALIDATE_SID(hSim);
	//TODO: obtain session context, check if session is busy, print exception if it is busy and return failure
	//TODO: if session is not busy, mark it busy
	memset(packetBuf, 0x00, 10);

	packetBuf[0] = pinType;
	memcpy(packetBuf+1, pin, strlen(pin)); //max pin len is 9 digits
	sim_send_oem_data(hSim, SIM_OEM_REQUEST_ENABLE_CHV, packetBuf, 10);
}

void sim_disable_chv(uint8_t hSim, uint8_t pinType, char* pin)
{
	uint8_t packetBuf[10];
	SIM_VALIDATE_SID(hSim);
	//TODO: obtain session context, check if session is busy, print exception if it is busy and return failure
	//TODO: if session is not busy, mark it busy
	memset(packetBuf, 0x00, 10);

	packetBuf[0] = pinType;
	memcpy(packetBuf+1, pin, strlen(pin)); //max pin len is 9 digits
	sim_send_oem_data(hSim, SIM_OEM_REQUEST_DISABLE_CHV, packetBuf, 10);
}

void sim_change_chv(uint8_t hSim, uint8_t pinType, char* old_pin, char* new_pin)
{
	uint8_t packetBuf[20];
	SIM_VALIDATE_SID(hSim);
	//TODO: obtain session context, check if session is busy, print exception if it is busy and return failure
	//TODO: if session is not busy, mark it busy
	memset(packetBuf, 0x00, 20);

	packetBuf[0] = pinType;
	memcpy(packetBuf+1, old_pin, strlen(old_pin)); //max puk len is 9 digits
	memcpy(packetBuf+11, new_pin, strlen(new_pin)); //max pin len is 9 digits
	sim_send_oem_data(hSim, SIM_OEM_REQUEST_CHANGE_CHV, packetBuf, 20);
}

void sim_unblock_chv(uint8_t hSim, uint8_t pinType, char* puk, char* pin)
{
	uint8_t packetBuf[20];
	SIM_VALIDATE_SID(hSim);
	//TODO: obtain session context, check if session is busy, print exception if it is busy and return failure
	//TODO: if session is not busy, mark it busy
	memset(packetBuf, 0x00, 20);

	packetBuf[0] = pinType;
	memcpy(packetBuf+1, puk, strlen(puk)); //max puk len is 9 digits
	memcpy(packetBuf+11, pin, strlen(pin)); //max pin len is 9 digits
	sim_send_oem_data(hSim, SIM_OEM_REQUEST_UNBLOCK_CHV, packetBuf, 20);
}

int sim_atk_open(void)
{
	//TODO: verify ATK session and create/open it and return handler to it?!
	DEBUG_I("sim_atk_open");
	sim_send_oem_data(0xA, SIM_OEM_REQUEST_ATK_OPEN, NULL, 0); //0xA hSim is hardcoded in bada
	return 0;
}

void sim_open_to_modem(uint8_t hSim)
{
	//TODO: verify, create and initialize session, send real hSim
	DEBUG_I("sim_open_to_modem");
	sim_send_oem_data(hSim, SIM_OEM_REQUEST_OPEN, NULL, 0); //why it starts from 4? hell knows
}

void sim_atk_send_packet(uint32_t atkType, uint32_t atkSubType, uint32_t atkBufLen, uint8_t* atkBuf)
{
	DEBUG_I("Sending sim_atk_send_packet\n");
	struct modem_io request;
	sim_atk_packet_header* atk_header;
	uint8_t* fifobuf;
	uint32_t bufLen = sizeof(sim_atk_packet_header) + atkBufLen;

	fifobuf = malloc(bufLen);
	atk_header = (sim_atk_packet_header*)(fifobuf);

	atk_header->atkType = atkType;
	atk_header->atkSubType = atkSubType;
	atk_header->atkBufLen = atkBufLen;

	memcpy(fifobuf + sizeof(sim_atk_packet_header), atkBuf, atkBufLen);	

	request.magic = 0xCAFECAFE;
	request.cmd = FIFO_PKT_SIM;
	request.datasize = bufLen;

	request.data = fifobuf;

	ipc_send(&request);

	free(fifobuf);
}

void sim_read_file_record(uint8_t hSim, simDataRequest *sim_data)
{
	//TODO: verify, create and initialize session, send real hSim
	uint8_t *data;

	data = malloc(sizeof(simDataRequest));
	memcpy(data, sim_data, sizeof(simDataRequest));

	DEBUG_I("Sending sim_read_file_record\n");
	sim_send_oem_data(hSim, SIM_OEM_REQUEST_READ_FILE_RECORD, data, sizeof(simDataRequest));  //why it starts from 4? hell knows
	free(data);
}

void sim_read_file_binary(uint8_t hSim, simDataRequest *sim_data)
{
	//TODO: verify, create and initialize session, send real hSim
	uint8_t *data;

	data = malloc(sizeof(simDataRequest));
	memcpy(data, sim_data, sizeof(simDataRequest));

	DEBUG_I("Sending sim_read_file_binary\n");
	sim_send_oem_data(hSim, SIM_OEM_REQUEST_READ_FILE_BINARY, data, sizeof(simDataRequest));  //why it starts from 4? hell knows
	free(data);
}

void sim_get_file_info(uint8_t hSim, uint16_t simDataType)
{
	//TODO: verify, create and initialize session, send real hSim
	uint8_t *data;

	data = malloc(sizeof(simDataType));
	memcpy(data,&simDataType,sizeof(simDataType));

	DEBUG_I("Sending sim_get_file_info\n");

	sim_send_oem_data(hSim, SIM_OEM_REQUEST_GET_FILE_INFO, data, sizeof(simDataType)); //why it starts from 4? hell knows
	free(data);
}
