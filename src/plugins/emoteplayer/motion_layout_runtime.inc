struct MotionNodeLayoutEntry {
	unsigned char *data = nullptr;
	int parent = -1;
	std::string label;
	int type = -1;
	tjs_int parameterIdx = -1;
};

struct MotionLayoutTransform {
	tjs_real x = 0.0;
	tjs_real y = 0.0;
	tjs_real scaleX = 1.0;
	tjs_real scaleY = 1.0;
	tjs_real opacity = 1.0;
	tjs_real originX = 0.0;
	tjs_real originY = 0.0;
	tjs_real width = 0.0;
	tjs_real height = 0.0;
	tjs_real angle = 0.0;
	tjs_real sx = 0.0;
	tjs_real sy = 0.0;
	tjs_real ox = 0.0;
	tjs_real oy = 0.0;
	tjs_real iconOriginX = 0.0;
	tjs_real iconOriginY = 0.0;
	tjs_real iconWidth = 0.0;
	tjs_real iconHeight = 0.0;
	MotionAffine2D affine;
};

struct MotionSurfaceStackState {
	bool valid = false;
	bool unsupportedDemux = false;
	tjs_real originX = 0.0;
	tjs_real originY = 0.0;
	tjs_real width = 0.0;
	tjs_real height = 0.0;
	tjs_real currCoordz = 0.0;
	MotionMat4 layoutChain;
	MotionRenderSurface lastSurface;
	std::vector<MotionRenderSurface> surfaces;
};

struct MotionLayoutBounds {
	bool valid = false;
	std::string sourceKey;
	bool opacityValid = false;
	tjs_real minX = 0.0;
	tjs_real minY = 0.0;
	tjs_real maxX = 0.0;
	tjs_real maxY = 0.0;
	tjs_real opacityMin = 1.0;
	tjs_real opacityMax = 1.0;
	tjs_int drawCount = 0;
	bool hasAffine = false;
	tjs_real angle = 0.0;
	tjs_real sx = 0.0;
	tjs_real sy = 0.0;
	tjs_real ox = 0.0;
	tjs_real oy = 0.0;
	tjs_real iconOriginX = 0.0;
	tjs_real iconOriginY = 0.0;
	tjs_real iconWidth = 0.0;
	tjs_real iconHeight = 0.0;
	tjs_real accumX = 0.0;
	tjs_real accumY = 0.0;
	tjs_real accumScaleX = 1.0;
	tjs_real accumScaleY = 1.0;
	MotionAffine2D affine;
	bool hasControlPoints = false;
	tjs_real controlPts[32] = {
		0.0, 0.0, 0.333333, 0.0, 0.666667, 0.0, 1.0, 0.0,
		0.0, 0.333333, 0.333333, 0.333333, 0.666667, 0.333333, 1.0, 0.333333,
		0.0, 0.666667, 0.333333, 0.666667, 0.666667, 0.666667, 1.0, 0.666667,
		0.0, 1.0, 0.333333, 1.0, 0.666667, 1.0, 1.0, 1.0
	};
};

#ifdef __EMSCRIPTEN__
struct MotionLayoutBuildPerfCounters {
	tjs_uint64 sourceReadMs = 0;
	tjs_uint64 sourceCanonMs = 0;
	tjs_uint64 originMs = 0;
	tjs_uint64 loopMs = 0;
	tjs_uint64 primaryCollectMs = 0;
	tjs_uint64 probeCopyMs = 0;
	tjs_uint64 probeCollectMs = 0;
	tjs_uint64 probeDiagMergeMs = 0;
	tjs_uint64 finalIrMs = 0;
	tjs_uint64 selfTestMs = 0;
	tjs_uint64 namedMergeTotalMs = 0;
	tjs_uint64 namedMergeSearchMs = 0;
	tjs_uint64 namedMergeSemanticsMs = 0;
	tjs_uint64 namedMergePushMs = 0;
	tjs_uint64 aliasTotalMs = 0;
	tjs_uint64 aliasBuildMs = 0;
	tjs_uint64 renderItemMs = 0;
	tjs_uint64 collectNamedMs = 0;
	tjs_uint64 collectMotionMs = 0;
	tjs_uint64 collectReadLayersMs = 0;
	tjs_uint64 collectFindMotionMs = 0;
	tjs_uint64 collectNodesMs = 0;
	tjs_uint64 collectSetupMs = 0;
	tjs_uint64 collectFirstPassMs = 0;
	tjs_uint64 collectVariantPassMs = 0;
	size_t primaryCalls = 0;
	size_t probeCalls = 0;
	size_t probeCopiedBytes = 0;
	size_t probeCopiedLayouts = 0;
	size_t probeIndexEntries = 0;
	size_t probeIndexHits = 0;
	size_t probeIndexMisses = 0;
	size_t probeIndexFallbacks = 0;
	size_t probeIndexMismatches = 0;
	size_t namedMergeCalls = 0;
	size_t namedMergeUpdates = 0;
	size_t namedMergeCreates = 0;
	size_t namedMergeScanSteps = 0;
	size_t aliasCalls = 0;
	size_t aliasNames = 0;
	size_t renderItemCalls = 0;
	size_t collectNamedCalls = 0;
	size_t collectMotionCalls = 0;
	size_t collectLayerCount = 0;
	size_t collectNodeCount = 0;
	size_t collectFrameVariantCount = 0;
	tjs_uint64 maxPrimaryMs = 0;
	tjs_uint64 maxProbeCopyMs = 0;
	tjs_uint64 maxProbeCollectMs = 0;
	tjs_uint64 maxNamedMergeMs = 0;
	tjs_uint64 maxAliasMs = 0;
	tjs_uint64 maxCollectNamedMs = 0;
	tjs_uint64 maxCollectMotionMs = 0;
	std::string maxPrimaryLabel;
	std::string maxProbeCopyLabel;
	std::string maxProbeCollectLabel;
	std::string maxNamedMergeLabel;
	std::string maxAliasLabel;
	std::string maxCollectNamedLabel;
	std::string maxCollectMotionLabel;
};

static MotionLayoutBuildPerfCounters *gMotionLayoutBuildPerf = nullptr;

static void MotionLayoutPerfNoteMax(tjs_uint64 ms, const std::string &label,
	tjs_uint64 &maxMs, std::string &maxLabel)
{
	if (ms > maxMs) {
		maxMs = ms;
		maxLabel = label;
	}
}
#endif

static MotionLayoutTransform MotionApplyLayoutFrame(const MotionLayoutTransform &base,
	const MotionFrameLayoutState &frame);

// Emit one JSONL line per vertex evaluated, for diff against krkrsdl3 trace.
// Only emits when the trace facility is initialized + enabled and the uv is
// at a corner (0,0)/(0.5,0.5)/(1,1). Caller should have populated
// `g_motionTraceM4Setup` with motion/tick/lim before calling.
static void MotionTraceM4EmitVertex(
	const std::vector<MotionRenderSurface> &surfaces,
	tjs_real u, tjs_real v,
	tjs_real vx, tjs_real vy,
	tjs_real ndcX, tjs_real ndcY,
	const ttstr &nodeLabel,
	const std::string &srcKey,
	tjs_real opacity)
{
	if (!MotionTraceM4UvIsCorner(u, v)) return;
	if (!g_motionTraceM4Setup.active) return;
	if (surfaces.empty()) return;
	if (!MotionTraceM4Wanted()) return;

	std::string line = "{";
	auto add_string_field = [&](const char *key, const std::string &val) {
		line += '"';
		line += key;
		line += "\":\"";
		line += MotionTraceJsonEscape(val);
		line += '"';
	};
	auto add_real_field = [&](const char *key, tjs_real val) {
		char buf[40];
		std::snprintf(buf, sizeof(buf), "%.6f", (double)val);
		line += '"';
		line += key;
		line += "\":";
		line += buf;
	};
	auto add_raw_field = [&](const char *key, const std::string &raw) {
		line += '"';
		line += key;
		line += "\":";
		line += raw;
	};

	add_string_field("motion", g_motionTraceM4Setup.motion);
	line += ',';
	add_real_field("tick", g_motionTraceM4Setup.tick);
	line += ',';
	add_string_field("nodeLabel", nodeLabel.AsNarrowStdString());
	line += ',';
	add_string_field("src", srcKey);
	line += ",\"uv\":[";
	{
		char buf[80];
		std::snprintf(buf, sizeof(buf), "%.6f,%.6f", (double)u, (double)v);
		line += buf;
	}
	line += "],\"surfaceCount\":";
	{
		char buf[16];
		std::snprintf(buf, sizeof(buf), "%d", (int)surfaces.size());
		line += buf;
	}
	line += ",\"lim\":[";
	{
		char buf[120];
		std::snprintf(buf, sizeof(buf), "%.6f,%.6f,%.6f,%.6f,%.6f",
			(double)g_motionTraceM4Setup.limOriginX,
			(double)g_motionTraceM4Setup.limOriginY,
			(double)g_motionTraceM4Setup.limWidth,
			(double)g_motionTraceM4Setup.limHeight,
			(double)g_motionTraceM4Setup.limZMax);
		line += buf;
	}
	line += "],\"surfaces\":[";
	// Emit in REVERSE storage order so the JSON's surfaces[0] is the
	// outermost / first-applied surface (matches krkrsdl3 trace convention).
	// Our internal storage is [leaf, outer]; eval iterates rbegin->rend so
	// outer is applied first. Reversing here aligns indices for diff tools.
	for (size_t k = 0; k < surfaces.size(); k++) {
		if (k) line += ',';
		size_t i = surfaces.size() - 1 - k;
		const MotionRenderSurface &s = surfaces[i];
		line += "{";
		add_raw_field("matTransRowMajor",
			MotionFloatsToJson(s.matTrans.m, 16));
		line += ",\"size\":[";
		char buf[80];
		std::snprintf(buf, sizeof(buf), "%.6f,%.6f",
			(double)s.originW, (double)s.originH);
		line += buf;
		line += "],\"origin\":[";
		std::snprintf(buf, sizeof(buf), "%.6f,%.6f",
			(double)s.originX, (double)s.originY);
		line += buf;
		line += "],";
		add_real_field("opa", s.opa);
		line += ',';
		std::snprintf(buf, sizeof(buf), "%d", (int)s.type);
		line += "\"renderType\":";
		line += buf;
		line += ',';
		add_raw_field("controlPts",
			MotionFloatsToJson(s.controlPts, 32));
		line += '}';
	}
	line += "],\"final\":{\"finalLastPt\":[";
	{
		// ndcX = lastX (after divide), ndcY = -lastY (matches krkrsdl3 emit
		// which logs finalLastPt with un-flipped Y, then glPosition with
		// Y flipped).
		char buf[120];
		std::snprintf(buf, sizeof(buf), "%.6f,%.6f,0.000000,1.000000",
			(double)ndcX, (double)(-ndcY));
		line += buf;
	}
	line += "],\"glPosition\":[";
	{
		char buf[120];
		std::snprintf(buf, sizeof(buf), "%.6f,%.6f,0.000000,1.000000",
			(double)ndcX, (double)ndcY);
		line += buf;
	}
	line += "],";
	add_real_field("totalOpa", opacity);
	line += ',';
	add_real_field("pixelX", vx);
	line += ',';
	add_real_field("pixelY", vy);
	line += "}";
	line += "}";

#ifdef __EMSCRIPTEN__
	// Emit each line as a console message with a unique prefix. Puppeteer
	// console listener filters by this prefix.
	KRKR_LOG_L3("[M4-TRACE] %s\n", line.c_str());
#else
	std::fprintf(stderr, "[M4-TRACE] %s\n", line.c_str());
#endif
}

static void MotionAddLayoutBounds(MotionLayoutBounds &bounds,
	tjs_real left, tjs_real top, tjs_real width, tjs_real height,
	tjs_real opacity = 1.0)
{
	if (width == 0.0 || height == 0.0) return;
	tjs_real right = left + width;
	tjs_real bottom = top + height;
	if (right < left) std::swap(left, right);
	if (bottom < top) std::swap(top, bottom);
	if (!bounds.valid) {
		bounds.valid = true;
		bounds.minX = left;
		bounds.minY = top;
		bounds.maxX = right;
		bounds.maxY = bottom;
	} else {
		bounds.minX = std::min(bounds.minX, left);
		bounds.minY = std::min(bounds.minY, top);
		bounds.maxX = std::max(bounds.maxX, right);
		bounds.maxY = std::max(bounds.maxY, bottom);
	}
	opacity = MotionClamp01(opacity);
	if (!bounds.opacityValid) {
		bounds.opacityValid = true;
		bounds.opacityMin = opacity;
		bounds.opacityMax = opacity;
	} else {
		bounds.opacityMin = std::min(bounds.opacityMin, opacity);
		bounds.opacityMax = std::max(bounds.opacityMax, opacity);
	}
	bounds.drawCount++;
}

static void MotionMergeLayoutBounds(MotionLayoutBounds &target, const MotionLayoutBounds &source)
{
	if (!source.valid) return;
	if (target.sourceKey.empty() && !source.sourceKey.empty()) {
		target.sourceKey = source.sourceKey;
	}
	MotionAddLayoutBounds(target, source.minX, source.minY,
		source.maxX - source.minX, source.maxY - source.minY,
		source.opacityValid ? source.opacityMax : 1.0);
	if (source.opacityValid) {
		if (!target.opacityValid) {
			target.opacityValid = true;
			target.opacityMin = source.opacityMin;
			target.opacityMax = source.opacityMax;
		} else {
			target.opacityMin = std::min(target.opacityMin, source.opacityMin);
			target.opacityMax = std::max(target.opacityMax, source.opacityMax);
		}
	}
	target.drawCount += std::max<tjs_int>(0, source.drawCount - 1);
}

static void MotionNormalizeNaNInfCoord(tjs_real &coordX, tjs_real &coordY,
	tjs_real limOriginX, tjs_real limOriginY, tjs_real limWidth, tjs_real limHeight,
	bool isInterpolationNext, int &nanCount, int &infCount)
{
	(void)isInterpolationNext;
	if (std::isnan(coordX)) {
		coordX = -limOriginX;
		nanCount++;
	}
	if (std::isnan(coordY)) {
		coordY = -limOriginY;
		nanCount++;
	}
	if (std::isinf(coordX)) {
		coordX = limWidth - limOriginX;
		infCount++;
	}
	if (std::isinf(coordY)) {
		coordY = limHeight - limOriginY;
		infCount++;
	}
}

static bool MotionReadFrameLayoutContent(psb_t &psb, const psb_objects_t *content,
	MotionFrameLayoutState &state)
{
	if (!content) return false;
	state = MotionFrameLayoutState();
	state.hasContent = true;
	std::string rawSrc;
	if (MotionPsbReadMemberString(psb, content, "src", rawSrc)) {
		if (rawSrc == "blank") {
			state.src.clear();
		} else {
			std::string icon;
			if (MotionPsbReadMemberString(psb, content, "icon", icon)) {
				bool isMotion = false;
				psb_objects_t *motion = nullptr;
				if (MotionUnpackObject(psb, content->get_data("motion"), motion)) {
					isMotion = true;
					MotionPsbReadMemberNumber(psb, motion, "timeOffset", state.timeOffset);
					delete motion;
				}
				state.src = std::string(isMotion ? "motion/" : "src/") +
					rawSrc + "/" + icon;
				state.src.erase(std::remove(state.src.begin(), state.src.end(), '\0'),
					state.src.end());
			} else {
				state.src = rawSrc;
			}
		}
	} else {
		state.src = "layout";
	}
	std::vector<double> coord;
	if (MotionPsbReadNumberList(psb, content->get_data("coord"), coord) && coord.size() == 3) {
		state.hasCoord = true;
		state.coordX = coord[0];
		state.coordY = coord[1];
		state.coordZ = coord[2];
	}
	MotionPsbReadMemberNumber(psb, content, "ox", state.ox);
	MotionPsbReadMemberNumber(psb, content, "oy", state.oy);
	MotionPsbReadMemberNumber(psb, content, "zx", state.scaleX);
	MotionPsbReadMemberNumber(psb, content, "zy", state.scaleY);
	MotionPsbReadMemberNumber(psb, content, "angle", state.angle);
	MotionPsbReadMemberNumber(psb, content, "sx", state.sx);
	MotionPsbReadMemberNumber(psb, content, "sy", state.sy);
	tjs_real opacity = state.opacity;
	if (MotionPsbReadMemberNumber(psb, content, "opa", opacity)) {
		state.opacity = opacity > 1.0 ? opacity / 255.0 : opacity;
	}
	psb_objects_t *mesh = nullptr;
	if (MotionUnpackObject(psb, content->get_data("mesh"), mesh)) {
		std::vector<double> bpVals;
		if (MotionPsbReadNumberList(psb, mesh->get_data("bp"), bpVals) && bpVals.size() == 32) {
			state.hasMeshBP = true;
			for (size_t k = 0; k < 32; k++) state.bp[k] = (tjs_real)bpVals[k];
		}
		delete mesh;
	}
	return true;
}

static bool MotionReadFrameLayoutState(psb_t &psb, unsigned char *frameData,
	MotionFrameLayoutState &state, tjs_real &time, tjs_int *outFrameType = nullptr)
{
	state = MotionFrameLayoutState();
	time = 0.0;
	psb_objects_t *frame = nullptr;
	if (!MotionUnpackObject(psb, frameData, frame)) return false;
	MotionPsbReadMemberNumber(psb, frame, "time", time);
	if (outFrameType) {
		int frameType = 0;
		if (MotionPsbReadMemberInt(psb, frame, "type", frameType)) *outFrameType = frameType;
	}
	psb_objects_t *content = nullptr;
	if (!MotionUnpackObject(psb, frame->get_data("content"), content)) {
		delete frame;
		return false;
	}
	bool ok = MotionReadFrameLayoutContent(psb, content, state);
	state.frameTime = time;
	state.frameType = outFrameType ? *outFrameType : 0;
	delete content;
	delete frame;
	return ok;
}

static bool MotionSameLayoutFrame(const MotionFrameLayoutState &a,
	const MotionFrameLayoutState &b)
{
	bool same = a.hasContent == b.hasContent &&
		a.hasCoord == b.hasCoord &&
		a.src == b.src &&
		a.coordX == b.coordX &&
		a.coordY == b.coordY &&
		a.coordZ == b.coordZ &&
		a.ox == b.ox &&
		a.oy == b.oy &&
		a.scaleX == b.scaleX &&
		a.scaleY == b.scaleY &&
		a.opacity == b.opacity &&
		a.timeOffset == b.timeOffset &&
		a.angle == b.angle &&
		a.sx == b.sx &&
		a.sy == b.sy;
	if (!same) return false;
	if (a.hasMeshBP != b.hasMeshBP) return false;
	if (a.hasMeshBP) {
		for (size_t i = 0; i < 32; i++) {
			if (a.bp[i] != b.bp[i]) return false;
		}
	}
	return true;
}

static void MotionAppendUniqueLayoutFrame(std::vector<MotionFrameLayoutState> &items,
	const MotionFrameLayoutState &candidate)
{
	if (!candidate.hasContent) return;
	for (size_t i = 0; i < items.size(); i++) {
		if (MotionSameLayoutFrame(items[i], candidate)) return;
	}
	items.push_back(candidate);
}

static bool MotionReadNodeLayoutFrames(psb_t &psb, const psb_objects_t *node,
	std::vector<MotionFrameLayoutState> &states)
{
	states.clear();
	if (!node) return false;
	std::vector<unsigned char*> frames;
	if (!MotionPsbReadCollection(psb, node->get_data("frameList"), frames)) return false;
	for (size_t i = 0; i < frames.size(); i++) {
		MotionFrameLayoutState candidate;
		tjs_real time = 0.0;
		if (!MotionReadFrameLayoutState(psb, frames[i], candidate, time)) continue;
		MotionAppendUniqueLayoutFrame(states, candidate);
	}
	return !states.empty();
}

static bool MotionReadMotionPriority(psb_t &psb, const psb_objects_t *motion,
	std::vector<tjs_int> &priority)
{
	priority.clear();
	if (!motion) return false;
	std::vector<unsigned char*> frames;
	if (!MotionPsbReadCollection(psb, motion->get_data("priority"), frames)) return false;
	for (size_t i = 0; i < frames.size(); i++) {
		psb_objects_t *frame = nullptr;
		if (!MotionUnpackObject(psb, frames[i], frame)) continue;
		std::vector<unsigned char*> entries;
		bool found = false;
		if (MotionPsbReadCollection(psb, frame->get_data("content"), entries) &&
			!entries.empty()) {
			for (size_t k = 0; k < entries.size(); k++) {
				int idx = 0;
				if (MotionPsbReadInt(psb, entries[k], idx)) {
					priority.push_back((tjs_int)idx);
					found = true;
				}
			}
		}
		delete frame;
		if (found) break;
	}
	if (priority.empty()) return false;
	std::reverse(priority.begin(), priority.end());
	return true;
}

static std::vector<tjs_int> MotionBuildDrawPriorityPath(
	const std::vector<tjs_int> *prefix, tjs_int localOrder)
{
	std::vector<tjs_int> path;
	if (prefix) path = *prefix;
	path.push_back(localOrder);
	return path;
}

static bool MotionReadFirstFrameTimeAndTypes(psb_t &psb, const psb_objects_t *node,
	tjs_real &firstTime, tjs_int &firstType, tjs_int &secondType)
{
	firstTime = 0.0;
	firstType = -1;
	secondType = -1;
	if (!node) return false;
	std::vector<unsigned char*> rawFrames;
	if (!MotionPsbReadCollection(psb, node->get_data("frameList"), rawFrames) ||
		rawFrames.size() != 2) {
		return false;
	}
	for (size_t fi = 0; fi < rawFrames.size(); fi++) {
		tjs_real ftime = 0.0;
		tjs_int ftype = 0;
		MotionFrameLayoutState dummyState;
		if (!MotionReadFrameLayoutState(psb, rawFrames[fi], dummyState, ftime, &ftype)) {
			return false;
		}
		if (fi == 0) {
			firstTime = ftime;
			firstType = ftype;
		} else {
			secondType = ftype;
		}
	}
	return true;
}

