/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2012 Dominik Marszk <dmarszk@gmail.com>
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
 
#ifndef __DEVICE_WAVE_SND_DATA_H__
#define __DEVICE_WAVE_SND_DATA_H__

#define SOUND_CFG_DATA_SIZE 0x19C
int32_t RCV_MSM_Data[] =	{	-1600	,	//# 1 Level  <RX_CALL_VOL>	1
					-1600	,	//# 2 Level
					-1200	,	//# 3 Level
					-1200	,	//# 4 Level
					-800	,	//# 5 Level
					-800	,	//# 6 Level
					-400	,	//# 7 Level
					-400	,	//# 8 Level
					0	,	//# 9 Level
					0	,	//# 10 Level
					400	,	//# 11 Level
					400	,	//# 12 Level
					800	,	//# 13 Level
					800	,	//# 14 Level
					800	,	//# 15 Level
					-3200	,	//# 1 Level <RX_DTMF_VOL>	1
					-3200	,	//# 2 Level
					-2900	,	//# 3 Level
					-2900	,	//# 4 Level
					-2600	,	//# 5 Level
					-2600	,	//# 6 Level
					-2300	,	//# 7 Level
					-2300	,	//# 8 Level
					-2000	,	//# 9 Level
					-2000	,	//# 10 Level
					-1800	,	//# 11 Level
					-1800	,	//# 12 Level
					-1600	,	//# 13 Level
					-1600	,	//# 14 Level
					-1600	,	//# 15 Level
					5	,	//# Tx Voice Volume <CODEC_GAIN>	1>
					-12	,	//# Tx DTMF gain
					5	,	//# CODEC Tx gain
					2	,	//# CODEC Rx gain
					-84	,	//# CODEC sidetone gain
					2	,	//# Whisper Tx Vol
					1	,	//# 0=OFF, 1=ESEC, 2=HEADSET, 3=AEC, 4=SPK, 5=BT HEADSET <ECHO_CANCELLER> 1
					0	,	//0x0000		# tx_agc_OnOff(On:0x0001, Off:0x0000) <TX_AGC> 1
					8192	,	//0x2000		# tx_agc_static_gain 
					0	,	//0x0000		# tx_agc_aig
					3840	,	//0x0F00		# tx_agc_exp_thres
					65296	,	//0xFF10		# tx_agc_exp_slope
					7680	,	//0x1E00		# tx_agc_compr_thres
					40960	,	//0xA000		# tx_agc_compr_slope
					0	,	//0x0001		# rx_agc_OnOff(On:0x0001, Off:0x0000)<RX_AGC> 1
					8192	,	//0x2000		# rx_agc_static_gain 
					0	,	//0x0000		# rx_agc_aig
					3840	,	//0x0F00		# rx_agc_exp_thres
					65296	,	//0xFF10		# rx_agc_exp_slope
					8320	,	//0x2080		# rx_agc_compr_thres
					40960	,	//0xA000		# rx_agc_compr_slopec
					64625	,	//0xFC71<TX_FILTER>	1
					64614	,	//0xFC66, 
					64225	,	//0xFAE1, 
					64980	,	//0xFDD4, 
					64640	,	//0xFC80, 
					65163	,	//0xFE8B, 
					20814	,	//0x514E, 
					874	,	//0x036a,<RX_FILTER>	1 
					64180	,	//0xfab4, 
					63966	,	//0xf9de, 
					2627	,	//0x0a43, 
					2796	,	//0x0aec, 
					60096	,	//0xeac0, 
					21572	,	//0x5444,
					32767	,	//		32767	0x7FFF
					2048	,	//		2048	0x800
					32767	,	//		32767	0x7FFF
					5279	,	//	 	5279	0x149F
					0	,	//	 	0	0x0000
					500	,	//		500	0x01F4
					2048	,	//		2048	0x800
					8192	,	//		8192	0x2000
					8192	,	//		8192	0x2000
					250	,	//	 	250	0xFA
					70	,	//		70	0x46
					2	,	//		2 	0x2
					767	,	//	 	767	0x2FF
					64	,	//		64	0x40
					32	,	//	 	32	0x20
					18000	,	//	 	18000	0x4650
					64	,	//	 	64	0x40
					16800	,	//	 	16800	0x41A0
					2048	,	//	 	2048	0x800
					99	,	//	 	99	0x63
					20000	,	//	 	20000	0x4E20
					20000	,	//	 	20000	0x4E20
					2	,	//	 	2	0x2
					12288	,	//		12288 	0x3000
					5376	,	//	 	5376	0x1500
					0	,	//		0 	0x0
					144	,	//	 	144	0x90
					256	,	//	 	256	0x100
					1024	,	//	 	1024	0x400
					512	,	//	 	512	0x200
					1024	,	//	 	1024	0x400
					768	,	//	 	768	0x300
					600	,	//	 	600	0x258
					400	,	//	 	400	0x190
					2900	,	//		2900	0x0B54
					128	,	//	 	128	0x80
					12000	,	//	 	12000	0x2EE0
					4000}; 		//		4000	0x0FA0
