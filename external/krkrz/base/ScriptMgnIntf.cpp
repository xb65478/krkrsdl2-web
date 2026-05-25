//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// TJS2 Script Managing
//---------------------------------------------------------------------------

#include "tjsCommHead.h"
#include <cstring>
#include <string>

#include "tjs.h"
#include "LogFilter.h"
#include "tjsDebug.h"
#include "tjsArray.h"
#include "ScriptMgnIntf.h"
#include "StorageIntf.h"
#include "DebugIntf.h"
#include "WindowIntf.h"
#include "LayerIntf.h"
#include "WaveIntf.h"
#include "TimerIntf.h"
#include "EventIntf.h"
#include "SystemIntf.h"
#include "PluginIntf.h"
#include "ClipboardIntf.h"
#include "PadIntf.h"
#include "MsgIntf.h"
#include "VideoOvlIntf.h"
#include "TextStream.h"
#include "Random.h"
#include "tjsRandomGenerator.h"
#include "SysInitIntf.h"
#include "PhaseVocoderFilter.h"
#ifdef WIN32
#endif
#if 1
#include "BasicDrawDevice.h"
#endif
#include "BinaryStream.h"
#include "SysInitImpl.h"
#include "SystemControl.h"
#include "Application.h"

#include "RectItf.h"
#include "MenuItemIntf.h"
#include "ImageFunction.h"
#include "BitmapIntf.h"
#include "tjsScriptBlock.h"
#if 0
#include "ApplicationSpecialPath.h"
#endif
#include "SystemImpl.h"
#include "BitmapLayerTreeOwner.h"
#include "Extension.h"

#ifdef KRKRZ_ENABLE_CANVAS
#include "CanvasIntf.h"
#include "OffscreenIntf.h"
#include "TextureIntf.h"
#include "Matrix44Intf.h"
#include "Matrix32Intf.h"
#include "ShaderProgramIntf.h"
#include "VertexBufferIntf.h"
#include "VertexBinderIntf.h"
#endif

//---------------------------------------------------------------------------
// global variables
//---------------------------------------------------------------------------
tTJS *TVPScriptEngine = NULL;
ttstr TVPStartupScriptName(TJS_W("startup.tjs"));
static ttstr TVPScriptTextEncoding(TJS_W("UTF-8"));
//---------------------------------------------------------------------------

#ifdef __EMSCRIPTEN__
static bool TVPIsDracuBringupEnabled()
{
	return TVPGetCommandLine(TJS_W("-krkr-dracu-bringup"));
}

static bool TVPWideStringContains(const ttstr &text, const tjs_char *needle)
{
	return needle && TJS_strstr(text.c_str(), needle) != NULL;
}

static bool TVPReplaceAndReport(ttstr &buffer, const tjs_char *from, const tjs_char *to)
{
	if(!TVPWideStringContains(buffer, from)) return false;
	buffer.Replace(from, to, false);
	return true;
}

static void TVPLogDracuScriptHack(const char *state, const ttstr &shortname)
{
	static int enabled_count = 0;
	static int disabled_count = 0;
	int &count = state && std::strcmp(state, "enabled") == 0 ? enabled_count : disabled_count;
	if (count++ >= 8) return;
	KRKR_LOG_L2(
		"[HACK-P0] id=P0-001 state=%s feature=script-rewrite script=%s target=DRACU flag=-krkr-dracu-bringup\n",
		state, shortname.AsNarrowStdString().c_str());
}

static void TVPLogDracuRewriteProbe(const ttstr &shortname, const ttstr &buffer)
{
	static int probe_count = 0;
	if (shortname != TJS_W("animkaglayer.tjs") &&
		shortname != TJS_W("affinesourcelayer.tjs") &&
		shortname != TJS_W("conductor.tjs")) return;
	if (probe_count++ >= 8) return;

	auto content = buffer.AsNarrowStdString();
	auto logLine = [&](const char *label, const char *needle) {
		auto pos = content.find(needle);
		fprintf(stderr, "[TJS-REWRITE] script=%s label=%s found=%d\n",
			shortname.AsNarrowStdString().c_str(), label, pos == std::string::npos ? 0 : 1);
		if (pos == std::string::npos) return;
		size_t end = content.find('\n', pos);
		if (end == std::string::npos) end = content.size();
		std::string line = content.substr(pos, end - pos);
		fprintf(stderr, "[TJS-REWRITE] %s >>>%s<<<\n", label, line.c_str());
	};

	logLine("motionPlaying-init", "var _motionPlaying = false;");
	logLine("setMotionVariable-guard", "if (_motionVariables === void || typeof _motionVariables != \"Object\") _motionVariables = %[];");
	logLine("motion-info-variable-bridge", "function getMotionVariable(name) { return getVariable(name); }");
		logLine("motion-area-variable-bridge", "var __motionOwnerGetVariable = global.Scripts.tryPropGet(__motionOwner, \"getVariable\", void);");
		logLine("handler-context", "if (_motionWorkMotionHandler === void) _motionWorkMotionHandler = workMotion incontextof this;");
		logLine("owner-variable-guard", "var __motionOwner = global.Scripts.tryPropGet(this, \"owner\", void);");
		logLine("inline-player-guard", "if (typeof mplayer != \"Object\") return _motionVariables[name];");
		logLine("inline-getVariable-guard", "var __mplayerGetVariable = global.Scripts.tryPropGet(__mplayer, \"getVariable\", void);");
		logLine("workMotion-guard", "if (__mplayer === void || typeof __mplayer != \"Object\") {");
		logLine("workMotion-self", "var __motionSelf = this;");
		logLine("motion-helper", "function _motionTryPropGet(obj, name, defval=void) {");
		logLine("motionWorking-getter", "var __mplayer = global.Scripts.tryPropGet(this, \"mplayer\", void);");
		logLine("motionPlaying-getter", "var __selfMotionPlaying = global.Scripts.tryPropGet(this, \"_motionPlaying\", false) ? true : false;");
		logLine("checkStopMotion-guard", "var __motionPlayingState = global.Scripts.tryPropGet(this, \"_motionPlaying\", false) ? true : false;");
	logLine("affine-calc-guard", "if (typeof _owners == \"undefined\" || _owners === void || typeof _owners != \"Object\") return;");
	logLine("affine-owner-guard", "var __affineOwnerCount = _owners.count;");
	logLine("conductor-interval-guard", "if (timer !== void && typeof timer == \"Object\") timer.interval = 0;");
	logLine("conductor-enabled-guard", "if (timer !== void && typeof timer == \"Object\") timer.enabled = false;");
}

static void TVPLogDracuFinalScriptWindow(const ttstr &shortname, const ttstr &buffer)
{
	static int window_count = 0;
	if (window_count++ >= 4) return;
	if (shortname != TJS_W("animkaglayer.tjs") &&
		shortname != TJS_W("motionaffinesourcelayer.tjs") &&
		shortname != TJS_W("kagenvimage.tjs")) return;
	auto content = buffer.AsNarrowStdString();
	size_t pos = 0;
	int line = 1;
	while (pos <= content.size()) {
		size_t next = content.find('\n', pos);
		if (next == std::string::npos) next = content.size();
		std::string lineText = content.substr(pos, next - pos);
		bool dump = false;
		if (shortname == TJS_W("animkaglayer.tjs")) {
			dump = (line >= 132 && line <= 152) ||
				(line >= 1378 && line <= 1394) ||
				(line >= 1566 && line <= 1582) ||
				lineText.find("__mplayerGetVariable") != std::string::npos ||
				lineText.find("__getCommandListFunc") != std::string::npos ||
				lineText.find("checkStopMotion") != std::string::npos;
			} else {
				dump = (line >= 98 && line <= 118) ||
					(line >= 238 && line <= 282) ||
					lineText.find("_motionAffinePlayerAlive") != std::string::npos;
			}
			if (shortname == TJS_W("kagenvimage.tjs")) {
				dump = (line >= 210 && line <= 235) ||
					lineText.find("__targetLayer") != std::string::npos;
			}
		if (dump) {
			fprintf(stderr, "[TJS-FINAL] %s:%d >>>%s<<<\n",
				shortname.AsNarrowStdString().c_str(), line, lineText.c_str());
		}
		if (next == content.size()) break;
		pos = next + 1;
		line++;
	}
}

static ttstr TVPGetMotionLayerHelperCompatScript()
{
	return TJS_W(
		"\r\n"
		" function getMotionLayerGetter(player, name) {"
		"  if (player !== void) {"
		"   if (name.indexOf(\"/\") >= 0) {"
		"    var direct = player.getLayerGetter(name);"
		"    if (direct !== void) return direct;"
		"    name = name.split(\"/\");"
		"    for (var i = 0; i < name.count - 1; i++) {"
		"     player = player.getLayerMotion(name[i]);"
		"     if (player === void) return void;"
		"    }"
		"    return player.getLayerGetter(name[name.count - 1]);"
		"   } else {"
		"    return player.getLayerGetter(name);"
		"   }"
		"  }"
		" }"
		" function getMotionLayerMotion(player, name) {"
		"  var lgetter = getMotionLayerGetter(player, name);"
		"  if (lgetter !== void) return lgetter.motion;"
		" }"
		" function getMotionLayerShape(player, name) {"
		"  var lgetter = getMotionLayerGetter(player, name);"
		"  if (lgetter !== void) return lgetter.shape;"
		" }"
		" if (typeof global != \"undefined\") {"
		"  if (typeof global.getMotionLayerGetter == \"undefined\") global.getMotionLayerGetter = getMotionLayerGetter;"
		"  if (typeof global.getMotionLayerMotion == \"undefined\") global.getMotionLayerMotion = getMotionLayerMotion;"
		"  if (typeof global.getMotionLayerShape == \"undefined\") global.getMotionLayerShape = getMotionLayerShape;"
		" }"
		"\r\n");
}

static ttstr TVPGetMotionEventBridgeScript()
{
	bool burstTraceEnabled = TVPGetCommandLine(TJS_W("-krkr-motion-click-burst-trace"));
	bool motionTraceEnabled = TVPGetCommandLine(TJS_W("-krkr-motion-trace"));
	const tjs_char * burstInit = burstTraceEnabled ? TJS_W("true") : TJS_W("false");
	const tjs_char * motionTraceInit = motionTraceEnabled ? TJS_W("true") : TJS_W("false");
	ttstr script = TJS_W(
		"if (typeof global != \"undefined\" && typeof global.__krkrMotionEventBridge == \"undefined\") {\r\n"
		" global.__krkrMotionClickBurstTrace = ");
	script += burstInit;
	script += TJS_W(";\r\n"
		" if (typeof global.__krkrMotionClickBurstTrace == \"undefined\") global.__krkrMotionClickBurstTrace = false;\r\n"
		" global.__krkrDiagTraceSlotButtons = ");
	script += motionTraceInit;
	script += TJS_W(";\r\n"
		" if (typeof global.__krkrDiagTraceSlotButtons == \"undefined\") global.__krkrDiagTraceSlotButtons = false;\r\n"
		" if (typeof global.__krkrMotionVerboseLog == \"undefined\") global.__krkrMotionVerboseLog = false;\r\n"
		" global.__krkrMotionEventState = void;\r\n"
		" global.__krkrMotionEventHandlerActive = false;\r\n"
		" global.__krkrMotionEventHandler = void;\r\n"
		" global.__krkrMotionEventLayer = void;\r\n"
		" global.__krkrMotionEventResourceManager = void;\r\n"
		" function __krkrMotionEventLog(msg) {\r\n"
		"  try {\r\n"
		"   if (global.__krkrMotionVerboseLog === true || global.__krkrMotionVerboseLog === 1 || global.__krkrMotionVerboseLog === \"1\") Debug.message(msg);\r\n"
		"  } catch(e) {}\r\n"
		" }\r\n"
		" function __krkrMotionEventErrorLog(msg) {\r\n"
		"  try { Debug.message(msg); } catch(e) {}\r\n"
		" }\r\n"
		" function __krkrMotionBurstTraceEnabled() {\r\n"
		"  try {\r\n"
		"   if (typeof global == \"undefined\" || global == void) return false;\r\n"
		"   var v = global.__krkrMotionClickBurstTrace;\r\n"
		"   return v === true || v === 1 || v === \"1\";\r\n"
		"  } catch(e) { return false; }\r\n"
		" }\r\n"
		" function __krkrMotionBurstLog(msg) {\r\n"
		"  try {\r\n"
		"   if (__krkrMotionBurstTraceEnabled()) Debug.message(msg);\r\n"
		"  } catch(e) {}\r\n"
		" }\r\n"
		" function __krkrMotionEventGet(elm, name, defval=void) {\r\n"
		"  if (elm === void || (typeof elm) != \"Object\") return defval;\r\n"
		"  try { var v = elm[name]; return v === void ? defval : v; } catch(e) { return defval; }\r\n"
		" }\r\n"
		" function __krkrMotionEventText(v) { return v === void ? \"\" : \"\" + v; }\r\n"
		" function __krkrMotionEventTruthy(v) {\r\n"
		"  if (v === void) return false;\r\n"
		"  if ((typeof v) == \"String\") return v != \"\" && v != \"false\" && v != \"0\";\r\n"
		"  return v ? true : false;\r\n"
		" }\r\n"
		" function __krkrMotionEventLayerText(layer) {\r\n"
		"  if (layer === void || (typeof layer) != \"Object\") return \"layer=void\";\r\n"
		"  var l=\"?\", t=\"?\", w=\"?\", h=\"?\", v=\"?\", o=\"?\";\r\n"
		"  try { l = \"\" + layer.left; } catch(e) {}\r\n"
		"  try { t = \"\" + layer.top; } catch(e) {}\r\n"
		"  try { w = \"\" + layer.width; } catch(e) {}\r\n"
		"  try { h = \"\" + layer.height; } catch(e) {}\r\n"
		"  try { v = \"\" + layer.visible; } catch(e) {}\r\n"
		"  try { o = \"\" + layer.opacity; } catch(e) {}\r\n"
		"  return \"layer=\" + l + \",\" + t + \" \" + w + \"x\" + h + \" visible=\" + v + \" opacity=\" + o;\r\n"
		" }\r\n"
		" function __krkrMotionEventTrace(phase, state, extra=void) {\r\n"
		"  var suffix = extra === void ? \"\" : (\" \" + extra);\r\n"
		"  if (state === void || (typeof state) != \"Object\") {\r\n"
		"   __krkrMotionEventLog(\"[KAG-MOTION-EV] \" + phase + \" state=void\" + suffix);\r\n"
		"   return;\r\n"
		"  }\r\n"
		"  __krkrMotionEventLog(\"[KAG-MOTION-EV] \" + phase + \" storage=\" + state.storage + \" motion=\" + state.motion + \" playing=\" + state.playing + \" waiting=\" + state.waiting + \" drawCount=\" + state.drawCount + \" \" + __krkrMotionEventLayerText(state.layer) + suffix);\r\n"
		" }\r\n"
		" function __krkrMotionEventEnsureMotion() {\r\n"
		"  if (typeof Motion == \"undefined\" || typeof Motion.ResourceManager == \"undefined\" || typeof Motion.Player == \"undefined\") {\r\n"
		"   try { Plugins.link(\"motionplayer.dll\"); } catch(e) { __krkrMotionEventErrorLog(\"[KAG-MOTION-EV] link failed \" + e.message); }\r\n"
		"  }\r\n"
		"  return typeof Motion != \"undefined\" && typeof Motion.ResourceManager != \"undefined\" && typeof Motion.Player != \"undefined\";\r\n"
		" }\r\n"
		" function __krkrMotionEventLayerSize() {\r\n"
		"  var win = (typeof Window != \"undefined\") ? Window.mainWindow : void;\r\n"
		"  var base = (win !== void && (typeof win) == \"Object\") ? win.primaryLayer : void;\r\n"
		"  var bw = 1280;\r\n"
		"  var bh = 720;\r\n"
		"  try { if (win !== void && win.scWidth > 0) bw = win.scWidth; } catch(e) {}\r\n"
		"  try { if (win !== void && win.scHeight > 0) bh = win.scHeight; } catch(e) {}\r\n"
		"  try { if (base !== void && base.width > 0) bw = base.width; } catch(e) {}\r\n"
		"  try { if (base !== void && base.height > 0) bh = base.height; } catch(e) {}\r\n"
		"  return %[win: win, base: base, width: bw, height: bh];\r\n"
		" }\r\n"
		" function __krkrMotionEventEnsureLayer() {\r\n"
		"  var size = __krkrMotionEventLayerSize();\r\n"
		"  var win = size.win;\r\n"
		"  var base = size.base;\r\n"
		"  if (win === void || base === void || (typeof base) != \"Object\") return void;\r\n"
		"  var bw = size.width;\r\n"
		"  var bh = size.height;\r\n"
		"  var layer = global.__krkrMotionEventLayer;\r\n"
		"  if (layer === void || (typeof layer) != \"Object\") {\r\n"
		"   var LayerClass = (typeof global.__krkrNativeLayer != \"undefined\") ? global.__krkrNativeLayer : Layer;\r\n"
		"   layer = new LayerClass(win, base);\r\n"
		"   layer.name = \"__krkrMotionEventLayer\";\r\n"
		"   global.__krkrMotionEventLayer = layer;\r\n"
		"  }\r\n"
		"  try { layer.setPos(0, 0); } catch(e) {}\r\n"
		"  try { layer.setSize(bw, bh); } catch(e) {}\r\n"
		"  try { layer.setImageSize(bw, bh); } catch(e) {}\r\n"
		"  try { layer.type = ltAlpha; } catch(e) { try { layer.type = 1; } catch(e2) {} }\r\n"
		"  try { layer.face = dfAlpha; } catch(e) { try { layer.face = 1; } catch(e2) {} }\r\n"
		"  try { layer.opacity = 255; } catch(e) {}\r\n"
		"  try { layer.visible = true; } catch(e) {}\r\n"
		"  try { layer.bringToFront(); } catch(e) {}\r\n"
		"  return layer;\r\n"
		" }\r\n"
		" function __krkrMotionEventHideLayer(reason=void) {\r\n"
		"  var layer = global.__krkrMotionEventLayer;\r\n"
		"  if (layer === void || (typeof layer) != \"Object\") return;\r\n"
		"  var oldW = 0, oldH = 0;\r\n"
		"  try { oldW = layer.width; oldH = layer.height; } catch(e) {}\r\n"
		"  if (!(oldW > 0)) oldW = 1280;\r\n"
		"  if (!(oldH > 0)) oldH = 720;\r\n"
		"  __krkrMotionEventLog(\"[KAG-MOTION-EV] hide-layer clear=0 winUpdate=0 reason=\" + (reason === void ? \"manual\" : reason) + \" \" + __krkrMotionEventLayerText(layer));\r\n"
		"  try { layer.opacity = 0; } catch(e) {}\r\n"
		"  try { layer.visible = false; } catch(e) {}\r\n"
		"  try { layer.update(0, 0, oldW, oldH); } catch(e) {}\r\n"
		"  try { layer.setPos(-8192, -8192); } catch(e) {}\r\n"
		"  try { layer.setSize(1, 1); } catch(e) {}\r\n"
		"  try { layer.setImageSize(1, 1); } catch(e) {}\r\n"
		"  try { layer.update(0, 0, 1, 1); } catch(e) {}\r\n"
		" }\r\n"
		" function __krkrMotionEventDraw(interval) {\r\n"
		"  var state = global.__krkrMotionEventState;\r\n"
		"  if (state === void || (typeof state) != \"Object\") return false;\r\n"
		"  var player = state.player;\r\n"
		"  if (player === void || (typeof player) != \"Object\") return false;\r\n"
		"  var layer = __krkrMotionEventEnsureLayer();\r\n"
		"  if (layer === void) return false;\r\n"
		"  var dt = interval === void ? 0 : +interval;\r\n"
		"  if (dt < 0 || dt > 1000) dt = 0;\r\n"
		"  var wasPlaying = state.playing ? true : false;\r\n"
		"  if (state.drawCount === void) state.drawCount = 0;\r\n"
		"  state.drawCount++;\r\n"
		"  if (state.drawCount <= 2) __krkrMotionEventTrace(\"draw-begin\", state, \"dt=\" + dt);\r\n"
		"  try { player.progress(dt); } catch(e) { __krkrMotionEventErrorLog(\"[KAG-MOTION-EV] progress failed storage=\" + state.storage + \" error=\" + e.message); return false; }\r\n"
		"  var stillPlaying = __krkrMotionEventRefreshPlaying(state, false, \"after-progress\");\r\n"
		"  var finalDraw = (!stillPlaying && wasPlaying);\r\n"
		"  if (!stillPlaying && !finalDraw) { __krkrMotionEventFinish(state, \"stopped-before-draw\"); return false; }\r\n"
		"  if (finalDraw) __krkrMotionEventTrace(\"draw-final\", state, \"dt=\" + dt + \" holdLayer=1\");\r\n"
		"  try { player.draw(layer); } catch(e) { __krkrMotionEventErrorLog(\"[KAG-MOTION-EV] draw failed storage=\" + state.storage + \" error=\" + e.message); return false; }\r\n"
		"  try { layer.bringToFront(); } catch(e) {}\r\n"
		"  try { layer.update(0, 0, layer.width, layer.height); } catch(e) {}\r\n"
		"  if (finalDraw) { __krkrMotionEventFinish(state, \"after-final-draw\"); return false; }\r\n"
		"  __krkrMotionEventRefreshPlaying(state, true, \"after-draw\");\r\n"
		"  return true;\r\n"
		" }\r\n"
		" function __krkrMotionEventStopHandler() {\r\n"
		"  try { if (global.__krkrMotionEventHandler !== void) System.removeContinuousHandler(global.__krkrMotionEventHandler); } catch(e) {}\r\n"
		"  global.__krkrMotionEventHandlerActive = false;\r\n"
		" }\r\n"
		" function __krkrMotionEventFinish(state=void, reason=void) {\r\n"
		"  if (state === void) state = global.__krkrMotionEventState;\r\n"
		"  if (state !== void && (typeof state) == \"Object\" && state.finished) return;\r\n"
		"  if (state !== void && (typeof state) == \"Object\") {\r\n"
		"   if (state.waiting) __krkrMotionEventLog(\"[KAG-MOTION-EV] waitmovie complete storage=\" + state.storage + \" motion=\" + state.motion);\r\n"
		"   state.waiting = false;\r\n"
		"   state.playing = false;\r\n"
		"   state.finished = true;\r\n"
		"   __krkrMotionEventTrace(\"finish\", state, \"reason=\" + (reason === void ? \"unknown\" : reason) + \" holdLayer=0 hide=1\");\r\n"
		"  }\r\n"
		"  __krkrMotionEventStopHandler();\r\n"
		"  __krkrMotionEventHideLayer(reason);\r\n"
		"  global.__krkrMotionEventState = void;\r\n"
		" }\r\n"
		" function __krkrMotionEventRefreshPlaying(state=void, finishOnStop=true, reason=void) {\r\n"
		"  if (state === void) state = global.__krkrMotionEventState;\r\n"
		"  if (state === void || (typeof state) != \"Object\") return false;\r\n"
		"  var player = state.player;\r\n"
		"  if (player === void || (typeof player) != \"Object\") { if (finishOnStop) __krkrMotionEventFinish(state, reason); return false; }\r\n"
		"  var playing = false;\r\n"
		"  try { playing = (player.playing || player.animating || player.allplaying || player.motionPlaying) ? true : false; } catch(e) { playing = false; }\r\n"
		"  state.playing = playing;\r\n"
		"  if (!playing && finishOnStop) __krkrMotionEventFinish(state, reason);\r\n"
		"  return playing;\r\n"
		" }\r\n"
		" function __krkrMotionEventShouldWait() {\r\n"
		"  var state = global.__krkrMotionEventState;\r\n"
		"  if (state === void || (typeof state) != \"Object\") return false;\r\n"
		"  if (!state.waiting) return false;\r\n"
		"  return __krkrMotionEventRefreshPlaying(state);\r\n"
		" }\r\n"
		" function __krkrMotionEventContinuous(tick) {\r\n"
		"  var state = global.__krkrMotionEventState;\r\n"
		"  if (state === void || (typeof state) != \"Object\") return;\r\n"
		"  var last = state.lastTick;\r\n"
		"  state.lastTick = tick;\r\n"
		"  if (last === void) {\r\n"
		"   __krkrMotionEventTrace(\"prime\", state, \"tick=\" + tick + \" draw=0\");\r\n"
		"   return;\r\n"
		"  }\r\n"
		"  var dt = last === void ? 0 : tick - last;\r\n"
		"  __krkrMotionBurstLog(\"[KAG-MOTION-BURST] continuous tick=\" + tick + \" dt=\" + dt + \" storage=\" + state.storage + \" motion=\" + state.motion + \" waiting=\" + state.waiting + \" playing=\" + state.playing + \" drawCount=\" + state.drawCount);\r\n"
		"  var ok = __krkrMotionEventDraw(dt);\r\n"
		"  if ((!ok || !state.playing) && global.__krkrMotionEventState === state) {\r\n"
		"   __krkrMotionEventFinish(state, \"continuous\");\r\n"
		"  }\r\n"
		" }\r\n"
		" function __krkrMotionEventStartHandler() {\r\n"
		"  if (global.__krkrMotionEventHandler === void || typeof global.__krkrMotionEventHandler == \"undefined\") {\r\n"
		"   global.__krkrMotionEventHandler = __krkrMotionEventContinuous;\r\n"
		"  }\r\n"
		"  if (!global.__krkrMotionEventHandlerActive) {\r\n"
		"   try { System.addContinuousHandler(global.__krkrMotionEventHandler); global.__krkrMotionEventHandlerActive = true; } catch(e) { __krkrMotionEventErrorLog(\"[KAG-MOTION-EV] handler failed \" + e.message); }\r\n"
		"  }\r\n"
		" }\r\n"
		" global.__krkrMotionEventBridge = function(elm) {\r\n"
		"  var storage = __krkrMotionEventText(__krkrMotionEventGet(elm, \"storage\", \"\"));\r\n"
		"  var chara = __krkrMotionEventText(__krkrMotionEventGet(elm, \"chara\", \"\"));\r\n"
		"  var motion = __krkrMotionEventText(__krkrMotionEventGet(elm, \"motion\", \"\"));\r\n"
		"  var waitmovie = __krkrMotionEventTruthy(__krkrMotionEventGet(elm, \"waitmovie\", void));\r\n"
		"  if (storage == \"\" && waitmovie) {\r\n"
		"   var state = global.__krkrMotionEventState;\r\n"
		"   var active = state !== void && (typeof state) == \"Object\";\r\n"
		"   if (active) { state.waiting = true; __krkrMotionEventStartHandler(); }\r\n"
		"   __krkrMotionBurstLog(\"[KAG-MOTION-BURST] waitmovie-toggle active=\" + active + \" playing=\" + (active ? state.playing : false));\r\n"
		"   __krkrMotionEventLog(\"[KAG-MOTION-EV] waitmovie state=\" + active + \" playing=\" + (active ? state.playing : false));\r\n"
		"   return active;\r\n"
		"  }\r\n"
		"  if (storage == \"\" || storage.toLowerCase().indexOf(\".psb\") < 0) return false;\r\n"
		"  if (!__krkrMotionEventEnsureMotion()) return false;\r\n"
		"  var layer = __krkrMotionEventEnsureLayer();\r\n"
		"  if (layer === void) { __krkrMotionEventErrorLog(\"[KAG-MOTION-EV] no layer storage=\" + storage); return false; }\r\n"
		"  try {\r\n"
		"   var win = Window.mainWindow;\r\n"
		"   var mgr = global.__krkrMotionEventResourceManager;\r\n"
		"   if (mgr === void || (typeof mgr) != \"Object\") {\r\n"
		"    mgr = new Motion.ResourceManager(win, 8);\r\n"
		"    global.__krkrMotionEventResourceManager = mgr;\r\n"
		"   }\r\n"
		"   mgr.load(storage);\r\n"
		"   var player = new Motion.Player(mgr);\r\n"
		"   if (chara != \"\") player.chara = chara;\r\n"
		"   try { player.visible = true; } catch(e) {}\r\n"
		"   try { player.opacity = 255; } catch(e) {}\r\n"
		"   try { player.setCoord(0, 0); } catch(e) {}\r\n"
		"   try { player.setScale(1.0); } catch(e) {}\r\n"
		"   var size = __krkrMotionEventLayerSize();\r\n"
		"   try { player.setAttachRect(0, 0, size.width, size.height); } catch(e) {}\r\n"
		"   var flag = 0;\r\n"
		"   try { flag = Motion.PlayFlagForce; } catch(e) { flag = 0; }\r\n"
		"   player.play(motion, flag);\r\n"
		"   global.__krkrMotionEventState = %[storage: storage, chara: chara, motion: motion, player: player, layer: layer, lastTick: void, playing: true, waiting: waitmovie, drawCount: 0, finished: false, primed: false];\r\n"
		"   __krkrMotionEventStartHandler();\r\n"
		"   __krkrMotionBurstLog(\"[KAG-MOTION-BURST] bridge-play storage=\" + storage + \" chara=\" + chara + \" motion=\" + motion + \" waitmovie=\" + waitmovie);\r\n"
		"   __krkrMotionEventLog(\"[KAG-MOTION-EV] play storage=\" + storage + \" chara=\" + chara + \" motion=\" + motion + \" waitmovie=\" + waitmovie);\r\n"
		"   return true;\r\n"
		"  } catch(e) {\r\n"
		"   __krkrMotionEventErrorLog(\"[KAG-MOTION-EV] failed storage=\" + storage + \" chara=\" + chara + \" motion=\" + motion + \" error=\" + e.message);\r\n"
		"   return false;\r\n"
		"  }\r\n"
		" };\r\n"
		" __krkrMotionEventLog(\"[KAG-MOTION-EV] bridge installed\");\r\n"
		"}\r\n");
	return script;
}
#endif


