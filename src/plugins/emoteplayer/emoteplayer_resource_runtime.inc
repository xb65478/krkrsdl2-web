class Motion {
public:
	static bool getD3DAvailable() { return false; }
	static bool getEnableD3D() { return enableD3D; }
	static void setEnableD3D(bool value) { enableD3D = value; }

private:
	inline static bool enableD3D = false;
};

#ifdef __EMSCRIPTEN__
struct MotionGlobalResourceCacheEntry {
	std::shared_ptr<MotionResource> resource;
	tjs_uint64 byteSize = 0;
	unsigned long long hitCount = 0;
	unsigned long long sequence = 0;
};

static std::map<std::string, MotionGlobalResourceCacheEntry> &MotionGlobalResourceCache()
{
	static std::map<std::string, MotionGlobalResourceCacheEntry> cache;
	return cache;
}

static std::vector<std::string> &MotionGlobalResourceCacheOrder()
{
	static std::vector<std::string> order;
	return order;
}

static unsigned long long &MotionGlobalResourceCacheSequence()
{
	static unsigned long long sequence = 0;
	return sequence;
}

static size_t MotionGlobalResourceCacheMaxEntries()
{
	return 16;
}

static tjs_uint64 MotionGlobalResourceCacheMaxBytes()
{
	return 96ULL * 1024ULL * 1024ULL;
}

static tjs_uint64 MotionGlobalResourceCacheTotalBytes()
{
	tjs_uint64 total = 0;
	for (const auto &item : MotionGlobalResourceCache()) {
		total += item.second.byteSize;
	}
	return total;
}

static void MotionGlobalResourceCacheEraseKey(const std::string &key, const char *reason)
{
	auto &cache = MotionGlobalResourceCache();
	auto it = cache.find(key);
	if (it == cache.end()) return;
	tjs_uint64 bytes = it->second.byteSize;
	unsigned long long hits = it->second.hitCount;
	cache.erase(it);
	auto &order = MotionGlobalResourceCacheOrder();
	order.erase(std::remove(order.begin(), order.end(), key), order.end());
	if (TVPPerfEnabled()) {
		fprintf(stderr,
			"[PERF] emote.resource.global_cache eviction key=%s reason=%s bytes=%llu hits=%llu entries=%zu total_bytes=%llu\n",
			key.c_str(),
			reason ? reason : "unknown",
			(unsigned long long)bytes,
			hits,
			cache.size(),
			(unsigned long long)MotionGlobalResourceCacheTotalBytes());
	}
}

static std::shared_ptr<MotionResource> MotionCloneGlobalResource(
	const std::shared_ptr<MotionResource> &source, tjs_int id)
{
	if (!source) return nullptr;
	std::shared_ptr<MotionResource> clone = std::make_shared<MotionResource>(*source);
	clone->id = id;
	clone->timeSemanticsDiag = MotionTimeSemanticsDiag();
	clone->limitSemanticsDiag = MotionLimitSemanticsDiag();
	clone->motionRenderMethodSemantics = MotionRenderMethodSemanticsDiag();
	clone->affineFrontMeshDiag = MotionAffineFrontMeshDiag();
	clone->meshDeformationDiag = MotionMeshDeformationDiag();
	return clone;
}

static std::shared_ptr<MotionResource> MotionGlobalResourceCacheFind(
	const std::string &key, tjs_int id, unsigned long long *hitCountOut)
{
	auto &cache = MotionGlobalResourceCache();
	auto it = cache.find(key);
	if (it == cache.end()) return nullptr;
	it->second.hitCount++;
	if (hitCountOut) *hitCountOut = it->second.hitCount;
	return MotionCloneGlobalResource(it->second.resource, id);
}

