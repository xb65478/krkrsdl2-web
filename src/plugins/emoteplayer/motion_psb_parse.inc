static const TVPStaticMotionBridgeSnapshot *MotionResolveStaticBridgeSnapshot(
	const ttstr &path, const MotionResource *resource)
{
	if (resource) {
		std::string storage = path.AsNarrowStdString();
		std::string objectName = resource->chara.AsNarrowStdString();
		std::string motionName = resource->motion.AsNarrowStdString();
		const TVPStaticMotionBridgeSnapshot *cached =
			TVPFindPSBStaticMotionBridgeSnapshot(
				storage.c_str(), objectName.c_str(), motionName.c_str());
		if (cached) return cached;
	}
	const TVPStaticMotionBridgeSnapshot *snapshot =
		TVPGetLastPSBStaticMotionBridgeSnapshot();
	if (MotionPathMatchesStaticBridgeSnapshot(path, snapshot)) return snapshot;
	return snapshot;
}

static bool MotionHasStaticBridgeTextureSnapshot(
	const ttstr &path, const MotionResource *resource)
{
	const TVPStaticMotionBridgeSnapshot *snapshot =
		MotionResolveStaticBridgeSnapshot(path, resource);
	return snapshot &&
		MotionPathMatchesStaticBridgeSnapshot(path, snapshot) &&
		!snapshot->items.empty();
}

static bool MotionHasStaticBridgeTextureSnapshot(
	const ttstr &path, const ttstr &objectName, const ttstr &motionName)
{
	std::string storage = path.AsNarrowStdString();
	const TVPStaticMotionBridgeSnapshot *snapshot =
		TVPFindPSBStaticMotionBridgeSnapshot(
			storage.c_str(),
			objectName.AsNarrowStdString().c_str(),
			motionName.AsNarrowStdString().c_str());
	return snapshot &&
		MotionPathMatchesStaticBridgeSnapshot(path, snapshot) &&
		!snapshot->items.empty();
}

static ttstr MotionString(const std::string &text)
{
	if (text.empty()) return ttstr();
	tjs_string converted;
	if (TVPUtf8ToUtf16(converted, text)) return ttstr(converted);
	return ttstr(text.c_str());
}

static std::string MotionCacheKey(const ttstr &path)
{
#ifdef __EMSCRIPTEN__
	std::string rawPath = path.AsNarrowStdString();
	if (rawPath.find(".psb") != std::string::npos || rawPath.find(".PSB") != std::string::npos) {
		KRKR_LOG_L3("[EMOTE-PSB-IO] cache-key begin path=%s\n", rawPath.c_str());
	}
#endif
	try {
		ttstr placed = TVPGetPlacedPath(path);
#ifdef __EMSCRIPTEN__
		if (rawPath.find(".psb") != std::string::npos || rawPath.find(".PSB") != std::string::npos) {
			KRKR_LOG_L3("[EMOTE-PSB-IO] cache-key placed path=%s placed=%s\n",
				rawPath.c_str(), placed.AsNarrowStdString().c_str());
		}
#endif
		if (placed.GetLen() > 0) return placed.AsNarrowStdString();
	} catch (...) {
#ifdef __EMSCRIPTEN__
		if (rawPath.find(".psb") != std::string::npos || rawPath.find(".PSB") != std::string::npos) {
			KRKR_LOG_L2("[EMOTE-PSB-IO] cache-key placed-exception path=%s\n", rawPath.c_str());
		}
#endif
	}
	return path.AsNarrowStdString();
}

