/*
 * This file is part of Samsung-RIL.
 *
 * Copyright (C) 2013 Paul Kocialkowski <contact@paulk.fr>
 *
 * Samsung-RIL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Samsung-RIL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Samsung-RIL.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SIM_H_
#define _SIM_H_

// Values from TS 11.11
#define SIM_COMMAND_READ_BINARY		0xB0
#define SIM_COMMAND_UPDATE_BINARY	0xD6
#define SIM_COMMAND_READ_RECORD		0xB2
#define SIM_COMMAND_UPDATE_RECORD	0xDC
#define SIM_COMMAND_SEEK		0xA2
#define SIM_COMMAND_GET_RESPONSE	0xC0

#define SIM_FILE_STRUCTURE_TRANSPARENT	0x00
#define SIM_FILE_STRUCTURE_LINEAR_FIXED	0x01
#define SIM_FILE_STRUCTURE_CYCLIC	0x03

#define SIM_FILE_TYPE_RFU		0x00
#define SIM_FILE_TYPE_MF		0x01
#define SIM_FILE_TYPE_DF		0x02
#define SIM_FILE_TYPE_EF		0x04

struct sim_file_id {
	unsigned short file_id;
	unsigned char type;
};

struct sim_file_id sim_file_ids[] = {
	{ 0x2F05, SIM_FILE_TYPE_EF },
	{ 0x2FE2, SIM_FILE_TYPE_EF },
	{ 0x3F00, SIM_FILE_TYPE_MF },
	{ 0x4F20, SIM_FILE_TYPE_EF },
	{ 0x5F3A, SIM_FILE_TYPE_DF },
	{ 0x6F05, SIM_FILE_TYPE_EF },
	{ 0x6F06, SIM_FILE_TYPE_EF },
	{ 0x6F07, SIM_FILE_TYPE_EF },
	{ 0x6F11, SIM_FILE_TYPE_EF },
	{ 0x6F13, SIM_FILE_TYPE_EF },
	{ 0x6F14, SIM_FILE_TYPE_EF },
	{ 0x6F15, SIM_FILE_TYPE_EF },
	{ 0x6F16, SIM_FILE_TYPE_EF },
	{ 0x6F17, SIM_FILE_TYPE_EF },
	{ 0x6F18, SIM_FILE_TYPE_EF },
	{ 0x6F38, SIM_FILE_TYPE_EF },
	{ 0x6F38, SIM_FILE_TYPE_EF },
	{ 0x6F3A, SIM_FILE_TYPE_EF },
	{ 0x6F40, SIM_FILE_TYPE_EF },
	{ 0x6F42, SIM_FILE_TYPE_EF },
	{ 0x6F45, SIM_FILE_TYPE_EF },
	{ 0x6F46, SIM_FILE_TYPE_EF },
	{ 0x6F48, SIM_FILE_TYPE_EF },
	{ 0x6F49, SIM_FILE_TYPE_EF },
	{ 0x6F4A, SIM_FILE_TYPE_EF },
	{ 0x6F4D, SIM_FILE_TYPE_EF },
	{ 0x6F50, SIM_FILE_TYPE_EF },
	{ 0x6F56, SIM_FILE_TYPE_EF },
	{ 0x6FAD, SIM_FILE_TYPE_EF },
	{ 0x6FAE, SIM_FILE_TYPE_EF },
	{ 0x6FB7, SIM_FILE_TYPE_EF },
	{ 0x6FC5, SIM_FILE_TYPE_EF },
	{ 0x6FC6, SIM_FILE_TYPE_EF },
	{ 0x6FC7, SIM_FILE_TYPE_EF },
	{ 0x6FC9, SIM_FILE_TYPE_EF },
	{ 0x6FCA, SIM_FILE_TYPE_EF },
	{ 0x6FCB, SIM_FILE_TYPE_EF },
	{ 0x6FCD, SIM_FILE_TYPE_EF },
	{ 0x7F10, SIM_FILE_TYPE_DF },
	{ 0x7F20, SIM_FILE_TYPE_DF },
};

int sim_file_ids_count = sizeof(sim_file_ids) / sizeof(sim_file_ids[0]);

struct sim_file_response {
	unsigned char rfu12[2];
	unsigned char file_size[2];
	unsigned char file_id[2];
	unsigned char file_type;
	unsigned char rfu3;
	unsigned char access_condition[3];
	unsigned char file_status;
	unsigned char file_length;
	unsigned char file_structure;
	unsigned char record_length;
} __attribute__((__packed__));

#endif
