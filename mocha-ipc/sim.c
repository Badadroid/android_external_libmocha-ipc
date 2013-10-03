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
sim_data_request sim_data;
static uint32_t current_simDataCount;

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
			DEBUG_I("Unknown SIM subType %d", simHeader->subType);
			sim_parse_event(sim_packet.simBuf, simHeader->bufLen);
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
	uint16_t current_simDataType;
	uint16_t current_simSomeType;
	switch(simEvent->eventType)
	{
		
		case SIM_EVENT_BEGIN:
//			DEBUG_I("SIM_NOT_READY");			
//			sim_status(1);	
			break;		
		case SIM_EVENT_SIM_OPEN:
			if (simEvent->eventStatus == SIM_CARD_NOT_PRESENT) {
				DEBUG_I("SIM_ABSENT");
				sim_status(0);
			}
			if (simEvent->eventStatus == SIM_OK) {
				DEBUG_I("SIM_READY");
				sim_status(2);
			}
			/* copying IMSI in BCD format from sim packet */
			int imsi_len = buf[0xAE];
			int i = 0;
			for (i = 0; i < imsi_len; i++)
				cached_bcd_imsi[i] = buf[i+0xB2];
			/*Converting IMSI out of dat stuff to ASCII*/
			imsi_bcd2ascii(cached_imsi, cached_bcd_imsi, imsi_len);
			/* Clean print of IMSI*/
			memset(buf + 0xAF, 0, 15);
			break;
		case SIM_EVENT_VERIFY_PIN1_IND:
			if (buf[sizeof(simEventPacketHeader)] == 3)
			{
				DEBUG_I("SIM_PUK");
				sim_status(4);
			} else {
				DEBUG_I("SIM_PIN");
				sim_status(3);
			}
			break;
		case SIM_EVENT_VERIFY_CHV:
			if (simEvent->eventStatus == SIM_OK) {
				pin_status(buf + sizeof(simEventPacketHeader));
			} else {
				DEBUG_I("SIM: something wrong with pin verify responce");
				DEBUG_I("SIM_PIN");
				sim_status(3);
			}
			break;
		case SIM_EVENT_UNBLOCK_CHV:
			if (simEvent->eventStatus == SIM_OK) {
				puk_status(buf + sizeof(simEventPacketHeader));
			} else {
				DEBUG_I("SIM: something wrong with puk verify responce");
				DEBUG_I("SIM_PUK");
				sim_status(4);
			}
			break;
		case SIM_EVENT_FILE_INFO:
			DEBUG_I("SIM_EVENT_FILE_INFO");	
			memcpy(&current_simDataCount, (buf + 30), 4);
			DEBUG_I("%s : current_simDataCount = 0x%x", __func__, current_simDataCount);
			if(current_simDataCount > 0) 
			{	
				memcpy(&current_simSomeType, (buf + 26), 2);
				DEBUG_I("%s : current_simSomeType = 0x%x", __func__, current_simSomeType);
					
				memcpy(&current_simDataType, (buf + 15), 2);
				DEBUG_I("%s : current_simDataType = 0x%x", __func__, current_simDataType);

				sim_data.simDataType = current_simDataType;
				sim_data.someType = current_simSomeType;
				sim_data.simInd1 = 0x02;
				sim_data.simInd2 = 0x01;
				sim_data.unk0 = 0x00;
				sim_data.unk1 = 0x00;
				sim_data.unk2 = 0x00;
				sim_data.unk3 = 0x00;
				sim_data.unk4 = 0x00;
				sim_data.unk5 = 0x00;
				sim_data.unk6 = 0x00;
				sim_data.unk7 = 0x00;
				sim_data.dataCounter = 0x01;
				DEBUG_I("Sent SIM Request type = 0x%x, packet no. %d, total packets = %d\n", sim_data.simDataType, sim_data.dataCounter, current_simDataCount);
				
				sim_get_data_from_modem(0x5, &sim_data);
			}
			break;
		case SIM_EVENT_READ_FILE:
			DEBUG_I("SIM_EVENT_READ_FILE");
			sim_io_response(buf + sizeof(simEventPacketHeader));
			sim_data.dataCounter += 1;
			if (sim_data.dataCounter <= current_simDataCount)
			{
				DEBUG_I("Sent SIM Request type = 0x%x, packet no. %d, total packets = %d\n", sim_data.simDataType, sim_data.dataCounter, current_simDataCount);
				sim_get_data_from_modem(0x5, &sim_data);
			}		
			break;
		default:
			DEBUG_I("SIM DEFAULT");
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
	sim_send_oem_data(hSim, SIM_EVENT_CHANGE_CHV, packetBuf, 10);
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
	sim_send_oem_data(hSim, SIM_EVENT_UNBLOCK_CHV, packetBuf, 20);
}

int sim_atk_open(void)
{
	//TODO: verify ATK session and create/open it and return handler to it?!
	DEBUG_I("sim_atk_open");
	sim_send_oem_data(0xA, 0x1B, NULL, 0); //0xA hSim is hardcoded in bada
	return 0;
}

void sim_open_to_modem(uint8_t hSim)
{
	//TODO: verify, create and initialize session, send real hSim
	DEBUG_I("sim_open_to_modem");
	sim_send_oem_data(hSim, 0x1, NULL, 0); //why it starts from 4? hell knows
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

void sim_status(int simCardStatus)
{
	DEBUG_I("SIM STATUS CHANGED");
	ipc_invoke_ril_cb(SIM_STATUS, (void*)simCardStatus);
}

void pin_status(uint8_t *lockStatus)
{
	DEBUG_I("PIN STATUS CHANGED");
	ipc_invoke_ril_cb(PIN_STATUS, (void*)lockStatus);
}

void puk_status(uint8_t *lockStatus)
{
	DEBUG_I("PUK STATUS CHANGED");
	ipc_invoke_ril_cb(PUK_STATUS, (void*)lockStatus);
}

void sim_get_data_from_modem(uint8_t hSim, sim_data_request *sim_data)
{
	ALOGE("%s: test me!", __func__);
	//TODO: verify, create and initialize session, send real hSim
	uint8_t *data;

	data = malloc(sizeof(sim_data_request));
	memcpy(data, sim_data, sizeof(sim_data_request));

	DEBUG_I("Sending sim_get_data_from_modem\n");
	sim_send_oem_data(hSim, 0x7, data, sizeof(sim_data_request));  //why it starts from 4? hell knows
	free(data);
}

void sim_data_request_to_modem(uint8_t hSim, uint16_t simDataType)
{
	ALOGE("%s: test me!", __func__);
	//TODO: verify, create and initialize session, send real hSim
	uint8_t *data;

	data = malloc(sizeof(simDataType));
	memcpy(data,&simDataType,sizeof(simDataType));

	DEBUG_I("Sending sim_data_request_to_modem\n");

	sim_send_oem_data(hSim, 0x3, data, sizeof(simDataType)); //why it starts from 4? hell knows
	free(data);
}

void sim_io_response(uint8_t* buf)
{
	ipc_invoke_ril_cb(SIM_IO_RESPONSE, (void*)buf);
}

