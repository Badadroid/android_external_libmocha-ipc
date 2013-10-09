/**
 * This file is part of mocha-ril.
 *
 * Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
 * Copyright (C) 2011 Paul Kocialkowski <contact@oaulk.fr>
 * Copyright (C) 2013 Nikolay Volkov <volk204@mail.ru>
 *
 * mocha-ril is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mocha-ril is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mocha-ril.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define LOG_TAG "RIL-Mocha-SMS"
#include <utils/Log.h>

#include "mocha-ril.h"
#include "util.h"
#include <tapi_nettext.h>


/**
 * Format conversion utils
 */

void ipc_sms_send_status(void* data)
{
	ALOGE("%s: test me!", __func__);
	tapiNettextCallBack* callBack = (tapiNettextCallBack*)(data);

	RIL_SMS_Response response;

	memset(&response, 0, sizeof(response));
	response.messageRef = 0;
	response.ackPDU = NULL;

	switch(callBack->status){
		case 0:
			DEBUG_I("%s : Message sent  ", __func__);
			response.errorCode = -1;
			ril_request_complete(ril_data.tokens.outgoing_sms, RIL_E_SUCCESS, &response, sizeof(response));
			return;

		default:
			DEBUG_I("%s : Message sending error  ", __func__);
			response.errorCode = 500;
			ril_request_complete(ril_data.tokens.outgoing_sms, RIL_E_GENERIC_FAILURE, &response, sizeof(response));
			return;
	}

}

/**
 * Outgoing SMS functions
 */

/**
 * In: RIL_REQUEST_SEND_SMS
 *   Send an SMS message.
 *
 * Out: TAPI_NETTEXT_SEND
 */
