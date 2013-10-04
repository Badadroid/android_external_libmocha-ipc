/**
 * This file is part of mocha-ril.
 *
 * Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
 * Copyright (C) 2011-2012 Paul Kocialkowski <contact@paulk.fr>
 * Copyright (C) 2013 Dominik Marszk <dmarszk@gmail.com>
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

#define LOG_TAG "RIL-Mocha-CALL"
#include <time.h>
#include <utils/Log.h>

#include "mocha-ril.h"
#include <tapi_call.h>

ril_call_context* find_active_call()
{
	int i;
	for(i = 0; i < MAX_CALLS; i++)
	{
		if(ril_data.calls[i] && ril_data.calls[i]->callId != 0xFFFFFFFF && 	ril_data.calls[i]->call_state == RIL_CALL_ACTIVE)
		{
			return ril_data.calls[i];
		}
	}
	ALOGE("Cannot find active call context!");
	return NULL;
}

ril_call_context* new_ril_call_context()
{
	int i;
	for(i = 0; i < MAX_CALLS; i++)
	{
		if(!ril_data.calls[i])
		{
			ril_data.calls[i] = calloc(1, sizeof(ril_call_context));
			return ril_data.calls[i];
		}
	}
	ALOGE("No free call contexts!");
	return NULL;
}

ril_call_context* find_ril_call_context(uint32_t callId)
{
	int i;
	for(i = 0; i < MAX_CALLS; i++)
	{
		if(ril_data.calls[i] && ril_data.calls[i]->callId == callId)
		{
			return ril_data.calls[i];
		}
	}
	ALOGE("Call context with Id %d not found!", callId);
	return NULL;
}

void release_ril_call_context(ril_call_context* ptr)
{
	int i;
	for(i = 0; i < MAX_CALLS; i++)
	{
		if(ril_data.calls[i] == ptr)
		{
			free(ril_data.calls[i]);
			ril_data.calls[i] = NULL;
			return;
		}
	}
	ALOGE("Tried to release non registered call context!");
}

void ipc_call_incoming(void* data)
{

	ALOGE("%s: Test me!", __func__);

	uint8_t newCallState = RIL_CALL_INCOMING;
	int i;
	for(i = 0; i < MAX_CALLS; i++)
	{
		if(ril_data.calls[i] && ril_data.calls[i]->callId != 0xFFFFFFFF && 	(ril_data.calls[i]->call_state == RIL_CALL_ACTIVE || ril_data.calls[i]->call_state == RIL_CALL_HOLDING))
		{
			newCallState = RIL_CALL_WAITING;
			break;
		}
	}


	ril_call_context* callCtxt = new_ril_call_context();
	tapiCallInfo* callInfo = (tapiCallInfo*)(data);
	
	ALOGE("ipc_call_incoming: Incoming call received from %s", callInfo->phoneNumber);
	ALOGE("ipc_call_incoming: callId = %d", callInfo->callId );
	
	if(!callCtxt)
		return;
	strcpy(callCtxt->number, callInfo->phoneNumber);
	callCtxt->callId = callInfo->callId;
	callCtxt->callType = callInfo->callType;
	callCtxt->call_state = newCallState;
	callCtxt->bMT = 1;
	
	ril_request_unsolicited(RIL_UNSOL_CALL_RING, NULL, 0);
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
}

void ipc_call_end(void* data)
{
	ril_call_context* callCtxt;
	tapiCallEnd* callEndInfo = (tapiCallEnd*)(data);
	ALOGE("%s : Test me! callId = %d, cause = %d", __func__, callEndInfo->callId, callEndInfo->cause);
	callCtxt = find_ril_call_context(callEndInfo->callId);
	if(!callCtxt)
		return;
	if(callCtxt->token != 0)
	{
		ril_request_complete(callCtxt->token, RIL_E_SUCCESS, NULL, 0);
		release_ril_call_context(callCtxt);
		return;
	}
	release_ril_call_context(callCtxt);
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
}

void ipc_call_setup_ind(void* data)
{
	ril_call_context* callCtxt;
	uint32_t callId = *(uint32_t *)(data);
	ALOGE("%s : Test me! callId = %d", __func__, callId);
	callCtxt = find_ril_call_context(0xFFFFFFFF);
	if(!callCtxt)
		return;
	callCtxt->callId = callId;
	callCtxt = find_ril_call_context(callId);
	if(callCtxt->token != 0)
	{
		ril_request_complete(callCtxt->token, RIL_E_SUCCESS, NULL, 0);
		callCtxt->token = 0;
	}
}

void ipc_call_alert(void* data)
{
	ril_call_context* callCtxt;
	uint32_t callId = *(uint32_t *)(data);
	uint32_t bAudioOn = *(uint32_t *)((uint8_t*)(data) + 4);
	ALOGE("%s : Test me! callId = %d, bAudioOn = %d", __func__, callId, bAudioOn);
	callCtxt = find_ril_call_context(callId);
	if(!callCtxt)
		return;
	callCtxt->call_state = RIL_CALL_ALERTING;
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
}

void ipc_call_connected(void* data)
{
	ril_call_context* callCtxt;
	uint32_t callId = *(uint32_t *)(data);
	DEBUG_I("%s : Test me! callId = %d", __func__, callId);
	callCtxt = find_ril_call_context(callId);
	if(!callCtxt)
		return;
	callCtxt->call_state = RIL_CALL_ACTIVE;
	if(callCtxt->token != 0)
	{
		ril_request_complete(callCtxt->token, RIL_E_SUCCESS, NULL, 0);
		callCtxt->token = 0;
		return;
	}
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
}

void ipc_call_dtmf_start(void* data)
{
	ALOGE("%s: test me!", __func__);

	tapiDtmfCnf* dtmfCnf = (tapiDtmfCnf*)(data);

	if (dtmfCnf->reason != 0) {
		ALOGD("%s: Apparently, something went wrong with DTMF (code=0x%x)",__func__, dtmfCnf->reason);
		goto error;
	}

	ril_request_complete(ril_data.tokens.dtmf_start, RIL_E_SUCCESS, NULL, 0);

	return;

error:
	ALOGE("%s: Error!", __func__);
	ril_request_complete(ril_data.tokens.dtmf_start, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void ipc_call_dtmf_stop(void* data)
{
	ALOGE("%s: test me!", __func__);
	tapiDtmfCnf* dtmfCnf = (tapiDtmfCnf*)(data);

	if (dtmfCnf->reason != 0) {
		ALOGD("%s: Apparently, something went wrong with DTMF (code=0x%x)", __func__, dtmfCnf->reason);
		goto error;
	}

	ril_request_complete(ril_data.tokens.dtmf_stop, RIL_E_SUCCESS, NULL, 0);

	return;

error:
	ALOGE("%s: Error!", __func__);
	ril_request_complete(ril_data.tokens.dtmf_stop, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void ipc_call_hold(void* data)
{
	ril_call_context* heldCtxt;
	tapiCallEnd* holdCnf = (tapiCallEnd*)(data);
	ALOGE("%s: test me!(callId:%d, cause:%d)", __func__, 
	holdCnf->callId, holdCnf->cause);
	
	heldCtxt = find_ril_call_context(holdCnf->callId);
	if(!heldCtxt)
		return;

	heldCtxt->call_state = RIL_CALL_HOLDING;
	
	if(heldCtxt->token != 0)
	{
		ril_request_complete(heldCtxt->token, RIL_E_SUCCESS, NULL, 0);
		heldCtxt->token = 0;
	}
}

void ipc_call_swap(void* data)
{
	ril_call_context* activatedCtxt, *heldCtxt;
	tapiSwapCnf* swapCnf = (tapiSwapCnf*)(data);
	ALOGE("%s: test me!(ActivatedCallNo:%d,HeldCallNo:%d, cause:%d)", __func__, 
	swapCnf->activatedCallId, swapCnf->heldCallId, swapCnf->cause);
	
	activatedCtxt = find_ril_call_context(swapCnf->activatedCallId);
	heldCtxt = find_ril_call_context(swapCnf->heldCallId);
	if(!activatedCtxt || !heldCtxt)
		return;

	activatedCtxt->call_state = RIL_CALL_ACTIVE;
	heldCtxt->call_state = RIL_CALL_HOLDING;
	
	if(heldCtxt->token != 0)
	{
		ril_request_complete(heldCtxt->token, RIL_E_SUCCESS, NULL, 0);
	}
	else if(activatedCtxt->token != 0)
	{
		ril_request_complete(activatedCtxt->token, RIL_E_SUCCESS, NULL, 0);
	}
	heldCtxt->token = 0;
	activatedCtxt->token = 0;
}

void ipc_call_activate(void* data)
{
	ril_call_context* activatedCtxt;
	tapiCallEnd* activateCnf = (tapiCallEnd*)(data);
	ALOGE("%s: test me!(callNo:%d, cause:%d)", __func__, 
	activateCnf->callId, activateCnf->cause);
	
	activatedCtxt = find_ril_call_context(activateCnf->callId);
	if(!activatedCtxt)
		return;

	activatedCtxt->call_state = RIL_CALL_ACTIVE;
	
	if(activatedCtxt->token != 0)
	{
		ril_request_complete(activatedCtxt->token, RIL_E_SUCCESS, NULL, 0);
		activatedCtxt->token = 0;
	}
}

void ipc_call_error(void* data)
{
	ril_call_context* errorCtxt;
	tapiCallError* errorInd = (tapiCallError*)(data);
	ALOGE("%s: test me!(callNo:%d, error:%d)", __func__, errorInd->callId, errorInd->error);

	errorCtxt = find_ril_call_context(errorInd->callId);
	if(!errorCtxt)
		return;

	if(errorCtxt->token != 0)
		ril_request_complete(errorCtxt->token, RIL_E_GENERIC_FAILURE, NULL, 0);
	release_ril_call_context(errorCtxt);
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
}

void ril_request_dial(RIL_Token t, void *data, size_t datalen)
{	
	ril_call_context* callCtxt;
	tapiCallSetup *callSetup;
	RIL_Dial *dial;
	int clir;

	if (find_ril_call_context(0xFFFFFFFF) != NULL || data == NULL || datalen < sizeof(RIL_Dial))
		goto error;

	dial = (RIL_Dial *) data;
	callCtxt = new_ril_call_context();
	if(!callCtxt)
		goto error;
	
	callCtxt->callId = 0xFFFFFFFF;
	callCtxt->call_state = RIL_CALL_DIALING;
	callCtxt->bMT = 0;
	strcpy(callCtxt->number, dial->address);

	callSetup = (tapiCallSetup *)malloc(sizeof(tapiCallSetup));
	memset(callSetup, 0, sizeof(tapiCallSetup));

	callSetup->contextType = TAPI_CALL_CONTEXT_TYPE_VOICE;
	callSetup->bUsed = 1;
	callSetup->hCall = 3;
	callSetup->hClient = 0x0; //0x41C8255C;
	callSetup->callNo = 0xFF;
	callSetup->bOriginated = 1;
	callSetup->nameMode = 2;
	strcpy(callSetup->callNum1, dial->address);
	callSetup->startTime = time(NULL);
	callSetup->endTime = time(NULL);
	callSetup->callType1 = TAPI_CALL_CALLTYPE_VOICE1;
	callSetup->callState = TAPI_CALL_STATE_ORIG;
	callSetup->unknown10 = 1;
	callSetup->unknown12 = 0x3A;
	callSetup->unknown14 = 1;
	callSetup->callType2 = TAPI_CALL_CALLTYPE_VOICE1;
	strcpy(callSetup->callNum2, dial->address);

	tapi_call_setup(callSetup);

	if (callCtxt->token != 0)
		/* pass error to the current request, another one is pending */
		goto error;

	callCtxt->token = t;

	free(callSetup);
	return;