static bool ReadStorageBytes(const ttstr &path, std::vector<unsigned char> &bytes, tjs_uint64 *size)
{
	bytes.clear();
	tTJSBinaryStream *stream = nullptr;
#ifdef __EMSCRIPTEN__
	std::string rawPath = path.AsNarrowStdString();
	bool tracePsb = rawPath.find(".psb") != std::string::npos || rawPath.find(".PSB") != std::string::npos;
	tjs_uint64 perfStart = tracePsb ? TVPGetTickCount() : 0;
	if (tracePsb) {
		KRKR_LOG_L3("[EMOTE-PSB-IO] read begin path=%s\n", rawPath.c_str());
	}
#endif
	try {
		stream = TVPCreateStream(path);
#ifdef __EMSCRIPTEN__
		if (tracePsb) {
			KRKR_LOG_L3("[EMOTE-PSB-IO] read stream-open path=%s stream=%p\n",
				rawPath.c_str(), (void*)stream);
		}
#endif
	} catch (...) {
		stream = nullptr;
#ifdef __EMSCRIPTEN__
		tjs_uint64 perfAfterOpen = tracePsb ? TVPGetTickCount() : 0;
		if (tracePsb) {
			KRKR_LOG_L2("[EMOTE-PSB-IO] read stream-exception path=%s\n", rawPath.c_str());
			fprintf(stderr,
				"[PERF] emote.psb.read path=%s ok=0 reason=open-exception bytes=0 stream_size=0 chunks=0 open_ms=%llu size_ms=0 read_ms=0 total_ms=%llu\n",
				rawPath.c_str(),
				(unsigned long long)(perfAfterOpen - perfStart),
				(unsigned long long)(perfAfterOpen - perfStart));
		}
#endif
	}
	if (!stream) {
#ifdef __EMSCRIPTEN__
		if (tracePsb) {
			tjs_uint64 perfEnd = TVPGetTickCount();
			fprintf(stderr,
				"[PERF] emote.psb.read path=%s ok=0 reason=open-failed bytes=0 stream_size=0 chunks=0 open_ms=%llu size_ms=0 read_ms=0 total_ms=%llu\n",
				rawPath.c_str(),
				(unsigned long long)(perfEnd - perfStart),
				(unsigned long long)(perfEnd - perfStart));
		}
#endif
		return false;
	}

#ifdef __EMSCRIPTEN__
	tjs_uint64 perfSizeStart = tracePsb ? TVPGetTickCount() : 0;
#endif
	tjs_uint64 streamSize = stream->GetSize();
	if (size) *size = streamSize;
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfAfterSize = tracePsb ? TVPGetTickCount() : 0;
	if (tracePsb) {
		KRKR_LOG_L3("[EMOTE-PSB-IO] read size path=%s size=%llu\n",
			rawPath.c_str(), (unsigned long long)streamSize);
	}
#endif
	if (streamSize == 0 || streamSize > 0x7fffffffULL) {
		delete stream;
#ifdef __EMSCRIPTEN__
		if (tracePsb) {
			tjs_uint64 perfEnd = TVPGetTickCount();
			fprintf(stderr,
				"[PERF] emote.psb.read path=%s ok=0 reason=invalid-size bytes=0 stream_size=%llu chunks=0 open_ms=%llu size_ms=%llu read_ms=0 total_ms=%llu\n",
				rawPath.c_str(),
				(unsigned long long)streamSize,
				(unsigned long long)(perfSizeStart - perfStart),
				(unsigned long long)(perfAfterSize - perfSizeStart),
				(unsigned long long)(perfEnd - perfStart));
		}
#endif
		return false;
	}

	bytes.resize((size_t)streamSize);
	tjs_uint64 totalRead = 0;
	tjs_uint chunks = 0;
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfReadStart = tracePsb ? TVPGetTickCount() : 0;
#endif
	try {
		const tjs_uint chunkSize = 64 * 1024;
		while (totalRead < streamSize) {
			tjs_uint request = (tjs_uint)std::min<tjs_uint64>(chunkSize, streamSize - totalRead);
			tjs_uint read = stream->Read(bytes.data() + (size_t)totalRead, request);
			chunks++;
#ifdef __EMSCRIPTEN__
			if (tracePsb && (totalRead == 0 || read == 0 || totalRead + read >= streamSize)) {
				KRKR_LOG_L3("[EMOTE-PSB-IO] read chunk path=%s offset=%llu request=%u got=%u size=%llu\n",
					rawPath.c_str(),
					(unsigned long long)totalRead,
					(unsigned)request,
					(unsigned)read,
					(unsigned long long)streamSize);
			}
#endif
			if (read == 0) break;
			totalRead += read;
		}
	} catch (...) {
#ifdef __EMSCRIPTEN__
		TVPAddLog(ttstr(TJS_W("[emoteplayer] ReadStorageBytes read failed: ")) + path +
			TJS_W(" offset=") + ttstr((tjs_int)totalRead) +
			TJS_W(" size=") + ttstr((tjs_int)streamSize));
		if (tracePsb) {
			tjs_uint64 perfEnd = TVPGetTickCount();
			fprintf(stderr,
				"[PERF] emote.psb.read path=%s ok=0 reason=read-exception bytes=%llu stream_size=%llu chunks=%u open_ms=%llu size_ms=%llu read_ms=%llu total_ms=%llu\n",
				rawPath.c_str(),
				(unsigned long long)totalRead,
				(unsigned long long)streamSize,
				(unsigned)chunks,
				(unsigned long long)(perfSizeStart - perfStart),
				(unsigned long long)(perfAfterSize - perfSizeStart),
				(unsigned long long)(perfEnd - perfReadStart),
				(unsigned long long)(perfEnd - perfStart));
		}
#endif
		delete stream;
		bytes.clear();
		return false;
	}
	delete stream;
	if (totalRead < streamSize) bytes.resize((size_t)totalRead);
#ifdef __EMSCRIPTEN__
	if (tracePsb) {
		tjs_uint64 perfEnd = TVPGetTickCount();
		KRKR_LOG_L3("[EMOTE-PSB-IO] read done path=%s total=%llu ok=%d\n",
			rawPath.c_str(),
			(unsigned long long)totalRead,
			bytes.size() >= 4 ? 1 : 0);
		fprintf(stderr,
			"[PERF] emote.psb.read path=%s ok=%d reason=done bytes=%llu stream_size=%llu chunks=%u open_ms=%llu size_ms=%llu read_ms=%llu total_ms=%llu\n",
			rawPath.c_str(),
			bytes.size() >= 4 ? 1 : 0,
			(unsigned long long)totalRead,
			(unsigned long long)streamSize,
			(unsigned)chunks,
			(unsigned long long)(perfSizeStart - perfStart),
			(unsigned long long)(perfAfterSize - perfSizeStart),
			(unsigned long long)(perfEnd - perfReadStart),
			(unsigned long long)(perfEnd - perfStart));
	}
#endif
	return bytes.size() >= 4;
}