static tjs_real MotionNormalizeAngleDeg(tjs_real angle)
{
	while (angle < 0.0) angle += 360.0;
	while (angle >= 360.0) angle -= 360.0;
	return angle;
}

static tjs_real MotionInterpolateAngleDeg(tjs_real a, tjs_real b, tjs_real ratio)
{
	if (b < 180.0 && a > 180.0) {
		return MotionNormalizeAngleDeg(a - 360.0 + (b + 360.0 - a) * ratio);
	}
	if (b > 180.0 && a < 180.0) {
		return MotionNormalizeAngleDeg(a + (b - 360.0 - a) * ratio);
	}
	return MotionNormalizeAngleDeg(a + (b - a) * ratio);
}

static std::string MotionVariableKey(const ttstr &name)
{
	std::string key;
	if (name.GetLen() > 0 && TVPUtf16ToUtf8(key, name.AsStdString())) return key;
	return name.AsNarrowStdString();
}

static void MotionAppendLayoutMergeKeyPart(std::string &key, const ttstr &value)
{
	std::string part = MotionVariableKey(value);
	key += std::to_string(part.size());
	key += ':';
	key += part;
	key += '|';
}

static std::string MotionMakeLayoutMergeKey(const ttstr &layoutName,
	const ttstr &objectName, const ttstr &motionName,
	const ttstr &contextObjectName, const ttstr &contextMotionName)
{
	std::string key;
	key.reserve(
		layoutName.GetLen() + objectName.GetLen() + motionName.GetLen() +
		contextObjectName.GetLen() + contextMotionName.GetLen() + 32);
	MotionAppendLayoutMergeKeyPart(key, layoutName);
	MotionAppendLayoutMergeKeyPart(key, objectName);
	MotionAppendLayoutMergeKeyPart(key, motionName);
	MotionAppendLayoutMergeKeyPart(key, contextObjectName);
	MotionAppendLayoutMergeKeyPart(key, contextMotionName);
	return key;
}

struct MotionLayoutMergeIndexState {
	MotionResource *resource = nullptr;
	std::unordered_map<std::string, tjs_int> entries;
	size_t writableHits = 0;
	size_t readOnlyHits = 0;
	size_t writableMisses = 0;
	size_t fallbackScans = 0;
	size_t mismatches = 0;
};

static MotionLayoutMergeIndexState *gMotionLayoutMergeIndex = nullptr;

struct MotionLayoutMergeIndexScope {
	explicit MotionLayoutMergeIndexScope(MotionLayoutMergeIndexState *state)
		: previous(gMotionLayoutMergeIndex)
	{
		gMotionLayoutMergeIndex = state;
	}

	~MotionLayoutMergeIndexScope()
	{
		gMotionLayoutMergeIndex = previous;
	}

	MotionLayoutMergeIndexState *previous;
};

static void MotionAppendUniqueVariableCandidate(std::vector<std::string> &candidates,
	const std::string &key)
{
	if (key.empty()) return;
	for (size_t i = 0; i < candidates.size(); i++) {
		if (candidates[i] == key) return;
	}
	candidates.push_back(key);
}

static bool MotionSelectorSegmentLooksInteractive(const std::string &segment);

static void MotionBuildParameterVariableCandidates(std::vector<std::string> &candidates,
	const std::string &localKey,
	const std::string &objectName,
	const std::string &motionName,
	const std::string &layoutScope,
	bool preferLocal)
{
	if (preferLocal) MotionAppendUniqueVariableCandidate(candidates, localKey);
	if (!layoutScope.empty()) {
		MotionAppendUniqueVariableCandidate(candidates, layoutScope + "/" + localKey);
		MotionAppendUniqueVariableCandidate(candidates, layoutScope + "/" + motionName);
		std::vector<std::string> parts = MotionSplitPath(layoutScope);
		for (std::vector<std::string>::const_reverse_iterator it = parts.rbegin();
			it != parts.rend(); ++it) {
			if (!MotionSelectorSegmentLooksInteractive(*it)) continue;
			MotionAppendUniqueVariableCandidate(candidates, *it + "/" + localKey);
			MotionAppendUniqueVariableCandidate(candidates, *it + "/" + motionName);
			MotionAppendUniqueVariableCandidate(candidates, *it);
			break;
		}
	}
	MotionAppendUniqueVariableCandidate(candidates, objectName + "/" + localKey);
	MotionAppendUniqueVariableCandidate(candidates, objectName + "/" + motionName);
	MotionAppendUniqueVariableCandidate(candidates, objectName);
	if (!preferLocal) MotionAppendUniqueVariableCandidate(candidates, localKey);
}

static bool MotionResolveParameterTick(const MotionInfo *motionInfo,
	tjs_int parameterIdx, const std::map<std::string, tjs_real> *variables,
	tjs_real &tick, const std::string *layoutScope = nullptr)
{
	if (!motionInfo || !variables) return false;
	if (parameterIdx < 0 || parameterIdx >= (tjs_int)motionInfo->variables.size()) {
		return false;
	}
	const MotionVariableInfo &parameter = motionInfo->variables[(size_t)parameterIdx];
	if (parameter.name.GetLen() <= 0 || parameter.division <= 0.0) return false;
	tjs_real value = parameter.minValue;
	std::string localKey = MotionVariableKey(parameter.name);
	std::string objectName = MotionVariableKey(motionInfo->objectName);
	std::string motionName = MotionVariableKey(motionInfo->name);
	std::vector<std::string> candidates;
	MotionBuildParameterVariableCandidates(candidates, localKey, objectName,
		motionName, layoutScope ? *layoutScope : std::string(), true);
	std::string sourceKey;
	for (size_t i = 0; i < candidates.size(); i++) {
		std::map<std::string, tjs_real>::const_iterator it =
			variables->find(candidates[i]);
		if (it == variables->end()) continue;
		if (!std::isfinite((double)it->second)) continue;
		value = it->second;
		sourceKey = candidates[i];
		break;
	}
	tjs_real span = parameter.maxValue - parameter.minValue;
	if (std::abs((double)span) <= 0.000001) return false;
	value = std::max<tjs_real>(std::min(parameter.minValue, parameter.maxValue),
		std::min<tjs_real>(std::max(parameter.minValue, parameter.maxValue), value));
	tick = parameter.division * ((value - parameter.minValue) / span);
#ifdef __EMSCRIPTEN__
	if (MotionClickBurstTraceActive() &&
		(MotionTraceInterestingSettingObject(objectName) ||
		 (layoutScope && MotionTraceInterestingSettingObject(*layoutScope)))) {
		char buf[512];
		std::snprintf(buf, sizeof(buf),
			"parameter-var object=%s motion=%s scope=%s local=%s source=%s value=%.3f tick=%.3f",
			objectName.c_str(), motionName.c_str(),
			layoutScope ? layoutScope->c_str() : "",
			localKey.c_str(), sourceKey.empty() ? "(default)" : sourceKey.c_str(),
			(double)value, (double)tick);
		MotionClickBurstTraceLog("layout", buf);
	}
#endif
	return true;
}

static std::map<std::string, tjs_real> MotionBuildNestedVariableValues(
	const std::map<std::string, tjs_real> *variables,
	const std::string &objectName,
	const std::string &motionName,
	const MotionInfo *motionInfo,
	const std::string &layoutScope = std::string())
{
	std::map<std::string, tjs_real> nested;
	if (!variables) return nested;
	nested = *variables;
	if (!motionInfo) return nested;
	for (size_t i = 0; i < motionInfo->variables.size(); i++) {
		const MotionVariableInfo &var = motionInfo->variables[i];
		std::string localKey = MotionVariableKey(var.name);
		if (localKey.empty()) continue;

		nested.erase(localKey);
		std::vector<std::string> candidates;
		MotionBuildParameterVariableCandidates(candidates, localKey, objectName,
			motionName, layoutScope, false);
		for (size_t c = 0; c < candidates.size(); c++) {
			std::map<std::string, tjs_real>::const_iterator it = variables->find(candidates[c]);
			if (it == variables->end()) continue;
			nested[localKey] = it->second;
#ifdef __EMSCRIPTEN__
			if (MotionClickBurstTraceActive() &&
				MotionTraceInterestingSettingObject(objectName)) {
				char buf[512];
				std::snprintf(buf, sizeof(buf),
					"nested-var object=%s motion=%s scope=%s local=%s source=%s value=%.3f",
					objectName.c_str(), motionName.c_str(), layoutScope.c_str(),
					localKey.c_str(), candidates[c].c_str(), (double)it->second);
				MotionClickBurstTraceLog("layout", buf);
			}
#endif
			break;
		}
	}
	return nested;
}

static MotionInfo *MotionFindMotionInfoForObjectMotion(MotionResource &resource,
	const ttstr &objectName, const std::string &motionName)
{
	for (size_t m = 0; m < resource.motions.size(); m++) {
		if (MotionNameMatches(resource.motions[m].objectName, objectName) &&
			MotionNameMatches(resource.motions[m].name, MotionString(motionName))) {
			return &resource.motions[m];
		}
	}
	return nullptr;
}

static bool MotionSelectorSegmentLooksInteractive(const std::string &segment)
{
	std::string lower = MotionLowerString(segment);
	return MotionStartsWith(lower, "bt") ||
		MotionStartsWith(lower, "tg") ||
		MotionStartsWith(lower, "sld") ||
		MotionStartsWith(lower, "slot") ||
		MotionStartsWith(lower, "page");
}

static bool MotionSelectorLeafConsumesInteractiveState(const std::string &segment)
{
	std::string lower = MotionLowerString(segment);
	return lower == "bt" ||
		lower == "state" ||
		lower == "select" ||
		lower == "value";
}

static void MotionAddSelectorCandidate(std::vector<std::string> &candidates,
	const std::map<std::string, tjs_real> *variables,
	const std::string &key)
{
	if (!variables || key.empty()) return;
	if (variables->find(key) == variables->end()) return;
	for (size_t i = 0; i < candidates.size(); i++) {
		if (candidates[i] == key) return;
	}
	candidates.push_back(key);
}

static void MotionAddSelectorSourceCandidate(std::vector<std::string> &candidates,
	const std::map<std::string, tjs_real> *variables,
	const std::string &src)
{
	if (!MotionStartsWith(src, "src/")) return;
	std::vector<std::string> parts = MotionSplitPath(src);
	if (parts.size() >= 2) {
		MotionAddSelectorCandidate(candidates, variables, parts[1]);
	}
}

static bool MotionResolveInferredSelectorValue(
	const std::vector<std::string> &candidates,
	const std::map<std::string, tjs_real> *variables,
	tjs_real &tick,
	std::string *selectedKey = nullptr)
{
	if (!variables) return false;
	for (size_t i = 0; i < candidates.size(); i++) {
		std::map<std::string, tjs_real>::const_iterator it =
			variables->find(candidates[i]);
		if (it == variables->end()) continue;
		if (!std::isfinite((double)it->second)) continue;
		tick = it->second;
		if (selectedKey) *selectedKey = candidates[i];
		return true;
	}
	return false;
}

static bool MotionResolveSourceSelectorFrameValue(
	const std::string &src, const std::string &selectorKey,
	tjs_real &value)
{
	std::string key = MotionLowerString(selectorKey);
	std::string lower = MotionLowerString(src);
	if (key == "helptext" && MotionStartsWith(lower, "src/helptext/")) {
		size_t pos = lower.find("tx_help_");
		if (pos == std::string::npos) return false;
		pos += std::strlen("tx_help_");
		int number = 0;
		bool anyDigit = false;
		while (pos < lower.size() && std::isdigit((unsigned char)lower[pos])) {
			anyDigit = true;
			number = number * 10 + (lower[pos] - '0');
			pos++;
		}
		if (!anyDigit) return false;
		value = number >= 100 ? (tjs_real)(number - 100) : (tjs_real)number;
		return true;
	}
	return false;
}

static MotionFrameLayoutState MotionInterpolateLayoutFrames(
	const MotionFrameLayoutState &curr,
	const MotionFrameLayoutState &next,
	tjs_real ratio)
{
	MotionFrameLayoutState selected = curr;
	selected.coordX = curr.coordX + (next.coordX - curr.coordX) * ratio;
	selected.coordY = curr.coordY + (next.coordY - curr.coordY) * ratio;
	selected.coordZ = curr.coordZ + (next.coordZ - curr.coordZ) * ratio;
	selected.ox = curr.ox + (next.ox - curr.ox) * ratio;
	selected.oy = curr.oy + (next.oy - curr.oy) * ratio;
	selected.scaleX = curr.scaleX + (next.scaleX - curr.scaleX) * ratio;
	selected.scaleY = curr.scaleY + (next.scaleY - curr.scaleY) * ratio;
	selected.opacity = curr.opacity + (next.opacity - curr.opacity) * ratio;
	selected.timeOffset =
		curr.timeOffset + (next.timeOffset - curr.timeOffset) * ratio;
	selected.angle = MotionInterpolateAngleDeg(curr.angle, next.angle, ratio);
	selected.sx = curr.sx + (next.sx - curr.sx) * ratio;
	selected.sy = curr.sy + (next.sy - curr.sy) * ratio;
	selected.hasCoord = curr.hasCoord || next.hasCoord;
	if (curr.hasMeshBP || next.hasMeshBP) {
		selected.hasMeshBP = true;
		for (size_t k = 0; k < 32; k++) {
			selected.bp[k] = curr.bp[k] + (next.bp[k] - curr.bp[k]) * ratio;
		}
	}
	return selected;
}

static bool MotionSelectNodeLayoutFrame(psb_t &psb, const psb_objects_t *node,
	MotionFrameLayoutState &state, bool preferFinalFrame,
	tjs_real tick = 0.0, tjs_real *outTimeOffset = nullptr,
	tjs_int *outFrameCount = nullptr, tjs_int *outType0Count = nullptr,
	const MotionLayoutTransform *limitContext = nullptr,
	MotionLimitSemanticsDiag *limitDiag = nullptr,
	const MotionInfo *motionInfo = nullptr, tjs_int parameterIdx = -1,
	const std::map<std::string, tjs_real> *variables = nullptr,
	const std::string *layoutPath = nullptr)
{
	state = MotionFrameLayoutState();
	if (outTimeOffset) *outTimeOffset = 0.0;
	if (outFrameCount) *outFrameCount = 0;
	if (outType0Count) *outType0Count = 0;
	if (!node) return false;
	std::vector<unsigned char*> frames;
	if (!MotionPsbReadCollection(psb, node->get_data("frameList"), frames)) return false;

	size_t totalFrames = frames.size();
	if (outFrameCount) *outFrameCount = (tjs_int)totalFrames;

	struct FrameWithMeta {
		MotionFrameLayoutState state;
		tjs_real time = 0.0;
		tjs_int frameType = 0;
	};
	std::vector<FrameWithMeta> parsedFrames;
	parsedFrames.reserve(totalFrames);
	for (size_t i = 0; i < totalFrames; i++) {
		tjs_real time = 0.0;
		tjs_int frameType = 0;
		MotionFrameLayoutState candidate;
		if (!MotionReadFrameLayoutState(psb, frames[i], candidate, time, &frameType)) continue;
		FrameWithMeta fwm;
		fwm.state = candidate;
		fwm.time = time;
		fwm.frameType = frameType;
		parsedFrames.push_back(fwm);
		if (frameType == 0 && outType0Count) (*outType0Count)++;
	}

	if (parsedFrames.empty()) return false;

	tjs_real parameterTick = 0.0;
	bool hasParameterTick = MotionResolveParameterTick(
		motionInfo, parameterIdx, variables, parameterTick, layoutPath);
	tjs_real selectorTick = 0.0;
	bool hasSelectorTick = false;
	bool selectorUsesSourceTick = false;
	if (!hasParameterTick && variables && !variables->empty()) {
		std::vector<std::string> pathCandidates;
		std::vector<std::string> sourceCandidates;
		std::string sourceSelectorKey;
		std::string nodeLabel;
		MotionPsbReadMemberString(psb, node, "label", nodeLabel);
		if (layoutPath && !layoutPath->empty()) {
			std::vector<std::string> parts = MotionSplitPath(*layoutPath);
			if (!parts.empty()) {
				const std::string &leaf = parts.back();
				bool leafIsInteractive = MotionSelectorSegmentLooksInteractive(leaf);
				if (parts.size() > 1 || !leafIsInteractive) {
					MotionAddSelectorCandidate(pathCandidates, variables, *layoutPath);
				}
				if (!leafIsInteractive) {
					MotionAddSelectorCandidate(pathCandidates, variables, leaf);
				}
				if (MotionSelectorLeafConsumesInteractiveState(leaf)) {
					for (std::vector<std::string>::const_reverse_iterator it = parts.rbegin();
						it != parts.rend(); ++it) {
						if (!MotionSelectorSegmentLooksInteractive(*it)) continue;
						MotionAddSelectorCandidate(pathCandidates, variables, *it + "/select");
						MotionAddSelectorCandidate(pathCandidates, variables, *it + "/state");
						MotionAddSelectorCandidate(pathCandidates, variables, *it + "/value");
					}
				}
			}
		}
		if (!MotionSelectorSegmentLooksInteractive(nodeLabel)) {
			MotionAddSelectorCandidate(pathCandidates, variables, nodeLabel);
		}
		for (size_t i = 0; i < parsedFrames.size(); i++) {
			MotionAddSelectorSourceCandidate(sourceCandidates, variables,
				parsedFrames[i].state.src);
		}
		hasSelectorTick = MotionResolveInferredSelectorValue(
			pathCandidates, variables, selectorTick);
		if (!hasSelectorTick) {
			hasSelectorTick = MotionResolveInferredSelectorValue(
				sourceCandidates, variables, selectorTick, &sourceSelectorKey);
			if (hasSelectorTick) {
				bool hadMappedSourceFrame = false;
				for (size_t i = 0; i < parsedFrames.size(); i++) {
					tjs_real sourceValue = 0.0;
					if (!MotionResolveSourceSelectorFrameValue(
						parsedFrames[i].state.src, sourceSelectorKey, sourceValue)) {
						continue;
					}
					hadMappedSourceFrame = true;
					if (std::abs((double)(sourceValue - selectorTick)) <= 0.0001) {
						state = parsedFrames[i].state;
						if (outTimeOffset) *outTimeOffset = state.timeOffset;
						return true;
					}
				}
				if (hadMappedSourceFrame) {
					state = MotionFrameLayoutState();
					if (outTimeOffset) *outTimeOffset = state.timeOffset;
					return true;
				}
				selectorUsesSourceTick = true;
			}
		}
	}
#ifdef __EMSCRIPTEN__
	if (MotionClickBurstTraceActive() && layoutPath &&
		MotionTraceInterestingSettingObject(*layoutPath)) {
		char buf[512];
		std::snprintf(buf, sizeof(buf),
			"select-frame path=%s tick=%.3f parameterIdx=%d hasParameter=%d parameterTick=%.3f hasSelector=%d selectorTick=%.3f frames=%zu",
			layoutPath->c_str(), (double)tick, (int)parameterIdx,
			(int)hasParameterTick, (double)parameterTick,
			(int)hasSelectorTick, (double)selectorTick,
			parsedFrames.size());
		MotionClickBurstTraceLog("layout", buf);
	}
#endif
	bool useTickSelection = hasParameterTick || hasSelectorTick || (tick > 0.0001);
	if (useTickSelection) {
		tjs_real effectiveTick = hasParameterTick ?
			parameterTick : (hasSelectorTick ? selectorTick : tick);
		if (!hasParameterTick && !hasSelectorTick && parsedFrames.size() == 2 &&
			parsedFrames[0].frameType == 2 &&
			parsedFrames[1].frameType == 0) {
			effectiveTick = parsedFrames[0].time;
		}

		bool foundByTime = false;
		bool foundNext = false;
		size_t currIdx = 0;
		size_t nextIdx = 0;
		tjs_real bestTime = -1.0;
		for (size_t i = 0; i < parsedFrames.size(); i++) {
			const FrameWithMeta &candidate = parsedFrames[i];
			if (candidate.time <= effectiveTick + 0.0001) {
				if (!foundByTime || candidate.time >= bestTime) {
					currIdx = i;
					bestTime = candidate.time;
					foundByTime = true;
				}
			} else if (!foundNext || candidate.time < parsedFrames[nextIdx].time) {
				nextIdx = i;
				foundNext = true;
			}
		}

		if (!foundByTime) {
			if (hasSelectorTick && !selectorUsesSourceTick) {
				state = parsedFrames[0].state;
			} else {
				state = MotionFrameLayoutState();
			}
			if (outTimeOffset) *outTimeOffset = state.timeOffset;
			return true;
		}

		const FrameWithMeta &curr = parsedFrames[currIdx];
		if (!curr.state.hasContent) {
			state = curr.state;
			if (outTimeOffset) *outTimeOffset = state.timeOffset;
			return true;
		}
		if (foundNext && parsedFrames[nextIdx].time > curr.time &&
			parsedFrames[nextIdx].state.hasContent &&
			effectiveTick > curr.time &&
			(curr.frameType != 2 || parsedFrames[nextIdx].frameType == 2)) {
			FrameWithMeta next = parsedFrames[nextIdx];
			MotionFrameLayoutState currState = curr.state;
			MotionFrameLayoutState nextState = next.state;
			if (limitContext && currState.hasCoord && nextState.hasCoord) {
				int nanC = 0, infC = 0;
				MotionNormalizeNaNInfCoord(currState.coordX, currState.coordY,
					limitContext->originX, limitContext->originY,
					limitContext->width, limitContext->height,
					false, nanC, infC);
				if (limitDiag) {
					limitDiag->nanCoordCount += nanC;
					limitDiag->infCoordCount += infC;
					limitDiag->interpolationNormalizeCount += nanC + infC;
				}
				nanC = 0; infC = 0;
				MotionNormalizeNaNInfCoord(nextState.coordX, nextState.coordY,
					limitContext->originX, limitContext->originY,
					limitContext->width, limitContext->height,
					true, nanC, infC);
				if (limitDiag) {
					limitDiag->nanCoordCount += nanC;
					limitDiag->infCoordCount += infC;
					limitDiag->interpolationNormalizeCount += nanC + infC;
				}
			}
			tjs_real ratio = (effectiveTick - curr.time) / (next.time - curr.time);
			ratio = std::max<tjs_real>(0.0, std::min<tjs_real>(1.0, ratio));
			state = MotionInterpolateLayoutFrames(currState, nextState, ratio);
			if (limitDiag) {
				limitDiag->interpolationFramePairCount++;
				if (currState.hasCoord && nextState.hasCoord) {
					limitDiag->coordInterpolationCount++;
				}
			}
		} else {
			state = curr.state;
		}
		if (outTimeOffset) *outTimeOffset = state.timeOffset;
		return true;
	}

	bool foundAtZero = false;
	MotionFrameLayoutState firstContent;
	MotionFrameLayoutState lastContent;
	MotionFrameLayoutState selected;
	for (size_t i = 0; i < parsedFrames.size(); i++) {
		const FrameWithMeta &fwm = parsedFrames[i];
		if (i == 0) firstContent = fwm.state;
		lastContent = fwm.state;
		if (fwm.time <= 0.0001) {
			selected = fwm.state;
			foundAtZero = true;
			if (outTimeOffset) *outTimeOffset = fwm.state.timeOffset;
		}
	}
	if (preferFinalFrame) {
		state = lastContent;
		if (outTimeOffset) *outTimeOffset = lastContent.timeOffset;
		return true;
	}
	if (foundAtZero) {
		state = selected;
		return true;
	}
	state = firstContent;
	if (outTimeOffset) *outTimeOffset = firstContent.timeOffset;
	return true;
}