static void MotionGlobalResourceCacheStore(
	const std::string &key, const std::shared_ptr<MotionResource> &resource)
{
	if (!resource || !resource->opened || !resource->parsed) return;
	auto &cache = MotionGlobalResourceCache();
	auto &order = MotionGlobalResourceCacheOrder();
	auto existing = cache.find(key);
	if (existing == cache.end()) {
		order.push_back(key);
	}
	MotionGlobalResourceCacheEntry entry;
	entry.resource = MotionCloneGlobalResource(resource, resource->id);
	entry.byteSize = resource->byteSize;
	entry.hitCount = existing != cache.end() ? existing->second.hitCount : 0;
	entry.sequence = ++MotionGlobalResourceCacheSequence();
	cache[key] = entry;
	while (cache.size() > MotionGlobalResourceCacheMaxEntries()) {
		if (order.empty()) break;
		MotionGlobalResourceCacheEraseKey(order.front(), "entry-limit");
	}
	while (MotionGlobalResourceCacheTotalBytes() > MotionGlobalResourceCacheMaxBytes()) {
		if (order.empty()) break;
		MotionGlobalResourceCacheEraseKey(order.front(), "byte-limit");
	}
}
#endif

class ResourceManager {
public:
	ResourceManager() = default;
	ResourceManager(iTJSDispatch2 *kagWindow, tjs_int cacheSize)
		: window(kagWindow), cacheSize(cacheSize)
	{
		KRKR_LOG_L2("[emoteplayer] ResourceManager created\n");
	}