static bool MotionPsbIsNumberType(psb_value_t *value)
{
	if (!value) return false;
	psb_value_t::type_t t = value->get_type();
	return (t >= psb_value_t::TYPE_NUMBER_N0 && t <= psb_value_t::TYPE_NUMBER_N8) ||
		t == psb_value_t::TYPE_FLOAT0 ||
		t == psb_value_t::TYPE_FLOAT ||
		t == psb_value_t::TYPE_DOUBLE;
}

static bool MotionPsbIsStringType(psb_value_t *value)
{
	if (!value) return false;
	psb_value_t::type_t t = value->get_type();
	return t >= psb_value_t::TYPE_STRING_N1 && t <= psb_value_t::TYPE_STRING_N4;
}

static bool MotionPsbIsResourceType(psb_value_t *value)
{
	if (!value) return false;
	psb_value_t::type_t t = value->get_type();
	return t >= psb_value_t::TYPE_RESOURCE_N1 &&
		t <= psb_value_t::TYPE_RESOURCE_N4;
}

static bool MotionPsbReadNumber(psb_t &psb, unsigned char *data, double &out)
{
	if (!data) return false;
	unsigned char *p = data;
	psb_value_t *value = psb.unpack(p);
	if (!MotionPsbIsNumberType(value)) {
		delete value;
		return false;
	}
	psb_number_t *number = static_cast<psb_number_t*>(value);
	switch (value->get_type()) {
	case psb_value_t::TYPE_FLOAT0:
	case psb_value_t::TYPE_FLOAT:
		out = number->get_float();
		break;
	case psb_value_t::TYPE_DOUBLE:
		out = number->get_double();
		break;
	default:
		out = (double)number->get_integer();
		break;
	}
	delete value;
	return true;
}

static bool MotionPsbReadInt(psb_t &psb, unsigned char *data, int &out)
{
	double value = 0.0;
	if (!MotionPsbReadNumber(psb, data, value)) return false;
	out = (int)std::lround(value);
	return true;
}

static bool MotionPsbReadString(psb_t &psb, unsigned char *data, std::string &out)
{
	if (!data) return false;
	unsigned char *p = data;
	psb_value_t *value = psb.unpack(p);
	if (!MotionPsbIsStringType(value)) {
		delete value;
		return false;
	}
	out = static_cast<psb_string_t*>(value)->get_string();
	delete value;
	return true;
}

static bool MotionPsbReadResourceLength(psb_t &psb, unsigned char *data,
	tjs_uint32 &out)
{
	out = 0;
	if (!data) return false;
	unsigned char *p = data;
	psb_value_t *value = psb.unpack(p);
	if (!MotionPsbIsResourceType(value)) {
		delete value;
		return false;
	}
	psb_resource_t *resource = static_cast<psb_resource_t*>(value);
	out = resource->get_length();
	delete value;
	return true;
}

static bool MotionPsbReadMemberNumber(psb_t &psb, const psb_objects_t *obj, const char *key, double &out)
{
	return obj && MotionPsbReadNumber(psb, obj->get_data(key), out);
}

static bool MotionPsbReadMemberInt(psb_t &psb, const psb_objects_t *obj, const char *key, int &out)
{
	return obj && MotionPsbReadInt(psb, obj->get_data(key), out);
}

static bool MotionPsbReadMemberString(psb_t &psb, const psb_objects_t *obj, const char *key, std::string &out)
{
	return obj && MotionPsbReadString(psb, obj->get_data(key), out);
}

static bool MotionPsbReadCollection(psb_t &psb, unsigned char *data, std::vector<unsigned char*> &entries)
{
	entries.clear();
	if (!data) return false;
	unsigned char *p = data;
	psb_value_t *value = psb.unpack(p);
	if (!value || value->get_type() != psb_value_t::TYPE_COLLECTION) {
		delete value;
		return false;
	}
	psb_collection_t *collection = static_cast<psb_collection_t*>(value);
	for (uint32_t i = 0; i < collection->size(); i++) {
		entries.push_back(collection->get(i));
	}
	delete value;
	return true;
}

