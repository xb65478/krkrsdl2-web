static std::string MotionLowerString(std::string value);

static void EnsurePSBFilePlugin()
{
	TVPLoadPSBFilePlugin();
}

// Motion renderer backend selection.
//   CpuIR (default): EmotePlayer::draw() consumes the current renderMethodIR
//     staging data and writes the live Layer through the CPU triangle raster.
//     This is the path used by normal game playback while M4/M7 are being
//     validated against real packages.
//   StaticPsb: explicit diagnostic fallback that routes every frame through
//     LoadImageIntoLayer / TVPLoadPSB so the static PSB compositor renders the
//     current tick onto the live Layer.
enum MotionRendererBackend {
	MotionRendererBackendStaticPsb = 0,
	MotionRendererBackendCpuIR = 1,
};

static MotionRendererBackend g_motionRendererBackend =
	MotionRendererBackendCpuIR;

static const tjs_char *MotionRendererBackendName(MotionRendererBackend backend)
{
	switch (backend) {
		case MotionRendererBackendCpuIR: return TJS_W("cpu-ir");
		case MotionRendererBackendStaticPsb:
		default: return TJS_W("static-psb");
	}
}

#ifdef __EMSCRIPTEN__
static bool TVPIsDracuBringupEnabled()
{
	return TVPGetCommandLine(TJS_W("-krkr-dracu-bringup"));
}

static void LogDracuMotionHitHack(const char *state, const char *feature, const ttstr &storage)
{
	static int log_count = 0;
	if (log_count++ >= 40) return;
	KRKR_LOG_L2(
		"[HACK-P0] id=P0-003 state=%s feature=%s storage=%s target=DRACU flag=-krkr-dracu-bringup\n",
		state, feature ? feature : "-", storage.AsNarrowStdString().c_str());
}

static void ForceLogoFallbackUpdate()
{
	if (!TVPMainWindow) return;
	try {
		iTVPDrawDevice *dd = TVPMainWindow->GetDrawDevice();
		KRKR_LOG_L2(
			"[emoteplayer] logo fallback update: window=%p drawdevice=%p\n",
			(void*)TVPMainWindow, (void*)dd);
		if (dd) {
			tTVPRect r(0, 0, 1280, 720);
			dd->RequestInvalidation(r);
		}
		TVPMainWindow->RequestUpdate();
	} catch (...) {
		emscripten_log(EM_LOG_CONSOLE | EM_LOG_ERROR,
			"[emoteplayer] logo fallback update failed");
	}
}

static void ForceMotionLayerUpdate()
{
	if (!TVPMainWindow) return;
	try {
		iTVPDrawDevice *dd = TVPMainWindow->GetDrawDevice();
		if (dd) {
			tTVPRect r(0, 0, 1280, 720);
			dd->RequestInvalidation(r);
		}
		TVPMainWindow->RequestUpdate();
	} catch (...) {
	}
}

static bool MotionVerboseLogWanted()
{
	static int cached = -1;
	if (cached >= 0) return cached != 0;
	bool enabled = TVPGetCommandLine(TJS_W("-krkr-motion-verbose-log"));
	if (!enabled) {
		enabled = EM_ASM_INT({
			return (globalThis.__krkrMotionVerboseLog === 1 ||
			globalThis.__krkrMotionVerboseLog === '1' ||
			globalThis.__krkrMotionVerboseLog === true) ? 1 : 0;
		}) != 0;
	}
	cached = enabled ? 1 : 0;
	return enabled;
}

static bool MotionRendererDiagnosticsWanted()
{
	static int cached = -1;
	if (cached >= 0) return cached != 0;
	bool enabled = TVPGetCommandLine(TJS_W("-krkr-motion-renderer-diagnostics"));
	if (!enabled) {
		enabled = EM_ASM_INT({
			return (globalThis.__krkrMotionRendererDiagnostics === 1 ||
				globalThis.__krkrMotionRendererDiagnostics === '1' ||
				globalThis.__krkrMotionRendererDiagnostics === true) ? 1 : 0;
		}) != 0;
	}
	cached = enabled ? 1 : 0;
	return enabled;
}