error:
	ALOGE("%s: Error!", __func__);
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void ril_request_get_current_calls(RIL_Token t)
{
	int i, j;

	RIL_Call **calls = NULL;
	j = 0;
	for (i = 0; i < MAX_CALLS; i++) {
		if(ril_data.calls[i] == NULL || ril_data.calls[i]->callId == 0xFFFFFFFF)
			continue;
		RIL_Call *call = (RIL_Call *) malloc(sizeof(RIL_Call));
		call->state = ril_data.calls[i]->call_state;
		call->index = i + 1;
		call->toa = (strlen(ril_data.calls[i]->number) > 0 && ril_data.calls[i]->number[0] == '+') ? 145 : 129;
		call->isMpty = 0;
		call->isMT = ril_data.calls[i]->bMT;
		call->als = 0;
		call->isVoice  = 1;
		call->isVoicePrivacy = 0;
		call->number = ril_data.calls[i]->number;
		call->numberPresentation = (strlen(ril_data.calls[i]->number) > 0) ? 0 : 2;
		call->name = NULL;
		call->namePresentation = 2;
		call->uusInfo = NULL;
		calls = (RIL_Call **) realloc(calls, (j + 1) * sizeof(RIL_Call *));
		calls[j++] = call;
	}

	ril_request_complete(t, RIL_E_SUCCESS, calls, (j * sizeof(RIL_Call *)));

	for (i = 0; i < j; i++) {
		free(calls[i]);
	}
	free(calls);
}