	tTJSVariant load(tTJSString path)
	{
		ttstr key(path);
#ifdef __EMSCRIPTEN__
		std::string rawPath = key.AsNarrowStdString();
		bool tracePsb = rawPath.find(".psb") != std::string::npos || rawPath.find(".PSB") != std::string::npos;
		tjs_uint64 perfStart = tracePsb ? TVPGetTickCount() : 0;
		if (tracePsb) {
			KRKR_LOG_L2("[EMOTE-RESOURCE] load begin path=%s\n", rawPath.c_str());
		}
#endif
		EnsurePSBFilePlugin();
#ifdef __EMSCRIPTEN__
		tjs_uint64 perfAfterEnsure = tracePsb ? TVPGetTickCount() : 0;
#endif
		std::string cacheKey = MotionCacheKey(key);
#ifdef __EMSCRIPTEN__
		tjs_uint64 perfAfterCacheKey = tracePsb ? TVPGetTickCount() : 0;
		if (tracePsb) {
			KRKR_LOG_L3("[EMOTE-RESOURCE] load cache-key path=%s key=%s\n",
				rawPath.c_str(), cacheKey.c_str());
		}
#endif
#ifdef __EMSCRIPTEN__
		tjs_uint64 perfCacheFindStart = tracePsb ? TVPGetTickCount() : 0;
#endif
		auto cached = cache.find(cacheKey);
#ifdef __EMSCRIPTEN__
		tjs_uint64 perfAfterCacheFind = tracePsb ? TVPGetTickCount() : 0;
#endif
		if (cached != cache.end()) {
#ifdef __EMSCRIPTEN__
			if (tracePsb) {
				KRKR_LOG_L2("[EMOTE-RESOURCE] load cache-hit path=%s\n", rawPath.c_str());
			}
#endif
			currentResource = cached->second;
#ifdef __EMSCRIPTEN__
			tjs_uint64 perfRegisterStart = tracePsb ? TVPGetTickCount() : 0;
#endif
			RegisterMotionResource(currentResource);
#ifdef __EMSCRIPTEN__
			tjs_uint64 perfAfterRegister = tracePsb ? TVPGetTickCount() : 0;
#endif
			loadedPath = currentResource ? currentResource->storage : key;
			loadedByteSize = currentResource ? currentResource->byteSize : 0;
			loaded = currentResource && currentResource->opened;
#ifdef __EMSCRIPTEN__
			if (tracePsb) {
				KRKR_LOG_L3("[EMOTE-RESOURCE] load make-result-begin path=%s cached=1\n", rawPath.c_str());
			}
#endif
#ifdef __EMSCRIPTEN__
			tjs_uint64 perfResultStart = tracePsb ? TVPGetTickCount() : 0;
#endif
			tTJSVariant result = currentResource ? makeResourceDictionary(*currentResource) : MakeEmptyDictionary();
#ifdef __EMSCRIPTEN__
			tjs_uint64 perfAfterResult = tracePsb ? TVPGetTickCount() : 0;
			if (tracePsb) {
				KRKR_LOG_L3("[EMOTE-RESOURCE] load make-result-done path=%s cached=1\n", rawPath.c_str());
				KRKR_LOG_L3("[EMOTE-RESOURCE] load return-root path=%s cached=1\n", rawPath.c_str());
				fprintf(stderr,
					"[PERF] emote.resource.load path=%s normalized_key=%s cache=manager-hit resource_id=%d manager_entries=%zu global_entries=%zu global_bytes=%llu opened=%d parsed=%d bytes=%llu motions=%zu layouts=%zu objects=%zu variables=%zu ensure_ms=%llu cache_key_ms=%llu cache_find_ms=%llu register_ms=%llu result_ms=%llu total_ms=%llu\n",
					rawPath.c_str(),
					cacheKey.c_str(),
					currentResource ? currentResource->id : -1,
					cache.size(),
					MotionGlobalResourceCache().size(),
					(unsigned long long)MotionGlobalResourceCacheTotalBytes(),
					currentResource && currentResource->opened ? 1 : 0,
					currentResource && currentResource->parsed ? 1 : 0,
					(unsigned long long)(currentResource ? currentResource->byteSize : 0),
					currentResource ? currentResource->motions.size() : 0,
					currentResource ? currentResource->layouts.size() : 0,
					currentResource ? currentResource->objects.size() : 0,
					currentResource ? currentResource->variables.size() : 0,
					(unsigned long long)(perfAfterEnsure - perfStart),
					(unsigned long long)(perfAfterCacheKey - perfAfterEnsure),
					(unsigned long long)(perfAfterCacheFind - perfCacheFindStart),
					(unsigned long long)(perfAfterRegister - perfRegisterStart),
					(unsigned long long)(perfAfterResult - perfResultStart),
					(unsigned long long)(perfAfterResult - perfStart));
			}
#endif
			return result;
		}

#ifdef __EMSCRIPTEN__
		tjs_uint64 perfGlobalFindStart = tracePsb ? TVPGetTickCount() : 0;
		unsigned long long globalHitCount = 0;
		std::shared_ptr<MotionResource> globalResource =
			MotionGlobalResourceCacheFind(cacheKey, nextResourceId, &globalHitCount);
		tjs_uint64 perfAfterGlobalFind = tracePsb ? TVPGetTickCount() : 0;
		if (globalResource) {
			nextResourceId++;
			cache[cacheKey] = globalResource;
			resources.push_back(globalResource);
			tjs_uint64 perfAfterCacheInsert = tracePsb ? TVPGetTickCount() : 0;
			tjs_uint64 perfRegisterStart = tracePsb ? TVPGetTickCount() : 0;
			RegisterMotionResource(globalResource);
			tjs_uint64 perfAfterRegister = tracePsb ? TVPGetTickCount() : 0;
			currentResource = globalResource;
			loadedPath = currentResource->storage;
			loadedByteSize = currentResource->byteSize;
			loaded = currentResource->opened;
			tjs_uint64 perfResultStart = tracePsb ? TVPGetTickCount() : 0;
			tTJSVariant result = makeResourceDictionary(*currentResource);
			tjs_uint64 perfAfterResult = tracePsb ? TVPGetTickCount() : 0;
			if (tracePsb) {
				fprintf(stderr,
					"[PERF] emote.resource.load path=%s normalized_key=%s cache=global-hit resource_id=%d manager_entries=%zu global_entries=%zu global_bytes=%llu global_hit_count=%llu opened=%d parsed=%d bytes=%llu motions=%zu layouts=%zu objects=%zu variables=%zu ensure_ms=%llu cache_key_ms=%llu cache_find_ms=%llu global_find_ms=%llu read_ms=0 parse_ms=0 cache_insert_ms=%llu register_ms=%llu result_ms=%llu total_ms=%llu\n",
					rawPath.c_str(),
					cacheKey.c_str(),
					currentResource->id,
					cache.size(),
					MotionGlobalResourceCache().size(),
					(unsigned long long)MotionGlobalResourceCacheTotalBytes(),
					globalHitCount,
					currentResource->opened ? 1 : 0,
					currentResource->parsed ? 1 : 0,
					(unsigned long long)currentResource->byteSize,
					currentResource->motions.size(),
					currentResource->layouts.size(),
					currentResource->objects.size(),
					currentResource->variables.size(),
					(unsigned long long)(perfAfterEnsure - perfStart),
					(unsigned long long)(perfAfterCacheKey - perfAfterEnsure),
					(unsigned long long)(perfAfterCacheFind - perfCacheFindStart),
					(unsigned long long)(perfAfterGlobalFind - perfGlobalFindStart),
					(unsigned long long)(perfAfterCacheInsert - perfAfterGlobalFind),
					(unsigned long long)(perfAfterRegister - perfRegisterStart),
					(unsigned long long)(perfAfterResult - perfResultStart),
					(unsigned long long)(perfAfterResult - perfStart));
			}
			return result;
		}
#endif

		std::vector<unsigned char> bytes;
		tjs_uint64 byteSize = 0;
#ifdef __EMSCRIPTEN__
		tjs_uint64 perfReadStart = tracePsb ? TVPGetTickCount() : 0;
		if (tracePsb) {
			KRKR_LOG_L2("[EMOTE-RESOURCE] load read-begin path=%s\n", rawPath.c_str());
		}
#endif
		bool opened = ReadStorageBytes(key, bytes, &byteSize);
#ifdef __EMSCRIPTEN__
		tjs_uint64 perfAfterRead = tracePsb ? TVPGetTickCount() : 0;
		if (tracePsb) {
			KRKR_LOG_L2("[EMOTE-RESOURCE] load read-done path=%s opened=%d size=%llu bytes=%zu\n",
				rawPath.c_str(), opened ? 1 : 0,
				(unsigned long long)byteSize, bytes.size());
		}
#endif
		std::shared_ptr<MotionResource> resource = std::make_shared<MotionResource>();
		resource->id = nextResourceId++;
		resource->storage = key;
		resource->byteSize = byteSize;
		resource->opened = opened;
#ifdef __EMSCRIPTEN__
		tjs_uint64 perfParseStart = tracePsb ? TVPGetTickCount() : 0;
#endif
		if (opened) {
#ifdef __EMSCRIPTEN__
			if (tracePsb) {
				KRKR_LOG_L2("[EMOTE-RESOURCE] load parse-begin path=%s size=%llu\n",
					rawPath.c_str(), (unsigned long long)byteSize);
			}
#endif
			ParseMotionResource(key, bytes, byteSize, *resource);
#ifdef __EMSCRIPTEN__
			if (tracePsb) {
				KRKR_LOG_L2("[EMOTE-RESOURCE] load parse-done path=%s parsed=%d motions=%zu layouts=%zu\n",
					rawPath.c_str(), resource->parsed ? 1 : 0,
					resource->motions.size(), resource->layouts.size());
				}
#endif
		}
#ifdef __EMSCRIPTEN__
		tjs_uint64 perfAfterParse = tracePsb ? TVPGetTickCount() : 0;
#endif

		cache[cacheKey] = resource;
		resources.push_back(resource);
#ifdef __EMSCRIPTEN__
		if (resource->opened && resource->parsed) {
			MotionGlobalResourceCacheStore(cacheKey, resource);
		}
#endif
#ifdef __EMSCRIPTEN__
		tjs_uint64 perfAfterCacheInsert = tracePsb ? TVPGetTickCount() : 0;
#endif
#ifdef __EMSCRIPTEN__
		if (tracePsb) {
			KRKR_LOG_L3("[EMOTE-RESOURCE] load post-cache path=%s\n", rawPath.c_str());
		}
#endif
#ifdef __EMSCRIPTEN__
		tjs_uint64 perfRegisterStart = tracePsb ? TVPGetTickCount() : 0;
#endif
		RegisterMotionResource(resource);
#ifdef __EMSCRIPTEN__
		tjs_uint64 perfAfterRegister = tracePsb ? TVPGetTickCount() : 0;
#endif
#ifdef __EMSCRIPTEN__
		if (tracePsb) {
			KRKR_LOG_L3("[EMOTE-RESOURCE] load post-register path=%s\n", rawPath.c_str());
		}
#endif
		currentResource = resource;
		loadedPath = key;
		loadedByteSize = byteSize;
		loaded = opened;
		KRKR_LOG_L2("[emoteplayer] ResourceManager.load: %s%s%s\n",
			key.AsNarrowStdString().c_str(),
			opened ? " (opened)" : " (open failed)",
			resource->parsed ? " (motion parsed)" : "");
#ifdef __EMSCRIPTEN__
		if (tracePsb) {
			KRKR_LOG_L3("[EMOTE-RESOURCE] load make-result-begin path=%s cached=0\n", rawPath.c_str());
		}
#endif
#ifdef __EMSCRIPTEN__
		tjs_uint64 perfResultStart = tracePsb ? TVPGetTickCount() : 0;
#endif
		tTJSVariant result = makeResourceDictionary(*resource);
#ifdef __EMSCRIPTEN__
		tjs_uint64 perfAfterResult = tracePsb ? TVPGetTickCount() : 0;
		if (tracePsb) {
			KRKR_LOG_L3("[EMOTE-RESOURCE] load make-result-done path=%s cached=0\n", rawPath.c_str());
			KRKR_LOG_L3("[EMOTE-RESOURCE] load return-root path=%s cached=0\n", rawPath.c_str());
			fprintf(stderr,
				"[PERF] emote.resource.load path=%s normalized_key=%s cache=miss resource_id=%d manager_entries=%zu global_entries=%zu global_bytes=%llu opened=%d parsed=%d bytes=%llu motions=%zu layouts=%zu objects=%zu variables=%zu ensure_ms=%llu cache_key_ms=%llu cache_find_ms=%llu global_find_ms=%llu read_ms=%llu parse_ms=%llu cache_insert_ms=%llu register_ms=%llu result_ms=%llu total_ms=%llu\n",
				rawPath.c_str(),
				cacheKey.c_str(),
				resource->id,
				cache.size(),
				MotionGlobalResourceCache().size(),
				(unsigned long long)MotionGlobalResourceCacheTotalBytes(),
				resource->opened ? 1 : 0,
				resource->parsed ? 1 : 0,
				(unsigned long long)resource->byteSize,
				resource->motions.size(),
				resource->layouts.size(),
				resource->objects.size(),
				resource->variables.size(),
				(unsigned long long)(perfAfterEnsure - perfStart),
				(unsigned long long)(perfAfterCacheKey - perfAfterEnsure),
				(unsigned long long)(perfAfterCacheFind - perfCacheFindStart),
				(unsigned long long)(perfAfterGlobalFind - perfGlobalFindStart),
				(unsigned long long)(perfAfterRead - perfReadStart),
				(unsigned long long)(perfAfterParse - perfParseStart),
				(unsigned long long)(perfAfterCacheInsert - perfAfterParse),
				(unsigned long long)(perfAfterRegister - perfRegisterStart),
				(unsigned long long)(perfAfterResult - perfResultStart),
				(unsigned long long)(perfAfterResult - perfStart));
		}
#endif
		return result;
	}