static MotionLayoutTransform MotionApplyLayoutFrame(const MotionLayoutTransform &base,
	const MotionFrameLayoutState &frame)
{
	MotionLayoutTransform out = base;
	if (!frame.hasContent) return out;
	MotionAffine2D local = MotionBuildLayoutLocalAffine(frame);
	out.affine = MotionAffineMultiply(base.affine, local);
	MotionAffineApply(out.affine, 0.0, 0.0, out.x, out.y);
	out.scaleX = base.scaleX * frame.scaleX;
	out.scaleY = base.scaleY * frame.scaleY;
	out.opacity = base.opacity * frame.opacity;
	out.angle = base.angle + frame.angle;
	out.sx = base.sx + frame.sx;
	out.sy = base.sy + frame.sy;
	out.ox = frame.ox;
	out.oy = frame.oy;
	if (base.originX != 0.0 || base.originY != 0.0 ||
		base.width > 0.0 || base.height > 0.0) {
		out.originX = base.originX;
		out.originY = base.originY;
		out.width = base.width;
		out.height = base.height;
	}
	return out;
}

static MotionSurfaceStackState MotionAdvanceSurfaceStackForFrame(
	const MotionSurfaceStackState &parent,
	const MotionFrameLayoutState &frame,
	tjs_int nodeType,
	tjs_real originX, tjs_real originY,
	tjs_real width, tjs_real height,
	tjs_real limOriginX, tjs_real limOriginY,
	tjs_real limWidth, tjs_real limHeight,
	const ttstr &label,
	MotionRenderMethodSemanticsDiag *diag,
	tjs_real limZMax = kMotionDefaultLimitZMax)
{
	if (limZMax < 0.0001) limZMax = kMotionDefaultLimitZMax;
	MotionSurfaceStackState out = parent;
	if (!frame.hasContent) return out;

	tjs_real frameCoordZ = frame.hasCoord ? frame.coordZ : 0.0;
	out.currCoordz = parent.currCoordz + frameCoordZ;

	MotionMat4 model = MotionMat4BuildModelFromFrame(frame);
	if (nodeType == 7) {
		return out;
	}
	bool isLayout =
		nodeType == 12 ||
		frame.src.empty() ||
		frame.src == "layout" ||
		frame.src == "clip" ||
		MotionStartsWith(frame.src, "motion/");

	if (isLayout) {
		out.valid = true;
		out.originX = originX;
		out.originY = originY;
		out.width = width;
		out.height = height;
		if (parent.valid && parent.lastSurface.type == 3) {
			out.layoutChain = MotionMat4Multiply(parent.layoutChain, model);
		} else if (parent.valid && parent.lastSurface.type == 2) {
			out = parent;
			MotionRenderSurface surface;
			if (out.surfaces.size() > 1) {
				MotionRenderSurface parentSurface = out.surfaces.back();
				out.surfaces.pop_back();
				const MotionRenderSurface &limSurface = out.surfaces.back();
				MotionMat4 demuxMat = MotionBuildDemuxMatFromSurface(parentSurface,
					true, limSurface.originX, limSurface.originY,
					limSurface.originW, limSurface.originH);
				surface = MotionBuildLayoutRenderSurface(
					MotionMat4Multiply(demuxMat, model),
					label, limSurface.originX, limSurface.originY,
					limSurface.originW, limSurface.originH,
					out.currCoordz);
				surface.opa *= parentSurface.opa;
				if (diag) diag->m3DemuxResolvedCount++;
				} else {
					surface = MotionBuildLayoutRenderSurface(model,
						label, originX, originY, width, height,
						out.currCoordz);
					surface.opa *= parent.lastSurface.opa;
				}
				out.layoutChain = surface.matTrans;
			out.lastSurface = surface;
			out.surfaces.push_back(surface);
			out.originX = surface.originX;
			out.originY = surface.originY;
			out.width = surface.originW;
			out.height = surface.originH;
			return out;
		} else {
			out.layoutChain = model;
		}
		out.lastSurface = MotionBuildLayoutRenderSurface(out.layoutChain,
			label, originX, originY, width, height,
			out.currCoordz);
		if (parent.valid && parent.lastSurface.type == 3) {
			out.lastSurface.opa = parent.lastSurface.opa;
		}
		if (!out.surfaces.empty() && parent.valid && parent.lastSurface.type == 3) {
			out.surfaces.back() = out.lastSurface;
		} else {
			out.surfaces.clear();
			out.surfaces.push_back(out.lastSurface);
		}
		return out;
	}

	MotionMat4 drawModel = MotionMat4ApplyIconLocal(model,
		originX, originY, frame.ox, frame.oy, width, height);
	if (parent.valid && parent.lastSurface.type == 2) {
		out = parent;
		MotionMat4 demuxMat = MotionBuildDemuxMatFromSurface(parent.lastSurface,
			false, 0.0, 0.0, 0.0, 0.0);
		MotionMat4 finalMat = MotionMat4Multiply(demuxMat, drawModel);
		MotionRenderSurface surface = MotionBuildLayoutRenderSurface(finalMat,
			label, originX, originY, width, height,
			out.currCoordz);
		surface.type = frame.hasMeshBP ? 1 : 2;
		surface.opa = parent.lastSurface.opa * frame.opacity;
		for (size_t k = 0; k < 32; k++) surface.controlPts[k] = frame.bp[k];
		if (out.surfaces.empty()) {
			out.surfaces.push_back(surface);
		} else {
			out.surfaces.back() = surface;
		}
		out.valid = true;
		out.unsupportedDemux = false;
		out.originX = originX;
		out.originY = originY;
		out.width = width;
		out.height = height;
		out.layoutChain = finalMat;
		out.lastSurface = surface;
		if (diag) diag->m3DemuxResolvedCount++;
		return out;
	}

	MotionMat4 finalMat;
	if (parent.valid && parent.lastSurface.type == 3) {
		MotionMat4 projection = MotionMat4Ortho(-limOriginX, limWidth - limOriginX,
			limHeight - limOriginY, -limOriginY, limZMax, -limZMax);
		finalMat = MotionMat4Multiply(projection,
			MotionMat4Multiply(parent.layoutChain, drawModel));
	} else {
		MotionMat4 projection = MotionMat4Ortho(-limOriginX, limWidth - limOriginX,
			limHeight - limOriginY, -limOriginY, limZMax, -limZMax);
		finalMat = MotionMat4Multiply(projection, drawModel);
	}
	out.valid = true;
	out.originX = originX;
	out.originY = originY;
	out.width = width;
	out.height = height;
	out.layoutChain = finalMat;
	out.lastSurface = MotionBuildLayoutRenderSurface(finalMat,
		label, originX, originY, width, height,
		out.currCoordz);
	out.lastSurface.type = frame.hasMeshBP ? 1 : 2;
	out.lastSurface.opa =
		(parent.valid && parent.lastSurface.type == 3) ?
		parent.lastSurface.opa * frame.opacity : frame.opacity;
	for (size_t k = 0; k < 32; k++) out.lastSurface.controlPts[k] = frame.bp[k];
	if (!out.surfaces.empty() && parent.valid && parent.lastSurface.type == 3) {
		out.surfaces.back() = out.lastSurface;
	} else {
		out.surfaces.clear();
		out.surfaces.push_back(out.lastSurface);
	}
	return out;
}

static void MotionCollectLayoutNodes(psb_t &psb, unsigned char *nodeData, int parent,
	std::vector<MotionNodeLayoutEntry> &nodes, int depth, tjs_int inheritedParameterIdx = -1)
{
	if (!nodeData || depth > 64) return;
	int myIndex = (int)nodes.size();
	MotionNodeLayoutEntry entry;
	entry.data = nodeData;
	entry.parent = parent;
	entry.parameterIdx = inheritedParameterIdx;
	nodes.push_back(entry);

	psb_objects_t *node = nullptr;
	if (!MotionUnpackObject(psb, nodeData, node)) return;
	MotionPsbReadMemberString(psb, node, "label", nodes[(size_t)myIndex].label);
	MotionPsbReadMemberInt(psb, node, "type", nodes[(size_t)myIndex].type);
	int parameterIdx = -1;
	if (MotionPsbReadMemberInt(psb, node, "parameterize", parameterIdx)) {
		nodes[(size_t)myIndex].parameterIdx = parameterIdx;
	}
	std::vector<unsigned char*> children;
	if (MotionPsbReadCollection(psb, node->get_data("children"), children)) {
		tjs_int childParameterIdx = nodes[(size_t)myIndex].parameterIdx;
		for (size_t i = 0; i < children.size(); i++) {
			MotionCollectLayoutNodes(psb, children[i], myIndex, nodes, depth + 1,
				childParameterIdx);
		}
	}
	delete node;
}

static void MotionAddSourceKey(std::map<std::string, MotionSourceIconInfo> &sourceMap,
	const std::string &key, const MotionSourceIconInfo &info)
{
	if (key.empty() || sourceMap.find(key) != sourceMap.end()) return;
	MotionSourceIconInfo stored = info;
	stored.key = key;
	if (stored.canonicalKey.empty()) stored.canonicalKey = info.name;
	sourceMap[key] = stored;
}

static void MotionReadSourceIcons(psb_t &psb, const psb_objects_t *root,
	std::map<std::string, MotionSourceIconInfo> &sourceMap)
{
	sourceMap.clear();
	psb_objects_t *sources = nullptr;
	if (!MotionUnpackObject(psb, root->get_data("source"), sources)) return;
	for (uint32_t si = 0; si < sources->size(); si++) {
		std::string sourceName = sources->get_name(si);
		psb_objects_t *source = nullptr;
		if (!MotionUnpackObject(psb, sources->get_data(si), source)) continue;
		psb_objects_t *icons = nullptr;
		if (MotionUnpackObject(psb, source->get_data("icon"), icons)) {
			for (uint32_t ii = 0; ii < icons->size(); ii++) {
				std::string iconName = icons->get_name(ii);
				psb_objects_t *icon = nullptr;
				if (!MotionUnpackObject(psb, icons->get_data(ii), icon)) continue;
				MotionSourceIconInfo info;
				info.sourceName = sourceName;
				info.iconName = iconName;
				info.name = "src/" + sourceName + "/" + iconName;
				info.canonicalKey = info.name;
				MotionPsbReadMemberNumber(psb, icon, "originX", info.originX);
				MotionPsbReadMemberNumber(psb, icon, "originY", info.originY);
				MotionPsbReadMemberNumber(psb, icon, "width", info.width);
				MotionPsbReadMemberNumber(psb, icon, "height", info.height);
				info.hasPixel = icon->get_data("pixel") != nullptr;
				info.hasPalette = icon->get_data("pal") != nullptr;
				MotionPsbReadResourceLength(psb, icon->get_data("pixel"), info.pixelLength);
				MotionPsbReadResourceLength(psb, icon->get_data("pal"), info.paletteLength);
				std::string compress;
				std::string type;
				MotionPsbReadMemberString(psb, icon, "compress", compress);
				MotionPsbReadMemberString(psb, icon, "type", type);
				bool rawRGBA =
					info.hasPixel &&
					info.width > 0.0 &&
					info.height > 0.0 &&
					(type == "RGBA8" || (compress.empty() && type.empty())) &&
					info.pixelLength >=
						(tjs_uint32)std::lround(info.width * info.height * 4.0);
				bool rlDecoded =
					info.hasPixel &&
					compress == "RL" &&
					(!info.hasPalette || info.paletteLength >= 4);
				info.decoderSupported =
					rlDecoded || rawRGBA;
				if (info.width > 0.0 && info.height > 0.0) {
					MotionAddSourceKey(sourceMap, info.name, info);
					MotionAddSourceKey(sourceMap, sourceName + "/" + iconName, info);
				}
				delete icon;
			}
			delete icons;
		}
		delete source;
	}
	delete sources;
}

static bool MotionInferScreenOriginFromSourceIcons(
	const std::map<std::string, MotionSourceIconInfo> &sourceMap,
	const MotionResource &resource, int &originX, int &originY)
{
	if (resource.width <= 0 || resource.height <= 0) return false;
	double bestScore = -1.0;
	int bestX = 0;
	int bestY = 0;
	for (std::map<std::string, MotionSourceIconInfo>::const_iterator it = sourceMap.begin();
		it != sourceMap.end(); ++it) {
		const MotionSourceIconInfo &icon = it->second;
		if (icon.originX == 0.0 && icon.originY == 0.0) continue;
		if (icon.width <= 0.0 || icon.height <= 0.0) continue;
		if (icon.originX < 0.0 || icon.originY < 0.0 ||
			icon.originX > icon.width || icon.originY > icon.height) {
			continue;
		}
		if (icon.width < (tjs_real)resource.width * 0.75 ||
			icon.height < (tjs_real)resource.height * 0.50) {
			continue;
		}
		if (icon.width > (tjs_real)resource.width * 2.0 ||
			icon.height > (tjs_real)resource.height * 2.0) {
			continue;
		}
		double widthFit = 1.0 - std::min(1.0,
			std::abs(icon.width - (tjs_real)resource.width) /
			std::max<tjs_real>(1.0, (tjs_real)resource.width));
		double heightFit = 1.0 - std::min(1.0,
			std::abs(icon.height - (tjs_real)resource.height) /
			std::max<tjs_real>(1.0, (tjs_real)resource.height));
		double originCenterFit = 1.0 - std::min(1.0,
			std::abs(icon.originX - icon.width / 2.0) /
			std::max<tjs_real>(1.0, icon.width / 2.0));
		double score = widthFit * 4.0 + heightFit * 2.0 + originCenterFit;
		if (score > bestScore) {
			bestScore = score;
			bestX = (int)std::lround(icon.originX);
			bestY = (int)std::lround(icon.originY);
		}
	}
	if (bestScore < 0.0) return false;
	originX = bestX;
	originY = bestY;
	return true;
}

static void MotionApplyMotionStyleCenterOriginFallback(
	const std::map<std::string, MotionSourceIconInfo> &sourceMap,
	const MotionResource &resource,
	int &originX, int &originY)
{
	if (resource.width <= 0 || resource.height <= 0) return;
	if (originX != 0 && originY != 0) return;
	bool hasCanvasSizedImage = false;
	for (std::map<std::string, MotionSourceIconInfo>::const_iterator it = sourceMap.begin();
		it != sourceMap.end(); ++it) {
		const MotionSourceIconInfo &icon = it->second;
		if (icon.width >= (tjs_real)resource.width * 0.75 &&
			icon.height >= (tjs_real)resource.height * 0.75) {
			hasCanvasSizedImage = true;
			break;
		}
	}
	if (hasCanvasSizedImage) return;
	if (originX == 0) originX = resource.width / 2;
	if (originY == 0) originY = resource.height / 2;
}

static bool MotionLayoutPrefersFinalFrame(const std::string &motionName)
{
	std::string lower = MotionLowerString(motionName);
	return MotionStartsWith(lower, "show") ||
		lower == "on" ||
		lower == "over" ||
		lower == "press" ||
		lower == "bt" ||
		lower == "state";
}

static bool MotionLayoutCollectsStateVariants(const std::string &motionName)
{
	return MotionLowerString(motionName) == "state";
}

static bool MotionNameMatches(const ttstr &a, const ttstr &b);

static ttstr MotionLeafLayoutSegment(const ttstr &path)
{
	std::string value = path.AsNarrowStdString();
	size_t slash = value.rfind('/');
	if (slash != std::string::npos && slash + 1 < value.size()) {
		value = value.substr(slash + 1);
	}
	return MotionString(value);
}

static bool MotionStringHasInteractivePrefix(const ttstr &value)
{
	std::string lower = value.AsLowerCase().AsNarrowStdString();
	return MotionStartsWith(lower, "bt") ||
		MotionStartsWith(lower, "tg") ||
		MotionStartsWith(lower, "sld") ||
		MotionStartsWith(lower, "slot") ||
		MotionStartsWith(lower, "page") ||
		lower.find("button") != std::string::npos ||
		lower.find("slider") != std::string::npos ||
		lower.find("toggle") != std::string::npos;
}

static bool MotionLayoutNameIsProxyAlias(const ttstr &value)
{
	std::string lower = value.AsLowerCase().AsNarrowStdString();
	return MotionStartsWith(lower, "slot/") ||
		MotionStartsWith(lower, "page/");
}

static bool MotionLayoutSegmentIsStateName(const std::string &segment)
{
	std::string lower = MotionLowerString(segment);
	return lower == "press" ||
		lower == "pressed" ||
		lower == "down" ||
		lower == "over" ||
		lower == "hover" ||
		lower == "focus" ||
		lower == "on" ||
		lower == "bt" ||
		lower == "base" ||
		lower == "normal" ||
		lower == "show" ||
		lower == "show2" ||
		lower == "state" ||
		lower == "select" ||
		lower == "slide" ||
		lower == "off" ||
		lower == "out" ||
		lower == "hide" ||
		lower == "hide2" ||
		lower == "disable" ||
		lower == "disabled" ||
		lower == "menubar" ||
		lower == "menubar_on";
}

static bool MotionLayoutSegmentIsHitPart(const std::string &segment)
{
	std::string lower = MotionLowerString(segment);
	return lower == "area" ||
		lower == "offarea" ||
		lower == "hit" ||
		lower == "button" ||
		lower == "slider" ||
		lower == "toggle" ||
		MotionStartsWith(lower, "bt") ||
		MotionStartsWith(lower, "tg") ||
		MotionStartsWith(lower, "sld") ||
		MotionStartsWith(lower, "slot") ||
		MotionStartsWith(lower, "page");
}

static ttstr MotionInferHitPartName(const ttstr &layoutName, const ttstr &objectName)
{
	std::vector<std::string> parts = MotionSplitPath(layoutName.AsNarrowStdString());
	for (std::vector<std::string>::const_reverse_iterator it = parts.rbegin();
		it != parts.rend(); ++it) {
		if (MotionLayoutSegmentIsHitPart(*it)) return MotionString(*it);
	}
	if (objectName.GetLen() > 0 &&
		MotionLayoutSegmentIsHitPart(objectName.AsNarrowStdString())) {
		return objectName;
	}
	return MotionLeafLayoutSegment(layoutName);
}

static ttstr MotionInferLayoutPathObjectName(const ttstr &layoutName,
	const ttstr &objectName, const ttstr &contextObjectName)
{
	std::vector<std::string> parts = MotionSplitPath(layoutName.AsNarrowStdString());
	for (size_t i = 0; i < parts.size(); i++) {
		const std::string &part = parts[i];
		if (MotionStartsWith(MotionLowerString(part), "bt") ||
			MotionStartsWith(MotionLowerString(part), "tg") ||
			MotionStartsWith(MotionLowerString(part), "sld") ||
			MotionStartsWith(MotionLowerString(part), "slot") ||
			MotionStartsWith(MotionLowerString(part), "page")) {
			return MotionString(part);
		}
	}
	if (objectName.GetLen() > 0 && MotionStringHasInteractivePrefix(objectName)) {
		return objectName;
	}
	if (contextObjectName.GetLen() > 0) return contextObjectName;
	return objectName;
}

static ttstr MotionInferHitObjectName(const ttstr &layoutName,
	const ttstr &objectName, const ttstr &contextObjectName)
{
	ttstr pathObject = MotionInferLayoutPathObjectName(
		layoutName, objectName, contextObjectName);
	if (pathObject.GetLen() > 0) return pathObject;
	std::string layout = layoutName.AsNarrowStdString();
	if (!layout.empty()) {
		size_t slash = layout.find('/');
		if (slash != std::string::npos && slash > 0) {
			return MotionString(layout.substr(0, slash));
		}
	}
	return objectName;
}