static tjs_real MotionEffectiveSpeedDivisor(tjs_real value)
{
	static const tjs_real defaultSpeedRatio = 20.0;
	static const tjs_real multiplierUpperBound = 4.0;

	if (value <= 0.0001) return defaultSpeedRatio;
	if (value <= multiplierUpperBound) return defaultSpeedRatio / value;
	return value;
}

static bool MotionClickBurstTraceWanted()
{
	static int cached = -1;
	if (cached >= 0) return cached != 0;
	bool enabled = TVPGetCommandLine(TJS_W("-krkr-motion-click-burst-trace"));
	if (!enabled) {
		enabled = EM_ASM_INT({
			return (globalThis.__krkrMotionClickBurstTrace === 1 ||
				globalThis.__krkrMotionClickBurstTrace === '1' ||
				globalThis.__krkrMotionClickBurstTrace === true) ? 1 : 0;
		}) != 0;
	}
	cached = enabled ? 1 : 0;
	return enabled;
}

struct MotionClickBurstTraceState {
	tjs_int remaining = 0;
	tjs_int generation = 0;
};
static MotionClickBurstTraceState g_motionClickBurstTrace;

static void MotionArmClickBurstTrace(const char *reason, double x = -1.0, double y = -1.0)
{
	if (!MotionClickBurstTraceWanted()) return;
	g_motionClickBurstTrace.remaining = 16;
	g_motionClickBurstTrace.generation++;
	std::fprintf(stderr,
		"[MOTION-CLICK-BURST] arm gen=%d reason=%s x=%.1f y=%.1f\n",
		(int)g_motionClickBurstTrace.generation,
		reason ? reason : "-",
		x, y);
	std::fflush(stderr);
}

static bool MotionClickBurstTraceActive()
{
	return MotionClickBurstTraceWanted() && g_motionClickBurstTrace.remaining > 0;
}

static void MotionClickBurstTraceLog(const char *tag, const std::string &line)
{
	if (!MotionClickBurstTraceActive()) return;
	std::fprintf(stderr, "[MOTION-CLICK-BURST] gen=%d rem=%d %s %s\n",
		(int)g_motionClickBurstTrace.generation,
		(int)g_motionClickBurstTrace.remaining,
		tag ? tag : "-",
		line.c_str());
	std::fflush(stderr);
	g_motionClickBurstTrace.remaining--;
}

// ----------------------------------------------------------------------------
// M4 trace facility: when MOTION_TRACE_M4=1 is set in TJS commandline (or in
// browser via globalThis.__motionTraceM4=1), emit per-vertex evaluation records
// to a JSONL file at /tmp/krkrsdl2_trace.jsonl in a format directly comparable
// to krkrsdl3's `m4-sample-step` / `m4-sample-final` actions.
//
// Each emitted line is one JSON object with keys:
//   motion, tick, nodeLabel, src, uv (2-array), surfaceCount, lim (5-array),
//   surfaces (array of {matTransRowMajor, size, origin, opa, controlPts}),
//   steps (array of {surfaceIndex, lastPtBeforeRemap, surfaceUv,
//                    bezierPosition, lastPtAfter, controlSource}),
//   final {finalLastPt, glPosition, totalOpa}
//
// Only emits at uv corners (0,0), (0.5,0.5), (1,1) to limit volume and align
// with krkrsdl3's traceM4KeyTicks sampling convention.
struct MotionTraceM4State {
	bool initialized = false;
	bool enabled = false;
};
static MotionTraceM4State g_motionTraceM4;

// Re-checks the runtime flag every call so the JS side can flip tracing on
// after the engine boots without having to rebuild. Cheap enough for the
// corner-uv-only emit path (~hundreds of lines per probe).
static bool MotionTraceM4Wanted()
{
#ifdef __EMSCRIPTEN__
	int v = EM_ASM_INT({
		return (globalThis.__motionTraceM4 === 1 ||
			globalThis.__motionTraceM4 === '1' ||
			globalThis.__motionTraceM4 === true) ? 1 : 0;
	});
	return v != 0;
#else
	const char *env = std::getenv("MOTION_TRACE_M4");
	return env && env[0] && env[0] != '0';
#endif
}