	tTJSVariant loadPSB(tTJSString path) { return load(path); }
	tTJSVariant loadFromMemory(tTJSVariant buffer) { return MakeEmptyDictionary(); }
	tTJSVariant loadPSBFromMemory(tTJSVariant buffer) { return MakeEmptyDictionary(); }

	void unload(tTJSString path)
	{
		ttstr key(path);
		KRKR_LOG_L2("[emoteplayer] ResourceManager.unload: %s\n",
			key.AsNarrowStdString().c_str());
		std::string cacheKey = MotionCacheKey(key);
		auto it = cache.find(cacheKey);
		if (it != cache.end()) {
			std::shared_ptr<MotionResource> removing = it->second;
			cache.erase(it);
			resources.erase(std::remove(resources.begin(), resources.end(), removing), resources.end());
			if (currentResource == removing) currentResource.reset();
		}
		if (!currentResource && !resources.empty()) currentResource = resources.back();
		if (currentResource) {
			loadedPath = currentResource->storage;
			loadedByteSize = currentResource->byteSize;
			loaded = currentResource->opened;
		} else {
			loadedPath.Clear();
			loadedByteSize = 0;
			loaded = false;
		}
	}

	void unloadAll()
	{
		KRKR_LOG_L2("[emoteplayer] ResourceManager.unloadAll\n");
#ifdef __EMSCRIPTEN__
		fprintf(stderr, "[PERF] emote.resource.unloadAll cache_entries=%zu resources=%zu current=%d\n",
			cache.size(), resources.size(), currentResource ? 1 : 0);
#endif
		cache.clear();
		resources.clear();
		currentResource.reset();
		loadedPath.Clear();
		loadedByteSize = 0;
		loaded = false;
	}