static ttstr MotionInferHitStateName(const MotionLayoutInfo &layout)
{
	if (layout.motionName.GetLen() > 0) return layout.motionName;
	if (layout.contextMotionName.GetLen() > 0) return layout.contextMotionName;
	std::vector<std::string> parts = MotionSplitPath(layout.name.AsNarrowStdString());
	for (std::vector<std::string>::const_reverse_iterator it = parts.rbegin();
		it != parts.rend(); ++it) {
		if (MotionLayoutSegmentIsStateName(*it)) return MotionString(*it);
	}
	return MotionLeafLayoutSegment(layout.name);
}

static ttstr MotionClassifyHitState(const ttstr &stateName)
{
	std::string lower = stateName.AsLowerCase().AsNarrowStdString();
	if (lower == "press" || lower == "pressed" || lower == "down") return TJS_W("press");
	if (lower == "over" || lower == "hover" || lower == "focus" || lower == "on") return TJS_W("hover");
	if (lower == "bt" || lower == "base" || lower == "normal" ||
		lower == "show" || lower == "show2" || lower == "state" ||
		lower == "select" || lower == "slide") {
		return TJS_W("active");
	}
	if (lower == "off" || lower == "out") return TJS_W("inactive");
	if (lower == "hide" || lower == "hide2" || lower == "disable" ||
		lower == "disabled") {
		return TJS_W("hidden");
	}
	return TJS_W("passive");
}

static tjs_int MotionHitStateBasePriority(const ttstr &stateClass)
{
	if (stateClass == TJS_W("press")) return 900;
	if (stateClass == TJS_W("hover")) return 800;
	if (stateClass == TJS_W("active")) return 700;
	if (stateClass == TJS_W("inactive")) return 350;
	if (stateClass == TJS_W("passive")) return 200;
	return 0;
}

static bool MotionHitStateClassIsVisible(const ttstr &stateClass)
{
	return stateClass != TJS_W("hidden");
}

static bool MotionHitStateClassIsInteractive(const ttstr &stateClass)
{
	return stateClass == TJS_W("press") ||
		stateClass == TJS_W("hover") ||
		stateClass == TJS_W("active") ||
		stateClass == TJS_W("inactive");
}

static ttstr MotionJoinLayoutSegments(const std::vector<std::string> &segments,
	size_t begin, size_t end)
{
	if (begin >= end || begin >= segments.size()) return ttstr();
	end = std::min(end, segments.size());
	std::string joined;
	for (size_t i = begin; i < end; i++) {
		if (segments[i].empty()) continue;
		if (!joined.empty()) joined += "/";
		joined += segments[i];
	}
	return MotionString(joined);
}

static ttstr MotionInferHitObjectPath(const ttstr &layoutName,
	const ttstr &hitObjectName)
{
	std::vector<std::string> parts = MotionSplitPath(layoutName.AsNarrowStdString());
	if (parts.empty()) return hitObjectName;
	std::string hitObject = hitObjectName.AsNarrowStdString();
	if (!hitObject.empty()) {
		for (size_t i = 0; i < parts.size(); i++) {
			if (MotionLowerString(parts[i]) == MotionLowerString(hitObject)) {
				return MotionJoinLayoutSegments(parts, 0, i + 1);
			}
		}
	}
	for (size_t i = 0; i < parts.size(); i++) {
		ttstr part = MotionString(parts[i]);
		if (MotionStringHasInteractivePrefix(part)) {
			return MotionJoinLayoutSegments(parts, 0, i + 1);
		}
		if (MotionLayoutSegmentIsStateName(parts[i]) ||
			MotionLayoutSegmentIsHitPart(parts[i])) {
			return MotionJoinLayoutSegments(parts, 0, i);
		}
	}
	if (parts.size() > 1) return MotionJoinLayoutSegments(parts, 0, parts.size() - 1);
	return hitObjectName;
}

static ttstr MotionInferHitStatePath(const ttstr &layoutName,
	const ttstr &objectPath, const ttstr &stateName)
{
	std::vector<std::string> parts = MotionSplitPath(layoutName.AsNarrowStdString());
	if (parts.empty()) return stateName;
	std::string state = stateName.AsNarrowStdString();
	if (!state.empty()) {
		for (std::vector<std::string>::const_reverse_iterator it = parts.rbegin();
			it != parts.rend(); ++it) {
			if (MotionLowerString(*it) == MotionLowerString(state)) {
				size_t end = (size_t)(it.base() - parts.begin());
				return MotionJoinLayoutSegments(parts, 0, end);
			}
		}
	}
	if (objectPath.GetLen() > 0 && stateName.GetLen() > 0) {
		ttstr path = objectPath;
		path += TJS_W("/");
		path += stateName;
		return path;
	}
	return stateName;
}

static ttstr MotionInferHitStateGroup(const ttstr &stateClass)
{
	if (stateClass == TJS_W("press")) return TJS_W("pressed");
	if (stateClass == TJS_W("hover")) return TJS_W("hovered");
	if (stateClass == TJS_W("active")) return TJS_W("active");
	if (stateClass == TJS_W("inactive")) return TJS_W("inactive");
	if (stateClass == TJS_W("hidden")) return TJS_W("hidden");
	return TJS_W("passive");
}

static void MotionResolveSemanticObjectPath(MotionLayoutInfo &layout);

static void MotionFinalizeLayoutHitSemantics(MotionLayoutInfo &layout)
{
	layout.hitObjectName = MotionInferHitObjectName(
		layout.name, layout.objectName, layout.contextObjectName);
	layout.hitStateName = MotionInferHitStateName(layout);
	layout.hitStateClass = MotionClassifyHitState(layout.hitStateName);
	layout.hitStateGroup = MotionInferHitStateGroup(layout.hitStateClass);
	layout.hitPartName = MotionInferHitPartName(layout.name, layout.objectName);
	layout.hitObjectPath = MotionInferHitObjectPath(layout.name, layout.hitObjectName);
	layout.hitStatePath = MotionInferHitStatePath(
		layout.name, layout.hitObjectPath, layout.hitStateName);
	MotionResolveSemanticObjectPath(layout);
	layout.hitLayoutProxy = MotionLayoutNameIsProxyAlias(layout.name);
	layout.hitPathDepth = (tjs_int)MotionSplitPath(layout.name.AsNarrowStdString()).size();
	layout.hitRole = TJS_W("visual");
	if (MotionStringHasInteractivePrefix(layout.name) ||
		MotionStringHasInteractivePrefix(layout.objectName) ||
		MotionStringHasInteractivePrefix(layout.hitObjectName) ||
		MotionStringHasInteractivePrefix(layout.hitPartName)) {
		layout.hitRole = TJS_W("control");
	}
	if (layout.hitPartName.AsLowerCase() == TJS_W("area") ||
		layout.hitPartName.AsLowerCase() == TJS_W("offarea")) {
		layout.hitRole = TJS_W("hit-area");
	}
	layout.hitPriority = MotionHitStateBasePriority(layout.hitStateClass);
	if (MotionStringHasInteractivePrefix(layout.name)) layout.hitPriority += 180;
	if (MotionStringHasInteractivePrefix(layout.objectName)) layout.hitPriority += 120;
	if (MotionStringHasInteractivePrefix(layout.hitObjectName)) layout.hitPriority += 120;
	if (layout.hitPartName.AsLowerCase() == TJS_W("area")) layout.hitPriority += 220;
	if (layout.hitPartName.AsLowerCase() == TJS_W("offarea")) layout.hitPriority += 120;
	if (layout.hitRole == TJS_W("control")) layout.hitPriority += 120;
	if (layout.hitRole == TJS_W("hit-area")) layout.hitPriority += 160;
	if (layout.hitLayoutProxy) layout.hitPriority -= 260;
	if (layout.contextObjectName.GetLen() > 0 &&
		MotionNameMatches(layout.contextObjectName, layout.hitObjectName)) {
		layout.hitPriority += 60;
	}
	layout.hitInteractive =
		MotionHitStateClassIsInteractive(layout.hitStateClass) &&
		MotionHitStateClassIsVisible(layout.hitStateClass) &&
		(MotionStringHasInteractivePrefix(layout.name) ||
			MotionStringHasInteractivePrefix(layout.objectName) ||
			MotionStringHasInteractivePrefix(layout.hitObjectName) ||
			MotionStringHasInteractivePrefix(layout.hitPartName) ||
			layout.hitRole == TJS_W("hit-area"));
	if (layout.hitInteractive) layout.hitPriority += 300;
	if (layout.opacityMax <= 0.001 || !MotionHitStateClassIsVisible(layout.hitStateClass)) {
		layout.hitInteractive = false;
		layout.hitPriority -= 1000;
	}
	layout.frontDrawCandidate = !layout.hitLayoutProxy &&
		MotionHitStateClassIsVisible(layout.hitStateClass) &&
		layout.opacityMax > 0.001;
	layout.frontHitCandidate = layout.frontDrawCandidate && layout.hitInteractive;
	if (!layout.frontHitCandidate &&
		layout.frontDrawCandidate &&
		layout.hitRole == TJS_W("hit-area")) {
		layout.frontHitCandidate = true;
	}
}

static void MotionMergeResourceNamedLayout(MotionResource &resource,
	const ttstr &layoutName, const ttstr &objectName, const ttstr &motionName,
	const ttstr &contextObjectName, const ttstr &contextMotionName,
	const MotionLayoutBounds &bounds, tjs_int renderMethodItemIndex = -1)
{
	if (!bounds.valid) return;
#ifdef __EMSCRIPTEN__
	bool perfEnabled = gMotionLayoutBuildPerf != nullptr;
	tjs_uint64 perfStart = perfEnabled ? TVPGetTickCount() : 0;
	tjs_uint64 perfAfterSearch = perfStart;
	tjs_uint64 perfSemanticsMs = 0;
	tjs_uint64 perfPushMs = 0;
	size_t perfScanSteps = 0;
	std::string perfLabel;
	if (perfEnabled) {
		perfLabel = layoutName.AsNarrowStdString() + "|" +
			objectName.AsNarrowStdString() + "/" +
			motionName.AsNarrowStdString();
	}
#endif
	MotionLayoutMergeIndexState *indexState = gMotionLayoutMergeIndex;
	const bool indexWritable =
		indexState && indexState->resource == &resource;
	const bool indexReadOnly =
		indexState && indexState->resource != &resource;
	std::string indexKey;
	size_t searchBegin = 0;
	size_t searchEnd = resource.layouts.size();
	bool skipSearchForIndexedCreate = false;
	if (indexState) {
		indexKey = MotionMakeLayoutMergeKey(layoutName, objectName, motionName,
			contextObjectName, contextMotionName);
		std::unordered_map<std::string, tjs_int>::const_iterator found =
			indexState->entries.find(indexKey);
		if (found != indexState->entries.end() &&
			found->second >= 0 &&
			found->second < (tjs_int)resource.layouts.size()) {
			size_t indexed = (size_t)found->second;
			const MotionLayoutInfo &item = resource.layouts[indexed];
			if (item.name == layoutName &&
				item.objectName == objectName &&
				item.motionName == motionName &&
				item.contextObjectName == contextObjectName &&
				item.contextMotionName == contextMotionName) {
				searchBegin = indexed;
				searchEnd = indexed + 1;
				if (indexWritable) indexState->writableHits++;
				else if (indexReadOnly) indexState->readOnlyHits++;
			} else {
				indexState->mismatches++;
				if (indexWritable) indexState->entries.erase(indexKey);
			}
		} else if (indexWritable) {
			indexState->writableMisses++;
			skipSearchForIndexedCreate = true;
			searchBegin = searchEnd;
		} else if (indexReadOnly) {
			indexState->fallbackScans++;
		}
	}
	for (size_t i = searchBegin; i < searchEnd; i++) {
#ifdef __EMSCRIPTEN__
		if (perfEnabled) perfScanSteps++;
#endif
		MotionLayoutInfo &item = resource.layouts[i];
		if (item.name == layoutName &&
			item.objectName == objectName &&
			item.motionName == motionName &&
			item.contextObjectName == contextObjectName &&
			item.contextMotionName == contextMotionName) {
#ifdef __EMSCRIPTEN__
			if (perfEnabled) perfAfterSearch = TVPGetTickCount();
#endif
			MotionLayoutBounds merged;
			merged.valid = true;
			merged.minX = item.left;
			merged.minY = item.top;
			merged.maxX = item.left + item.width;
			merged.maxY = item.top + item.height;
			merged.drawCount = item.drawCount;
			MotionMergeLayoutBounds(merged, bounds);
			item.left = merged.minX;
			item.top = merged.minY;
			item.width = merged.maxX - merged.minX;
			item.height = merged.maxY - merged.minY;
			item.centerX = item.left + item.width / 2.0;
			item.centerY = item.top + item.height / 2.0;
			if (merged.opacityValid) {
				item.opacityMin = merged.opacityMin;
				item.opacityMax = merged.opacityMax;
			}
			item.drawCount = merged.drawCount;
			if (!bounds.sourceKey.empty()) {
				item.sourceKey = MotionString(bounds.sourceKey);
			}
#ifdef __EMSCRIPTEN__
			tjs_uint64 perfSemanticsStart = perfEnabled ? TVPGetTickCount() : 0;
#endif
			MotionFinalizeLayoutHitSemantics(item);
#ifdef __EMSCRIPTEN__
			if (perfEnabled) perfSemanticsMs += TVPGetTickCount() - perfSemanticsStart;
#endif
			if (renderMethodItemIndex >= 0 &&
				renderMethodItemIndex < (tjs_int)resource.renderMethodIR.size()) {
				resource.renderMethodIR[(size_t)renderMethodItemIndex].layoutIndex = (tjs_int)i;
				resource.renderMethodIR[(size_t)renderMethodItemIndex].linkedToLayout = true;
				item.renderMethodItemIndex = renderMethodItemIndex;
			}
			if (bounds.hasAffine) {
				item.angle = bounds.angle;
				item.sx = bounds.sx;
				item.sy = bounds.sy;
				item.ox = bounds.ox;
				item.oy = bounds.oy;
				item.iconOriginX = bounds.iconOriginX;
				item.iconOriginY = bounds.iconOriginY;
				item.iconWidth = bounds.iconWidth;
				item.iconHeight = bounds.iconHeight;
				item.accumX = bounds.accumX;
				item.accumY = bounds.accumY;
				item.accumScaleX = bounds.accumScaleX;
				item.accumScaleY = bounds.accumScaleY;
				item.affineValid = true;
				item.affine = bounds.affine;
			}
			if (bounds.hasControlPoints) {
				item.hasControlPoints = true;
				for (size_t k = 0; k < 32; k++) item.controlPts[k] = bounds.controlPts[k];
			}
			if (indexWritable && !indexKey.empty()) {
				indexState->entries[indexKey] = (tjs_int)i;
			}
#ifdef __EMSCRIPTEN__
			if (perfEnabled) {
				tjs_uint64 perfEnd = TVPGetTickCount();
				MotionLayoutBuildPerfCounters &perf = *gMotionLayoutBuildPerf;
				perf.namedMergeCalls++;
				perf.namedMergeUpdates++;
				perf.namedMergeScanSteps += perfScanSteps;
				perf.namedMergeTotalMs += perfEnd - perfStart;
				perf.namedMergeSearchMs += perfAfterSearch - perfStart;
				perf.namedMergeSemanticsMs += perfSemanticsMs;
				MotionLayoutPerfNoteMax(perfEnd - perfStart, perfLabel,
					perf.maxNamedMergeMs, perf.maxNamedMergeLabel);
			}
#endif
			return;
		}
	}
#ifdef __EMSCRIPTEN__
	if (perfEnabled) perfAfterSearch = TVPGetTickCount();
#endif
	if (!skipSearchForIndexedCreate && indexWritable && !indexKey.empty()) {
		indexState->writableMisses++;
	}

	MotionLayoutInfo layout;
	layout.name = layoutName;
	layout.sourceKey = bounds.sourceKey.empty() ? ttstr() : MotionString(bounds.sourceKey);
	layout.objectName = objectName;
	layout.motionName = motionName;
	layout.contextObjectName = contextObjectName;
	layout.contextMotionName = contextMotionName;
	layout.left = bounds.minX;
	layout.top = bounds.minY;
	layout.width = bounds.maxX - bounds.minX;
	layout.height = bounds.maxY - bounds.minY;
	layout.centerX = layout.left + layout.width / 2.0;
	layout.centerY = layout.top + layout.height / 2.0;
	if (bounds.opacityValid) {
		layout.opacityMin = bounds.opacityMin;
		layout.opacityMax = bounds.opacityMax;
	}
	layout.drawCount = bounds.drawCount;
	if (bounds.hasAffine) {
		layout.angle = bounds.angle;
		layout.sx = bounds.sx;
		layout.sy = bounds.sy;
		layout.ox = bounds.ox;
		layout.oy = bounds.oy;
		layout.iconOriginX = bounds.iconOriginX;
		layout.iconOriginY = bounds.iconOriginY;
		layout.iconWidth = bounds.iconWidth;
		layout.iconHeight = bounds.iconHeight;
		layout.accumX = bounds.accumX;
		layout.accumY = bounds.accumY;
		layout.accumScaleX = bounds.accumScaleX;
		layout.accumScaleY = bounds.accumScaleY;
		layout.affineValid = true;
		layout.affine = bounds.affine;
	}
	if (bounds.hasControlPoints) {
		layout.hasControlPoints = true;
		for (size_t k = 0; k < 32; k++) layout.controlPts[k] = bounds.controlPts[k];
	}
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfSemanticsStart = perfEnabled ? TVPGetTickCount() : 0;
#endif
	MotionFinalizeLayoutHitSemantics(layout);
#ifdef __EMSCRIPTEN__
	if (perfEnabled) perfSemanticsMs += TVPGetTickCount() - perfSemanticsStart;
#endif
	tjs_int newIndex = (tjs_int)resource.layouts.size();
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfPushStart = perfEnabled ? TVPGetTickCount() : 0;
#endif
	resource.layouts.push_back(layout);
#ifdef __EMSCRIPTEN__
	if (perfEnabled) perfPushMs += TVPGetTickCount() - perfPushStart;
#endif
	if (renderMethodItemIndex >= 0 &&
		renderMethodItemIndex < (tjs_int)resource.renderMethodIR.size()) {
		resource.renderMethodIR[(size_t)renderMethodItemIndex].layoutIndex = newIndex;
		resource.renderMethodIR[(size_t)renderMethodItemIndex].linkedToLayout = true;
		resource.layouts[(size_t)newIndex].renderMethodItemIndex = renderMethodItemIndex;
	}
	if (indexWritable && !indexKey.empty()) {
		indexState->entries[indexKey] = newIndex;
	}
#ifdef __EMSCRIPTEN__
	if (perfEnabled) {
		tjs_uint64 perfEnd = TVPGetTickCount();
		MotionLayoutBuildPerfCounters &perf = *gMotionLayoutBuildPerf;
		perf.namedMergeCalls++;
		perf.namedMergeCreates++;
		perf.namedMergeScanSteps += perfScanSteps;
		perf.namedMergeTotalMs += perfEnd - perfStart;
		perf.namedMergeSearchMs += perfAfterSearch - perfStart;
		perf.namedMergeSemanticsMs += perfSemanticsMs;
		perf.namedMergePushMs += perfPushMs;
		MotionLayoutPerfNoteMax(perfEnd - perfStart, perfLabel,
			perf.maxNamedMergeMs, perf.maxNamedMergeLabel);
	}
#endif
}

static void MotionMergeResourceLayout(MotionResource &resource,
	const ttstr &objectName, const ttstr &motionName,
	const ttstr &contextObjectName, const ttstr &contextMotionName,
	const MotionLayoutBounds &bounds, tjs_int renderMethodItemIndex = -1)
{
	MotionMergeResourceNamedLayout(resource, objectName, objectName, motionName,
		contextObjectName, contextMotionName, bounds, renderMethodItemIndex);
}

static void MotionMergeResourceLayoutAliases(MotionResource &resource,
	const std::string &layoutName, const ttstr &objectName, const ttstr &motionName,
	const ttstr &contextObjectName, const ttstr &contextMotionName,
	const MotionLayoutBounds &bounds, tjs_int renderMethodItemIndex = -1)
{
#ifdef __EMSCRIPTEN__
	bool perfEnabled = gMotionLayoutBuildPerf != nullptr;
	tjs_uint64 perfStart = perfEnabled ? TVPGetTickCount() : 0;
#endif
	std::vector<std::string> aliases = MotionLayoutNameAliases(layoutName);
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfAfterBuild = perfEnabled ? TVPGetTickCount() : perfStart;
#endif
	for (size_t i = 0; i < aliases.size(); i++) {
		MotionMergeResourceNamedLayout(resource, MotionString(aliases[i]),
			objectName, motionName, contextObjectName, contextMotionName,
			bounds, i == 0 ? renderMethodItemIndex : -1);
	}
#ifdef __EMSCRIPTEN__
	if (perfEnabled) {
		tjs_uint64 perfEnd = TVPGetTickCount();
		MotionLayoutBuildPerfCounters &perf = *gMotionLayoutBuildPerf;
		perf.aliasCalls++;
		perf.aliasNames += aliases.size();
		perf.aliasTotalMs += perfEnd - perfStart;
		perf.aliasBuildMs += perfAfterBuild - perfStart;
		MotionLayoutPerfNoteMax(perfEnd - perfStart, layoutName,
			perf.maxAliasMs, perf.maxAliasLabel);
	}
#endif
}

