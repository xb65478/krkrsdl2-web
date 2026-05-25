//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Script/Window Event Handling and Dispatching / System Idle Event Delivering
//---------------------------------------------------------------------------
#include "tjsCommHead.h"
#include "LogFilter.h"

#include <algorithm>
#include "SysInitIntf.h"
#include "EventIntf.h"
#include "WindowIntf.h"
#include "tjsDictionary.h"
#include "MsgIntf.h"
#include "ScriptMgnIntf.h"
#include "TickCount.h"
#include <cstring>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
extern void TVPLogMotionHostBurstTrace(const char *tag, double a, double b, double c, double d);
#endif

static bool TVPSystemVariablesSavePending = false;
static bool TVPSystemVariablesSaveInProgress = false;
static tjs_uint TVPSystemVariablesSaveRequestCount = 0;
static tjs_uint TVPSystemVariablesSaveFlushCount = 0;
static tjs_uint TVPSystemVariablesSaveFailedFlushCount = 0;
static tjs_uint64 TVPSystemVariablesSaveEarliestTick = 0;
static const tjs_uint64 TVPSystemVariablesSaveDelayMs = 80;
static const tjs_uint TVPSystemVariablesSaveMaxFailedRetries = 3;

struct tTVPSystemVariablesSaveInProgressScope
{
	tTVPSystemVariablesSaveInProgressScope()
	{
		TVPSystemVariablesSaveInProgress = true;
	}
	~tTVPSystemVariablesSaveInProgressScope()
	{
		TVPSystemVariablesSaveInProgress = false;
	}
};

static bool TVPGetGlobalBoolFlag(iTJSDispatch2 *global, const tjs_char *name)
{
	tTJSVariant value;
	return TJS_SUCCEEDED(global->PropGet(0, name, NULL, &value, global)) &&
		value.Type() != tvtVoid && (tjs_int)value != 0;
}

static void TVPSetGlobalBoolFlag(iTJSDispatch2 *global, const tjs_char *name, bool value)
{
	tTJSVariant flag((tjs_int)(value ? 1 : 0));
	global->PropSet(TJS_MEMBERENSURE, name, NULL, &flag, global);
}

static void TVPSetSystemVariablesNativeSaveFlag(iTJSDispatch2 *global, bool value)
{
	TVPSetGlobalBoolFlag(global, TJS_W("__krkrDoSystemVariablesSaveNow"), value);
}

struct tTVPSystemVariablesNativeSaveScope
{
	iTJSDispatch2 *Global;
	tTVPSystemVariablesNativeSaveScope(iTJSDispatch2 *global) : Global(global)
	{
		TVPSetSystemVariablesNativeSaveFlag(Global, true);
	}
	~tTVPSystemVariablesNativeSaveScope()
	{
		TVPSetSystemVariablesNativeSaveFlag(Global, false);
	}
};

static bool TVPDoSaveSystemVariablesImpl()
{
	if(TVPSystemVariablesSaveInProgress)
	{
#ifdef __EMSCRIPTEN__
		KRKR_LOG_L2("[SYSVAR-SAVE] source=TVPDoSaveSystemVariables skipped=reentrant\n");
#endif
		return false;
	}
	tTVPSystemVariablesSaveInProgressScope inProgress;
	try
	{
		iTJSDispatch2 *global = TVPGetScriptDispatch();
		if(!global)
		{
#ifdef __EMSCRIPTEN__
			KRKR_LOG_L2("[SYSVAR-SAVE] source=TVPDoSaveSystemVariables skipped=no-global\n");
#endif
			return false;
		}
#ifdef __EMSCRIPTEN__
		tTJSVariant loaded;
		if(!(TJS_SUCCEEDED(global->PropGet(0, TJS_W("__krkrSystemVariablesLoaded"), NULL, &loaded, global)) &&
			loaded.Type() != tvtVoid && (tjs_int)loaded))
		{
			KRKR_LOG_L2("[SYSVAR-SAVE] source=TVPDoSaveSystemVariables skipped=not-loaded\n");
			return false;
		}
#endif
		tTJSVariant var;
		if(TJS_SUCCEEDED(global->PropGet(0, TJS_W("kag"), NULL, &var, global)) && var.Type() == tvtObject)
		{
			iTJSDispatch2 *kag = var.AsObjectNoAddRef();
			tTJSVariant save;
			if(TJS_SUCCEEDED(kag->PropGet(0, TJS_W("saveSystemVariables"), NULL, &save, kag)) && save.Type() == tvtObject)
			{
				iTJSDispatch2 *fn = save.AsObjectNoAddRef();
				if(fn && fn->IsInstanceOf(0, 0, 0, TJS_W("Function"), fn) == TJS_S_TRUE)
				{
					tTVPSystemVariablesNativeSaveScope nativeSave(global);
#ifdef __EMSCRIPTEN__
					KRKR_LOG_L2("[SYSVAR-SAVE] source=TVPDoSaveSystemVariables begin\n");
#endif
					fn->FuncCall(0, NULL, NULL, NULL, 0, NULL, kag);
#ifdef __EMSCRIPTEN__
					KRKR_LOG_L2("[SYSVAR-SAVE] source=TVPDoSaveSystemVariables done\n");
#endif
					return true;
				}
			}
		}
	}
	catch(...)
	{
#ifdef __EMSCRIPTEN__
		KRKR_LOG_L2("[SYSVAR-SAVE] source=TVPDoSaveSystemVariables failed\n");
#endif
	}
	return false;
}

void TVPRequestSaveSystemVariables(const char *source)
{
	const char *requestSource = source ? source : "unknown";
	bool suppressStartupRequest = false;
#ifdef __EMSCRIPTEN__
	bool clearedStartupSuppress = false;
#endif
	try
	{
		iTJSDispatch2 *global = TVPGetScriptDispatch();
		if(global)
		{
			const bool startupSuppressed =
				TVPGetGlobalBoolFlag(global, TJS_W("__krkrSuppressStartupSystemVariablesSaveRequest"));
			const bool startupOnlySource =
				std::strcmp(requestSource, "MainWindow.saveSystemVariables") == 0 ||
				std::strcmp(requestSource, "MainWindow.initial") == 0;
			if(startupSuppressed && startupOnlySource)
			{
				suppressStartupRequest = true;
				TVPSetGlobalBoolFlag(global, TJS_W("__krkrSuppressStartupSystemVariablesSaveRequest"), false);
			}
			else if(startupSuppressed)
			{
				TVPSetGlobalBoolFlag(global, TJS_W("__krkrSuppressStartupSystemVariablesSaveRequest"), false);
#ifdef __EMSCRIPTEN__
				clearedStartupSuppress = true;
#endif
			}
		}
	}
	catch(...)
	{
	}
	if(suppressStartupRequest)
	{
#ifdef __EMSCRIPTEN__
		KRKR_LOG_L2("[SYSVAR-SAVE] request source=%s skipped=startup-suppressed pending=%d suppressCleared=1\n",
			requestSource,
			TVPSystemVariablesSavePending ? 1 : 0);
#endif
		return;
	}
	TVPSystemVariablesSaveRequestCount++;
	TVPSystemVariablesSavePending = true;
	TVPStartTickCount();
	TVPSystemVariablesSaveEarliestTick = TVPGetTickCount() + TVPSystemVariablesSaveDelayMs;
#ifdef __EMSCRIPTEN__
	if(clearedStartupSuppress)
	{
		KRKR_LOG_L2("[SYSVAR-SAVE] startup-suppress-cleared source=%s\n", requestSource);
	}
	KRKR_LOG_L2("[SYSVAR-SAVE] request source=%s count=%u pending=1 inProgress=%d delayMs=%u\n",
		requestSource,
		(unsigned)TVPSystemVariablesSaveRequestCount,
		TVPSystemVariablesSaveInProgress ? 1 : 0,
		(unsigned)TVPSystemVariablesSaveDelayMs);
#else
	(void)source;
#endif
}