	void clearCache()
	{
		KRKR_LOG_L2("[emoteplayer] ResourceManager.clearCache\n");
#ifdef __EMSCRIPTEN__
		fprintf(stderr, "[PERF] emote.resource.clearCache cache_entries=%zu resources=%zu current=%d\n",
			cache.size(), resources.size(), currentResource ? 1 : 0);
#endif
		unloadAll();
	}

	static void setEmotePSBDecryptSeed(tjs_int decryptKey)
	{
		emotePSBDecryptSeed = decryptKey;
	}

	static void setEmotePSBDecryptFunc(tTJSVariant functionClosure)
	{
		emotePSBDecryptFunc = functionClosure;
	}

	tjs_int getModelCount() const { return (tjs_int)resources.size(); }
	tjs_int getModelWidth(tjs_int id) const
	{
		std::shared_ptr<MotionResource> resource = getResourceAt(id);
		return resource ? resource->width : 0;
	}
	tjs_int getModelHeight(tjs_int id) const
	{
		std::shared_ptr<MotionResource> resource = getResourceAt(id);
		return resource ? resource->height : 0;
	}
	tjs_int getModelFrameCount(tjs_int id) const
	{
		std::shared_ptr<MotionResource> resource = getResourceAt(id);
		return resource ? getFrameCount(*resource) : 0;
	}
	tjs_int getModelMotionCount(tjs_int id) const
	{
		std::shared_ptr<MotionResource> resource = getResourceAt(id);
		return resource ? (tjs_int)resource->motions.size() : 0;
	}
	tTJSString getModelMotionName(tjs_int id, tjs_int index) const
	{
		std::shared_ptr<MotionResource> resource = getResourceAt(id);
		if (!resource || index < 0 || index >= (tjs_int)resource->motions.size()) return TJS_W("");
		return resource->motions[(size_t)index].name;
	}
	tjs_int getModelVariableCount(tjs_int id) const
	{
		std::shared_ptr<MotionResource> resource = getResourceAt(id);
		return resource ? (tjs_int)resource->variables.size() : 0;
	}
	tTJSString getModelVariableName(tjs_int id, tjs_int index) const
	{
		std::shared_ptr<MotionResource> resource = getResourceAt(id);
		if (!resource || index < 0 || index >= (tjs_int)resource->variables.size()) return TJS_W("");
		return resource->variables[(size_t)index].name;
	}
	tjs_real getModelVariableMin(tjs_int id, tjs_int index) const
	{
		std::shared_ptr<MotionResource> resource = getResourceAt(id);
		if (!resource || index < 0 || index >= (tjs_int)resource->variables.size()) return 0.0;
		return resource->variables[(size_t)index].minValue;
	}
	tjs_real getModelVariableMax(tjs_int id, tjs_int index) const
	{
		std::shared_ptr<MotionResource> resource = getResourceAt(id);
		if (!resource || index < 0 || index >= (tjs_int)resource->variables.size()) return 0.0;
		return resource->variables[(size_t)index].maxValue;
	}
	tTJSVariant getResourceDump(tjs_int id) const
	{
		std::shared_ptr<MotionResource> resource = id < 0 ? currentResource : getResourceAt(id);
		if (!resource) return MakeEmptyDictionary();
		return MakeMotionResourceDump(*resource);
	}
	bool isLoaded(tjs_int id) const
	{
		if (id < 0) return loaded;
		std::shared_ptr<MotionResource> resource = getResourceAt(id);
		return resource && resource->opened;
	}
	const ttstr &getLoadedPath() const { return loadedPath; }
	bool hasLoadedPath() const { return loadedPath.GetLen() > 0; }
	std::shared_ptr<MotionResource> getCurrentResource() const { return currentResource; }
	std::shared_ptr<MotionResource> findResource(tTJSString path) const
	{
		ttstr key(path);
		auto it = cache.find(MotionCacheKey(key));
		if (it != cache.end()) return it->second;
		return nullptr;
	}
	private:
	static tjs_int getFrameCount(const MotionResource &resource)
	{
		tjs_int count = 0;
		for (size_t i = 0; i < resource.motions.size(); i++) {
			count = std::max(count, resource.motions[i].frameCount);
			count = std::max(count, (tjs_int)std::ceil(resource.motions[i].syncTime));
		}
		return count;
	}