static void MotionAddRenderMethodIRItem(MotionResource &resource,
	const MotionRenderMethodItem &item)
{
	if (item.label.GetLen() <= 0) return;
#ifdef __EMSCRIPTEN__
	bool perfEnabled = gMotionLayoutBuildPerf != nullptr;
	tjs_uint64 perfStart = perfEnabled ? TVPGetTickCount() : 0;
#endif
	MotionRenderMethodItem stored = item;
	if (stored.surfaces.empty()) {
		MotionRefreshRenderMethodItemSurface(stored);
	}
	resource.renderMethodIR.push_back(stored);
	MotionRenderMethodSemanticsDiag &diag = resource.motionRenderMethodSemantics;
	diag.metadataSurveyFrameCount++;
	if (stored.hasControlPoints) diag.metadataSurveyBpCandidateCount++;
	switch (stored.type) {
		case 1: diag.drawItemType1Count++; break;
		case 2: diag.drawItemType2Count++; break;
		case 3: diag.layoutMergeCount++; break;
	}
	if (stored.isFromNestedMotion) diag.nestedMotionRenderPassCount++;
#ifdef __EMSCRIPTEN__
	if (perfEnabled) {
		tjs_uint64 perfEnd = TVPGetTickCount();
		MotionLayoutBuildPerfCounters &perf = *gMotionLayoutBuildPerf;
		perf.renderItemCalls++;
		perf.renderItemMs += perfEnd - perfStart;
	}
#endif
}

static void MotionFinalizeRenderMethodIR(MotionResource &resource)
{
	MotionRenderMethodSemanticsDiag &diag = resource.motionRenderMethodSemantics;
	diag.probed = 1;
	diag.stackBuilt = !resource.renderMethodIR.empty();
	diag.totalItems = (tjs_int)resource.renderMethodIR.size();
	diag.linkedLayoutCount = 0;
	diag.surfaceCount = 0;
	diag.itemWithSurfaceCount = 0;
	diag.m2SingleSurfaceItemCount = 0;
	diag.m3MultiSurfaceItemCount = 0;
	for (size_t i = 0; i < resource.renderMethodIR.size(); i++) {
		tjs_int surfaces = (tjs_int)resource.renderMethodIR[i].surfaces.size();
		diag.surfaceCount += surfaces;
		if (surfaces > 0) diag.itemWithSurfaceCount++;
		if (resource.renderMethodIR[i].m2ProgressSurface && surfaces == 1) {
			diag.m2SingleSurfaceItemCount++;
		}
		if (resource.renderMethodIR[i].m2ProgressSurface && surfaces > 1) {
			diag.m3MultiSurfaceItemCount++;
		}
		if (resource.renderMethodIR[i].linkedToLayout &&
			resource.renderMethodIR[i].layoutIndex >= 0) {
			diag.linkedLayoutCount++;
		}
	}

	if (!resource.renderMethodIR.empty()) {
		const MotionRenderMethodItem &first = resource.renderMethodIR[0];
		diag.firstSampleType = ttstr((tjs_int)first.type);
		diag.firstSampleLabel = first.label;
		diag.firstSampleOriginX = first.originX;
		diag.firstSampleOriginY = first.originY;
		diag.firstSampleWidth = first.width;
		diag.firstSampleHeight = first.height;
		diag.firstSampleOpacity = first.opacity;
		diag.firstSampleHasControlPoints = first.hasControlPoints;
		diag.firstSampleSurfaceCount = (tjs_int)first.surfaces.size();
		if (!first.surfaces.empty()) {
			const MotionRenderSurface &surface = first.surfaces[0];
			diag.firstSampleSurfaceOriginX = surface.originX;
			diag.firstSampleSurfaceOriginY = surface.originY;
			diag.firstSampleSurfaceOriginW = surface.originW;
			diag.firstSampleSurfaceOriginH = surface.originH;
			diag.firstSampleSurfaceOpa = surface.opa;
			diag.firstSampleSurfaceType = surface.type;
			diag.firstSampleSurfaceMat00 = surface.matTrans.m[0];
			diag.firstSampleSurfaceMat11 = surface.matTrans.m[5];
			diag.firstSampleSurfaceHasStencil = surface.hasStencil;
			diag.firstSampleSurfaceCurrCoordz = surface.currCoordz;
		}
	}

	if (diag.drawItemType1Count == 0) {
		if (diag.metadataSurveyBpCandidateCount > 0) {
			diag.skipReason = "bp-present-in-metadata-but-not-in-type1-ir-items";
		} else if (diag.metadataSurveyFrameCount == 0) {
			diag.skipReason = "no-frames-with-content";
		} else {
			diag.skipReason = "no-bp-in-scanned-frames";
		}
	}

	if (resource.renderMethodIR.empty() && diag.metadataSurveyFrameCount == 0) {
		diag.skipReason = "no-frames-with-content-for-render-method-ir";
	}
}

static bool MotionCollectNamedLayoutBounds(psb_t &psb,
	const psb_objects_t *objects,
	const std::map<std::string, MotionSourceIconInfo> &sourceMap,
	const std::string &objectName,
	const std::string &motionName,
	const MotionLayoutTransform &base,
	const std::string &layoutScope,
	MotionResource &resource,
	const ttstr &contextObjectName,
	const ttstr &contextMotionName,
	int canvasW,
	int canvasH,
	std::set<std::string> &motionStack,
	int depth,
	MotionLayoutBounds &bounds,
	tjs_real tick = 0.0,
	MotionTimeSemanticsDiag *timeDiag = nullptr,
	MotionLimitSemanticsDiag *limitDiag = nullptr,
	const MotionSurfaceStackState *inheritedStack = nullptr,
	const std::map<std::string, tjs_real> *variables = nullptr,
	const std::vector<tjs_int> *priorityPathPrefix = nullptr);

static std::vector<ttstr> MotionChooseLayoutMotions(const MotionObjectInfo &object);

static bool MotionCollectFrameLayoutBounds(psb_t &psb, const psb_objects_t *objects,
	const std::map<std::string, MotionSourceIconInfo> &sourceMap,
	const MotionFrameLayoutState &frame, const MotionLayoutTransform &transform,
	MotionResource &resource, const ttstr &contextObjectName,
	const ttstr &contextMotionName, int canvasW, int canvasH,
	const std::string &layoutScope, std::set<std::string> &motionStack,
	int depth, MotionLayoutBounds &bounds,
	tjs_real tick,
	MotionTimeSemanticsDiag *timeDiag,
	MotionLimitSemanticsDiag *limitDiag,
	const MotionSurfaceStackState *inheritedStack = nullptr,
	const std::map<std::string, tjs_real> *variables = nullptr,
	const std::vector<tjs_int> *priorityPathPrefix = nullptr)
{
	if (!frame.hasContent || frame.src.empty() || frame.opacity <= 0.001) return false;
	if (MotionStartsWith(frame.src, "src/")) {
		std::map<std::string, MotionSourceIconInfo>::const_iterator it = sourceMap.find(frame.src);
		if (it == sourceMap.end()) return false;
		const MotionSourceIconInfo &icon = it->second;
		if (!icon.hasPixel || !icon.decoderSupported) return false;
		if (bounds.sourceKey.empty()) bounds.sourceKey = frame.src;
		bool fullScreenNoCoord = !frame.hasCoord && canvasW > 0 && canvasH > 0 &&
			icon.width >= canvasW && icon.height >= canvasH;
		if (fullScreenNoCoord) {
			MotionAddLayoutBounds(bounds, 0.0, 0.0,
				(tjs_real)canvasW, (tjs_real)canvasH,
				transform.opacity);
			return true;
		}
		tjs_real aMinX, aMinY, aMaxX, aMaxY;
		tjs_real iconOriginX = transform.width > 0.0 ? transform.originX : icon.originX;
		tjs_real iconOriginY = transform.height > 0.0 ? transform.originY : icon.originY;
		MotionAffine2D drawAffine = MotionBuildIconDrawAffine(
			transform.affine,
			transform.ox, transform.oy,
			icon.width, icon.height,
			iconOriginX, iconOriginY);
		MotionCollectAffineQuadCorners(drawAffine, aMinX, aMinY, aMaxX, aMaxY);
		MotionAddLayoutBounds(bounds, aMinX, aMinY,
			aMaxX - aMinX, aMaxY - aMinY,
			transform.opacity);
		if (!bounds.hasAffine) {
			bounds.hasAffine = true;
			bounds.angle = transform.angle;
			bounds.sx = transform.sx;
			bounds.sy = transform.sy;
			bounds.ox = transform.ox;
			bounds.oy = transform.oy;
			bounds.iconOriginX = iconOriginX;
			bounds.iconOriginY = iconOriginY;
			bounds.iconWidth = icon.width;
			bounds.iconHeight = icon.height;
			bounds.accumX = transform.x;
			bounds.accumY = transform.y;
			bounds.accumScaleX = transform.scaleX;
			bounds.accumScaleY = transform.scaleY;
			bounds.affine = drawAffine;
		}
		if (frame.hasMeshBP && !bounds.hasControlPoints) {
			bounds.hasControlPoints = true;
			for (size_t k = 0; k < 32; k++) bounds.controlPts[k] = frame.bp[k];
		}
		return true;
	}
	if (frame.src == "shape/rect" || frame.src == "shape/circle") {
		const tjs_real shapeUnit = 16.0;
		tjs_real width = std::abs(transform.scaleX) * shapeUnit;
		tjs_real height = std::abs(transform.scaleY) * shapeUnit;
		MotionAddLayoutBounds(bounds,
			transform.x - width / 2.0,
			transform.y - height / 2.0,
			width, height, transform.opacity);
		return true;
	}
	if (MotionStartsWith(frame.src, "motion/")) {
		std::vector<std::string> parts = MotionSplitPath(frame.src);
		if (parts.size() < 3) return false;
		tjs_real childTick = tick + frame.timeOffset;
		const MotionInfo *childMotionInfo = nullptr;
		for (size_t m = 0; m < resource.motions.size(); m++) {
			if (MotionNameMatches(resource.motions[m].objectName, MotionString(parts[1])) &&
				MotionNameMatches(resource.motions[m].name, MotionString(parts[2]))) {
				childMotionInfo = &resource.motions[m];
				break;
			}
		}
		std::string childLayoutScope = layoutScope;
		std::vector<std::string> childScopeParts = MotionSplitPath(childLayoutScope);
		bool childScopeAlreadyAtObject =
			!childScopeParts.empty() && childScopeParts.back() == parts[1];
		if (!childLayoutScope.empty() && !childScopeAlreadyAtObject) {
			childLayoutScope = MotionJoinLayoutPath(childLayoutScope, parts[1]);
		} else if (childLayoutScope.empty() && !parts[1].empty()) {
			childLayoutScope = parts[1];
		}
		std::map<std::string, tjs_real> childVariables =
			MotionBuildNestedVariableValues(
				variables, parts[1], parts[2], childMotionInfo, childLayoutScope);
		const std::map<std::string, tjs_real> *childVariablePtr =
			variables ? &childVariables : nullptr;
		if (timeDiag) {
			timeDiag->nestedMotionCount++;
			if (frame.timeOffset != 0.0) timeDiag->timeOffsetNonzeroCount++;
			if (timeDiag->firstNestedMotionPath.empty()) {
				timeDiag->firstNestedMotionPath = frame.src;
				timeDiag->parentTickSample = tick;
				timeDiag->childEvalTickSample = childTick;
				timeDiag->timeOffsetSample = frame.timeOffset;
			}
			timeDiag->timeOffsetConsumed = true;
		}
		if (limitDiag) {
			limitDiag->childLimPropCount++;
			if (limitDiag->sampleChildLimOriginX == 0.0 && limitDiag->sampleChildLimOriginY == 0.0 &&
				(transform.originX != 0.0 || transform.originY != 0.0 ||
				 transform.width > 0.0 || transform.height > 0.0)) {
				limitDiag->sampleChildLimOriginX = transform.originX;
				limitDiag->sampleChildLimOriginY = transform.originY;
				limitDiag->sampleChildLimWidth = transform.width;
				limitDiag->sampleChildLimHeight = transform.height;
			}
		}
		MotionLayoutBounds childBounds;
		if (MotionCollectNamedLayoutBounds(psb, objects, sourceMap,
			parts[1], parts[2], transform, childLayoutScope, resource,
			contextObjectName, contextMotionName, canvasW, canvasH,
			motionStack, depth + 1, childBounds, childTick, timeDiag, limitDiag,
			inheritedStack, childVariablePtr, priorityPathPrefix)) {
			MotionMergeLayoutBounds(bounds, childBounds);
			return true;
		}
	}
	return false;
}
static bool MotionCollectMotionLayoutBounds(psb_t &psb, const psb_objects_t *objects,
	const psb_objects_t *motion,
	const std::map<std::string, MotionSourceIconInfo> &sourceMap,
	const MotionLayoutTransform &base, const std::string &layoutScope,
	MotionResource &resource,
	const std::string &objectName,
	const ttstr &contextObjectName, const ttstr &contextMotionName,
	const std::string &motionName, int canvasW, int canvasH,
	std::set<std::string> &motionStack, int depth, MotionLayoutBounds &bounds,
	tjs_real tick,
	MotionTimeSemanticsDiag *timeDiag,
	MotionLimitSemanticsDiag *limitDiag,
	const MotionSurfaceStackState *inheritedStack = nullptr,
	const std::map<std::string, tjs_real> *variables = nullptr,
	const std::vector<tjs_int> *priorityPathPrefix = nullptr)
{
	if (!motion || depth > 16) return false;
#ifdef __EMSCRIPTEN__
	bool perfEnabled = gMotionLayoutBuildPerf != nullptr;
	tjs_uint64 perfStart = perfEnabled ? TVPGetTickCount() : 0;
	tjs_uint64 perfAfterReadLayers = perfStart;
	tjs_uint64 perfAfterFindMotion = perfStart;
	tjs_uint64 perfAfterCollectNodes = perfStart;
	tjs_uint64 perfAfterSetup = perfStart;
	tjs_uint64 perfAfterFirstPass = perfStart;
	tjs_uint64 perfAfterVariantPass = perfStart;
	std::string perfLabel;
	if (perfEnabled) {
		perfLabel = objectName + "/" + motionName + " depth=" +
			std::to_string(depth);
	}
#endif
	std::vector<unsigned char*> layers;
	if (!MotionPsbReadCollection(psb, motion->get_data("layer"), layers)) return false;
#ifdef __EMSCRIPTEN__
	if (perfEnabled) perfAfterReadLayers = TVPGetTickCount();
#endif

	tjs_real effectiveTick = tick;
	if (timeDiag && depth == 0) {
		timeDiag->tickProbeCount++;
		timeDiag->requestedTickSample = tick;
	}

	MotionInfo *mi = nullptr;
	if (!objectName.empty()) {
		mi = MotionFindMotionInfoForObjectMotion(resource,
			MotionString(objectName), motionName);
	}
	for (size_t m = 0; m < resource.motions.size(); m++) {
		if (mi) break;
		if (MotionNameMatches(resource.motions[m].name, MotionString(motionName)) &&
			MotionNameMatches(resource.motions[m].objectName, contextObjectName)) {
			mi = &resource.motions[m];
			break;
		}
	}
	if (!mi) {
		for (size_t m = 0; m < resource.motions.size(); m++) {
			if (MotionNameMatches(resource.motions[m].name, MotionString(motionName))) {
				mi = &resource.motions[m];
				break;
			}
		}
	}
#ifdef __EMSCRIPTEN__
	if (MotionClickBurstTraceActive() &&
		MotionTraceInterestingSettingObject(objectName)) {
		char buf[512];
		std::snprintf(buf, sizeof(buf),
			"collect-motion object=%s motion=%s context=%s/%s selected=%s/%s parameterIdx=%d vars=%zu",
			objectName.c_str(), motionName.c_str(),
			contextObjectName.AsNarrowStdString().c_str(),
			contextMotionName.AsNarrowStdString().c_str(),
			mi ? mi->objectName.AsNarrowStdString().c_str() : "(null)",
			mi ? mi->name.AsNarrowStdString().c_str() : "(null)",
			mi ? (int)mi->parameterIdx : -1,
			mi ? mi->variables.size() : (size_t)0);
		MotionClickBurstTraceLog("layout", buf);
	}
#endif

	if (mi && mi->selfSyncTime > 0.0 && effectiveTick > mi->selfSyncTime) {
		effectiveTick = mi->selfSyncTime;
		if (timeDiag) timeDiag->selfSyncTimeClamped = true;
	}
	if (timeDiag && depth == 0) {
		timeDiag->effectiveTickSample = effectiveTick;
		if (mi) {
			timeDiag->metadataNestedMotionRefCount += mi->motionRefCount;
			timeDiag->metadataTimeOffsetNonzeroCount += mi->timeOffsetNonzeroRefCount;
		}
	}
#ifdef __EMSCRIPTEN__
	if (perfEnabled) perfAfterFindMotion = TVPGetTickCount();
#endif

	std::vector<MotionNodeLayoutEntry> nodes;
	for (size_t i = 0; i < layers.size(); i++) {
		MotionCollectLayoutNodes(psb, layers[i], -1, nodes, 0,
			mi ? mi->parameterIdx : -1);
	}
#ifdef __EMSCRIPTEN__
	if (perfEnabled) perfAfterCollectNodes = TVPGetTickCount();
#endif
	if (nodes.empty()) {
#ifdef __EMSCRIPTEN__
		if (perfEnabled) {
			tjs_uint64 perfEnd = TVPGetTickCount();
			MotionLayoutBuildPerfCounters &perf = *gMotionLayoutBuildPerf;
			perf.collectMotionCalls++;
			perf.collectLayerCount += layers.size();
			perf.collectMotionMs += perfEnd - perfStart;
			perf.collectReadLayersMs += perfAfterReadLayers - perfStart;
			perf.collectFindMotionMs += perfAfterFindMotion - perfAfterReadLayers;
			perf.collectNodesMs += perfAfterCollectNodes - perfAfterFindMotion;
			MotionLayoutPerfNoteMax(perfEnd - perfStart, perfLabel,
				perf.maxCollectMotionMs, perf.maxCollectMotionLabel);
		}
#endif
		return false;
	}

		std::vector<MotionFrameLayoutState> frames(nodes.size());
		std::vector<std::vector<MotionFrameLayoutState> > frameVariants(nodes.size());
		std::vector<MotionLayoutTransform> transforms(nodes.size());
		std::vector<MotionSurfaceStackState> stackStates(nodes.size());
	std::vector<std::string> layoutPaths(nodes.size());
	std::vector<tjs_int> renderMethodItemIndices(nodes.size(), -1);
	std::vector<tjs_int> drawPriority;
	MotionReadMotionPriority(psb, motion, drawPriority);
	std::vector<tjs_int> drawPriorityOrder(nodes.size(), -1);
	for (size_t pi = 0; pi < drawPriority.size(); pi++) {
		tjs_int nodeIndex = drawPriority[pi];
		if (nodeIndex >= 0 && nodeIndex < (tjs_int)drawPriorityOrder.size() &&
			drawPriorityOrder[(size_t)nodeIndex] < 0) {
			drawPriorityOrder[(size_t)nodeIndex] = (tjs_int)pi;
		}
	}
	bool preferFinalFrame = MotionLayoutPrefersFinalFrame(motionName);
	bool collectStateVariants =
		MotionLayoutCollectsStateVariants(motionName) &&
		(!variables || variables->empty());
	bool twoFrameHoldApplied = false;
#ifdef __EMSCRIPTEN__
	if (perfEnabled) perfAfterSetup = TVPGetTickCount();
#endif
	for (size_t i = 0; i < nodes.size(); i++) {
		MotionLayoutTransform parent = base;
		if (nodes[i].parent >= 0 && nodes[i].parent < (int)i) {
			parent = transforms[(size_t)nodes[i].parent];
		}
		std::string parentPath = layoutScope;
		if (nodes[i].parent >= 0 && nodes[i].parent < (int)i) {
			parentPath = layoutPaths[(size_t)nodes[i].parent];
		}
		layoutPaths[i] = MotionJoinLayoutPath(parentPath, nodes[i].label);
		psb_objects_t *node = nullptr;
		if (MotionUnpackObject(psb, nodes[i].data, node)) {
				tjs_real nodeTimeOffset = 0.0;
				tjs_int nodeFrameCount = 0;
				tjs_int nodeType0Count = 0;
				MotionSelectNodeLayoutFrame(psb, node, frames[i], preferFinalFrame,
					effectiveTick, &nodeTimeOffset, &nodeFrameCount, &nodeType0Count,
					&parent, limitDiag, mi, nodes[i].parameterIdx, variables,
					&layoutPaths[i]);

			if (!twoFrameHoldApplied && effectiveTick > 0.0001 && !preferFinalFrame && mi &&
				nodeFrameCount == 2 && nodeType0Count == 1) {
				tjs_real firstFrameTime = 0.0;
				tjs_int typeAt0 = -1;
				tjs_int typeAt1 = -1;
				MotionReadFirstFrameTimeAndTypes(psb, node, firstFrameTime, typeAt0, typeAt1);
				if (typeAt0 == 2 && typeAt1 == 0) {
					effectiveTick = firstFrameTime;
					twoFrameHoldApplied = true;
					if (timeDiag) {
						timeDiag->twoFrameSpecialApplied = true;
						timeDiag->twoFrameHoldTickSample = firstFrameTime;
						if (depth == 0) timeDiag->effectiveTickSample = effectiveTick;
					}
						for (size_t fi = 0; fi < frames.size(); fi++) {
							psb_objects_t *node2 = nullptr;
							if (MotionUnpackObject(psb, nodes[fi].data, node2)) {
								tjs_real dummyOffset = 0.0;
								MotionSelectNodeLayoutFrame(psb, node2, frames[fi], false,
									effectiveTick, &dummyOffset, nullptr, nullptr,
									nullptr, nullptr, mi, nodes[fi].parameterIdx, variables,
									&layoutPaths[fi]);
								delete node2;
							}
						}
				}
			}
			if (collectStateVariants) MotionReadNodeLayoutFrames(psb, node, frameVariants[i]);
			delete node;
		}

		if (frameVariants[i].empty() && frames[i].hasContent) {
			frameVariants[i].push_back(frames[i]);
		}
		if (limitDiag && limitDiag->sampleParentLimWidth == 0.0 &&
			limitDiag->sampleParentLimHeight == 0.0 &&
			(parent.originX != 0.0 || parent.originY != 0.0 ||
			 parent.width > 0.0 || parent.height > 0.0)) {
			limitDiag->sampleParentLimOriginX = parent.originX;
			limitDiag->sampleParentLimOriginY = parent.originY;
			limitDiag->sampleParentLimWidth = parent.width;
			limitDiag->sampleParentLimHeight = parent.height;
		}
		if (limitDiag) {
			if (nodes[i].type == 12) limitDiag->type12LimInheritCount++;
			if (nodes[i].type == 7) limitDiag->nondrawableLimInheritCount++;
		}
		bool haveNanInfSample = false;
		int nanC = 0, infC = 0;
		if (frames[i].hasContent && frames[i].hasCoord) {
			tjs_real befX = frames[i].coordX;
			tjs_real befY = frames[i].coordY;
			nanC = 0; infC = 0;
			MotionNormalizeNaNInfCoord(frames[i].coordX, frames[i].coordY,
				parent.originX, parent.originY, parent.width, parent.height,
				false, nanC, infC);
			if (limitDiag) {
				limitDiag->nanCoordCount += nanC;
				limitDiag->infCoordCount += infC;
				limitDiag->singleFrameNormalizeCount++;
				limitDiag->metadataSurveyFrameCount++;
				if (!haveNanInfSample && (nanC > 0 || infC > 0)) {
					limitDiag->sampleCoordBeforeX = befX;
					limitDiag->sampleCoordBeforeY = befY;
					limitDiag->sampleCoordAfterX = frames[i].coordX;
					limitDiag->sampleCoordAfterY = frames[i].coordY;
					haveNanInfSample = true;
				}
			}
		}
		for (size_t vi = 0; vi < frameVariants[i].size(); vi++) {
			if (frameVariants[i][vi].hasContent && frameVariants[i][vi].hasCoord) {
				tjs_real befVX = frameVariants[i][vi].coordX;
				tjs_real befVY = frameVariants[i][vi].coordY;
				nanC = 0; infC = 0;
				MotionNormalizeNaNInfCoord(frameVariants[i][vi].coordX,
					frameVariants[i][vi].coordY,
					parent.originX, parent.originY, parent.width, parent.height,
					false, nanC, infC);
				if (limitDiag) {
					limitDiag->nanCoordCount += nanC;
					limitDiag->infCoordCount += infC;
					limitDiag->singleFrameNormalizeCount++;
					if (!frames[i].hasCoord) limitDiag->metadataSurveyFrameCount++;
					if (!haveNanInfSample && (nanC > 0 || infC > 0)) {
						limitDiag->sampleCoordBeforeX = befVX;
						limitDiag->sampleCoordBeforeY = befVY;
						limitDiag->sampleCoordAfterX = frameVariants[i][vi].coordX;
						limitDiag->sampleCoordAfterY = frameVariants[i][vi].coordY;
						haveNanInfSample = true;
					}
				}
			}
		}
		transforms[i] = frames[i].hasContent ?
			MotionApplyLayoutFrame(parent, frames[i]) : parent;
		if (frames[i].hasContent && MotionStartsWith(frames[i].src, "src/")) {
			std::map<std::string, MotionSourceIconInfo>::const_iterator iconIt =
				sourceMap.find(frames[i].src);
			if (iconIt != sourceMap.end()) {
				transforms[i].originX = iconIt->second.originX;
				transforms[i].originY = iconIt->second.originY;
				transforms[i].width = iconIt->second.width;
				transforms[i].height = iconIt->second.height;
				if (limitDiag && limitDiag->sampleFrameOriginX == 0.0 &&
					limitDiag->sampleFrameOriginY == 0.0) {
					limitDiag->sampleFrameOriginX = iconIt->second.originX;
					limitDiag->sampleFrameOriginY = iconIt->second.originY;
					limitDiag->sampleFrameWidth = iconIt->second.width;
					limitDiag->sampleFrameHeight = iconIt->second.height;
				}
			}
		}
		MotionSurfaceStackState parentStack;
		if (nodes[i].parent >= 0 && nodes[i].parent < (int)i) {
			parentStack = stackStates[(size_t)nodes[i].parent];
		} else if (inheritedStack) {
			parentStack = *inheritedStack;
		}
		stackStates[i] = parentStack;

		if (frames[i].hasContent) {
			MotionRenderMethodItem item;
			item.label = layoutPaths[i].empty() ?
				MotionString(nodes[i].label) : MotionString(layoutPaths[i]);
			item.srcKey = frames[i].src;
			item.opacity = transforms[i].opacity;
			item.hasControlPoints = frames[i].hasMeshBP;
			item.depth = depth;
			item.sourceNodeIndex = (tjs_int)i;
			item.drawPriority = drawPriorityOrder[i] >= 0 ?
				drawPriorityOrder[i] : (tjs_int)(drawPriority.size() + i);
			item.drawPriorityPath =
				MotionBuildDrawPriorityPath(priorityPathPrefix, item.drawPriority);
			item.isFromLayout =
				(nodes[i].type == 12 || nodes[i].type == 7 ||
				 frames[i].src.empty() || frames[i].src == "layout");
			item.isFromNestedMotion = MotionStartsWith(frames[i].src, "motion/");
			item.originX = transforms[i].originX;
			item.originY = transforms[i].originY;
			item.width = transforms[i].width;
			item.height = transforms[i].height;
			item.affineValid = false;
			if (MotionStartsWith(frames[i].src, "src/") && frames[i].hasMeshBP) {
				item.type = 1;
				for (size_t k = 0; k < 32; k++) item.controlPts[k] = frames[i].bp[k];
			} else if (MotionStartsWith(frames[i].src, "src/")) {
				item.type = 2;
			} else {
				item.type = 3;
			}
			if (MotionStartsWith(frames[i].src, "src/")) {
				std::map<std::string, MotionSourceIconInfo>::const_iterator iconIt =
					sourceMap.find(frames[i].src);
				if (iconIt != sourceMap.end()) {
					const MotionSourceIconInfo &icon = iconIt->second;
					tjs_real iconOriginX =
						transforms[i].width > 0.0 ? transforms[i].originX : icon.originX;
					tjs_real iconOriginY =
						transforms[i].height > 0.0 ? transforms[i].originY : icon.originY;
					item.originX = iconOriginX;
					item.originY = iconOriginY;
					item.width = icon.width;
					item.height = icon.height;
					item.affineValid = true;
					item.affine = MotionBuildIconDrawAffine(
						transforms[i].affine,
						transforms[i].ox, transforms[i].oy,
						icon.width, icon.height,
						iconOriginX, iconOriginY);
				}
			}
			stackStates[i] = MotionAdvanceSurfaceStackForFrame(
				parentStack, frames[i], nodes[i].type,
				item.originX, item.originY, item.width, item.height,
				parent.originX, parent.originY, parent.width, parent.height,
				item.label, &resource.motionRenderMethodSemantics);
			// krkrsdl3 children consume the current node's resolved lim region after
			// layout merge/demux, not the pre-progress parent transform snapshot.
			if (stackStates[i].valid) {
				transforms[i].originX = stackStates[i].originX;
				transforms[i].originY = stackStates[i].originY;
				transforms[i].width = stackStates[i].width;
				transforms[i].height = stackStates[i].height;
			}
			if (stackStates[i].unsupportedDemux) {
				item.m2UnsupportedDemux = true;
			} else if (stackStates[i].valid) {
				item.surfaces = stackStates[i].surfaces;
				if (item.surfaces.empty()) item.surfaces.push_back(stackStates[i].lastSurface);
				item.m2ProgressSurface = true;
			}
			renderMethodItemIndices[i] = (tjs_int)resource.renderMethodIR.size();
			MotionAddRenderMethodIRItem(resource, item);
		}
	}
#ifdef __EMSCRIPTEN__
	if (perfEnabled) perfAfterFirstPass = TVPGetTickCount();
#endif

	for (size_t i = 0; i < frameVariants.size(); i++) {
		if (frameVariants[i].empty()) continue;
		tjs_int itemDrawPriority = drawPriorityOrder[i] >= 0 ?
			drawPriorityOrder[i] : (tjs_int)(drawPriority.size() + i);
		std::vector<tjs_int> itemPriorityPath =
			MotionBuildDrawPriorityPath(priorityPathPrefix, itemDrawPriority);
		MotionLayoutTransform parent = base;
		MotionSurfaceStackState parentStack;
		const MotionSurfaceStackState *parentStackPtr = nullptr;
		if (nodes[i].parent >= 0 && nodes[i].parent < (int)i) {
			parent = transforms[(size_t)nodes[i].parent];
			parentStack = stackStates[(size_t)nodes[i].parent];
			parentStackPtr = &parentStack;
		} else if (inheritedStack) {
			parentStack = *inheritedStack;
			parentStackPtr = &parentStack;
		}
		for (size_t vi = 0; vi < frameVariants[i].size(); vi++) {
			const MotionFrameLayoutState &frame = frameVariants[i][vi];
			MotionLayoutTransform transform = MotionApplyLayoutFrame(parent, frame);
			if (frame.hasContent && MotionStartsWith(frame.src, "src/")) {
				std::map<std::string, MotionSourceIconInfo>::const_iterator iconIt =
					sourceMap.find(frame.src);
				if (iconIt != sourceMap.end()) {
					transform.originX = iconIt->second.originX;
					transform.originY = iconIt->second.originY;
					transform.width = iconIt->second.width;
					transform.height = iconIt->second.height;
				}
			}
			MotionLayoutBounds frameBounds;
			MotionSurfaceStackState nestedMotionStack;
			const MotionSurfaceStackState *frameInheritedStack =
				(nodes[i].parent < 0 && inheritedStack) ? inheritedStack : nullptr;
			if (frame.hasContent && MotionStartsWith(frame.src, "motion/")) {
				nestedMotionStack = MotionAdvanceSurfaceStackForFrame(
					parentStackPtr ? *parentStackPtr : MotionSurfaceStackState(),
					frame, nodes[i].type,
					transform.originX, transform.originY,
					transform.width, transform.height,
					parent.originX, parent.originY, parent.width, parent.height,
					layoutPaths[i].empty() ?
						MotionString(nodes[i].label) : MotionString(layoutPaths[i]),
					nullptr);
				frameInheritedStack = &nestedMotionStack;
			}
					if (MotionCollectFrameLayoutBounds(psb, objects, sourceMap,
						frame, transform, resource, contextObjectName, contextMotionName,
						canvasW, canvasH, layoutPaths[i], motionStack, depth, frameBounds,
						effectiveTick, timeDiag, limitDiag, frameInheritedStack, variables,
						&itemPriorityPath)) {
					MotionMergeLayoutBounds(bounds, frameBounds);
				if (!nodes[i].label.empty() && !layoutPaths[i].empty()) {
					MotionMergeResourceLayoutAliases(resource,
						layoutPaths[i], MotionString(nodes[i].label),
						MotionString(motionName), contextObjectName,
						contextMotionName, frameBounds,
						vi == 0 ? renderMethodItemIndices[i] : -1);
				}
			}
		}
	}
#ifdef __EMSCRIPTEN__
	if (perfEnabled) {
		perfAfterVariantPass = TVPGetTickCount();
		size_t variantCount = 0;
		for (size_t i = 0; i < frameVariants.size(); i++) {
			variantCount += frameVariants[i].size();
		}
		MotionLayoutBuildPerfCounters &perf = *gMotionLayoutBuildPerf;
		perf.collectMotionCalls++;
		perf.collectLayerCount += layers.size();
		perf.collectNodeCount += nodes.size();
		perf.collectFrameVariantCount += variantCount;
		perf.collectMotionMs += perfAfterVariantPass - perfStart;
		perf.collectReadLayersMs += perfAfterReadLayers - perfStart;
		perf.collectFindMotionMs += perfAfterFindMotion - perfAfterReadLayers;
		perf.collectNodesMs += perfAfterCollectNodes - perfAfterFindMotion;
		perf.collectSetupMs += perfAfterSetup - perfAfterCollectNodes;
		perf.collectFirstPassMs += perfAfterFirstPass - perfAfterSetup;
		perf.collectVariantPassMs += perfAfterVariantPass - perfAfterFirstPass;
		MotionLayoutPerfNoteMax(perfAfterVariantPass - perfStart, perfLabel,
			perf.maxCollectMotionMs, perf.maxCollectMotionLabel);
	}
#endif
	return bounds.valid;
}