static bool MotionPsbReadNumberList(psb_t &psb, unsigned char *data, std::vector<double> &out)
{
	out.clear();
	std::vector<unsigned char*> entries;
	if (!MotionPsbReadCollection(psb, data, entries)) return false;
	for (size_t i = 0; i < entries.size(); i++) {
		double value = 0.0;
		if (!MotionPsbReadNumber(psb, entries[i], value)) return false;
		out.push_back(value);
	}
	return !out.empty();
}

static std::vector<std::string> MotionSplitPath(const std::string &path)
{
	std::vector<std::string> parts;
	size_t start = 0;
	while (start <= path.size()) {
		size_t slash = path.find('/', start);
		if (slash == std::string::npos) slash = path.size();
		if (slash > start) parts.push_back(path.substr(start, slash - start));
		if (slash == path.size()) break;
		start = slash + 1;
	}
	return parts;
}

static bool MotionParseMotionRefPath(const std::string &ref,
	std::string &objectName, std::string &motionName, std::string &iconName)
{
	objectName.clear();
	motionName.clear();
	iconName.clear();
	std::vector<std::string> parts = MotionSplitPath(ref);
	if (parts.size() < 3 || parts[0] != "motion") return false;
	objectName = parts[1];
	motionName = parts[2];
	if (parts.size() >= 4) {
		iconName = parts[3];
		for (size_t i = 4; i < parts.size(); i++) iconName += "/" + parts[i];
	}
	return !objectName.empty() && !motionName.empty();
}

static bool MotionStartsWith(const std::string &value, const char *prefix)
{
	size_t len = strlen(prefix);
	return value.size() >= len && value.compare(0, len, prefix) == 0;
}

static bool MotionTraceInterestingSettingObject(const std::string &objectName)
{
	return objectName.find("tg_") != std::string::npos ||
		objectName.find("skip") != std::string::npos;
}

static std::string MotionJoinLayoutPath(const std::string &prefix,
	const std::string &name)
{
	if (name.empty()) return prefix;
	if (prefix.empty()) return name;
	return prefix + "/" + name;
}

static void MotionAppendUniqueLayoutName(std::vector<std::string> &names,
	const std::string &name)
{
	if (name.empty()) return;
	for (size_t i = 0; i < names.size(); i++) {
		if (names[i] == name) return;
	}
	names.push_back(name);
}

static std::string MotionStripFirstLayoutSegment(const std::string &path)
{
	size_t slash = path.find('/');
	if (slash == std::string::npos || slash + 1 >= path.size()) return std::string();
	return path.substr(slash + 1);
}

static bool MotionPathStartsWithAny(const std::string &path, const char *const *prefixes)
{
	for (size_t i = 0; prefixes && prefixes[i]; i++) {
		if (MotionStartsWith(path, prefixes[i])) return true;
	}
	return false;
}

static std::string MotionAddScopedLayoutSegment(const std::string &path,
	const char *scope)
{
	if (!scope || !*scope || path.empty()) return std::string();
	std::string prefix = std::string(scope) + "/";
	if (MotionStartsWith(path, prefix.c_str())) return std::string();
	return prefix + path;
}

static std::string MotionRemoveBaseLayoutSegment(const std::string &path)
{
	static const char marker[] = "/base/";
	size_t pos = path.find(marker);
	if (pos != std::string::npos) {
		return path.substr(0, pos) + "/" + path.substr(pos + strlen(marker));
	}
	static const char suffix[] = "/base";
	size_t suffixLen = strlen(suffix);
	if (path.size() > suffixLen &&
		path.compare(path.size() - suffixLen, suffixLen, suffix) == 0) {
		return path.substr(0, path.size() - suffixLen);
	}
	return std::string();
}

static std::vector<std::string> MotionLayoutNameAliases(const std::string &path)
{
	std::vector<std::string> names;
	MotionAppendUniqueLayoutName(names, path);

	static const char *const scopedRoots[] = { "slot/", "page/", nullptr };
	const char *const scopeNames[] = { "slot", "page", nullptr };
	if (!MotionPathStartsWithAny(path, scopedRoots)) {
		for (size_t i = 0; scopeNames[i]; i++) {
			MotionAppendUniqueLayoutName(names,
				MotionAddScopedLayoutSegment(path, scopeNames[i]));
		}
	}

	std::string noBase = MotionRemoveBaseLayoutSegment(path);
	MotionAppendUniqueLayoutName(names, noBase);
	std::string stripped = MotionStripFirstLayoutSegment(path);
	MotionAppendUniqueLayoutName(names, stripped);
	if (!noBase.empty()) {
		MotionAppendUniqueLayoutName(names, MotionStripFirstLayoutSegment(noBase));
		if (!MotionPathStartsWithAny(noBase, scopedRoots)) {
			for (size_t i = 0; scopeNames[i]; i++) {
				MotionAppendUniqueLayoutName(names,
					MotionAddScopedLayoutSegment(noBase, scopeNames[i]));
			}
		}
	}
	return names;
}