//---------------------------------------------------------------------------
// Garbage Collection stuff
//---------------------------------------------------------------------------
class tTVPTJSGCCallback : public tTVPCompactEventCallbackIntf
{
	void TJS_INTF_METHOD OnCompact(tjs_int level)
	{
		// OnCompact method from tTVPCompactEventCallbackIntf
		// called when the application is idle, deactivated, minimized, or etc...
		if(TVPScriptEngine)
		{
			if(level >= TVP_COMPACT_LEVEL_IDLE)
			{
				TVPScriptEngine->DoGarbageCollection();
			}
		}
	}
} static TVPTJSGCCallback;
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// TVPInitScriptEngine
//---------------------------------------------------------------------------
static bool TVPScriptEngineInit = false;
void TVPInitScriptEngine()
{
	if(TVPScriptEngineInit) return;
	TVPScriptEngineInit = true;

	tTJSVariant val;

	// Set eval expression mode
	if(TVPGetCommandLine(TJS_W("-evalcontext"), &val) )
	{
		ttstr str(val);
		if(str == TJS_W("global"))
		{
			TJSEvalOperatorIsOnGlobal = true;
			TJSWarnOnNonGlobalEvalOperator = true;
		}
	}

	// Set igonre-prop compat mode
	if(TVPGetCommandLine(TJS_W("-unaryaster"), &val) )
	{
		ttstr str(val);
		if(str == TJS_W("compat"))
		{
			TJSUnaryAsteriskIgnoresPropAccess = true;
		}
	}

	// Set debug mode
	if(TVPGetCommandLine(TJS_W("-debug"), &val) )
	{
		ttstr str(val);
		if(str == TJS_W("yes"))
		{
			TJSEnableDebugMode = true;
			TVPAddImportantLog((const tjs_char *)TVPWarnDebugOptionEnabled);
			TJSWarnOnExecutionOnDeletingObject = true;
		}
	}
	// Set Read text encoding
	if(TVPGetCommandLine(TJS_W("-readencoding"), &val) )
	{
		ttstr str(val);
		TVPSetDefaultReadEncoding( str );
	}
	TVPScriptTextEncoding = ttstr(TVPGetDefaultReadEncoding());

#ifdef TVP_START_UP_SCRIPT_NAME
	TVPStartupScriptName = TVP_START_UP_SCRIPT_NAME;
#else
	// Set startup script name
	if(TVPGetCommandLine(TJS_W("-startup"), &val) )
	{
		ttstr str(val);
		TVPStartupScriptName = str;
	}
#endif

	// create script engine object
	TVPScriptEngine = new tTJS();

	// add kirikiriz
	TVPScriptEngine->SetPPValue( TJS_W("kirikiriz"), 1 );

	// system definition
#ifdef WIN32
	TVPScriptEngine->SetPPValue( TJS_W("windows"), 1 );
#endif

#ifdef ANDROID
	TVPScriptEngine->SetPPValue( TJS_W("android"), 1 );
#endif

	// set TJSGetRandomBits128
	TJSGetRandomBits128 = TVPGetRandomBits128;

	// script system initialization
	TVPScriptEngine->ExecScript( TVPGetSystemInitializeScript() );

	// set console output gateway handler
	TVPScriptEngine->SetConsoleOutput(TVPGetTJS2ConsoleOutputGateway());


	// set text stream functions
	TJSCreateTextStreamForRead = TVPCreateTextStreamForRead;
	TJSCreateTextStreamForWrite = TVPCreateTextStreamForWrite;

	// set binary stream functions
	TJSCreateBinaryStreamForRead = TVPCreateBinaryStreamInterfaceForRead;
	TJSCreateBinaryStreamForWrite = TVPCreateBinaryStreamInterfaceForWrite;

	// register some TVP classes/objects/functions/propeties
	iTJSDispatch2 *dsp;
	iTJSDispatch2 *global = TVPScriptEngine->GetGlobalNoAddRef();


#define REGISTER_OBJECT(classname, instance) \
	KRKR_LOG_L3("REGISTER_OBJECT: calling instance for " #classname "\n"); \
	dsp = (instance); \
	KRKR_LOG_L3("REGISTER_OBJECT: got instance for " #classname "\n"); \
	val = tTJSVariant(dsp/*, dsp*/); \
	dsp->Release(); \
	KRKR_LOG_L3("REGISTER_OBJECT: calling PropSet for " #classname "\n"); \
	global->PropSet(TJS_MEMBERENSURE|TJS_IGNOREPROP, TJS_W(#classname), NULL, \
		&val, global); \
	KRKR_LOG_L3("REGISTER_OBJECT: done for " #classname "\n");

#include <stdio.h>
	/* classes */
#ifdef __EMSCRIPTEN__
#define LOG_STEP(name) do { KRKR_LOG_L3("Registering %s\n", name); } while(0)
#else
#define LOG_STEP(name)
#endif
	LOG_STEP("Debug");
	REGISTER_OBJECT(Debug, TVPCreateNativeClass_Debug());
	LOG_STEP("Font");
	REGISTER_OBJECT(Font, TVPCreateNativeClass_Font());
	LOG_STEP("Layer");
	REGISTER_OBJECT(Layer, TVPCreateNativeClass_Layer());
#ifdef __EMSCRIPTEN__
	// Some game frameworks shadow the public Layer symbol with their own
	// script-side layer class. Keep the native constructor reachable for
	// compatibility shims that must allocate a plain off-screen Layer.
	if(TJS_SUCCEEDED(global->PropGet(TJS_MEMBERMUSTEXIST, TJS_W("Layer"), NULL, &val, global)))
		global->PropSet(TJS_MEMBERENSURE|TJS_IGNOREPROP, TJS_W("__krkrNativeLayer"), NULL, &val, global);
#endif
	LOG_STEP("Timer");
	REGISTER_OBJECT(Timer, TVPCreateNativeClass_Timer());
	LOG_STEP("AsyncTrigger");
	REGISTER_OBJECT(AsyncTrigger, TVPCreateNativeClass_AsyncTrigger());
	LOG_STEP("System");
	REGISTER_OBJECT(System, TVPCreateNativeClass_System());
	LOG_STEP("Storages");
	REGISTER_OBJECT(Storages, TVPCreateNativeClass_Storages());
	LOG_STEP("Plugins");
	REGISTER_OBJECT(Plugins, TVPCreateNativeClass_Plugins());
	LOG_STEP("VideoOverlay");
	REGISTER_OBJECT(VideoOverlay, TVPCreateNativeClass_VideoOverlay());
	LOG_STEP("Clipboard");
	REGISTER_OBJECT(Clipboard, TVPCreateNativeClass_Clipboard());
	LOG_STEP("Pad");
	REGISTER_OBJECT(Pad, TVPCreateNativeClass_Pad());
	LOG_STEP("Scripts");
	REGISTER_OBJECT(Scripts, TVPCreateNativeClass_Scripts()); // declared in this file
	LOG_STEP("Rect");
	REGISTER_OBJECT(Rect, TVPCreateNativeClass_Rect());
#ifndef __EMSCRIPTEN__
	LOG_STEP("MenuItem");
	REGISTER_OBJECT(MenuItem, TVPCreateNativeClass_MenuItem());
#endif
	LOG_STEP("Bitmap");
	REGISTER_OBJECT(Bitmap, TVPCreateNativeClass_Bitmap());
	LOG_STEP("ImageFunction");
	REGISTER_OBJECT(ImageFunction, TVPCreateNativeClass_ImageFunction());
	LOG_STEP("BitmapLayerTreeOwner");
	REGISTER_OBJECT(BitmapLayerTreeOwner, TVPCreateNativeClass_BitmapLayerTreeOwner());
#ifdef KRKRZ_ENABLE_CANVAS
	LOG_STEP("Canvas");
	REGISTER_OBJECT(Canvas, TVPCreateNativeClass_Canvas());
	LOG_STEP("Texture");
	REGISTER_OBJECT(Texture, TVPCreateNativeClass_Texture());
	LOG_STEP("Offscreen");
	REGISTER_OBJECT(Offscreen, TVPCreateNativeClass_Offscreen());
	LOG_STEP("Matrix44");
	REGISTER_OBJECT(Matrix44, TVPCreateNativeClass_Matrix44());
	LOG_STEP("Matrix32");
	REGISTER_OBJECT(Matrix32, TVPCreateNativeClass_Matrix32());
	LOG_STEP("ShaderProgram");
	REGISTER_OBJECT(ShaderProgram, TVPCreateNativeClass_ShaderProgram());
	LOG_STEP("VertexBuffer");
	REGISTER_OBJECT(VertexBuffer, TVPCreateNativeClass_VertexBuffer());
	LOG_STEP("VertexBinder");
	REGISTER_OBJECT(VertexBinder, TVPCreateNativeClass_VertexBinder());
#endif
	LOG_STEP("Done Registering");

	/* WaveSoundBuffer and its filters */
	LOG_STEP("WaveSoundBuffer");
	iTJSDispatch2 * waveclass = NULL;
	REGISTER_OBJECT( WaveSoundBuffer, ( waveclass = TVPCreateNativeClass_SoundBuffer() ) );
	LOG_STEP("PhaseVocoder");
	dsp = new tTJSNC_PhaseVocoder();
	val = tTJSVariant(dsp);
	dsp->Release();
	waveclass->PropSet(TJS_MEMBERENSURE|TJS_IGNOREPROP|TJS_STATICMEMBER,
		TJS_W("PhaseVocoder"), NULL, &val, waveclass);

	/* Window and its drawdevices */
	LOG_STEP("Window");
	iTJSDispatch2 * windowclass = NULL;
	REGISTER_OBJECT(Window, (windowclass = TVPCreateNativeClass_Window()));
#ifdef WIN32
#endif
#if 1
	LOG_STEP("BasicDrawDevice");
	dsp = new tTJSNC_BasicDrawDevice();
	val = tTJSVariant(dsp);
	dsp->Release();
	windowclass->PropSet(TJS_MEMBERENSURE|TJS_IGNOREPROP|TJS_STATICMEMBER,
		TJS_W("BasicDrawDevice"), NULL, &val, windowclass);
#endif
	// Add Extension Classes
	LOG_STEP("TVPCauseAtInstallExtensionClass");
	TVPCauseAtInstallExtensionClass( global );

#ifdef __EMSCRIPTEN__
	try
	{
		TVPExecuteScript(TVPGetMotionEventBridgeScript(), TJS_W("krkr_motion_event_bridge.tjs"), 0, static_cast<tTJSVariant*>(nullptr));
	}
	catch(eTJS &e)
	{
		fprintf(stderr, "[KAG-MOTION-EV] bridge install failed: %s\n",
			e.GetMessage().AsNarrowStdString().c_str());
	}
	catch(...)
	{
		fprintf(stderr, "[KAG-MOTION-EV] bridge install failed: unknown\n");
	}
#endif

	// Garbage Collection Hook
	LOG_STEP("TVPAddCompactEventHook");
	TVPAddCompactEventHook(&TVPTJSGCCallback);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPUninitScriptEngine
//---------------------------------------------------------------------------
static bool TVPScriptEngineUninit = false;
void TVPUninitScriptEngine()
{
	if(TVPScriptEngineUninit) return;
	TVPScriptEngineUninit = true;

	TVPScriptEngine->Shutdown();
	TVPScriptEngine->Release();
	/*
		Objects, theirs lives are contolled by reference counter, may not be all
		freed here in some occations.
	*/
	TVPScriptEngine = NULL;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPRestartScriptEngine
//---------------------------------------------------------------------------
void TVPRestartScriptEngine()
{
	TVPUninitScriptEngine();
	TVPScriptEngineInit = false;
	TVPInitScriptEngine();
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPGetScriptEngine
//---------------------------------------------------------------------------
tTJS * TVPGetScriptEngine()
{
	return TVPScriptEngine;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPGetScriptDispatch
//---------------------------------------------------------------------------
iTJSDispatch2 * TVPGetScriptDispatch()
{
	if(TVPScriptEngine) return TVPScriptEngine->GetGlobal(); else return NULL;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPExecuteScript
//---------------------------------------------------------------------------
void TVPExecuteScript(const ttstr& content, tTJSVariant *result)
{
	if(TVPScriptEngine)
		TVPScriptEngine->ExecScript(content, result);
	else
		TVPThrowInternalError;
}
//---------------------------------------------------------------------------
void TVPExecuteScript(const ttstr& content, const ttstr &name, tjs_int lineofs, tTJSVariant *result)
{
	if(TVPScriptEngine)
		TVPScriptEngine->ExecScript(content, result, NULL, &name, lineofs);
	else
		TVPThrowInternalError;
}
//---------------------------------------------------------------------------
void TVPExecuteScript(const ttstr& content, iTJSDispatch2 *context, tTJSVariant *result)
{
	if(TVPScriptEngine)
		TVPScriptEngine->ExecScript(content, result, context);
	else
		TVPThrowInternalError;
}
//---------------------------------------------------------------------------
void TVPExecuteScript(const ttstr& content, const ttstr &name, tjs_int lineofs, iTJSDispatch2 *context, tTJSVariant *result)
{
	if(TVPScriptEngine)
		TVPScriptEngine->ExecScript(content, result, context, &name, lineofs);
	else
		TVPThrowInternalError;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPExecuteExpression
//---------------------------------------------------------------------------
void TVPExecuteExpression(const ttstr& content, tTJSVariant *result)
{
	TVPExecuteExpression(content, NULL, result);
}
//---------------------------------------------------------------------------
void TVPExecuteExpression(const ttstr& content, const ttstr &name, tjs_int lineofs, tTJSVariant *result)
{
	TVPExecuteExpression(content, name, lineofs, NULL, result);
}
//---------------------------------------------------------------------------
void TVPExecuteExpression(const ttstr& content, iTJSDispatch2 *context, tTJSVariant *result)
{
	if(TVPScriptEngine)
	{
		iTJSConsoleOutput *output = TVPScriptEngine->GetConsoleOutput();
		TVPScriptEngine->SetConsoleOutput(NULL); // once set TJS console to null
		try
		{
			TVPScriptEngine->EvalExpression(content, result, context);
		}
		catch(...)
		{
			TVPScriptEngine->SetConsoleOutput(output);
			throw;
		}
		TVPScriptEngine->SetConsoleOutput(output);
	}
	else
	{
		TVPThrowInternalError;
	}
}
//---------------------------------------------------------------------------
void TVPExecuteExpression(const ttstr& content, const ttstr &name, tjs_int lineofs, iTJSDispatch2 *context, tTJSVariant *result)
{
	if(TVPScriptEngine)
	{
		iTJSConsoleOutput *output = TVPScriptEngine->GetConsoleOutput();
		TVPScriptEngine->SetConsoleOutput(NULL); // once set TJS console to null
		try
		{
			TVPScriptEngine->EvalExpression(content, result, context, &name, lineofs);
		}
		catch(...)
		{
			TVPScriptEngine->SetConsoleOutput(output);
			throw;
		}
		TVPScriptEngine->SetConsoleOutput(output);
	}
	else
	{
		TVPThrowInternalError;
	}
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// TVPExecuteBytecode
//---------------------------------------------------------------------------
void TVPExecuteBytecode( const tjs_uint8* content, size_t len, iTJSDispatch2 *context, tTJSVariant *result, const tjs_char *name )
{
	if(!TVPScriptEngine) TVPThrowInternalError;

	TVPScriptEngine->LoadByteCode( content, len, result, context, name);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TVPExecuteStorage(const ttstr &name, tTJSVariant *result, bool isexpression,
	const tjs_char * modestr)
{
	TVPExecuteStorage(name, NULL, result, isexpression, modestr);
}
//---------------------------------------------------------------------------
void TVPExecuteStorage(const ttstr &name, iTJSDispatch2 *context, tTJSVariant *result, bool isexpression,
	const tjs_char * modestr)
{
	// execute storage which contains script
	if(!TVPScriptEngine) TVPThrowInternalError;

	{ // for bytecode
		ttstr place(TVPSearchPlacedPath(name));
		ttstr shortname(TVPExtractStorageName(place));
		tTJSBinaryStream* stream = TVPCreateBinaryStreamForRead(place, modestr);
		if( stream ) {
			bool isbytecode = false;
			try {
				isbytecode = TVPScriptEngine->LoadByteCode( stream, result, context, shortname.c_str() );
			} catch(...) {
				delete stream;
				throw;
			}
			delete stream;
			if( isbytecode ) return;
		}
	}

	ttstr place(TVPSearchPlacedPath(name));
	ttstr shortname(TVPExtractStorageName(place));

	iTJSTextReadStream * stream = TVPCreateTextStreamForReadByEncoding(place, modestr,TVPScriptTextEncoding);
	ttstr buffer;
	try
	{
		stream->Read(buffer, 0);
	}
	catch(...)
	{
		stream->Destruct();
		throw;
	}
	stream->Destruct();

#ifdef __EMSCRIPTEN__
		// Script dump probes are useful while developing a rewrite, but keeping
		// them enabled pollutes fatal sentinels and slows browser diagnostics.
		if (TVPGetCommandLine(TJS_W("-krkr-script-snippet-dump"))) {
		std::string nm = shortname.AsNarrowStdString();
		fprintf(stderr, "[TJS-LOAD] %s len=%zu\n", nm.c_str(), (size_t)buffer.length());
			if (nm == "kagenvplayer.tjs" || nm == "kagenvironment.tjs" ||
				nm == "kagenvbase.tjs" || nm == "motionaffinesourcelayer.tjs" ||
				nm == "affinesourcelayer.tjs" ||
				nm == "asdaffinesourcelayer.tjs" || nm == "affinelayer.tjs" ||
				nm == "prerenderfontex.tjs" || nm == "quickmenu.tjs" ||
				nm == "animkaglayer.tjs" || nm == "kaglayer.tjs" || nm == "sqscr.tjs" ||
				nm == "main.motioninfo" || nm == "custom.tjs" ||
				nm == "override.tjs" || nm == "conductor.tjs") {
			std::string content = buffer.AsNarrowStdString();
			auto has = [&](size_t begin, size_t end, const char *needle) {
				return content.find(needle, begin) < end;
			};
			auto nearby = [&](int line, const char *needleLine) {
					return line >= 100 && line <= 145 && nm == "motionaffinesourcelayer.tjs" ||
						line >= 95 && line <= 135 && nm == "affinesourcelayer.tjs" ||
					strstr(needleLine, "sceneplay") ||
					strstr(needleLine, "entryDelay") ||
					strstr(needleLine, "extractDelay") ||
					strstr(needleLine, "onMotionStop") ||
					strstr(needleLine, "notifyOwner") ||
					strstr(needleLine, "click_wait") ||
					strstr(needleLine, "canWaitMovie") ||
					strstr(needleLine, "waitMovie") ||
					strstr(needleLine, "onMovieStop") ||
					strstr(needleLine, "updateFlip") ||
					strstr(needleLine, "drawAffine");
			};
			int line = 1;
			size_t pos = 0;
			while (pos <= content.size()) {
				size_t next = content.find('\n', pos);
				if (next == std::string::npos) next = content.size();
				std::string lineText = content.substr(pos, next - pos);
				bool dump = nearby(line, lineText.c_str());
				if (nm == "kagenvplayer.tjs") {
					dump = dump ||
						(line >= 250 && line <= 520) ||
						(line >= 900 && line <= 1600) ||
						(line >= 380 && line <= 450) ||
						(line >= 1600 && line <= 1830) ||
						(line >= 1830 && line <= 1895) ||
						(line >= 2070 && line <= 2135) ||
						(line >= 2235 && line <= 2295) ||
						(line >= 2295 && line <= 2335) ||
						(line >= 2335 && line <= 2385) ||
						(line >= 2645 && line <= 2705);
					dump = dump ||
						strstr(lineText.c_str(), "convertSceneText") ||
						strstr(lineText.c_str(), "loadScene") ||
						strstr(lineText.c_str(), "findScene") ||
						strstr(lineText.c_str(), "getLine") ||
						strstr(lineText.c_str(), "scene.texts") ||
						strstr(lineText.c_str(), "loadStruct") ||
						strstr(lineText.c_str(), "readonlydb") ||
						strstr(lineText.c_str(), "scenePath");
				}
				if (nm == "prerenderfontex.tjs") {
					dump = dump || (line >= 150 && line <= 210);
				}
						if (nm == "motionaffinesourcelayer.tjs") {
							dump = dump || (line >= 240 && line <= 295);
						}
						if (nm == "affinesourcelayer.tjs") {
							dump = dump ||
								(line >= 95 && line <= 135) ||
								strstr(lineText.c_str(), "calcAffine") ||
								strstr(lineText.c_str(), "_owners");
						}
				if (nm == "quickmenu.tjs" || nm == "main.motioninfo") {
					dump = dump ||
						strstr(lineText.c_str(), "main.psb") ||
						strstr(lineText.c_str(), "MSGWIN") ||
						strstr(lineText.c_str(), "QuickMenu") ||
						strstr(lineText.c_str(), "Motion") ||
						strstr(lineText.c_str(), "motion") ||
						strstr(lineText.c_str(), "visible") ||
						strstr(lineText.c_str(), "opacity") ||
						strstr(lineText.c_str(), "bringToFront") ||
						strstr(lineText.c_str(), "show") ||
						strstr(lineText.c_str(), "hide");
				}
					if (nm == "animkaglayer.tjs") {
						dump = dump ||
							(line >= 1320 && line <= 1450) ||
							strstr(lineText.c_str(), "getMotionVariable") ||
							strstr(lineText.c_str(), "workMotion") ||
							strstr(lineText.c_str(), "motionPlaying") ||
							strstr(lineText.c_str(), "_motionVariables");
					}
					if (nm == "kaglayer.tjs") {
						dump = dump ||
							(line >= 60 && line <= 120) ||
							strstr(lineText.c_str(), "class KAGLayer") ||
							strstr(lineText.c_str(), "function KAGLayer");
					}
					if (nm == "conductor.tjs") {
						dump = dump ||
							(line >= 380 && line <= 420) ||
							strstr(lineText.c_str(), "timerCallback") ||
							strstr(lineText.c_str(), "checkStopMotion");
					}
				if (nm == "custom.tjs") {
					dump = dump ||
						(line >= 1600 && line <= 1685) ||
						strstr(lineText.c_str(), "drawItemText") ||
						strstr(lineText.c_str(), "storeMenu") ||
						strstr(lineText.c_str(), "restoreMenu") ||
						strstr(lineText.c_str(), "save_ui.psb") ||
						strstr(lineText.c_str(), "addsyshook") ||
						strstr(lineText.c_str(), "addsysscript");
				}
				if (nm == "override.tjs") {
					dump = dump ||
						(line >= 670 && line <= 725) ||
						strstr(lineText.c_str(), "SystemArgumentInfo") ||
						strstr(lineText.c_str(), "updateSysArgMenuItem") ||
						strstr(lineText.c_str(), "configMenuItem") ||
						strstr(lineText.c_str(), "drawItem");
				}
				if (nm == "sqscr.tjs") {
					dump = dump || (line >= 90 && line <= 145);
				}
				if (!dump && has(pos, next, "function ")) {
					dump = has(pos, next, "sceneplay") ||
						has(pos, next, "entryDelay") ||
						has(pos, next, "extractDelay");
				}
				if (dump) {
					fprintf(stderr, "[TJS-SNIP] %s:%d >>>%s<<<\n",
						nm.c_str(), line, lineText.c_str());
				}
				if (next == content.size()) break;
				pos = next + 1;
				line++;
			}
		}
		fflush(stderr);
	}
	#endif

		#ifdef __EMSCRIPTEN__
			if(TVPIsDracuBringupEnabled()) {
				KRKR_LOG_L3("[SCRIPT-LOAD-ALL] shortname=%s\n", shortname.AsNarrowStdString().c_str());
					if(shortname == TJS_W("Initialize.tjs") ||
						shortname == TJS_W("animkaglayer.tjs") ||
						shortname == TJS_W("custom.tjs") ||
						shortname == TJS_W("conductor.tjs") ||
						shortname == TJS_W("kagenvplayer.tjs") ||
						shortname == TJS_W("system.tjs")) {
						TVPLogDracuScriptHack("enabled", shortname);
					}
			if(shortname == TJS_W("conductor.tjs")) {
				buffer.Replace(
					TJS_W("\t\t\t\t\t\ttimer.interval = 0;\r\n"),
					TJS_W("\t\t\t\t\t\tif (timer !== void && (typeof timer) == \"Object\") timer.interval = 0;\r\n"),
					false);
				buffer.Replace(
					TJS_W("\t\t\ttimer.enabled = false;\r\n"),
					TJS_W("\t\t\tif (timer !== void && (typeof timer) == \"Object\") timer.enabled = false;\r\n"),
					false);
			}
				if(shortname == TJS_W("custom.tjs")) {
					buffer.Replace(
						TJS_W("return getMotionLayerMotion(minfo.mplayer, \"slot%02d\".sprintf(num+1));"),
					TJS_W("return global.getMotionLayerMotion(minfo.mplayer, \"slot%02d\".sprintf(num+1));"),
					false);
				buffer.Replace(
					TJS_W("return getMotionLayerShape(getSlot(num), name);"),
					TJS_W("var shape = global.getMotionLayerShape(getSlot(num), name);\r\n"
					      "\t\tif (shape !== void) return shape;\r\n"
					      "\t\tvar slot = \"slot%02d\".sprintf(num+1);\r\n"
					      "\t\tshape = global.getMotionLayerShape(minfo.mplayer, slot + \"/base/\" + name);\r\n"
					      "\t\tif (shape !== void) return shape;\r\n"
					      "\t\tshape = global.getMotionLayerShape(minfo.mplayer, slot + \"/\" + name);\r\n"
					      "\t\tif (shape !== void) return shape;\r\n"
					      "\t\tshape = global.getMotionLayerShape(minfo.mplayer, \"slot/\" + slot + \"/base/\" + name);\r\n"
					      "\t\tif (shape !== void) return shape;\r\n"
					      "\t\treturn global.getMotionLayerShape(minfo.mplayer, \"slot/\" + slot + \"/\" + name);"),
					false);
				buffer.Replace(
					TJS_W("\tfunction drawItemText(lay, text, elm, padding = 4) {\r\n"
					      "\t\tvar tmp = kag.temporaryLayer;\r\n"),
					TJS_W("\tfunction ensureTemporaryLayer(lay) {\r\n"
					      "\t\tvar tmp = kag.temporaryLayer;\r\n"
					      "\t\tif (tmp !== void && (typeof tmp) == \"Object\") return tmp;\r\n"
					      "\t\ttry {\r\n"
					      "\t\t\tvar win = Window.mainWindow;\r\n"
					      "\t\t\tvar base = (win !== void && (typeof win) == \"Object\") ? win.primaryLayer : void;\r\n"
					      "\t\t\tif (win === void || (typeof win) != \"Object\" || base === void || (typeof base) != \"Object\") {\r\n"
					      "\t\t\t\tdm(\"[KAG-TEMP-LAYER] missing layType=\" + typeof lay + \" winType=\" + typeof win + \" baseType=\" + typeof base);\r\n"
					      "\t\t\t\treturn void;\r\n"
					      "\t\t\t}\r\n"
					      "\t\t\tvar LayerClass = (typeof global != \"undefined\" && typeof global.__krkrNativeLayer != \"undefined\") ? global.__krkrNativeLayer : Layer;\r\n"
					      "\t\t\ttmp = new LayerClass(win, base);\r\n"
					      "\t\t\ttmp.name = \"temporaryLayer\";\r\n"
					      "\t\t\ttmp.visible = false;\r\n"
					      "\t\t\ttmp.type = 1;\r\n"
					      "\t\t\ttmp.face = 1;\r\n"
					      "\t\t\ttmp.opacity = 255;\r\n"
					      "\t\t\tkag.temporaryLayer = tmp;\r\n"
					      "\t\t\tdm(\"[KAG-TEMP-LAYER] created\");\r\n"
					      "\t\t} catch(e) {\r\n"
					      "\t\t\tdm(\"[KAG-TEMP-LAYER] failed \" + e);\r\n"
					      "\t\t\treturn void;\r\n"
					      "\t\t}\r\n"
					      "\t\treturn tmp;\r\n"
					      "\t}\r\n"
					      "\r\n"
					      "\tfunction drawItemText(lay, text, elm, padding = 4) {\r\n"
					      "\t\tvar tmp = ensureTemporaryLayer(lay);\r\n"),
					false);
					buffer = TVPGetMotionLayerHelperCompatScript() + buffer;
				}
						if(shortname == TJS_W("motionaffinesourcelayer.tjs")) {
									buffer.Replace(
										TJS_W("\tfunction onMotionUpdate(tick) {\r\n"),
										TJS_W("\tfunction onMotionUpdate(tick) {\r\n"
										      "\t\tif (_player === void || (typeof _player) != \"Object\") return;\r\n"
										      "\t\tif (!_motionAffinePlayerAlive()) return;\r\n"),
										false);
									buffer.Replace(
										TJS_W("function onMotionUpdate(tick) {\r\n"),
										TJS_W("function onMotionUpdate(tick) {\r\n"
										      "\tif (_player === void || (typeof _player) != \"Object\") return;\r\n"
										      "\tif (!_motionAffinePlayerAlive()) return;\r\n"),
										false);
								buffer.Replace(
									TJS_W("\tfunction setVariable(name, value) {\r\n"
									      "\t\t_player.setVariable(name, value);\r\n"
									      "\t\treturn true;\r\n"
									      "\t}\r\n"),
								TJS_W("\tfunction _motionAffinePlayerAlive() {\r\n"
								      "\t\tif (_player === void || (typeof _player) != \"Object\") return false;\r\n"
								      "\t\ttry { if (global.Scripts.isNullContext(_player)) return false; } catch(e) { return false; }\r\n"
								      "\t\ttry { if (!(isvalid _player)) return false; } catch(e) { return false; }\r\n"
								      "\t\treturn true;\r\n"
								      "\t}\r\n"
								      "\tfunction setVariable(name, value) {\r\n"
								      "\t\tif (!_motionAffinePlayerAlive()) return false;\r\n"
								      "\t\tvar __setVariable = global.Scripts.tryPropGet(_player, \"setVariable\", void);\r\n"
								      "\t\tif ((typeof __setVariable) != \"Object\") return false;\r\n"
								      "\t\ttry { _player.setVariable(name, value); } catch(e) { return false; }\r\n"
								      "\t\treturn true;\r\n"
								      "\t}\r\n"),
								false);
							buffer.Replace(
								TJS_W("\tfunction getVariable(name) {\r\n"
								      "\t\treturn _player.getVariable(name);\r\n"
								      "\t}\r\n"),
								TJS_W("\tfunction getVariable(name) {\r\n"
								      "\t\tif (!_motionAffinePlayerAlive()) return void;\r\n"
								      "\t\tvar __getVariable = global.Scripts.tryPropGet(_player, \"getVariable\", void);\r\n"
								      "\t\tif ((typeof __getVariable) != \"Object\") return void;\r\n"
								      "\t\ttry { return _player.getVariable(name); } catch(e) { return void; }\r\n"
								      "\t}\r\n"
								      "\tfunction progress(msTime) {\r\n"
								      "\t\tif (!_motionAffinePlayerAlive()) return false;\r\n"
								      "\t\tvar __progress = global.Scripts.tryPropGet(_player, \"progress\", void);\r\n"
								      "\t\tif ((typeof __progress) != \"Object\") return false;\r\n"
								      "\t\ttry { _player.progress(msTime); } catch(e) { return false; }\r\n"
								      "\t\treturn true;\r\n"
								      "\t}\r\n"
								      "\tfunction getCommandList() {\r\n"
								      "\t\tif (!_motionAffinePlayerAlive()) return [];\r\n"
								      "\t\tvar __getCommandList = global.Scripts.tryPropGet(_player, \"getCommandList\", void);\r\n"
								      "\t\tif ((typeof __getCommandList) != \"Object\") return [];\r\n"
								      "\t\ttry { return _player.getCommandList(); } catch(e) { return []; }\r\n"
								      "\t}\r\n"
								      "\tproperty playing {\r\n"
								      "\t\tgetter() {\r\n"
								      "\t\t\tif (!_motionAffinePlayerAlive()) return false;\r\n"
								      "\t\t\treturn global.Scripts.tryPropGet(_player, \"playing\", false) ? true : false;\r\n"
								      "\t\t}\r\n"
								      "\t}\r\n"
								      "\tproperty allplaying {\r\n"
								      "\t\tgetter() {\r\n"
								      "\t\t\tif (!_motionAffinePlayerAlive()) return false;\r\n"
								      "\t\t\treturn global.Scripts.tryPropGet(_player, \"allplaying\", false) ? true : false;\r\n"
								      "\t\t}\r\n"
								      "\t}\r\n"
								      "\tproperty motionPlaying {\r\n"
								      "\t\tgetter() {\r\n"
								      "\t\t\tif (!_motionAffinePlayerAlive()) return false;\r\n"
								      "\t\t\treturn global.Scripts.tryPropGet(_player, \"motionPlaying\", false) ? true : false;\r\n"
								      "\t\t}\r\n"
								      "\t}\r\n"),
								false);
								buffer.Replace(
									TJS_W("\tfunction getOptions() {\r\n"
									      "\t\tif (_player.chara !== void) {\r\n"),
									TJS_W("\tfunction getOptions() {\r\n"
									      "\t\tif (!_motionAffinePlayerAlive()) return %[];\r\n"
									      "\t\tif (_player.chara !== void) {\r\n"),
								false);
								buffer.Replace(
									TJS_W("\tfunction clone(newwindow, instance) {\r\n"
									      "\t\tif (newwindow === void) {\r\n"
									      "\t\t\tnewwindow = _window;\r\n"
									      "\t\t}\r\n"
									      "\t\tif (instance === void) {\r\n"
									      "\t\t\tinstance = new global.MotionAffineSourceLayer(newwindow, _separate);\r\n"
									      "\t\t}\r\n"
									      "\t\tif (_storage !== void) {\r\n"
									      "\t\t\tinstance.loadImages(_storage);\r\n"
									      "\t\t\tinstance._player.chara     = _player.chara;\r\n"
									      "\t\t\tinstance._player.motion    = _player.motion;\r\n"
									      "\t\t\tinstance._player.tickCount = _player.tickCount;\r\n"
									      "\t\t\tinstance._player.speed     = _player.speed;\r\n"
									      "\t\t}\r\n"
									      "\t\tif (instance._player.playing) {\r\n"
									      "\t\t\tinstance.onMotionStart();\r\n"
									      "\t\t}\r\n"
									      "\t\treturn super.clone(newwindow, instance);\r\n"
									      "\t}\r\n"),
									TJS_W("\tfunction clone(newwindow, instance) {\r\n"
									      "\t\tif (newwindow === void) {\r\n"
									      "\t\t\tnewwindow = _window;\r\n"
									      "\t\t}\r\n"
									      "\t\tif (instance === void) {\r\n"
									      "\t\t\tinstance = new global.MotionAffineSourceLayer(newwindow, _separate);\r\n"
									      "\t\t}\r\n"
									      "\t\tif (_storage !== void) {\r\n"
									      "\t\t\tinstance.loadImages(_storage);\r\n"
									      "\t\t\tif (_motionAffinePlayerAlive()) {\r\n"
									      "\t\t\t\ttry { instance._player.chara = _player.chara; } catch(e) {}\r\n"
									      "\t\t\t\ttry { instance._player.motion = _player.motion; } catch(e) {}\r\n"
									      "\t\t\t\ttry { instance._player.tickCount = _player.tickCount; } catch(e) {}\r\n"
									      "\t\t\t\ttry { instance._player.speed = _player.speed; } catch(e) {}\r\n"
									      "\t\t\t\ttry { instance._player.timeScale = _player.timeScale; } catch(e) {}\r\n"
									      "\t\t\t\ttry { instance._player.paused = _player.paused; } catch(e) {}\r\n"
									      "\t\t\t\ttry { instance._player.animating = _player.animating; } catch(e) {}\r\n"
									      "\t\t\t\tvar __srcPlaying = false;\r\n"
									      "\t\t\t\tvar __srcAllPlaying = false;\r\n"
									      "\t\t\t\tvar __srcMotionPlaying = false;\r\n"
									      "\t\t\t\ttry { __srcPlaying = _player.playing ? true : false; } catch(e) {}\r\n"
									      "\t\t\t\ttry { __srcAllPlaying = _player.allplaying ? true : false; } catch(e) {}\r\n"
									      "\t\t\t\ttry { __srcMotionPlaying = _player.motionPlaying ? true : false; } catch(e) {}\r\n"
									      "\t\t\t\tif (__srcPlaying || __srcAllPlaying || __srcMotionPlaying) {\r\n"
									      "\t\t\t\t\ttry { instance._player.playing = true; } catch(e) {}\r\n"
									      "\t\t\t\t\ttry { instance._player.allplaying = __srcAllPlaying || __srcMotionPlaying; } catch(e) {}\r\n"
									      "\t\t\t\t\ttry { instance._player.motionPlaying = __srcMotionPlaying || __srcPlaying; } catch(e) {}\r\n"
									      "\t\t\t\t}\r\n"
									      "\t\t\t}\r\n"
									      "\t\t}\r\n"
									      "\t\tvar __clonePlaying = false;\r\n"
									      "\t\ttry { __clonePlaying = instance._player.playing || instance._player.allplaying || instance._player.motionPlaying; } catch(e) { __clonePlaying = false; }\r\n"
									      "\t\tif (__clonePlaying) {\r\n"
									      "\t\t\tinstance.onMotionStart();\r\n"
									      "\t\t}\r\n"
									      "\t\treturn super.clone(newwindow, instance);\r\n"
									      "\t}\r\n"),
									false);
								buffer.Replace(
									TJS_W("\t\t\t\tinstance._player.speed     = _player.speed;\r\n"),
									TJS_W("\t\t\t\tinstance._player.speed     = _player.speed;\r\n"
									      "\t\t\t\ttry { if (_player.playing) instance._player.playing = true; } catch(e) {}\r\n"
									      "\t\t\t\ttry { if (_player.allplaying) instance._player.allplaying = true; } catch(e) {}\r\n"),
									false);
								buffer.Replace(
									TJS_W("\tfunction drawAffine(target, src) {\r\n"),
									TJS_W("\tfunction drawAffine(target, src) {\r\n"
									      "\t\tif (!_motionAffinePlayerAlive()) return;\r\n"),
								false);
					}
					if(shortname == TJS_W("affinesourcelayer.tjs")) {
							buffer.Replace(
								TJS_W("\tfunction calcAffine() {\r\n"),
								TJS_W("\tfunction calcAffine() {\r\n"
								      "\t\tif (typeof _owners == \"undefined\" || _owners === void || typeof _owners != \"Object\") return;\r\n"
								      "\t\tvar __affineOwnerCount = _owners.count;\r\n"
								      "\t\tif (__affineOwnerCount === void || __affineOwnerCount <= 0) return;\r\n"),
								false);
							buffer.Replace(
								TJS_W("function calcAffine() {\r\n"),
								TJS_W("function calcAffine() {\r\n"
								      "\tif (typeof _owners == \"undefined\" || _owners === void || typeof _owners != \"Object\") return;\r\n"
								      "\tvar __affineOwnerCount = _owners.count;\r\n"
								      "\tif (__affineOwnerCount === void || __affineOwnerCount <= 0) return;\r\n"),
								false);
					}
					if(shortname == TJS_W("Initialize.tjs")) {
						buffer.Replace(
							TJS_W("KAGLoadScript(\"LayerEx.tjs\");\r\n"),
							TJS_W("try { Plugins.link(\"scriptsEx.dll\"); } catch(__krkrScriptsExEarlyError) { try { dm(\"[SYSVAR-SAVE] scriptsEx early-link failed \" + __krkrScriptsExEarlyError.message); } catch(__krkrScriptsExEarlyLogError) {} }\r\n"
							      "KAGLoadScript(\"LayerEx.tjs\");\r\n"),
							false);
						buffer.Replace(
							TJS_W("KAGLoadScript(\"LayerEx.tjs\");\n"),
							TJS_W("try { Plugins.link(\"scriptsEx.dll\"); } catch(__krkrScriptsExEarlyError) { try { dm(\"[SYSVAR-SAVE] scriptsEx early-link failed \" + __krkrScriptsExEarlyError.message); } catch(__krkrScriptsExEarlyLogError) {} }\n"
							      "KAGLoadScript(\"LayerEx.tjs\");\n"),
							false);
						buffer.Replace(
							TJS_W("KAGLoadScript(\"motion.tjs\") if (KAGConfigEnabled(\"motionEnabled\", true) && CanLoadPlugin(\"motionplayer.dll\"));"),
						TJS_W("{ var __motionEnabled = KAGConfigEnabled(\"motionEnabled\", true);"
					      "  var __motionCanLoad = CanLoadPlugin(\"motionplayer.dll\");"
				      "  dm(\"[emscripten-motion] gate enabled=\" + __motionEnabled + \" canLoad=\" + __motionCanLoad);"
				      "  if (__motionEnabled && __motionCanLoad) {"
				      "    Plugins.link(\"motionplayer.dll\");"
				      "    KAGLoadScript(\"motion.tjs\");"
				      "  }"
				      "}"),
					false);
				}
					bool isMainWindowScript =
						shortname == TJS_W("mainwindow.tjs") ||
						shortname == TJS_W("MainWindow.tjs") ||
						(TVPWideStringContains(buffer, TJS_W("function loadSystemVariables()")) &&
						 TVPWideStringContains(buffer, TJS_W("function saveSystemVariables()")) &&
						 TVPWideStringContains(buffer, TJS_W("dataName + \"sc.ksd\"")) &&
						 TVPWideStringContains(buffer, TJS_W("dataName + \"su.ksd\"")));
					if(isMainWindowScript) {
						bool replacedInitialSave = TVPReplaceAndReport(buffer,
							TJS_W("\t\t\tsaveSystemVariables();\r\n"
							      "\r\n"
							      "\t\t\t// !visible "),
							TJS_W("\t\t\tvar __krkrInitialSc = saveDataLocation + \"/\" + dataName + \"sc.ksd\";\r\n"
							      "\t\t\tvar __krkrInitialSu = saveDataLocation + \"/\" + dataName + \"su.ksd\";\r\n"
							      "\t\t\tvar __krkrInitialScExists = Storages.isExistentStorage(__krkrInitialSc);\r\n"
							      "\t\t\tvar __krkrInitialSuExists = Storages.isExistentStorage(__krkrInitialSu);\r\n"
							      "\t\t\tvar __krkrInitialHasStorage = __krkrInitialScExists && __krkrInitialSuExists;\r\n"
							      "\t\t\tglobal.__krkrSkipInitialSystemVariablesSave = __krkrInitialHasStorage;\r\n"
							      "\t\t\tglobal.__krkrSuppressStartupSystemVariablesSaveRequest = __krkrInitialHasStorage;\r\n"
							      "\t\t\ttry { global.Scripts.logSystemVariablesSave(\"initial\", __krkrInitialSc + \",\" + __krkrInitialSu, \"skipped=\" + (__krkrInitialHasStorage ? 1 : 0) + \" scExists=\" + __krkrInitialScExists + \" suExists=\" + __krkrInitialSuExists + \" loadedFromStorage=\" + global.__krkrSystemVariablesLoadedFromStorage); } catch(__krkrInitialSaveLogError) { try { dm(\"[SYSVAR-SAVE] source=MainWindow.initial skipped=\" + (__krkrInitialHasStorage ? 1 : 0) + \" scExists=\" + __krkrInitialScExists + \" suExists=\" + __krkrInitialSuExists); } catch(__krkrInitialSaveLogError2) {} }\r\n"
							      "\t\t\tif (__krkrInitialHasStorage) {\r\n"
							      "\t\t\t\tdm(\"[SYSVAR-SAVE] source=MainWindow.initial skipped=1 scExists=\" + __krkrInitialScExists + \" suExists=\" + __krkrInitialSuExists);\r\n"
							      "\t\t\t} else {\r\n"
							      "\t\t\t\tdm(\"[SYSVAR-SAVE] source=MainWindow.initial requested=1 reason=missing scExists=\" + __krkrInitialScExists + \" suExists=\" + __krkrInitialSuExists);\r\n"
							      "\t\t\t\tsaveSystemVariables();\r\n"
							      "\t\t\t}\r\n"
							      "\r\n"
							      "\t\t\t// !visible "));
						if(!replacedInitialSave) replacedInitialSave = TVPReplaceAndReport(buffer,
							TJS_W("\t\t\tsaveSystemVariables();\n"
							      "\n"
							      "\t\t\t// !visible "),
							TJS_W("\t\t\tvar __krkrInitialSc = saveDataLocation + \"/\" + dataName + \"sc.ksd\";\n"
							      "\t\t\tvar __krkrInitialSu = saveDataLocation + \"/\" + dataName + \"su.ksd\";\n"
							      "\t\t\tvar __krkrInitialScExists = Storages.isExistentStorage(__krkrInitialSc);\n"
							      "\t\t\tvar __krkrInitialSuExists = Storages.isExistentStorage(__krkrInitialSu);\n"
							      "\t\t\tvar __krkrInitialHasStorage = __krkrInitialScExists && __krkrInitialSuExists;\n"
							      "\t\t\tglobal.__krkrSkipInitialSystemVariablesSave = __krkrInitialHasStorage;\n"
							      "\t\t\tglobal.__krkrSuppressStartupSystemVariablesSaveRequest = __krkrInitialHasStorage;\n"
							      "\t\t\ttry { global.Scripts.logSystemVariablesSave(\"initial\", __krkrInitialSc + \",\" + __krkrInitialSu, \"skipped=\" + (__krkrInitialHasStorage ? 1 : 0) + \" scExists=\" + __krkrInitialScExists + \" suExists=\" + __krkrInitialSuExists + \" loadedFromStorage=\" + global.__krkrSystemVariablesLoadedFromStorage); } catch(__krkrInitialSaveLogError) { try { dm(\"[SYSVAR-SAVE] source=MainWindow.initial skipped=\" + (__krkrInitialHasStorage ? 1 : 0) + \" scExists=\" + __krkrInitialScExists + \" suExists=\" + __krkrInitialSuExists); } catch(__krkrInitialSaveLogError2) {} }\n"
							      "\t\t\tif (__krkrInitialHasStorage) {\n"
							      "\t\t\t\tdm(\"[SYSVAR-SAVE] source=MainWindow.initial skipped=1 scExists=\" + __krkrInitialScExists + \" suExists=\" + __krkrInitialSuExists);\n"
							      "\t\t\t} else {\n"
							      "\t\t\t\tdm(\"[SYSVAR-SAVE] source=MainWindow.initial requested=1 reason=missing scExists=\" + __krkrInitialScExists + \" suExists=\" + __krkrInitialSuExists);\n"
							      "\t\t\t\tsaveSystemVariables();\n"
							      "\t\t\t}\n"
							      "\n"
							      "\t\t\t// !visible "));
						bool replacedInitialFullscreen = TVPReplaceAndReport(buffer,
							TJS_W("\t\t\tonFullScreenMenuItemClick(this) if (visible && (!!fs || forcefull));\r\n"),
							TJS_W("\t\t\tif (visible && (!!fs || forcefull)) {\r\n"
							      "\t\t\t\tvar __krkrPrevSuppressSystemVariablesSaveRequest = (typeof global.__krkrSuppressSystemVariablesSaveRequest != \"undefined\" && global.__krkrSuppressSystemVariablesSaveRequest);\r\n"
							      "\t\t\t\tglobal.__krkrSuppressSystemVariablesSaveRequest = true;\r\n"
							      "\t\t\t\ttry { onFullScreenMenuItemClick(this); } catch(__krkrInitialFullscreenError) { global.__krkrSuppressSystemVariablesSaveRequest = __krkrPrevSuppressSystemVariablesSaveRequest; throw __krkrInitialFullscreenError; }\r\n"
							      "\t\t\t\tglobal.__krkrSuppressSystemVariablesSaveRequest = __krkrPrevSuppressSystemVariablesSaveRequest;\r\n"
							      "\t\t\t}\r\n"));
						if(!replacedInitialFullscreen) replacedInitialFullscreen = TVPReplaceAndReport(buffer,
							TJS_W("\t\t\tonFullScreenMenuItemClick(this) if (visible && (!!fs || forcefull));\n"),
							TJS_W("\t\t\tif (visible && (!!fs || forcefull)) {\n"
							      "\t\t\t\tvar __krkrPrevSuppressSystemVariablesSaveRequest = (typeof global.__krkrSuppressSystemVariablesSaveRequest != \"undefined\" && global.__krkrSuppressSystemVariablesSaveRequest);\n"
							      "\t\t\t\tglobal.__krkrSuppressSystemVariablesSaveRequest = true;\n"
							      "\t\t\t\ttry { onFullScreenMenuItemClick(this); } catch(__krkrInitialFullscreenError) { global.__krkrSuppressSystemVariablesSaveRequest = __krkrPrevSuppressSystemVariablesSaveRequest; throw __krkrInitialFullscreenError; }\n"
							      "\t\t\t\tglobal.__krkrSuppressSystemVariablesSaveRequest = __krkrPrevSuppressSystemVariablesSaveRequest;\n"
							      "\t\t\t}\n"));
						bool replacedEnter = TVPReplaceAndReport(buffer,
							TJS_W("\tfunction saveSystemVariables()\r\n"
							      "\t{\r\n"),
							TJS_W("\tfunction saveSystemVariables()\r\n"
							      "\t{\r\n"
							      "\t\tif (typeof global.__krkrDoSystemVariablesSaveNow == \"undefined\" || !global.__krkrDoSystemVariablesSaveNow) {\r\n"
							      "\t\t\tvar __krkrSaveSuppressed = (typeof global.__krkrSuppressSystemVariablesSaveRequest != \"undefined\" && global.__krkrSuppressSystemVariablesSaveRequest);\r\n"
							      "\t\t\tvar __krkrStartupSaveSuppressed = (typeof global.__krkrSuppressStartupSystemVariablesSaveRequest != \"undefined\" && global.__krkrSuppressStartupSystemVariablesSaveRequest);\r\n"
							      "\t\t\tif (__krkrStartupSaveSuppressed) {\r\n"
							      "\t\t\t\ttry { global.Scripts.logSystemVariablesSave(\"startup-direct-suppressed\", \"\", \"source=MainWindow.saveSystemVariables\"); } catch(__krkrStartupSaveLogError) { try { dm(\"[SYSVAR-SAVE] source=MainWindow.saveSystemVariables startup-suppressed=1\"); } catch(__krkrStartupSaveLogError2) {} }\r\n"
							      "\t\t\t\tglobal.__krkrSuppressStartupSystemVariablesSaveRequest = false;\r\n"
							      "\t\t\t\treturn;\r\n"
							      "\t\t\t}\r\n"
							      "\t\t\tif (!__krkrSaveSuppressed && typeof global.Scripts == \"Object\" && typeof global.Scripts.requestSystemVariablesSave == \"Object\") {\r\n"
							      "\t\t\t\tglobal.Scripts.requestSystemVariablesSave(\"MainWindow.saveSystemVariables\");\r\n"
							      "\t\t\t\tdm(\"[SYSVAR-SAVE] source=MainWindow.saveSystemVariables requested\");\r\n"
							      "\t\t\t} else {\r\n"
							      "\t\t\t\tdm(\"[SYSVAR-SAVE] source=MainWindow.saveSystemVariables suppressed=\" + __krkrSaveSuppressed);\r\n"
							      "\t\t\t}\r\n"
							      "\t\t\treturn;\r\n"
							      "\t\t}\r\n"
							      "\t\ttry { global.Scripts.logSystemVariablesSave(\"enter\", \"\"); } catch(__krkrSaveLogError) { try { dm(\"[SYSVAR-SAVE] stage=enter\"); } catch(__krkrSaveLogError2) {} }\r\n"));
						if(!replacedEnter) replacedEnter = TVPReplaceAndReport(buffer,
							TJS_W("\tfunction saveSystemVariables()\n"
							      "\t{\n"),
							TJS_W("\tfunction saveSystemVariables()\n"
							      "\t{\n"
							      "\t\tif (typeof global.__krkrDoSystemVariablesSaveNow == \"undefined\" || !global.__krkrDoSystemVariablesSaveNow) {\n"
							      "\t\t\tvar __krkrSaveSuppressed = (typeof global.__krkrSuppressSystemVariablesSaveRequest != \"undefined\" && global.__krkrSuppressSystemVariablesSaveRequest);\n"
							      "\t\t\tvar __krkrStartupSaveSuppressed = (typeof global.__krkrSuppressStartupSystemVariablesSaveRequest != \"undefined\" && global.__krkrSuppressStartupSystemVariablesSaveRequest);\n"
							      "\t\t\tif (__krkrStartupSaveSuppressed) {\n"
							      "\t\t\t\ttry { global.Scripts.logSystemVariablesSave(\"startup-direct-suppressed\", \"\", \"source=MainWindow.saveSystemVariables\"); } catch(__krkrStartupSaveLogError) { try { dm(\"[SYSVAR-SAVE] source=MainWindow.saveSystemVariables startup-suppressed=1\"); } catch(__krkrStartupSaveLogError2) {} }\n"
							      "\t\t\t\tglobal.__krkrSuppressStartupSystemVariablesSaveRequest = false;\n"
							      "\t\t\t\treturn;\n"
							      "\t\t\t}\n"
							      "\t\t\tif (!__krkrSaveSuppressed && typeof global.Scripts == \"Object\" && typeof global.Scripts.requestSystemVariablesSave == \"Object\") {\n"
							      "\t\t\t\tglobal.Scripts.requestSystemVariablesSave(\"MainWindow.saveSystemVariables\");\n"
							      "\t\t\t\tdm(\"[SYSVAR-SAVE] source=MainWindow.saveSystemVariables requested\");\n"
							      "\t\t\t} else {\n"
							      "\t\t\t\tdm(\"[SYSVAR-SAVE] source=MainWindow.saveSystemVariables suppressed=\" + __krkrSaveSuppressed);\n"
							      "\t\t\t}\n"
							      "\t\t\treturn;\n"
							      "\t\t}\n"
							      "\t\ttry { global.Scripts.logSystemVariablesSave(\"enter\", \"\"); } catch(__krkrSaveLogError) { try { dm(\"[SYSVAR-SAVE] stage=enter\"); } catch(__krkrSaveLogError2) {} }\n"));
						bool replacedSaveStruct = TVPReplaceAndReport(buffer,
							TJS_W("\t\t\t\tvar fn = saveDataLocation + \"/\" + dataName + \"sc.ksd\";\r\n"
							      "\t\t\t\t(Dictionary.saveStruct incontextof scflags)(fn, saveDataMode);\r\n"
							      "\t\t\t\tvar fn = saveDataLocation + \"/\" + dataName + \"su.ksd\";\r\n"
							      "\t\t\t\t(Dictionary.saveStruct incontextof sflags)(fn, saveDataMode);\r\n"),
							TJS_W("\t\t\t\tvar fn = saveDataLocation + \"/\" + dataName + \"sc.ksd\";\r\n"
							      "\t\t\t\ttry { global.Scripts.logSystemVariablesSave(\"before-sc\", fn); } catch(__krkrSaveLogError) { try { dm(\"[SYSVAR-SAVE] stage=before-sc path=\" + fn); } catch(__krkrSaveLogError2) {} }\r\n"
							      "\t\t\t\t(Dictionary.saveStruct incontextof scflags)(fn, saveDataMode);\r\n"
							      "\t\t\t\ttry { global.Scripts.logSystemVariablesSave(\"after-sc\", fn); } catch(__krkrSaveLogError) { try { dm(\"[SYSVAR-SAVE] stage=after-sc path=\" + fn); } catch(__krkrSaveLogError2) {} }\r\n"
							      "\t\t\t\tvar fn = saveDataLocation + \"/\" + dataName + \"su.ksd\";\r\n"
							      "\t\t\t\ttry { global.Scripts.logSystemVariablesSave(\"before-su\", fn); } catch(__krkrSaveLogError) { try { dm(\"[SYSVAR-SAVE] stage=before-su path=\" + fn); } catch(__krkrSaveLogError2) {} }\r\n"
							      "\t\t\t\t(Dictionary.saveStruct incontextof sflags)(fn, saveDataMode);\r\n"
							      "\t\t\t\ttry { global.Scripts.logSystemVariablesSave(\"after-su\", fn); } catch(__krkrSaveLogError) { try { dm(\"[SYSVAR-SAVE] stage=after-su path=\" + fn); } catch(__krkrSaveLogError2) {} }\r\n"));
						if(!replacedSaveStruct) replacedSaveStruct = TVPReplaceAndReport(buffer,
							TJS_W("\t\t\t\tvar fn = saveDataLocation + \"/\" + dataName + \"sc.ksd\";\n"
							      "\t\t\t\t(Dictionary.saveStruct incontextof scflags)(fn, saveDataMode);\n"
							      "\t\t\t\tvar fn = saveDataLocation + \"/\" + dataName + \"su.ksd\";\n"
							      "\t\t\t\t(Dictionary.saveStruct incontextof sflags)(fn, saveDataMode);\n"),
							TJS_W("\t\t\t\tvar fn = saveDataLocation + \"/\" + dataName + \"sc.ksd\";\n"
							      "\t\t\t\ttry { global.Scripts.logSystemVariablesSave(\"before-sc\", fn); } catch(__krkrSaveLogError) { try { dm(\"[SYSVAR-SAVE] stage=before-sc path=\" + fn); } catch(__krkrSaveLogError2) {} }\n"
							      "\t\t\t\t(Dictionary.saveStruct incontextof scflags)(fn, saveDataMode);\n"
							      "\t\t\t\ttry { global.Scripts.logSystemVariablesSave(\"after-sc\", fn); } catch(__krkrSaveLogError) { try { dm(\"[SYSVAR-SAVE] stage=after-sc path=\" + fn); } catch(__krkrSaveLogError2) {} }\n"
							      "\t\t\t\tvar fn = saveDataLocation + \"/\" + dataName + \"su.ksd\";\n"
							      "\t\t\t\ttry { global.Scripts.logSystemVariablesSave(\"before-su\", fn); } catch(__krkrSaveLogError) { try { dm(\"[SYSVAR-SAVE] stage=before-su path=\" + fn); } catch(__krkrSaveLogError2) {} }\n"
							      "\t\t\t\t(Dictionary.saveStruct incontextof sflags)(fn, saveDataMode);\n"
							      "\t\t\t\ttry { global.Scripts.logSystemVariablesSave(\"after-su\", fn); } catch(__krkrSaveLogError) { try { dm(\"[SYSVAR-SAVE] stage=after-su path=\" + fn); } catch(__krkrSaveLogError2) {} }\n"));
						bool replacedLoadHeader = TVPReplaceAndReport(buffer,
							TJS_W("\tfunction loadSystemVariables()\r\n"
							      "\t{\r\n"),
							TJS_W("\tfunction loadSystemVariables()\r\n"
							      "\t{\r\n"
							      "\t\tvar __krkrDiagScPath = \"\";\r\n"
							      "\t\tvar __krkrDiagSuPath = \"\";\r\n"
							      "\t\tvar __krkrDiagScExists = false;\r\n"
							      "\t\tvar __krkrDiagSuExists = false;\r\n"));
						if(!replacedLoadHeader) replacedLoadHeader = TVPReplaceAndReport(buffer,
							TJS_W("\tfunction loadSystemVariables()\n"
							      "\t{\n"),
							TJS_W("\tfunction loadSystemVariables()\n"
							      "\t{\n"
							      "\t\tvar __krkrDiagScPath = \"\";\n"
							      "\t\tvar __krkrDiagSuPath = \"\";\n"
							      "\t\tvar __krkrDiagScExists = false;\n"
							      "\t\tvar __krkrDiagSuExists = false;\n"));
						bool replacedScExists = TVPReplaceAndReport(buffer,
							TJS_W("\t\t\tvar fn = saveDataLocation + \"/\" + dataName +\r\n"
							      "\t\t\t\t\"sc.ksd\";\r\n"
							      "\t\t\tif(Storages.isExistentStorage(fn))\r\n"),
							TJS_W("\t\t\tvar fn = saveDataLocation + \"/\" + dataName +\r\n"
							      "\t\t\t\t\"sc.ksd\";\r\n"
							      "\t\t\t__krkrDiagScPath = fn;\r\n"
							      "\t\t\t__krkrDiagScExists = Storages.isExistentStorage(fn);\r\n"
							      "\t\t\tif(__krkrDiagScExists)\r\n"));
						if(!replacedScExists) replacedScExists = TVPReplaceAndReport(buffer,
							TJS_W("\t\t\tvar fn = saveDataLocation + \"/\" + dataName +\n"
							      "\t\t\t\t\"sc.ksd\";\n"
							      "\t\t\tif(Storages.isExistentStorage(fn))\n"),
							TJS_W("\t\t\tvar fn = saveDataLocation + \"/\" + dataName +\n"
							      "\t\t\t\t\"sc.ksd\";\n"
							      "\t\t\t__krkrDiagScPath = fn;\n"
							      "\t\t\t__krkrDiagScExists = Storages.isExistentStorage(fn);\n"
							      "\t\t\tif(__krkrDiagScExists)\n"));
						bool replacedSuExists = TVPReplaceAndReport(buffer,
							TJS_W("\t\t\tvar fn = saveDataLocation + \"/\" + dataName +\r\n"
							      "\t\t\t\t\"su.ksd\";\r\n"
							      "\t\t\tif(Storages.isExistentStorage(fn))\r\n"),
							TJS_W("\t\t\tvar fn = saveDataLocation + \"/\" + dataName +\r\n"
							      "\t\t\t\t\"su.ksd\";\r\n"
							      "\t\t\t__krkrDiagSuPath = fn;\r\n"
							      "\t\t\t__krkrDiagSuExists = Storages.isExistentStorage(fn);\r\n"
							      "\t\t\tif(__krkrDiagSuExists)\r\n"));
						if(!replacedSuExists) replacedSuExists = TVPReplaceAndReport(buffer,
							TJS_W("\t\t\tvar fn = saveDataLocation + \"/\" + dataName +\n"
							      "\t\t\t\t\"su.ksd\";\n"
							      "\t\t\tif(Storages.isExistentStorage(fn))\n"),
							TJS_W("\t\t\tvar fn = saveDataLocation + \"/\" + dataName +\n"
							      "\t\t\t\t\"su.ksd\";\n"
							      "\t\t\t__krkrDiagSuPath = fn;\n"
							      "\t\t\t__krkrDiagSuExists = Storages.isExistentStorage(fn);\n"
							      "\t\t\tif(__krkrDiagSuExists)\n"));
						bool replacedLoadTail = TVPReplaceAndReport(buffer,
							TJS_W("\t\tcatch(e)\r\n"
							      "\t\t{\r\n"
							      "\t\t\terrorInform(\"initexception\", e.message);\r\n"
							      "\t\t\tscflags = %[];\r\n"
							      "\t\t\tsflags = %[];\r\n"
							      "\t\t}\r\n"
							      "\t}\r\n"),
							TJS_W("\t\tcatch(e)\r\n"
							      "\t\t{\r\n"
							      "\t\t\ttry { dm(\"[SYSVAR-DIAG] stage=load-exception saveDataLocation=\" + saveDataLocation + \" dataName=\" + dataName + \" scPath=\" + __krkrDiagScPath + \" suPath=\" + __krkrDiagSuPath + \" scExists=\" + __krkrDiagScExists + \" suExists=\" + __krkrDiagSuExists + \" message=\" + e.message); } catch(__krkrDiagLogError) {}\r\n"
							      "\t\t\terrorInform(\"initexception\", e.message);\r\n"
							      "\t\t\tscflags = %[];\r\n"
							      "\t\t\tsflags = %[];\r\n"
							      "\t\t}\r\n"
							      "\t\ttry {\r\n"
							      "\t\t\tglobal.__krkrSystemVariablesLoaded = true;\r\n"
							      "\t\t\tglobal.__krkrSystemVariablesLoadedFromStorage = __krkrDiagScExists && __krkrDiagSuExists;\r\n"
							      "\t\t\tdm(\"[SYSVAR-DIAG] stage=loaded saveDataLocation=\" + saveDataLocation + \" dataName=\" + dataName + \" dataPath=\" + System.dataPath + \" scPath=\" + __krkrDiagScPath + \" suPath=\" + __krkrDiagSuPath + \" scExists=\" + __krkrDiagScExists + \" suExists=\" + __krkrDiagSuExists + \" loadedFromStorage=\" + global.__krkrSystemVariablesLoadedFromStorage + \" sflags.menulock=\" + sflags.menulock + \" scflags.allskip=\" + scflags.allskip + \" kag.allskip=\" + allskip + \" scflags.afterskip=\" + scflags.afterskip + \" kag.afterskip=\" + afterskip + \" scflags.enterSkipOnReadedLabel=\" + scflags.enterSkipOnReadedLabel + \" enterSkipOnReadedLabel=\" + enterSkipOnReadedLabel + \" scflags.nosewhenskip=\" + scflags.nosewhenskip + \" nosewhenskip=\" + nosewhenskip);\r\n"
							      "\t\t} catch(__krkrDiagLogError) {\r\n"
							      "\t\t\ttry { dm(\"[SYSVAR-DIAG] stage=loaded-log-exception message=\" + __krkrDiagLogError.message); } catch(__krkrDiagLogError2) {}\r\n"
							      "\t\t}\r\n"
							      "\t}\r\n"));
							if(!replacedLoadTail) replacedLoadTail = TVPReplaceAndReport(buffer,
								TJS_W("\t\tcatch(e)\n"
							      "\t\t{\n"
							      "\t\t\terrorInform(\"initexception\", e.message);\n"
							      "\t\t\tscflags = %[];\n"
							      "\t\t\tsflags = %[];\n"
							      "\t\t}\n"
							      "\t}\n"),
							TJS_W("\t\tcatch(e)\n"
							      "\t\t{\n"
							      "\t\t\ttry { dm(\"[SYSVAR-DIAG] stage=load-exception saveDataLocation=\" + saveDataLocation + \" dataName=\" + dataName + \" scPath=\" + __krkrDiagScPath + \" suPath=\" + __krkrDiagSuPath + \" scExists=\" + __krkrDiagScExists + \" suExists=\" + __krkrDiagSuExists + \" message=\" + e.message); } catch(__krkrDiagLogError) {}\n"
							      "\t\t\terrorInform(\"initexception\", e.message);\n"
							      "\t\t\tscflags = %[];\n"
							      "\t\t\tsflags = %[];\n"
							      "\t\t}\n"
							      "\t\ttry {\n"
							      "\t\t\tglobal.__krkrSystemVariablesLoaded = true;\n"
							      "\t\t\tglobal.__krkrSystemVariablesLoadedFromStorage = __krkrDiagScExists && __krkrDiagSuExists;\n"
							      "\t\t\tdm(\"[SYSVAR-DIAG] stage=loaded saveDataLocation=\" + saveDataLocation + \" dataName=\" + dataName + \" dataPath=\" + System.dataPath + \" scPath=\" + __krkrDiagScPath + \" suPath=\" + __krkrDiagSuPath + \" scExists=\" + __krkrDiagScExists + \" suExists=\" + __krkrDiagSuExists + \" loadedFromStorage=\" + global.__krkrSystemVariablesLoadedFromStorage + \" sflags.menulock=\" + sflags.menulock + \" scflags.allskip=\" + scflags.allskip + \" kag.allskip=\" + allskip + \" scflags.afterskip=\" + scflags.afterskip + \" kag.afterskip=\" + afterskip + \" scflags.enterSkipOnReadedLabel=\" + scflags.enterSkipOnReadedLabel + \" enterSkipOnReadedLabel=\" + enterSkipOnReadedLabel + \" scflags.nosewhenskip=\" + scflags.nosewhenskip + \" nosewhenskip=\" + nosewhenskip);\n"
							      "\t\t} catch(__krkrDiagLogError) {\n"
							      "\t\t\ttry { dm(\"[SYSVAR-DIAG] stage=loaded-log-exception message=\" + __krkrDiagLogError.message); } catch(__krkrDiagLogError2) {}\n"
							      "\t\t}\n"
							      "\t}\n"));
						fprintf(stderr,
							"[SYSVAR-REWRITE] script=%s initial=%d fullscreen=%d enter=%d savelines=%d loadHeader=%d sc=%d su=%d loadTail=%d\n",
							shortname.AsNarrowStdString().c_str(),
							replacedInitialSave ? 1 : 0,
							replacedInitialFullscreen ? 1 : 0,
							replacedEnter ? 1 : 0,
							replacedSaveStruct ? 1 : 0,
							replacedLoadHeader ? 1 : 0,
							replacedScExists ? 1 : 0,
							replacedSuExists ? 1 : 0,
							replacedLoadTail ? 1 : 0);
					}
						if(shortname == TJS_W("kagenvimage.tjs")) {
							buffer.Replace(
								TJS_W("\tfunction calcAffine() {};"),
							TJS_W("\tfunction calcAffine() {\r\n"
							      "\t\ttry {\r\n"
							      "\t\t\tif (layer !== void && (typeof layer) == \"Object\") {\r\n"
							      "\t\t\t\tvar __targetLayer = global.Scripts.tryPropGet(layer, \"targetLayer\", void);\r\n"
							      "\t\t\t\tif (__targetLayer !== void && (typeof __targetLayer) == \"Object\") __targetLayer.calcAffine();\r\n"
							      "\t\t\t}\r\n"
							      "\t\t} catch(e) {}\r\n"
							      "\t}\r\n"),
							false);
						buffer.Replace(
							TJS_W("\tfunction calcUpdate() {};"),
							TJS_W("\tfunction calcUpdate() {\r\n"
							      "\t\ttry {\r\n"
							      "\t\t\tif (layer !== void && (typeof layer) == \"Object\") {\r\n"
							      "\t\t\t\tvar __targetLayer = global.Scripts.tryPropGet(layer, \"targetLayer\", void);\r\n"
							      "\t\t\t\tif (__targetLayer !== void && (typeof __targetLayer) == \"Object\") __targetLayer.calcUpdate(0,0,1,1);\r\n"
							      "\t\t\t}\r\n"
							      "\t\t} catch(e) {}\r\n"
							      "\t}\r\n"),
							false);
					}
					if(shortname == TJS_W("animkaglayer.tjs")) {
					buffer.Replace(
						TJS_W("\tvar _motionVariables = %[];"),
						TJS_W("\t// _motionVariables is initialized near _motionPlaying for web compat;"),
						false);
							buffer.Replace(
								TJS_W("\tvar _motionPlaying;"),
								TJS_W("\tvar _motionPlaying = false;\r\n"
								      "\tvar _motionVariables = %[];\r\n"
								      "\tvar _motionLastTick = void;\r\n"
								      "\tvar _motionInterval = 0;\r\n"
								      "\tvar _motionCommandList = [];\r\n"
								      "\tvar _motionWorkMotionHandler = void;\r\n"
								      "\tfunction _motionTryPropGet(obj, name, defval=void) {\r\n"
								      "\t\tif (obj === void || (typeof obj) != \"Object\") return defval;\r\n"
								      "\t\ttry { return global.Scripts.tryPropGet(obj, name, defval); } catch(e) { return defval; }\r\n"
								      "\t}\r\n"
								      "\tfunction _motionTryPropSet(obj, name, value) {\r\n"
								      "\t\tif (obj === void || (typeof obj) != \"Object\") return false;\r\n"
								      "\t\ttry { return global.Scripts.propSet(obj, name, value) ? true : false; } catch(e) { return false; }\r\n"
								      "\t}\r\n"
									      "\tfunction _motionTruthy(value) { return value !== void && value ? true : false; }\r\n"
									      "\tglobal._motionDiagLastX = void;\r\n"
									      "\tglobal._motionDiagLastY = void;\r\n"
									      "\tfunction _motionDiagRememberPoint(x, y) {\r\n"
									      "\t\tglobal._motionDiagLastX = x;\r\n"
									      "\t\tglobal._motionDiagLastY = y;\r\n"
									      "\t}\r\n"
									      "\tfunction _motionDiagTraceEnabled() {\r\n"
									      "\t\treturn typeof global != \"undefined\" && typeof global.__krkrDiagTraceSlotButtons != \"undefined\" && global.__krkrDiagTraceSlotButtons === true;\r\n"
									      "\t}\r\n"
									      "\tglobal._motionDiagTraceEnabled = _motionDiagTraceEnabled;\r\n"
									      "\tfunction _motionDiagToken(value) {\r\n"
									      "\t\tif (value === void) return \"void\";\r\n"
									      "\t\treturn \"\" + value;\r\n"
									      "\t}\r\n"
									      "\tglobal._motionDiagToken = _motionDiagToken;\r\n"
									      "\tfunction _motionDiagResolveText(x, y) {\r\n"
									      "\t\tif (!global._motionDiagTraceEnabled()) return \"\";\r\n"
									      "\t\tif (x === void || y === void) return \" frontResolve=missing-point\";\r\n"
									      "\t\ttry {\r\n"
									      "\t\t\tvar __mplayer = global.Scripts.tryPropGet(this, \"mplayer\", void);\r\n"
									      "\t\t\tif (__mplayer === void || typeof __mplayer != \"Object\") {\r\n"
									      "\t\t\t\tvar __owner = global.Scripts.tryPropGet(this, \"owner\", void);\r\n"
									      "\t\t\t\tif (__owner !== void && typeof __owner == \"Object\") __mplayer = global.Scripts.tryPropGet(__owner, \"mplayer\", void);\r\n"
									      "\t\t\t}\r\n"
									      "\t\t\tif (__mplayer === void || (typeof __mplayer) != \"Object\") return \" frontResolve=void\";\r\n"
									      "\t\t\tvar __resolveFrontHit = global.Scripts.tryPropGet(__mplayer, \"resolveFrontHit\", void);\r\n"
									      "\t\t\tif ((typeof __resolveFrontHit) != \"Object\") return \" frontResolve=missing\";\r\n"
									      "\t\t\tvar __r = __mplayer.resolveFrontHit(x, y);\r\n"
									      "\t\t\tif (__r === void || (typeof __r) != \"Object\") return \" frontResolve=void\";\r\n"
									      "\t\t\tvar __hitAreaCandidate = \"void\";\r\n"
									      "\t\t\tvar __hitAreaObject = \"void\";\r\n"
									      "\t\t\tvar __candidates = global.Scripts.tryPropGet(__r, \"candidates\", void);\r\n"
									      "\t\t\tif (__candidates !== void && (typeof __candidates) == \"Object\") {\r\n"
									      "\t\t\t\tfor (var __ci = 0; __ci < __candidates.count; __ci++) {\r\n"
									      "\t\t\t\t\tvar __candidate = __candidates[__ci];\r\n"
									      "\t\t\t\t\tif (__candidate !== void && (typeof __candidate) == \"Object\" && global.Scripts.tryPropGet(__candidate, \"hitRole\", void) == \"hit-area\") {\r\n"
									      "\t\t\t\t\t\t__hitAreaCandidate = global._motionDiagToken(global.Scripts.tryPropGet(__candidate, \"name\", void));\r\n"
									      "\t\t\t\t\t\t__hitAreaObject = global._motionDiagToken(global.Scripts.tryPropGet(__candidate, \"hitObjectName\", void));\r\n"
									      "\t\t\t\t\t\tbreak;\r\n"
									      "\t\t\t\t\t}\r\n"
									      "\t\t\t\t}\r\n"
									      "\t\t\t}\r\n"
									      "\t\t\treturn \" frontResolveHit=\" + global._motionDiagToken(global.Scripts.tryPropGet(__r, \"hit\", void)) +\r\n"
									      "\t\t\t\t\" frontResolveName=\" + global._motionDiagToken(global.Scripts.tryPropGet(__r, \"name\", void)) +\r\n"
									      "\t\t\t\t\" frontResolveObject=\" + global._motionDiagToken(global.Scripts.tryPropGet(__r, \"hitObjectName\", void)) +\r\n"
									      "\t\t\t\t\" frontResolveObjectPath=\" + global._motionDiagToken(global.Scripts.tryPropGet(__r, \"hitObjectPath\", void)) +\r\n"
									      "\t\t\t\t\" frontResolveState=\" + global._motionDiagToken(global.Scripts.tryPropGet(__r, \"hitStateName\", void)) +\r\n"
									      "\t\t\t\t\" frontResolveStatePath=\" + global._motionDiagToken(global.Scripts.tryPropGet(__r, \"hitStatePath\", void)) +\r\n"
									      "\t\t\t\t\" frontResolveRole=\" + global._motionDiagToken(global.Scripts.tryPropGet(__r, \"hitRole\", void)) +\r\n"
									      "\t\t\t\t\" frontResolveSemanticMode=\" + global._motionDiagToken(global.Scripts.tryPropGet(__r, \"semanticResolveMode\", void)) +\r\n"
									      "\t\t\t\t\" frontResolveSemanticName=\" + global._motionDiagToken(global.Scripts.tryPropGet(__r, \"semanticName\", void)) +\r\n"
									      "\t\t\t\t\" frontResolveSemanticObject=\" + global._motionDiagToken(global.Scripts.tryPropGet(__r, \"semanticObjectName\", void)) +\r\n"
									      "\t\t\t\t\" frontResolveSemanticObjectPath=\" + global._motionDiagToken(global.Scripts.tryPropGet(__r, \"semanticObjectPath\", void)) +\r\n"
									      "\t\t\t\t\" frontResolveSemanticState=\" + global._motionDiagToken(global.Scripts.tryPropGet(__r, \"semanticStateName\", void)) +\r\n"
									      "\t\t\t\t\" frontResolveSemanticStatePath=\" + global._motionDiagToken(global.Scripts.tryPropGet(__r, \"semanticStatePath\", void)) +\r\n"
									      "\t\t\t\t\" frontResolveSemanticRole=\" + global._motionDiagToken(global.Scripts.tryPropGet(__r, \"semanticRole\", void)) +\r\n"
									      "\t\t\t\t\" frontResolvePointHitObjectStateCount=\" + global._motionDiagToken(global.Scripts.tryPropGet(__r, \"pointHitObjectStateCount\", void)) +\r\n"
									      "\t\t\t\t\" frontResolvePointDrawObjectStateCount=\" + global._motionDiagToken(global.Scripts.tryPropGet(__r, \"pointDrawObjectStateCount\", void)) +\r\n"
									      "\t\t\t\t\" frontResolveHitAreaCandidate=\" + __hitAreaCandidate +\r\n"
									      "\t\t\t\t\" frontResolveHitAreaObject=\" + __hitAreaObject;\r\n"
									      "\t\t} catch(e) {\r\n"
									      "\t\t\treturn \" frontResolveError=\" + e;\r\n"
									      "\t\t}\r\n"
									      "\t}\r\n"
									      "\tglobal._motionDiagResolveText = _motionDiagResolveText;\r\n"),
									false);
						buffer.Replace(
							TJS_W("\t\t\tSystem.addContinuousHandler(workMotion);\r\n"),
							TJS_W("\t\t\tif (_motionWorkMotionHandler === void) _motionWorkMotionHandler = workMotion incontextof this;\r\n"
							      "\t\t\tSystem.addContinuousHandler(_motionWorkMotionHandler);\r\n"),
							false);
						buffer.Replace(
							TJS_W("\t\t\tSystem.removeContinuousHandler(workMotion);\r\n"),
							TJS_W("\t\t\tif (_motionWorkMotionHandler !== void) {\r\n"
							      "\t\t\t\tSystem.removeContinuousHandler(_motionWorkMotionHandler);\r\n"
							      "\t\t\t\t_motionWorkMotionHandler = void;\r\n"
							      "\t\t\t} else {\r\n"
							      "\t\t\t\tSystem.removeContinuousHandler(workMotion);\r\n"
							      "\t\t\t}\r\n"),
							false);
							buffer.Replace(
								TJS_W("\t\treturn layer.getMotionVariable(name);\r\n"),
								TJS_W("\t\tif (layer !== void && (typeof layer) == \"Object\" && isvalid layer) {\r\n"
									      "\t\t\tvar __layerGetMotionVariable = global.Scripts.tryPropGet(layer, \"getMotionVariable\", void);\r\n"
									      "\t\t\tif ((typeof __layerGetMotionVariable) == \"Object\") return layer.getMotionVariable(name);\r\n"
									      "\t\t\tvar __layerGetVariable = global.Scripts.tryPropGet(layer, \"getVariable\", void);\r\n"
									      "\t\t\tif ((typeof __layerGetVariable) == \"Object\") return layer.getVariable(name);\r\n"
									      "\t\t}\r\n"
									      "\t\treturn void;\r\n"),
									false);
							buffer.Replace(
								TJS_W("\tfunction getVariable(name) {\r\n"
								      "\t\tif (layer !== void && typeof layer == \"Object\" && isvalid layer && typeof layer.getMotionVariable == \"Object\") return layer.getMotionVariable(name);\r\n"
								      "\t\tif (layer !== void && typeof layer == \"Object\" && isvalid layer && typeof layer.getVariable == \"Object\") return layer.getVariable(name);\r\n"
								      "\t\treturn void;\r\n"
								      "\t}\r\n"),
									TJS_W("\tfunction getVariable(name) {\r\n"
									      "\t\tif (layer !== void && (typeof layer) == \"Object\" && isvalid layer) {\r\n"
									      "\t\t\tvar __layerGetMotionVariable = global.Scripts.tryPropGet(layer, \"getMotionVariable\", void);\r\n"
									      "\t\t\tif ((typeof __layerGetMotionVariable) == \"Object\") return layer.getMotionVariable(name);\r\n"
									      "\t\t\tvar __layerGetVariable = global.Scripts.tryPropGet(layer, \"getVariable\", void);\r\n"
									      "\t\t\tif ((typeof __layerGetVariable) == \"Object\") return layer.getVariable(name);\r\n"
									      "\t\t}\r\n"
									      "\t\treturn void;\r\n"
									      "\t}\r\n"
									      "\tfunction getMotionVariable(name) { return getVariable(name); }\r\n"),
								false);
							buffer.Replace(
								TJS_W("\tfunction setVariable(name, value) {\r\n"
								      "\t\tlayer.setMotionVariable(name, value);\r\n"
								      "\t}\r\n"),
									TJS_W("\tfunction setVariable(name, value) {\r\n"
									      "\t\tif (layer !== void && (typeof layer) == \"Object\" && isvalid layer) {\r\n"
									      "\t\t\tvar __layerSetMotionVariable = global.Scripts.tryPropGet(layer, \"setMotionVariable\", void);\r\n"
									      "\t\t\tif ((typeof __layerSetMotionVariable) == \"Object\") { layer.setMotionVariable(name, value); return; }\r\n"
									      "\t\t\tvar __layerSetVariable = global.Scripts.tryPropGet(layer, \"setVariable\", void);\r\n"
									      "\t\t\tif ((typeof __layerSetVariable) == \"Object\") layer.setVariable(name, value);\r\n"
									      "\t\t}\r\n"
									      "\t}\r\n"
									      "\tfunction setMotionVariable(name, value) { setVariable(name, value); }\r\n"),
								false);
							buffer.Replace(
								TJS_W("\t\towner.setMotionVariable(@\"${this.name}/${name}\", value);\r\n"),
								TJS_W("\t\tvar __motionOwner = global.Scripts.tryPropGet(this, \"owner\", void);\r\n"
								      "\t\tif (__motionOwner !== void && (typeof __motionOwner) == \"Object\" && isvalid __motionOwner) {\r\n"
								      "\t\t\tvar __motionOwnerSetMotionVariable = global.Scripts.tryPropGet(__motionOwner, \"setMotionVariable\", void);\r\n"
								      "\t\t\tvar __motionOwnerSetVariable = global.Scripts.tryPropGet(__motionOwner, \"setVariable\", void);\r\n"
								      "\t\t\tif ((typeof __motionOwnerSetMotionVariable) == \"Object\") __motionOwner.setMotionVariable(@\"${this.name}/${name}\", value);\r\n"
								      "\t\t\telse if ((typeof __motionOwnerSetVariable) == \"Object\") __motionOwner.setVariable(@\"${this.name}/${name}\", value);\r\n"
								      "\t\t}\r\n"),
								false);
							buffer.Replace(
								TJS_W("\t\tvar ret = owner.getMotionVariable(@\"${this.name}/${name}\");\r\n"),
								TJS_W("\t\tvar ret = void;\r\n"
								      "\t\tvar __motionOwner = global.Scripts.tryPropGet(this, \"owner\", void);\r\n"
								      "\t\tif (__motionOwner !== void && (typeof __motionOwner) == \"Object\" && isvalid __motionOwner) {\r\n"
								      "\t\t\tvar __motionOwnerGetMotionVariable = global.Scripts.tryPropGet(__motionOwner, \"getMotionVariable\", void);\r\n"
								      "\t\t\tvar __motionOwnerGetVariable = global.Scripts.tryPropGet(__motionOwner, \"getVariable\", void);\r\n"
								      "\t\t\tif ((typeof __motionOwnerGetMotionVariable) == \"Object\") ret = __motionOwner.getMotionVariable(@\"${this.name}/${name}\");\r\n"
								      "\t\t\telse if ((typeof __motionOwnerGetVariable) == \"Object\") ret = __motionOwner.getVariable(@\"${this.name}/${name}\");\r\n"
								      "\t\t}\r\n"),
								false);
							buffer.Replace(
								TJS_W("_motionManager = Window.mainWindow.motion_manager;"),
								TJS_W("_motionManager = Window.mainWindow.motion_manager;"
				      " if (_motionManager === void) {"
				      "   if (typeof global.MotionResourceManager != \"undefined\")"
				      "     _motionManager = new global.MotionResourceManager(Window.mainWindow);"
				      "   else"
				      "     _motionManager = new Motion.ResourceManager(Window.mainWindow, 8);"
				      "   Window.mainWindow.motion_manager = _motionManager;"
							      " }"),
								false);
					buffer.Replace(
						TJS_W("\tfunction checkInit() {\r\n"
						      "\t\tif (_motionRedraw) {\r\n"
						      "\t\t\texecMotionFunc(\"onInit\", mplayer.motion);\r\n"
						      "\t\t\tinitButtons();\r\n"
						      "\t\t\t_motionRedraw = false;\r\n"
						      "\t\t}\r\n"
						      "\t}\r\n"),
						TJS_W("\tfunction checkInit() {\r\n"
						      "\t\tif (_motionRedraw) {\r\n"
						      "\t\t\tvar __diagChara = \"void\";\r\n"
						      "\t\t\tvar __diagMotion = \"void\";\r\n"
						      "\t\t\tvar __diagMenuBefore = \"void\";\r\n"
						      "\t\t\tvar __diagStateBefore = \"void\";\r\n"
						      "\t\t\tvar __diagSfMenulock = \"void\";\r\n"
						      "\t\t\ttry { __diagChara = global.Scripts.tryPropGet(mplayer, \"chara\", \"void\"); } catch(e) { __diagChara = \"ERR:\" + e.message; }\r\n"
						      "\t\t\ttry { __diagMotion = global.Scripts.tryPropGet(mplayer, \"motion\", \"void\"); } catch(e) { __diagMotion = \"ERR:\" + e.message; }\r\n"
						      "\t\t\ttry { __diagMenuBefore = mplayer.getVariable(\"menu\"); } catch(e) { __diagMenuBefore = \"ERR:\" + e.message; }\r\n"
						      "\t\t\ttry { __diagStateBefore = mplayer.getVariable(\"state\"); } catch(e) { __diagStateBefore = \"ERR:\" + e.message; }\r\n"
						      "\t\t\ttry { __diagSfMenulock = sf.menulock; } catch(e) { __diagSfMenulock = \"ERR:\" + e.message; }\r\n"
						      "\t\t\tif (__diagChara == \"MSGWIN\" || systemMotion) dm(\"[MSGWIN-INIT-DIAG] stage=before chara=\" + __diagChara + \" motion=\" + __diagMotion + \" system=\" + systemMotion + \" redraw=\" + _motionRedraw + \" sf.menulock=\" + __diagSfMenulock + \" menu=\" + __diagMenuBefore + \" state=\" + __diagStateBefore + \" buttons=\" + _motionButtons.count + \" areas=\" + _motionAreas.count);\r\n"
						      "\t\t\texecMotionFunc(\"onInit\", mplayer.motion);\r\n"
						      "\t\t\tvar __diagMenuAfterOnInit = \"void\";\r\n"
						      "\t\t\tvar __diagStateAfterOnInit = \"void\";\r\n"
						      "\t\t\ttry { __diagMenuAfterOnInit = mplayer.getVariable(\"menu\"); } catch(e) { __diagMenuAfterOnInit = \"ERR:\" + e.message; }\r\n"
						      "\t\t\ttry { __diagStateAfterOnInit = mplayer.getVariable(\"state\"); } catch(e) { __diagStateAfterOnInit = \"ERR:\" + e.message; }\r\n"
						      "\t\t\tif (__diagChara == \"MSGWIN\" || systemMotion) dm(\"[MSGWIN-INIT-DIAG] stage=after-onInit chara=\" + __diagChara + \" motion=\" + __diagMotion + \" sf.menulock=\" + __diagSfMenulock + \" menu=\" + __diagMenuAfterOnInit + \" state=\" + __diagStateAfterOnInit + \" buttons=\" + _motionButtons.count + \" areas=\" + _motionAreas.count);\r\n"
						      "\t\t\tinitButtons();\r\n"
						      "\t\t\tif (__diagChara == \"MSGWIN\" || systemMotion) dm(\"[MSGWIN-INIT-DIAG] stage=after-initButtons chara=\" + __diagChara + \" motion=\" + __diagMotion + \" sf.menulock=\" + __diagSfMenulock + \" menu=\" + __diagMenuAfterOnInit + \" state=\" + __diagStateAfterOnInit + \" buttons=\" + _motionButtons.count + \" areas=\" + _motionAreas.count);\r\n"
						      "\t\t\t_motionRedraw = false;\r\n"
						      "\t\t}\r\n"
						      "\t}\r\n"),
						false);
					buffer.Replace(
						TJS_W("\tfunction onHitTest(x, y, hit) {\r\n"
						      "\t\tif (motionWorking) {\r\n"
					      "\t\t\tvar contain = mplayer.contains(x,y);\r\n"
					      "\t\t\thit =  contain || prevContain || _motionAllHit;\r\n"
					      "\t\t\tprevContain = contain;\r\n"
					      "\t\t}\r\n"
					      "\t\treturn super.onHitTest(x, y, hit);\r\n"
					      "\t}\r\n"),
					TJS_W("\tfunction onHitTest(x, y, hit) {\r\n"
					      "\t\tif (motionWorking) {\r\n"
					      "\t\t\tvar contain = mplayer.contains(x,y);\r\n"
					      "\t\t\thit =  contain || prevContain || _motionAllHit;\r\n"
					      "\t\t\tprevContain = contain;\r\n"
					      "\t\t\treturn super.onHitTest(x, y, hit);\r\n"
					      "\t\t}\r\n"
					      "\t\treturn super.onHitTest(x, y, hit);\r\n"
					      "\t}\r\n"),
					false);
						buffer.Replace(
							TJS_W("\tfunction setMotionVariable(name, value) {\r\n"
							      "\t\tif (motionWorking) {\r\n"
							      "\t\t\t_motionVariables[name] = value;\r\n"
							      "\t\t\tmplayer.setVariable(name, value);\r\n"
							      "\t\t}\r\n"
							      "\t}\r\n"),
											TJS_W("\tfunction setMotionVariable(name, value) {\r\n"
												      "\t\tif (_motionVariables === void || typeof _motionVariables != \"Object\") _motionVariables = %[];\r\n"
													      "\t\t_motionVariables[name] = value;\r\n"
													      "\t\tvar __mplayer = global.Scripts.tryPropGet(this, \"mplayer\", void);\r\n"
													      "\t\tif (__mplayer === void || (typeof __mplayer) != \"Object\") return;\r\n"
													      "\t\tvar __mplayerSetVariable = global.Scripts.tryPropGet(__mplayer, \"setVariable\", void);\r\n"
													      "\t\tif ((typeof __mplayerSetVariable) != \"Object\") return;\r\n"
													      "\t\ttry { __mplayer.setVariable(name, value); } catch(e) {}\r\n"
											      "\t}\r\n"),
											false);
						buffer.Replace(
							TJS_W("\tfunction getMotionVariable(name) {\r\n"
							      "\t\tif (motionWorking) {\r\n"
						      "\t\t\tvar ret = mplayer.getVariable(name);\r\n"
						      "\t\t\tif (ret !== void) {\r\n"
						      "\t\t\t\t_motionVariables[name] = ret;\r\n"
						      "\t\t\t} else {\r\n"
						      "\t\t\t\tdelete _motionVariables[name];\r\n"
						      "\t\t\t}\r\n"
						      "\t\t\treturn ret;\r\n"
							      "\t\t}\r\n"
							      "\t}\r\n"),
									TJS_W("\tfunction getMotionVariable(name) {\r\n"
										      "\t\tif (_motionVariables === void || typeof _motionVariables != \"Object\") _motionVariables = %[];\r\n"
										      "\t\tvar __mplayer = global.Scripts.tryPropGet(this, \"mplayer\", void);\r\n"
										      "\t\tif (__mplayer === void || (typeof __mplayer) != \"Object\") return _motionVariables[name];\r\n"
										      "\t\tvar __mplayerGetVariable = global.Scripts.tryPropGet(__mplayer, \"getVariable\", void);\r\n"
										      "\t\tif ((typeof __mplayerGetVariable) != \"Object\") return _motionVariables[name];\r\n"
										      "\t\tvar ret = _motionVariables[name];\r\n"
										      "\t\ttry { ret = __mplayer.getVariable(name); } catch(e) { return _motionVariables[name]; }\r\n"
								      "\t\tif (ret !== void) {\r\n"
							      "\t\t\t_motionVariables[name] = ret;\r\n"
							      "\t\t} else {\r\n"
						      "\t\t\tdelete _motionVariables[name];\r\n"
						      "\t\t}\r\n"
						      "\t\treturn ret;\r\n"
						      "\t}\r\n"),
								false);
							buffer.Replace(
								TJS_W("\tfunction workMotion(tick) {\r\n"
							      "\t\tif (!mplayer) return;\r\n"
						      "\r\n"
						      "\t\tif (_motionLastTick === void) {\r\n"
						      "\t\t\t_motionLastTick = tick;\r\n"
						      "\t\t}\r\n"
						      "\t\twith (mplayer) {\r\n"
						      "\t\t\tif (motionPlaying || (.allplaying && nodeVisible)) {\r\n"
						      "\t\t\t\tcheckInit();\r\n"
						      "\t\t\t\t_motionInterval = tick - _motionLastTick;\r\n"
						      "\t\t\t\t.progress(_motionInterval);\r\n"
						      "\t\t\t\tvar newCommandList = .getCommandList();\t\r\n"
						      "\t\t\t\tif (!Scripts.equalStruct(_motionCommandList, newCommandList)) {\r\n"
						      "\t\t\t\t\tupdate(0,0,1,1);\r\n"
						      "\t\t\t\t\t_motionCommandList = newCommandList;\r\n"
						      "\t\t\t\t\t//dm(@\"${name}:motionInterval:${_motionInterval}\");\r\n"
						      "\t\t\t\t}\r\n"
						      "\t\t\t}\r\n"
						      "\t\t\tif (!.playing) {\r\n"
						      "\t\t\t\tcheckStopMotion();\r\n"
						      "\t\t\t}\r\n"
								      "\t\t}\r\n"
								      "\t\t_motionLastTick = tick;\r\n"
								      "\t}\r\n"),
									TJS_W("\tfunction workMotion(tick) {\r\n"
									      "\t\tvar __motionSelf = this;\r\n"
									      "\t\tvar __mplayer = global.Scripts.tryPropGet(__motionSelf, \"mplayer\", void);\r\n"
									      "\t\tif (__mplayer === void || (typeof __mplayer) != \"Object\") {\r\n"
								      "\t\t\ttry { _motionPlaying = false; } catch(e) {}\r\n"
								      "\t\t\tif (_motionCommandList === void || typeof _motionCommandList != \"Object\") _motionCommandList = [];\r\n"
								      "\t\t\telse _motionCommandList = [];\r\n"
								      "\t\t\t_motionLastTick = tick;\r\n"
								      "\t\t\tif (_motionWorkMotionHandler !== void) {\r\n"
								      "\t\t\t\ttry { System.removeContinuousHandler(_motionWorkMotionHandler); } catch(e) {}\r\n"
								      "\t\t\t\t_motionWorkMotionHandler = void;\r\n"
								      "\t\t\t}\r\n"
								      "\t\t\treturn;\r\n"
								      "\t\t}\r\n"
									      "\t\tvar __progressFunc = global.Scripts.tryPropGet(__mplayer, \"progress\", void);\r\n"
									      "\t\tvar __getCommandListFunc = global.Scripts.tryPropGet(__mplayer, \"getCommandList\", void);\r\n"
									      "\t\tvar __canProgress = (typeof __progressFunc) == \"Object\";\r\n"
									      "\t\tvar __canGetCommands = (typeof __getCommandListFunc) == \"Object\";\r\n"
									      "\t\tvar __playingValue = global.Scripts.tryPropGet(__mplayer, \"playing\", void);\r\n"
									      "\t\tvar __motionPlayingValue = global.Scripts.tryPropGet(__mplayer, \"motionPlaying\", void);\r\n"
									      "\t\tvar __allplayingValue = global.Scripts.tryPropGet(__mplayer, \"allplaying\", void);\r\n"
								      "\t\tif (__playingValue === void && __motionPlayingValue === void && __allplayingValue === void && !__canProgress && !__canGetCommands) {\r\n"
								      "\t\t\ttry { _motionPlaying = false; } catch(e) {}\r\n"
								      "\t\t\t_motionLastTick = tick;\r\n"
								      "\t\t\tif (_motionWorkMotionHandler !== void) {\r\n"
								      "\t\t\t\ttry { System.removeContinuousHandler(_motionWorkMotionHandler); } catch(e) {}\r\n"
								      "\t\t\t\t_motionWorkMotionHandler = void;\r\n"
								      "\t\t\t}\r\n"
								      "\t\t\treturn;\r\n"
								      "\t\t}\r\n"
								      "\t\tif (_motionLastTick === void) {\r\n"
								      "\t\t\t_motionLastTick = tick;\r\n"
								      "\t\t}\r\n"
								      "\t\tvar __playing = false;\r\n"
								      "\t\tif (__motionPlayingValue !== void) __playing = __motionPlayingValue ? true : false;\r\n"
								      "\t\tif (!__playing && __playingValue !== void) __playing = __playingValue ? true : false;\r\n"
								      "\t\tvar __allplaying = false;\r\n"
								      "\t\tif (__allplayingValue !== void) __allplaying = __allplayingValue ? true : false;\r\n"
									      "\t\tvar __selfMotionPlaying = global.Scripts.tryPropGet(__motionSelf, \"_motionPlaying\", false) ? true : false;\r\n"
									      "\t\tvar __nodeVisible = global.Scripts.tryPropGet(__motionSelf, \"nodeVisible\", true) ? true : false;\r\n"
								      "\t\tvar __motionPlaying = __selfMotionPlaying || __playing;\r\n"
								      "\t\tif (__motionPlaying || (__allplaying && __nodeVisible)) {\r\n"
									      "\t\t\tif (typeof __krkrMotionBurstTraceEnabled == \"Function\" && __krkrMotionBurstTraceEnabled()) dm(\"[MOTION-WORK-BURST] tick=\" + tick + \" dt=\" + (tick - _motionLastTick) + \" selfPlaying=\" + __selfMotionPlaying + \" playing=\" + __playing + \" allplaying=\" + __allplaying + \" nodeVisible=\" + __nodeVisible + \" motion=\" + global.Scripts.tryPropGet(__mplayer, \"motion\", \"void\") + \" chara=\" + global.Scripts.tryPropGet(__mplayer, \"chara\", \"void\"));\r\n"
									      "\t\t\tcheckInit();\r\n"
									      "\t\t\t_motionInterval = tick - _motionLastTick;\r\n"
									      "\t\t\tif (__canProgress) { try { __mplayer.progress(_motionInterval); } catch(e) { __canProgress = false; } }\r\n"
									      "\t\t\t__playingValue = global.Scripts.tryPropGet(__mplayer, \"playing\", void);\r\n"
									      "\t\t\t__motionPlayingValue = global.Scripts.tryPropGet(__mplayer, \"motionPlaying\", void);\r\n"
									      "\t\t\t__allplayingValue = global.Scripts.tryPropGet(__mplayer, \"allplaying\", void);\r\n"
									      "\t\t\t__playing = false;\r\n"
									      "\t\t\tif (__motionPlayingValue !== void) __playing = __motionPlayingValue ? true : false;\r\n"
									      "\t\t\tif (!__playing && __playingValue !== void) __playing = __playingValue ? true : false;\r\n"
									      "\t\t\t__allplaying = false;\r\n"
									      "\t\t\tif (__allplayingValue !== void) __allplaying = __allplayingValue ? true : false;\r\n"
									      "\t\t\tvar newCommandList = [];\r\n"
									      "\t\t\tif (__canGetCommands) { try { newCommandList = __mplayer.getCommandList(); } catch(e) { __canGetCommands = false; newCommandList = []; } }\r\n"
								      "\t\t\tif (_motionCommandList === void || typeof _motionCommandList != \"Object\") _motionCommandList = [];\r\n"
								      "\t\t\tif (newCommandList === void || typeof newCommandList != \"Object\") newCommandList = [];\r\n"
								      "\t\t\tvar __motionListsEqual = false;\r\n"
								      "\t\t\tif ((typeof _motionCommandList) == \"Object\" && (typeof newCommandList) == \"Object\") {\r\n"
								      "\t\t\t\t__motionListsEqual = _motionCommandList.count == newCommandList.count;\r\n"
								      "\t\t\t\tif (__motionListsEqual) {\r\n"
								      "\t\t\t\t\tfor (var __motionListIndex = 0; __motionListIndex < newCommandList.count; __motionListIndex++) {\r\n"
								      "\t\t\t\t\t\tif (_motionCommandList[__motionListIndex] != newCommandList[__motionListIndex]) {\r\n"
								      "\t\t\t\t\t\t\t__motionListsEqual = false;\r\n"
								      "\t\t\t\t\t\t\tbreak;\r\n"
								      "\t\t\t\t\t\t}\r\n"
								      "\t\t\t\t\t}\r\n"
								      "\t\t\t\t}\r\n"
								      "\t\t\t}\r\n"
								      "\t\t\tif (!__motionListsEqual) {\r\n"
								      "\t\t\t\tupdate(0,0,1,1);\r\n"
								      "\t\t\t\t_motionCommandList = newCommandList;\r\n"
								      "\t\t\t}\r\n"
								      "\t\t}\r\n"
								      "\t\tif (!__playing && __selfMotionPlaying) {\r\n"
								      "\t\t\tcheckStopMotion();\r\n"
								      "\t\t}\r\n"
								      "\t\t_motionLastTick = tick;\r\n"
								      "\t}\r\n"),
								false);
						buffer.Replace(
							TJS_W("\tfunction clearMotionVariables() {\r\n"
						      "\t\t(Dictionary.clear incontextof _motionVariables)();\r\n"
						      "\t}\r\n"),
						TJS_W("\tfunction clearMotionVariables() {\r\n"
						      "\t\tif (_motionVariables === void || typeof _motionVariables != \"Object\") _motionVariables = %[];\r\n"
						      "\t\t(Dictionary.clear incontextof _motionVariables)();\r\n"
						      "\t}\r\n"),
						false);
								buffer.Replace(
									TJS_W("\t\t\treturn mplayer !== void;\r\n"),
										TJS_W("\t\t\tvar __mplayer = global.Scripts.tryPropGet(this, \"mplayer\", void);\r\n"
										      "\t\t\treturn __mplayer !== void && (typeof __mplayer) == \"Object\";\r\n"),
										false);
							buffer.Replace(
								TJS_W("\t\t\treturn _motionPlaying || (mplayer !== void && mplayer.playing);\r\n"),
									TJS_W("\t\t\tvar __selfMotionPlaying = global.Scripts.tryPropGet(this, \"_motionPlaying\", false) ? true : false;\r\n"
									      "\t\t\tif (__selfMotionPlaying) return true;\r\n"
									      "\t\t\tvar __mplayer = global.Scripts.tryPropGet(this, \"mplayer\", void);\r\n"
									      "\t\t\tif (__mplayer === void || (typeof __mplayer) != \"Object\") return false;\r\n"
									      "\t\t\tvar __motionPlayingValue = global.Scripts.tryPropGet(__mplayer, \"motionPlaying\", void);\r\n"
									      "\t\t\tif (__motionPlayingValue !== void) return __motionPlayingValue ? true : false;\r\n"
									      "\t\t\tvar __playingValue = global.Scripts.tryPropGet(__mplayer, \"playing\", void);\r\n"
									      "\t\t\tif (__playingValue !== void) return __playingValue ? true : false;\r\n"
									      "\t\t\treturn false;\r\n"),
								false);
							buffer.Replace(
								TJS_W("\tfunction checkStartMotion() {\r\n"
								      "\t\tif (!_motionPlaying) {\r\n"),
										TJS_W("\tfunction checkStartMotion() {\r\n"
										      "\t\tvar __motionPlayingState = false;\r\n"
									      "\t\t__motionPlayingState = global.Scripts.tryPropGet(this, \"_motionPlaying\", false) ? true : false;\r\n"
									      "\t\tif (!__motionPlayingState) {\r\n"),
										false);
							buffer.Replace(
								TJS_W("\t\t\tvar motion = mplayer.motion;\r\n"
								      "\t\t\tif (typeof window.onMotionStart == \"Object\") window.onMotionStart(this);\r\n"
								      "\t\t\tonMotionStart(motion);\r\n"
								      "\t\t\texecMotionFunc(\"onStart\", motion);\r\n"),
								TJS_W("\t\t\tvar __mplayer = global.Scripts.tryPropGet(this, \"mplayer\", void);\r\n"
								      "\t\t\tvar motion = (__mplayer !== void && (typeof __mplayer) == \"Object\") ? global.Scripts.tryPropGet(__mplayer, \"motion\", void) : void;\r\n"
								      "\t\t\ttry { if ((typeof window.onMotionStart) == \"Object\") window.onMotionStart(this); } catch(e) {}\r\n"
								      "\t\t\ttry { onMotionStart(motion); } catch(e) {}\r\n"
								      "\t\t\ttry { execMotionFunc(\"onStart\", motion); } catch(e) {}\r\n"),
								false);
							buffer.Replace(
								TJS_W("\t\t\t_motionPlaying = true;\r\n"),
									TJS_W("\t\t\ttry { global.Scripts.propSet(this, \"_motionPlaying\", true); } catch(e) {}\r\n"),
									false);
							buffer.Replace(
								TJS_W("\tfunction checkStopMotion() {\r\n"
								      "\t\tif (_motionPlaying) {\r\n"),
										TJS_W("\tfunction checkStopMotion() {\r\n"
										      "\t\tvar __motionPlayingState = false;\r\n"
									      "\t\t__motionPlayingState = global.Scripts.tryPropGet(this, \"_motionPlaying\", false) ? true : false;\r\n"
									      "\t\tif (__motionPlayingState) {\r\n"),
										false);
							buffer.Replace(
								TJS_W("\t\t\tvar motion = mplayer.motion;\r\n"
								      "\t\t\tif (typeof window.onMotionStop == \"Object\") window.onMotionStop(this);\r\n"
								      "\t\t\tonMotionStop(motion);\r\n"
								      "\t\t\texecMotionFunc(\"onStop\", motion);\r\n"),
								TJS_W("\t\t\tvar __mplayer = global.Scripts.tryPropGet(this, \"mplayer\", void);\r\n"
								      "\t\t\tvar motion = (__mplayer !== void && (typeof __mplayer) == \"Object\") ? global.Scripts.tryPropGet(__mplayer, \"motion\", void) : void;\r\n"
								      "\t\t\ttry { if ((typeof window.onMotionStop) == \"Object\") window.onMotionStop(this); } catch(e) {}\r\n"
								      "\t\t\ttry { onMotionStop(motion); } catch(e) {}\r\n"
								      "\t\t\ttry { execMotionFunc(\"onStop\", motion); } catch(e) {}\r\n"),
								false);
							buffer.Replace(
								TJS_W("\t\t\t_motionPlaying = false;\r\n"),
									TJS_W("\t\t\ttry { global.Scripts.propSet(this, \"_motionPlaying\", false); } catch(e) {}\r\n"),
									false);
							buffer.Replace(
								TJS_W("\tfunction skipMotion() {\r\n"
								      "\t\tif (motionWorking && _motionPlaying) {\r\n"),
										TJS_W("\tfunction skipMotion() {\r\n"
										      "\t\tvar __motionPlayingState = false;\r\n"
									      "\t\t__motionPlayingState = global.Scripts.tryPropGet(this, \"_motionPlaying\", false) ? true : false;\r\n"
									      "\t\tif (motionWorking && __motionPlayingState) {\r\n"),
										false);
					buffer.Replace(
						TJS_W("if (typeof window.onPrimaryClick == \"Object\")\n\t\t\t\t\t\twindow.onPrimaryClick();"),
						TJS_W("if (typeof window.onPrimaryClick == \"Object\") {"
					      "dm(\"[INPUT-TJS] onPrimaryClick before x=\" + x + \" y=\" + y + \" clickWaiting=\" + window.clickWaiting + \" isWaiting=\" + window.isWaiting);"
					      "window.onPrimaryClick();"
					      "dm(\"[INPUT-TJS] onPrimaryClick after clickWaiting=\" + window.clickWaiting + \" isWaiting=\" + window.isWaiting);"
					      "}"),
					true);
					buffer.Replace(
						TJS_W("\tfunction onMouseDown(x, y, button) {\r\n"
						      "\t\tif (motionStable) {"),
						TJS_W("\tfunction onMouseDown(x, y, button) {\r\n"
						      "\t\t_motionDiagRememberPoint(x, y);\r\n"
						      "\t\tdm(\"[MOTION-DOWN] x=\" + x + \" y=\" + y + \" button=\" + button + \" stable=\" + motionStable + \" lock=\" + motionLock + \" focus=\" + motionFocus + \" buttons=\" + _motionButtons.count + \" areas=\" + _motionAreas.count + \" system=\" + systemMotion + \" inStable=\" + window.inStable + \" playing=\" + (typeof mplayer == \"Object\" ? mplayer.playing : \"void\") + (global._motionDiagResolveText incontextof this)(x, y));\r\n"
						      "\t\tif (motionStable) {"),
						false);
					buffer.Replace(
						TJS_W("\tfunction onMouseDown(x, y, button) {\r\n"
						      "\t\tif (!motionLock) {"),
						TJS_W("\tfunction onMouseDown(x, y, button) {\r\n"
						      "\t\t_motionDiagRememberPoint(x, y);\r\n"
						      "\t\tdm(\"[MOTION-DOWN] x=\" + x + \" y=\" + y + \" button=\" + button + \" lock=\" + motionLock + \" focus=\" + motionFocus + \" buttons=\" + _motionButtons.count + \" areas=\" + _motionAreas.count + \" system=\" + systemMotion + \" chara=\" + (typeof mplayer == \"Object\" ? mplayer.chara : \"void\") + \" motionName=\" + (typeof mplayer == \"Object\" ? mplayer.motion : \"void\") + (global._motionDiagResolveText incontextof this)(x, y));\r\n"
						      "\t\tif (!motionLock) {"),
							false);
					buffer.Replace(
						TJS_W("\t\t\t\t\tif (!button.disable && typeof button.onPress != \"undefined\") {\r\n"
						      "\t\t\t\t\t\t_motionGrab = button.onPress(x, y);\r\n"
						      "\t\t\t\t\t\tmplayer.progress(0);"),
							TJS_W("\t\t\t\t\tif (!button.disable && typeof button.onPress != \"undefined\") {\r\n"
							      "\t\t\t\t\t\tvar __diagPress = (typeof global != \"undefined\" && typeof global.__krkrDiagTraceSlotButtons != \"undefined\" && global.__krkrDiagTraceSlotButtons === true);\r\n"
							      "\t\t\t\t\t\tvar __pressVarName = typeof button.varname != \"undefined\" ? button.varname : \"\";\r\n"
							      "\t\t\t\t\t\tif (__diagPress) {\r\n"
						      "\t\t\t\t\t\t\tvar __pressValue = \"void\";\r\n"
					      "\t\t\t\t\t\t\ttry { if (__pressVarName != \"\") __pressValue = button.getVariable(__pressVarName); } catch(e) { __pressValue = \"ERR:\" + e.message; }\r\n"
					      "\t\t\t\t\t\t\tvar __pressArea = \"void\";\r\n"
					      "\t\t\t\t\t\t\ttry { if (typeof button.getArea != \"undefined\") __pressArea = button.getArea(x, y); } catch(e) { __pressArea = \"ERR:\" + e.message; }\r\n"
						      "\t\t\t\t\t\t\tdm(\"[MOTION-CHECK] kind=press-before x=\" + x + \" y=\" + y + \" index=\" + _motionFocus + \" name=\" + button.name + \" disable=\" + button.disable + \" var=\" + __pressVarName + \" value=\" + __pressValue + \" area=\" + __pressArea + \" changeType=\" + (typeof button.change != \"undefined\" ? typeof button.change : \"undefined\") + \" expType=\" + (typeof button.exp != \"undefined\" ? typeof button.exp : \"undefined\") + (global._motionDiagResolveText incontextof this)(x, y));\r\n"
						      "\t\t\t\t\t\t}\r\n"
						      "\t\t\t\t\t\t_motionGrab = button.onPress(x, y);\r\n"
						      "\t\t\t\t\t\tif (__diagPress) dm(\"[MOTION-CHECK] kind=press-after x=\" + x + \" y=\" + y + \" index=\" + _motionFocus + \" name=\" + button.name + \" grab=\" + _motionGrab + (global._motionDiagResolveText incontextof this)(x, y));\r\n"
						      "\t\t\t\t\t\tmplayer.progress(0);"),
					false);
				buffer.Replace(
					TJS_W("\tfunction updateData(newvalue, drag=false, force=false) {\r\n"
					      "\t\tvar value = getVariable(varname);"),
					TJS_W("\tfunction updateData(newvalue, drag=false, force=false) {\r\n"
					      "\t\tvar value = getVariable(varname);\r\n"
					      "\t\tvar __diagMotionValue = (typeof global != \"undefined\" && typeof global.__krkrDiagTraceSlotButtons != \"undefined\" && global.__krkrDiagTraceSlotButtons === true);\r\n"
					      "\t\tvar __diagMotionValueName = \"\";\r\n"
					      "\t\tif (__diagMotionValue) { try { __diagMotionValueName = valueInfo.valuename; } catch(e) { __diagMotionValueName = \"ERR:\" + e.message; } }\r\n"
					      "\t\tif (__diagMotionValue) dm(\"[MOTION-CHECK] kind=value-begin name=\" + name + \" var=\" + varname + \" valueName=\" + __diagMotionValueName + \" old=\" + value + \" requested=\" + newvalue + \" drag=\" + drag + \" force=\" + force + \" max=\" + max + \" changeType=\" + typeof change + \" expType=\" + typeof exp);"),
					false);
				buffer.Replace(
					TJS_W("\t\tif (newvalue != value || force) {\r\n"
					      "\t\t\towner.exec(change, this, newvalue, drag);"),
					TJS_W("\t\tif (__diagMotionValue && !(newvalue != value || force)) dm(\"[MOTION-CHECK] kind=value-noop name=\" + name + \" var=\" + varname + \" old=\" + value + \" requested=\" + newvalue + \" force=\" + force);\r\n"
					      "\t\tif (newvalue != value || force) {\r\n"
					      "\t\t\tvar __diagDryRunMotionAction = false;\r\n"
					      "\t\t\tif (typeof global != \"undefined\" && typeof global.__krkrDiagDryRunMotionActionPrefixes != \"undefined\") {\r\n"
					      "\t\t\t\tvar __diagDryRunPrefixes = \"\" + global.__krkrDiagDryRunMotionActionPrefixes;\r\n"
					      "\t\t\t\tvar __diagDryRunPrefixList = __diagDryRunPrefixes.split(\",\");\r\n"
					      "\t\t\t\tfor (var __diagDryRunPrefixIndex = 0; __diagDryRunPrefixIndex < __diagDryRunPrefixList.count; __diagDryRunPrefixIndex++) {\r\n"
					      "\t\t\t\t\tvar __diagDryRunPrefix = __diagDryRunPrefixList[__diagDryRunPrefixIndex];\r\n"
					      "\t\t\t\t\tif (__diagDryRunPrefix != \"\" && name.indexOf(__diagDryRunPrefix) == 0) { __diagDryRunMotionAction = true; break; }\r\n"
					      "\t\t\t\t}\r\n"
					      "\t\t\t}\r\n"
					      "\t\t\tif (__diagDryRunMotionAction) {\r\n"
					      "\t\t\t\tdm(\"[MOTION-ACTION-DRYRUN] kind=value name=\" + name + \" var=\" + varname + \" old=\" + value + \" new=\" + newvalue + \" drag=\" + drag + \" force=\" + force + \" changeType=\" + typeof change + \" expType=\" + typeof exp + (global._motionDiagResolveText incontextof this)(global._motionDiagLastX, global._motionDiagLastY));\r\n"
					      "\t\t\t\treturn;\r\n"
					      "\t\t\t}\r\n"
					      "\t\t\tif (__diagMotionValue) dm(\"[MOTION-ACTION] kind=value name=\" + name + \" var=\" + varname + \" valueName=\" + __diagMotionValueName + \" old=\" + value + \" new=\" + newvalue + \" drag=\" + drag + \" force=\" + force + \" changeType=\" + typeof change + \" expType=\" + typeof exp + (global._motionDiagResolveText incontextof this)(global._motionDiagLastX, global._motionDiagLastY));\r\n"
					      "\t\t\towner.exec(change, this, newvalue, drag);"),
					false);
				buffer.Replace(
					TJS_W("\t\t\towner.exec(change, this, newvalue, drag);\r\n"
					      "\t\t\tsetVariable(varname, newvalue);"),
					TJS_W("\t\t\towner.exec(change, this, newvalue, drag);\r\n"
					      "\t\t\tsetVariable(varname, newvalue);\r\n"
					      "\t\t\tif (__diagMotionValue) {\r\n"
					      "\t\t\t\tvar __diagMotionRuntimeValue = \"void\";\r\n"
					      "\t\t\t\ttry { __diagMotionRuntimeValue = getVariable(varname); } catch(e) { __diagMotionRuntimeValue = \"ERR:\" + e.message; }\r\n"
					      "\t\t\t\tdm(\"[MOTION-CHECK] kind=value-runtime name=\" + name + \" var=\" + varname + \" value=\" + __diagMotionRuntimeValue);\r\n"
					      "\t\t\t}"),
					false);
				buffer.Replace(
					TJS_W("\t\t\tsetValue(newvalue);"),
					TJS_W("\t\t\tvar __diagMotionSystemBefore = \"\";\r\n"
					      "\t\t\tif (__diagMotionValue) { try { __diagMotionSystemBefore = getValue(); } catch(e) { __diagMotionSystemBefore = \"ERR:\" + e.message; } }\r\n"
					      "\t\t\tsetValue(newvalue);\r\n"
					      "\t\t\ttry {\r\n"
					      "\t\t\t\tvar __krkrMotionValueName = \"\";\r\n"
					      "\t\t\t\ttry { __krkrMotionValueName = valueInfo.valuename; } catch(__krkrValueNameError) { __krkrMotionValueName = \"\"; }\r\n"
						      "\t\t\t\tif (!drag) {\r\n"
						      "\t\t\t\t\tvar __krkrRequestSystemVariablesSave = global.Scripts.tryPropGet(global.Scripts, \"requestSystemVariablesSave\", void);\r\n"
						      "\t\t\t\t\tif ((typeof __krkrRequestSystemVariablesSave) == \"Object\") {\r\n"
						      "\t\t\t\t\t\tglobal.Scripts.requestSystemVariablesSave(\"MotionValue:\" + name + \":\" + __krkrMotionValueName);\r\n"
						      "\t\t\t\t\t\ttry { dm(\"[SYSVAR-SAVE] source=MotionValue requested name=\" + name + \" valueName=\" + __krkrMotionValueName + \" drag=\" + drag + \" force=\" + force); } catch(__krkrMotionSaveLogError0) {}\r\n"
						      "\t\t\t\t\t}\r\n"
					      "\t\t\t\t} else {\r\n"
					      "\t\t\t\t\ttry { dm(\"[SYSVAR-SAVE] source=MotionValue skipped=drag name=\" + name + \" valueName=\" + __krkrMotionValueName); } catch(__krkrMotionSaveSkipLogError) {}\r\n"
					      "\t\t\t\t}\r\n"
					      "\t\t\t} catch(__krkrMotionSaveError) {\r\n"
					      "\t\t\t\ttry { dm(\"[SYSVAR-SAVE] source=MotionValue failed name=\" + name + \" message=\" + __krkrMotionSaveError.message); } catch(__krkrMotionSaveLogError) {}\r\n"
					      "\t\t\t}\r\n"
					      "\t\t\tif (__diagMotionValue) {\r\n"
					      "\t\t\t\tvar __diagMotionSystemAfter = \"\";\r\n"
					      "\t\t\t\ttry { __diagMotionSystemAfter = getValue(); } catch(e) { __diagMotionSystemAfter = \"ERR:\" + e.message; }\r\n"
					      "\t\t\t\tdm(\"[MOTION-CHECK] kind=value-system name=\" + name + \" var=\" + varname + \" valueName=\" + __diagMotionValueName + \" before=\" + __diagMotionSystemBefore + \" after=\" + __diagMotionSystemAfter + \" requested=\" + newvalue);\r\n"
					      "\t\t\t}"),
					false);
				buffer.Replace(
					TJS_W("\t\t\towner.exec(exp, this, owner);"),
					TJS_W("\t\t\towner.exec(exp, this, owner);\r\n"
					      "\t\t\tif (__diagMotionValue) dm(\"[MOTION-CHECK] kind=value-exp-after name=\" + name + \" var=\" + varname + \" value=\" + getVariable(varname));"),
					false);
				buffer.Replace(
					TJS_W("\tfunction onPress(x, y) {\r\n"
					      "\t\tvar n = getArea(x, y);\r\n"
					      "\t\tif (n === void) {\r\n"
					      "\t\t\tn = (getVariable(varname) + 1) % (max+1);\r\n"
					      "\t\t}\r\n"
					      "\t\tupdateData(n);\r\n"
					      "\t}\r\n"),
					TJS_W("\tfunction onPress(x, y) {\r\n"
					      "\t\tvar __diagMotionToggle = (typeof global != \"undefined\" && typeof global.__krkrDiagTraceSlotButtons != \"undefined\" && global.__krkrDiagTraceSlotButtons === true);\r\n"
					      "\t\tvar __oldToggleValue = getVariable(varname);\r\n"
					      "\t\tvar n = getArea(x, y);\r\n"
					      "\t\tif (__diagMotionToggle) dm(\"[MOTION-CHECK] kind=toggle-press name=\" + name + \" x=\" + x + \" y=\" + y + \" var=\" + varname + \" old=\" + __oldToggleValue + \" area=\" + n + \" max=\" + max + (global._motionDiagResolveText incontextof this)(x, y));\r\n"
					      "\t\tif (n === void) {\r\n"
					      "\t\t\tn = (getVariable(varname) + 1) % (max+1);\r\n"
					      "\t\t}\r\n"
					      "\t\tif (__diagMotionToggle) dm(\"[MOTION-CHECK] kind=toggle-resolve name=\" + name + \" old=\" + __oldToggleValue + \" new=\" + n);\r\n"
					      "\t\tupdateData(n);\r\n"
					      "\t}\r\n"),
					false);
					buffer.Replace(
						TJS_W("\t\t\t\tif (button.checkMouseMove(x, y)) {\r\n"
						      "\t\t\t\t\tmotionFocus = i;"),
							      TJS_W("\t\t\t\tif (typeof global != \"undefined\" && typeof global.__krkrDiagTraceSlotButtons != \"undefined\" && global.__krkrDiagTraceSlotButtons === true && (((x >= 30 && x <= 630 && y >= 80 && y <= 205) || (x >= 0 && x <= 650 && y >= 40 && y <= 360) || y >= 620 || systemMotion || mplayer.chara == \"CONFIG_SCREEN\"))) {\r\n"
						      "\t\t\t\t\tvar __diagHitFocus = button.disable ? false : button.checkMouseMove(x, y);\r\n"
						      "\t\t\t\t\tdm(\"[MOTION-CANDIDATE] x=\" + x + \" y=\" + y + \" index=\" + i + \" name=\" + button.name + \" disable=\" + button.disable + \" hit=\" + __diagHitFocus + \" chara=\" + mplayer.chara + \" motionName=\" + mplayer.motion + (global._motionDiagResolveText incontextof this)(x, y));\r\n"
						      "\t\t\t\t}\r\n"
						      "\t\t\t\tif (button.checkMouseMove(x, y)) {\r\n"
						      "\t\t\t\t\tdm(\"[MOTION-FOCUS] x=\" + x + \" y=\" + y + \" index=\" + i + \" name=\" + button.name + \" disable=\" + button.disable + (global._motionDiagResolveText incontextof this)(x, y));\r\n"
						      "\t\t\t\t\tmotionFocus = i;"),
						false);
				buffer.Replace(
					TJS_W("\t\t\t\t// [???] ������Ԃ̂��̂̓`�F�b�N�ΏۊO�iMotionArea.checkMouseMove�ɓ����ׂ��H�j\r\n"
					      "\t\t\t\tif (button.disable) continue;\r\n\r\n"
					      "\t\t\t\tif (button.checkMouseMove(x, y)) {"),
					      TJS_W("\t\t\t\t// [???] ������Ԃ̂��̂̓`�F�b�N�ΏۊO�iMotionArea.checkMouseMove�ɓ����ׂ��H�j\r\n"
						      "\t\t\t\tif (typeof global != \"undefined\" && typeof global.__krkrDiagTraceSlotButtons != \"undefined\" && global.__krkrDiagTraceSlotButtons === true && (((x >= 30 && x <= 630 && y >= 80 && y <= 205) || (x >= 0 && x <= 650 && y >= 40 && y <= 360) || y >= 620 || systemMotion || mplayer.chara == \"CONFIG_SCREEN\"))) {\r\n"
					      "\t\t\t\t\tvar __diagHit = button.disable ? false : button.checkMouseMove(x, y);\r\n"
					      "\t\t\t\t\tdm(\"[MOTION-CANDIDATE] x=\" + x + \" y=\" + y + \" index=\" + i + \" name=\" + button.name + \" disable=\" + button.disable + \" hit=\" + __diagHit + \" chara=\" + mplayer.chara + \" motionName=\" + mplayer.motion + (global._motionDiagResolveText incontextof this)(x, y));\r\n"
					      "\t\t\t\t}\r\n"
						      "\t\t\t\tif (typeof global != \"undefined\" && typeof global.__krkrDiagTraceSlotButtons != \"undefined\" && global.__krkrDiagTraceSlotButtons === true && (((x >= 30 && x <= 630 && y >= 80 && y <= 205) || (x >= 0 && x <= 650 && y >= 40 && y <= 360) || y >= 620 || systemMotion || mplayer.chara == \"CONFIG_SCREEN\"))) {\r\n"
					      "\t\t\t\t\tvar __slotGetter = getLayerGetter(button.name);\r\n"
					      "\t\t\t\t\tvar __slotDirectGetter = mplayer.getLayerGetter(button.name);\r\n"
					      "\t\t\t\t\tvar __slotMotion = (__slotGetter !== void ? __slotGetter.motion : void);\r\n"
					      "\t\t\t\t\tvar __slotDirectMotion = (__slotDirectGetter !== void ? __slotDirectGetter.motion : void);\r\n"
					      "\t\t\t\t\tvar __slotRect = (__slotGetter !== void ? (__slotGetter.left + \",\" + __slotGetter.top + \",\" + __slotGetter.width + \",\" + __slotGetter.height) : \"void\");\r\n"
					      "\t\t\t\t\tvar __slotDirectRect = (__slotDirectGetter !== void ? (__slotDirectGetter.left + \",\" + __slotDirectGetter.top + \",\" + __slotDirectGetter.width + \",\" + __slotDirectGetter.height) : \"void\");\r\n"
					      "\t\t\t\t\tvar __slotShapeRect = (__slotMotion !== void ? (__slotMotion.l + \",\" + __slotMotion.t + \",\" + __slotMotion.w + \",\" + __slotMotion.h) : \"void\");\r\n"
					      "\t\t\t\t\tdm(\"[MOTION-CHECK] x=\" + x + \" y=\" + y + \" name=\" + button.name + \" disable=\" + button.disable + \" getter=\" + (__slotGetter !== void) + \" directGetter=\" + (__slotDirectGetter !== void) + \" rect=\" + __slotRect + \" directRect=\" + __slotDirectRect + \" motion=\" + (__slotMotion !== void) + \" directMotion=\" + (__slotDirectMotion !== void) + \" contains=\" + (__slotMotion !== void ? __slotMotion.contains(x, y) : \"void\") + \" directContains=\" + (__slotDirectMotion !== void ? __slotDirectMotion.contains(x, y) : \"void\") + \" playerContains=\" + mplayer.contains(button.name, x, y) + \" shapeRect=\" + __slotShapeRect + \" chara=\" + mplayer.chara + \" motionName=\" + mplayer.motion + (global._motionDiagResolveText incontextof this)(x, y));\r\n"
					      "\t\t\t\t}\r\n"
					      "\t\t\t\tif (button.disable) continue;\r\n\r\n"
					      "\t\t\t\tif (button.checkMouseMove(x, y)) {"),
					false);
				buffer.Replace(
					TJS_W("\t\t\tmotionFocus = void;\r\n"
					      "\t\t\tcursor = window.cursorDefault;"),
					TJS_W("\t\t\tdm(\"[MOTION-FOCUS] clear x=\" + x + \" y=\" + y + \" buttons=\" + _motionButtons.count);\r\n"
					      "\t\t\tmotionFocus = void;\r\n"
					      "\t\t\tcursor = window.cursorDefault;"),
					false);
					buffer.Replace(
						TJS_W("\t\texec(button.change, button, 0, false);"),
						TJS_W("\t\tdm(\"[MOTION-ACTION] name=\" + button.name + \" id=\" + button.id + \" storage=\" + button.storage + \" target=\" + button.target + \" expType=\" + typeof button.exp + \" exp=\" + (typeof button.exp == \"String\" ? button.exp : \"\") + (global._motionDiagResolveText incontextof this)(global._motionDiagLastX, global._motionDiagLastY));\r\n"
						      "\t\tvar __diagDryRunMotionAction = false;\r\n"
						      "\t\tif (typeof global != \"undefined\" && typeof global.__krkrDiagDryRunSlotAction != \"undefined\" && global.__krkrDiagDryRunSlotAction === true && button.name.indexOf(\"slot\") >= 0) __diagDryRunMotionAction = true;\r\n"
						      "\t\tif (typeof global != \"undefined\" && typeof global.__krkrDiagDryRunMotionActionPrefixes != \"undefined\") {\r\n"
						      "\t\t\tvar __diagDryRunPrefixes = \"\" + global.__krkrDiagDryRunMotionActionPrefixes;\r\n"
						      "\t\t\tif (__diagDryRunPrefixes != \"\") {\r\n"
						      "\t\t\t\tvar __diagDryRunPrefixList = __diagDryRunPrefixes.split(\",\");\r\n"
						      "\t\t\t\tfor (var __diagDryRunPrefixIndex = 0; __diagDryRunPrefixIndex < __diagDryRunPrefixList.count; __diagDryRunPrefixIndex++) {\r\n"
						      "\t\t\t\t\tvar __diagDryRunPrefix = __diagDryRunPrefixList[__diagDryRunPrefixIndex];\r\n"
						      "\t\t\t\t\tif (__diagDryRunPrefix != \"\" && button.name.indexOf(__diagDryRunPrefix) == 0) { __diagDryRunMotionAction = true; break; }\r\n"
						      "\t\t\t\t}\r\n"
						      "\t\t\t}\r\n"
						      "\t\t}\r\n"
						      "\t\tif (__diagDryRunMotionAction) {\r\n"
						      "\t\t\tdm(\"[MOTION-ACTION-DRYRUN] name=\" + button.name + \" id=\" + button.id + \" storage=\" + button.storage + \" target=\" + button.target + \" prefixes=\" + (typeof global != \"undefined\" && typeof global.__krkrDiagDryRunMotionActionPrefixes != \"undefined\" ? global.__krkrDiagDryRunMotionActionPrefixes : \"\") + (global._motionDiagResolveText incontextof this)(global._motionDiagLastX, global._motionDiagLastY));\r\n"
						      "\t\t\treturn true;\r\n"
						      "\t\t}\r\n"
						      "\t\texec(button.change, button, 0, false);"),
							false);
				}
			if(shortname == TJS_W("system.tjs")) {
				buffer.Replace(
					TJS_W("\tfunction  hide(*)   { return do_(_hide,, *); }\r\n"),
					TJS_W("\tfunction  hide(*)   { dm(\"[SYSTEM-ACTION] name=hide\"); return do_(_hide,, *); }\r\n"),
					false);
				buffer.Replace(
					TJS_W("\tfunction  option(*) { return _tglSubMode(\"option\", \"toggleOption\", _option,\"configMenuItem\", *); }\r\n"),
					TJS_W("\tfunction  option(*) { dm(\"[SYSTEM-ACTION] name=option\"); return _tglSubMode(\"option\", \"toggleOption\", _option,\"configMenuItem\", *); }\r\n"),
					false);
				buffer.Replace(
					TJS_W("\tfunction  save(*) { return _tglSubMode(\"save\", \"toggleSave\", _save, \"storeMenu\",   *); }\r\n"),
					TJS_W("\tfunction  save(*) { dm(\"[SYSTEM-ACTION] name=save\"); return _tglSubMode(\"save\", \"toggleSave\", _save, \"storeMenu\",   *); }\r\n"),
					false);
				buffer.Replace(
					TJS_W("\tfunction  load(*) { return _tglSubMode(\"load\", \"toggleLoad\", _load, \"restoreMenu\", *); }\r\n"),
					TJS_W("\tfunction  load(*) { dm(\"[SYSTEM-ACTION] name=load\"); return _tglSubMode(\"load\", \"toggleLoad\", _load, \"restoreMenu\", *); }\r\n"),
					false);
				buffer.Replace(
					TJS_W("\tfunction  qsave(*) { return do_(_qsave_, \"quickStoreMenu\",   *); }\r\n"),
					TJS_W("\tfunction  qsave(*) { dm(\"[SYSTEM-ACTION] name=qsave\"); return do_(_qsave_, \"quickStoreMenu\",   *); }\r\n"),
					false);
				buffer.Replace(
					TJS_W("\tfunction  qload(*) {        do_(_qload_, \"quickRestoreMenu\", *) if (canQuickLoad); return true; }\r\n"),
					TJS_W("\tfunction  qload(*) { dm(\"[SYSTEM-ACTION] name=qload canQuickLoad=\" + canQuickLoad);        do_(_qload_, \"quickRestoreMenu\", *) if (canQuickLoad); return true; }\r\n"),
					false);
			}
			if(shortname == TJS_W("kagenvplayer.tjs")) {
#ifdef __EMSCRIPTEN__
			fprintf(stderr, "[SCRIPT-LOAD] shortname=%s\n", shortname.AsNarrowStdString().c_str());
#endif
			buffer.Replace(
				TJS_W("function start(elm) {\r\n"
				      "\t\tclear();\r\n"
				      "\t\tstartScene(elm.storage, elm.target, elm.point);\r\n"
				      "\t\treturn 0;\r\n"
				      "\t}"),
				TJS_W("function start(elm) {\r\n"
				      "\t\tdm(\"[KENV] start storage=\" + elm.storage + \" target=\" + elm.target + \" point=\" + elm.point);\r\n"
				      "\t\tclear();\r\n"
				      "\t\tstartScene(elm.storage, elm.target, elm.point);\r\n"
				      "\t\tdm(\"[KENV] start-after scene=\" + curSceneName + \" cur=\" + cur + \" point=\" + curPoint + \" scenario=\" + (scenario !== void) + \" lines=\" + ((scenario !== void && scenario.lines !== void) ? scenario.lines.count : -1));\r\n"
				      "\t\treturn 0;\r\n"
				      "\t}"),
				false);
			buffer.Replace(
				TJS_W("\t\t// \x81\x40\x8d\x73\x82\xf0\x8e\xe6\x82\xe8\x8fo\x82\xb7\r\n"
				      "\t\tvar obj = getLine(cur++);\r\n"),
				TJS_W("\t\t// \x81\x40\x8d\x73\x82\xf0\x8e\xe6\x82\xe8\x8fo\x82\xb7\r\n"
				      "\t\tvar __kenv_cur_before = cur;\r\n"
				      "\t\tvar obj = getLine(cur++);\r\n"
				      "\t\tvar __kenv_obj_info = \"\";\r\n"
				      "\t\tif (obj !== void) {\r\n"
				      "\t\t\tif (typeof obj == \"Object\") __kenv_obj_info = \" first=\" + obj[0] + \" count=\" + obj.count + \" savePoint=\" + obj[SAVE_POINT] + \" saveText=\" + obj[SAVE_TEXT];\r\n"
				      "\t\t\telse __kenv_obj_info = \" value=\" + obj;\r\n"
				      "\t\t}\r\n"
				      "\t\tdm(\"[KENV] sceneplay cur=\" + __kenv_cur_before + \" next=\" + cur + \" scene=\" + curSceneName + \" objType=\" + typeof obj + __kenv_obj_info);\r\n"),
				false);
			buffer.Replace(
				TJS_W("\t\tvar obj = getLine(cur++);\r\n"
				      "\r\n"
				      "\t\t// "),
				TJS_W("\t\tvar __kenv_cur_before = cur;\r\n"
				      "\t\tvar obj = getLine(cur++);\r\n"
				      "\t\tvar __kenv_obj_info = \"\";\r\n"
				      "\t\tif (obj !== void) {\r\n"
				      "\t\t\tif (typeof obj == \"Object\") __kenv_obj_info = \" first=\" + obj[0] + \" count=\" + obj.count + \" savePoint=\" + obj[SAVE_POINT] + \" saveText=\" + obj[SAVE_TEXT];\r\n"
				      "\t\t\telse __kenv_obj_info = \" value=\" + obj;\r\n"
				      "\t\t}\r\n"
				      "\t\tdm(\"[KENV] sceneplay cur=\" + __kenv_cur_before + \" next=\" + cur + \" scene=\" + curSceneName + \" objType=\" + typeof obj + __kenv_obj_info);\r\n"
				      "\r\n"
				      "\t\t// "),
				false);
			buffer.Replace(
				TJS_W("\t\t\t\tcurText = getText(curTextId);\r\n"
				      "\t\t\t\tif (curText === void) {"),
				TJS_W("\t\t\t\tcurText = getText(curTextId);\r\n"
				      "\t\t\t\tvar __kenv_text_info = \"\";\r\n"
				      "\t\t\t\tif (curText !== void && typeof curText == \"Object\") __kenv_text_info = \" text=\" + curText.text + \" name=\" + curText.name + \" disp=\" + curText.dispname;\r\n"
				      "\t\t\t\tdm(\"[KENV] gettext id=\" + curTextId + \" type=\" + typeof curText + __kenv_text_info);\r\n"
				      "\t\t\t\tif (curText === void) {"),
				false);
				buffer.Replace(
						TJS_W("\t\t\t\t\textractText(curText);\r\n"),
						TJS_W("\t\t\t\t\tdm(\"[KENV] extractText id=\" + curTextId);\r\n"
				      "\t\t\t\t\textractText(curText);\r\n"),
						false);
				}
				} else if(shortname == TJS_W("Initialize.tjs") ||
					shortname == TJS_W("animkaglayer.tjs") ||
					shortname == TJS_W("custom.tjs") ||
					shortname == TJS_W("kagenvplayer.tjs") ||
					shortname == TJS_W("system.tjs")) {
					TVPLogDracuScriptHack("disabled", shortname);
				} else if(shortname == TJS_W("kagenvironment.tjs") ||
					shortname == TJS_W("KAGEnvironment.tjs")) {
					KRKR_LOG_L2("[KENV-INJECT] entering kagenvironment inject, buflen=%d\n", (int)buffer.length());
					buffer.Replace(
						TJS_W("function addFastTag() {\r\n\t\tkag.addFastTag(...);\r\n\t}"),
						TJS_W("function addFastTag() {\r\n\t\tdm(\"[KENV-AFT] tagname=\" + (arguments[0] !== void ? arguments[0] : \"?\"));\r\n\t\tkag.addFastTag(...);\r\n\t}"),
						false);
					KRKR_LOG_L2("[KENV-INJECT] after replace, buflen=%d\n", (int)buffer.length());
				}
		#endif

		if(TVPScriptEngine)
		{
			if(!isexpression) {
				TVPScriptEngine->ExecScript(buffer, result, context,
					&shortname);
				if(false && shortname == TJS_W("motion.tjs")) {
					try {
						TVPExecuteScript(TJS_W(
							"try {"
							"  if (typeof global != \"undefined\" && typeof global.MotionResourceManager != \"undefined\""
							"      && typeof Window != \"undefined\" && Window.mainWindow !== void"
							"      && Window.mainWindow.motion_manager === void) {"
							"    Window.mainWindow.motion_manager = new global.MotionResourceManager(Window.mainWindow);"
							"    Debug.message('[emoteplayer] Window.mainWindow.motion_manager initialized');"
							"  }"
							"} catch(e) {"
							"  Debug.message('[emoteplayer] motion_manager setup failed: ' + e.message);"
							"}"));
					} catch(...) {
					}
				}
			}
			else
				TVPScriptEngine->EvalExpression(buffer, result, context,
					&shortname);
		}
	}
//---------------------------------------------------------------------------
void TVPCompileStorage( const ttstr& name, bool isrequestresult, bool outputdebug, bool isexpression, const ttstr& outputpath ) {
	// execute storage which contains script
	if(!TVPScriptEngine) TVPThrowInternalError;

	ttstr place(TVPSearchPlacedPath(name));
	ttstr shortname(TVPExtractStorageName(place));
	iTJSTextReadStream * stream = TVPCreateTextStreamForReadByEncoding(place, TJS_W(""),TVPScriptTextEncoding);

	ttstr buffer;
	try {
		stream->Read(buffer, 0);
	} catch(...) {
		stream->Destruct();
		throw;
	}
	stream->Destruct();

	tTJSBinaryStream* outputstream = TVPCreateStream(outputpath, TJS_BS_WRITE);
	if(TVPScriptEngine) {
		try {
			TVPScriptEngine->CompileScript( buffer.c_str(), outputstream, isrequestresult, outputdebug, isexpression, name.c_str(), 0 );
		} catch(...) {
			delete outputstream;
			throw;
		}
	}
	delete outputstream;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// TVPCreateMessageMapFile
//---------------------------------------------------------------------------
void TVPCreateMessageMapFile(const ttstr &filename)
{
#ifdef TJS_TEXT_OUT_CRLF
	ttstr script(TJS_W("{\r\n\tvar r = System.assignMessage;\r\n"));
#else
	ttstr script(TJS_W("{\n\tvar r = System.assignMessage;\n"));
#endif

	script += TJSCreateMessageMapString();

	script += TJS_W("}");

	iTJSTextWriteStream * stream = TVPCreateTextStreamForWrite(
		filename, TJS_W(""));
	try
	{
		stream->Write(script);
	}
	catch(...)
	{
		stream->Destruct();
		throw;
	}

	stream->Destruct();
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPDumpScriptEngine
//---------------------------------------------------------------------------
void TVPDumpScriptEngine()
{
	TVPTJS2StartDump();
	TVPScriptEngine->SetConsoleOutput(TVPGetTJS2DumpOutputGateway());
	try
	{
		TVPScriptEngine->Dump();
	}
	catch(...)
	{
		TVPTJS2EndDump();
		TVPScriptEngine->SetConsoleOutput(TVPGetTJS2ConsoleOutputGateway());
		throw;
	}
	TVPScriptEngine->SetConsoleOutput(TVPGetTJS2ConsoleOutputGateway());
	TVPTJS2EndDump();
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPExecuteStartupScript
//---------------------------------------------------------------------------
void TVPExecuteStartupScript()
{
	// execute "startup.tjs"
	try
	{
		try
		{
			TVPAddLog( TVPInfoLoadingStartupScript + TVPStartupScriptName );
			TVPExecuteStorage(TVPStartupScriptName);
			TVPAddLog( (const tjs_char*)TVPInfoStartupScriptEnded );
		}
		TJS_CONVERT_TO_TJS_EXCEPTION
	}
#if defined(__EMSCRIPTEN__) && defined(__EMSCRIPTEN_PTHREADS__)
	catch(eTJSScriptException &e)
	{
		TVPBeforeProcessUnhandledException();
		e.AddTrace(ttstr(TJS_W("startup")));
		if(!TVPProcessUnhandledException(e))
			TVPShowScriptException(e);
		TVPAddLog(TJS_W("(info) startup.tjs threw exception but continuing in Emscripten mode"));
		TVPAddLog( (const tjs_char*)TVPInfoStartupScriptEnded );
	}
	catch(eTJSScriptError &e)
	{
		TVPBeforeProcessUnhandledException();
		e.AddTrace(ttstr(TJS_W("startup")));
		if(!TVPProcessUnhandledException(e))
			TVPShowScriptException(e);
		TVPAddLog(TJS_W("(info) startup.tjs threw exception but continuing in Emscripten mode"));
		TVPAddLog( (const tjs_char*)TVPInfoStartupScriptEnded );
	}
	catch(eTJS &e)
	{
		TVPBeforeProcessUnhandledException();
		if(!TVPProcessUnhandledException(e))
			TVPShowScriptException(e);
		TVPAddLog(TJS_W("(info) startup.tjs threw exception but continuing in Emscripten mode"));
		TVPAddLog( (const tjs_char*)TVPInfoStartupScriptEnded );
	}
#else
	TVP_CATCH_AND_SHOW_SCRIPT_EXCEPTION(TJS_W("startup"))
#endif
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// unhandled exception handler related
//---------------------------------------------------------------------------
static bool  TJSGetSystem_exceptionHandler_Object(tTJSVariantClosure & dest)
{
	// get System.exceptionHandler
	iTJSDispatch2 * global = TVPGetScriptEngine()->GetGlobalNoAddRef();
	if(!global) return false;

	tTJSVariant val;
	tTJSVariant val2;
	tTJSVariantClosure clo;

	tjs_error er;
	er = global->PropGet(TJS_MEMBERMUSTEXIST, TJS_W("System"), NULL, &val, global);
	if(TJS_FAILED(er)) return false;

	if(val.Type() != tvtObject) return false;

	clo = val.AsObjectClosureNoAddRef();

	if(clo.Object == NULL) return false;

	clo.PropGet(TJS_MEMBERMUSTEXIST, TJS_W("exceptionHandler"), NULL, &val2, NULL);

	if(val2.Type() != tvtObject) return false;

	dest = val2.AsObjectClosure();

	if(!dest.Object)
	{
		dest.Release();
		return false;
	}

	return true;
}
//---------------------------------------------------------------------------
bool TVPProcessUnhandledException(eTJSScriptException &e)
{
	bool result;
	tTJSVariantClosure clo;
	clo.Object = clo.ObjThis = NULL;

	try
	{
		// get the script engine
		tTJS *engine = TVPGetScriptEngine();
		if(!engine)
			return false; // the script engine had been shutdown

		// get System.exceptionHandler
		if(!TJSGetSystem_exceptionHandler_Object(clo))
			return false; // System.exceptionHandler cannot be retrieved

		// execute clo
		tTJSVariant obj(e.GetValue());

		tTJSVariant *pval[] =  { &obj };

		tTJSVariant res;

		clo.FuncCall(0, NULL, NULL, &res, 1, pval, NULL);

		result = res.operator bool();
	}
	catch(eTJSScriptError &e)
	{
		clo.Release();
		TVPShowScriptException(e);
	}
	catch(eTJS &e)
	{
		clo.Release();
		TVPShowScriptException(e);
	}
	catch(...)
	{
		clo.Release();
		throw;
	}
	clo.Release();

	return result;
}
//---------------------------------------------------------------------------
bool TVPProcessUnhandledException(eTJSScriptError &e)
{
	bool result;
	tTJSVariantClosure clo;
	clo.Object = clo.ObjThis = NULL;

	try
	{
		// get the script engine
		tTJS *engine = TVPGetScriptEngine();
		if(!engine)
			return false; // the script engine had been shutdown

		// get System.exceptionHandler
		if(!TJSGetSystem_exceptionHandler_Object(clo))
			return false; // System.exceptionHandler cannot be retrieved

		// execute clo
		tTJSVariant obj;
		tTJSVariant msg(e.GetMessage());
		tTJSVariant trace(e.GetTrace());
		TJSGetExceptionObject(engine, &obj, msg, &trace);

		tTJSVariant *pval[] =  { &obj };

		tTJSVariant res;

		clo.FuncCall(0, NULL, NULL, &res, 1, pval, NULL);

		result = res.operator bool();
	}
	catch(eTJSScriptError &e)
	{
		clo.Release();
		TVPShowScriptException(e);
	}
	catch(eTJS &e)
	{
		clo.Release();
		TVPShowScriptException(e);
	}
	catch(...)
	{
		clo.Release();
		throw;
	}
	clo.Release();

	return result;
}
//---------------------------------------------------------------------------
bool TVPProcessUnhandledException(eTJS &e)
{
	bool result;
	tTJSVariantClosure clo;
	clo.Object = clo.ObjThis = NULL;

	try
	{
		// get the script engine
		tTJS *engine = TVPGetScriptEngine();
		if(!engine)
			return false; // the script engine had been shutdown

		// get System.exceptionHandler
		if(!TJSGetSystem_exceptionHandler_Object(clo))
			return false; // System.exceptionHandler cannot be retrieved

		// execute clo
		tTJSVariant obj;
		tTJSVariant msg(e.GetMessage());
		TJSGetExceptionObject(engine, &obj, msg);

		tTJSVariant *pval[] =  { &obj };

		tTJSVariant res;

		clo.FuncCall(0, NULL, NULL, &res, 1, pval, NULL);

		result = res.operator bool();
	}
	catch(eTJSScriptError &e)
	{
		clo.Release();
		TVPShowScriptException(e);
	}
	catch(eTJS &e)
	{
		clo.Release();
		TVPShowScriptException(e);
	}
	catch(...)
	{
		clo.Release();
		throw;
	}
	clo.Release();

	return result;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void TVPStartObjectHashMap()
{
	// addref ObjectHashMap if the program is being debugged.
	if(TJSEnableDebugMode)
		TJSAddRefObjectHashMap();
}

//---------------------------------------------------------------------------
// TVPBeforeProcessUnhandledException
//---------------------------------------------------------------------------
void TVPBeforeProcessUnhandledException()
{
#if 0
	TVPDumpHWException();
#endif
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPShowScriptException
//---------------------------------------------------------------------------
/*
	These functions display the error location, reason, etc.
	And disable the script event dispatching to avoid massive occurrence of
	errors.
*/
//---------------------------------------------------------------------------
void TVPShowScriptException(eTJS &e)
{
	TVPSetSystemEventDisabledState(true);
	TVPOnError();

	if(!TVPSystemUninitCalled)
	{
		ttstr errstr = (ttstr(TVPScriptExceptionRaised) + TJS_W("\n") + e.GetMessage());
		TVPAddLog(ttstr(TVPScriptExceptionRaised) + TJS_W("\n") + e.GetMessage());
#if defined(__EMSCRIPTEN__)
		fprintf(stderr, "[TJS-EXCEPTION] %s\n", e.GetMessage().AsNarrowStdString().c_str());
#endif
#if defined(__EMSCRIPTEN__)
		// In Emscripten, NEVER terminate on script exceptions, and never block alerts.
		// Just log the error and allow the engine loop to continue ticking.
		TVPSetSystemEventDisabledState(false);
#else
		Application->MessageDlg( errstr.AsStdString(), tjs_string(), mtError, mbOK );
		TVPTerminateSync(1);
#endif
	}
}
//---------------------------------------------------------------------------
void TVPShowScriptException(eTJSScriptError &e)
{
	TVPSetSystemEventDisabledState(true);
	TVPOnError();

	if(!TVPSystemUninitCalled)
	{
		ttstr errstr = (ttstr(TVPScriptExceptionRaised) + TJS_W("\n") + e.GetMessage());
		TVPAddLog(ttstr(TVPScriptExceptionRaised) + TJS_W("\n") + e.GetMessage());
		if(e.GetTrace().GetLen() != 0)
			TVPAddLog(ttstr(TJS_W("trace : ")) + e.GetTrace());
#if defined(__EMSCRIPTEN__)
		fprintf(stderr, "[TJS-EXCEPTION] %s\n", e.GetMessage().AsNarrowStdString().c_str());
		if(e.GetTrace().GetLen() != 0)
			fprintf(stderr, "[TJS-TRACE] %s\n", e.GetTrace().AsNarrowStdString().c_str());
#endif
#if defined(__EMSCRIPTEN__)
		// In Emscripten, NEVER terminate on script exceptions.
		// Just log the error and allow the engine loop to continue ticking.
		TVPSetSystemEventDisabledState(false);
#else
		Application->MessageDlg( errstr.AsStdString(), Application->GetTitle(), mtStop, mbOK );

#ifdef TVP_ENABLE_EXECUTE_AT_EXCEPTION
		const tjs_char* scriptName = e.GetBlockNoAddRef()->GetName();
		if( scriptName != NULL && scriptName[0] != 0 ) {
			ttstr path(scriptName);
			try {
				ttstr newpath = TVPGetPlacedPath(path);
				if( newpath.IsEmpty() ) {
					path = TVPNormalizeStorageName(path);
				} else {
					path = newpath;
				}
				TVPGetLocalName( path );
				tjs_string scriptPath( path.AsStdString() );
				tjs_int lineno = 1+e.GetBlockNoAddRef()->SrcPosToLine(e.GetPosition() )- e.GetBlockNoAddRef()->GetLineOffset();

#if defined(WIN32) && defined(_DEBUG) && !defined(ENABLE_DEBUGGER)
// デバッガ実行されている時、Visual Studio で行ジャンプする時の指定をデバッグ出力に出して、break で停止する
				if( ::IsDebuggerPresent() ) {
					tjs_string debuglile( tjs_string(TJS_W("2>"))+path.AsStdString()+TJS_W("(")+to_tjs_string(lineno)+TJS_W("): error :") + errstr.AsStdString() );
					::OutputDebugString( debuglile.c_str() );
					// ここで breakで停止した時、直前の出力行をダブルクリックすれば、例外箇所のスクリプトをVisual Studioで開ける
					::DebugBreak();
				}
#endif
				scriptPath = tjs_string(TJS_W("\"")) + scriptPath + tjs_string(TJS_W("\""));
				tTJSVariant val;
				if( TVPGetCommandLine(TJS_W("-exceptionexe"), &val) )
				{
					ttstr exepath(val);
					//exepath = ttstr(TJS_W("\"")) + exepath + ttstr(TJS_W("\""));
					if( TVPGetCommandLine(TJS_W("-exceptionarg"), &val) )
					{
						ttstr arg(val);
						if( !exepath.IsEmpty() && !arg.IsEmpty() ) {
							tjs_string str( arg.AsStdString() );
							str = ApplicationSpecialPath::ReplaceStringAll( str, tjs_string(TJS_W("%filepath%")), scriptPath );
							str = ApplicationSpecialPath::ReplaceStringAll( str, tjs_string(TJS_W("%line%")), to_tjs_string(lineno) );
							//exepath = exepath + ttstr(str);
							//_wsystem( exepath.c_str() );
							arg = ttstr(str);
							TVPAddLog( ttstr(TJS_W("(execute) "))+exepath+ttstr(TJS_W(" "))+arg);
#if defined(WIN32)
							TVPShellExecute( exepath, arg );
#endif	// Android では Intent で他のアプリに送れるようにする方がよい
						}
					}
				}
			} catch(...) {
			}
		}
#endif
		TVPTerminateSync(1);
#endif
	}
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPInitializeStartupScript
//---------------------------------------------------------------------------
void TVPInitializeStartupScript()
{
	TVPStartObjectHashMap();

	TVPExecuteStartupScript();
	if(TVPTerminateOnNoWindowStartup && TVPGetWindowCount() == 0 ) {
		// no window is created and main window is invisible
		Application->Terminate();
	}
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// tTJSNC_Scripts
//---------------------------------------------------------------------------
tjs_uint32 tTJSNC_Scripts::ClassID = -1;
tTJSNC_Scripts::tTJSNC_Scripts() : inherited(TJS_W("Scripts"))
{
	// registration of native members

	TJS_BEGIN_NATIVE_MEMBERS(Scripts)
	TJS_DECL_EMPTY_FINALIZE_METHOD
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL_NO_INSTANCE(/*TJS class name*/Scripts)
{
	return TJS_S_OK;
}
TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/Scripts)
//----------------------------------------------------------------------

//-- methods

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/execStorage)
{
	// execute script which stored in storage
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr name = *param[0];

	ttstr modestr;
	if(numparams >=2 && param[1]->Type() != tvtVoid)
		modestr = *param[1];

	iTJSDispatch2 *context = numparams >= 3 && param[2]->Type() != tvtVoid ? param[2]->AsObjectNoAddRef() : NULL;

	TVPExecuteStorage(name, context, result, false, modestr.c_str());

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/execStorage)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/evalStorage)
{
	// execute expression which stored in storage
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr name = *param[0];

	ttstr modestr;
	if(numparams >=2 && param[1]->Type() != tvtVoid)
		modestr = *param[1];

	iTJSDispatch2 *context = numparams >= 3 && param[2]->Type() != tvtVoid ? param[2]->AsObjectNoAddRef() : NULL;

	TVPExecuteStorage(name, context, result, true, modestr.c_str());

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/evalStorage)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/compileStorage) // bytecode
{
	if(numparams < 2) return TJS_E_BADPARAMCOUNT;

	ttstr name = *param[0];
	ttstr output = *param[1];

	bool isresult = false;
	if( numparams >= 3 && (tjs_int)*param[2] ) {
		isresult = true;
	}

	bool outputdebug = false;
	if( numparams >= 4 && (tjs_int)*param[3] ) {
		outputdebug = true;
	}

	bool isexpression = false;
	if( numparams >= 5 && (tjs_int)*param[4] ) {
		isexpression = true;
	}
	TVPCompileStorage( name, isresult, outputdebug, isexpression, output );

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/compileStorage)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/exec)
{
	// execute given string as a script
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr content = *param[0];

	ttstr name;
	tjs_int lineofs = 0;
	if(numparams >= 2 && param[1]->Type() != tvtVoid) name = *param[1];
	if(numparams >= 3 && param[2]->Type() != tvtVoid) lineofs = *param[2];

	iTJSDispatch2 *context = numparams >= 4 && param[3]->Type() != tvtVoid ? param[3]->AsObjectNoAddRef() : NULL;

	if(TVPScriptEngine)
		TVPScriptEngine->ExecScript(content, result, context,
			&name, lineofs);
	else
		TVPThrowInternalError;

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/exec)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/eval)
{
	// execute given string as a script
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr content = *param[0];

	ttstr name;
	tjs_int lineofs = 0;
	if(numparams >= 2 && param[1]->Type() != tvtVoid) name = *param[1];
	if(numparams >= 3 && param[2]->Type() != tvtVoid) lineofs = *param[2];

	iTJSDispatch2 *context = numparams >= 4 && param[3]->Type() != tvtVoid ? param[3]->AsObjectNoAddRef() : NULL;

	if(TVPScriptEngine)
		TVPScriptEngine->EvalExpression(content, result, context,
			&name, lineofs);
	else
		TVPThrowInternalError;

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/eval)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/dump)
{
	// execute given string as a script
	TVPDumpScriptEngine();

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/dump)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/getTraceString)
{
	// get current stack trace as string
	tjs_int limit = 0;

	if(numparams >= 1 && param[0]->Type() != tvtVoid)
		limit = *param[0];

	if(result)
	{
		*result = TJSGetStackTraceString(limit);
	}

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/getTraceString)
//----------------------------------------------------------------------
#ifdef TJS_DEBUG_DUMP_STRING
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/dumpStringHeap)
{
	// dump all strings held by TJS2 framework
	TJSDumpStringHeap();

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/dumpStringHeap)
#endif
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setCallMissing) /* UNDOCUMENTED: subject to change */
{
	// set to call "missing" method
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	iTJSDispatch2 *dsp = param[0]->AsObjectNoAddRef();

	if(dsp)
	{
		tTJSVariant missing(TJS_W("missing"));
		dsp->ClassInstanceInfo(TJS_CII_SET_MISSING, 0, &missing);
	}

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/setCallMissing) /* UNDOCUMENTED: subject to change */
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/getClassNames) /* UNDOCUMENTED: subject to change */
{
	// get class name as an array, last (most end) class first.
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	iTJSDispatch2 *dsp = param[0]->AsObjectNoAddRef();

	if(dsp)
	{
		iTJSDispatch2 * array =  TJSCreateArrayObject();
		try
		{
			tjs_uint num = 0;
			while(true)
			{
				tTJSVariant val;
				tjs_error err = dsp->ClassInstanceInfo(TJS_CII_GET, num, &val);
				if(TJS_FAILED(err)) break;
				array->PropSetByNum(TJS_MEMBERENSURE, num, &val, array);
				num ++;
			}
			if(result) *result = tTJSVariant(array, array);
		}
		catch(...)
		{
			array->Release();
			throw;
		}
		array->Release();
	}
	else
	{
		return TJS_E_FAIL;
	}

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/getClassNames) /* UNDOCUMENTED: subject to change */
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(textEncoding)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		*result = TVPScriptTextEncoding;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER
	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TVPScriptTextEncoding = *param;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL(textEncoding)
//----------------------------------------------------------------------

	TJS_END_NATIVE_MEMBERS
}
//---------------------------------------------------------------------------
tTJSNativeInstance * tTJSNC_Scripts::CreateNativeInstance()
{
	// this class cannot create an instance
	TVPThrowExceptionMessage(TVPCannotCreateInstance);

	return NULL;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// TVPCreateNativeClass_Scripts
//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_Scripts()
{
	tTJSNC_Scripts *cls = new tTJSNC_Scripts();

	// setup some platform-specific members

//----------------------------------------------------------------------

// currently none

//----------------------------------------------------------------------
	return cls;
}
//---------------------------------------------------------------------------