void ril_request_hangup(RIL_Token t, void *data, size_t datalen)
{
	ril_call_context* callCtxt;
	if(datalen < 4)
		goto error;
	callCtxt = ril_data.calls[*(int *)(data) - 1];
	if(!callCtxt)
		goto error;
	
	if(callCtxt->token != 0)
		/* pass error to the current request, another one is pending */
		goto error;

	callCtxt->token = t;

	tapi_call_release(callCtxt->callType, callCtxt->callId, 0x0);
	
	return;
error:
	ALOGE("%s: Error!", __func__);
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void ril_request_hangup_waiting_or_background(RIL_Token t)
{
	int i;
	ril_call_context* activeCtxt = NULL;
	ril_call_context* holdCtxt = NULL;
	ril_call_context* waitCtxt = NULL;
	ril_call_context* incomingCtxt = NULL;
	for(i = 0; i < MAX_CALLS; i++)
	{
		if(ril_data.calls[i] && ril_data.calls[i]->callId != 0xFFFFFFFF)
		{
			if(ril_data.calls[i]->call_state == RIL_CALL_ACTIVE)
			{
				activeCtxt = ril_data.calls[i];
				if (activeCtxt->token != 0)
					/* pass error to the current request, another one is pending */
					goto error;
				ALOGE("%s: active callId = %d", __func__, activeCtxt->callId);
			}
			if(ril_data.calls[i]->call_state == RIL_CALL_HOLDING)
			{
				holdCtxt = ril_data.calls[i];
				if (holdCtxt->token != 0)
					/* pass error to the current request, another one is pending */
					goto error;
				ALOGE("%s: hold callId = %d", __func__, holdCtxt->callId);
			}
			if(ril_data.calls[i]->call_state == RIL_CALL_WAITING)
			{
				waitCtxt = ril_data.calls[i];
				if (waitCtxt->token != 0)
					/* pass error to the current request, another one is pending */
					goto error;
				ALOGE("%s: wait callId = %d", __func__, waitCtxt->callId);
			}
			if(ril_data.calls[i]->call_state == RIL_CALL_INCOMING)
			{
				incomingCtxt = ril_data.calls[i];
				if (incomingCtxt->token != 0)
					/* pass error to the current request, another one is pending */
					goto error;
				ALOGE("%s: incoming callId = %d", __func__, incomingCtxt->callId);
			}
		}
	}
	if(waitCtxt && holdCtxt)
	{
		ALOGE("%s: waiting/hangup callId = %d", __func__, waitCtxt->callId);
		ALOGE("%s: hold/holding callId = %d", __func__, holdCtxt->callId);
		waitCtxt->token = t;
		tapi_call_release(waitCtxt->callType, waitCtxt->callId, 0x0);
	}
	else if(waitCtxt && activeCtxt)
	{

		ALOGE("%s: waiting/hangup callId = %d", __func__, waitCtxt->callId);
		ALOGE("%s: active/activate callId = %d", __func__, activeCtxt->callId);
		waitCtxt->token = t;
		tapi_call_release(waitCtxt->callType, waitCtxt->callId, 0x0);
	}
	else if(activeCtxt)
	{
		ALOGE("%s: active/hangup callId = %d", __func__, activeCtxt->callId);
		activeCtxt->token = t;
		tapi_call_release(activeCtxt->callType, activeCtxt->callId, 0x0);
	}
	else if(holdCtxt)
	{
		ALOGE("%s: hold/hangup callId = %d", __func__, holdCtxt->callId);
		holdCtxt->token = t;
		tapi_call_release(holdCtxt->callType, holdCtxt->callId, 0x0);
	}
	else if(waitCtxt)
	{
		ALOGE("%s: wait/hangup callId = %d", __func__, waitCtxt->callId);
		waitCtxt->token = t;
		tapi_call_release(waitCtxt->callType, waitCtxt->callId, 0x0);
	}
	else if(incomingCtxt)
	{
		ALOGE("%s: incoming/hangup callId = %d", __func__, incomingCtxt->callId);
		incomingCtxt->token = t;
		tapi_call_release(incomingCtxt->callType, incomingCtxt->callId, 0x0);
	}
	else if(!activeCtxt && !holdCtxt && !waitCtxt && !incomingCtxt)
		/* if calls was rejected from another side, but RIL still don't know about it */
		ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);
	return;
error:
	ALOGE("%s: Error!", __func__);
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}	