	std::shared_ptr<MotionResource> getResourceAt(tjs_int id) const
	{
		if (id < 0 || id >= (tjs_int)resources.size()) return nullptr;
		return resources[(size_t)id];
	}

	static tTJSVariant makeStringArray(const std::vector<ttstr> &items)
	{
		iTJSDispatch2 *array = TJSCreateArrayObject();
		for (size_t i = 0; i < items.size(); i++) {
			tTJSVariant value(items[i]);
			array->PropSetByNum(TJS_MEMBERENSURE, (tjs_int)i, &value, array);
		}
		tTJSVariant result(array, array);
		array->Release();
		return result;
	}

	static tTJSVariant makeResourceDictionary(const MotionResource &resource)
	{
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		SetTjsProperty(dict, TJS_W("id"), tTJSVariant((tjs_int)resource.id));
		SetTjsProperty(dict, TJS_W("storage"), tTJSVariant(resource.storage));
		SetTjsProperty(dict, TJS_W("byteSize"), tTJSVariant((tjs_int64)resource.byteSize));
		SetTjsProperty(dict, TJS_W("valid"), tTJSVariant((tjs_int)(resource.opened ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("parsed"), tTJSVariant((tjs_int)(resource.parsed ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("isMotion"), tTJSVariant((tjs_int)(resource.isMotion ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("spec"), tTJSVariant(resource.spec));
		SetTjsProperty(dict, TJS_W("label"), tTJSVariant(resource.label));
		SetTjsProperty(dict, TJS_W("width"), tTJSVariant(resource.width));
		SetTjsProperty(dict, TJS_W("height"), tTJSVariant(resource.height));
		SetTjsProperty(dict, TJS_W("chara"), tTJSVariant(resource.chara));
		SetTjsProperty(dict, TJS_W("motion"), tTJSVariant(resource.motion));
		SetTjsProperty(dict, TJS_W("motionCount"), tTJSVariant((tjs_int)resource.motions.size()));
		SetTjsProperty(dict, TJS_W("objectCount"), tTJSVariant((tjs_int)resource.objects.size()));
		SetTjsProperty(dict, TJS_W("variableCount"), tTJSVariant((tjs_int)resource.variables.size()));

		if (resource.hasMetadata && (resource.chara.GetLen() > 0 || resource.motion.GetLen() > 0)) {
			iTJSDispatch2 *metadata = TJSCreateDictionaryObject();
			iTJSDispatch2 *base = TJSCreateDictionaryObject();
			SetTjsProperty(base, TJS_W("chara"), tTJSVariant(resource.chara));
			SetTjsProperty(base, TJS_W("motion"), tTJSVariant(resource.motion));
			tTJSVariant baseValue(base, base);
			base->Release();
			SetTjsProperty(metadata, TJS_W("base"), baseValue);
			tTJSVariant metadataValue(metadata, metadata);
			metadata->Release();
			SetTjsProperty(dict, TJS_W("metadata"), metadataValue);
		}

		tTJSVariant result(dict, dict);
		dict->Release();
		return result;
	}

	iTJSDispatch2 *window = nullptr;
	tjs_int cacheSize = 0;
	ttstr loadedPath;
	tjs_uint64 loadedByteSize = 0;
	bool loaded = false;
	tjs_int nextResourceId = 0;
	std::map<std::string, std::shared_ptr<MotionResource>> cache;
	std::vector<std::shared_ptr<MotionResource>> resources;
	std::shared_ptr<MotionResource> currentResource;
	inline static tjs_int emotePSBDecryptSeed = 0;
	inline static tTJSVariant emotePSBDecryptFunc;
};

class SeparateLayerAdaptor {
public:
		SeparateLayerAdaptor() = default;
		SeparateLayerAdaptor(iTJSDispatch2 *targetLayer) { setSourceLayer(targetLayer); }
		~SeparateLayerAdaptor()
		{
			setLayer(nullptr);
		}

		void assign(iTJSDispatch2 *anotherAdaptor)
		{
			SeparateLayerAdaptor *nativeAdaptor =
				ncbInstanceAdaptor<SeparateLayerAdaptor>::GetNativeInstance(anotherAdaptor, false);
			iTJSDispatch2 *source = nativeAdaptor ? nativeAdaptor->getLayer() : anotherAdaptor;
			tTJSNI_BaseLayer *dstLayer = MotionGetNativeLayer(getLayer());
			tTJSNI_BaseLayer *srcLayer = MotionGetNativeLayer(source);
			if (dstLayer && srcLayer) {
				ensureDrawableLayerSize(dstLayer, srcLayer);
				dstLayer->AssignImages(srcLayer);
				dstLayer->Update();
				return;
			}
			setSourceLayer(source);
		}
		void clear()
		{
			tTJSNI_BaseLayer *drawLayer = MotionGetNativeLayer(layer);
			if (drawLayer) drawLayer->Update();
		}
		iTJSDispatch2 *getLayer() const { return layer; }

	tjs_int get_absolute() const { return absolute; }
	void set_absolute(tjs_int value)
	{
		absolute = value;
		tTJSNI_BaseLayer *drawLayer = MotionGetNativeLayer(layer);
		if (drawLayer) {
			try {
				drawLayer->SetAbsoluteOrderIndex(value);
			} catch (...) {
			}
		}
	}

	private:
		static void ensureDrawableLayerSize(tTJSNI_BaseLayer *dst, tTJSNI_BaseLayer *src)
		{
			if (!dst || !src) return;
			if (dst->GetType() == ltBinder || dst->GetType() == ltEffect ||
				dst->GetType() == ltFilter) {
				dst->SetType(ltAlpha);
			}
			tjs_int width = 0;
			tjs_int height = 0;
			if (src->GetHasImage()) {
				width = (tjs_int)src->GetImageWidth();
				height = (tjs_int)src->GetImageHeight();
			}
			if (width <= 0) width = (tjs_int)src->GetWidth();
			if (height <= 0) height = (tjs_int)src->GetHeight();
			if (width <= 0) width = 1;
			if (height <= 0) height = 1;
			if ((tjs_int)dst->GetWidth() != width || (tjs_int)dst->GetHeight() != height) {
				dst->SetSize((tjs_uint)width, (tjs_uint)height);
			}
			if (!dst->GetHasImage()) dst->SetHasImage(true);
			if ((tjs_int)dst->GetImageWidth() != width ||
				(tjs_int)dst->GetImageHeight() != height) {
				dst->SetImageSize((tjs_uint)width, (tjs_uint)height);
			}
		}

		void setSourceLayer(iTJSDispatch2 *targetLayer)
		{
			tTJSNI_BaseLayer *targetNative = MotionGetNativeLayer(targetLayer);
			if (targetNative) {
				if (!targetNative->GetHasImage()) targetNative->SetHasImage(true);
				ensureDrawableLayerSize(targetNative, targetNative);
				if (targetNative->GetType() == ltBinder || targetNative->GetType() == ltEffect ||
					targetNative->GetType() == ltFilter) {
					targetNative->SetType(ltAlpha);
				}
			}
			setLayer(targetLayer);
		}

		void setLayer(iTJSDispatch2 *targetLayer)
		{
			if (targetLayer) targetLayer->AddRef();
			if (layer) layer->Release();
			layer = targetLayer;
		}

		iTJSDispatch2 *layer = nullptr;
		tjs_int absolute = 0;
	};