int32_t EAR_MSM_Data[] =	{	-1000	,	//# 1 Level <RX_CALL_VOL>	1
					-1000	,	//# 2 Level
					-700	,	//# 3 Level
					-700	,	//# 4 Level
					-400	,	//# 5 Level
					-400	,	//# 6 Level
					-100	,	//# 7 Level
					-100	,	//# 8 Level
					200	,	//# 9 Level
					200	,	//# 10 Level
					500	,	//# 11 Level
					500	,	//# 12 Level
					800	,	//# 13 Level
					800	,	//# 14 Level
					800	,	
					-3200	,	//# 1 Level <RX_DTMF_VOL>	1
					-3200	,	//# 2 Level
					-3100	,	//# 3 Level
					-3100	,	//# 4 Level
					-3000	,	//# 5 Level
					-3000	,	//# 6 Level
					-2900	,	//# 7 Level
					-2900	,	//# 8 Level
					-2800	,	//# 9 Level
					-2800	,	//# 10 Level
					-2700	,	//# 11 Level
					-2700	,	//# 12 Level
					-2600	,	//# 13 Level
					-2600	,	//# 14 Level
					-2600	,	//# 15 Level
					11	,	//# Tx Voice Volume <CODEC_GAIN>	1
					-12	,	//# Tx DTMF gain
					7	,	//# CODEC Tx gain
					5	,	//# CODEC Rx gain
					-84	,	//# CODEC sidetone gain
					2	,	//# Whisper Tx Vol
					2	,	//# 0=OFF, 1=ESEC, 2=HEADSET, 3=AEC, 4=SPK, 5=BT HEADSET <ECHO_CANCELLER> 1
					0	,	//0x0000		# tx_agc_OnOff(On:0x0001, Off:0x0000)  <TX_AGC> 1
					8192	,	//0x2000		# tx_agc_static_gain 
					0	,	//0x0000		# tx_agc_aig
					1792	,	//0x0700		# tx_agc_exp_thres
					65408	,	//0xFF80		# tx_agc_exp_slope
					9600	,	//0x2580		# tx_agc_compr_thres
					32768	,	//0x8000		# tx_agc_compr_slope
					1	,	//0x0001		# rx_agc_OnOff(On:0x0001, Off:0x0000) <RX_AGC> 1
					8192	,	//0x2000		# rx_agc_static_gain 
					0	,	//0x0000		# rx_agc_aig
					2816	,	//0x0B00		# rx_agc_exp_thres
					65317	,	//0xFF25		# rx_agc_exp_slope
					8320	,	//0x2080		# rx_agc_compr_thres
					34048	,	//0x8500		# rx_agc_compr_slopec
					532	,	//0x0214, <TX_FILTER>	1
					65342	,	//0xFF3E, 
					65216	,	//0xFEC0, 
					64637	,	//0xFC7D, 
					63358	,	//0xF77E, 
					64864	,	//0xFD60, 
					17635	,	//0x44E3, 
					31	,	//0x001F, <RX_FILTER>	1
					96	,	//0x0060, 
					77	,	//0x004D, 
					33	,	//0x0021, 
					65259	,	//0xFEEB, 
					65118	,	//0xFE5E, 
					17793	,	//0x4581, 
					32767	,	//		32767	0x7FFF
					2048	,	//		2048	0x800
					32767	,	//		32767	0x7FFF
					4255	,	//		4255	0x109F
					0	,	//		0	0x0
					500	,	//		500	0x01F4
					2048	,	//		2048	0x800
					8192	,	//		8192	0x2000
					8192	,	//		8192	0x2000
					512	,	//		512	0x200
					60	,	//		60	0x3C
					1	,	//		1	0x1
					767	,	//		767	0x2FF
					64	,	//		64	0x40
					32	,	//		32	0x20
					10000	,	//		10000	0x2710
					64	,	//		64	0x40
					16800	,	//		16800	0x41A0
					2048	,	//		2048	0x800
					99	,	//		99	0x63
					20000	,	//		20000	0x4E20
					20000	,	//		20000	0x4E20
					1	,	//		1	0x1
					14000	,	//		14000	0x36B0
					2000	,	//		2000	0x7D0
					0	,	//		0	0x0
					100	,	//		100	0x64
					228	,	//		228	0xE4
					1024	,	//		1024	0x400
					512	,	//		512	0x200
					512	,	//		512	0x200
					768	,	//		768	0x300
					512	,	//		512	0x200
					400	,	//		400	0x190
					3000	,	//		3000	0xBB8
					600	,	//		600	0x258
					13000	,	//		13000	0x32C8
					4000	};	//		4000	0xFA0
