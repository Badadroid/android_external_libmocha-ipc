/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2012 Dominik Marszk <dmarszk@gmail.com>
 *                    KB <kbjetdroid@gmail.com>
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

#ifndef __TAPI_NETWORK_H__
#define __TAPI_NETWORK_H__

#if defined(DEVICE_JET)
#include "device/jet/tapi_network.h"
#elif defined(DEVICE_WAVE)
#include "device/wave/tapi_network.h"
#endif

/**
 * All the TAPI Network context structures generic to all Mocha devices will be defined here
 */

//TODO: check if following definitions does apply to JET too, confirm it for WAVE
typedef struct {
	uint8_t state;
	uint8_t cause;
} __attribute__((__packed__)) tapiNetRegistrationFail;

#define NET_MAX_NAME_LEN 40 //it's kinda guess
#define NET_MAX_SPN_LEN 24 //same as above

typedef struct { //not sure if it's specific to WAVE or same for JET, 
//most likely sizeof should be 0x78 bytes for WAVE
	uint8_t serviceLevel;
	uint8_t serviceType;
	uint8_t psServiceType;
	uint8_t _unknown_; //its possible that here is whole systemId structure
	uint8_t systemType; //9 - unknown, ignored when offline mode, 7 - unknown
	uint16_t _unknown2_;
	uint8_t sysIdFormat;
	uint32_t networkMode;
	uint32_t systemId;
	uint32_t bForbidden;
	uint32_t bHome;
	uint32_t bEquivalent;
	char name[NET_MAX_NAME_LEN];
	char spn[NET_MAX_SPN_LEN];
	uint32_t bRoaming;
	uint32_t bDisplayPplmn;
	uint32_t bDisplaySpn;
	uint8_t _unknown3_[8]; //this can belong to NetworkInfo or registrationFail
	tapiNetRegistrationFail registrationFail;	
	uint8_t _unknown4_[6]; //this can belong to NetworkInfo or registrationFail
} __attribute__((__packed__)) tapiNetworkInfo;

typedef struct {
	/* sizeof should be 0x14 for WAVE */
	/* Hex values next to variables are corresponding Bada NV Integer ids */
	uint32_t bAutoSelection; /* 0x55C, PLMN search mode */
	uint32_t bPoweronGprsAttach; /* 0x1AC */
	uint8_t networkOrder; /* 0x55D, val: 0/1/2, Bada uses only 0-1 - GSM first/WCDMA first <or way around> */
	uint8_t serviceDomain; /* 0x1AD, val: 0/1/2/<maybe3> Bada uses 0-2 - Combined/PS/CS*/
	uint16_t align1; /* uninitialized */
	uint32_t networkMode; /* 0x55E */
	uint8_t subscriptionMode; /* hardcoded to 0 in PhoneShell */
	uint8_t bFlightMode; /* 0x16C, this is also called offlineMode */
	uint8_t unknown; /* hardcoded 2, seems unused by AMSS */
	uint8_t align2; /* uninitialized */
} __attribute__((__packed__)) tapiStartupNetworkInfo;

typedef struct {
//most likely sizeof should be 0x1C bytes for WAVE
	uint32_t bCellChanged;
	uint32_t bRACChanged;
	uint32_t bLACChanged;
	uint32_t bPLMNChanged;
	uint8_t cellId[4];
	uint8_t racId;
	uint8_t lacId[2];
	uint8_t plmnId[3];
	uint8_t cbchStatus;
	uint8_t _unknown_;
} __attribute__((__packed__)) tapiCellInfo;

typedef struct {
	uint8_t rxLevel;
	uint8_t rxQual; //always 0xFF?
	uint8_t unknown; //always 0x00?
} __attribute__((__packed__)) tapiRadioInfo;

enum TAPI_NETWORK_DLS_ADJ {
	TAPI_NETWORK_DLS_ADJ_NO_ADJUSTMENT = 0,
	TAPI_NETWORK_DLS_ADJ_PLUS_ONE_HOUR = 1,
	TAPI_NETWORK_DLS_ADJ_PLUS_TWO_HOURS = 2,
};

typedef struct {
	uint32_t bNetworkInfoAvail; /* Le wild guess */
	uint32_t bNetworkTimeAvail;
	uint32_t bNetworkTimeZoneAvail;
	uint32_t bNetworkDLSAvail;
	uint8_t someName[40];
	uint8_t networkName[40];
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t tz; /* In quarters,  */
	uint8_t dls;
	uint8_t unknown3[4]; /* Unused or some system info */
	uint16_t mcc; /* low bytes of U32 systemId */
	uint16_t mnc; /* high bytes of U32 systemId */
} __attribute__((__packed__)) tapiNitzInfo;

typedef struct {
	tapiNetworkInfo networkInfo;
	uint8_t networkOrder;
} tapiNetwork;

enum TAPI_NETWORK_SYSTEM {
	TAPI_NETWORK_SYSTEM_UMTS = 7,
	TAPI_NETWORK_SYSTEM_GSM = 5,
};

typedef struct {
	uint8_t systemType;
	uint8_t unknown2;
	uint8_t unknown3[6];
	uint16_t MNC;
	uint16_t MCC;
	uint32_t bForbidden;
	uint32_t bCurrent;
	uint32_t bAvailable;
	char name[64];
} __attribute__((__packed__)) tapiNetSearchCnf;

void tapi_network_parser(uint16_t tapiNetworkType, uint32_t tapiNetworkLength, uint8_t *tapiNetworkData);


void tapi_network_init(void);
void tapi_network_startup(tapiStartupNetworkInfo* network_startup_info);
void tapi_network_shutdown(uint8_t mode);
void tapi_set_offline_mode(uint8_t mode);
void tapi_network_select(tapiNetSearchCnf* net_select);
void tapi_network_reselect(uint8_t mode);
void tapi_network_search(void);
void tapi_set_selection_mode(uint8_t mode);
void tapi_network_set_mode(uint32_t mode);
void tapi_set_subscription_mode(uint8_t mode);

void tapi_network_api_request(uint32_t tapiNetLength, uint8_t *tapiNetData);
void tapi_network_startup(tapiStartupNetworkInfo* network_startup_info);
void tapi_network_set_subscription_mode(uint32_t tapiNetLength, uint8_t *tapiNetData);
void tapi_network_radio_info(uint32_t tapiNetLength, uint8_t *tapiNetData);
void tapi_network_network_select_ind(uint32_t tapiNetLength, uint8_t *tapiNetData);
void tapi_network_common_error(uint32_t tapiNetLength, uint8_t *tapiNetData);
void tapi_network_cell_info(uint32_t tapiNetLength, uint8_t *tapiNetData);
void tapi_network_nitz_info_ind(uint32_t tapiNetLength, uint8_t *tapiNetData);

#endif
