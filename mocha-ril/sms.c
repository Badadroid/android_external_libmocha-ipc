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
			ril_data.tokens.outgoing_sms = RIL_TOKEN_DATA_WAITING;
			return;

		default:
			DEBUG_I("%s : Message sending error  ", __func__);
			response.errorCode = 500;
			ril_request_complete(ril_data.tokens.outgoing_sms, RIL_E_GENERIC_FAILURE, &response, sizeof(response));
			ril_data.tokens.outgoing_sms = RIL_TOKEN_DATA_WAITING;
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

	if (data == NULL || size < 2 * sizeof(char *) || ril_data.tokens.outgoing_sms != RIL_TOKEN_DATA_WAITING)
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
	char *number_smsc, *number_oa, *number, *a, *b;
	char *tp_pid, *tp_ud, *message;
	char tp_scts[15], tp_dcs[3], tp_udl[3];
	char pdu[400], buf[50], c[3];
	char *message_tmp, *number_tmp;
	unsigned int i , len, message_length, dcs;
	time_t l_time;

	number_smsc = a = b = number = number_oa = tp_pid =
	tp_ud = message = message_tmp = number_tmp = NULL;

	memset(tp_scts, 0, sizeof(tp_scts));
	memset(tp_dcs, 0, sizeof(tp_dcs));
	memset(tp_udl, 0, sizeof(tp_udl));
	memset(pdu, 0, sizeof(pdu));
	memset(c, 0, sizeof(c));

	/* Convert sms packet to SMS-DELIVER PDU type  format
	SMS = SCA + TPDU */

	//SCA
	//Convert nettextInfo->serviceCenter to SCA

	number_smsc = nettextInfo->SMSC;
	if ((strlen(number_smsc) % 2) > 0)
		strcat(number_smsc, "F");

	if (strlen(number_smsc) > 0)
	{
		len =  (strlen(number_smsc) / 2 ) + 1;
		sprintf(c, "%02x", len);
		strcat(pdu, c);
		strcat(pdu, "91");

		i = 0;
		while (i < strlen(number_smsc)) {
			a = &(number_smsc[i+1]);
			strncat(pdu, a, 1);
			b = &(number_smsc[i]);
			strncat(pdu, b, 1);
			i = i + 2;
		}
	}
	else
		strcat(pdu, "00");

	DEBUG_I("%s : sca = %s", __func__, pdu);

	//TPDU

	/* Protocol Data Unit Type (PDU Type) */

	if (nettextInfo->dischargeTime == 0x00)
		if (nettextInfo->nUDH == 1 || nettextInfo->msgType == 0x10)
			strcat(pdu, "44");
		else
			strcat(pdu, "04");
	else
		strcat(pdu, "0600");

	/* TP-OA: TP- Originating-Address
		Convert nettextInfo->phoneNumber to TP-OA */

	number_oa = nettextInfo->szFromNumber;

	if (nettextInfo->TON_FromNumber == 5 ) {
		len = ascii2gsm7(number_oa, (unsigned char **)&number_tmp);
		number = data2string((unsigned char *)number_tmp, strlen(number_tmp));

		sprintf(c, "%02x", strlen(number));
		strcat(pdu, c);
		strcat(pdu, "D0");
		strcat(pdu, number);
	} else {
		len = strlen(number_oa);

		if ((len % 2) > 0)
			strcat(number_oa, "F");

		sprintf(c, "%02x", len);
		strcat(pdu, c);
		if (nettextInfo->TON_FromNumber == 1 )
			strcat(pdu, "91");
		else
			strcat(pdu, "81");

		i = 0;
		while (i < len) {
			a = &(number_oa[i+1]);
			strncat(pdu, a, 1);
			b = &(number_oa[i]);
			strncat(pdu, b, 1);
			i = i + 2;
		}
	}

	DEBUG_I("%s : sca + pdu_type + tp_oa = %s", __func__, pdu);

	if (number != NULL)
		 free (number);

	/* TP-PID : TP-Protocol-Identifier */

	if (nettextInfo->dischargeTime == 0x00) {
		tp_pid = "00";
		strcat (pdu, tp_pid);
	}

	/* TP-SCTS: TP-Service-Centre-Time-Stamp
		Convert nettextInfo->timestamp and nettextInfo->time_zone to TP-SCTS */

	l_time = nettextInfo->scTime;

	strftime(buf, sizeof(buf), "%y%m%d%H%M%S", gmtime(&l_time));
	sprintf(c, "%02d", nettextInfo->time_zone);
	strcat(buf, c);

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
		sprintf(c, "%02d", nettextInfo->time_zone);
		strcat(buf, c);

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

	/* TP-UD: TP-User Data
		Convert nettextInfo->messageBody to TP-UD */

	message_length = nettextInfo->messageLength;

	if (nettextInfo->alphabetType == 3 || nettextInfo->msgType == 0x10) {
		/*TP-DCS: TP-Data-Coding-Scheme */
		if (nettextInfo->msgType == 0x10) {
			DEBUG_I("%s : TP-DCS = ASCII (MMS receiving)", __func__);
			dcs = 0x04;
		} else {
			DEBUG_I("%s : TP-DCS = Unicode", __func__);
			dcs = 0x08;
		}
		message = data2string((char *)nettextInfo->messageBody,message_length);
		tp_ud = calloc(message_length * 2 + 3, sizeof(*message));
		if (nettextInfo->nUDH == 1) {
			strcat(tp_ud, "05");
			message_length += 1;
			strcat(tp_ud, message);
		}
		else
			strcat(tp_ud, message);
	} else {
		/*TP-DCS: TP-Data-Coding-Scheme */
		dcs = 0x00; //gsm7
		DEBUG_I("%s : TP-DCS = GSM7", __func__);

		if (nettextInfo->nUDH == 1) {
			message = calloc(message_length + 3, sizeof(*tp_ud));
			message_length += 2;
			strcat(message, "0000000");
			strncat(message, (char *)(nettextInfo->messageBody + 5), message_length);
			len = ascii2gsm7(message, (unsigned char **)&message_tmp);
			message_tmp[0] = 0x05;
			i = 0;
			while (i < 5) {
				message_tmp[i + 1] = nettextInfo->messageBody[i];
				i++;
			}
			tp_ud = data2string((unsigned char *)message_tmp, len);
		} else {
			len = ascii2gsm7((char *)nettextInfo->messageBody, (unsigned char **)&message_tmp);
			tp_ud = data2string((unsigned char *)message_tmp, len);
		}
	}

	DEBUG_I("%s : tp_ud = %s", __func__, tp_ud);

	if (nettextInfo->bFlash == 1 && nettextInfo->classType == 0)
		dcs += 0x10;

	sprintf(tp_dcs, "%02x", dcs);

	/* TP-UDL:TP-User-Data-Length */
	sprintf(tp_udl, "%02x", message_length);
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