void ril_request_send_sms(RIL_Token t, void *data, size_t length)
{
	tapiNettextInfo* mess;
	char *pdu, *a, *message;
	unsigned char *smsc, *pdu_hex, *p, *message_tmp;
	int smsc_length, pdu_length, pdu_hex_length, i, numberLen, send_msg_type;
	a = pdu = message = NULL;
	smsc = pdu_hex = p = message_tmp = NULL;

	if (data == NULL || length < 2 * sizeof(char *))
		return;

	pdu = ((char **) data)[1];
	pdu_length = 0;

	if (pdu != NULL) {
		pdu_length = strlen(pdu) + 1;
		pdu = strdup(pdu);
	}

	/* We first need to get SMS SVC before sending the message */

	smsc_length = strlen((char *) ril_data.smsc_number);
	smsc = (unsigned char *) strdup((char *) ril_data.smsc_number);

	if (pdu == NULL || pdu_length <= 0 || smsc == NULL || smsc_length <= 0) {
		ALOGE("Provided PDU or SMSC is invalid! Aborting");
		ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
		return;
	}

	if ((pdu_length / 2 + smsc_length) > 0xfe) {
		ALOGE("PDU or SMSC too large, aborting");
		ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
		return;
	}

	pdu_hex_length = pdu_length % 2 == 0 ? pdu_length / 2 :	(pdu_length ^ 1) / 2;

	pdu_hex = calloc(1, pdu_hex_length);

	hex2bin(pdu, pdu_length, pdu_hex);

	send_msg_type = 0;

	/* PDU operations */

	int pdu_tp_da_index = 2;

	unsigned char pdu_tp_da_len = pdu_hex[pdu_tp_da_index];

	if (pdu_tp_da_len > 0xff / 2) {

		ALOGE("PDU TP-DA Len failed (0x%x)\n", pdu_tp_da_len);

		goto pdu_end;

	}

	ALOGD("PDU TP-DA Len is 0x%x\n", pdu_tp_da_len);

	int pdu_tp_udh_index = pdu_tp_da_index + pdu_tp_da_len / 2 + 5;

	if (pdu_tp_da_len % 2 > 0)
		pdu_tp_udh_index += 1;
		

	unsigned char pdu_tp_udh_len = pdu_hex[pdu_tp_udh_index];

	if (pdu_tp_udh_len > 0xff / 2 || pdu_tp_udh_len < 5) {

		ALOGE("PDU TP-UDH Len failed (0x%x)\n", pdu_tp_udh_len);

		goto pdu_end;

	}

	ALOGD("PDU TP-UDH Len is 0x%x\n", pdu_tp_udh_len);

	int pdu_tp_udh_num_index = pdu_tp_udh_index + 4;

	unsigned char pdu_tp_udh_num = pdu_hex[pdu_tp_udh_num_index];

	if (pdu_tp_udh_num > 0xf) {

		ALOGE("PDU TP-UDH Num failed (0x%x)\n", pdu_tp_udh_num);

		goto pdu_end;

	}

	int pdu_tp_udh_seq_index = pdu_tp_udh_index + 5;

	unsigned char pdu_tp_udh_seq = pdu_hex[pdu_tp_udh_seq_index];

	if (pdu_tp_udh_seq > 0xf || pdu_tp_udh_seq > pdu_tp_udh_num) {

		ALOGE("PDU TP-UDH Seq failed (0x%x)\n", pdu_tp_udh_seq);

		goto pdu_end;

	}

	ALOGD("We are sending message %d on %d\n", pdu_tp_udh_seq, pdu_tp_udh_num);

	if (pdu_tp_udh_num > 1) {

		ALOGD("We are sending a multi-part message!");

		send_msg_type = 1; //multi-part

	}


	pdu_end:

	DEBUG_I("%s : pdu : %s", __func__, pdu);

	numberLen = (uint8_t)pdu_tp_da_len;	

	mess = (tapiNettextInfo *)malloc(sizeof(tapiNettextInfo));
	memset(mess, 0, sizeof(tapiNettextInfo));

	mess->NPI_ToNumber= 0x01; // 01

	if (pdu_hex[pdu_tp_da_index + 1] == 0x91)
		mess->TON_ToNumber= 0x01; // 01 - international
	else
		mess->TON_ToNumber= 0x00; // 00 - national

	mess->lengthToNumber = numberLen;

	if (numberLen % 2 > 0)
		numberLen = numberLen + 1;
		
	i = 0;
	
	while (i < numberLen)
	{
		mess->szToNumber[i] = pdu[i + 9];
		if ( pdu[i + 8] != 'f')
		mess->szToNumber[i+1] =pdu[i + 8]; 	
		i = i + 2;		
	}


	mess->scTime = time(NULL);

	mess->NPI_SMSC = 0x01;

	if (smsc[2] == 0x39 && smsc [3] == 0x31)
		mess->TON_SMSC = 0x01; // 01 - international
	else
		mess->TON_SMSC= 0x00; // 00 - national

	if (smsc[smsc_length - 2] == 'f' || smsc[smsc_length - 2] == 'F')
		mess->lengthSMSC = smsc_length - 5;
	else
		mess->lengthSMSC = smsc_length - 4;

	i = 4;

	while (i < smsc_length)
	{
		mess->SMSC[i - 4] = smsc[i + 1];
		if ( smsc[i] != 'f')
		mess->SMSC[i - 3] =smsc[i]; 	
		i = i + 2;		
	}

	if (pdu_hex[0] == 0x21 || pdu_hex[0] == 0x61)
		mess->bSRR = 0x01;

	mess->validityValue = 0xFF; //FF

	mess->classType = 0x04; //04	

	if (send_msg_type == 1)
	{
		mess->nUDH = 0x01; //multipart SMS
		mess->bUDHI = 0x01;
		mess->messageLength = pdu_hex[(numberLen / 2) + 6] - 1;
	}
	else
		mess->messageLength = pdu_hex[(numberLen / 2) + 6];
	
	if (pdu_hex[(numberLen / 2) + 5] == 8)
	{
		DEBUG_I("%s : DCS - Unicode", __func__);
		mess->alphabetType = 0x03; //Unicode
		if (send_msg_type == 0)
		{
			int k = (numberLen / 2) + 7;
			for (i = 0; i < pdu_hex[(numberLen / 2) + 6]; i++)
				mess->messageBody[i] = pdu_hex[i + k];
		}
		else 
		{
			int k = (numberLen / 2) + 8;
			for (i = 0; i < pdu_hex[(numberLen / 2) + 6] - 1; i++)
				mess->messageBody[i] = pdu_hex[i + k];
		}	
	} else {
		DEBUG_I("%s : DCS - GSM7", __func__);
		mess->alphabetType = 0x00; //GSM7
		int k = (numberLen / 2) + 7;

		message_tmp = malloc(((pdu_hex[(numberLen / 2) + 6]) * 2) + 1);
		memset(message_tmp, 0, ((pdu_hex[(numberLen / 2) + 6]) * 2) + 1);
			for (i = 0; i < pdu_hex[(numberLen / 2) + 6]; i++)
				message_tmp[i] = pdu_hex[i + k];

		gsm72ascii(message_tmp, &message, pdu_hex[(numberLen / 2) + 6]);
		
		if (send_msg_type == 0)
		{

			for (i = 0; i < pdu_hex[(numberLen / 2) + 6]; i++)
				mess->messageBody[i] = message[i];
		}
		else
		{
			mess->messageLength = mess->messageLength - 1;
			for (i = 0; i < pdu_hex[(numberLen / 2) + 6] - 2; i++)
				mess->messageBody[i] = message[i + 2];
			for (i = 0; i < 5; i++)
				mess->messageBody[i] = pdu_hex[(numberLen / 2) + 8 + i];

		}

		free(message_tmp);

	}

	tapi_nettext_set_net_burst(0);
	tapi_nettext_send((uint8_t *)mess);
	
	ril_data.tokens.outgoing_sms = t;

	if (pdu != NULL)
		free(pdu);

	if (smsc != NULL)
		free(smsc);

	if (mess != NULL)	
		free(mess);

	if (pdu_hex != NULL)
		free(pdu_hex);

	if (message != NULL)
		free(message);
}