static std::string MotionLowerString(std::string value)
{
	std::transform(value.begin(), value.end(), value.begin(),
		[](unsigned char c) { return (char)std::tolower(c); });
	return value;
}

static bool MotionUnpackObject(psb_t &psb, unsigned char *data, psb_objects_t *&out)
{
	out = nullptr;
	if (!data) return false;
	unsigned char *p = data;
	psb_value_t *value = psb.unpack(p);
	if (!value || value->get_type() != psb_value_t::TYPE_OBJECTS) {
		delete value;
		return false;
	}
	out = static_cast<psb_objects_t*>(value);
	return true;
}

static void MotionAddVariable(std::vector<MotionVariableInfo> &variables, const MotionVariableInfo &item)
{
	if (item.name.GetLen() <= 0) return;
	for (size_t i = 0; i < variables.size(); i++) {
		if (variables[i].name == item.name) {
			variables[i].minValue = std::min(variables[i].minValue, item.minValue);
			variables[i].maxValue = std::max(variables[i].maxValue, item.maxValue);
			if (item.division > 0.0) variables[i].division = item.division;
			return;
		}
	}
	variables.push_back(item);
}

static void MotionAppendAttachmentRef(MotionInfo &info,
	const std::string &ref, const ttstr &nodeLabel, tjs_real frameTime)
{
	if (ref.empty()) return;
	MotionAttachmentRefInfo item;
	item.ownerObjectName = info.objectName;
	item.ownerMotionName = info.name;
	item.nodeLabel = nodeLabel;
	item.refPath = MotionString(ref);
	std::string objectName;
	std::string motionName;
	std::string iconName;
	if (MotionParseMotionRefPath(ref, objectName, motionName, iconName)) {
		item.refObjectName = MotionString(objectName);
		item.refMotionName = MotionString(motionName);
		item.refIconName = MotionString(iconName);
	}
	item.frameTime = frameTime;
	info.attachmentRefs.push_back(item);
	info.motionRefs.push_back(ref);
	info.motionRefCount++;
}

static void MotionScanFrameContent(psb_t &psb, const psb_objects_t *frame,
	MotionInfo &info, double &maxTime, const ttstr &nodeLabel = ttstr())
{
	double time = 0.0;
	MotionPsbReadMemberNumber(psb, frame, "time", time);

	unsigned char *contentData = frame->get_data("content");
	psb_objects_t *content = nullptr;
	if (MotionUnpackObject(psb, contentData, content)) {
		std::string src;
		if (MotionPsbReadMemberString(psb, content, "src", src)) {
			if (MotionStartsWith(src, "motion/")) {
				MotionAppendAttachmentRef(info, src, nodeLabel, time);
			}
			if (time > maxTime) maxTime = time;
			info.frameCount++;
		} else if (content->size() > 0) {
			std::string icon;
			if (MotionPsbReadMemberString(psb, content, "icon", icon)) {
				psb_objects_t *motionContent = nullptr;
				if (MotionUnpackObject(psb, content->get_data("motion"), motionContent)) {
					std::string rawSrc;
					if (MotionPsbReadMemberString(psb, content, "src", rawSrc)) {
						std::string ref = std::string("motion/") + rawSrc + "/" + icon;
						ref.erase(std::remove(ref.begin(), ref.end(), '\0'), ref.end());
						MotionAppendAttachmentRef(info, ref, nodeLabel, time);
					}
					tjs_real timeOffset = 0.0;
					if (MotionPsbReadMemberNumber(psb, motionContent, "timeOffset", timeOffset) &&
						std::abs((double)timeOffset) > 0.0001) {
						info.timeOffsetNonzeroRefCount++;
					}
					delete motionContent;
				}
			}
			if (time > maxTime) maxTime = time;
			info.frameCount++;
		}
		delete content;
	} else if (contentData) {
		if (time > maxTime) maxTime = time;
		info.frameCount++;
	}
}