void TVPDoSaveSystemVariables()
{
	(void)TVPDoSaveSystemVariablesImpl();
}

void TVPFlushPendingSystemVariablesSave(const char *reason, bool force)
{
	if(!force && !TVPSystemVariablesSavePending) return;
	if(!force)
	{
		TVPStartTickCount();
		if(TVPGetTickCount() < TVPSystemVariablesSaveEarliestTick) return;
	}
	if(TVPSystemVariablesSaveInProgress)
	{
#ifdef __EMSCRIPTEN__
		KRKR_LOG_L2("[SYSVAR-SAVE] flush reason=%s skipped=reentrant pending=%d\n",
			reason ? reason : "unknown",
			TVPSystemVariablesSavePending ? 1 : 0);
#else
		(void)reason;
#endif
		return;
	}
	TVPSystemVariablesSaveFlushCount++;
#ifdef __EMSCRIPTEN__
	KRKR_LOG_L2("[SYSVAR-SAVE] flush reason=%s count=%u pending=%d force=%d\n",
		reason ? reason : "unknown",
		(unsigned)TVPSystemVariablesSaveFlushCount,
		TVPSystemVariablesSavePending ? 1 : 0,
		force ? 1 : 0);
	#endif
	const bool hadPending = TVPSystemVariablesSavePending;
	const tjs_uint requestCountBeforeFlush = TVPSystemVariablesSaveRequestCount;
	TVPSystemVariablesSavePending = false;
	bool saved = TVPDoSaveSystemVariablesImpl();
	if(saved)
	{
		TVPSystemVariablesSaveFailedFlushCount = 0;
	}
	else if(hadPending)
	{
		TVPSystemVariablesSaveFailedFlushCount++;
	}
	if(TVPSystemVariablesSaveRequestCount != requestCountBeforeFlush ||
		(hadPending && !saved && TVPSystemVariablesSaveFailedFlushCount < TVPSystemVariablesSaveMaxFailedRetries))
	{
		TVPSystemVariablesSavePending = true;
		TVPStartTickCount();
		TVPSystemVariablesSaveEarliestTick = TVPGetTickCount() + TVPSystemVariablesSaveDelayMs;
	}
#ifdef __EMSCRIPTEN__
	else if(hadPending && !saved)
	{
		KRKR_LOG_L2("[SYSVAR-SAVE] flush reason=%s failed-dropping-pending failures=%u\n",
			reason ? reason : "unknown",
			(unsigned)TVPSystemVariablesSaveFailedFlushCount);
	}
#endif
}




//---------------------------------------------------------------------------
// tTVPEvent  : script event class
//---------------------------------------------------------------------------
extern tjs_uint64 TVPEventSequenceNumber;
class tTVPEvent
{
	iTJSDispatch2 *Target;
	iTJSDispatch2 *Source;
	ttstr EventName;
	tjs_uint32 Tag;
	tjs_uint NumArgs;
	tTJSVariant *Args;
	tjs_uint32 Flags;
	tjs_uint64 Sequence;

public:
	tTVPEvent(iTJSDispatch2 *target, iTJSDispatch2 *source,
		ttstr &eventname, tjs_uint32 tag, tjs_uint numargs, tTJSVariant *args,
		tjs_uint32 flags)
	{
		// constructor

		// eventname is not a const object but this object only touch to
		// eventname.GetHint()

		Args = NULL;
		Target = NULL;
		Source = NULL;

		Sequence = TVPEventSequenceNumber;
		EventName = eventname;
		NumArgs = numargs;
		Args = new tTJSVariant[NumArgs];
		for(tjs_uint i=0; i<NumArgs; i++)
			Args[i]=args[i];
		Target = target;
		Source = source;
		Tag = tag;
		Flags = flags;
		if(Target) Target->AddRef();
		if(Source) Source->AddRef();
	}


	tTVPEvent(const tTVPEvent &ref)
	{
		// copy constructor
		Args = NULL;
		Target = NULL;
		Source = NULL;

		EventName = ref.EventName;
		NumArgs = ref.NumArgs;
		Args = new tTJSVariant[NumArgs];
		for(tjs_uint i=0; i<NumArgs; i++)
			Args[i]=ref.Args[i];
		Target = ref.Target;
		Source = ref.Source;
		Tag = ref.Tag;
		if(Target) Target->AddRef();
		if(Source) Source->AddRef();
	}

	~tTVPEvent()
	{
		if(Args) delete [] Args;
		if(Target) Target->Release();
		if(Source) Source->Release();
	}

	void Deliver()
	{
		if(!TJSIsObjectValid(Target->IsValid(0, NULL, NULL, Target)))
			return; // The target had been invalidated
		tTJSVariant **ArgsPtr = new tTJSVariant*[NumArgs];
		for(tjs_uint i=0; i<NumArgs; i++)
			ArgsPtr[i] = Args + i;
		try
		{
			Target->FuncCall(0, EventName.c_str(), EventName.GetHint(),
				NULL, NumArgs, ArgsPtr,
				Target);
		}
		catch(...)
		{
			delete [] ArgsPtr;
			throw;
		}
		delete [] ArgsPtr;
	}


	iTJSDispatch2 * GetTargetNoAddRef() const { return Target; }
	iTJSDispatch2 * GetSourceNoAddRef() const { return Source; }
	ttstr & GetEventName() { return EventName; }
	tjs_uint32 GetTag() const { return Tag; }
	tjs_uint32 GetFlags() const { return Flags; }
	tjs_uint64 GetSequence() const;
};
//---------------------------------------------------------------------------
tjs_uint64 tTVPEvent::GetSequence() const { return Sequence; }
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// tTVPWinUpdateEvent : window update event class
//---------------------------------------------------------------------------
class tTVPWinUpdateEvent
{
	tTJSNI_BaseWindow *Window;

public:
	tTVPWinUpdateEvent(tTJSNI_BaseWindow *window)
	{
		Window = window;
	}