/**
 * Incoming SMS functions
 */

/**
 * In: IPC_INCOMING_SMS
 *   Message to notify an incoming message
 *
 * Out: RIL_UNSOL_RESPONSE_NEW_SMS or RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT
 *   Notify RILJ about the incoming message
 */

void ipc_incoming_sms(void* data)
{
	tapiNettextInfo* nettextInfo = (tapiNettextInfo*)(data);
	char *number_smsc, *da_len, *sca, *a, *b;
	char *number_oa, *tp_oa;
	char *pdu_type, *tp_pid, *tp_dcs, *tp_udl, *tp_ud, *message; 
	char tp_scts[15];
	char pdu[400];
	char c[3];
	char *number, *number2, *len_char;
	char *message_tmp, *number_tmp, *message_bin;
	uint8_t *mess, dcs;
	unsigned int i , len_sca, len_oa, message_length, len_mess;
	char buf[50];
	time_t l_time;

	number_smsc = da_len = sca = a = b = number = number_oa = tp_oa =
	pdu_type = tp_pid = tp_dcs = tp_udl = tp_ud = message =
	number2 = len_char = message_tmp = number_tmp = message_bin = NULL;

	memset(tp_scts, 0, sizeof(tp_scts));
	memset(pdu, 0, sizeof(pdu));
	memset(c, 0, sizeof(c));

	/* Convert sms packet to SMS-DELIVER PDU type  format
	SMS = SCA + TPDU */
	
	//SCA
	//Convert nettextInfo->serviceCenter to SCA
	
	number_smsc = nettextInfo->SMSC;
	if ((strlen(number_smsc) % 2) > 0) 
		strcat(number_smsc, "F");

	number = malloc(strlen(number_smsc) + 1);
	memset(number, 0, strlen(number_smsc) + 1);

	i = 0;	
	
	while (i < strlen(number_smsc)) 
	{
		a = &(number_smsc[i+1]);
		strncat(number, a, 1);
		b = &(number_smsc[i]);
		strncat(number, b, 1);
		i = i + 2;
	}

	sca = malloc(strlen(number) + 5);
	memset(sca, 0, strlen(number) + 5);

	len_sca =  (strlen(number) / 2 ) + 1;
	asprintf(&len_char, "%02X", len_sca);
	strcat(sca, len_char);
	strcat(sca, "91");
	strcat(sca, number);

	DEBUG_I("%s : sca = %s", __func__, sca);

	strcat (pdu, sca);

	if (number != NULL)	
		free (number);

	if (sca != NULL)	
		free (sca);

	len_char = NULL;
	

	//TPDU

	/* Protocol Data Unit Type (PDU Type) 
	SMS-DELIVER

	TP-MTI:   00
	TP-MMS:   04
	TP-SRI:   20
	TP-RP:    00
	TP-UDHI:  00		*/

	if (nettextInfo->dischargeTime == 0x00)
	{
		if (nettextInfo->nUDH == 1)
		{
			pdu_type = "44";
		}
		else 
		{
			pdu_type = "04";
		}
		DEBUG_I("%s : pdu_type = %s", __func__, pdu_type);		
		strcat (pdu, pdu_type);
	}

	else
	{
		pdu_type = "06";
		DEBUG_I("%s : pdu_type = %s", __func__, pdu_type);		
		strcat (pdu, pdu_type);
		strcat (pdu, "00");

	}



	// TP-OA: TP- Originating-Address
	//Convert nettextInfo->phoneNumber to TP-OA

	number_oa = nettextInfo->szFromNumber;

	if (nettextInfo->TON_FromNumber == 5 )
	{	
		ascii2gsm7(number_oa, (unsigned char **)&number_tmp, strlen(number_oa));

		number2 = malloc((strlen(number_tmp)* 2) + 1);
		memset(number2, 0, (strlen(number_tmp)* 2) + 1);
		
		bin2hex((unsigned char *)number_tmp, strlen(number_tmp), number2);

		tp_oa = malloc(strlen(number2)  + 5);
		memset(tp_oa, 0, strlen(number2) + 5);

		asprintf(&len_char, "%02X", strlen(number2));
		strcat(tp_oa, len_char);
		strcat(tp_oa, "D0");
		strcat(tp_oa, number2);
		DEBUG_I("%s : tp_oa = %s", __func__, tp_oa);		
	} else {
		len_oa =  strlen(number_oa);

		if ((strlen(number_oa) % 2) > 0) 
			strcat(number_oa, "F");

		number2 = malloc(strlen(number_oa) + 1);
		memset(number2, 0, strlen(number_oa) + 1);

		i = 0;	
		while (i < strlen(number_oa)) 
		{
			a = &(number_oa[i+1]);
			strncat(number2, a, 1);
			b = &(number_oa[i]);
			strncat(number2, b, 1);
			i = i + 2;
		}
	
		tp_oa = malloc(strlen(number2) + 5);
		memset(tp_oa, 0, strlen(number2) + 5);
		asprintf(&len_char, "%02X", len_oa);
		strcat(tp_oa, len_char);
			if (nettextInfo->TON_FromNumber == 1 )	
				strcat(tp_oa, "91");
			else
				strcat(tp_oa, "81");
		strcat(tp_oa, number2);
		DEBUG_I("%s : tp_oa = %s", __func__, tp_oa);
	}

	strcat (pdu, tp_oa);

	if (number2 != NULL)	
		 free (number2);

	if (tp_oa != NULL)	
		free (tp_oa);

	len_char = NULL;

	//TP-PID : TP-Protocol-Identifier 

	if (nettextInfo->dischargeTime == 0x00)
	{
		tp_pid = "00";
		strcat (pdu, tp_pid);
	}

	//TP-SCTS: TP-Service-Centre-Time-Stamp
	//Convert nettextInfo->timestamp and nettextInfo->time_zone to TP-SCTS
	
	l_time = nettextInfo->scTime;

	strftime(buf, sizeof(buf), "%y%m%d%H%M%S", gmtime(&l_time));

	asprintf(&a, "%02d", nettextInfo->time_zone);

	strcat(buf, a);

	i = 0;
	while (i < 14) 
	{
		a = &(buf[i+1]);
		strncat(tp_scts, a, 1);
		b = &(buf[i]);
		strncat(tp_scts, b, 1);
		i = i + 2;
	}

	DEBUG_I("%s : scTime = %s", __func__, tp_scts);

	if (nettextInfo->dischargeTime != 0x00)
	{
		strcat (pdu, tp_scts);
		
		memset(tp_scts, 0, 15);
	
		l_time = nettextInfo->dischargeTime;

		strftime(buf, sizeof(buf), "%y%m%d%H%M%S", gmtime(&l_time));

		asprintf(&a, "%02d", nettextInfo->time_zone);

		strcat(buf, a);

			i = 0;
		while (i < 14) 
		{
			a = &(buf[i+1]);
			strncat(tp_scts, a, 1);
			b = &(buf[i]);
			strncat(tp_scts, b, 1);
			i = i + 2;
		}

		DEBUG_I("%s : dischargeTime = %s", __func__, tp_scts);

		strcat (pdu, tp_scts);
		
		if (nettextInfo->statusReport == 0)
			strcat (pdu, "00"); 
		else 
			strcat (pdu, "01");

		DEBUG_I("%s : pdu = %s", __func__, pdu);
	
		ril_request_unsolicited(RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT, pdu, strlen(pdu));

		return;
	}
 
	//TP-UD: TP-User Data
	//Convert messageBody to TP-UD

	mess = nettextInfo->messageBody;
	message_length = nettextInfo->messageLength;

	message = malloc((message_length * 2) + 3);
	memset(message, 0, (message_length * 2) + 3);

	i = 0;

	if (nettextInfo->nUDH == 1)
	{
		strcat(message, "05");
		message_length += 1; 
	}
		
	while (i < nettextInfo->messageLength)
	{
		sprintf(c, "%02X",mess[i]);
		strcat(message, c);
		i++;
	}

	if (nettextInfo->alphabetType == 3) 
	{
		/*TP-DCS: TP-Data-Coding-Scheme */
		dcs = 0x08; //Unicode
		DEBUG_I("%s : TP-DCS = Unicode", __func__);

		tp_ud = malloc(strlen(message) + 2);
		memset(tp_ud, 0, strlen(message) + 2);

		strcat(tp_ud,message);
	} else {
		/*TP-DCS: TP-Data-Coding-Scheme */
		dcs = 0x00; //gsm7
		DEBUG_I("%s : TP-DCS = GSM7", __func__);

		if (nettextInfo->nUDH == 1)
		{
			message_bin = malloc(strlen(message) + 3);
			memset(message_bin, 0, strlen(message) + 3);

			strcat(message_bin, "0000000");
			strcat(message_bin, (char *)(mess + 5));

			len_mess = ascii2gsm7(message_bin, (unsigned char **)&message_tmp, strlen(message) + 2);

			tp_ud = malloc(len_mess  + 1);
			memset(tp_ud, 0, len_mess + 1);

			bin2hex((unsigned char *)(message_tmp), len_mess / 2, tp_ud);
			
			i = 0;
			while (i < 12)
			{
				tp_ud[i] = message[i];
				i++;
			}

			message_length += 1; 

			if (message_bin != NULL)	
				free(message_bin);
		}	
		else
		{
			len_mess = ascii2gsm7((char *)mess, (unsigned char **)&message_tmp, strlen(message));

			tp_ud = malloc(len_mess + 1);
			memset(tp_ud, 0, len_mess + 1);

			bin2hex((unsigned char *)message_tmp, len_mess / 2, tp_ud);		
		}
	}

	DEBUG_I("%s : tp_ud = %s", __func__, tp_ud);

	/*
	if (nettextInfo->bFlash == 1) 
		dcs += 0x10; */

	asprintf(&tp_dcs, "%02X", dcs);

	//TP-UDL:TP-User-Data-Length

	asprintf(&tp_udl, "%02X", message_length);
	DEBUG_I("%s : tp_udl = %s", __func__, tp_udl);

	strcat (pdu, tp_dcs);
	strcat (pdu, tp_scts);
	strcat (pdu, tp_udl);	
	strcat (pdu, tp_ud);

	DEBUG_I("%s : pdu = %s", __func__, pdu);
	
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_NEW_SMS, pdu, strlen(pdu));

	if (message != NULL)	
		free (message);

	if (tp_ud != NULL)	
		free (tp_ud);

	if (message_tmp != NULL)
		free (message_tmp);
}

void nettext_cb_setup(void)
{
	tapi_nettext_cb_settings cb_sett_buf;
	int i;

	cb_sett_buf.ext_cb = 0x0;
	cb_sett_buf.ext_cb_enable = 0x0;
	cb_sett_buf.enable_all_combined_cb_channels = 0x1;
	cb_sett_buf.combined_language_type = 0x0;
	cb_sett_buf.number_of_combined_cbmi = 0x367FFF;
	for (i = 0; i < 40; i++)
		cb_sett_buf.cb_info[i] = 0x0;
	tapi_nettext_set_cb_settings(&cb_sett_buf);
}