void ril_request_hangup_foreground_resume_background(RIL_Token t)
{
	int i;
	ril_call_context* activeCtxt = NULL;
	ril_call_context* holdCtxt = NULL;
	for(i = 0; i < MAX_CALLS; i++)
	{
		if(ril_data.calls[i] && ril_data.calls[i]->callId != 0xFFFFFFFF)
		{
			if(ril_data.calls[i]->call_state == RIL_CALL_ACTIVE)
			{
				activeCtxt = ril_data.calls[i];
				if (activeCtxt->token != 0)
					/* pass error to the current request, another one is pending */
					goto error;
				ALOGE("%s: active callId = %d", __func__, activeCtxt->callId);
			}
			if(ril_data.calls[i]->call_state == RIL_CALL_HOLDING)
			{
				holdCtxt = ril_data.calls[i];
				if (holdCtxt->token != 0)
					/* pass error to the current request, another one is pending */
					goto error;
				ALOGE("%s: hold callId = %d", __func__, holdCtxt->callId);
			}
		}
	}
	if(activeCtxt && holdCtxt)
	{
		ALOGE("%s: active/hangup callId = %d", __func__, activeCtxt->callId);
		ALOGE("%s: hold/active callId = %d", __func__, holdCtxt->callId);
		holdCtxt->token = t;
		tapi_call_release(activeCtxt->callType, activeCtxt->callId, 0x0);
		usleep(300000);
		tapi_call_activate(holdCtxt->callId);
	}
	else if(activeCtxt)
	{
		ALOGE("%s: active/hangup callId = %d", __func__, activeCtxt->callId);
		activeCtxt->token = t;
		tapi_call_release(activeCtxt->callType, activeCtxt->callId, 0x0);
	}
	else if(holdCtxt)
	{
		ALOGE("%s: hold/hangup callId = %d", __func__, holdCtxt->callId);
		holdCtxt->token = t;
		tapi_call_release(holdCtxt->callType, holdCtxt->callId, 0x0);
	}
	else if(!activeCtxt && !holdCtxt)
		/* if call was rejected from another side, but RIL still don't know about it */
		ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);
	return;