	tTVPWinUpdateEvent(const tTVPWinUpdateEvent & ref)
	{
		Window = ref.Window;
	}

	~tTVPWinUpdateEvent()
	{
	}

	void Deliver() const
	{
		Window->UpdateContent();
	}

    tTJSNI_BaseWindow * GetWindow() const { return Window; }

	void MarkEmpty() { Window = NULL; }

	bool IsEmpty() const { return Window == NULL; }
};
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
// global/static definitions
//---------------------------------------------------------------------------
// event queue must be a globally sequential queue
std::vector<tTVPBaseInputEvent *> TVPInputEventQueue;
std::vector<tTVPEvent *> TVPEventQueue;
std::vector<tTVPWinUpdateEvent> TVPWinUpdateEventQueue;
bool TVPExclusiveEventPosted = false; // true if exclusive event is posted
tjs_uint64 TVPEventSequenceNumber = 0; // event sequence number
tjs_uint64 TVPEventSequenceNumberToProcess = 0;
	// current event sequence which must be processed

static void TVPDestroyEventQueue()
{
	// delete all event objects
	// deletion of event object may cause other deletion of event objects.
	{
		std::vector<tTVPEvent *>::iterator i;
		while(TVPEventQueue.size())
		{
			i = TVPEventQueue.end() -1;
			tTVPEvent * ev = *i;
			TVPEventQueue.erase(i);
			delete ev;
		}
	}
//--
	{
		std::vector<tTVPBaseInputEvent *>::iterator i;
		while(TVPInputEventQueue.size())
		{
			i = TVPInputEventQueue.end() - 1;
			tTVPBaseInputEvent * ev = *i;
			TVPInputEventQueue.erase(i);
			delete ev;
		}
	}
}

static tTVPAtExit TVPDestroyEventQueueAtExit
	(TVP_ATEXIT_PRI_PREPARE, TVPDestroyEventQueue);

bool TVPEventDisabled = false;
bool TVPEventInterrupting = false;