static bool MotionCollectNamedLayoutBounds(psb_t &psb,
	const psb_objects_t *objects,
	const std::map<std::string, MotionSourceIconInfo> &sourceMap,
	const std::string &objectName,
	const std::string &motionName,
	const MotionLayoutTransform &base,
	const std::string &layoutScope,
	MotionResource &resource,
	const ttstr &contextObjectName,
	const ttstr &contextMotionName,
	int canvasW,
	int canvasH,
	std::set<std::string> &motionStack,
	int depth,
	MotionLayoutBounds &bounds,
	tjs_real tick,
	MotionTimeSemanticsDiag *timeDiag,
	MotionLimitSemanticsDiag *limitDiag,
	const MotionSurfaceStackState *inheritedStack,
	const std::map<std::string, tjs_real> *variables,
	const std::vector<tjs_int> *priorityPathPrefix)
{
	bounds = MotionLayoutBounds();
	if (!objects || depth > 16) return false;
	std::string key = objectName + "/" + motionName;
	if (motionStack.find(key) != motionStack.end()) return false;
#ifdef __EMSCRIPTEN__
	bool perfEnabled = gMotionLayoutBuildPerf != nullptr;
	tjs_uint64 perfStart = perfEnabled ? TVPGetTickCount() : 0;
	std::string perfLabel;
	if (perfEnabled) {
		perfLabel = key + " scope=" + layoutScope + " depth=" + std::to_string(depth);
	}
#endif
	motionStack.insert(key);

	psb_objects_t *object = nullptr;
	psb_objects_t *motionRoot = nullptr;
	psb_objects_t *motion = nullptr;
	bool ok = false;
	if (MotionUnpackObject(psb, objects->get_data(objectName), object)) {
		if (MotionUnpackObject(psb, object->get_data("motion"), motionRoot)) {
				if (MotionUnpackObject(psb, motionRoot->get_data(motionName), motion)) {
					ok = MotionCollectMotionLayoutBounds(psb, objects, motion, sourceMap,
						base, layoutScope, resource, objectName,
						contextObjectName, contextMotionName, motionName,
						canvasW, canvasH, motionStack, depth, bounds,
						tick, timeDiag, limitDiag, inheritedStack, variables,
						priorityPathPrefix);
				}
			}
	}

	if (motion) delete motion;
	if (motionRoot) delete motionRoot;
	if (object) delete object;
	motionStack.erase(key);

	if (ok && bounds.valid) {
		MotionMergeResourceLayout(resource,
			MotionString(objectName), MotionString(motionName),
			contextObjectName, contextMotionName, bounds);
		if (!layoutScope.empty()) {
			MotionMergeResourceLayoutAliases(resource,
				layoutScope, MotionString(objectName),
				MotionString(motionName), contextObjectName,
				contextMotionName, bounds);
		}
	}
#ifdef __EMSCRIPTEN__
	if (perfEnabled) {
		tjs_uint64 perfEnd = TVPGetTickCount();
		MotionLayoutBuildPerfCounters &perf = *gMotionLayoutBuildPerf;
		perf.collectNamedCalls++;
		perf.collectNamedMs += perfEnd - perfStart;
		MotionLayoutPerfNoteMax(perfEnd - perfStart, perfLabel,
			perf.maxCollectNamedMs, perf.maxCollectNamedLabel);
	}
#endif
	return ok;
}

static bool MotionObjectHasMotion(const MotionObjectInfo &object, const ttstr &name)
{
	for (size_t i = 0; i < object.motions.size(); i++) {
		if (object.motions[i].name == name) return true;
	}
	return false;
}

static bool MotionHasLayoutMotion(const std::vector<ttstr> &motions, const ttstr &name)
{
	for (size_t i = 0; i < motions.size(); i++) {
		if (motions[i] == name) return true;
	}
	return false;
}

static std::vector<ttstr> MotionChooseLayoutMotions(const MotionObjectInfo &object)
{
	std::vector<ttstr> result;
	static const tjs_char *preferred[] = {
		TJS_W("show"), TJS_W("show2"), TJS_W("normal"),
		TJS_W("menubar"), TJS_W("menubar_on"),
		TJS_W("on"), TJS_W("bt"), TJS_W("state"),
		TJS_W("over"), TJS_W("press"), TJS_W("off"), TJS_W("hide")
	};
	for (size_t i = 0; i < sizeof(preferred) / sizeof(preferred[0]); i++) {
		ttstr name(preferred[i]);
		if (MotionObjectHasMotion(object, name) && !MotionHasLayoutMotion(result, name)) {
			result.push_back(name);
		}
	}
	if (result.empty() && !object.motions.empty()) result.push_back(object.motions[0].name);
	return result;
}

static void MotionInitLayoutProbeResource(const MotionResource &source,
	MotionResource &probe)
{
	probe = MotionResource();
	probe.id = source.id;
	probe.storage = source.storage;
	probe.byteSize = source.byteSize;
	probe.opened = source.opened;
	probe.parsed = source.parsed;
	probe.isMotion = source.isMotion;
	probe.hasMetadata = source.hasMetadata;
	probe.spec = source.spec;
	probe.label = source.label;
	probe.chara = source.chara;
	probe.motion = source.motion;
	probe.width = source.width;
	probe.height = source.height;
	// Probe collection only needs parsed motion metadata as input.  Layouts,
	// render IR, PSB bytes, source icon dumps, and diagnostics are fresh outputs.
	probe.motions = source.motions;
	probe.variables = source.variables;
}