error:
	ALOGE("%s: Error!", __func__);
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void ril_request_answer(RIL_Token t)
{
	int i;
	ALOGE("%s: Test me!", __func__);
	
	for(i = 0; i < MAX_CALLS; i++)
	{
		if(ril_data.calls[i] && ril_data.calls[i]->callId != 0xFFFFFFFF && ril_data.calls[i]->call_state == RIL_CALL_INCOMING)
		{
			ALOGE("%s: answering callId = %d", __func__, ril_data.calls[i]->callId);
			if (ril_data.calls[i]->token != 0)
				/* pass error to the current request, another one is pending */
				goto error;
			ril_data.calls[i]->token = t;
			tapi_call_answer(ril_data.calls[i]->callType, ril_data.calls[i]->callId);
		}
	}
	return;
error:
	ALOGE("%s: Error!", __func__);
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

/**
 * In: RIL_REQUEST_LAST_CALL_FAIL_CAUSE
 *   Reason why last call was terminated
 */
void ril_request_last_call_fail_cause(RIL_Token t)
{
	RIL_LastCallFailCause fail_cause;
	
	ALOGE("%s: Implement me!", __func__);
	
	fail_cause = CALL_FAIL_NORMAL;

	ril_request_complete(t, RIL_E_SUCCESS, &fail_cause, sizeof(RIL_LastCallFailCause));
}

void ril_request_dtmf(RIL_Token t, void *data, int length)
{
	ALOGE("%s: Implement me!", __func__);
	
	ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);
}