static void MotionScanNode(psb_t &psb, unsigned char *nodeData,
	MotionInfo &info, double &maxTime, int depth)
{
	if (!nodeData || depth > 64) return;
	psb_objects_t *node = nullptr;
	if (!MotionUnpackObject(psb, nodeData, node)) return;

	MotionNodeInfo nodeInfo;
	std::string text;
	if (MotionPsbReadMemberString(psb, node, "label", text)) nodeInfo.label = MotionString(text);
	int intValue = 0;
	if (MotionPsbReadMemberInt(psb, node, "type", intValue)) nodeInfo.type = intValue;
	if (MotionPsbReadMemberInt(psb, node, "parameterize", intValue)) nodeInfo.parameterIdx = intValue;

	std::vector<unsigned char*> frames;
	if (MotionPsbReadCollection(psb, node->get_data("frameList"), frames)) {
		nodeInfo.frameCount = (tjs_int)frames.size();
		for (size_t i = 0; i < frames.size(); i++) {
			psb_objects_t *frame = nullptr;
			if (!MotionUnpackObject(psb, frames[i], frame)) continue;
			MotionScanFrameContent(psb, frame, info, maxTime, nodeInfo.label);
			psb_objects_t *content = nullptr;
			if (MotionUnpackObject(psb, frame->get_data("content"), content)) {
				std::string src;
				if (MotionPsbReadMemberString(psb, content, "src", src) &&
					MotionStartsWith(src, "motion/")) {
					nodeInfo.motionRefs.push_back(src);
				}
				std::string icon;
				if (MotionPsbReadMemberString(psb, content, "icon", icon)) {
					psb_objects_t *motionContent = nullptr;
					if (MotionUnpackObject(psb, content->get_data("motion"), motionContent)) {
						std::string rawSrc;
						if (MotionPsbReadMemberString(psb, content, "src", rawSrc)) {
							std::string ref = std::string("motion/") + rawSrc + "/" + icon;
							ref.erase(std::remove(ref.begin(), ref.end(), '\0'), ref.end());
							nodeInfo.motionRefs.push_back(ref);
						}
						delete motionContent;
					}
				}
				delete content;
			}
			delete frame;
		}
	}

	std::vector<unsigned char*> children;
	if (MotionPsbReadCollection(psb, node->get_data("children"), children)) {
		nodeInfo.childCount = (tjs_int)children.size();
		for (size_t i = 0; i < children.size(); i++) {
			MotionScanNode(psb, children[i], info, maxTime, depth + 1);
		}
	}
	if (nodeInfo.label.GetLen() > 0 || nodeInfo.frameCount > 0 || nodeInfo.childCount > 0 ||
		!nodeInfo.motionRefs.empty()) {
		info.nodes.push_back(nodeInfo);
	}
	delete node;
}

static void MotionReadParameters(psb_t &psb, const psb_objects_t *motion, MotionInfo &info)
{
	std::vector<unsigned char*> entries;
	bool found = false;
	if (MotionPsbReadCollection(psb, motion->get_data("parameter"), entries)) {
		for (size_t i = 0; i < entries.size(); i++) {
			psb_objects_t *param = nullptr;
			if (!MotionUnpackObject(psb, entries[i], param)) continue;
			std::string id;
			MotionVariableInfo var;
			if (MotionPsbReadMemberString(psb, param, "id", id)) var.name = MotionString(id);
			double value = 0.0;
			if (MotionPsbReadMemberNumber(psb, param, "rangeBegin", value)) var.minValue = value;
			if (MotionPsbReadMemberNumber(psb, param, "rangeEnd", value)) var.maxValue = value;
			if (MotionPsbReadMemberNumber(psb, param, "division", value)) var.division = value;
			MotionAddVariable(info.variables, var);
			found = true;
			delete param;
		}
	}
	if (found) return;

	psb_objects_t *param = nullptr;
	if (MotionUnpackObject(psb, motion->get_data("parameterize"), param)) {
		std::string id;
		MotionVariableInfo var;
		if (MotionPsbReadMemberString(psb, param, "id", id)) var.name = MotionString(id);
		double value = 0.0;
		if (MotionPsbReadMemberNumber(psb, param, "rangeBegin", value)) var.minValue = value;
		if (MotionPsbReadMemberNumber(psb, param, "rangeEnd", value)) var.maxValue = value;
		if (MotionPsbReadMemberNumber(psb, param, "division", value)) var.division = value;
		if (var.division <= 0.0) {
			var.division = std::abs((double)(var.maxValue - var.minValue));
		}
		MotionAddVariable(info.variables, var);
		if (var.name.GetLen() > 0) info.parameterIdx = 0;
		delete param;
	}
}

static bool MotionNameMatches(const ttstr &a, const ttstr &b);

static MotionInfo MotionReadMotion(psb_t &psb, const psb_objects_t *motion,
	const ttstr &objectName, const ttstr &motionName)
{
	MotionInfo info;
	info.objectName = objectName;
	info.name = motionName;
	double value = 0.0;
	if (MotionPsbReadMemberNumber(psb, motion, "lastTime", value)) info.lastTime = value;
	if (MotionPsbReadMemberNumber(psb, motion, "loopTime", value)) info.loopTime = value;
	int intValue = 0;
	if (MotionPsbReadMemberInt(psb, motion, "parameterize", intValue)) {
		info.parameterIdx = intValue;
	}

	std::vector<unsigned char*> layers;
	double maxTime = 0.0;
	if (MotionPsbReadCollection(psb, motion->get_data("layer"), layers)) {
		for (size_t i = 0; i < layers.size(); i++) {
			MotionScanNode(psb, layers[i], info, maxTime, 0);
		}
	}
	info.selfSyncTime = maxTime;
	info.syncTime = std::max(info.lastTime, info.selfSyncTime);
	MotionReadParameters(psb, motion, info);
	return info;
}