static void MotionBuildResourceLayouts(psb_t &psb, const psb_objects_t *root,
	const psb_objects_t *objects, MotionResource &resource)
{
#ifdef __EMSCRIPTEN__
	std::string perfPath = resource.storage.AsNarrowStdString();
	bool perfTrace = perfPath.find(".psb") != std::string::npos ||
		perfPath.find(".PSB") != std::string::npos;
	MotionLayoutBuildPerfCounters perfCounters;
	MotionLayoutBuildPerfCounters *perfPrevious = gMotionLayoutBuildPerf;
	if (perfTrace) gMotionLayoutBuildPerf = &perfCounters;
	tjs_uint64 perfBuildStart = perfTrace ? TVPGetTickCount() : 0;
#endif
	MotionLayoutMergeIndexState layoutIndexState;
	layoutIndexState.resource = &resource;
	layoutIndexState.entries.reserve(
		std::max<size_t>(resource.motions.size() * 96, (size_t)1024));
	MotionLayoutMergeIndexScope layoutIndexScope(&layoutIndexState);
	resource.layouts.clear();
	resource.sourceIcons.clear();
	resource.sourceAliasCount = 0;
	resource.timeSemanticsDiag = MotionTimeSemanticsDiag();
	resource.limitSemanticsDiag = MotionLimitSemanticsDiag();
	resource.motionRenderMethodSemantics = MotionRenderMethodSemanticsDiag();
	resource.renderMethodIR.clear();
	std::map<std::string, MotionSourceIconInfo> sourceMap;
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfSourceReadStart = perfTrace ? TVPGetTickCount() : 0;
#endif
	MotionReadSourceIcons(psb, root, sourceMap);
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfAfterSourceRead = perfTrace ? TVPGetTickCount() : 0;
	if (perfTrace) perfCounters.sourceReadMs += perfAfterSourceRead - perfSourceReadStart;
	tjs_uint64 perfSourceCanonStart = perfAfterSourceRead;
#endif
	std::set<std::string> canonicalSourceKeys;
	for (std::map<std::string, MotionSourceIconInfo>::const_iterator it = sourceMap.begin();
		it != sourceMap.end(); ++it) {
		std::string canonical = it->second.canonicalKey.empty() ?
			it->second.name : it->second.canonicalKey;
		if (canonical.empty()) canonical = it->first;
		if (canonicalSourceKeys.insert(canonical).second) {
			resource.sourceIcons.push_back(it->second);
		} else {
			resource.sourceAliasCount++;
		}
		if (it->first != canonical) resource.sourceAliasCount++;
	}
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfAfterSourceCanon = perfTrace ? TVPGetTickCount() : 0;
	if (perfTrace) perfCounters.sourceCanonMs += perfAfterSourceCanon - perfSourceCanonStart;
#endif
	if (sourceMap.empty()) {
#ifdef __EMSCRIPTEN__
		if (perfTrace) {
			tjs_uint64 perfEnd = TVPGetTickCount();
			fprintf(stderr,
				"[PERF] emote.layout.build path=%s ok=0 reason=no-source-icons source_read_ms=%llu source_canon_ms=%llu total_ms=%llu\n",
				perfPath.c_str(),
				(unsigned long long)perfCounters.sourceReadMs,
				(unsigned long long)perfCounters.sourceCanonMs,
				(unsigned long long)(perfEnd - perfBuildStart));
			gMotionLayoutBuildPerf = perfPrevious;
		}
#endif
		return;
	}

#ifdef __EMSCRIPTEN__
	tjs_uint64 perfOriginStart = perfTrace ? TVPGetTickCount() : 0;
#endif
	int screenOriginX = 0;
	int screenOriginY = 0;
	psb_objects_t *screen = nullptr;
	if (MotionUnpackObject(psb, root->get_data("screenSize"), screen)) {
		MotionPsbReadMemberInt(psb, screen, "originX", screenOriginX);
		MotionPsbReadMemberInt(psb, screen, "originY", screenOriginY);
		delete screen;
	}
	if (screenOriginX == 0 && screenOriginY == 0 &&
		resource.width > 0 && resource.height > 0) {
		for (std::map<std::string, MotionSourceIconInfo>::const_iterator it = sourceMap.begin();
			it != sourceMap.end(); ++it) {
			const MotionSourceIconInfo &icon = it->second;
			if (icon.width >= resource.width && icon.height >= resource.height &&
				(icon.originX != 0.0 || icon.originY != 0.0)) {
				screenOriginX = (int)std::lround(icon.originX);
				screenOriginY = (int)std::lround(icon.originY);
				break;
			}
		}
	}
	if (screenOriginX == 0 && screenOriginY == 0) {
		MotionInferScreenOriginFromSourceIcons(
			sourceMap, resource, screenOriginX, screenOriginY);
	}
	if ((screenOriginX == 0 || screenOriginY == 0) &&
		resource.width > 0 && resource.height > 0) {
		for (std::map<std::string, MotionSourceIconInfo>::const_iterator it = sourceMap.begin();
			it != sourceMap.end(); ++it) {
			const MotionSourceIconInfo &icon = it->second;
			if (icon.width < (tjs_real)resource.width ||
				icon.height < (tjs_real)resource.height) {
				continue;
			}
			if (screenOriginX == 0 && icon.originX != 0.0) {
				screenOriginX = (int)std::lround(icon.originX);
			}
			if (screenOriginY == 0 && icon.originY != 0.0) {
				screenOriginY = (int)std::lround(icon.originY);
			}
			if (screenOriginX != 0 && screenOriginY != 0) break;
		}
	}
	if (screenOriginX == 0 || screenOriginY == 0) {
		int inferredX = 0;
		int inferredY = 0;
		if (MotionInferScreenOriginFromSourceIcons(
			sourceMap, resource, inferredX, inferredY)) {
			if (screenOriginX == 0) screenOriginX = inferredX;
			if (screenOriginY == 0) screenOriginY = inferredY;
		}
	}
	MotionApplyMotionStyleCenterOriginFallback(
		sourceMap, resource, screenOriginX, screenOriginY);
	ttstr lowerLabel = resource.label.AsLowerCase();
	if (resource.width > 0 && resource.height > 0) {
		if (screenOriginX == 0 && lowerLabel == TJS_W("main")) {
			screenOriginX = resource.width / 2;
		}
		if (screenOriginY == 0 &&
			(lowerLabel == TJS_W("main") || lowerLabel == TJS_W("autoskip"))) {
			screenOriginY = resource.height / 2;
		}
	}

	MotionLayoutTransform base;
	base.x = screenOriginX;
	base.y = screenOriginY;
	base.originX = (tjs_real)screenOriginX;
	base.originY = (tjs_real)screenOriginY;
	base.width = (tjs_real)resource.width;
	base.height = (tjs_real)resource.height;
	base.affine = MotionAffineTranslate((tjs_real)screenOriginX, (tjs_real)screenOriginY);
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfAfterOrigin = perfTrace ? TVPGetTickCount() : 0;
	if (perfTrace) perfCounters.originMs += perfAfterOrigin - perfOriginStart;
	tjs_uint64 perfLoopStart = perfAfterOrigin;
#endif

	MotionTimeSemanticsDiag combinedTimeDiag;
	MotionLimitSemanticsDiag combinedLimitDiag;
	for (size_t i = 0; i < resource.objects.size(); i++) {
		const MotionObjectInfo &objectInfo = resource.objects[i];
		if (objectInfo.motions.empty()) continue;
		std::vector<ttstr> motionNames = MotionChooseLayoutMotions(objectInfo);
		for (size_t mi = 0; mi < motionNames.size(); mi++) {
			const ttstr &motionName = motionNames[mi];
			if (motionName.GetLen() <= 0) continue;
			MotionLayoutBounds bounds;
			std::set<std::string> motionStack;
			MotionTimeSemanticsDiag probeDiag;
			MotionLimitSemanticsDiag probeLimitDiag;
			tjs_real probeTick = 0.0;
			const MotionInfo *motionInfo = nullptr;
			for (size_t m = 0; m < objectInfo.motions.size(); m++) {
				if (MotionNameMatches(objectInfo.motions[m].name, motionName)) {
					motionInfo = &objectInfo.motions[m];
					break;
				}
			}
			if (motionInfo && motionInfo->syncTime > 0.0) {
				probeTick = std::max<tjs_real>(1.0,
					std::min<tjs_real>(motionInfo->syncTime, motionInfo->syncTime * 0.5));
			}
#ifdef __EMSCRIPTEN__
			std::string perfMotionLabel;
			if (perfTrace) {
				perfMotionLabel = objectInfo.name.AsNarrowStdString() + "/" +
					motionName.AsNarrowStdString();
			}
			tjs_uint64 perfPrimaryStart = perfTrace ? TVPGetTickCount() : 0;
#endif
			MotionCollectNamedLayoutBounds(psb, objects, sourceMap,
				objectInfo.name.AsNarrowStdString(),
				motionName.AsNarrowStdString(), base, std::string(), resource,
				objectInfo.name, motionName,
				resource.width, resource.height, motionStack, 0, bounds,
				0.0, &combinedTimeDiag, &combinedLimitDiag);
#ifdef __EMSCRIPTEN__
			tjs_uint64 perfAfterPrimary = perfTrace ? TVPGetTickCount() : 0;
			tjs_uint64 perfPrimaryMs = perfTrace ? perfAfterPrimary - perfPrimaryStart : 0;
			if (perfTrace) {
				perfCounters.primaryCalls++;
				perfCounters.primaryCollectMs += perfPrimaryMs;
				MotionLayoutPerfNoteMax(perfPrimaryMs, perfMotionLabel,
					perfCounters.maxPrimaryMs, perfCounters.maxPrimaryLabel);
			}
#endif
			if (probeTick > 0.0001) {
				MotionLayoutBounds probeBounds;
				std::set<std::string> probeStack;
#ifdef __EMSCRIPTEN__
				tjs_uint64 perfProbeCopyStart = perfTrace ? TVPGetTickCount() : 0;
#endif
				MotionResource probeResource;
				MotionInitLayoutProbeResource(resource, probeResource);
				MotionLayoutMergeIndexState probeIndexState;
				probeIndexState.resource = &probeResource;
				probeIndexState.entries.reserve((size_t)512);
#ifdef __EMSCRIPTEN__
				tjs_uint64 perfAfterProbeCopy = perfTrace ? TVPGetTickCount() : 0;
				tjs_uint64 perfProbeCopyMs = perfTrace ? perfAfterProbeCopy - perfProbeCopyStart : 0;
				if (perfTrace) {
					perfCounters.probeCalls++;
					perfCounters.probeCopyMs += perfProbeCopyMs;
					perfCounters.probeCopiedBytes += probeResource.bytes.size();
					perfCounters.probeCopiedLayouts += probeResource.layouts.size();
					MotionLayoutPerfNoteMax(perfProbeCopyMs, perfMotionLabel,
						perfCounters.maxProbeCopyMs, perfCounters.maxProbeCopyLabel);
				}
				tjs_uint64 perfProbeCollectStart = perfTrace ? TVPGetTickCount() : 0;
#endif
				{
					MotionLayoutMergeIndexScope probeIndexScope(&probeIndexState);
					MotionCollectNamedLayoutBounds(psb, objects, sourceMap,
						objectInfo.name.AsNarrowStdString(),
						motionName.AsNarrowStdString(), base, std::string(), probeResource,
						objectInfo.name, motionName,
						resource.width, resource.height, probeStack, 0, probeBounds,
						probeTick, &probeDiag, &probeLimitDiag);
				}
#ifdef __EMSCRIPTEN__
				tjs_uint64 perfAfterProbeCollect = perfTrace ? TVPGetTickCount() : 0;
				tjs_uint64 perfProbeCollectMs = perfTrace ? perfAfterProbeCollect - perfProbeCollectStart : 0;
				if (perfTrace) {
					perfCounters.probeCollectMs += perfProbeCollectMs;
					perfCounters.probeIndexEntries += probeIndexState.entries.size();
					perfCounters.probeIndexHits +=
						probeIndexState.writableHits + probeIndexState.readOnlyHits;
					perfCounters.probeIndexMisses += probeIndexState.writableMisses;
					perfCounters.probeIndexFallbacks += probeIndexState.fallbackScans;
					perfCounters.probeIndexMismatches += probeIndexState.mismatches;
					MotionLayoutPerfNoteMax(perfProbeCollectMs, perfMotionLabel,
						perfCounters.maxProbeCollectMs, perfCounters.maxProbeCollectLabel);
				}
				tjs_uint64 perfDiagMergeStart = perfTrace ? TVPGetTickCount() : 0;
#endif
				combinedTimeDiag.tickProbeCount += probeDiag.tickProbeCount;
				combinedTimeDiag.metadataNestedMotionRefCount += probeDiag.metadataNestedMotionRefCount;
				combinedTimeDiag.metadataTimeOffsetNonzeroCount += probeDiag.metadataTimeOffsetNonzeroCount;
				combinedTimeDiag.nestedMotionCount += probeDiag.nestedMotionCount;
				combinedTimeDiag.timeOffsetNonzeroCount += probeDiag.timeOffsetNonzeroCount;
				combinedTimeDiag.selfSyncTimeClamped =
					combinedTimeDiag.selfSyncTimeClamped || probeDiag.selfSyncTimeClamped;
				combinedTimeDiag.twoFrameSpecialApplied =
					combinedTimeDiag.twoFrameSpecialApplied || probeDiag.twoFrameSpecialApplied;
				combinedTimeDiag.timeOffsetConsumed =
					combinedTimeDiag.timeOffsetConsumed || probeDiag.timeOffsetConsumed;
				if (combinedTimeDiag.firstNestedMotionPath.empty() &&
					!probeDiag.firstNestedMotionPath.empty()) {
					combinedTimeDiag.firstNestedMotionPath = probeDiag.firstNestedMotionPath;
					combinedTimeDiag.parentTickSample = probeDiag.parentTickSample;
					combinedTimeDiag.childEvalTickSample = probeDiag.childEvalTickSample;
					combinedTimeDiag.timeOffsetSample = probeDiag.timeOffsetSample;
					combinedTimeDiag.requestedTickSample = probeDiag.requestedTickSample;
					combinedTimeDiag.effectiveTickSample = probeDiag.effectiveTickSample;
				}
				if (combinedTimeDiag.twoFrameHoldTickSample == 0.0 &&
					probeDiag.twoFrameHoldTickSample != 0.0) {
					combinedTimeDiag.twoFrameHoldTickSample = probeDiag.twoFrameHoldTickSample;
				}
				bool shouldCopyLimitCoordSample =
					combinedLimitDiag.nanCoordCount == 0 &&
					combinedLimitDiag.infCoordCount == 0 &&
					(probeLimitDiag.nanCoordCount > 0 || probeLimitDiag.infCoordCount > 0);
				combinedLimitDiag.nanCoordCount += probeLimitDiag.nanCoordCount;
				combinedLimitDiag.infCoordCount += probeLimitDiag.infCoordCount;
				combinedLimitDiag.type12LimInheritCount += probeLimitDiag.type12LimInheritCount;
				combinedLimitDiag.nondrawableLimInheritCount += probeLimitDiag.nondrawableLimInheritCount;
				combinedLimitDiag.childLimPropCount += probeLimitDiag.childLimPropCount;
				if (probeLimitDiag.nanInfPathVerified) combinedLimitDiag.nanInfPathVerified = true;
				if (shouldCopyLimitCoordSample) {
					combinedLimitDiag.sampleCoordBeforeX = probeLimitDiag.sampleCoordBeforeX;
					combinedLimitDiag.sampleCoordBeforeY = probeLimitDiag.sampleCoordBeforeY;
					combinedLimitDiag.sampleCoordAfterX = probeLimitDiag.sampleCoordAfterX;
					combinedLimitDiag.sampleCoordAfterY = probeLimitDiag.sampleCoordAfterY;
				}
				combinedLimitDiag.metadataSurveyFrameCount += probeLimitDiag.metadataSurveyFrameCount;
				combinedLimitDiag.metadataSurveyNanCandidateCount += probeLimitDiag.metadataSurveyNanCandidateCount;
				combinedLimitDiag.metadataSurveyInfCandidateCount += probeLimitDiag.metadataSurveyInfCandidateCount;
				combinedLimitDiag.singleFrameNormalizeCount += probeLimitDiag.singleFrameNormalizeCount;
				combinedLimitDiag.interpolationFramePairCount += probeLimitDiag.interpolationFramePairCount;
				combinedLimitDiag.coordInterpolationCount += probeLimitDiag.coordInterpolationCount;
				combinedLimitDiag.interpolationNormalizeCount += probeLimitDiag.interpolationNormalizeCount;
#ifdef __EMSCRIPTEN__
				if (perfTrace) {
					tjs_uint64 perfAfterDiagMerge = TVPGetTickCount();
					perfCounters.probeDiagMergeMs += perfAfterDiagMerge - perfDiagMergeStart;
					if (perfPrimaryMs >= 250 || perfProbeCopyMs >= 250 ||
						perfProbeCollectMs >= 250) {
						fprintf(stderr,
							"[PERF] emote.layout.motion path=%s object_motion=%s primary_ms=%llu probe_copy_ms=%llu probe_collect_ms=%llu layouts_now=%zu probe_layouts=%zu\n",
							perfPath.c_str(),
							perfMotionLabel.c_str(),
							(unsigned long long)perfPrimaryMs,
							(unsigned long long)perfProbeCopyMs,
							(unsigned long long)perfProbeCollectMs,
							resource.layouts.size(),
							probeResource.layouts.size());
					}
				}
#endif
			}
		}
	}
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfAfterLoop = perfTrace ? TVPGetTickCount() : 0;
	if (perfTrace) perfCounters.loopMs += perfAfterLoop - perfLoopStart;
#endif
	combinedTimeDiag.probed = 1;
	if (combinedTimeDiag.nestedMotionCount == 0) {
		combinedTimeDiag.skipReason =
			combinedTimeDiag.metadataNestedMotionRefCount > 0 ?
			"nested-motion-not-in-selected-layout" :
			"no-nested-motion";
	}
	resource.timeSemanticsDiag = combinedTimeDiag;
	combinedLimitDiag.probed = 1;
	if (combinedLimitDiag.nanCoordCount == 0 && combinedLimitDiag.infCoordCount == 0) {
		if (combinedLimitDiag.metadataSurveyFrameCount == 0) {
			combinedLimitDiag.skipReason = "no-frames-with-coords";
		} else {
			combinedLimitDiag.skipReason = "no-nan-inf-in-evaluated-frames";
		}
	}
	if (combinedLimitDiag.nanCoordCount > 0 || combinedLimitDiag.infCoordCount > 0) {
		combinedLimitDiag.nanInfPathVerified = true;
	}
	resource.limitSemanticsDiag = combinedLimitDiag;
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfFinalIrStart = perfTrace ? TVPGetTickCount() : 0;
#endif
	MotionFinalizeRenderMethodIR(resource);
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfAfterFinalIr = perfTrace ? TVPGetTickCount() : 0;
	if (perfTrace) perfCounters.finalIrMs += perfAfterFinalIr - perfFinalIrStart;
	tjs_uint64 perfSelfTestStart = perfAfterFinalIr;
#endif
	MotionRunM2ProgressSurfaceSelfTest(resource.motionRenderMethodSemantics);
	MotionRunM3DemuxSurfaceSelfTest(resource.motionRenderMethodSemantics);
	MotionRunM4RendererUploadSelfTest(resource.motionRenderMethodSemantics);
	MotionRunM5BezierSurfaceChainSelfTest(resource.motionRenderMethodSemantics);
#ifdef __EMSCRIPTEN__
		if (perfTrace) {
			tjs_uint64 perfEnd = TVPGetTickCount();
			perfCounters.selfTestMs += perfEnd - perfSelfTestStart;
			fprintf(stderr,
				"[PERF] emote.layout.build path=%s ok=1 source_icons=%zu source_aliases=%d objects=%zu motions=%zu layouts=%zu render_items=%zu primary_calls=%zu probe_calls=%zu probe_copied_layouts=%zu probe_copied_bytes=%zu merge_calls=%zu merge_create=%zu merge_update=%zu merge_scan_steps=%zu index_entries=%zu index_hit=%zu index_ro_hit=%zu index_miss=%zu index_fallback=%zu index_mismatch=%zu probe_index_entries=%zu probe_index_hit=%zu probe_index_miss=%zu probe_index_fallback=%zu probe_index_mismatch=%zu alias_calls=%zu alias_names=%zu render_item_calls=%zu collect_named_calls=%zu collect_motion_calls=%zu collect_layers=%zu collect_nodes=%zu collect_variants=%zu source_read_ms=%llu source_canon_ms=%llu origin_ms=%llu loop_ms=%llu primary_ms=%llu probe_copy_ms=%llu probe_collect_ms=%llu probe_diag_ms=%llu collect_named_ms=%llu collect_motion_ms=%llu collect_read_layers_ms=%llu collect_find_motion_ms=%llu collect_nodes_ms=%llu collect_setup_ms=%llu collect_first_ms=%llu collect_variant_ms=%llu named_merge_ms=%llu merge_search_ms=%llu merge_semantics_ms=%llu merge_push_ms=%llu alias_ms=%llu alias_build_ms=%llu render_item_ms=%llu final_ir_ms=%llu selftest_ms=%llu total_ms=%llu max_primary_ms=%llu max_primary=%s max_probe_copy_ms=%llu max_probe_copy=%s max_probe_collect_ms=%llu max_probe_collect=%s max_merge_ms=%llu max_merge=%s max_alias_ms=%llu max_alias=%s max_collect_motion_ms=%llu max_collect_motion=%s\n",
			perfPath.c_str(),
			resource.sourceIcons.size(),
			(int)resource.sourceAliasCount,
			resource.objects.size(),
			resource.motions.size(),
			resource.layouts.size(),
			resource.renderMethodIR.size(),
			perfCounters.primaryCalls,
			perfCounters.probeCalls,
			perfCounters.probeCopiedLayouts,
			perfCounters.probeCopiedBytes,
			perfCounters.namedMergeCalls,
			perfCounters.namedMergeCreates,
			perfCounters.namedMergeUpdates,
			perfCounters.namedMergeScanSteps,
				layoutIndexState.entries.size(),
				layoutIndexState.writableHits,
				layoutIndexState.readOnlyHits,
				layoutIndexState.writableMisses,
				layoutIndexState.fallbackScans,
				layoutIndexState.mismatches,
				perfCounters.probeIndexEntries,
				perfCounters.probeIndexHits,
				perfCounters.probeIndexMisses,
				perfCounters.probeIndexFallbacks,
				perfCounters.probeIndexMismatches,
				perfCounters.aliasCalls,
			perfCounters.aliasNames,
			perfCounters.renderItemCalls,
			perfCounters.collectNamedCalls,
			perfCounters.collectMotionCalls,
			perfCounters.collectLayerCount,
			perfCounters.collectNodeCount,
			perfCounters.collectFrameVariantCount,
			(unsigned long long)perfCounters.sourceReadMs,
			(unsigned long long)perfCounters.sourceCanonMs,
			(unsigned long long)perfCounters.originMs,
			(unsigned long long)perfCounters.loopMs,
			(unsigned long long)perfCounters.primaryCollectMs,
			(unsigned long long)perfCounters.probeCopyMs,
			(unsigned long long)perfCounters.probeCollectMs,
			(unsigned long long)perfCounters.probeDiagMergeMs,
			(unsigned long long)perfCounters.collectNamedMs,
			(unsigned long long)perfCounters.collectMotionMs,
			(unsigned long long)perfCounters.collectReadLayersMs,
			(unsigned long long)perfCounters.collectFindMotionMs,
			(unsigned long long)perfCounters.collectNodesMs,
			(unsigned long long)perfCounters.collectSetupMs,
			(unsigned long long)perfCounters.collectFirstPassMs,
			(unsigned long long)perfCounters.collectVariantPassMs,
			(unsigned long long)perfCounters.namedMergeTotalMs,
			(unsigned long long)perfCounters.namedMergeSearchMs,
			(unsigned long long)perfCounters.namedMergeSemanticsMs,
			(unsigned long long)perfCounters.namedMergePushMs,
			(unsigned long long)perfCounters.aliasTotalMs,
			(unsigned long long)perfCounters.aliasBuildMs,
			(unsigned long long)perfCounters.renderItemMs,
			(unsigned long long)perfCounters.finalIrMs,
			(unsigned long long)perfCounters.selfTestMs,
			(unsigned long long)(perfEnd - perfBuildStart),
			(unsigned long long)perfCounters.maxPrimaryMs,
			perfCounters.maxPrimaryLabel.empty() ? "-" : perfCounters.maxPrimaryLabel.c_str(),
			(unsigned long long)perfCounters.maxProbeCopyMs,
			perfCounters.maxProbeCopyLabel.empty() ? "-" : perfCounters.maxProbeCopyLabel.c_str(),
			(unsigned long long)perfCounters.maxProbeCollectMs,
			perfCounters.maxProbeCollectLabel.empty() ? "-" : perfCounters.maxProbeCollectLabel.c_str(),
			(unsigned long long)perfCounters.maxNamedMergeMs,
			perfCounters.maxNamedMergeLabel.empty() ? "-" : perfCounters.maxNamedMergeLabel.c_str(),
			(unsigned long long)perfCounters.maxAliasMs,
			perfCounters.maxAliasLabel.empty() ? "-" : perfCounters.maxAliasLabel.c_str(),
			(unsigned long long)perfCounters.maxCollectMotionMs,
			perfCounters.maxCollectMotionLabel.empty() ? "-" : perfCounters.maxCollectMotionLabel.c_str());
		gMotionLayoutBuildPerf = perfPrevious;
	}
#endif
}

