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

#ifndef __DEVICE_WAVE_SS_H__
#define __DEVICE_WAVE_SS_H__

/**
 * All the TAPI SS context structures specific to Wave will be defined here
 */
enum TAPI_SS_TYPE
{
	TAPI_SS_BARRING_ACT_REQ 		= 0x5C,
	TAPI_SS_BARRING_DEACT_REQ 		= 0x5D,
	TAPI_SS_BARRING_CHANGE_PW_REQ 		= 0x5E,
	TAPI_SS_BARRING_QUERY_STATUS_REQ 	= 0x5F,
	TAPI_SS_FORWARD_ACT_REQ 		= 0x60,
	TAPI_SS_FORWARD_DEACT_REQ 		= 0x61,
	TAPI_SS_FORWARD_REG_REQ 		= 0x62,
	TAPI_SS_FORWARD_DEREG_REQ 		= 0x63,
	TAPI_SS_FORWARD_QUERY_STATUS_REQ 	= 0x64,
	TAPI_SS_WAITING_ACT_REQ 		= 0x65,
	TAPI_SS_WAITING_DEACT_REQ 		= 0x66,
	TAPI_SS_WAITING_QUERY_STATUS_REQ 	= 0x67,
	TAPI_SS_SEND_USSD_STRING_REQ 		= 0x68,
	TAPI_SS_GET_PASSWORD_RSP 		= 0x69,
	TAPI_SS_BARRING_ACT_CNF 		= 0x6A,
	TAPI_SS_BARRING_DEACT_CNF 		= 0x6B,
	TAPI_SS_BARRING_CHANGE_PW_CNF 		= 0x6C,
	TAPI_SS_BARRING_QUERY_STATUS_CNF 	= 0x6D,
	TAPI_SS_FORWARD_ACT_CNF 		= 0x6E,
	TAPI_SS_FORWARD_DEACT_CNF 		= 0x6F,
	TAPI_SS_FORWARD_REG_CNF 		= 0x70,
	TAPI_SS_FORWARD_DEREG_CNF 		= 0x71,
	TAPI_SS_FORWARD_QUERY_STATUS_CNF 	= 0x72,
	TAPI_SS_WAITING_ACT_CNF 		= 0x73,
	TAPI_SS_WAITING_DEACT_CNF 		= 0x74,
	TAPI_SS_WAITING_QUERY_STATUS_CNF 	= 0x75,
	/* 0x76 unused? */
	TAPI_SS_COMMON_ERROR_IND 		= 0x77,
	TAPI_SS_REJECT_IND 			= 0x78,
	TAPI_SS_SS_GET_PW_IND 			= 0x79,
	TAPI_SS_BARRING_ACT_IND 		= 0x7A,
	TAPI_SS_BARRING_DEACT_IND 		= 0x7B,
	TAPI_SS_BARRING_CHANGE_PW_IND 		= 0x7C,
	TAPI_SS_BARRING_QUERY_STATUS_IND 	= 0x7D,
	TAPI_SS_FORWARD_ACT_IND 		= 0x7E,
	TAPI_SS_FORWARD_DEACT_IND 		= 0x7F,
	TAPI_SS_FORWARD_REG_IND 		= 0x80,
	TAPI_SS_FORWARD_DEREG_IND 		= 0x81,
	TAPI_SS_FORWARD_QUERY_STATUS_IND 	= 0x82,
	TAPI_SS_WAITING_ACTIVATION_IND 		= 0x83,
	TAPI_SS_WAITING_DEACTIVATION_IND 	= 0x84,
	TAPI_SS_WAITING_QUERY_STATUS_IND 	= 0x85,
	TAPI_SS_USSD_CNF 			= 0x86,
	TAPI_SS_USSD_IND 			= 0x87,
	TAPI_SS_USSD_RSP 			= 0x88,
	/* 0x89 unused? */
	TAPI_SS_IDENT_STATUS_IND 		= 0x8A,
	TAPI_SS_CNAP_IND 			= 0x8B,
};

#endif