int32_t SPK_MSM_Data[] =	{	-1600	,	//# 1 Level <RX_CALL_VOL>	1
					-1600	,	//# 2 Level
					-1350	,	//# 3 Level
					-1350	,	//# 4 Level
					-1100	,	//# 5 Level
					-1100	,	//# 6 Level
					-850	,	//# 7 Level
					-850	,	//# 8 Level
					-600	,	//# 9 Level
					-600	,	//# 10 Level
					-300	,	//# 11 Level
					-300	,	//# 12 Level
					0	,	//# 13 Level
					0	,	//# 14 Level
					0	,	
					-4200	,	//# 1 Level <RX_DTMF_VOL>	1
					-4000	,	//# 2 Level
					-3800	,	//# 3 Level
					-3600	,	//# 4 Level
					-3400	,	//# 5 Level
					-3200	,	//# 6 Level
					-3000	,	//# 7 Level
					-2800	,	//# 8 Level
					-2600	,	//# 9 Level
					-2400	,	//# 10 Level
					-2200	,	//# 11 Level
					-2000	,	//# 12 Level
					-1800	,	//# 13 Level
					-1600	,	//# 14 Level
					-1600	,	//# 15 Level
					7       ,	//# Tx Voice Volume <CODEC_GAIN>	1
					-12	,	//# Tx DTMF gain
					9	,	//# CODEC Tx gain
					12	,	//# CODEC Rx gain
					-84	,	//# CODEC sidetone gain
					6	,	//# Whisper Tx Vol
					4	,	//# 0=OFF, 1=ESEC, 2=HEADSET, 3=AEC, 4=SPK, 5=BT HEADSET <ECHO_CANCELLER> 1
					1	,	//0x0001		# tx_agc_OnOff(On:0x0001, Off:0x0000) <TX_AGC> 1
					9191	,	//0x23E7		# tx_agc_static_gain 
					0	,	//0x0000		# tx_agc_aig
					3200	,	//0x0C80		# tx_agc_exp_thres
					65344	,	//0xFF40		# tx_agc_exp_slope
					8448	,	//0x2100		# tx_agc_compr_thres
					32768	,	//0x8000		# tx_agc_compr_slope
					1	,	//0x0001		# rx_agc_OnOff(On:0x0001, Off:0x0000) <RX_AGC> 1
					9191	,	//0x23E7		# rx_agc_static_gain 
					0	,	//0x0000		# rx_agc_aig
					2816	,	//0x0B00		# rx_agc_exp_thres
					65392	,	//0xFF70		# rx_agc_exp_slope
					7680	,	//0x1E00		# rx_agc_compr_thres
					40960	,	//0xA000		# rx_agc_compr_slopec
					295	,	//0x0127,<TX_FILTER>	1
					65513	,	//0xffe9,
					65263	,	//0xfeef,
					64786	,	//0xfd12,
					65188	,	//0xfea4,
					65420	,	//0xff8c,
					17026	,	//0x4282,
					0	,	//0x0000, <RX_FILTER>	1
					0	,	//0x0000, 
					0	,	//0x0000, 
					0	,	//0x0000, 
					0	,	//0x0000, 
					0	,	//0x0000, 
					0	,	//0x0000, 
					32767	,	//		32767	0x7FFF
					2048	,	//		2048	0x800
					32767	,	//		32767	0x7FFF
					6047	,	//		6047	0x179F
					0	,	//		0	0x0
					500	,	//		500	0x1F4
					2048	,	//		2048	0x800
					8192	,	//		8192	0x2000
					8192	,	//		8192	0x2000
					288	,	//		288	0x120
					140	,	//		140	0x8C
					1	,	//		1	0x1
					400	,	//		400	0x190
					64	,	//		64	0x40
					32	,	//		32	0x20
					14000	,	//		14000	0x36B0
					64	,	//		64	0x40
					16800	,	//		16800	0x41A0
					2048	,	//		2048	0x800
					99	,	//		99	0x63
					20000	,	//		20000	0x4E20
					20000	,	//		20000	0x4E20
					1	,	//		1	0x1
					12288	,	//		12288	0x3000
					4096	,	//		4096	0x1000
					0	,	//		0	0x0
					272	,	//		272	0x110
					256	,	//		256	0x100
					7680	,	//		7680	0x1E00
					512	,	//		512	0x200
					1024	,	//		1024	0x400
					768	,	//		768	0x300
					486	,	//		486	0x1E6
					400	,	//		400	0x190
					10349	,	//		10349	0x286D
					512	,	//		512	0x200
					13000	,	//		13000	0x32C8
					4000	};	//		4000	0xFA0
