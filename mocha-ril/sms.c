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

void ipc_sms_send_status(void* data)
{
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

void ril_request_send_sms(RIL_Token t, void *data, size_t size)
{
	tapiNettextInfo* mess;
	char **values = NULL;
	unsigned char *smsc = NULL;
	unsigned char *pdu = NULL;
	size_t pdu_size;
	char *message = NULL;
	int pdu_type, pdu_tp_da_index, pdu_tp_da_len, pdu_tp_udl_index, pdu_tp_ud_len, pdu_tp_ud_index, send_msg_type;

	if (data == NULL || size < 2 * sizeof(char *))
		goto error;

	values = (char **) data;
	if (values[1] == NULL)
		goto error;

	ALOGD("%s: PDU: %s", __func__, values[1]);

	pdu_size = string2data_size(values[1]);
	if (pdu_size == 0)
		goto error;

	pdu = string2data(values[1]);
	if (pdu == NULL)
		goto error;

	if (values[0] != NULL) {
		smsc = string2data(values[0]);
		if (smsc == NULL)
			goto error;
	} else {
		smsc = string2data(ril_data.smsc_number);
		if (smsc == NULL)
			goto error;
	}

	/* PDU parsing */
	pdu_type = pdu[0];
	ALOGD("%s: PDU Type is 0x%x", __func__, pdu_type);

	pdu_tp_da_index = 2;
	pdu_tp_da_len = pdu[pdu_tp_da_index];
	ALOGD("%s: PDU TP-DA Len is 0x%x", __func__, pdu_tp_da_len);

	pdu_tp_udl_index = pdu_tp_da_index + pdu_tp_da_len / 2 + 4;
	if (pdu_tp_da_len % 2 > 0)
		pdu_tp_udl_index += 1;
	pdu_tp_ud_len = pdu[pdu_tp_udl_index];
	ALOGD("%s: PDU TP-UD Len is 0x%x", __func__, pdu_tp_ud_len);

	pdu_tp_ud_index = pdu_tp_udl_index + 1;

	if (pdu_type == 0x41 || pdu_type == 0x61) {
		ALOGD("%s: We are sending a multi-part message!",  __func__);
		ALOGD("%s: We are sending message %d on %d", __func__, pdu[pdu_tp_ud_index + 5], pdu[pdu_tp_ud_index + 4]);
		send_msg_type = 1; //multi-part
	}
	else
		send_msg_type = 0;

	/* Convert PDU to tapiNettextInfo structure */

	mess = (tapiNettextInfo *)calloc(1, sizeof(*mess));

	mess->NPI_ToNumber = 0x01;

	if (pdu[pdu_tp_da_index + 1] == 0x91)
		mess->TON_ToNumber= 0x01;

	mess->lengthToNumber = pdu_tp_da_len;

	if (pdu_tp_da_len % 2 > 0)
		pdu_tp_da_len = pdu_tp_da_len + 1;

	bcd2ascii(mess->szToNumber, pdu + pdu_tp_da_index + 2, pdu_tp_da_len / 2);

	mess->scTime = time(NULL);

	mess->NPI_SMSC = 0x01;

	if (smsc[1] == 0x91)
		mess->TON_SMSC = 0x01;

	mess->lengthSMSC = (smsc[0] - 1) * 2;

	bcd2ascii(mess->SMSC, smsc + 2, smsc[0] - 1);

	if (pdu_type == 0x21 || pdu_type == 0x61)
		mess->bSRR = 0x01;

	mess->validityValue = 0xFF;
	mess->classType = 0x04;

	if (send_msg_type == 1) {
		mess->nUDH = 0x01; //multipart SMS
		mess->bUDHI = 0x01;
		mess->messageLength = pdu_tp_ud_len - 1;
	}
	else
		mess->messageLength = pdu_tp_ud_len;

	if (pdu[pdu_tp_udl_index - 1] == 8) {
		DEBUG_I("%s : DCS - Unicode", __func__);
		mess->alphabetType = 0x03; //Unicode
		if (send_msg_type == 0)
			memcpy(mess->messageBody, pdu + pdu_tp_ud_index, pdu_tp_ud_len);
		else
			memcpy(mess->messageBody, pdu + pdu_tp_ud_index + 1, pdu_tp_ud_len - 1);
	} else {
		DEBUG_I("%s : DCS - GSM7", __func__);
		mess->alphabetType = 0x00; //GSM7
		gsm72ascii(pdu + pdu_tp_ud_index, &message, pdu_size - pdu_tp_ud_index);
		if (send_msg_type == 0)	{
			memcpy(mess->messageBody, message, pdu_tp_ud_len);
		} else {
			mess->messageLength = mess->messageLength - 1;
			memcpy(mess->messageBody, pdu + pdu_tp_ud_index + 1, 5);
			memcpy(mess->messageBody + 5, message + 7, pdu_tp_ud_len - 7);
		}
	}
	tapi_nettext_set_net_burst(0);
	tapi_nettext_send((uint8_t *)mess);
	ril_data.tokens.outgoing_sms = t;

	free(mess);
	free(pdu);
	free(smsc);

	if(message != NULL)
		free(message);

	return;

error:

	ril_request_complete(t, RIL_E_SMS_SEND_FAIL_RETRY, NULL, 0);
}

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
		if (nettextInfo->nUDH == 1 || nettextInfo->msgType == 0x10) {
			pdu_type = "44";
		}
		else {
			pdu_type = "04";
		}
		DEBUG_I("%s : pdu_type = %s", __func__, pdu_type);		
		strcat(pdu, pdu_type);
	} else {
		pdu_type = "06";
		DEBUG_I("%s : pdu_type = %s", __func__, pdu_type);		
		strcat(pdu, pdu_type);
		strcat(pdu, "00");
	}

	// TP-OA: TP- Originating-Address
	//Convert nettextInfo->phoneNumber to TP-OA

	number_oa = nettextInfo->szFromNumber;

	if (nettextInfo->TON_FromNumber == 5 )
	{
		len_mess = ascii2gsm7(number_oa, (unsigned char **)&number_tmp);
		number2 = data2string((unsigned char *)number_tmp, strlen(number_tmp));

		tp_oa = malloc(strlen(number2)  + 5);
		memset(tp_oa, 0, strlen(number2) + 5);

		asprintf(&len_char, "%02X", strlen(number2));
		strcat(tp_oa, len_char);
		strcat(tp_oa, "D0");
		strcat(tp_oa, number2);
		DEBUG_I("%s : tp_oa = %s", __func__, tp_oa);		
	} else {
		len_oa = strlen(number_oa);

		if ((strlen(number_oa) % 2) > 0)
			strcat(number_oa, "F");

		number2 = malloc(strlen(number_oa) + 1);
		memset(number2, 0, strlen(number_oa) + 1);

		i = 0;
		while (i < strlen(number_oa)) {
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

	if (nettextInfo->dischargeTime == 0x00) {
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
	while (i < 14) {
		a = &(buf[i+1]);
		strncat(tp_scts, a, 1);
		b = &(buf[i]);
		strncat(tp_scts, b, 1);
		i = i + 2;
	}

	DEBUG_I("%s : scTime = %s", __func__, tp_scts);

	if (nettextInfo->dischargeTime != 0x00) {
		strcat(pdu, tp_scts);
		memset(tp_scts, 0, 15);
		l_time = nettextInfo->dischargeTime;
		strftime(buf, sizeof(buf), "%y%m%d%H%M%S", gmtime(&l_time));
		asprintf(&a, "%02d", nettextInfo->time_zone);
		strcat(buf, a);
		i = 0;
		while (i < 14) {
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

	message = calloc((message_length * 2) + 3, sizeof(*message));

	i = 0;

	if (nettextInfo->nUDH == 1) {
		strcat(message, "05");
		message_length += 1;
	}

	while (i < nettextInfo->messageLength) {
		sprintf(c, "%02X",mess[i]);
		strcat(message, c);
		i++;
	}

	if (nettextInfo->alphabetType == 3 || nettextInfo->msgType == 0x10) {
		/*TP-DCS: TP-Data-Coding-Scheme */
		if (nettextInfo->msgType == 0x10) {
			DEBUG_I("%s : TP-DCS = ASCII (MMS receiving)", __func__);
			dcs = 0x04;
		}
		else {
			DEBUG_I("%s : TP-DCS = Unicode", __func__);
			dcs = 0x08; //Unicode
		}

		tp_ud = calloc(strlen(message) + 2, sizeof(*tp_ud));

		strcat(tp_ud,message);
	} else {
		/*TP-DCS: TP-Data-Coding-Scheme */
		dcs = 0x00; //gsm7
		DEBUG_I("%s : TP-DCS = GSM7", __func__);

		if (nettextInfo->nUDH == 1) {
			message_bin = malloc(strlen(message) + 3);
			memset(message_bin, 0, strlen(message) + 3);

			strcat(message_bin, "0000000");
			strcat(message_bin, (char *)(mess + 5));

			len_mess = ascii2gsm7(message_bin, (unsigned char **)&message_tmp);
			tp_ud = data2string((unsigned char *)message_tmp, len_mess);

			i = 0;
			while (i < 12) {
				tp_ud[i] = message[i];
				i++;
			}

			message_length += 1;

			if (message_bin != NULL)
				free(message_bin);
		}
		else
		{
			len_mess = ascii2gsm7((char *)mess, (unsigned char **)&message_tmp);
			tp_ud = data2string((unsigned char *)message_tmp, len_mess);
		}
	}

	DEBUG_I("%s : tp_ud = %s", __func__, tp_ud);


	if (nettextInfo->bFlash == 1 && nettextInfo->classType == 0)
		dcs += 0x10;

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

void ril_request_send_sms_expect_more(RIL_Token t, void *data, size_t length)
{
	// No particular treatment here, we already have a queue
	ril_request_send_sms(t, data, length);
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