static void MotionTraceM4Init()
{
	g_motionTraceM4.initialized = true;
	g_motionTraceM4.enabled = MotionTraceM4Wanted();
}

static bool MotionTraceM4UvIsCorner(tjs_real u, tjs_real v)
{
	const tjs_real eps = 1e-3;
	bool u0 = std::abs(u - 0.0) < eps;
	bool u05 = std::abs(u - 0.5) < eps;
	bool u1 = std::abs(u - 1.0) < eps;
	bool v0 = std::abs(v - 0.0) < eps;
	bool v05 = std::abs(v - 0.5) < eps;
	bool v1 = std::abs(v - 1.0) < eps;
	return (u0 && v0) || (u05 && v05) || (u1 && v1);
}

static std::string MotionTraceJsonEscape(const std::string &s)
{
	std::string out;
	out.reserve(s.size() + 2);
	for (char c : s) {
		switch (c) {
			case '"': out += "\\\""; break;
			case '\\': out += "\\\\"; break;
			case '\b': out += "\\b"; break;
			case '\f': out += "\\f"; break;
			case '\n': out += "\\n"; break;
			case '\r': out += "\\r"; break;
			case '\t': out += "\\t"; break;
			default:
				if ((unsigned char)c < 0x20) {
					char buf[8];
					std::snprintf(buf, sizeof(buf), "\\u%04x", (int)(unsigned char)c);
					out += buf;
				} else {
					out += c;
				}
		}
	}
	return out;
}

// Caller-visible setup context for the trace facility. Set by EmotePlayer::draw
// before invoking upload prep / shadow / offscreen, so the per-vertex emit can
// embed motion / tick / lim into each line.
struct MotionTraceM4SetupContext {
	std::string motion;        // "yuzulogo" / "back_white" / ...
	tjs_real tick = 0.0;
	tjs_real limOriginX = 0.0;
	tjs_real limOriginY = 0.0;
	tjs_real limWidth = 0.0;
	tjs_real limHeight = 0.0;
	tjs_real limZMax = 30.0;
	bool active = false;
};
static MotionTraceM4SetupContext g_motionTraceM4Setup;

struct MotionRenderSurface;  // forward
struct MotionMat4;           // forward

static std::string MotionFloatsToJson(const tjs_real *vals, size_t n)
{
	std::string out = "[";
	for (size_t i = 0; i < n; i++) {
		if (i) out += ',';
		char buf[40];
		std::snprintf(buf, sizeof(buf), "%.6f", (double)vals[i]);
		out += buf;
	}
	out += ']';
	return out;
}

static std::string MotionFloatsToJson(const float *vals, size_t n)
{
	std::string out = "[";
	for (size_t i = 0; i < n; i++) {
		if (i) out += ',';
		char buf[40];
		std::snprintf(buf, sizeof(buf), "%.6f", (double)vals[i]);
		out += buf;
	}
	out += ']';
	return out;
}

static bool MotionPathMatchesStaticBridgeSnapshot(
	const ttstr &path, const TVPStaticMotionBridgeSnapshot *snapshot)
{
	if (!snapshot || snapshot->storage.empty()) return false;
	std::string lhs = MotionLowerString(path.AsNarrowStdString());
	std::string rhs = MotionLowerString(snapshot->storage);
	if (lhs == rhs) return true;
	size_t lslash = lhs.find_last_of("/\\");
	size_t rslash = rhs.find_last_of("/\\");
	std::string lbase = (lslash == std::string::npos) ? lhs : lhs.substr(lslash + 1);
	std::string rbase = (rslash == std::string::npos) ? rhs : rhs.substr(rslash + 1);
	return !lbase.empty() && lbase == rbase;
}