static bool MotionComputeBaseLayoutTransform(psb_t &psb, const psb_objects_t *root,
	const std::map<std::string, MotionSourceIconInfo> &sourceMap,
	const MotionResource &resource, MotionLayoutTransform &base)
{
	int screenOriginX = 0;
	int screenOriginY = 0;
	psb_objects_t *screen = nullptr;
	if (root && MotionUnpackObject(psb, root->get_data("screenSize"), screen)) {
		MotionPsbReadMemberInt(psb, screen, "originX", screenOriginX);
		MotionPsbReadMemberInt(psb, screen, "originY", screenOriginY);
		delete screen;
	}
	if (screenOriginX == 0 && screenOriginY == 0 &&
		resource.width > 0 && resource.height > 0) {
		for (std::map<std::string, MotionSourceIconInfo>::const_iterator it = sourceMap.begin();
			it != sourceMap.end(); ++it) {
			const MotionSourceIconInfo &icon = it->second;
			if (icon.width >= resource.width && icon.height >= resource.height &&
				(icon.originX != 0.0 || icon.originY != 0.0)) {
				screenOriginX = (int)std::lround(icon.originX);
				screenOriginY = (int)std::lround(icon.originY);
				break;
			}
		}
	}
	if (screenOriginX == 0 && screenOriginY == 0) {
		MotionInferScreenOriginFromSourceIcons(
			sourceMap, resource, screenOriginX, screenOriginY);
	}
	if ((screenOriginX == 0 || screenOriginY == 0) &&
		resource.width > 0 && resource.height > 0) {
		for (std::map<std::string, MotionSourceIconInfo>::const_iterator it = sourceMap.begin();
			it != sourceMap.end(); ++it) {
			const MotionSourceIconInfo &icon = it->second;
			if (icon.width < (tjs_real)resource.width ||
				icon.height < (tjs_real)resource.height) {
				continue;
			}
			if (screenOriginX == 0 && icon.originX != 0.0) {
				screenOriginX = (int)std::lround(icon.originX);
			}
			if (screenOriginY == 0 && icon.originY != 0.0) {
				screenOriginY = (int)std::lround(icon.originY);
			}
			if (screenOriginX != 0 && screenOriginY != 0) break;
		}
	}
	if (screenOriginX == 0 || screenOriginY == 0) {
		int inferredX = 0;
		int inferredY = 0;
		if (MotionInferScreenOriginFromSourceIcons(
			sourceMap, resource, inferredX, inferredY)) {
			if (screenOriginX == 0) screenOriginX = inferredX;
			if (screenOriginY == 0) screenOriginY = inferredY;
		}
	}
	MotionApplyMotionStyleCenterOriginFallback(
		sourceMap, resource, screenOriginX, screenOriginY);
	ttstr lowerLabel = resource.label.AsLowerCase();
	if (resource.width > 0 && resource.height > 0) {
		if (screenOriginX == 0 && lowerLabel == TJS_W("main")) {
			screenOriginX = resource.width / 2;
		}
		if (screenOriginY == 0 &&
			(lowerLabel == TJS_W("main") || lowerLabel == TJS_W("autoskip"))) {
			screenOriginY = resource.height / 2;
		}
	}

	base = MotionLayoutTransform();
	base.x = screenOriginX;
	base.y = screenOriginY;
	base.originX = (tjs_real)screenOriginX;
	base.originY = (tjs_real)screenOriginY;
	base.width = (tjs_real)resource.width;
	base.height = (tjs_real)resource.height;
	base.affine = MotionAffineTranslate((tjs_real)screenOriginX, (tjs_real)screenOriginY);
	return !sourceMap.empty();
}

static bool MotionLayoutRenderableForRuntimeDraw(const MotionLayoutInfo &layout)
{
	return !layout.hitLayoutProxy &&
		layout.width > 0.0 &&
		layout.height > 0.0 &&
		layout.drawCount > 0 &&
		MotionHitStateClassIsVisible(layout.hitStateClass) &&
		(layout.opacityMax > 0.001 ||
		 layout.hitRole == TJS_W("visual") ||
		 MotionStartsWith(layout.motionName.AsLowerCase().AsNarrowStdString(), "show") ||
			 MotionStartsWith(layout.contextMotionName.AsLowerCase().AsNarrowStdString(), "show"));
}

static bool MotionLayoutIsRuntimeTextureLeaf(
	const MotionResource *resource, const MotionLayoutInfo &layout)
{
	if (layout.sourceKey.GetLen() <= 0 || layout.renderMethodItemIndex < 0) return false;
	if (!MotionStartsWith(layout.sourceKey.AsNarrowStdString(), "src/")) return false;
	if (!resource ||
		layout.renderMethodItemIndex >= (tjs_int)resource->renderMethodIR.size()) {
		return false;
	}
	const MotionRenderMethodItem &item =
		resource->renderMethodIR[(size_t)layout.renderMethodItemIndex];
	return !item.isFromLayout &&
		!item.isFromNestedMotion &&
		(item.type == 1 || item.type == 2) &&
		item.srcKey == layout.sourceKey.AsNarrowStdString();
}

static bool MotionResourceHasSourceKey(const MotionResource &resource,
	const std::string &key)
{
	if (key.empty()) return false;
	for (size_t i = 0; i < resource.sourceIcons.size(); i++) {
		const MotionSourceIconInfo &icon = resource.sourceIcons[i];
		if (icon.key == key || icon.name == key || icon.canonicalKey == key) return true;
	}
	return false;
}

static void MotionFinalizeObjectGraphDiag(MotionResource &resource)
{
	MotionObjectGraphDiag diag;
	diag.graphBuilt = resource.parsed;
	diag.objectCount = (tjs_int)resource.objects.size();
	diag.motionCount = (tjs_int)resource.motions.size();
	diag.layoutCount = (tjs_int)resource.layouts.size();
	diag.renderMethodCount = (tjs_int)resource.renderMethodIR.size();
	diag.sourceIconCount = (tjs_int)resource.sourceIcons.size();
	diag.sourceAliasCount = resource.sourceAliasCount;
	std::set<std::string> sourceKeysUsed;

	for (size_t i = 0; i < resource.motions.size(); i++) {
		const MotionInfo &motion = resource.motions[i];
		diag.nodeCount += (tjs_int)motion.nodes.size();
		for (size_t r = 0; r < motion.attachmentRefs.size(); r++) {
			const MotionAttachmentRefInfo &ref = motion.attachmentRefs[r];
			diag.motionRefCount++;
			if (ref.resolved) diag.resolvedMotionRefCount++;
			else diag.unresolvedMotionRefCount++;
			if (ref.external) diag.externalMotionRefCount++;
		}
	}
	for (size_t i = 0; i < resource.layouts.size(); i++) {
		const MotionLayoutInfo &layout = resource.layouts[i];
		if (layout.objectName.GetLen() > 0 || layout.contextObjectName.GetLen() > 0 ||
			layout.hitObjectName.GetLen() > 0) {
			diag.layoutWithObjectCount++;
		}
		if (layout.motionName.GetLen() > 0 || layout.contextMotionName.GetLen() > 0 ||
			layout.hitStateName.GetLen() > 0) {
			diag.layoutWithMotionCount++;
		}
		if (layout.sourceKey.GetLen() > 0) diag.layoutWithSourceKeyCount++;
		if (layout.renderMethodItemIndex >= 0) diag.layoutWithRenderMethodLinkCount++;
	}
	for (size_t i = 0; i < resource.renderMethodIR.size(); i++) {
		const MotionRenderMethodItem &item = resource.renderMethodIR[i];
		if (MotionStartsWith(item.srcKey, "src/")) {
			diag.renderMethodWithSourceKeyCount++;
			sourceKeysUsed.insert(item.srcKey);
		}
		if (item.linkedToLayout && item.layoutIndex >= 0) {
			diag.renderMethodLinkedLayoutCount++;
		}
	}
	diag.sourceKeysUsedCount = (tjs_int)sourceKeysUsed.size();
	for (std::set<std::string>::const_iterator it = sourceKeysUsed.begin();
		it != sourceKeysUsed.end(); ++it) {
		if (MotionResourceHasSourceKey(resource, *it)) diag.sourceKeysResolvedCount++;
		else diag.sourceKeysUnresolvedCount++;
	}
	if (!diag.graphBuilt) diag.skipReason = "resource-not-parsed";
	else if (diag.objectCount == 0) diag.skipReason = "no-objects";
	else if (diag.motionCount == 0) diag.skipReason = "no-motions";
	else if (diag.sourceIconCount == 0) diag.skipReason = "no-source-icons";
	else if (diag.layoutCount == 0) diag.skipReason = "no-layouts";
	else if (diag.renderMethodCount == 0) diag.skipReason = "no-render-method";
	resource.objectGraphDiag = diag;
}

static bool MotionRenderMethodItemRenderableForRuntimeDraw(
	const MotionRenderMethodItem &item)
{
	return (item.type == 1 || item.type == 2) &&
		!item.isFromLayout &&
		!item.srcKey.empty() &&
		MotionStartsWith(item.srcKey, "src/") &&
		item.opacity > 0.001 &&
		item.width > 0.0 &&
		item.height > 0.0 &&
		item.affineValid;
}

static bool MotionBuildRuntimeDrawResource(const MotionResource &baseResource,
	const ttstr &objectName, const ttstr &motionName, tjs_real tick,
	tjs_real drawLimitOriginX, tjs_real drawLimitOriginY,
	tjs_real drawLimitWidth, tjs_real drawLimitHeight,
	MotionResource &runtimeResource,
	const std::map<std::string, tjs_real> *variables = nullptr)
{
	runtimeResource = baseResource;
	if (drawLimitWidth > 0.0 && drawLimitHeight > 0.0) {
		runtimeResource.width = (tjs_int)std::lround(drawLimitWidth);
		runtimeResource.height = (tjs_int)std::lround(drawLimitHeight);
	}
	runtimeResource.layouts.clear();
	runtimeResource.renderMethodIR.clear();
	runtimeResource.timeSemanticsDiag = MotionTimeSemanticsDiag();
	runtimeResource.limitSemanticsDiag = MotionLimitSemanticsDiag();
	runtimeResource.motionRenderMethodSemantics = MotionRenderMethodSemanticsDiag();
	runtimeResource.affineFrontMeshDiag = MotionAffineFrontMeshDiag();
	runtimeResource.meshDeformationDiag = MotionMeshDeformationDiag();
	if (baseResource.bytes.size() < 4 || !baseResource.parsed) return false;

	try {
		psb_t psb(const_cast<unsigned char*>(baseResource.bytes.data()));
		const psb_objects_t *root = psb.get_objects();
		if (!root) return false;
		psb_objects_t *objects = nullptr;
		if (!MotionUnpackObject(psb, root->get_data("object"), objects)) return false;

		std::map<std::string, MotionSourceIconInfo> sourceMap;
		MotionReadSourceIcons(psb, root, sourceMap);
		MotionLayoutTransform base;
		if (!MotionComputeBaseLayoutTransform(psb, root, sourceMap, runtimeResource, base)) {
			delete objects;
			return false;
		}
		tjs_real runtimeLimitZMax = kMotionDefaultLimitZMax;

		std::string objectText = objectName.AsNarrowStdString();
		std::string motionText = motionName.AsNarrowStdString();
		if (objectText.empty()) objectText = baseResource.chara.AsNarrowStdString();
		if (motionText.empty()) motionText = baseResource.motion.AsNarrowStdString();
		if (objectText.empty() || motionText.empty()) {
			delete objects;
			return false;
		}

		MotionLayoutBounds bounds;
		std::set<std::string> motionStack;
		MotionTimeSemanticsDiag timeDiag;
		MotionLimitSemanticsDiag limitDiag;
		limitDiag.sampleParentLimOriginX = drawLimitOriginX;
		limitDiag.sampleParentLimOriginY = drawLimitOriginY;
		limitDiag.sampleParentLimWidth = drawLimitWidth;
		limitDiag.sampleParentLimHeight = drawLimitHeight;
		limitDiag.sampleParentLimZMax = runtimeLimitZMax;
		bool ok = MotionCollectNamedLayoutBounds(psb, objects, sourceMap,
			objectText, motionText, base, std::string(), runtimeResource,
			MotionString(objectText), MotionString(motionText),
			runtimeResource.width, runtimeResource.height, motionStack, 0, bounds,
			std::max<tjs_real>(0.0, tick), &timeDiag, &limitDiag, nullptr,
			variables);
		delete objects;
		if (!ok || runtimeResource.layouts.empty()) return false;

		timeDiag.probed = 1;
		if (timeDiag.nestedMotionCount == 0) {
			timeDiag.skipReason =
				timeDiag.metadataNestedMotionRefCount > 0 ?
				"nested-motion-not-in-selected-runtime-layout" :
				"no-nested-motion";
		}
		limitDiag.probed = 1;
		if (limitDiag.nanCoordCount == 0 && limitDiag.infCoordCount == 0) {
			limitDiag.skipReason =
				limitDiag.metadataSurveyFrameCount == 0 ?
				"no-frames-with-coords" :
				"no-nan-inf-in-evaluated-frames";
		}
		if (limitDiag.nanCoordCount > 0 || limitDiag.infCoordCount > 0) {
			limitDiag.nanInfPathVerified = true;
		}
		runtimeResource.timeSemanticsDiag = timeDiag;
			runtimeResource.limitSemanticsDiag = limitDiag;
			MotionFinalizeRenderMethodIR(runtimeResource);
			MotionRunM2ProgressSurfaceSelfTest(runtimeResource.motionRenderMethodSemantics);
			MotionRunM3DemuxSurfaceSelfTest(runtimeResource.motionRenderMethodSemantics);
			MotionRunM4RendererUploadSelfTest(runtimeResource.motionRenderMethodSemantics);
			MotionRunM5BezierSurfaceChainSelfTest(runtimeResource.motionRenderMethodSemantics);
			runtimeResource.chara = MotionString(objectText);
		runtimeResource.motion = MotionString(motionText);
		for (size_t i = 0; i < runtimeResource.layouts.size(); i++) {
			if (MotionLayoutRenderableForRuntimeDraw(runtimeResource.layouts[i])) return true;
		}
		return false;
	} catch (...) {
		return false;
	}
}

static bool ParseMotionResource(const ttstr &path, const std::vector<unsigned char> &bytes,
	tjs_uint64 byteSize, MotionResource &resource)
{
#ifdef __EMSCRIPTEN__
	std::string rawPath = path.AsNarrowStdString();
	bool tracePsb = rawPath.find(".psb") != std::string::npos || rawPath.find(".PSB") != std::string::npos;
	tjs_uint64 perfStart = tracePsb ? TVPGetTickCount() : 0;
#endif
	resource.storage = path;
	resource.byteSize = byteSize;
	resource.bytes = bytes;
	resource.opened = !bytes.empty();
	resource.parsed = false;
	resource.isMotion = false;
	resource.hasMetadata = false;
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfAfterCopy = tracePsb ? TVPGetTickCount() : 0;
#endif
	if (bytes.size() < 4 || bytes[0] != 'P' || bytes[1] != 'S' || bytes[2] != 'B') {
		return false;
	}

	try {
		psb_t psb(const_cast<unsigned char*>(bytes.data()));
#ifdef __EMSCRIPTEN__
		tjs_uint64 perfAfterCtor = tracePsb ? TVPGetTickCount() : 0;
#endif
		const psb_objects_t *root = psb.get_objects();
		if (!root) {
#ifdef __EMSCRIPTEN__
			if (tracePsb) {
				tjs_uint64 perfEnd = TVPGetTickCount();
				fprintf(stderr,
					"[PERF] emote.psb.parse path=%s ok=0 reason=no-root bytes=%llu copy_ms=%llu psb_ctor_ms=%llu total_ms=%llu\n",
					rawPath.c_str(),
					(unsigned long long)byteSize,
					(unsigned long long)(perfAfterCopy - perfStart),
					(unsigned long long)(perfAfterCtor - perfAfterCopy),
					(unsigned long long)(perfEnd - perfStart));
			}
#endif
			return false;
		}

		std::string value;
		if (MotionPsbReadMemberString(psb, root, "spec", value)) {
			resource.spec = MotionString(value);
		}
		if (MotionPsbReadMemberString(psb, root, "label", value)) resource.label = MotionString(value);
		MotionReadMetadata(psb, root, resource);

		psb_objects_t *screen = nullptr;
		if (MotionUnpackObject(psb, root->get_data("screenSize"), screen)) {
			int dimension = 0;
			if (MotionPsbReadMemberInt(psb, screen, "width", dimension)) resource.width = dimension;
			if (MotionPsbReadMemberInt(psb, screen, "height", dimension)) resource.height = dimension;
			delete screen;
		}
#ifdef __EMSCRIPTEN__
		tjs_uint64 perfAfterMetadata = tracePsb ? TVPGetTickCount() : 0;
		tjs_uint64 perfObjectsStart = perfAfterMetadata;
		tjs_uint64 perfAfterObjects = perfAfterMetadata;
		tjs_uint64 perfLayoutStart = perfAfterMetadata;
		tjs_uint64 perfAfterLayout = perfAfterMetadata;
#endif

		psb_objects_t *objects = nullptr;
		if (MotionUnpackObject(psb, root->get_data("object"), objects)) {
#ifdef __EMSCRIPTEN__
			perfObjectsStart = tracePsb ? TVPGetTickCount() : perfObjectsStart;
#endif
			for (uint32_t i = 0; i < objects->size(); i++) {
				std::string objectName = objects->get_name(i);
				psb_objects_t *object = nullptr;
				if (!MotionUnpackObject(psb, objects->get_data(i), object)) continue;

				MotionObjectInfo objectInfo;
				objectInfo.name = MotionString(objectName);
				if (MotionPsbReadMemberString(psb, object, "type", value)) objectInfo.type = MotionString(value);

				psb_objects_t *motionRoot = nullptr;
				if (MotionUnpackObject(psb, object->get_data("motion"), motionRoot)) {
					for (uint32_t m = 0; m < motionRoot->size(); m++) {
						std::string motionName = motionRoot->get_name(m);
						psb_objects_t *motionObject = nullptr;
						if (!MotionUnpackObject(psb, motionRoot->get_data(m), motionObject)) continue;
						MotionInfo info = MotionReadMotion(psb, motionObject,
							objectInfo.name, MotionString(motionName));
						for (size_t v = 0; v < info.variables.size(); v++) {
							MotionAddVariable(resource.variables, info.variables[v]);
						}
						objectInfo.motions.push_back(info);
						resource.motions.push_back(info);
						delete motionObject;
					}
					delete motionRoot;
				}
				resource.objects.push_back(objectInfo);
				delete object;
			}
#ifdef __EMSCRIPTEN__
			perfAfterObjects = tracePsb ? TVPGetTickCount() : perfAfterObjects;
			perfLayoutStart = perfAfterObjects;
#endif
			MotionBuildResourceLayouts(psb, root, objects, resource);
#ifdef __EMSCRIPTEN__
			perfAfterLayout = tracePsb ? TVPGetTickCount() : perfAfterLayout;
#endif
			delete objects;
		}

#ifdef __EMSCRIPTEN__
		tjs_uint64 perfFinalizeStart = tracePsb ? TVPGetTickCount() : 0;
#endif
		MotionResolveNestedSyncTimes(resource);
		if (resource.chara.GetLen() <= 0 && !resource.objects.empty()) {
			resource.chara = resource.objects[0].name;
		}
		if (resource.motion.GetLen() <= 0) {
			for (size_t i = 0; i < resource.objects.size() && resource.motion.GetLen() <= 0; i++) {
				if (!resource.objects[i].motions.empty()) resource.motion = resource.objects[i].motions[0].name;
			}
		}
		resource.parsed = true;
		MotionFinalizeObjectGraphDiag(resource);
#ifdef __EMSCRIPTEN__
		if (tracePsb) {
			tjs_uint64 perfEnd = TVPGetTickCount();
			fprintf(stderr,
				"[PERF] emote.psb.parse path=%s ok=1 bytes=%llu objects=%zu motions=%zu layouts=%zu variables=%zu copy_ms=%llu psb_ctor_ms=%llu metadata_ms=%llu object_motion_ms=%llu layout_ms=%llu finalize_ms=%llu total_ms=%llu\n",
				rawPath.c_str(),
				(unsigned long long)byteSize,
				resource.objects.size(),
				resource.motions.size(),
				resource.layouts.size(),
				resource.variables.size(),
				(unsigned long long)(perfAfterCopy - perfStart),
				(unsigned long long)(perfAfterCtor - perfAfterCopy),
				(unsigned long long)(perfAfterMetadata - perfAfterCtor),
				(unsigned long long)(perfAfterObjects - perfObjectsStart),
				(unsigned long long)(perfAfterLayout - perfLayoutStart),
				(unsigned long long)(perfEnd - perfFinalizeStart),
				(unsigned long long)(perfEnd - perfStart));
		}
#endif
		return true;
	} catch (...) {
#ifdef __EMSCRIPTEN__
		if (tracePsb) {
			tjs_uint64 perfEnd = TVPGetTickCount();
			fprintf(stderr,
				"[PERF] emote.psb.parse path=%s ok=0 reason=exception bytes=%llu total_ms=%llu\n",
				rawPath.c_str(),
				(unsigned long long)byteSize,
				(unsigned long long)(perfEnd - perfStart));
		}
#endif
		TVPAddLog(ttstr(TJS_W("[emoteplayer] failed to introspect PSB: ")) + path);
		return false;
	}
}