static void MotionResolveNestedSyncTimes(MotionResource &resource)
{
	resource.attachmentRefs.clear();
	for (int pass = 0; pass < 8; pass++) {
		bool changed = false;
		for (size_t i = 0; i < resource.motions.size(); i++) {
			MotionInfo &motion = resource.motions[i];
			tjs_real before = motion.syncTime;
			for (size_t r = 0; r < motion.attachmentRefs.size(); r++) {
				MotionAttachmentRefInfo &ref = motion.attachmentRefs[r];
				if (ref.refObjectName.GetLen() <= 0 || ref.refMotionName.GetLen() <= 0) continue;
				for (size_t j = 0; j < resource.motions.size(); j++) {
					if (MotionNameMatches(resource.motions[j].objectName, ref.refObjectName) &&
						MotionNameMatches(resource.motions[j].name, ref.refMotionName)) {
						ref.resolved = true;
						motion.syncTime = std::max(motion.syncTime, resource.motions[j].syncTime);
					}
				}
				ref.external = !ref.resolved;
			}
			if (motion.syncTime != before) changed = true;
		}
		if (!changed) break;
	}
	for (size_t oi = 0; oi < resource.objects.size(); oi++) {
		MotionObjectInfo &object = resource.objects[oi];
		for (size_t mi = 0; mi < object.motions.size(); mi++) {
			for (size_t ri = 0; ri < resource.motions.size(); ri++) {
				if (MotionNameMatches(object.name, resource.motions[ri].objectName) &&
					MotionNameMatches(object.motions[mi].name, resource.motions[ri].name)) {
					object.motions[mi] = resource.motions[ri];
					break;
				}
			}
		}
	}
	for (size_t i = 0; i < resource.motions.size(); i++) {
		for (size_t r = 0; r < resource.motions[i].attachmentRefs.size(); r++) {
			MotionAttachmentRefInfo ref = resource.motions[i].attachmentRefs[r];
			bool resolved = false;
			for (size_t j = 0; j < resource.motions.size(); j++) {
				if (MotionNameMatches(resource.motions[j].objectName, ref.refObjectName) &&
					MotionNameMatches(resource.motions[j].name, ref.refMotionName)) {
					resolved = true;
					break;
				}
			}
			ref.resolved = resolved;
			ref.external = !resolved;
			resource.motions[i].attachmentRefs[r] = ref;
			resource.attachmentRefs.push_back(ref);
		}
	}
}

static void MotionReadMetadataVariables(psb_t &psb, const psb_objects_t *metadata,
	MotionResource &resource)
{
	std::vector<unsigned char*> entries;
	if (!MotionPsbReadCollection(psb, metadata->get_data("variableList"), entries)) return;
	for (size_t i = 0; i < entries.size(); i++) {
		psb_objects_t *item = nullptr;
		if (!MotionUnpackObject(psb, entries[i], item)) continue;
		std::string label;
		MotionVariableInfo var;
		if (MotionPsbReadMemberString(psb, item, "label", label)) var.name = MotionString(label);
		MotionAddVariable(resource.variables, var);
		delete item;
	}
}

static MotionSelectorInfo MotionReadSelector(psb_t &psb, const psb_objects_t *selector)
{
	MotionSelectorInfo info;
	std::string label;
	if (MotionPsbReadMemberString(psb, selector, "label", label)) {
		info.label = MotionString(label);
	}
	std::vector<unsigned char*> entries;
	if (MotionPsbReadCollection(psb, selector->get_data("optionList"), entries)) {
		for (size_t i = 0; i < entries.size(); i++) {
			psb_objects_t *item = nullptr;
			if (!MotionUnpackObject(psb, entries[i], item)) continue;
			MotionSelectorOptionInfo option;
			std::string optionLabel;
			double value = 0.0;
			if (MotionPsbReadMemberString(psb, item, "label", optionLabel)) {
				option.label = MotionString(optionLabel);
			}
			if (MotionPsbReadMemberNumber(psb, item, "offValue", value)) option.offValue = value;
			if (MotionPsbReadMemberNumber(psb, item, "onValue", value)) option.onValue = value;
			if (option.label.GetLen() > 0) info.options.push_back(option);
			delete item;
		}
	}
	return info;
}