static bool MotionIsWhiteLogoBackground(const ttstr &storage, const ttstr &motion)
{
	(void)storage;
	std::string lowerMotion = motion.AsLowerCase().AsNarrowStdString();
	return lowerMotion.find("white") != std::string::npos;
}

static void HidePsbLogoFallback()
{
	try {
		TVPExecuteExpression(TJS_W(
			"(function(){"
			"  try {"
			"    var l = typeof global.__psbInterceptLayer !== 'undefined' ? global.__psbInterceptLayer : void;"
			"    if (l !== void) { l.visible = false; l.update(0, 0, l.width, l.height); }"
			"  } catch(e) {}"
			"})()"), static_cast<tTJSVariant*>(nullptr));
		ForceLogoFallbackUpdate();
	} catch (...) {
	}
}

static bool ShowPsbLogoFallback(const ttstr &storage, const ttstr &motion)
{
	EnsurePSBFilePlugin();
	bool whiteBackground = MotionIsWhiteLogoBackground(storage, motion);
	bool scriptOK = false;
	try {
		tTJSVariant scriptResult;
		ttstr script = TJS_W(
			"(function(){"
			"  try {"
			"    var w = Window.mainWindow;"
			"    var base = w.primaryLayer;"
			"    if (base === void) return 'no primaryLayer';"
			"    var tmp = new Layer(w, base);"
			"    tmp.loadImages('");
		script += storage;
		script += TJS_W(
			"');"
			"    tmp.setSizeToImageSize();"
			"    var iw = tmp.imageWidth, ih = tmp.imageHeight;"
			"    var bw = base.width, bh = base.height;"
			"    var dx = int((bw - iw) / 2);"
			"    var dy = int((bh - ih) / 2);"
			"    var old = typeof global.__psbInterceptLayer !== 'undefined' ? global.__psbInterceptLayer : void;"
			"    if (old !== void) { try { old.visible = false; old.update(0, 0, old.width, old.height); } catch(e) {} }"
			"    var logo = new Layer(w, base);"
			"    global.__psbInterceptLayer = logo;"
			"    logo.setPos(0, 0);"
			"    logo.setSize(bw, bh);"
			"    logo.type = 1;"
			"    logo.face = 1;"
			"    logo.opacity = 255;"
			"    logo.visible = true;"
			"    logo.fillRect(0, 0, bw, bh, ");
		script += whiteBackground ? TJS_W("0xffffffff") : TJS_W("0xff000000");
		script += TJS_W(
			");"
			"    logo.operateRect(dx, dy, tmp, 0, 0, iw, ih);"
			"    tmp.visible = false;"
			"    logo.bringToFront();"
			"    logo.update(0, 0, bw, bh);"
			"    var cpx = logo.getMainPixel(dx + (iw >> 1), dy + (ih >> 1));"
			"    return 'OK ' + iw + 'x' + ih + ' at(' + dx + ',' + dy + ') base=' + bw + 'x' + bh + ' centerPx=' + cpx;"
			"  } catch(e) { return 'ERR: ' + e.message; }"
			"})()");
		TVPExecuteExpression(script, &scriptResult);
		if (scriptResult.Type() == tvtString) {
			ttstr s = ttstr(scriptResult.GetString());
			std::string sstr = s.AsNarrowStdString();
			KRKR_LOG_L2("[emoteplayer] logo fallback result: %s\n", sstr.c_str());
			scriptOK = sstr.compare(0, 3, "OK ") == 0;
		} else {
			KRKR_LOG_L2(
				"[emoteplayer] logo fallback result: <non-string type=%d>\n",
				(int)scriptResult.Type());
		}
	} catch(eTJS &e) {
		emscripten_log(EM_LOG_CONSOLE | EM_LOG_ERROR,
			"[emoteplayer] logo fallback TJS exception: %s",
			e.GetMessage().AsNarrowStdString().c_str());
	} catch(...) {
		emscripten_log(EM_LOG_CONSOLE | EM_LOG_ERROR,
			"[emoteplayer] logo fallback script exec failed");
	}

	if (scriptOK) ForceLogoFallbackUpdate();
	return scriptOK;
}
#endif