int32_t BTH_MSM_Data[] =	{	500	,	//# 1 Level <RX_CALL_VOL>	1
					500	,	//# 2 Level
					500	,	//# 3 Level
					500	,	//# 4 Level
					500	,	//# 5 Level
					500	,	//# 6 Level
					500	,	//# 7 Level
					500	,	//# 8 Level
					500	,	//# 9 Level
					500	,	//# 10 Level
					500	,	//# 11 Level
					500	,	//# 12 Level
					500	,	//# 13 Level
					500	,	//# 14 Level
					500	,	//# 15 Level
					-3000	,	//# 1 Level <RX_DTMF_VOL>	1
					-2600	,	//# 2 Level
					-2400	,	//# 3 Level
					-2400	,	//# 4 Level
					-2200	,	//# 5 Level
					-2200	,	//# 6 Level
					-2000	,	//# 7 Level
					-2000	,	//# 8 Level
					-1800	,	//# 9 Level
					-1800	,	//# 10 Level
					-1600	,	//# 11 Level
					-1600	,	//# 12 Level
					-1400	,	//# 13 Level
					-1400	,	//# 14 Level
					-1400	,	//# 15 Level
					4	,	//	# Tx Voice Volume <CODEC_GAIN>	1
					-12	,	//	# Tx DTMF gain
					10	,	//	# CODEC Tx gain
					0	,	//	# CODEC Rx gain
					-84	,	//	# CODEC sidetone gain
					6	,	//	# Whisper Tx Vol
					5	,	//	# 0=OFF, 1=ESEC, 2=HEADSET, 3=AEC, 4=SPK, 5=BT HEADSET<ECHO_CANCELLER> 1
					1	,	//0x0001		# tx_agc_OnOff(On:0x0001, Off:0x0000) <TX_AGC> 1
					14567	,	//0x38E7		# tx_agc_static_gain 
					0	,	//0x0000		# tx_agc_aig
					3600	,	//0x0E10		# tx_agc_exp_thres
					65328	,	//0xFF30		# tx_agc_exp_slope
					9600	,	//0x2580		# tx_agc_compr_thres
					32768	,	//0x8000		# tx_agc_compr_slope
					1	,	//0x0001		# rx_agc_OnOff(On:0x0001, Off:0x0000) <RX_AGC> 1
					8192	,	//0x2000		# rx_agc_static_gain 
					0	,	//0x0000		# rx_agc_aig
					4480	,	//0x1180		# rx_agc_exp_thres
					65360	,	//0xFF50		# rx_agc_exp_slope
					9600	,	//0x2580		# rx_agc_compr_thres
					32768	,	//0x8000		# rx_agc_compr_slopec
					0	,	//0x0000, <TX_FILTER>	1
					0	,	//0x0000, 
					0	,	//0x0000, 
					0	,	//0x0000, 
					0	,	//0x0000, 
					0	,	//0x0000, 
					0	,	//0x0000, 
					65523	,	//0xFFF3,<RX_FILTER>	1 
					65476	,	//0xFFC4, 
					65241	,	//0xFED9, 
					64827	,	//0xFD3B, 
					64717	,	//0xFCCD, 
					65169	,	//0xFE91, 
					16355	,	//0x3FE3, 
					32767	,	//		32767	0x7FFF
					2048	,	//		2048	0x800
					8667	,	//		8667	0x21DB
					4255	,	//		4255	0x109F
					0	,	//		0	0x0
					120	,	//		120	0x78
					2048	,	//		2048	0x800
					8192	,	//		8192	0x2000
					8192	,	//		8192	0x2000
					420	,	//		420	0x1A4
					200	,	//		200	0xC8
					1	,	//		1	0x1
					400	,	//		400	0x190
					64	,	//		64	0x40
					32	,	//		32	0x20
					14711	,	//		14711	0x3977
					64	,	//		64	0x40
					16522	,	//		16522	0x408A
					2048	,	//		2048	0x800
					99	,	//		99	0x63
					20000	,	//		20000	0x4E20
					20000	,	//		20000	0x4E20
					1	,	//		1	0x1
					24000	,	//		24000	0x5DC0
					12000	,	//		12000	0x2EE0
					0	,	//		0	0x0
					262	,	//		262	0x106
					353	,	//		353	0x161
					7680	,	//		7680	0x1E00
					700	,	//		700	0x2BC
					6418	,	//		6418	0x1912
					768	,	//		768	0x300
					600	,	//		600	0x258
					400	,	//		400	0x190
					7336	,	//		7336	0x1CA8
					768	,	//		768	0x300
					11605	,	//		11605	0x2D55
					256	};	//		256	0x100

#endif