static void MotionReadMetadataSelectors(psb_t &psb, const psb_objects_t *metadata,
	MotionResource &resource)
{
	std::vector<unsigned char*> selectors;
	if (!MotionPsbReadCollection(psb, metadata->get_data("selectorControl"), selectors)) return;
	for (size_t i = 0; i < selectors.size(); i++) {
		psb_objects_t *selector = nullptr;
		if (!MotionUnpackObject(psb, selectors[i], selector)) continue;
		MotionSelectorInfo info = MotionReadSelector(psb, selector);
		if (info.label.GetLen() > 0 && !info.options.empty()) {
			resource.selectors.push_back(info);
		}
		delete selector;
	}
}

static MotionTimelineFrameInfo MotionReadTimelineFrame(psb_t &psb, const psb_objects_t *frame)
{
	MotionTimelineFrameInfo info;
	double number = 0.0;
	if (MotionPsbReadMemberNumber(psb, frame, "time", number)) info.time = number;
	int intValue = 0;
	if (MotionPsbReadMemberInt(psb, frame, "type", intValue)) info.type = intValue;
	psb_objects_t *content = nullptr;
	if (MotionUnpackObject(psb, frame->get_data("content"), content)) {
		info.hasContent = true;
		if (MotionPsbReadMemberNumber(psb, content, "value", number)) info.value = number;
		if (MotionPsbReadMemberNumber(psb, content, "easing", number)) info.easing = number;
		delete content;
	}
	return info;
}

static MotionTimelineVariableInfo MotionReadTimelineVariable(psb_t &psb, const psb_objects_t *variable)
{
	MotionTimelineVariableInfo info;
	std::string label;
	if (MotionPsbReadMemberString(psb, variable, "label", label)) info.label = MotionString(label);
	std::vector<unsigned char*> frames;
	if (MotionPsbReadCollection(psb, variable->get_data("frameList"), frames)) {
		for (size_t i = 0; i < frames.size(); i++) {
			psb_objects_t *frame = nullptr;
			if (!MotionUnpackObject(psb, frames[i], frame)) continue;
			info.frames.push_back(MotionReadTimelineFrame(psb, frame));
			delete frame;
		}
	}
	return info;
}

static MotionTimelineInfo MotionReadTimeline(psb_t &psb, const psb_objects_t *timeline)
{
	MotionTimelineInfo info;
	std::string label;
	if (MotionPsbReadMemberString(psb, timeline, "label", label)) info.label = MotionString(label);
	int intValue = 0;
	if (MotionPsbReadMemberInt(psb, timeline, "lastTime", intValue)) info.lastTime = intValue;
	if (MotionPsbReadMemberInt(psb, timeline, "loopBegin", intValue)) info.loopBegin = intValue;
	if (MotionPsbReadMemberInt(psb, timeline, "loopEnd", intValue)) info.loopEnd = intValue;
	if (MotionPsbReadMemberInt(psb, timeline, "diff", intValue)) info.diff = intValue;

	std::vector<unsigned char*> variables;
	if (MotionPsbReadCollection(psb, timeline->get_data("variableList"), variables)) {
		for (size_t i = 0; i < variables.size(); i++) {
			psb_objects_t *variable = nullptr;
			if (!MotionUnpackObject(psb, variables[i], variable)) continue;
			info.variables.push_back(MotionReadTimelineVariable(psb, variable));
			delete variable;
		}
	}
	return info;
}

static void MotionReadMetadataTimelines(psb_t &psb, const psb_objects_t *metadata,
	MotionResource &resource)
{
	std::vector<unsigned char*> timelines;
	if (!MotionPsbReadCollection(psb, metadata->get_data("timelineControl"), timelines)) return;
	for (size_t i = 0; i < timelines.size(); i++) {
		psb_objects_t *timeline = nullptr;
		if (!MotionUnpackObject(psb, timelines[i], timeline)) continue;
		resource.timelines.push_back(MotionReadTimeline(psb, timeline));
		delete timeline;
	}
}

static void MotionReadMetadata(psb_t &psb, const psb_objects_t *root, MotionResource &resource)
{
	psb_objects_t *metadata = nullptr;
	if (!MotionUnpackObject(psb, root->get_data("metadata"), metadata)) {
		resource.isMotion = true;
		return;
	}
	resource.hasMetadata = true;
	resource.isMotion = false;

	psb_objects_t *base = nullptr;
	if (MotionUnpackObject(psb, metadata->get_data("base"), base)) {
		std::string value;
		if (MotionPsbReadMemberString(psb, base, "chara", value)) resource.chara = MotionString(value);
		if (MotionPsbReadMemberString(psb, base, "motion", value)) resource.motion = MotionString(value);
		delete base;
	}
	MotionReadMetadataVariables(psb, metadata, resource);
	MotionReadMetadataSelectors(psb, metadata, resource);
	MotionReadMetadataTimelines(psb, metadata, resource);
	delete metadata;
}