void ril_request_dtmf_start(RIL_Token t, void *data, int length)
{
	unsigned char tone;
	ril_call_context* activeCall = find_active_call();
	
	if (activeCall == NULL || data == NULL || length < (int) sizeof(unsigned char))
		goto error;

	tone = *((unsigned char *) data);

	if (ril_data.state.dtmf_tone != 0) {
		ALOGD("%s: Another tone wasn't stopped, stopping it before anything", __func__);
		tapi_stop_dtmf(activeCall->callId);
		usleep(300);
	}

	ril_data.state.dtmf_tone = tone;

	tapi_start_dtmf(activeCall->callId, tone);

	ril_data.tokens.dtmf_start = t;

	return;

error:
	ALOGE("%s: Error!", __func__);
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void ril_request_dtmf_stop(RIL_Token t)
{
	ril_call_context* activeCall = find_active_call();
	
	if (activeCall == NULL)
		goto error;
		
	ril_data.state.dtmf_tone = 0;

	tapi_stop_dtmf(activeCall->callId);

	ril_data.tokens.dtmf_stop = t;
error:
	ALOGE("%s: Error!", __func__);
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void ril_request_switch_waiting_or_holding_and_active(RIL_Token t)
{
	int i;
	ril_call_context* activeCtxt = NULL;
	ril_call_context* holdCtxt = NULL;
	ril_call_context* waitCtxt = NULL;
	for(i = 0; i < MAX_CALLS; i++)
	{
		if(ril_data.calls[i] && ril_data.calls[i]->callId != 0xFFFFFFFF)
		{
			if(ril_data.calls[i]->call_state == RIL_CALL_ACTIVE)
			{
				activeCtxt = ril_data.calls[i];
				if (activeCtxt->token != 0)
					/* pass error to the current request, another one is pending */
					goto error;
				ALOGE("%s: active callId = %d", __func__, activeCtxt->callId);
			}
			if(ril_data.calls[i]->call_state == RIL_CALL_HOLDING)
			{
				holdCtxt = ril_data.calls[i];
				if (holdCtxt->token != 0)
					/* pass error to the current request, another one is pending */
					goto error;
				ALOGE("%s: hold callId = %d", __func__, holdCtxt->callId);
			}
			if(ril_data.calls[i]->call_state == RIL_CALL_WAITING)
			{
				waitCtxt = ril_data.calls[i];
				if (waitCtxt->token != 0)
					/* pass error to the current request, another one is pending */
					goto error;
				ALOGE("%s: wait callId = %d", __func__, waitCtxt->callId);
			}
		}
	}
	if(activeCtxt && holdCtxt)
	{
		ALOGE("%s: active/holding callId = %d", __func__, activeCtxt->callId);
		ALOGE("%s: hold/activating callId = %d", __func__, holdCtxt->callId);
		activeCtxt->token = t;
		tapi_calls_swap(activeCtxt->callId, holdCtxt->callId);
	}
	else if(activeCtxt && waitCtxt)
	{
		ALOGE("%s: active/holding callId = %d", __func__, activeCtxt->callId);
		ALOGE("%s: waiting/activating callId = %d", __func__, waitCtxt->callId);
		tapi_call_hold(waitCtxt->callId);

		usleep(500000);
		activeCtxt->token = t;
		tapi_calls_swap(activeCtxt->callId, waitCtxt->callId);
	}
	else if(holdCtxt && waitCtxt)
	{
		ALOGE("%s: hold/holding callId = %d", __func__, holdCtxt->callId);
		ALOGE("%s: wait/activating callId = %d", __func__, waitCtxt->callId);
		waitCtxt->token = 0;
		tapi_call_answer(waitCtxt->callType, waitCtxt->callId);
	}
	else if(activeCtxt)
	{
		ALOGE("%s: active/holding callId = %d", __func__, activeCtxt->callId);
		activeCtxt->token = t;
		tapi_call_hold(activeCtxt->callId);
	}
	else if(holdCtxt)
	{
		ALOGE("%s: hold/activating callId = %d", __func__, holdCtxt->callId);
		holdCtxt->token = t;
		tapi_call_activate(holdCtxt->callId);
	}
	else if(!activeCtxt && !holdCtxt && !waitCtxt)
		/* if calls was rejected from another side, but RIL still don't know about it */
		ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);
	return;

error:
	ALOGE("%s: Error!", __func__);
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}
