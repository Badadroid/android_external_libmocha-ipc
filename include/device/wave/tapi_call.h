/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2012 Dominik Marszk <dmarszk@gmail.com>
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
 *
 */

#ifndef __DEVICE_WAVE_CALL_H__
#define __DEVICE_WAVE_CALL_H__

/**
 * All the TAPI call context structures specific to Wave will be defined here
 */
enum TAPI_CALL_TYPE
{
	TAPI_CALL_APIREQ = 0x0,
	TAPI_CALL_SETUP	= 0x1,
	TAPI_CALL_ANSWER = 0x2,
	TAPI_CALL_RELEASE = 0x3,
	TAPI_CALL_START_DTMF = 0x4,
	TAPI_CALL_STOP_DTMF = 0x5,
	TAPI_CALL_FLASH = 0x7,
	TAPI_CALL_ACTIVATE = 0x8,
	TAPI_CALL_HOLD = 0x9,
	TAPI_CALL_SWAP = 0xA,
	TAPI_CALL_SETUP_CONFERENCE = 0xB,
	TAPI_CALL_SPLIT_CONFERENCE = 0xC,
	TAPI_CALL_TRANSFER = 0xD,
	TAPI_CALL_MODIFY_TYPE = 0xE,
	TAPI_CALL_REDIRECT = 0xF,
	TAPI_CALL_ACTIVATE_CCBS = 0x10, /* CCBS - Call Completion on Busy Subscriber */
	TAPI_CALL_RESPOND_RECALL = 0x11,
	TAPI_CALL_SEND_USER_INFO = 0x12,
	TAPI_CALL_SETUP_IND = 0x13,	
	TAPI_CALL_INCOMING_IND  = 0x14,
	TAPI_CALL_ALERT_IND = 0x15,
	TAPI_CALL_CONNECTED_IND = 0x16,
	TAPI_CALL_CONNECTING_IND = 0x17,	
	TAPI_CALL_END_IND = 0x18,
	TAPI_CALL_START_DTMF_CNF = 0x19, /* CNF is probably CoNFerence or CoNFig*/
	TAPI_CALL_STOP_DTMF_CNF = 0x1A,
	TAPI_CALL_HOLD_CNF = 0x1B,
	TAPI_CALL_SWAP_CNF = 0x1C,
	TAPI_CALL_ACTIVATE_CNF = 0x1D,
	TAPI_CALL_SETUP_CONFERENCE_CNF = 0x1E,
	TAPI_CALL_SPLIT_CONFERENCE_CNF = 0x1F,
	TAPI_CALL_TRANSFER_CNF = 0x20,
	TAPI_CALL_MODIFY_TYPE_CNF = 0x21,
	TAPI_CALL_FORWARD_IND = 0x22,
	TAPI_CALL_ACTIVATE_IND = 0x23,
	TAPI_CALL_HOLD_IND = 0x24,
	TAPI_CALL_TRANSFER_IND = 0x25,
	TAPI_CALL_SETUP_CONFERENCE_IND = 0x26,
	TAPI_CALL_BARRING_IND = 0x27,
	TAPI_CALL_WAITING_IND = 0x28,
	TAPI_CALL_CUG_INFO_IND = 0x29,
	TAPI_CALL_AOC_INFO_IND = 0x2A,
	TAPI_CALL_SS_NOTIFY_IND = 0x2B,
	TAPI_CALL_CALLING_NAME_INFO_IND = 0x2C,
	TAPI_CALL_REDIRECT_CNF = 0x2D,
	TAPI_CALL_CCBS_POSSIBLE_IND = 0x2E,
	TAPI_CALL_ACTIVATE_CCBS_CNF = 0x2F,
	TAPI_CALL_RECALL_IND = 0x30,
	TAPI_CALL_USER_INFO_IND = 0x31,
	TAPI_CALL_CONNECTED_NUMBER_IND = 0x32,
	TAPI_CALL_CLI_SUPRESSION_REJ_IND = 0x33,
	TAPI_CALL_ERROR_IND = 0x34,
};

enum TAPI_CALL_CALLTYPE
{
	TAPI_CALL_CALLTYPE_VOICE1 	= 0x0, 
	TAPI_CALL_CALLTYPE_VOICE2 	= 0x1, 
	TAPI_CALL_CALLTYPE_DATA 	= 0x3,
	TAPI_CALL_CALLTYPE_VOICE3 	= 0x8, 
	TAPI_CALL_CALLTYPE_VOICE4 	= 0x9,
	TAPI_CALL_CALLTYPE_VIDEO 	= 0xA,
	//types 2, 4, 5, 6, 7 are unknown, maybe these aren't used
};

enum TAPI_CALL_CONTEXT_TYPE
{
	TAPI_CALL_CONTEXT_TYPE_VOICE 	= 1, 
	TAPI_CALL_CONTEXT_TYPE_DATA 	= 2, 
	TAPI_CALL_CONTEXT_TYPE_VIDEO 	= 3, 
};

enum TAPI_CALL_STATE
{
	TAPI_CALL_STATE_IDLE 		= 0,
	TAPI_CALL_STATE_ORIG 		= 1, 
	TAPI_CALL_STATE_SETUPIND 	= 2, 
	TAPI_CALL_STATE_CONNECTING 	= 3, 
	TAPI_CALL_STATE_ALERT 		= 4, 
	TAPI_CALL_STATE_INCOM 		= 5, 
	TAPI_CALL_STATE_ANSWER 		= 6, 
	TAPI_CALL_STATE_CONV 		= 7, 
	TAPI_CALL_STATE_ENDING 		= 8,
	TAPI_CALL_STATE_END 		= 9, 
	TAPI_CALL_STATE_INVALID 	= 10, 
};
 
#endif