//#define TVP_EVENT_TASK_RETURN_TICK 100000
	/* TVP event system once returns to Operation system when
		TVP_EVENT_TASK_RETURN_TICK is elapsed during event delivering. */
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// TVPPostEvent
//---------------------------------------------------------------------------
void TVPPostEvent(iTJSDispatch2 * source, iTJSDispatch2 *target,
	ttstr &eventname, tjs_uint32 tag, tjs_uint32 flag,
	tjs_uint numargs, tTJSVariant *args)
{
	bool evdisabled = TVPEventDisabled || TVPGetSystemEventDisabledState();

	if((flag & TVP_EPT_DISCARDABLE) &&
		(TVPEventInterrupting || evdisabled)) return;

	tjs_int method = flag & TVP_EPT_METHOD_MASK;

	if(method == TVP_EPT_IMMEDIATE)
	{
		// the event is delivered immediately

		if(evdisabled) return;

		try
		{
			try
			{
				tTVPEvent(target, source, eventname, tag, numargs, args, flag).
					Deliver();
			}
			TJS_CONVERT_TO_TJS_EXCEPTION
		}
		TVP_CATCH_AND_SHOW_SCRIPT_EXCEPTION(TJS_W("immediate event"));

		return;
	}


	if(method == TVP_EPT_REMOVE_POST)
	{
		// events in queue that have same target/source/name/tag are to be removed
		std::vector<tTVPEvent *>::iterator i;
		i = TVPEventQueue.begin();
		while(/*TVPEventQueue.size() &&*/ i != TVPEventQueue.end())
		{
			if(source == (*i)->GetSourceNoAddRef() &&
				target == (*i)->GetTargetNoAddRef() &&
				eventname == (*i)->GetEventName() &&
				((tag==0)?true:(tag==(*i)->GetTag())) )
			{
				tTVPEvent *ev = *i;
				TVPEventQueue.erase(i);
				i = TVPEventQueue.begin();
				delete ev;
			}
			else
			{
				i++;
			}
		}
	}

	// put into queue
	TVPEventQueue.push_back(new tTVPEvent(target, source, eventname, tag,
									numargs, args, flag));

	// is exclusive?
	if((flag & TVP_EPT_PRIO_MASK) == TVP_EPT_EXCLUSIVE) TVPExclusiveEventPosted = true;

	// make sure that the event is to be delivered.
	TVPInvokeEvents();
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPCancelEvents
//---------------------------------------------------------------------------
tjs_int TVPCancelEvents(iTJSDispatch2 * source, iTJSDispatch2 *target,
	const ttstr &eventname, tjs_uint32 tag)
{
	tjs_int count = 0;
	std::vector<tTVPEvent *>::iterator i;
	i = TVPEventQueue.begin();
	while(/*TVPEventQueue.size() &&*/ i != TVPEventQueue.end())
	{
		if(source == (*i)->GetSourceNoAddRef() &&
			target == (*i)->GetTargetNoAddRef() &&
			eventname == (*i)->GetEventName() &&
				((tag==0)?true:(tag==(*i)->GetTag())) )
		{
			tTVPEvent *ev = *i;
			TVPEventQueue.erase(i);
			i = TVPEventQueue.begin();
			delete ev;
			count ++;
		}
		else
		{
			i++;
		}
	}
	return count;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPAreEventsInQueue
//---------------------------------------------------------------------------
bool TVPAreEventsInQueue(iTJSDispatch2 * source, iTJSDispatch2 *target,
	const ttstr &eventname, tjs_uint32 tag)
{
	std::vector<tTVPEvent *>::iterator i;
	i = TVPEventQueue.begin();
	while(/*TVPEventQueue.size() &&*/ i != TVPEventQueue.end())
	{
		if(source == (*i)->GetSourceNoAddRef() &&
			target == (*i)->GetTargetNoAddRef() &&
			eventname == (*i)->GetEventName() &&
				((tag==0)?true:(tag==(*i)->GetTag())) )
		return true;
		i++;
	}
	return false;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPCountEventsInQueue
//---------------------------------------------------------------------------
tjs_int TVPCountEventsInQueue(iTJSDispatch2 * source, iTJSDispatch2 *target,
	const ttstr &eventname, tjs_uint32 tag)
{
	tjs_int count = 0;
	std::vector<tTVPEvent *>::iterator i;
	i = TVPEventQueue.begin();
	while(/*TVPEventQueue.size() &&*/ i != TVPEventQueue.end())
	{
		if(source == (*i)->GetSourceNoAddRef() &&
			target == (*i)->GetTargetNoAddRef() &&
			eventname == (*i)->GetEventName() &&
				((tag==0)?true:(tag==(*i)->GetTag())) )
		count ++;
		i++;
	}
	return count;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPCancelEventByTag
//---------------------------------------------------------------------------
void TVPCancelEventsByTag(iTJSDispatch2 * source, iTJSDispatch2 *target,
	tjs_uint32 tag)
{
	std::vector<tTVPEvent *>::iterator i;
	i = TVPEventQueue.begin();
	while(/*TVPEventQueue.size() &&*/ i != TVPEventQueue.end())
	{
		if(source == (*i)->GetSourceNoAddRef() &&
			target == (*i)->GetTargetNoAddRef() &&
				((tag==0)?true:(tag==(*i)->GetTag())) )
		{
			tTVPEvent *ev = *i;
			TVPEventQueue.erase(i);
			i = TVPEventQueue.begin();
			delete ev;
		}
		else
		{
			i++;
		}
	}
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPCancelSourceEvent
//---------------------------------------------------------------------------
void TVPCancelSourceEvents(iTJSDispatch2 * source)
{
	std::vector<tTVPEvent *>::iterator i;
	i = TVPEventQueue.begin();
	while(/*TVPEventQueue.size() &&*/ i != TVPEventQueue.end())
	{
		if(source == (*i)->GetSourceNoAddRef())
		{
			tTVPEvent *ev = *i;
			TVPEventQueue.erase(i);
			i = TVPEventQueue.begin();
			delete ev;
		}
		else
		{
			i++;
		}
	}
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPDiscardAllDiscardableEvents
//---------------------------------------------------------------------------
void TVPDiscardAllDiscardableEvents()
{
	std::vector<tTVPEvent *>::iterator i;
	i = TVPEventQueue.begin();
	while(/*TVPEventQueue.size() &&*/ i != TVPEventQueue.end())
	{
		if((*i)->GetFlags() & TVP_EPT_DISCARDABLE)
		{
			tTVPEvent *ev = *i;
			TVPEventQueue.erase(i);
			i = TVPEventQueue.begin();
			delete ev;
		}
		else
		{
			i++;
		}
	}
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPDeliverAllEvents
//---------------------------------------------------------------------------
static void _TVPDeliverEventByPrio(tjs_uint prio)
{
	while(true)
	{
		tTVPEvent *e;

		// retrieve item to deliver
		if(TVPEventQueue.size() == 0) break;
		std::vector<tTVPEvent *>::iterator i =
			TVPEventQueue.begin();
		while(i != TVPEventQueue.end())
		{
			if((*i)->GetSequence() <= TVPEventSequenceNumberToProcess &&
				(((*i)->GetFlags() & TVP_EPT_PRIO_MASK) == prio)) break;
			i++;
		}
		if(i == TVPEventQueue.end()) break;
		e = *i;
		TVPEventQueue.erase(i);

		// event delivering
		try
		{
			e->Deliver();
		}
		catch(...)
		{
			delete e;
			throw;
		}
		delete e;
	}
}


static bool _TVPDeliverAllEvents2()
{
	TVPExclusiveEventPosted = false;

	// process exclusive events
	_TVPDeliverEventByPrio(TVP_EPT_EXCLUSIVE);

	// check exclusive events
	if(TVPExclusiveEventPosted) return true;

	// process input event queue
	while(true)
	{
		tTVPBaseInputEvent *e;

		// retrieve item to deliver
		if(TVPInputEventQueue.size() == 0) break;
		std::vector<tTVPBaseInputEvent *>::iterator i =
			TVPInputEventQueue.begin();
		e = *i;
		TVPInputEventQueue.erase(i);

		// event delivering
		try
		{
			e->Deliver();
		}
		catch(...)
		{
			delete e;
			throw;
		}
		delete e;

		// check exclusive events
		if(TVPExclusiveEventPosted) return true;

	}

	// process normal event queue
	_TVPDeliverEventByPrio(TVP_EPT_NORMAL);

	// check exclusive events
	if(TVPExclusiveEventPosted) return true;

	return true;
}
//---------------------------------------------------------------------------
static bool _TVPDeliverAllEvents()
{
	// deliver all pending events to targets.
	if(TVPEventDisabled) return true;

	// event invokation was received...
	TVPEventReceived();

	// for script event objects

	bool ret_value;

	ret_value = _TVPDeliverAllEvents2();

	return ret_value;
}
//---------------------------------------------------------------------------
void TVPDeliverAllEvents()
{
	bool r = true;

#ifdef __EMSCRIPTEN__
	static int deliver_call_count = 0;
	deliver_call_count++;
	if (TVPLogL3() && (deliver_call_count <= 5 || deliver_call_count % 500 == 0)) {
		fprintf(stderr, "[DIAG-EVT] TVPDeliverAllEvents #%d qsize=%d exclusive=%d interrupting=%d\n",
			deliver_call_count, (int)TVPEventQueue.size(), (int)TVPExclusiveEventPosted, (int)TVPEventInterrupting);
	}
#endif

	if(!TVPEventInterrupting)
	{
		TVPEventSequenceNumberToProcess = TVPEventSequenceNumber;
		TVPEventSequenceNumber ++; // increment sequence number
	}

	TVPEventInterrupting = false;

	try
	{
	   try
		{
			r = _TVPDeliverAllEvents();
		}
		TJS_CONVERT_TO_TJS_EXCEPTION
	}
	TVP_CATCH_AND_SHOW_SCRIPT_EXCEPTION(TJS_W("event"));

	if(!r)
	{
		// event processing is to be interrupted
		// XXX: currently this is not functional
		TVPEventInterrupting = true;
		TVPCallDeliverAllEventsOnIdle();
	}

	if(!TVPExclusiveEventPosted && !TVPEventInterrupting)
	{
		try
		{
			try
			{
				// process idle event queue
				_TVPDeliverEventByPrio(TVP_EPT_IDLE);
			}
			TJS_CONVERT_TO_TJS_EXCEPTION
		}
		TVP_CATCH_AND_SHOW_SCRIPT_EXCEPTION(TJS_W("idle event"));

		// process continuous events
		if(TVPProcessContinuousHandlerEventFlag)
		{
			TVPProcessContinuousHandlerEventFlag = false; // processed
			// XXX: strictly saying, we need something like InterlockedExchange
			// to look/set this flag, because TVPProcessContinuousHandlerEventFlag
			// may be accessed by another thread. But I have no dought about
			// that no one does care of missing one event in rare race condition.
#ifdef __EMSCRIPTEN__
			double cont_start_ms = emscripten_get_now();
#endif
			TVPDeliverContinuousEvent();
#ifdef __EMSCRIPTEN__
			TVPLogMotionHostBurstTrace("idle-cont",
				emscripten_get_now() - cont_start_ms,
				0.0,
				0.0,
				0.0);
#endif
		}

		try
		{
		   try
			{
				// for window content updating
#ifdef __EMSCRIPTEN__
				double wup_start_ms = emscripten_get_now();
#endif
				TVPDeliverWindowUpdateEvents();
#ifdef __EMSCRIPTEN__
				TVPLogMotionHostBurstTrace("idle-winupd",
					emscripten_get_now() - wup_start_ms,
					(double)TVPWinUpdateEventQueue.size(),
					0.0,
					0.0);
#endif
			}
			TJS_CONVERT_TO_TJS_EXCEPTION
		}
		TVP_CATCH_AND_SHOW_SCRIPT_EXCEPTION(TJS_W("window update"));
	}

	if(TVPEventQueue.size() == 0)
	{
		TVPEventSequenceNumber = 0; // reset the number
	}

}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
// TVPPostWindowUpdate
//---------------------------------------------------------------------------
bool TVPWindowUpdateEventsDelivering = false;
void TVPPostWindowUpdate(tTJSNI_BaseWindow *window)
{
#ifdef __EMSCRIPTEN__
	static int pwu_count = 0;
	pwu_count++;
	if (TVPLogL3() && (pwu_count <= 10 || pwu_count % 500 == 0)) {
		fprintf(stderr, "[DIAG-PWU] TVPPostWindowUpdate #%d window=%p delivering=%d qsize=%d\n",
			pwu_count, (void*)window, (int)TVPWindowUpdateEventsDelivering,
			(int)TVPWinUpdateEventQueue.size());
	}
#endif

	if(!TVPWindowUpdateEventsDelivering)
	{
		if(TVPWinUpdateEventQueue.size())
		{
			// since duplication is not allowed ...
			std::vector<tTVPWinUpdateEvent>::const_iterator i;
			for(i = TVPWinUpdateEventQueue.begin();
				i !=TVPWinUpdateEventQueue.end(); i++)
			{
				if(!i->IsEmpty() && window == i->GetWindow()) return;
			}
		}
	}
	else
	{
		if(TVPWinUpdateEventQueue.size())
		{
			// duplication is allowed up to two
			tjs_int count = 0;
			std::vector<tTVPWinUpdateEvent>::const_iterator i;
			for(i = TVPWinUpdateEventQueue.begin();
				i !=TVPWinUpdateEventQueue.end(); i++)
			{
				if(!i->IsEmpty() && window == i->GetWindow())
				{
					count++;
					if(count == 2) return;
				}
			}
		}
	}

	// put into queue.
	TVPWinUpdateEventQueue.push_back(tTVPWinUpdateEvent(window));

	// make sure that the event is to be delivered.
	TVPInvokeEvents();
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPRemoveWindowUpdate
//---------------------------------------------------------------------------
void TVPRemoveWindowUpdate(tTJSNI_BaseWindow *window)
{
	// removes all window update events from queue.
	if(TVPWinUpdateEventQueue.size())
	{
		std::vector<tTVPWinUpdateEvent>::iterator i;
		for(i = TVPWinUpdateEventQueue.begin();
			i !=TVPWinUpdateEventQueue.end(); i++)
		{
			if(!i->IsEmpty() && window == i->GetWindow())
				i->MarkEmpty();
		}
	}
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPDeliverWindowUpdateEvents
//---------------------------------------------------------------------------
void TVPDeliverWindowUpdateEvents()
{
	if(TVPWindowUpdateEventsDelivering) return; // does not allow re-entering
	TVPWindowUpdateEventsDelivering = true;

#ifdef __EMSCRIPTEN__
	static int wue_call_count = 0;
	wue_call_count++;
	if (TVPLogL3() && TVPWinUpdateEventQueue.size() > 0 && (wue_call_count <= 5 || wue_call_count % 200 == 0)) {
		fprintf(stderr, "[DIAG-WUE] WindowUpdateEvents #%d qsize=%d\n",
			wue_call_count, (int)TVPWinUpdateEventQueue.size());
	}
#endif

	try
	{
		for(tjs_uint i = 0; i < TVPWinUpdateEventQueue.size(); i++)
		{
			if(!TVPWinUpdateEventQueue[i].IsEmpty())
				TVPWinUpdateEventQueue[i].Deliver();
		}
	}
	catch(...)
	{
		TVPWinUpdateEventQueue.clear();
		TVPWindowUpdateEventsDelivering = false;
		throw;
	}
	TVPWinUpdateEventQueue.clear();
	TVPWindowUpdateEventsDelivering = false;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// Input Event related
//---------------------------------------------------------------------------
tjs_int TVPInputEventTagMax = 0;
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPPostInputEvent
//---------------------------------------------------------------------------
void TVPPostInputEvent(tTVPBaseInputEvent *ev, tjs_uint32 flags)
{
	// flag check
	if((flags & TVP_EPT_DISCARDABLE) &&
		(TVPEventDisabled || TVPGetSystemEventDisabledState()))
	{
		delete ev;
		return;
	}

	if(flags & TVP_EPT_REMOVE_POST)
	{
		// cancel previously posted events
		TVPCancelInputEvents(ev->GetSource(), ev->GetTag());
	}


	// push into the event queue
	TVPInputEventQueue.push_back(ev);

	// make sure that the event is to be delivered.
	TVPInvokeEvents();
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPCancelInputEvents
//---------------------------------------------------------------------------
void TVPCancelInputEvents(void * source)
{
	// removes all evens which have the same source
	if(TVPInputEventQueue.size())
	{
		std::vector<tTVPBaseInputEvent *>::iterator i;
		for(i = TVPInputEventQueue.begin();
			i !=TVPInputEventQueue.end();)
		{
			if(source == (*i)->GetSource())
			{
				tTVPBaseInputEvent *ev = *i;
				i = TVPInputEventQueue.erase(i);
				delete ev;
			}
			else
			{
				i++;
			}
		}
	}
}
//---------------------------------------------------------------------------
void TVPCancelInputEvents(void * source, tjs_int tag)
{
	// removes all evens which have the same source and the same tag
	if(TVPInputEventQueue.size())
	{
		std::vector<tTVPBaseInputEvent *>::iterator i;
		for(i = TVPInputEventQueue.begin();
			i !=TVPInputEventQueue.end();)
		{
			if(source == (*i)->GetSource() && tag == (*i)->GetTag())
			{
				tTVPBaseInputEvent *ev = *i;
				i = TVPInputEventQueue.erase(i);
				delete ev;
			}
			else
			{
				i++;
			}
		}
	}
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPGetInputEventCount
//---------------------------------------------------------------------------
tjs_int TVPGetInputEventCount()
{
	return (tjs_int)TVPInputEventQueue.size();
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPCreateEventObject
//---------------------------------------------------------------------------
iTJSDispatch2 * TVPCreateEventObject(const tjs_char *type,
	iTJSDispatch2 *targthis, iTJSDispatch2 *targ)
{
	// create a dictionary object for event dispatching ( to "action" method )
	iTJSDispatch2 * object = TJSCreateDictionaryObject();

	static ttstr type_name(TJS_W("type"));
	static ttstr target_name(TJS_W("target"));

	{
		tTJSVariant val(type);
		if(TJS_FAILED(object->PropSet(TJS_MEMBERENSURE|TJS_IGNOREPROP,
			type_name.c_str(), type_name.GetHint(), &val, object)))
				TVPThrowInternalError;
	}

	{
		tTJSVariant val(targthis, targ);
		if(TJS_FAILED(object->PropSet(TJS_MEMBERENSURE|TJS_IGNOREPROP,
			target_name.c_str(), target_name.GetHint(), &val, object)))
				TVPThrowInternalError;
	}
	
	return object;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
ttstr TVPActionName(TJS_W("action"));
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// Continuous Event Delivering related
//---------------------------------------------------------------------------
bool TVPProcessContinuousHandlerEventFlag = false;
static std::vector<tTVPContinuousEventCallbackIntf *> TVPContinuousEventVector;
static std::vector<tTJSVariantClosure> TVPContinuousHandlerVector;
static bool TVPContinuousEventProcessing = false;

static void TVPDestroyContinuousHandlerVector()
{
	std::vector<tTJSVariantClosure>::iterator i;
	for(i = TVPContinuousHandlerVector.begin();
		i != TVPContinuousHandlerVector.end();
		i++)
	{
		i->Release();
	}
	TVPContinuousHandlerVector.clear();
}

static tTVPAtExit TVPDestroyContinuousHandlerVectorAtExit
	(TVP_ATEXIT_PRI_PREPARE, TVPDestroyContinuousHandlerVector);
//---------------------------------------------------------------------------
void TVPAddContinuousEventHook(tTVPContinuousEventCallbackIntf *cb)
{
	TVPBeginContinuousEvent();
	TVPContinuousEventVector.push_back(cb);
}
//---------------------------------------------------------------------------
void TVPRemoveContinuousEventHook(tTVPContinuousEventCallbackIntf *cb)
{
	std::vector<tTVPContinuousEventCallbackIntf *>::iterator i;
	for(i = TVPContinuousEventVector.begin();
		i !=TVPContinuousEventVector.end();)
	{
		if(cb == *i) *i = NULL; // simply assign a null
		i++;
	}
}
//---------------------------------------------------------------------------
static void _TVPDeliverContinuousEvent() // internal
{
	TVPStartTickCount();
	tjs_uint64 tick = TVPGetTickCount();

#ifdef __EMSCRIPTEN__
	static int ce_call_count = 0;
	ce_call_count++;
	if (TVPLogL3() && (ce_call_count <= 5 || ce_call_count % 500 == 0)) {
		fprintf(stderr, "[DIAG-CE] _TVPDeliverContinuousEvent #%d vecsize=%d handlersize=%d disabled=%d\n",
			ce_call_count, (int)TVPContinuousEventVector.size(),
			(int)TVPContinuousHandlerVector.size(), (int)TVPEventDisabled);
	}
#endif

	if(TVPContinuousEventVector.size())
	{
		bool emptyflag = false;
		for(tjs_uint32 i = 0; i < TVPContinuousEventVector.size(); i++)
		{
			// note that the handler can remove itself while the event
			if(TVPContinuousEventVector[i])
				TVPContinuousEventVector[i]->OnContinuousCallback(tick);
			else
				emptyflag = true;

			if(TVPExclusiveEventPosted) return;  // check exclusive events
		}

		if(emptyflag)
		{
			// the array has empty cell

			// eliminate empty
            std::vector<tTVPContinuousEventCallbackIntf *>::iterator i;
			for(i = TVPContinuousEventVector.begin();
				i !=TVPContinuousEventVector.end();)
			{
				if(*i == NULL)
					i = TVPContinuousEventVector.erase(i);
				else
					i++;
			}
		}
	}

	if(!TVPEventDisabled && TVPContinuousHandlerVector.size())
	{
		bool emptyflag = false;
		tTJSVariant vtick((tjs_int64)tick);
		tTJSVariant *pvtick = &vtick;
#ifdef __EMSCRIPTEN__
		static int ch_trace_count = 0;
		ch_trace_count++;
		bool do_trace = (ch_trace_count <= 3 || ch_trace_count % 2000 == 0);
#endif
		for(tjs_uint i = 0; i < TVPContinuousHandlerVector.size(); i++)
		{
			if(TVPContinuousHandlerVector[i].Object)
			{
#ifdef __EMSCRIPTEN__
				if (do_trace) {
					iTJSDispatch2 *ot = TVPContinuousHandlerVector[i].ObjThis;
					if (ot) {
						tTJSVariant classname;
						tjs_error cnr = ot->ClassInstanceInfo(TJS_CII_GET, 0, &classname);
						std::string cn_str = "<no-class>";
						if (TJS_SUCCEEDED(cnr) && classname.Type() == tvtString) {
							cn_str = ttstr(classname.GetString()).AsNarrowStdString();
						}
						tTJSVariant name_val;
						tjs_error nr = ot->PropGet(TJS_MEMBERMUSTEXIST, TJS_W("name"), NULL, &name_val, ot);
						std::string name_str = "(none)";
						if (TJS_SUCCEEDED(nr) && name_val.Type() == tvtString) {
							name_str = ttstr(name_val.GetString()).AsNarrowStdString();
						}
						tTJSVariant cond_val;
						tjs_error cr = ot->PropGet(TJS_MEMBERMUSTEXIST, TJS_W("conductor"), NULL, &cond_val, ot);
						KRKR_LOG_L3("[CH-TRACE] #%d h[%u] class=%s name=%s hasCond=%d\n",
							ch_trace_count, i, cn_str.c_str(), name_str.c_str(),
							TJS_SUCCEEDED(cr) ? 1 : 0);
					} else {
						KRKR_LOG_L3("[CH-TRACE] #%d h[%u] objthis=NULL\n",
							ch_trace_count, i);
					}
				}
#endif
				tjs_error er;
				try
				{
					er =
						TVPContinuousHandlerVector[i].FuncCall(0, NULL, NULL, NULL, 1, &pvtick, NULL);
				}
				catch(eTJS &e)
				{
#if defined(__EMSCRIPTEN__)
					static int exc_count = 0;
					exc_count++;
					if (exc_count <= 10) {
						ttstr msg = e.GetMessage();
						iTJSDispatch2 *ot = TVPContinuousHandlerVector[i].ObjThis;
						std::string cn_str = "<no-class>";
						std::string name_str = "(none)";
						if (ot) {
							tTJSVariant classname;
							tjs_error cnr = ot->ClassInstanceInfo(TJS_CII_GET, 0, &classname);
							if (TJS_SUCCEEDED(cnr) && classname.Type() == tvtString) {
								cn_str = ttstr(classname.GetString()).AsNarrowStdString();
							}
							tTJSVariant name_val;
							tjs_error nr = ot->PropGet(TJS_MEMBERMUSTEXIST, TJS_W("name"), NULL, &name_val, ot);
							if (TJS_SUCCEEDED(nr) && name_val.Type() == tvtString) {
								name_str = ttstr(name_val.GetString()).AsNarrowStdString();
							}
						}
						fprintf(stderr, "[CH-EXC] handler[%u] class=%s name=%s TJS exception (#%d): %ls\n",
							i, cn_str.c_str(), name_str.c_str(), exc_count, msg.c_str());
					}
					continue;
#else
					TVPContinuousHandlerVector[i].Release();
					TVPContinuousHandlerVector[i].Object =
					TVPContinuousHandlerVector[i].ObjThis = NULL;
					throw;
#endif
				}
				catch(...)
				{
					TVPContinuousHandlerVector[i].Release();
					TVPContinuousHandlerVector[i].Object =
					TVPContinuousHandlerVector[i].ObjThis = NULL;
					throw;
				}
				if(TJS_FAILED(er))
				{
					TVPContinuousHandlerVector[i].Release();
					TVPContinuousHandlerVector[i].Object =
					TVPContinuousHandlerVector[i].ObjThis = NULL;
					emptyflag = true;
				}
				if(TVPExclusiveEventPosted) return;
			}
			else
			{
				emptyflag = true;
			}

		}

		if(emptyflag)
		{
			// eliminate empty
            std::vector<tTJSVariantClosure>::iterator i;
			for(i = TVPContinuousHandlerVector.begin();
				i !=TVPContinuousHandlerVector.end();)
			{
				if(!i->Object)
				{
					i->Release();
					i = TVPContinuousHandlerVector.erase(i);
				}
				else
				{
					i++;
				}
			}
		}
	}

	if(!TVPContinuousEventVector.size() && !TVPContinuousHandlerVector.size())
		TVPEndContinuousEvent();
}
//---------------------------------------------------------------------------
void TVPDeliverContinuousEvent()
{
	if(TVPContinuousEventProcessing) return;
	TVPContinuousEventProcessing = true;
	try
	{
		try
		{
			try
			{
				_TVPDeliverContinuousEvent();
			}
			catch(...)
			{
				TVPContinuousEventProcessing = false;
				throw;
			}
		}
		TJS_CONVERT_TO_TJS_EXCEPTION
	}
	TVP_CATCH_AND_SHOW_SCRIPT_EXCEPTION(TJS_W("continuous event"));

	TVPContinuousEventProcessing = false;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TVPAddContinuousHandler(tTJSVariantClosure clo)
{
	std::vector<tTJSVariantClosure>::iterator i;
	i = std::find(TVPContinuousHandlerVector.begin(),
		TVPContinuousHandlerVector.end(), clo);
	if(i == TVPContinuousHandlerVector.end())
	{
#ifdef __EMSCRIPTEN__
		iTJSDispatch2 *ot = clo.ObjThis;
		std::string cn = "<unknown>";
		std::string nm = "(none)";
		if (ot) {
			tTJSVariant cv;
			if (TJS_SUCCEEDED(ot->ClassInstanceInfo(TJS_CII_GET, 0, &cv)) && cv.Type() == tvtString)
				cn = ttstr(cv.GetString()).AsNarrowStdString();
			tTJSVariant nv;
			if (TJS_SUCCEEDED(ot->PropGet(TJS_MEMBERMUSTEXIST, TJS_W("name"), NULL, &nv, ot)) && nv.Type() == tvtString)
				nm = ttstr(nv.GetString()).AsNarrowStdString();
		}
			KRKR_LOG_L3("[CH-ADD] Adding handler class=%s name=%s obj=%p objthis=%p total=%d\n",
				cn.c_str(), nm.c_str(), (void*)clo.Object, (void*)clo.ObjThis,
				(int)TVPContinuousHandlerVector.size() + 1);
#endif
		TVPBeginContinuousEvent();
		clo.AddRef();
		TVPContinuousHandlerVector.push_back(clo);
	}
}
//---------------------------------------------------------------------------
void TVPRemoveContinuousHandler(tTJSVariantClosure clo)
{
	std::vector<tTJSVariantClosure>::iterator i;
	i = std::find(TVPContinuousHandlerVector.begin(),
		TVPContinuousHandlerVector.end(), clo);
	if(i != TVPContinuousHandlerVector.end())
	{
#ifdef __EMSCRIPTEN__
		int idx = (int)(i - TVPContinuousHandlerVector.begin());
		iTJSDispatch2 *ot = clo.ObjThis;
		std::string cn = "<unknown>";
		std::string nm = "(none)";
		if (ot) {
			tTJSVariant cv;
			if (TJS_SUCCEEDED(ot->ClassInstanceInfo(TJS_CII_GET, 0, &cv)) && cv.Type() == tvtString)
				cn = ttstr(cv.GetString()).AsNarrowStdString();
			tTJSVariant nv;
			if (TJS_SUCCEEDED(ot->PropGet(TJS_MEMBERMUSTEXIST, TJS_W("name"), NULL, &nv, ot)) && nv.Type() == tvtString)
				nm = ttstr(nv.GetString()).AsNarrowStdString();
		}
			KRKR_LOG_L3("[CH-REMOVE] Removing handler[%d] class=%s name=%s obj=%p objthis=%p remaining=%d\n",
				idx, cn.c_str(), nm.c_str(), (void*)clo.Object, (void*)clo.ObjThis,
				(int)TVPContinuousHandlerVector.size() - 1);
#endif
		i->Release();
		i->Object = i->ObjThis = NULL;
	}
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// "Compact" Event Delivering related
//---------------------------------------------------------------------------
// Compact events are to be delivered when:
// 1. the application is in idle state for long duration
// 2. the application had been deactivated ( application has lost the focus )
// 3. the application had been minimized
// these are to reduce memory usage, like garbage collection, cache cleaning,
// or etc ...
//---------------------------------------------------------------------------
static std::vector<tTVPCompactEventCallbackIntf *> TVPCompactEventVector;
//---------------------------------------------------------------------------
void TVPAddCompactEventHook(tTVPCompactEventCallbackIntf *cb)
{
	TVPCompactEventVector.push_back(cb);
}
//---------------------------------------------------------------------------
void TVPRemoveCompactEventHook(tTVPCompactEventCallbackIntf *cb)
{
	std::vector<tTVPCompactEventCallbackIntf *>::iterator i;
	for(i = TVPCompactEventVector.begin();
		i !=TVPCompactEventVector.end();)
	{
		if(cb == *i) *i = NULL; // simply assign a null
		i++;
	}
}
//---------------------------------------------------------------------------
void TVPDeliverCompactEvent(tjs_int level)
{
	// must be called by each platforms's implementation
	//std::vector<tTVPCompactEventCallbackIntf *>::iterator i;
	if(TVPCompactEventVector.size())
	{
		bool emptyflag = false;
		for(tjs_uint i = 0; i < TVPCompactEventVector.size(); i ++)
		{
			// note that the handler can remove itself while the event
			try
			{
				try
				{
					if(TVPCompactEventVector[i])
						TVPCompactEventVector[i]->OnCompact(level); else emptyflag = true;
				}
				TJS_CONVERT_TO_TJS_EXCEPTION
			}
			TVP_CATCH_AND_SHOW_SCRIPT_EXCEPTION_FORCE_SHOW_EXCEPTION(TJS_W("Compact Event"));
		}

		if(emptyflag)
		{
			// the array has empty cell

			// eliminate empty
			std::vector<tTVPCompactEventCallbackIntf *>::iterator i;
			for(i = TVPCompactEventVector.begin();
				i !=TVPCompactEventVector.end();)
			{
				if(*i == NULL)
					i = TVPCompactEventVector.erase(i);
				else
					i++;
			}
		}
	}
	if(level >= TVP_COMPACT_LEVEL_MAX) TVPFlushPendingSystemVariablesSave("compact-max", false);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// AsyncTrigger related
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// tTJSNI_AsyncTrigger
//---------------------------------------------------------------------------
tTJSNI_AsyncTrigger::tTJSNI_AsyncTrigger()
{
	Owner = NULL;
	Cached = true;
	IdlePendingCount = 0;
	Mode = atmNormal;
	ActionOwner.Object = ActionOwner.ObjThis = NULL;
	ActionName = TVPActionName;
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD
		tTJSNI_AsyncTrigger::Construct(tjs_int numparams, tTJSVariant **param,
			iTJSDispatch2 *tjs_obj)
{
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	tjs_error hr = inherited::Construct(numparams, param, tjs_obj);
	if(TJS_FAILED(hr)) return hr;

	if(numparams >= 2 && param[1]->Type() != tvtVoid)
		ActionName = *param[1]; // action function to be called

	ActionOwner = param[0]->AsObjectClosure();
	Owner = tjs_obj;

	return TJS_S_OK;
}
//---------------------------------------------------------------------------
void TJS_INTF_METHOD tTJSNI_AsyncTrigger::Invalidate()
{
	TVPCancelSourceEvents(Owner);
	Owner = NULL;

	ActionOwner.Release();
	ActionOwner.ObjThis = ActionOwner.Object = NULL;

	inherited::Invalidate();
}
//---------------------------------------------------------------------------
void tTJSNI_AsyncTrigger::Trigger()
{
	// trigger event
	if(Owner)
	{
		if(Cached)
		{
			// remove undelivered events from queue when "Cached" flag is set
			TVPCancelSourceEvents(Owner);
		}
		static ttstr eventname(TJS_W("onFire"));

		tjs_uint32 flags = TVP_EPT_POST;
		if(Mode == atmExclusive) flags |= TVP_EPT_EXCLUSIVE;  // fire exclusive event
		if(Mode == atmAtIdle)    flags |= TVP_EPT_IDLE;       // fire idle event

		TVPPostEvent(Owner, Owner, eventname, 0, flags, 0, NULL);
	}
}
//---------------------------------------------------------------------------
void tTJSNI_AsyncTrigger::Cancel()
{
	// cancel event
	if(Owner) TVPCancelSourceEvents(Owner);
	IdlePendingCount = 0;
}
//---------------------------------------------------------------------------
void tTJSNI_AsyncTrigger::SetCached(bool b)
{
	// set cached operation flag.
	// when this flag is set, only one event is delivered at once.
	if(Cached != b)
	{
		Cached = b;
		Cancel(); // all events are canceled
	}
}
//---------------------------------------------------------------------------
void tTJSNI_AsyncTrigger::SetMode(tTVPAsyncTriggerMode m)
{
	if(Mode != m)
	{
		Mode = m;
		Cancel();
	}
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// tTJSNC_AsyncTrigger
//---------------------------------------------------------------------------
tjs_uint32 tTJSNC_AsyncTrigger::ClassID = -1;
tTJSNC_AsyncTrigger::tTJSNC_AsyncTrigger() : inherited(TJS_W("AsyncTrigger"))
{
	// registration of native members

	TJS_BEGIN_NATIVE_MEMBERS(AsyncTrigger) // constructor
	TJS_DECL_EMPTY_FINALIZE_METHOD
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var.name*/_this, /*var.type*/tTJSNI_AsyncTrigger,
	/*TJS class name*/AsyncTrigger)
{
	return TJS_S_OK;
}
TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/AsyncTrigger)
//----------------------------------------------------------------------

//-- methods

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/trigger)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_AsyncTrigger);
	_this->Trigger();
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/trigger)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/cancel)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_AsyncTrigger);
	_this->Cancel();
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/cancel)
//----------------------------------------------------------------------

//-- events

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/onFire)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
		/*var. type*/tTJSNI_AsyncTrigger);

	tTJSVariantClosure obj = _this->GetActionOwnerNoAddRef();
	if(obj.Object)
	{
		ttstr & actionname = _this->GetActionName();
		TVP_ACTION_INVOKE_BEGIN(0, "onFire", objthis);
		TVP_ACTION_INVOKE_END_NAME(obj,
			actionname.IsEmpty() ? NULL :actionname.c_str(),
			actionname.IsEmpty() ? NULL :actionname.GetHint());
	}

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/onFire)
//----------------------------------------------------------------------

//--properties

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(cached)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_AsyncTrigger);
		*result = _this->GetCached();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_AsyncTrigger);
		_this->SetCached(*param);
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(cached)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(mode)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_AsyncTrigger);
		*result = (tjs_int)_this->GetMode();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_AsyncTrigger);
		_this->SetMode((tTVPAsyncTriggerMode)(tjs_int)*param);
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(mode)
//----------------------------------------------------------------------
	TJS_END_NATIVE_MEMBERS

}
//---------------------------------------------------------------------------
tTJSNativeInstance *tTJSNC_AsyncTrigger::CreateNativeInstance()
{
	return new tTJSNI_AsyncTrigger();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_AsyncTrigger()
{
	return new tTJSNC_AsyncTrigger();
}
//---------------------------------------------------------------------------
