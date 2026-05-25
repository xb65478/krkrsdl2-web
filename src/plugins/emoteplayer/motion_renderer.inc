struct MotionRendererPlanEntryInfo {
	const MotionLayoutInfo *layout = nullptr;
	const MotionRenderMethodItem *renderMethodItem = nullptr;
	tjs_int renderMethodItemIndex = -1;
	tjs_int sourceIndex = 0;
	tjs_int drawItemIndex = 0;
	tjs_int drawOrder = 0;
	tjs_int vertexOffset = 0;
	tjs_int vertexCount = 0;
	tjs_int indexOffset = 0;
	tjs_int indexCount = 0;
	tjs_int firstCellIndex = 0;
	tjs_int cellCount = 0;
	tjs_int quadCount = 0;
	bool commandFromRenderMethodIR = false;
	ttstr vertexSlice;
	ttstr indexSlice;
	ttstr firstCellLocalIndices;
	ttstr sampleHash;
};

struct MotionRendererCommandEntryInfo {
	tjs_int drawItemIndex = 0;
	tjs_int drawOrder = 0;
	tjs_int sourceIndex = 0;
	tjs_int renderMethodItemIndex = -1;
	tjs_int vertexOffset = 0;
	tjs_int vertexCount = 0;
	tjs_int indexOffset = 0;
	tjs_int indexCount = 0;
	tjs_int firstCellIndex = 0;
	tjs_int cellCount = 0;
	tjs_int quadCount = 0;
	bool commandFromRenderMethodIR = false;
	ttstr vertexSlice;
	ttstr indexSlice;
	ttstr firstCellLocalIndices;
	ttstr sampleHash;
	tjs_int consumedAtIndex = 0;
};

struct MotionRendererCommandConsumerSummary {
	bool built = false;
	bool ran = false;
	ttstr source;
	tjs_int commandCount = 0;
	tjs_int vertexTotal = 0;
	tjs_int indexTotal = 0;
	tjs_int cellTotal = 0;
	bool offsetsContiguous = true;
	bool perEntryOk = true;
	bool orderOk = true;
	bool sliceOk = true;
	ttstr firstCommandHash;
	ttstr lastCommandHash;
	tjs_uint64 commandHash = 0;
	bool matchesProducer = false;
	ttstr mismatchKind;
	tjs_int mismatchIndex = -1;
	ttstr mismatchDetails;
	bool consumedIR = false;
	ttstr rendererCommandSource;
};

struct MotionRendererUploadPrepSummary {
	bool ran = false;
	bool prepared = false;
	ttstr source;
	tjs_int commandCount = 0;
	tjs_int vertexBufferCount = 0;
	tjs_int indexBufferCount = 0;
	tjs_int cellBufferCount = 0;
	tjs_int vertexWriteCount = 0;
	tjs_int indexWriteCount = 0;
	tjs_int alphaWriteCount = 0;
	tjs_int uvWriteCount = 0;
	bool offsetsOk = true;
	bool rangesOk = true;
	bool perCommandOk = true;
	tjs_uint64 vertexHash = 0;
	tjs_uint64 indexHash = 0;
	tjs_uint64 alphaHash = 0;
	tjs_uint64 uvHash = 0;
	ttstr firstVertexSample;
	ttstr lastVertexSample;
	ttstr firstIndexSample;
	ttstr lastIndexSample;
	ttstr mismatchKind;
	tjs_int mismatchIndex = -1;
	ttstr mismatchDetails;
	bool stagingArtifactPopulated = false;
	tjs_uint64 stagingVertexArrayHash = 0;
	tjs_uint64 stagingIndexArrayHash = 0;
	tjs_uint64 stagingAlphaArrayHash = 0;
	tjs_uint64 stagingUvArrayHash = 0;
	ttstr stagingFirstPositionSample;
	ttstr stagingLastPositionSample;
	ttstr stagingFirstUvSample;
	ttstr stagingLastUvSample;
	ttstr stagingFirstAlphaSample;
	ttstr stagingLastAlphaSample;
	ttstr stagingIndexRangeMin;
	ttstr stagingIndexRangeMax;
	tjs_int m4SurfaceChainCommandCount = 0;
	tjs_int m4MaxSurfaceChainLength = 0;
	tjs_int m4RemappedVertexCount = 0;
	tjs_int m4ViewportTransformedVertexCount = 0;
	tjs_real m4SampleNdcX = 0.0;
	tjs_real m4SampleNdcY = 0.0;
	tjs_real m4SamplePixelX = 0.0;
	tjs_real m4SamplePixelY = 0.0;
	tjs_real m4SampleMaxAbsError = 0.0;
};

struct MotionStagingArtifact {
	std::vector<float> position;
	std::vector<float> uv;
	std::vector<tjs_int> alpha;
	std::vector<tjs_int> index;
	std::vector<float> coordZ;
};

struct MotionShadowDrawDiagnostic {
	bool ran = false;
	bool consumed = false;
	ttstr source;
	tjs_int consumedCommandCount = 0;
	tjs_int consumedVertexCount = 0;
	tjs_int consumedIndexCount = 0;
	tjs_int consumedCellCount = 0;
	ttstr indexRangeMin;
	ttstr indexRangeMax;
	bool indexInRange = true;
	tjs_real coverageMinX = 0.0;
	tjs_real coverageMinY = 0.0;
	tjs_real coverageMaxX = 0.0;
	tjs_real coverageMaxY = 0.0;
	tjs_int coveredTriangleCount = 0;
	tjs_int coveredCellCount = 0;
	tjs_int nonZeroAlphaVertexCount = 0;
	tjs_uint64 consumedPosHash = 0;
	tjs_uint64 consumedUvHash = 0;
	tjs_uint64 consumedAlphaHash = 0;
	tjs_uint64 consumedIndexHash = 0;
	tjs_uint64 consumedCompositeHash = 0;
	ttstr firstConsumedTriangle;
	ttstr lastConsumedTriangle;
	ttstr firstConsumedPixelSample;
	ttstr mismatchKind;
	tjs_int mismatchIndex = -1;
	ttstr mismatchDetails;
};

struct MotionOffscreenDrawDiagnostic {
	bool ran = false;
	bool consumed = false;
	ttstr source;
	tjs_int width = 0;
	tjs_int height = 0;
	tjs_int tileGridW = 0;
	tjs_int tileGridH = 0;
	tjs_int consumedCommandCount = 0;
	tjs_int consumedTriangleCount = 0;
	tjs_int consumedCellCount = 0;
	tjs_int coveredTileCount = 0;
	tjs_int alphaNonZeroTileCount = 0;
	tjs_real alphaAccum = 0.0;
	tjs_real boundsMinX = 0.0;
	tjs_real boundsMinY = 0.0;
	tjs_real boundsMaxX = 0.0;
	tjs_real boundsMaxY = 0.0;
	tjs_uint64 coverageHash = 0;
	tjs_uint64 alphaHash = 0;
	tjs_uint64 compositeHash = 0;
	ttstr firstOccupiedTileSample;
	ttstr lastOccupiedTileSample;
	ttstr firstAlphaTileSample;
	ttstr mismatchKind;
	tjs_int mismatchIndex = -1;
	ttstr mismatchDetails;
};

struct MotionVisibleRenderBridge {
	bool built = false;
	bool rendered = false;
	ttstr source;
	tjs_int targetWidth = 0;
	tjs_int targetHeight = 0;
	bool fallbackStillUsed = true;
	bool rendererPathUsed = false;
	tjs_uint64 pixelHash = 0;
	tjs_int pixelCount = 0;
	tjs_int alphaNonZeroPixelCount = 0;
	tjs_real alphaAccum = 0.0;
	tjs_int staticSnapshotCanvasWidth = 0;
	tjs_int staticSnapshotCanvasHeight = 0;
	tjs_int staticSnapshotOffsetX = 0;
	tjs_int staticSnapshotOffsetY = 0;
	bool staticSnapshotUsedMotionLayout = false;
	tjs_int staticSnapshotItemCount = 0;
	tjs_real staticReplayScaleX = 0.0;
	tjs_real staticReplayScaleY = 0.0;
	bool staticReplayTargetMatchesCanvas = false;
	bool staticReplayBoundsValid = false;
	tjs_real staticReplayBoundsMinX = 0.0;
	tjs_real staticReplayBoundsMinY = 0.0;
	tjs_real staticReplayBoundsMaxX = 0.0;
	tjs_real staticReplayBoundsMaxY = 0.0;
	bool staticReplayCoverageValid = false;
	tjs_real staticReplayCoverageMinX = 0.0;
	tjs_real staticReplayCoverageMinY = 0.0;
	tjs_real staticReplayCoverageMaxX = 0.0;
	tjs_real staticReplayCoverageMaxY = 0.0;
	bool staticReplayInBounds = false;
	bool staticReplayCoverageMatchesBounds = false;
	bool staticReplayCoverageInsideTarget = false;
	bool staticReplayViewportClippingUsed = false;
	bool staticReplayViewportClipOk = false;
	tjs_int staticReplaySkippedItemCount = 0;
	tjs_int staticReplayClippedItemCount = 0;
	tjs_int staticReplayFullyOutsideItemCount = 0;
	ttstr staticFirstItemSample;
	ttstr staticLastItemSample;
	tjs_int textureUniqueRefCount = 0;
	tjs_int textureTotalPixelCount = 0;
	tjs_int textureAlphaNonZeroPixelCount = 0;
	tjs_int textureRedPixelCount = 0;
	tjs_int textureWhitePixelCount = 0;
	tjs_int textureBlackPixelCount = 0;
	tjs_uint64 textureColorHash = 0;
	ttstr textureColorSummary;
	tjs_int sampledSourcePixelCount = 0;
	tjs_int sampledSourceAlphaNonZeroPixelCount = 0;
	tjs_int sampledSourceRedPixelCount = 0;
	tjs_int sampledSourceWhitePixelCount = 0;
	tjs_int sampledSourceBlackPixelCount = 0;
	tjs_uint64 sampledSourceColorHash = 0;
	tjs_real sampledUvMinU = 0.0;
	tjs_real sampledUvMaxU = 0.0;
	tjs_real sampledUvMinV = 0.0;
	tjs_real sampledUvMaxV = 0.0;
	bool sampledUvBoundsValid = false;
	ttstr sampledSourceColorSummary;
	tjs_int finalLayerPixelCount = 0;
	tjs_int finalLayerAlphaNonZeroPixelCount = 0;
	tjs_int finalLayerRedPixelCount = 0;
	tjs_int finalLayerWhitePixelCount = 0;
	tjs_int finalLayerBlackPixelCount = 0;
	tjs_int finalLayerColorBucketCount = 0;
	tjs_uint64 finalLayerColorHash = 0;
	ttstr finalLayerColorSummary;
	ttstr maxNonBackgroundCommandCoverageSummary;
	ttstr maxNonBackgroundCommandIRSummary;
	ttstr maxBlackCommandCoverageSummary;
	ttstr maxBlackCommandIRSummary;
	ttstr mismatchKind;
	tjs_int mismatchIndex = -1;
	ttstr mismatchDetails;
	ttstr visibleBridgePath;
	tjs_int triangleCount = 0;
	tjs_int commandCount = 0;
	ttstr maxCommandCoverageSummary;
	ttstr maxCommandIRSummary;
};

struct MotionDiagnosticRenderBridge {
	bool built = false;
	bool bridgeWouldDraw  = false  ;
	bool fallbackStillUsed = true;
	ttstr source;
	tjs_int targetWidth = 0;
	tjs_int targetHeight = 0;
	bool compatibleWithLayerTarget = false;
	ttstr compatibleWithLayerTargetDetail;
	tjs_int commandCount = 0;
	tjs_int cellCount = 0;
	tjs_int triangleCount = 0;
	tjs_int coveredTileCount = 0;
	tjs_int alphaNonZeroTileCount = 0;
	tjs_real alphaAccum = 0.0;
	tjs_uint64 coverageHash = 0;
	tjs_uint64 alphaHash = 0;
	tjs_uint64 compositeHash = 0;
	tjs_real boundsMinX = 0.0;
	tjs_real boundsMinY = 0.0;
	tjs_real boundsMaxX = 0.0;
	tjs_real boundsMaxY = 0.0;
	ttstr blendModeSummary;
	ttstr alphaModeSummary;
	ttstr pixelCoverageSummary;
	ttstr firstTileSample;
	ttstr lastTileSample;
	ttstr firstAlphaTileSample;
	tjs_int tileGridW = 0;
	tjs_int tileGridH = 0;
	ttstr mismatchKind;
	tjs_int mismatchIndex = -1;
	ttstr mismatchDetails;
	ttstr drawTargetCompatNote;
};

struct MotionRendererCommandPlanMatchSummary {
	bool compared = false;
	tjs_int commandCount = 0;
	tjs_int planCount = 0;
	tjs_int comparedCount = 0;
	tjs_int entryMatchCount = 0;
	tjs_uint64 commandSharedHash = 0;
	tjs_uint64 planHash = 0;
	bool matches = false;
	ttstr mismatchKind;
	tjs_int mismatchIndex = -1;
	ttstr mismatchField;
	ttstr mismatchDetails;
};

static void MotionHashPlanInt(tjs_uint64 &hash, tjs_int value)
{
	hash ^= (tjs_uint64)(tjs_uint32)value;
	hash *= (tjs_uint64)1099511628211ULL;
}

static void MotionHashPlanDouble(tjs_uint64 &hash, tjs_real value)
{
	tjs_int32 asInt = (tjs_int32)std::lround(value * 1000.0);
	MotionHashPlanInt(hash, asInt);
}

static ttstr MotionPlanHashText(tjs_uint64 hash)
{
	return ttstr((tjs_int)((hash >> 32) & 0x7fffffff)) + TJS_W(":") +
		ttstr((tjs_int)(hash & 0x7fffffff));
}

static ttstr MotionBuildRendererPlanSampleHash(const MotionRendererPlanEntryInfo &entry)
{
	return ttstr(TJS_W("o")) + ttstr(entry.drawItemIndex) +
		TJS_W(":do") + ttstr(entry.drawOrder) +
		TJS_W(":s") + ttstr(entry.sourceIndex) +
		TJS_W(":ri") + ttstr(entry.renderMethodItemIndex) +
		TJS_W(":vo") + ttstr(entry.vertexOffset) +
		TJS_W(":io") + ttstr(entry.indexOffset) +
		TJS_W(":v") + ttstr(entry.vertexCount) +
		TJS_W(":i") + ttstr(entry.indexCount) +
		TJS_W(":fc") + ttstr(entry.firstCellIndex) +
		TJS_W(":c") + ttstr(entry.cellCount);
}

static std::vector<MotionRendererPlanEntryInfo>
MotionBuildRendererPlanEntries(const MotionResource *resource, tjs_real meshDivisionRatio)
{
	std::vector<MotionRendererPlanEntryInfo> entries;
	std::vector<std::pair<const MotionLayoutInfo*, tjs_int> > items =
		MotionCollectFrontLayouts(resource, false);
	tjs_int division = (tjs_int)std::ceil(std::max<tjs_real>(1.0, meshDivisionRatio));
	division = std::max<tjs_int>(1, division);
	tjs_int vertexTotal = 0;
	tjs_int indexTotal = 0;
	tjs_int quadTotal = 0;
	for (size_t i = 0; i < items.size(); i++) {
		tjs_int itemQuadCount = division * division;
		tjs_int itemVertexCount = (division + 1) * (division + 1);
		tjs_int itemIndexCount = itemQuadCount * 6;
		MotionRendererPlanEntryInfo entry;
		entry.layout = items[i].first;
		entry.sourceIndex = items[i].second;
		entry.drawItemIndex = (tjs_int)i;
		entry.drawOrder = (tjs_int)i;
		entry.vertexOffset = vertexTotal;
		entry.vertexCount = itemVertexCount;
		entry.indexOffset = indexTotal;
		entry.indexCount = itemIndexCount;
		entry.firstCellIndex = quadTotal;
		entry.cellCount = itemQuadCount;
		entry.quadCount = itemQuadCount;
		entry.renderMethodItemIndex =
			entry.layout ? entry.layout->renderMethodItemIndex : -1;
		entry.commandFromRenderMethodIR = entry.renderMethodItemIndex >= 0;
		if (resource && entry.commandFromRenderMethodIR &&
			entry.renderMethodItemIndex < (tjs_int)resource->renderMethodIR.size()) {
			entry.renderMethodItem =
				&resource->renderMethodIR[(size_t)entry.renderMethodItemIndex];
		}
		entry.vertexSlice = ttstr((tjs_int)vertexTotal) + TJS_W("-") +
			ttstr((tjs_int)(vertexTotal + itemVertexCount - 1));
		entry.indexSlice = ttstr((tjs_int)indexTotal) + TJS_W("-") +
			ttstr((tjs_int)(indexTotal + itemIndexCount - 1));
		tjs_int v00 = 0;
		tjs_int v01 = 1;
		tjs_int v10 = division + 1;
		tjs_int v11 = division + 2;
		entry.firstCellLocalIndices = ttstr(v00) + TJS_W(",") + ttstr(v01) +
			TJS_W(",") + ttstr(v10) + TJS_W(",") + ttstr(v10) +
			TJS_W(",") + ttstr(v01) + TJS_W(",") + ttstr(v11);
		entry.sampleHash = MotionBuildRendererPlanSampleHash(entry);
		entries.push_back(entry);
		vertexTotal += itemVertexCount;
		indexTotal += itemIndexCount;
		quadTotal += itemQuadCount;
	}
	return entries;
}

static tjs_uint64 MotionHashRendererPlanEntries(
	const std::vector<MotionRendererPlanEntryInfo> &entries)
{
	tjs_uint64 hash = (tjs_uint64)1469598103934665603ULL;
	for (size_t i = 0; i < entries.size(); i++) {
		const MotionRendererPlanEntryInfo &entry = entries[i];
		MotionHashPlanInt(hash, entry.drawItemIndex);
		MotionHashPlanInt(hash, entry.drawOrder);
		MotionHashPlanInt(hash, entry.sourceIndex);
		MotionHashPlanInt(hash, entry.renderMethodItemIndex);
		MotionHashPlanInt(hash, entry.vertexOffset);
		MotionHashPlanInt(hash, entry.vertexCount);
		MotionHashPlanInt(hash, entry.indexOffset);
		MotionHashPlanInt(hash, entry.indexCount);
		MotionHashPlanInt(hash, entry.firstCellIndex);
		MotionHashPlanInt(hash, entry.cellCount);
		MotionHashPlanInt(hash, entry.quadCount);
	}
	return hash;
}

static tjs_int MotionRendererPlanVertexTotal(
	const std::vector<MotionRendererPlanEntryInfo> &entries)
{
	tjs_int total = 0;
	for (size_t i = 0; i < entries.size(); i++) total += entries[i].vertexCount;
	return total;
}

static tjs_int MotionRendererPlanIndexTotal(
	const std::vector<MotionRendererPlanEntryInfo> &entries)
{
	tjs_int total = 0;
	for (size_t i = 0; i < entries.size(); i++) total += entries[i].indexCount;
	return total;
}

static tjs_int MotionRendererPlanQuadTotal(
	const std::vector<MotionRendererPlanEntryInfo> &entries)
{
	tjs_int total = 0;
	for (size_t i = 0; i < entries.size(); i++) total += entries[i].quadCount;
	return total;
}

static ttstr MotionBuildRendererCommandSampleHash(const MotionRendererCommandEntryInfo &entry)
{
	return ttstr(TJS_W("o")) + ttstr(entry.drawItemIndex) +
		TJS_W(":do") + ttstr(entry.drawOrder) +
		TJS_W(":s") + ttstr(entry.sourceIndex) +
		TJS_W(":ri") + ttstr(entry.renderMethodItemIndex) +
		TJS_W(":vo") + ttstr(entry.vertexOffset) +
		TJS_W(":io") + ttstr(entry.indexOffset) +
		TJS_W(":v") + ttstr(entry.vertexCount) +
		TJS_W(":i") + ttstr(entry.indexCount) +
		TJS_W(":fc") + ttstr(entry.firstCellIndex) +
		TJS_W(":c") + ttstr(entry.cellCount) +
		TJS_W(":q") + ttstr(entry.quadCount) +
		TJS_W(":ca") + ttstr(entry.consumedAtIndex);
}

static std::vector<MotionRendererCommandEntryInfo>
MotionBuildRendererCommandEntriesFromDrawItems(const MotionResource *resource,
	tjs_real meshDivisionRatio)
{
	std::vector<MotionRendererCommandEntryInfo> entries;
	if (resource && !resource->renderMethodIR.empty()) {
		tjs_int division = (tjs_int)std::ceil(std::max<tjs_real>(1.0, meshDivisionRatio));
		division = std::max<tjs_int>(1, division);
		tjs_int vertexTotal = 0;
		tjs_int indexTotal = 0;
		tjs_int quadTotal = 0;
		std::vector<size_t> orderedIR;
		orderedIR.reserve(resource->renderMethodIR.size());
		for (size_t ri = 0; ri < resource->renderMethodIR.size(); ri++) {
			orderedIR.push_back(ri);
		}
		std::stable_sort(orderedIR.begin(), orderedIR.end(),
			[resource](size_t a, size_t b) {
				const MotionRenderMethodItem &left = resource->renderMethodIR[a];
				const MotionRenderMethodItem &right = resource->renderMethodIR[b];
				if (!left.drawPriorityPath.empty() || !right.drawPriorityPath.empty()) {
					return std::lexicographical_compare(
						left.drawPriorityPath.begin(), left.drawPriorityPath.end(),
						right.drawPriorityPath.begin(), right.drawPriorityPath.end());
				}
				if (left.drawPriority != right.drawPriority) {
					return left.drawPriority < right.drawPriority;
				}
				return a < b;
			});
		for (size_t oi = 0; oi < orderedIR.size(); oi++) {
			size_t ri = orderedIR[oi];
			const MotionRenderMethodItem &item = resource->renderMethodIR[ri];
			if (!MotionRenderMethodItemRenderableForRuntimeDraw(item)) continue;
			tjs_int commandIndex = (tjs_int)entries.size();
			tjs_int itemQuadCount = division * division;
			tjs_int itemVertexCount = (division + 1) * (division + 1);
			tjs_int itemIndexCount = itemQuadCount * 6;
			MotionRendererCommandEntryInfo entry;
			entry.drawItemIndex = commandIndex;
			entry.drawOrder = commandIndex;
			entry.sourceIndex = item.layoutIndex >= 0 ? item.layoutIndex : (tjs_int)ri;
			entry.renderMethodItemIndex = (tjs_int)ri;
			entry.commandFromRenderMethodIR = true;
			entry.vertexOffset = vertexTotal;
			entry.vertexCount = itemVertexCount;
			entry.indexOffset = indexTotal;
			entry.indexCount = itemIndexCount;
			entry.firstCellIndex = quadTotal;
			entry.cellCount = itemQuadCount;
			entry.quadCount = itemQuadCount;
			entry.vertexSlice = ttstr((tjs_int)vertexTotal) + TJS_W("-") +
				ttstr((tjs_int)(vertexTotal + itemVertexCount - 1));
			entry.indexSlice = ttstr((tjs_int)indexTotal) + TJS_W("-") +
				ttstr((tjs_int)(indexTotal + itemIndexCount - 1));
			tjs_int v00 = 0;
			tjs_int v01 = 1;
			tjs_int v10 = division + 1;
			tjs_int v11 = division + 2;
			entry.firstCellLocalIndices = ttstr(v00) + TJS_W(",") + ttstr(v01) +
				TJS_W(",") + ttstr(v10) + TJS_W(",") + ttstr(v10) +
				TJS_W(",") + ttstr(v01) + TJS_W(",") + ttstr(v11);
			entry.consumedAtIndex = commandIndex;
			entry.sampleHash = MotionBuildRendererCommandSampleHash(entry);
			entries.push_back(entry);
			vertexTotal += entry.vertexCount;
			indexTotal += entry.indexCount;
			quadTotal += entry.cellCount;
		}
		if (!entries.empty()) return entries;
	}
	std::vector<std::pair<const MotionLayoutInfo*, tjs_int> > items =
		MotionCollectRuntimeDrawLayouts(resource);
	tjs_int division = (tjs_int)std::ceil(std::max<tjs_real>(1.0, meshDivisionRatio));
	division = std::max<tjs_int>(1, division);
	tjs_int vertexTotal = 0;
	tjs_int indexTotal = 0;
	tjs_int quadTotal = 0;
	for (size_t i = 0; i < items.size(); i++) {
		const MotionLayoutInfo *layout = items[i].first;
		if (!layout) continue;
		if (layout->sourceKey.GetLen() <= 0) continue;
		if (layout->opacityMax <= 0.001) continue;
		tjs_int commandIndex = (tjs_int)entries.size();
		tjs_int itemQuadCount = division * division;
		tjs_int itemVertexCount = (division + 1) * (division + 1);
		tjs_int itemIndexCount = itemQuadCount * 6;
		MotionRendererCommandEntryInfo entry;
		entry.drawItemIndex = commandIndex;
		entry.drawOrder = commandIndex;
		entry.sourceIndex = items[i].second;
		entry.renderMethodItemIndex = layout->renderMethodItemIndex;
		entry.commandFromRenderMethodIR = entry.renderMethodItemIndex >= 0;
		entry.vertexOffset = vertexTotal;
		entry.vertexCount = itemVertexCount;
		entry.indexOffset = indexTotal;
		entry.indexCount = itemIndexCount;
		entry.firstCellIndex = quadTotal;
		entry.cellCount = itemQuadCount;
		entry.quadCount = itemQuadCount;
		entry.vertexSlice = ttstr((tjs_int)vertexTotal) + TJS_W("-") +
			ttstr((tjs_int)(vertexTotal + itemVertexCount - 1));
		entry.indexSlice = ttstr((tjs_int)indexTotal) + TJS_W("-") +
			ttstr((tjs_int)(indexTotal + itemIndexCount - 1));
		tjs_int v00 = 0;
		tjs_int v01 = 1;
		tjs_int v10 = division + 1;
		tjs_int v11 = division + 2;
		entry.firstCellLocalIndices = ttstr(v00) + TJS_W(",") + ttstr(v01) +
			TJS_W(",") + ttstr(v10) + TJS_W(",") + ttstr(v10) +
			TJS_W(",") + ttstr(v01) + TJS_W(",") + ttstr(v11);
		entry.consumedAtIndex = commandIndex;
		entry.sampleHash = MotionBuildRendererCommandSampleHash(entry);
		entries.push_back(entry);
		vertexTotal += entry.vertexCount;
		indexTotal += entry.indexCount;
		quadTotal += entry.cellCount;
	}
	return entries;
}

static void MotionRebuildRendererCommandOffsets(
	std::vector<MotionRendererCommandEntryInfo> &entries)
{
	tjs_int vertexTotal = 0;
	tjs_int indexTotal = 0;
	tjs_int quadTotal = 0;
	for (size_t i = 0; i < entries.size(); i++) {
		MotionRendererCommandEntryInfo &entry = entries[i];
		entry.drawItemIndex = (tjs_int)i;
		entry.drawOrder = (tjs_int)i;
		entry.consumedAtIndex = (tjs_int)i;
		entry.vertexOffset = vertexTotal;
		entry.indexOffset = indexTotal;
		entry.firstCellIndex = quadTotal;
		entry.vertexSlice = ttstr((tjs_int)vertexTotal) + TJS_W("-") +
			ttstr((tjs_int)(vertexTotal + entry.vertexCount - 1));
		entry.indexSlice = ttstr((tjs_int)indexTotal) + TJS_W("-") +
			ttstr((tjs_int)(indexTotal + entry.indexCount - 1));
		entry.sampleHash = MotionBuildRendererCommandSampleHash(entry);
		vertexTotal += entry.vertexCount;
		indexTotal += entry.indexCount;
		quadTotal += entry.cellCount;
	}
}

static void MotionFilterRendererCommandsBySnapshotTextures(
	std::vector<MotionRendererCommandEntryInfo> &entries,
	const MotionResource *resource,
	const TVPStaticMotionBridgeSnapshot *snapshot)
{
	if (!resource || !snapshot || snapshot->items.empty() || entries.empty()) return;
	std::vector<MotionRendererCommandEntryInfo> filtered;
	filtered.reserve(entries.size());
	for (size_t i = 0; i < entries.size(); i++) {
		const MotionRendererCommandEntryInfo &entry = entries[i];
		const MotionRenderMethodItem *item = nullptr;
		if (entry.renderMethodItemIndex >= 0 &&
			entry.renderMethodItemIndex < (tjs_int)resource->renderMethodIR.size()) {
			item = &resource->renderMethodIR[(size_t)entry.renderMethodItemIndex];
		}
		if (item &&
			!MotionFindSnapshotItemForSourceKey(snapshot, item->srcKey)) {
			continue;
		}
		filtered.push_back(entry);
	}
	if (filtered.size() == entries.size()) return;
	entries.swap(filtered);
	MotionRebuildRendererCommandOffsets(entries);
}

static tjs_uint64 MotionHashRendererCommandEntries(
	const std::vector<MotionRendererCommandEntryInfo> &entries)
{
	tjs_uint64 hash = (tjs_uint64)1469598103934665607ULL;
	for (size_t i = 0; i < entries.size(); i++) {
		const MotionRendererCommandEntryInfo &entry = entries[i];
		MotionHashPlanInt(hash, entry.drawItemIndex);
		MotionHashPlanInt(hash, entry.drawOrder);
		MotionHashPlanInt(hash, entry.sourceIndex);
		MotionHashPlanInt(hash, entry.renderMethodItemIndex);
		MotionHashPlanInt(hash, entry.vertexOffset);
		MotionHashPlanInt(hash, entry.vertexCount);
		MotionHashPlanInt(hash, entry.indexOffset);
		MotionHashPlanInt(hash, entry.indexCount);
		MotionHashPlanInt(hash, entry.firstCellIndex);
		MotionHashPlanInt(hash, entry.cellCount);
		MotionHashPlanInt(hash, entry.quadCount);
		MotionHashPlanInt(hash, entry.consumedAtIndex);
	}
	return hash;
}

static tjs_uint64 MotionHashRendererCommandSharedFields(
	const std::vector<MotionRendererCommandEntryInfo> &entries)
{
	tjs_uint64 hash = (tjs_uint64)1469598103934665603ULL;
	for (size_t i = 0; i < entries.size(); i++) {
		const MotionRendererCommandEntryInfo &entry = entries[i];
		MotionHashPlanInt(hash, entry.drawItemIndex);
		MotionHashPlanInt(hash, entry.drawOrder);
		MotionHashPlanInt(hash, entry.sourceIndex);
		MotionHashPlanInt(hash, entry.renderMethodItemIndex);
		MotionHashPlanInt(hash, entry.vertexOffset);
		MotionHashPlanInt(hash, entry.vertexCount);
		MotionHashPlanInt(hash, entry.indexOffset);
		MotionHashPlanInt(hash, entry.indexCount);
		MotionHashPlanInt(hash, entry.firstCellIndex);
		MotionHashPlanInt(hash, entry.cellCount);
		MotionHashPlanInt(hash, entry.quadCount);
	}
	return hash;
}

static tjs_int MotionRendererCommandVertexTotal(
	const std::vector<MotionRendererCommandEntryInfo> &entries)
{
	tjs_int total = 0;
	for (size_t i = 0; i < entries.size(); i++) total += entries[i].vertexCount;
	return total;
}

static tjs_int MotionRendererCommandIndexTotal(
	const std::vector<MotionRendererCommandEntryInfo> &entries)
{
	tjs_int total = 0;
	for (size_t i = 0; i < entries.size(); i++) total += entries[i].indexCount;
	return total;
}

static tjs_int MotionRendererCommandQuadTotal(
	const std::vector<MotionRendererCommandEntryInfo> &entries)
{
	tjs_int total = 0;
	for (size_t i = 0; i < entries.size(); i++) total += entries[i].cellCount;
	return total;
}

static bool MotionCompareRendererCommandEntryToPlan(
	const MotionRendererCommandEntryInfo &cmdE,
	const MotionRendererPlanEntryInfo &planE, ttstr &mismatchField,
	ttstr &mismatchDetails)
{
	if (cmdE.drawItemIndex != planE.drawItemIndex) {
		mismatchField = TJS_W("drawItemIndex");
		mismatchDetails = ttstr(TJS_W("cmd:")) + ttstr(cmdE.drawItemIndex) +
			TJS_W(" plan:") + ttstr(planE.drawItemIndex);
		return false;
	}
	if (cmdE.drawOrder != planE.drawOrder) {
		mismatchField = TJS_W("drawOrder");
		mismatchDetails = ttstr(TJS_W("cmd:")) + ttstr(cmdE.drawOrder) +
			TJS_W(" plan:") + ttstr(planE.drawOrder);
		return false;
	}
	if (cmdE.sourceIndex != planE.sourceIndex) {
		mismatchField = TJS_W("sourceIndex");
		mismatchDetails = ttstr(TJS_W("cmd:")) + ttstr(cmdE.sourceIndex) +
			TJS_W(" plan:") + ttstr(planE.sourceIndex);
		return false;
	}
	if (cmdE.renderMethodItemIndex != planE.renderMethodItemIndex) {
		mismatchField = TJS_W("renderMethodItemIndex");
		mismatchDetails = ttstr(TJS_W("cmd:")) + ttstr(cmdE.renderMethodItemIndex) +
			TJS_W(" plan:") + ttstr(planE.renderMethodItemIndex);
		return false;
	}
	if (cmdE.vertexOffset != planE.vertexOffset) {
		mismatchField = TJS_W("vertexOffset");
		mismatchDetails = ttstr(TJS_W("cmd:")) + ttstr(cmdE.vertexOffset) +
			TJS_W(" plan:") + ttstr(planE.vertexOffset);
		return false;
	}
	if (cmdE.vertexCount != planE.vertexCount) {
		mismatchField = TJS_W("vertexCount");
		mismatchDetails = ttstr(TJS_W("cmd:")) + ttstr(cmdE.vertexCount) +
			TJS_W(" plan:") + ttstr(planE.vertexCount);
		return false;
	}
	if (cmdE.indexOffset != planE.indexOffset) {
		mismatchField = TJS_W("indexOffset");
		mismatchDetails = ttstr(TJS_W("cmd:")) + ttstr(cmdE.indexOffset) +
			TJS_W(" plan:") + ttstr(planE.indexOffset);
		return false;
	}
	if (cmdE.indexCount != planE.indexCount) {
		mismatchField = TJS_W("indexCount");
		mismatchDetails = ttstr(TJS_W("cmd:")) + ttstr(cmdE.indexCount) +
			TJS_W(" plan:") + ttstr(planE.indexCount);
		return false;
	}
	if (cmdE.firstCellIndex != planE.firstCellIndex) {
		mismatchField = TJS_W("firstCellIndex");
		mismatchDetails = ttstr(TJS_W("cmd:")) + ttstr(cmdE.firstCellIndex) +
			TJS_W(" plan:") + ttstr(planE.firstCellIndex);
		return false;
	}
	if (cmdE.cellCount != planE.cellCount) {
		mismatchField = TJS_W("cellCount");
		mismatchDetails = ttstr(TJS_W("cmd:")) + ttstr(cmdE.cellCount) +
			TJS_W(" plan:") + ttstr(planE.cellCount);
		return false;
	}
	if (cmdE.quadCount != planE.quadCount) {
		mismatchField = TJS_W("quadCount");
		mismatchDetails = ttstr(TJS_W("cmd:")) + ttstr(cmdE.quadCount) +
			TJS_W(" plan:") + ttstr(planE.quadCount);
		return false;
	}
	if (cmdE.vertexSlice != planE.vertexSlice) {
		mismatchField = TJS_W("vertexSlice");
		mismatchDetails = ttstr(TJS_W("cmd:")) + cmdE.vertexSlice +
			TJS_W(" plan:") + planE.vertexSlice;
		return false;
	}
	if (cmdE.indexSlice != planE.indexSlice) {
		mismatchField = TJS_W("indexSlice");
		mismatchDetails = ttstr(TJS_W("cmd:")) + cmdE.indexSlice +
			TJS_W(" plan:") + planE.indexSlice;
		return false;
	}
	if (cmdE.firstCellLocalIndices != planE.firstCellLocalIndices) {
		mismatchField = TJS_W("firstCellLocalIndices");
		mismatchDetails = ttstr(TJS_W("cmd:")) + cmdE.firstCellLocalIndices +
			TJS_W(" plan:") + planE.firstCellLocalIndices;
		return false;
	}
	return true;
}

static MotionRendererCommandPlanMatchSummary MotionCompareRendererCommandsToPlan(
	const std::vector<MotionRendererCommandEntryInfo> &rendererCommandEntries,
	const std::vector<MotionRendererPlanEntryInfo> &planEntries,
	bool rendererCommandBuilt)
{
	MotionRendererCommandPlanMatchSummary summary;
	summary.compared = rendererCommandBuilt;
	summary.commandCount = (tjs_int)rendererCommandEntries.size();
	summary.planCount = (tjs_int)planEntries.size();
	summary.comparedCount =
		std::min(summary.commandCount, summary.planCount);
	summary.commandSharedHash =
		MotionHashRendererCommandSharedFields(rendererCommandEntries);
	summary.planHash = MotionHashRendererPlanEntries(planEntries);

	if (!rendererCommandBuilt) {
		summary.mismatchKind = TJS_W("not-built");
		summary.mismatchField = TJS_W("not-built");
		return summary;
	}

	for (tjs_int i = 0; i < summary.comparedCount; i++) {
		ttstr mismatchField;
		ttstr mismatchDetails;
		if (MotionCompareRendererCommandEntryToPlan(
				rendererCommandEntries[(size_t)i], planEntries[(size_t)i],
				mismatchField, mismatchDetails)) {
			summary.entryMatchCount++;
			continue;
		}
		if (summary.mismatchIndex < 0) {
			summary.mismatchKind = TJS_W("field");
			summary.mismatchIndex = i;
			summary.mismatchField = mismatchField;
			summary.mismatchDetails = mismatchDetails;
		}
	}

	if (summary.commandCount != summary.planCount) {
		if (summary.mismatchIndex < 0) {
			summary.mismatchKind = TJS_W("count");
			summary.mismatchField = TJS_W("count");
			summary.mismatchDetails =
				ttstr(TJS_W("cmd:")) + ttstr(summary.commandCount) +
				TJS_W(" plan:") + ttstr(summary.planCount);
		}
		return summary;
	}

	if (summary.mismatchIndex >= 0) return summary;

	if (summary.commandSharedHash != summary.planHash) {
		summary.mismatchKind = TJS_W("hash");
		summary.mismatchField = TJS_W("hash");
		summary.mismatchDetails =
			ttstr(TJS_W("cmdShared:")) +
			MotionPlanHashText(summary.commandSharedHash) +
			TJS_W(" plan:") + MotionPlanHashText(summary.planHash);
		return summary;
	}

	summary.matches = true;
	summary.mismatchKind = TJS_W("none");
	summary.mismatchField = TJS_W("none");
	return summary;
}

static MotionRendererCommandConsumerSummary MotionConsumeRendererCommands(
	const std::vector<MotionRendererCommandEntryInfo> &rendererCommandEntries)
{
	MotionRendererCommandConsumerSummary summary;
	summary.built = true;
	summary.ran = true;
	summary.source = TJS_W("rendererCommandEntries");

	if (rendererCommandEntries.empty()) {
		summary.perEntryOk = true;
		summary.orderOk = true;
		summary.sliceOk = true;
		summary.offsetsContiguous = true;
		summary.mismatchKind = TJS_W("empty");
		return summary;
	}

	summary.commandCount = (tjs_int)rendererCommandEntries.size();
	summary.offsetsContiguous = true;
	summary.perEntryOk = true;
	summary.orderOk = true;
	summary.sliceOk = true;

	tjs_uint64 consumedHash = (tjs_uint64)1469598103934665607ULL;
	tjs_int expectedVertexOffset = 0;
	tjs_int expectedIndexOffset = 0;
	tjs_int expectedCellOffset = 0;

	for (tjs_int i = 0; i < summary.commandCount; i++) {
		const MotionRendererCommandEntryInfo &entry = rendererCommandEntries[(size_t)i];

		summary.vertexTotal += entry.vertexCount;
		summary.indexTotal += entry.indexCount;
		summary.cellTotal += entry.cellCount;

		if (entry.drawItemIndex != i) {
			summary.orderOk = false;
			if (summary.mismatchIndex < 0) {
				summary.mismatchKind = TJS_W("order");
				summary.mismatchIndex = i;
				summary.mismatchDetails = ttstr(TJS_W("drawItemIndex cmd:")) +
					ttstr(entry.drawItemIndex) + TJS_W(" expected:") + ttstr(i);
			}
		}
		if (entry.vertexCount <= 0 || entry.indexCount <= 0 ||
			entry.cellCount <= 0 || entry.quadCount != entry.cellCount) {
			summary.perEntryOk = false;
			if (summary.mismatchIndex < 0) {
				summary.mismatchKind = TJS_W("entry");
				summary.mismatchIndex = i;
				summary.mismatchDetails = ttstr(TJS_W("counts v:")) +
					ttstr(entry.vertexCount) + TJS_W(" i:") +
					ttstr(entry.indexCount) + TJS_W(" c:") +
					ttstr(entry.cellCount) + TJS_W(" q:") +
					ttstr(entry.quadCount);
			}
		}

		ttstr expectedVertexSlice = ttstr(entry.vertexOffset) + TJS_W("-") +
			ttstr(entry.vertexOffset + entry.vertexCount - 1);
		ttstr expectedIndexSlice = ttstr(entry.indexOffset) + TJS_W("-") +
			ttstr(entry.indexOffset + entry.indexCount - 1);
		if (entry.vertexSlice != expectedVertexSlice ||
			entry.indexSlice != expectedIndexSlice) {
			summary.sliceOk = false;
			if (summary.mismatchIndex < 0) {
				summary.mismatchKind = TJS_W("slice");
				summary.mismatchIndex = i;
				summary.mismatchDetails = ttstr(TJS_W("vertexSlice cmd:") +
					entry.vertexSlice + TJS_W(" expected:") +
					expectedVertexSlice + TJS_W(" indexSlice cmd:") +
					entry.indexSlice + TJS_W(" expected:") + expectedIndexSlice);
			}
		}

		if (entry.vertexOffset != expectedVertexOffset ||
			entry.indexOffset != expectedIndexOffset ||
			entry.firstCellIndex != expectedCellOffset) {
			summary.offsetsContiguous = false;
			if (summary.mismatchIndex < 0) {
				summary.mismatchKind = TJS_W("offset");
				summary.mismatchIndex = i;
				summary.mismatchDetails = ttstr(TJS_W("vertexOffset cmd:")) +
					ttstr(entry.vertexOffset) + TJS_W(" expected:") +
					ttstr(expectedVertexOffset) + TJS_W(" indexOffset cmd:") +
					ttstr(entry.indexOffset) + TJS_W(" expected:") +
					ttstr(expectedIndexOffset) + TJS_W(" firstCellIndex cmd:") +
					ttstr(entry.firstCellIndex) + TJS_W(" expected:") +
					ttstr(expectedCellOffset);
			}
		}
		expectedVertexOffset += entry.vertexCount;
		expectedIndexOffset += entry.indexCount;
		expectedCellOffset += entry.cellCount;

		if (i == 0) {
			summary.firstCommandHash = entry.sampleHash;
		}
		if (i == summary.commandCount - 1) {
			summary.lastCommandHash = entry.sampleHash;
		}

		MotionHashPlanInt(consumedHash, entry.drawItemIndex);
		MotionHashPlanInt(consumedHash, entry.drawOrder);
		MotionHashPlanInt(consumedHash, entry.sourceIndex);
		MotionHashPlanInt(consumedHash, entry.renderMethodItemIndex);
		MotionHashPlanInt(consumedHash, entry.vertexOffset);
		MotionHashPlanInt(consumedHash, entry.vertexCount);
		MotionHashPlanInt(consumedHash, entry.indexOffset);
		MotionHashPlanInt(consumedHash, entry.indexCount);
		MotionHashPlanInt(consumedHash, entry.firstCellIndex);
		MotionHashPlanInt(consumedHash, entry.cellCount);
		MotionHashPlanInt(consumedHash, entry.quadCount);
		MotionHashPlanInt(consumedHash, entry.consumedAtIndex);
	}
	summary.commandHash = consumedHash;

	summary.matchesProducer = true;
	if (summary.mismatchIndex < 0) {
		summary.mismatchKind = TJS_W("none");
		summary.mismatchIndex = -1;
	}

	if (!summary.offsetsContiguous) {
		summary.matchesProducer = false;
		if (summary.mismatchKind == TJS_W("none")) summary.mismatchKind = TJS_W("offset");
	}
	if (!summary.perEntryOk) {
		summary.matchesProducer = false;
		if (summary.mismatchKind == TJS_W("none")) summary.mismatchKind = TJS_W("entry");
	}
	if (!summary.orderOk) {
		summary.matchesProducer = false;
		if (summary.mismatchKind == TJS_W("none")) summary.mismatchKind = TJS_W("order");
	}
	if (!summary.sliceOk) {
		summary.matchesProducer = false;
		if (summary.mismatchKind == TJS_W("none")) summary.mismatchKind = TJS_W("slice");
	}

	return summary;
}

static MotionRendererUploadPrepSummary MotionPrepareRendererUploadFromCommands(
	const std::vector<MotionRendererCommandEntryInfo> &rendererCommandEntries,
	const MotionResource *resource, tjs_real meshDivisionRatio,
	MotionStagingArtifact *stagingArtifact = nullptr)
{
	MotionRendererUploadPrepSummary summary;
	summary.ran = true;
	summary.source = TJS_W("rendererCommandEntries");
	summary.commandCount = (tjs_int)rendererCommandEntries.size();
	if (rendererCommandEntries.empty()) {
		summary.prepared = false;
		summary.offsetsOk = true;
		summary.rangesOk = true;
		summary.perCommandOk = true;
		summary.mismatchKind = TJS_W("empty");
		return summary;
	}

	tjs_int division = (tjs_int)std::ceil(std::max<tjs_real>(1.0, meshDivisionRatio));
	division = std::max<tjs_int>(1, division);

	tjs_uint64 vHash = (tjs_uint64)1469598103934665611ULL;
	tjs_uint64 iHash = (tjs_uint64)1469598103934665613ULL;
	tjs_uint64 aHash = (tjs_uint64)1469598103934665617ULL;
	tjs_uint64 uHash = (tjs_uint64)1469598103934665619ULL;

	tjs_int totalVertexCount = MotionRendererCommandVertexTotal(rendererCommandEntries);
	tjs_int totalIndexCount = MotionRendererCommandIndexTotal(rendererCommandEntries);
	tjs_int totalCellCount = MotionRendererCommandQuadTotal(rendererCommandEntries);
	tjs_real viewportWidth = resource && resource->width > 0 ? (tjs_real)resource->width : 1280.0;
	tjs_real viewportHeight = resource && resource->height > 0 ? (tjs_real)resource->height : 720.0;

	summary.vertexBufferCount = totalVertexCount;
	summary.indexBufferCount = totalIndexCount;
	summary.cellBufferCount = totalCellCount;

	std::vector<float> posBuf((size_t)totalVertexCount * 2, 0.0f);
	std::vector<float> uvBuf((size_t)totalVertexCount * 2, 0.0f);
	std::vector<tjs_int> alphaBuf((size_t)totalVertexCount, 0);
	std::vector<tjs_int> idxBuf((size_t)totalIndexCount, 0);
	std::vector<float> posBufZ((size_t)totalVertexCount, 0.0f);

	bool firstVertexCollected = false;
	bool lastVertexCollected = false;
	bool firstIndexCollected = false;

	for (tjs_int ci = 0; ci < summary.commandCount; ci++) {
		const MotionRendererCommandEntryInfo &cmd = rendererCommandEntries[(size_t)ci];
		if (cmd.vertexCount <= 0 || cmd.indexCount <= 0 || cmd.cellCount <= 0) {
			summary.perCommandOk = false;
			if (summary.mismatchIndex < 0) {
				summary.mismatchKind = TJS_W("command-entry-invalid");
				summary.mismatchIndex = ci;
				summary.mismatchDetails = ttstr(TJS_W("cmd ")) + ttstr(ci) +
					TJS_W(" v:") + ttstr(cmd.vertexCount) +
					TJS_W(" i:") + ttstr(cmd.indexCount) +
					TJS_W(" c:") + ttstr(cmd.cellCount);
			}
			continue;
		}

		tjs_int expectedVertexCount = (division + 1) * (division + 1);
		tjs_int expectedIndexCount = division * division * 6;
		tjs_int expectedCellCount = division * division;
		if (cmd.vertexCount != expectedVertexCount ||
			cmd.indexCount != expectedIndexCount ||
			cmd.cellCount != expectedCellCount) {
			summary.perCommandOk = false;
			if (summary.mismatchIndex < 0) {
				summary.mismatchKind = TJS_W("command-math-mismatch");
				summary.mismatchIndex = ci;
				summary.mismatchDetails = ttstr(TJS_W("cmd ")) + ttstr(ci) +
					TJS_W(" v:") + ttstr(cmd.vertexCount) + TJS_W("/") + ttstr(expectedVertexCount) +
					TJS_W(" i:") + ttstr(cmd.indexCount) + TJS_W("/") + ttstr(expectedIndexCount) +
					TJS_W(" c:") + ttstr(cmd.cellCount) + TJS_W("/") + ttstr(expectedCellCount) +
					TJS_W(" D:") + ttstr(division);
			}
		}

		const MotionLayoutInfo *layout = nullptr;
		const MotionRenderMethodItem *irItem = nullptr;
		if (resource && cmd.renderMethodItemIndex >= 0 &&
			cmd.renderMethodItemIndex < (tjs_int)resource->renderMethodIR.size()) {
			irItem = &resource->renderMethodIR[(size_t)cmd.renderMethodItemIndex];
		}
		if (!irItem && resource && cmd.sourceIndex >= 0 &&
			cmd.sourceIndex < (tjs_int)resource->layouts.size()) {
			layout = &resource->layouts[(size_t)cmd.sourceIndex];
		}
		if (!irItem && !layout) {
			summary.perCommandOk = false;
			if (summary.mismatchIndex < 0) {
				summary.mismatchKind = TJS_W("layout-missing");
				summary.mismatchIndex = ci;
				summary.mismatchDetails = ttstr(TJS_W("cmd ")) + ttstr(ci) +
					TJS_W(" sourceIndex:") + ttstr(cmd.sourceIndex) +
					TJS_W(" layouts:") + ttstr(resource ? (tjs_int)resource->layouts.size() : 0);
			}
			continue;
		}

		tjs_real geomLeft = layout ? layout->left : 0.0;
		tjs_real geomTop = layout ? layout->top : 0.0;
		tjs_real geomWidth = irItem ? irItem->width : layout->width;
		tjs_real geomHeight = irItem ? irItem->height : layout->height;
		tjs_int alphaVal = MotionAlphaFromOpacity(
			irItem ? irItem->opacity : layout->opacityMax);

		bool useAffine = irItem ? irItem->affineValid : MotionLayoutNeedsAffinePath(layout);
		const MotionAffine2D &drawAffine = irItem ? irItem->affine : layout->affine;
		bool useControlPts = irItem ?
			(irItem->hasControlPoints && !MotionControlPtsAreIdentity(irItem->controlPts)) :
			(layout->hasControlPoints && !MotionControlPtsAreIdentity(layout->controlPts));
		const tjs_real *controlPts = irItem ? irItem->controlPts : layout->controlPts;
		const std::vector<MotionRenderSurface> *surfaces =
			(irItem && !irItem->surfaces.empty()) ? &irItem->surfaces : nullptr;
		tjs_int remapBase = 0;
		if (surfaces && !surfaces->empty()) {
			summary.m4SurfaceChainCommandCount++;
			if ((tjs_int)surfaces->size() > summary.m4MaxSurfaceChainLength) {
				summary.m4MaxSurfaceChainLength = (tjs_int)surfaces->size();
			}
		}

		for (tjs_int gy = 0; gy <= division; gy++) {
			tjs_real vGrid = (tjs_real)gy / (tjs_real)division;
			for (tjs_int gx = 0; gx <= division; gx++) {
				tjs_real uGrid = (tjs_real)gx / (tjs_real)division;
				tjs_real vx, vy, vz = 0.0;
				tjs_real sampleNdcX = 0.0;
				tjs_real sampleNdcY = 0.0;
				tjs_real sampleNdcZ = 0.0;
				if (surfaces && MotionEvalSurfaceChainVertex(*surfaces, uGrid, vGrid,
					viewportWidth, viewportHeight, vx, vy,
					&sampleNdcX, &sampleNdcY, &remapBase,
					&sampleNdcZ, &vz)) {
					summary.m4RemappedVertexCount += remapBase;
					summary.m4ViewportTransformedVertexCount++;
					if (summary.m4ViewportTransformedVertexCount == 1) {
						summary.m4SampleNdcX = sampleNdcX;
						summary.m4SampleNdcY = sampleNdcY;
						summary.m4SamplePixelX = vx;
						summary.m4SamplePixelY = vy;
					}
					// M4 trace: emit per-vertex JSONL for diff against
					// krkrsdl3 ground-truth at corner uvs.
					if (irItem) {
						MotionTraceM4EmitVertex(*surfaces, uGrid, vGrid,
							vx, vy, sampleNdcX, sampleNdcY,
							irItem->label, irItem->srcKey,
							irItem->opacity);
					}
				} else {
					tjs_real du = uGrid, dv = vGrid;
					if (useControlPts) {
						MotionEvalBezierSurface(controlPts, uGrid, vGrid, du, dv);
					}
					if (useAffine) {
						MotionApplyIconDrawAffineToPoint(drawAffine, du, dv, vx, vy);
					} else {
						vx = geomLeft + geomWidth * du;
						vy = geomTop + geomHeight * dv;
					}
				}
				tjs_real u = vGrid;
				tjs_real v = uGrid;
				tjs_int globalVidx = cmd.vertexOffset +
					gy * (division + 1) + gx;
				summary.vertexWriteCount++;
				float storedVx = (float)vx;
				float storedVy = (float)vy;
				MotionHashPlanDouble(vHash, storedVx);
				MotionHashPlanDouble(vHash, storedVy);
				summary.alphaWriteCount++;
				MotionHashPlanInt(aHash, alphaVal);
				summary.uvWriteCount++;
				MotionHashPlanDouble(uHash, u);
				MotionHashPlanDouble(uHash, v);

				size_t pi = (size_t)globalVidx * 2;
				if (pi + 1 < posBuf.size()) {
					posBuf[pi] = storedVx;
					posBuf[pi + 1] = storedVy;
				}
				if ((size_t)globalVidx < posBufZ.size()) {
					posBufZ[(size_t)globalVidx] = (float)vz;
				}
				if (pi + 1 < uvBuf.size()) {
					uvBuf[pi] = u;
					uvBuf[pi + 1] = v;
				}
				if ((size_t)globalVidx < alphaBuf.size()) {
					alphaBuf[(size_t)globalVidx] = alphaVal;
				}

				if (!firstVertexCollected) {
					firstVertexCollected = true;
					summary.firstVertexSample =
						ttstr(TJS_W("v[")) + ttstr(globalVidx) +
						TJS_W("]=(") + ttstr((tjs_int)std::lround(vx)) +
						TJS_W(",") + ttstr((tjs_int)std::lround(vy)) + TJS_W(")");
				}
				if (ci == summary.commandCount - 1 &&
					gy == division && gx == division) {
					lastVertexCollected = true;
					summary.lastVertexSample =
						ttstr(TJS_W("v[")) + ttstr(globalVidx) +
						TJS_W("]=(") + ttstr((tjs_int)std::lround(vx)) +
						TJS_W(",") + ttstr((tjs_int)std::lround(vy)) + TJS_W(")");
				}
			}
		}

		for (tjs_int gy = 0; gy < division; gy++) {
			for (tjs_int gx = 0; gx < division; gx++) {
				tjs_int v00 = cmd.vertexOffset + gy * (division + 1) + gx;
				tjs_int v01 = v00 + 1;
				tjs_int v10 = v00 + (division + 1);
				tjs_int v11 = v10 + 1;
				tjs_int vMax = cmd.vertexOffset + cmd.vertexCount - 1;
				if (v00 < cmd.vertexOffset || v01 < cmd.vertexOffset ||
					v10 < cmd.vertexOffset || v11 < cmd.vertexOffset ||
					v00 > vMax || v01 > vMax || v10 > vMax || v11 > vMax) {
					summary.rangesOk = false;
					if (summary.mismatchIndex < 0) {
						summary.mismatchKind = TJS_W("index-out-of-range");
						summary.mismatchIndex = ci;
						summary.mismatchDetails = ttstr(TJS_W("cmd ")) + ttstr(ci) +
							TJS_W(" cell(") + ttstr(gx) + TJS_W(",") + ttstr(gy) +
							TJS_W(") idx:") + ttstr(v00) + TJS_W(",") + ttstr(v01) +
							TJS_W(",") + ttstr(v10) + TJS_W(",") + ttstr(v11) +
							TJS_W(" range:") + ttstr(cmd.vertexOffset) +
							TJS_W("-") + ttstr(vMax);
					}
				}
				tjs_int i0 = cmd.indexOffset + (gy * division + gx) * 6;
				summary.indexWriteCount += 6;
				MotionHashPlanInt(iHash, v00);
				MotionHashPlanInt(iHash, v01);
				MotionHashPlanInt(iHash, v10);
				MotionHashPlanInt(iHash, v10);
				MotionHashPlanInt(iHash, v01);
				MotionHashPlanInt(iHash, v11);

				size_t ib = (size_t)i0;
				if (ib + 5 < idxBuf.size()) {
					idxBuf[ib] = v00;
					idxBuf[ib + 1] = v01;
					idxBuf[ib + 2] = v10;
					idxBuf[ib + 3] = v10;
					idxBuf[ib + 4] = v01;
					idxBuf[ib + 5] = v11;
				}

				if (ci == 0 && gy == 0 && gx == 0) {
					summary.firstIndexSample =
						ttstr(TJS_W("i[")) + ttstr(i0) +
						TJS_W("]=(") + ttstr(v00) + TJS_W(",") + ttstr(v01) +
						TJS_W(",") + ttstr(v10) + TJS_W(",") + ttstr(v11) + TJS_W(")");
					firstIndexCollected = true;
				}
			}
		}

		if (ci == summary.commandCount - 1 && division > 0) {
			tjs_int lastCellIdx = division * division - 1;
			tjs_int lastGy = division - 1;
			tjs_int lastGx = division - 1;
			tjs_int lv00 = cmd.vertexOffset + lastGy * (division + 1) + lastGx;
			tjs_int lv11 = lv00 + (division + 1) + 1;
			tjs_int li0 = cmd.indexOffset + lastCellIdx * 6;
			summary.lastIndexSample =
				ttstr(TJS_W("i[")) + ttstr(li0) +
				TJS_W("]=(") + ttstr(lv00) + TJS_W(",") +
				ttstr(lv00 + 1) + TJS_W(",") +
				ttstr(lv00 + division + 1) + TJS_W(",") + ttstr(lv11) + TJS_W(")");
		}
	}

	summary.prepared = summary.vertexWriteCount > 0 &&
		summary.indexWriteCount > 0 &&
		summary.alphaWriteCount > 0 &&
		summary.uvWriteCount > 0;
	summary.vertexHash = vHash;
	summary.indexHash = iHash;
	summary.alphaHash = aHash;
	summary.uvHash = uHash;

	tjs_uint64 paHash = (tjs_uint64)1469598103934665611ULL;
	tjs_uint64 uaHash = (tjs_uint64)1469598103934665619ULL;
	tjs_uint64 aaHash = (tjs_uint64)1469598103934665617ULL;
	tjs_uint64 iaHash = (tjs_uint64)1469598103934665613ULL;
	for (size_t vi = 0; vi < posBuf.size(); vi += 2) {
		MotionHashPlanDouble(paHash, posBuf[vi]);
		MotionHashPlanDouble(paHash, posBuf[vi + 1]);
	}
	for (size_t ui = 0; ui < uvBuf.size(); ui += 2) {
		MotionHashPlanDouble(uaHash, uvBuf[ui]);
		MotionHashPlanDouble(uaHash, uvBuf[ui + 1]);
	}
	for (size_t ai = 0; ai < alphaBuf.size(); ai++) {
		MotionHashPlanInt(aaHash, alphaBuf[ai]);
	}
	for (size_t ii = 0; ii < idxBuf.size(); ii++) {
		MotionHashPlanInt(iaHash, idxBuf[ii]);
	}

	bool phOk = (paHash == vHash);
	bool uhOk = (uaHash == uHash);
	bool ahOk = (aaHash == aHash);
	bool ihOk = (iaHash == iHash);
	summary.stagingArtifactPopulated = phOk && uhOk && ahOk && ihOk;
	summary.stagingVertexArrayHash = paHash;
	summary.stagingIndexArrayHash = iaHash;
	summary.stagingAlphaArrayHash = aaHash;
	summary.stagingUvArrayHash = uaHash;

	if (!posBuf.empty()) {
		summary.stagingFirstPositionSample =
			ttstr(TJS_W("p[0]=(")) + ttstr((tjs_int)std::lround(posBuf[0])) +
			TJS_W(",") + ttstr((tjs_int)std::lround(posBuf[1])) + TJS_W(")");
		size_t last = posBuf.size() - 2;
		summary.stagingLastPositionSample =
			ttstr(TJS_W("p[")) + ttstr((tjs_int)(last / 2)) + TJS_W("]=(") +
			ttstr((tjs_int)std::lround(posBuf[last])) +
			TJS_W(",") + ttstr((tjs_int)std::lround(posBuf[last + 1])) + TJS_W(")");
	}
	if (!uvBuf.empty()) {
		summary.stagingFirstUvSample =
			ttstr(TJS_W("uv[0]=(")) + ttstr((tjs_int)std::lround(uvBuf[0] * 1000)) +
			TJS_W(",") + ttstr((tjs_int)std::lround(uvBuf[1] * 1000)) + TJS_W(")");
		size_t last = uvBuf.size() - 2;
		summary.stagingLastUvSample =
			ttstr(TJS_W("uv[")) + ttstr((tjs_int)(last / 2)) + TJS_W("]=(") +
			ttstr((tjs_int)std::lround(uvBuf[last] * 1000)) +
			TJS_W(",") + ttstr((tjs_int)std::lround(uvBuf[last + 1] * 1000)) + TJS_W(")");
	}
	if (!alphaBuf.empty()) {
		summary.stagingFirstAlphaSample =
			ttstr(TJS_W("a[0]=")) + ttstr(alphaBuf[0]);
		size_t last = alphaBuf.size() - 1;
		summary.stagingLastAlphaSample =
			ttstr(TJS_W("a[")) + ttstr((tjs_int)last) + TJS_W("]=") + ttstr(alphaBuf[last]);
	}
	if (!idxBuf.empty()) {
		tjs_int idxMin = idxBuf[0];
		tjs_int idxMax = idxBuf[0];
		for (size_t ii = 0; ii < idxBuf.size(); ii++) {
			if (idxBuf[ii] < idxMin) idxMin = idxBuf[ii];
			if (idxBuf[ii] > idxMax) idxMax = idxBuf[ii];
		}
		summary.stagingIndexRangeMin = ttstr(idxMin);
		summary.stagingIndexRangeMax = ttstr(idxMax);
	}

	if (!summary.stagingArtifactPopulated) {
		summary.perCommandOk = false;
		if (summary.mismatchIndex < 0) {
			summary.mismatchKind = TJS_W("staging-hash-mismatch");
			summary.mismatchDetails = ttstr(TJS_W("posOk:")) + ttstr(phOk ? 1 : 0) +
				TJS_W(" uvOk:") + ttstr(uhOk ? 1 : 0) +
				TJS_W(" alphaOk:") + ttstr(ahOk ? 1 : 0) +
				TJS_W(" idxOk:") + ttstr(ihOk ? 1 : 0);
		}
	}
	if (summary.m4SurfaceChainCommandCount > 0 &&
		summary.m4ViewportTransformedVertexCount <= 0) {
		summary.perCommandOk = false;
		if (summary.mismatchIndex < 0) {
			summary.mismatchKind = TJS_W("m4-surface-chain-empty");
			summary.mismatchDetails =
				TJS_W("surfaceCommands:") +
				ttstr(summary.m4SurfaceChainCommandCount);
		}
	}

	tjs_int expectedTotVertex = totalVertexCount;
	tjs_int expectedTotIndex = totalIndexCount;
	tjs_int expectedTotCell = totalCellCount;
	if (summary.vertexWriteCount != expectedTotVertex ||
		summary.indexWriteCount != expectedTotIndex ||
		summary.cellBufferCount != expectedTotCell) {
		summary.perCommandOk = false;
		if (summary.mismatchIndex < 0) {
			summary.mismatchKind = TJS_W("write-count-mismatch");
			summary.mismatchDetails = ttstr(TJS_W("vWrote:")) + ttstr(summary.vertexWriteCount) +
				TJS_W("/") + ttstr(expectedTotVertex) +
				TJS_W(" iWrote:") + ttstr(summary.indexWriteCount) +
				TJS_W("/") + ttstr(expectedTotIndex) +
				TJS_W(" cells:") + ttstr(summary.cellBufferCount) +
				TJS_W("/") + ttstr(expectedTotCell);
		}
	}

	summary.offsetsOk = true;
	tjs_int expectedVOff = 0;
	tjs_int expectedIOff = 0;
	tjs_int expectedCOff = 0;
	for (tjs_int ci = 0; ci < summary.commandCount && summary.offsetsOk; ci++) {
		const MotionRendererCommandEntryInfo &cmd = rendererCommandEntries[(size_t)ci];
		if (cmd.vertexOffset != expectedVOff ||
			cmd.indexOffset != expectedIOff ||
			cmd.firstCellIndex != expectedCOff) {
			summary.offsetsOk = false;
			if (summary.mismatchIndex < 0) {
				summary.mismatchKind = TJS_W("offset");
				summary.mismatchIndex = ci;
				summary.mismatchDetails = ttstr(TJS_W("cmd ")) + ttstr(ci) +
					TJS_W(" vo:") + ttstr(cmd.vertexOffset) + TJS_W("/") + ttstr(expectedVOff) +
					TJS_W(" io:") + ttstr(cmd.indexOffset) + TJS_W("/") + ttstr(expectedIOff) +
					TJS_W(" fc:") + ttstr(cmd.firstCellIndex) + TJS_W("/") + ttstr(expectedCOff);
			}
		}
		expectedVOff += cmd.vertexCount;
		expectedIOff += cmd.indexCount;
		expectedCOff += cmd.cellCount;
	}

	if (summary.perCommandOk && summary.rangesOk && summary.offsetsOk) {
		if (summary.mismatchIndex < 0) {
			summary.mismatchKind = TJS_W("none");
			summary.mismatchIndex = -1;
		}
	}

	if (stagingArtifact) {
		stagingArtifact->position = posBuf;
		stagingArtifact->uv = uvBuf;
		stagingArtifact->alpha = alphaBuf;
		stagingArtifact->index = idxBuf;
		stagingArtifact->coordZ = posBufZ;
	}

	return summary;
}

static MotionShadowDrawDiagnostic MotionConsumeRendererStagingForShadowDraw(
	const std::vector<MotionRendererCommandEntryInfo> &rendererCommandEntries,
	const MotionStagingArtifact &stagingArtifact,
	const MotionRendererUploadPrepSummary &uploadPrepSummary)
{
	MotionShadowDrawDiagnostic diag;
	diag.ran = true;
	diag.source = TJS_W("rendererCommandEntries+stagingArtifact");
	diag.consumedCommandCount = (tjs_int)rendererCommandEntries.size();
	diag.consumedVertexCount = (tjs_int)(stagingArtifact.position.size() / 2);
	diag.consumedIndexCount = (tjs_int)stagingArtifact.index.size();

	if (stagingArtifact.position.empty() || stagingArtifact.index.empty() ||
		rendererCommandEntries.empty()) {
		diag.consumed = false;
		diag.mismatchKind = TJS_W("empty-staging");
		return diag;
	}

	tjs_int totalCells = 0;
	for (size_t i = 0; i < rendererCommandEntries.size(); i++) {
		totalCells += rendererCommandEntries[i].cellCount;
	}
	diag.consumedCellCount = totalCells;

	tjs_uint64 posHash = (tjs_uint64)1469598103934665611ULL;
	tjs_uint64 uvHash = (tjs_uint64)1469598103934665619ULL;
	tjs_uint64 alphaHash = (tjs_uint64)1469598103934665617ULL;
	tjs_uint64 indexHash = (tjs_uint64)1469598103934665613ULL;

	bool covBoundsInit = false;
	bool firstTriangleCollected = false;
	diag.coverageMinX = 0.0;
	diag.coverageMinY = 0.0;
	diag.coverageMaxX = 0.0;
	diag.coverageMaxY = 0.0;
	diag.coveredTriangleCount = 0;
	diag.coveredCellCount = 0;
	diag.nonZeroAlphaVertexCount = 0;
	diag.indexInRange = true;
	tjs_int maxVertIdx = (tjs_int)(stagingArtifact.position.size() / 2) - 1;

	tjs_int idxMin = 0;
	tjs_int idxMax = 0;
	bool idxFirst = true;

	for (tjs_int ci = 0; ci < (tjs_int)rendererCommandEntries.size(); ci++) {
		const MotionRendererCommandEntryInfo &cmd = rendererCommandEntries[(size_t)ci];
		if (cmd.indexCount <= 0 || cmd.indexOffset < 0) continue;

		tjs_int idxEnd = cmd.indexOffset + cmd.indexCount;
		if (idxEnd > (tjs_int)stagingArtifact.index.size()) {
			diag.indexInRange = false;
			if (diag.mismatchIndex < 0) {
				diag.mismatchIndex = ci;
				diag.mismatchKind = TJS_W("idx-out-of-bounds");
				diag.mismatchDetails = ttstr(TJS_W("cmd ")) + ttstr(ci) +
					TJS_W(" indexRange:") + ttstr(cmd.indexOffset) +
					TJS_W("-") + ttstr(idxEnd - 1) +
					TJS_W(" bufSize:") + ttstr((tjs_int)stagingArtifact.index.size());
			}
		}

		tjs_int quads = cmd.indexCount / 6;
		tjs_int actualCells = 0;
		for (tjs_int qi = 0; qi < quads; qi++) {
			tjs_int i0 = cmd.indexOffset + qi * 6;
			if (i0 + 5 >= (tjs_int)stagingArtifact.index.size()) continue;
			tjs_int v0 = stagingArtifact.index[(size_t)i0];
			tjs_int v1 = stagingArtifact.index[(size_t)i0 + 1];
			tjs_int v2 = stagingArtifact.index[(size_t)i0 + 2];
			tjs_int v4 = stagingArtifact.index[(size_t)i0 + 4];
			tjs_int v5 = stagingArtifact.index[(size_t)i0 + 5];

			bool vertexRangeOk = !(v0 < 0 || v1 < 0 || v2 < 0 || v4 < 0 || v5 < 0 ||
				v0 > maxVertIdx || v1 > maxVertIdx || v2 > maxVertIdx ||
				v4 > maxVertIdx || v5 > maxVertIdx);
			if (!vertexRangeOk) {
				diag.indexInRange = false;
				if (diag.mismatchIndex < 0) {
					diag.mismatchIndex = ci;
					diag.mismatchKind = TJS_W("idx-out-of-vertex-range");
					diag.mismatchDetails = ttstr(TJS_W("cmd ")) + ttstr(ci) +
						TJS_W(" quads:") + ttstr(qi) +
						TJS_W(" idx:") + ttstr(v0) + TJS_W(",") + ttstr(v1) +
						TJS_W(",") + ttstr(v2) + TJS_W(",") + ttstr(v4) +
						TJS_W(",") + ttstr(v5) +
						TJS_W(" maxVert:") + ttstr(maxVertIdx);
				}
				continue;
			}

			float px0 = stagingArtifact.position[(size_t)v0 * 2];
			float py0 = stagingArtifact.position[(size_t)v0 * 2 + 1];
			float px1 = stagingArtifact.position[(size_t)v1 * 2];
			float py1 = stagingArtifact.position[(size_t)v1 * 2 + 1];
			float px2 = stagingArtifact.position[(size_t)v2 * 2];
			float py2 = stagingArtifact.position[(size_t)v2 * 2 + 1];
			float px4 = stagingArtifact.position[(size_t)v4 * 2];
			float py4 = stagingArtifact.position[(size_t)v4 * 2 + 1];
			float px5 = stagingArtifact.position[(size_t)v5 * 2];
			float py5 = stagingArtifact.position[(size_t)v5 * 2 + 1];

			float tri1X[3] = { px0, px1, px2 };
			float tri1Y[3] = { py0, py1, py2 };
			float tri2X[3] = { px2, px4, px5 };
			float tri2Y[3] = { py2, py4, py5 };

			for (int t = 0; t < 2; t++) {
				const float *tx = (t == 0) ? tri1X : tri2X;
				const float *ty = (t == 0) ? tri1Y : tri2Y;
				float tMinX = std::min(std::min(tx[0], tx[1]), tx[2]);
				float tMaxX = std::max(std::max(tx[0], tx[1]), tx[2]);
				float tMinY = std::min(std::min(ty[0], ty[1]), ty[2]);
				float tMaxY = std::max(std::max(ty[0], ty[1]), ty[2]);
				if (!covBoundsInit) {
					covBoundsInit = true;
					diag.coverageMinX = tMinX;
					diag.coverageMinY = tMinY;
					diag.coverageMaxX = tMaxX;
					diag.coverageMaxY = tMaxY;
				} else {
					if (tMinX < diag.coverageMinX) diag.coverageMinX = tMinX;
					if (tMinY < diag.coverageMinY) diag.coverageMinY = tMinY;
					if (tMaxX > diag.coverageMaxX) diag.coverageMaxX = tMaxX;
					if (tMaxY > diag.coverageMaxY) diag.coverageMaxY = tMaxY;
				}
				diag.coveredTriangleCount++;
			}

			actualCells++;
			if (!firstTriangleCollected) {
				firstTriangleCollected = true;
				float cx = (px0 + px1 + px2 + px4 + px5) / 5.0f;
				float cy = (py0 + py1 + py2 + py4 + py5) / 5.0f;
				diag.firstConsumedTriangle =
					ttstr(TJS_W("cmd")) + ttstr(ci) +
					TJS_W(" q") + ttstr(qi) +
					TJS_W(" tri0=(") + ttstr((tjs_int)std::lround(px0)) +
					TJS_W(",") + ttstr((tjs_int)std::lround(py0)) +
					TJS_W(")-(") + ttstr((tjs_int)std::lround(px1)) +
					TJS_W(",") + ttstr((tjs_int)std::lround(py1)) +
					TJS_W(")-(") + ttstr((tjs_int)std::lround(px2)) +
					TJS_W(",") + ttstr((tjs_int)std::lround(py2)) +
					TJS_W(") center~(") + ttstr((tjs_int)std::lround(cx)) +
					TJS_W(",") + ttstr((tjs_int)std::lround(cy)) + TJS_W(")");
				tjs_int a0 = (size_t)v0 < stagingArtifact.alpha.size() ? stagingArtifact.alpha[(size_t)v0] : -1;
				tjs_int a1 = (size_t)v1 < stagingArtifact.alpha.size() ? stagingArtifact.alpha[(size_t)v1] : -1;
				tjs_int a2 = (size_t)v2 < stagingArtifact.alpha.size() ? stagingArtifact.alpha[(size_t)v2] : -1;
				diag.firstConsumedPixelSample =
					ttstr(TJS_W("alpha(")) + ttstr(v0) + TJS_W(",") + ttstr(v1) +
					TJS_W(",") + ttstr(v2) + TJS_W(")=(") + ttstr(a0) +
					TJS_W(",") + ttstr(a1) + TJS_W(",") + ttstr(a2) + TJS_W(")");
			}
		}
		diag.coveredCellCount += actualCells;

		for (int ii = 0; ii < cmd.indexCount; ii++) {
			tjs_int vi = cmd.indexOffset + ii;
			if (vi >= 0 && (size_t)vi < stagingArtifact.index.size()) {
				tjs_int sv = stagingArtifact.index[(size_t)vi];
				if (idxFirst) { idxFirst = false; idxMin = sv; idxMax = sv; }
				else { if (sv < idxMin) idxMin = sv; if (sv > idxMax) idxMax = sv; }
				MotionHashPlanInt(indexHash, sv);
			} else {
				diag.indexInRange = false;
				if (diag.mismatchIndex < 0) {
					diag.mismatchIndex = ci;
					diag.mismatchKind = TJS_W("idx-oob-read");
					diag.mismatchDetails = ttstr(TJS_W("cmd ")) + ttstr(ci) +
						TJS_W(" idx:") + ttstr(vi) +
						TJS_W(" bufSize:") + ttstr((tjs_int)stagingArtifact.index.size());
				}
			}
		}
	}

	for (size_t vi = 0; vi < stagingArtifact.position.size(); vi += 2) {
		MotionHashPlanDouble(posHash, stagingArtifact.position[vi]);
		MotionHashPlanDouble(posHash, stagingArtifact.position[vi + 1]);
	}
	for (size_t ui = 0; ui < stagingArtifact.uv.size(); ui += 2) {
		MotionHashPlanDouble(uvHash, stagingArtifact.uv[ui]);
		MotionHashPlanDouble(uvHash, stagingArtifact.uv[ui + 1]);
	}
	for (size_t ai = 0; ai < stagingArtifact.alpha.size(); ai++) {
		tjs_int a = stagingArtifact.alpha[ai];
		MotionHashPlanInt(alphaHash, a);
		if (a > 0) diag.nonZeroAlphaVertexCount++;
	}

	diag.consumedPosHash = posHash;
	diag.consumedUvHash = uvHash;
	diag.consumedAlphaHash = alphaHash;
	diag.consumedIndexHash = indexHash;

	diag.consumedCompositeHash = ((posHash * (tjs_uint64)1099511628211ULL) ^
		(uvHash * (tjs_uint64)1099511628213ULL) ^
		(alphaHash * (tjs_uint64)1099511628217ULL) ^
		(indexHash * (tjs_uint64)1099511628219ULL));

	diag.indexRangeMin = ttstr(idxMin);
	diag.indexRangeMax = ttstr(idxMax);

	if (diag.mismatchIndex < 0) {
		diag.mismatchKind = TJS_W("none");
	}

	diag.consumed = covBoundsInit && diag.coveredTriangleCount > 0;
	if (!diag.consumed && diag.mismatchIndex < 0) {
		diag.mismatchKind = TJS_W("no-coverage");
	}

	if (!diag.indexInRange && diag.mismatchIndex < 0) {
		diag.mismatchKind = TJS_W("index-not-in-range");
		diag.mismatchIndex = 0;
	}

	return diag;
}

static bool MotionRunAffineFrontMeshSelfTest(MotionAffineFrontMeshDiag &diag)
{
	diag.affineSelfTested = 1;

	MotionFrameLayoutState parent;
	parent.hasContent = true;
	parent.coordX = 100.0;
	parent.coordY = 50.0;
	parent.angle = 30.0;
	parent.sx = 0.25;
	parent.sy = -0.125;
	parent.scaleX = 2.0;
	parent.scaleY = 3.0;

	MotionFrameLayoutState child;
	child.hasContent = true;
	child.coordX = 7.0;
	child.coordY = -11.0;
	child.angle = -10.0;
	child.sx = -0.2;
	child.sy = 0.1;
	child.scaleX = 0.5;
	child.scaleY = 1.5;

	MotionLayoutTransform root;
	root.affine = MotionAffineIdentity();
	MotionLayoutTransform parentTx = MotionApplyLayoutFrame(root, parent);
	MotionLayoutTransform childTx = MotionApplyLayoutFrame(parentTx, child);

	tjs_real sx = 2.0;
	tjs_real sy = 3.0;
	tjs_real ox, oy;
	MotionApplyAffineToPoint(1.0, 1.0,
		0.0, 0.0,
		1.0, 1.0,
		0.0, 0.0,
		sx, sy,
		0.25, -0.5,
		0.0,
		0.0, 0.0,
		ox, oy);
	bool shearOk = std::abs(ox - 2.75) < 0.001 && std::abs(oy - 2.0) < 0.001;
	diag.affineSelfTestShearOk = shearOk ? 1 : 0;

	MotionAffine2D expectedParent = MotionAffineMultiply(MotionAffineIdentity(),
		MotionBuildLayoutLocalAffine(parent));
	MotionAffine2D expectedChild = MotionAffineMultiply(expectedParent,
		MotionBuildLayoutLocalAffine(child));
	bool nestedOk =
		std::abs(childTx.affine.m00 - expectedChild.m00) < 0.001 &&
		std::abs(childTx.affine.m01 - expectedChild.m01) < 0.001 &&
		std::abs(childTx.affine.m10 - expectedChild.m10) < 0.001 &&
		std::abs(childTx.affine.m11 - expectedChild.m11) < 0.001 &&
		std::abs(childTx.affine.tx - expectedChild.tx) < 0.001 &&
		std::abs(childTx.affine.ty - expectedChild.ty) < 0.001;
	diag.affineSelfTestNestedOk = nestedOk ? 1 : 0;

	MotionAffine2D quad = MotionBuildIconDrawAffine(childTx.affine,
		4.0, -6.0, 32.0, 48.0, 10.0, 12.0);
	tjs_real q0x, q0y, q1x, q1y, q2x, q2y, q3x, q3y;
	MotionApplyIconDrawAffineToPoint(quad, 0.0, 0.0, q0x, q0y);
	MotionApplyIconDrawAffineToPoint(quad, 1.0, 0.0, q1x, q1y);
	MotionApplyIconDrawAffineToPoint(quad, 1.0, 1.0, q2x, q2y);
	MotionApplyIconDrawAffineToPoint(quad, 0.0, 1.0, q3x, q3y);
	bool nonAxisOk =
		(std::abs(q0y - q1y) > 0.001) ||
		(std::abs(q1x - q2x) > 0.001) ||
		(std::abs(q2y - q3y) > 0.001) ||
		(std::abs(q3x - q0x) > 0.001);
	diag.affineSelfTestNonAxisAligned = nonAxisOk ? 1 : 0;

	diag.affineConsumerSelfTested = 1;
	MotionResource synthetic;
	synthetic.opened = true;
	synthetic.parsed = true;
	synthetic.storage = TJS_W("affine-front-mesh-selftest");
	synthetic.width = 320;
	synthetic.height = 240;

	MotionLayoutInfo layout;
	layout.name = TJS_W("affine/front-mesh-selftest");
	layout.sourceKey = TJS_W("src/affine-selftest");
	layout.objectName = TJS_W("affine_selftest");
	layout.motionName = TJS_W("show");
	layout.hitObjectName = TJS_W("affine_selftest");
	layout.hitObjectPath = TJS_W("affine_selftest");
	layout.hitStateName = TJS_W("show");
	layout.hitStateClass = TJS_W("show");
	layout.hitStateGroup = TJS_W("active");
	layout.hitStatePath = TJS_W("affine_selftest/show");
	layout.hitPartName = TJS_W("front");
	layout.hitRole = TJS_W("control");
	layout.frontDrawCandidate = true;
	layout.drawCount = 1;
	layout.opacityMin = 1.0;
	layout.opacityMax = 1.0;
	layout.angle = child.angle;
	layout.sx = child.sx;
	layout.sy = child.sy;
	layout.ox = 4.0;
	layout.oy = -6.0;
	layout.iconOriginX = 10.0;
	layout.iconOriginY = 12.0;
	layout.iconWidth = 32.0;
	layout.iconHeight = 48.0;
	layout.affineValid = true;
	layout.affine = quad;
	tjs_real minX, minY, maxX, maxY;
	MotionCollectAffineQuadCorners(quad, minX, minY, maxX, maxY);
	layout.left = minX;
	layout.top = minY;
	layout.width = maxX - minX;
	layout.height = maxY - minY;
	layout.centerX = layout.left + layout.width * 0.5;
	layout.centerY = layout.top + layout.height * 0.5;
	synthetic.layouts.push_back(layout);

	std::vector<MotionRendererCommandEntryInfo> commands =
		MotionBuildRendererCommandEntriesFromDrawItems(&synthetic, 2.0);
	MotionStagingArtifact staging;
	MotionRendererUploadPrepSummary upload =
		MotionPrepareRendererUploadFromCommands(commands, &synthetic, 2.0, &staging);
	MotionShadowDrawDiagnostic shadow =
		MotionConsumeRendererStagingForShadowDraw(commands, staging, upload);

	diag.affineConsumerCommandCount = (tjs_int)commands.size();
	diag.affineConsumerVertexCount = upload.vertexBufferCount;
	diag.affineConsumerIndexCount = upload.indexBufferCount;
	diag.affineConsumerCellCount = upload.cellBufferCount;
	diag.affineConsumerTriangleCount = shadow.coveredTriangleCount;
	diag.affineConsumerUploadPrepared = upload.prepared ? 1 : 0;
	diag.affineConsumerStagingPopulated = upload.stagingArtifactPopulated ? 1 : 0;
	diag.affineConsumerShadowConsumed = shadow.consumed ? 1 : 0;

	bool consumerNonAxisOk = false;
	if (staging.position.size() >= 18) {
		tjs_real sx0 = staging.position[0];
		tjs_real sy0 = staging.position[1];
		tjs_real sx1p = staging.position[2];
		tjs_real sy1p = staging.position[3];
		tjs_real sx3p = staging.position[6];
		tjs_real sy3p = staging.position[7];
		consumerNonAxisOk =
			std::abs(sy0 - sy1p) > 0.001 ||
			std::abs(sx0 - sx3p) > 0.001;
	}
	diag.affineConsumerSelfTestNonAxisAligned = consumerNonAxisOk ? 1 : 0;
	bool consumerOk =
		commands.size() == 1 &&
		upload.prepared &&
		upload.stagingArtifactPopulated &&
		upload.perCommandOk &&
		upload.rangesOk &&
		upload.offsetsOk &&
		shadow.consumed &&
		shadow.indexInRange &&
		shadow.coveredCellCount == 4 &&
		shadow.coveredTriangleCount == 8 &&
		consumerNonAxisOk;
	diag.affineConsumerSelfTestPassed = consumerOk ? 1 : 0;
	if (!consumerOk) {
		ttstr uploadKind = upload.mismatchKind.IsEmpty() ?
			ttstr(TJS_W("empty")) : upload.mismatchKind;
		ttstr shadowKind = shadow.mismatchKind.IsEmpty() ?
			ttstr(TJS_W("empty")) : shadow.mismatchKind;
		ttstr details = TJS_W("commands=") + ttstr((tjs_int)commands.size()) +
			TJS_W(" upload=") + uploadKind +
			TJS_W(" shadow=") + shadowKind +
			TJS_W(" nonAxis=") + ttstr(consumerNonAxisOk ? 1 : 0);
		diag.consumerSelfTestMismatchKind =
			(uploadKind == TJS_W("none")) ?
			shadowKind.AsNarrowStdString() : uploadKind.AsNarrowStdString();
		diag.consumerSelfTestDetails = details.AsNarrowStdString();
	}

	diag.affineSelfTestPassed =
		(shearOk && nestedOk && nonAxisOk && consumerOk) ? 1 : 0;
	return diag.affineSelfTestPassed == 1;
}

static bool MotionRunM2ProgressSurfaceSelfTest(MotionRenderMethodSemanticsDiag &diag)
{
	diag.m2SelfTested = 1;

	MotionSurfaceStackState root;
	root.valid = true;
	root.originX = 100.0;
	root.originY = 80.0;
	root.width = 640.0;
	root.height = 480.0;
	root.layoutChain = MotionMat4Identity();
	root.lastSurface = MotionBuildLayoutRenderSurface(root.layoutChain,
		TJS_W("m2/root"), root.originX, root.originY, root.width, root.height);
	root.surfaces.push_back(root.lastSurface);

	MotionFrameLayoutState layoutA;
	layoutA.hasContent = true;
	layoutA.src = "layout";
	layoutA.hasCoord = true;
	layoutA.coordX = 20.0;
	layoutA.coordY = 30.0;
	layoutA.scaleX = 1.25;
	layoutA.scaleY = 0.75;
	layoutA.angle = 10.0;
	layoutA.sx = 0.05;
	layoutA.sy = -0.025;

	MotionSurfaceStackState stateA = MotionAdvanceSurfaceStackForFrame(root,
		layoutA, 3, root.originX, root.originY, root.width, root.height,
		root.originX, root.originY, root.width, root.height,
		TJS_W("m2/layout-a"), nullptr);

	MotionFrameLayoutState layoutB;
	layoutB.hasContent = true;
	layoutB.src = "layout";
	layoutB.hasCoord = true;
	layoutB.coordX = -12.0;
	layoutB.coordY = 9.0;
	layoutB.scaleX = 0.8;
	layoutB.scaleY = 1.1;
	layoutB.angle = -7.0;
	layoutB.sx = -0.04;
	layoutB.sy = 0.02;

	MotionSurfaceStackState stateB = MotionAdvanceSurfaceStackForFrame(stateA,
		layoutB, 3, root.originX, root.originY, root.width, root.height,
		stateA.originX, stateA.originY, stateA.width, stateA.height,
		TJS_W("m2/layout-b"), nullptr);

	MotionFrameLayoutState leaf;
	leaf.hasContent = true;
	leaf.src = "src/synth/icon";
	leaf.hasCoord = true;
	leaf.coordX = 17.0;
	leaf.coordY = -5.0;
	leaf.scaleX = 1.2;
	leaf.scaleY = 0.9;
	leaf.angle = 15.0;
	leaf.sx = 0.03;
	leaf.sy = -0.02;
	leaf.ox = 3.0;
	leaf.oy = 4.0;
	leaf.opacity = 0.75;

	const tjs_real iconOriginX = 8.0;
	const tjs_real iconOriginY = 6.0;
	const tjs_real iconW = 32.0;
	const tjs_real iconH = 24.0;
	MotionSurfaceStackState leafState = MotionAdvanceSurfaceStackForFrame(stateB,
		leaf, 2, iconOriginX, iconOriginY, iconW, iconH,
		stateB.originX, stateB.originY, stateB.width, stateB.height,
		TJS_W("m2/icon-leaf"), nullptr);

		MotionMat4 expected = MotionMat4Identity();
		expected = MotionMat4Multiply(expected, MotionMat4BuildModelFromFrame(layoutA));
		expected = MotionMat4Multiply(expected, MotionMat4BuildModelFromFrame(layoutB));
	MotionMat4 leafModel = MotionMat4ApplyIconLocal(
		MotionMat4BuildModelFromFrame(leaf),
		iconOriginX, iconOriginY, leaf.ox, leaf.oy, iconW, iconH);
		expected = MotionMat4Multiply(
			MotionMat4Ortho(-stateB.originX, stateB.width - stateB.originX,
				stateB.height - stateB.originY, -stateB.originY,
				kMotionDefaultLimitZMax, -kMotionDefaultLimitZMax),
			MotionMat4Multiply(expected, leafModel));

	tjs_real maxErr = 0.0;
	bool ok = leafState.valid &&
		!leafState.unsupportedDemux &&
		leafState.lastSurface.type == 2 &&
		std::abs(leafState.lastSurface.opa - leaf.opacity) <= 0.001 &&
		MotionMat4AlmostEqual(leafState.lastSurface.matTrans, expected, 0.001, maxErr);

	diag.m2SampleMaxAbsError = maxErr;
	diag.m2SampleMat00 = leafState.lastSurface.matTrans.m[0];
	diag.m2SampleMat11 = leafState.lastSurface.matTrans.m[5];
	diag.m2SampleMat03 = leafState.lastSurface.matTrans.m[3];
	diag.m2SampleMat13 = leafState.lastSurface.matTrans.m[7];
	diag.m2SelfTestPassed = ok ? 1 : 0;
	return ok;
}

static bool MotionRunM3DemuxSurfaceSelfTest(MotionRenderMethodSemanticsDiag &diag)
{
	diag.m3SelfTested = 1;

	MotionSurfaceStackState root;
	root.valid = true;
	root.originX = 100.0;
	root.originY = 80.0;
	root.width = 640.0;
	root.height = 480.0;
	root.layoutChain = MotionMat4Identity();
	root.lastSurface = MotionBuildLayoutRenderSurface(root.layoutChain,
		TJS_W("m3/root"), root.originX, root.originY, root.width, root.height);
	root.surfaces.push_back(root.lastSurface);

	MotionFrameLayoutState clipFrame;
	clipFrame.hasContent = true;
	clipFrame.src = "src/synth/clip";
	clipFrame.hasCoord = true;
	clipFrame.coordX = 40.0;
	clipFrame.coordY = -18.0;
	clipFrame.scaleX = 1.15;
	clipFrame.scaleY = 0.85;
	clipFrame.angle = 8.0;
	clipFrame.sx = 0.02;
	clipFrame.sy = -0.03;
	clipFrame.opacity = 0.8;

	const tjs_real clipOriginX = 12.0;
	const tjs_real clipOriginY = 10.0;
	const tjs_real clipW = 160.0;
	const tjs_real clipH = 96.0;
	MotionSurfaceStackState clipState = MotionAdvanceSurfaceStackForFrame(root,
		clipFrame, 2, clipOriginX, clipOriginY, clipW, clipH,
		root.originX, root.originY, root.width, root.height,
		TJS_W("m3/plate"), nullptr);

	MotionFrameLayoutState layoutFrame;
	layoutFrame.hasContent = true;
	layoutFrame.src = "layout";
	layoutFrame.hasCoord = true;
	layoutFrame.coordX = -7.0;
	layoutFrame.coordY = 13.0;
	layoutFrame.scaleX = 0.7;
	layoutFrame.scaleY = 1.3;
	layoutFrame.angle = -5.0;
	layoutFrame.sx = -0.01;
	layoutFrame.sy = 0.04;

	MotionSurfaceStackState layoutState = MotionAdvanceSurfaceStackForFrame(clipState,
		layoutFrame, 3, root.originX, root.originY, root.width, root.height,
		clipState.originX, clipState.originY, clipState.width, clipState.height,
		TJS_W("m3/layout-after-plate"), nullptr);

	MotionFrameLayoutState letterFrame;
	letterFrame.hasContent = true;
	letterFrame.src = "src/synth/letter";
	letterFrame.hasCoord = true;
	letterFrame.coordX = 11.0;
	letterFrame.coordY = 5.0;
	letterFrame.scaleX = 0.9;
	letterFrame.scaleY = 1.1;
	letterFrame.angle = 12.0;
	letterFrame.sx = 0.03;
	letterFrame.sy = -0.02;
	letterFrame.ox = 2.0;
	letterFrame.oy = 1.0;
	letterFrame.opacity = 0.6;

	const tjs_real letterOriginX = 4.0;
	const tjs_real letterOriginY = 3.0;
	const tjs_real letterW = 48.0;
	const tjs_real letterH = 36.0;
	MotionSurfaceStackState letterState = MotionAdvanceSurfaceStackForFrame(layoutState,
		letterFrame, 2, letterOriginX, letterOriginY, letterW, letterH,
		layoutState.originX, layoutState.originY, layoutState.width, layoutState.height,
		TJS_W("m3/letter"), nullptr);

	MotionFrameLayoutState directFrame;
	directFrame.hasContent = true;
	directFrame.src = "src/synth/direct-child";
	directFrame.hasCoord = true;
	directFrame.coordX = -3.0;
	directFrame.coordY = 8.0;
	directFrame.scaleX = 1.05;
	directFrame.scaleY = 0.95;
	directFrame.angle = -9.0;
	directFrame.sx = 0.01;
	directFrame.sy = 0.02;
	directFrame.ox = 1.0;
	directFrame.oy = 2.0;
	directFrame.opacity = 0.7;
	const tjs_real directOriginX = 5.0;
	const tjs_real directOriginY = 4.0;
	const tjs_real directW = 24.0;
	const tjs_real directH = 18.0;
	MotionSurfaceStackState directState = MotionAdvanceSurfaceStackForFrame(letterState,
		directFrame, 2, directOriginX, directOriginY, directW, directH,
		letterState.originX, letterState.originY, letterState.width, letterState.height,
		TJS_W("m3/direct-child"), nullptr);

	MotionFrameLayoutState demuxLayoutFrame;
	demuxLayoutFrame.hasContent = true;
	demuxLayoutFrame.src = "layout";
	demuxLayoutFrame.hasCoord = true;
	demuxLayoutFrame.coordX = 6.0;
	demuxLayoutFrame.coordY = -4.0;
	demuxLayoutFrame.scaleX = 1.1;
	demuxLayoutFrame.scaleY = 0.8;
	demuxLayoutFrame.angle = 4.0;
	demuxLayoutFrame.sx = 0.02;
	demuxLayoutFrame.sy = -0.01;
	MotionSurfaceStackState demuxLayoutState = MotionAdvanceSurfaceStackForFrame(letterState,
		demuxLayoutFrame, 3, root.originX, root.originY, root.width, root.height,
		letterState.originX, letterState.originY, letterState.width, letterState.height,
		TJS_W("m3/layout-child"), nullptr);

	MotionFrameLayoutState nestedLeafFrame;
	nestedLeafFrame.hasContent = true;
	nestedLeafFrame.src = "src/synth/nested-leaf";
	nestedLeafFrame.hasCoord = true;
	nestedLeafFrame.coordX = 9.0;
	nestedLeafFrame.coordY = -2.0;
	nestedLeafFrame.scaleX = 0.85;
	nestedLeafFrame.scaleY = 1.2;
	nestedLeafFrame.angle = 6.0;
	nestedLeafFrame.sx = -0.02;
	nestedLeafFrame.sy = 0.01;
	nestedLeafFrame.ox = 1.5;
	nestedLeafFrame.oy = 2.5;
	nestedLeafFrame.opacity = 0.65;
	const tjs_real nestedOriginX = 3.0;
	const tjs_real nestedOriginY = 2.0;
	const tjs_real nestedW = 30.0;
	const tjs_real nestedH = 20.0;
	MotionSurfaceStackState nestedLeafState = MotionAdvanceSurfaceStackForFrame(demuxLayoutState,
		nestedLeafFrame, 2, nestedOriginX, nestedOriginY, nestedW, nestedH,
		demuxLayoutState.originX, demuxLayoutState.originY,
		demuxLayoutState.width, demuxLayoutState.height,
		TJS_W("m3/nested-leaf"), nullptr);

		MotionMat4 clipExpected = MotionMat4Multiply(
			MotionMat4Ortho(-root.originX, root.width - root.originX,
				root.height - root.originY, -root.originY,
				kMotionDefaultLimitZMax, -kMotionDefaultLimitZMax),
			MotionMat4ApplyIconLocal(MotionMat4BuildModelFromFrame(clipFrame),
				clipOriginX, clipOriginY, clipFrame.ox, clipFrame.oy, clipW, clipH));
	MotionMat4 demuxLayout = MotionBuildDemuxMatFromSurface(
		clipState.lastSurface, true,
		root.originX, root.originY, root.width, root.height);
	(void)demuxLayout;
	MotionMat4 layoutExpected = MotionMat4BuildModelFromFrame(layoutFrame);
		MotionMat4 letterExpected = MotionMat4Multiply(
			MotionMat4Ortho(-root.originX, root.width - root.originX,
				root.height - root.originY, -root.originY,
				kMotionDefaultLimitZMax, -kMotionDefaultLimitZMax),
			MotionMat4Multiply(layoutExpected,
				MotionMat4ApplyIconLocal(MotionMat4BuildModelFromFrame(letterFrame),
			letterOriginX, letterOriginY, letterFrame.ox, letterFrame.oy,
			letterW, letterH)));
	MotionMat4 directExpected = MotionMat4Multiply(
		MotionBuildDemuxMatFromSurface(letterState.lastSurface,
			false, 0.0, 0.0, 0.0, 0.0),
		MotionMat4ApplyIconLocal(MotionMat4BuildModelFromFrame(directFrame),
			directOriginX, directOriginY, directFrame.ox, directFrame.oy,
			directW, directH));
	MotionMat4 layoutChildExpected = MotionMat4Multiply(
		MotionBuildDemuxMatFromSurface(letterState.lastSurface,
			true, clipState.lastSurface.originX, clipState.lastSurface.originY,
			clipState.lastSurface.originW, clipState.lastSurface.originH),
		MotionMat4BuildModelFromFrame(demuxLayoutFrame));
		MotionMat4 nestedExpected = MotionMat4Multiply(
			MotionMat4Ortho(-clipState.lastSurface.originX,
				clipState.lastSurface.originW - clipState.lastSurface.originX,
				clipState.lastSurface.originH - clipState.lastSurface.originY,
				-clipState.lastSurface.originY,
				kMotionDefaultLimitZMax, -kMotionDefaultLimitZMax),
			MotionMat4Multiply(layoutChildExpected,
			MotionMat4ApplyIconLocal(MotionMat4BuildModelFromFrame(nestedLeafFrame),
				nestedOriginX, nestedOriginY, nestedLeafFrame.ox, nestedLeafFrame.oy,
				nestedW, nestedH)));

	tjs_real maxErr = 0.0;
	tjs_real errA = 0.0;
	tjs_real errB = 0.0;
	bool clipOk = clipState.valid &&
		clipState.surfaces.size() == 1 &&
		clipState.lastSurface.type == 2 &&
		MotionMat4AlmostEqual(clipState.lastSurface.matTrans,
			clipExpected, 0.001, errA);
	maxErr = std::max(maxErr, errA);
	bool layoutOk = layoutState.valid &&
		layoutState.surfaces.size() == 2 &&
		layoutState.surfaces[0].type == 2 &&
		layoutState.lastSurface.type == 3 &&
		MotionMat4AlmostEqual(layoutState.lastSurface.matTrans,
			layoutExpected, 0.001, errA);
	maxErr = std::max(maxErr, errA);
	bool letterOk = letterState.valid &&
		!letterState.unsupportedDemux &&
		letterState.surfaces.size() == 2 &&
		letterState.surfaces[0].type == 2 &&
		letterState.lastSurface.type == 2 &&
		std::abs(letterState.lastSurface.opa -
			(clipFrame.opacity * letterFrame.opacity)) <= 0.001 &&
		MotionMat4AlmostEqual(letterState.surfaces[0].matTrans,
			clipExpected, 0.001, errA) &&
		MotionMat4AlmostEqual(letterState.lastSurface.matTrans,
			letterExpected, 0.001, errB);
	maxErr = std::max(maxErr, errB);
	maxErr = std::max(maxErr, errA);
	bool directOk = directState.valid &&
		directState.surfaces.size() == 2 &&
		directState.surfaces[0].type == 2 &&
		directState.lastSurface.type == 2 &&
		std::abs(directState.lastSurface.opa -
			(clipFrame.opacity * letterFrame.opacity * directFrame.opacity)) <= 0.001 &&
		MotionMat4AlmostEqual(directState.lastSurface.matTrans,
			directExpected, 0.001, errA);
	maxErr = std::max(maxErr, errA);
	bool demuxLayoutOk = demuxLayoutState.valid &&
		demuxLayoutState.surfaces.size() == 2 &&
		demuxLayoutState.surfaces[0].type == 2 &&
		demuxLayoutState.lastSurface.type == 3 &&
		MotionMat4AlmostEqual(demuxLayoutState.lastSurface.matTrans,
			layoutChildExpected, 0.001, errA);
	maxErr = std::max(maxErr, errA);
	bool nestedLeafOk = nestedLeafState.valid &&
		nestedLeafState.surfaces.size() == 2 &&
		nestedLeafState.surfaces[0].type == 2 &&
		nestedLeafState.lastSurface.type == 2 &&
		std::abs(nestedLeafState.lastSurface.opa -
			(clipFrame.opacity * letterFrame.opacity * nestedLeafFrame.opacity)) <= 0.001 &&
		MotionMat4AlmostEqual(nestedLeafState.lastSurface.matTrans,
			nestedExpected, 0.001, errA);
	maxErr = std::max(maxErr, errA);

	diag.m3SampleSurfaceCount = (tjs_int)nestedLeafState.surfaces.size();
	diag.m3SampleMaxAbsError = maxErr;
	diag.m3SampleMat00 = nestedLeafState.lastSurface.matTrans.m[0];
	diag.m3SampleMat11 = nestedLeafState.lastSurface.matTrans.m[5];
	diag.m3SampleMat03 = nestedLeafState.lastSurface.matTrans.m[3];
	diag.m3SampleMat13 = nestedLeafState.lastSurface.matTrans.m[7];
	diag.m3SelfTestPassed =
		(clipOk && layoutOk && letterOk && directOk && demuxLayoutOk && nestedLeafOk) ? 1 : 0;
	return diag.m3SelfTestPassed == 1;
}

static bool MotionRunM4RendererUploadSelfTest(MotionRenderMethodSemanticsDiag &diag)
{
	diag.m4SelfTested = 1;

	MotionResource synthetic;
	synthetic.opened = true;
	synthetic.parsed = true;
	synthetic.storage = TJS_W("m4-renderer-upload-selftest");
	synthetic.width = 200;
	synthetic.height = 120;

	MotionRenderMethodItem item;
	item.type = 2;
	item.label = TJS_W("m4/leaf");
	item.srcKey = "src/m4/leaf";
	item.originX = 0.0;
	item.originY = 0.0;
	item.width = 20.0;
	item.height = 10.0;
	item.opacity = 0.75;
	item.layoutIndex = -1;
	item.m2ProgressSurface = true;
	item.affineValid = true;

	MotionRenderSurface parent;
	parent.type = 2;
	parent.originX = 0.0;
	parent.originY = 0.0;
	parent.originW = 1.0;
	parent.originH = 1.0;
	parent.opa = 1.0;
	parent.matTrans = MotionMat4Identity();

	MotionRenderSurface leaf;
	leaf.type = 2;
	leaf.originX = 0.0;
	leaf.originY = 0.0;
	leaf.originW = 20.0;
	leaf.originH = 10.0;
	leaf.opa = item.opacity;
	leaf.matTrans = MotionMat4Identity();
	leaf.matTrans.m[0] = 0.6;
	leaf.matTrans.m[5] = 0.4;
	leaf.matTrans.m[3] = -0.2;
	leaf.matTrans.m[7] = 0.1;
	item.surfaces.push_back(parent);
	item.surfaces.push_back(leaf);
	synthetic.renderMethodIR.push_back(item);

	std::vector<MotionRendererCommandEntryInfo> commands =
		MotionBuildRendererCommandEntriesFromDrawItems(&synthetic, 2.0);
	MotionStagingArtifact staging;
	MotionRendererUploadPrepSummary upload =
		MotionPrepareRendererUploadFromCommands(commands, &synthetic, 2.0, &staging);
	MotionShadowDrawDiagnostic shadow =
		MotionConsumeRendererStagingForShadowDraw(commands, staging, upload);

	tjs_real samplePx = 0.0;
	tjs_real samplePy = 0.0;
	tjs_real sampleNdcX = 0.0;
	tjs_real sampleNdcY = 0.0;
	tjs_int remaps = 0;
	bool evalOk = MotionEvalSurfaceChainVertex(item.surfaces, 0.0, 0.0,
		(tjs_real)synthetic.width, (tjs_real)synthetic.height,
		samplePx, samplePy, &sampleNdcX, &sampleNdcY, &remaps);
	tjs_real manualLeafX = -0.2;
	tjs_real manualLeafY = 0.1;
	tjs_real manualRemapX = 1.0 - (manualLeafY / 2.0 + 0.5);
	tjs_real manualRemapY = 0.5 + manualLeafX / 2.0;
	// The default control-point grid used by the shader is transposed relative
	// to the scalar u/v inputs, so the expected surface output is (remapY, remapX).
	tjs_real manualSurfaceX = manualRemapY;
	tjs_real manualSurfaceY = manualRemapX;
	tjs_real manualNdcX = manualSurfaceX;
	tjs_real manualNdcY = -manualSurfaceY;
	tjs_real manualPx = (manualNdcX * 0.5 + 0.5) * (tjs_real)synthetic.width;
	tjs_real manualPy = (manualNdcY * 0.5 + 0.5) * (tjs_real)synthetic.height;
	tjs_real maxErr = 0.0;
	maxErr = std::max(maxErr, std::abs(sampleNdcX - manualNdcX));
	maxErr = std::max(maxErr, std::abs(sampleNdcY - manualNdcY));
	maxErr = std::max(maxErr, std::abs(samplePx - manualPx));
	maxErr = std::max(maxErr, std::abs(samplePy - manualPy));
	diag.m4SampleMaxAbsError = std::max(diag.m4SampleMaxAbsError, maxErr);
	diag.m4SurfaceChainCommandCount = upload.m4SurfaceChainCommandCount;
	diag.m4MaxSurfaceChainLength = upload.m4MaxSurfaceChainLength;
	diag.m4RemappedVertexCount = upload.m4RemappedVertexCount;
	diag.m4ViewportTransformedVertexCount = upload.m4ViewportTransformedVertexCount;
	diag.m4SampleNdcX = upload.m4SampleNdcX;
	diag.m4SampleNdcY = upload.m4SampleNdcY;
	diag.m4SamplePixelX = upload.m4SamplePixelX;
	diag.m4SamplePixelY = upload.m4SamplePixelY;

	bool ok =
		commands.size() == 1 &&
		evalOk &&
		remaps == 1 &&
		maxErr <= 0.001 &&
		upload.prepared &&
		upload.stagingArtifactPopulated &&
		upload.perCommandOk &&
		upload.rangesOk &&
		upload.offsetsOk &&
		upload.m4SurfaceChainCommandCount == 1 &&
		upload.m4MaxSurfaceChainLength == 2 &&
		upload.m4ViewportTransformedVertexCount == 9 &&
		upload.m4RemappedVertexCount == 9 &&
		shadow.consumed &&
		shadow.indexInRange &&
		shadow.coveredCellCount == 4 &&
		shadow.coveredTriangleCount == 8;
	diag.m4SelfTestPassed = ok ? 1 : 0;
	return ok;
}

static bool MotionRunM5BezierSurfaceChainSelfTest(MotionRenderMethodSemanticsDiag &diag)
{
	diag.m5SelfTested = 1;

	MotionResource synthetic;
	synthetic.opened = true;
	synthetic.parsed = true;
	synthetic.storage = TJS_W("m5-bezier-surface-chain-selftest");
	synthetic.width = 320;
	synthetic.height = 240;

	tjs_real synthBp[32] = {
		0.0, 0.0, 0.2, 0.1, 0.6, -0.05, 1.0, 0.0,
		0.1, 0.25, 0.3, 0.3, 0.7, 0.3, 0.9, 0.25,
		-0.05, 0.6, 0.35, 0.7, 0.65, 0.65, 1.05, 0.6,
		0.0, 1.0, 0.25, 0.9, 0.75, 1.0, 1.0, 1.0
	};

	MotionRenderMethodItem item;
	item.type = 1;
	item.label = TJS_W("m5/type1-leaf");
	item.srcKey = "src/m5/type1-leaf";
	item.originX = 0.0;
	item.originY = 0.0;
	item.width = 40.0;
	item.height = 30.0;
	item.opacity = 1.0;
	item.affineValid = true;
	item.hasControlPoints = true;
	for (size_t k = 0; k < 32; k++) item.controlPts[k] = synthBp[k];

	MotionRenderSurface parent;
	parent.type = 2;
	parent.originX = 0.0;
	parent.originY = 0.0;
	parent.originW = 1.0;
	parent.originH = 1.0;
	parent.opa = 1.0;
	parent.matTrans = MotionMat4Identity();

	MotionRenderSurface leaf;
	leaf.type = 1;
	leaf.originX = 0.0;
	leaf.originY = 0.0;
	leaf.originW = 40.0;
	leaf.originH = 30.0;
	leaf.opa = 1.0;
	leaf.matTrans = MotionMat4Identity();
	leaf.matTrans.m[0] = 0.8;
	leaf.matTrans.m[5] = -0.6;
	leaf.matTrans.m[3] = -0.2;
	leaf.matTrans.m[7] = 0.3;
	for (size_t k = 0; k < 32; k++) leaf.controlPts[k] = synthBp[k];

	MotionRenderSurface affineLeaf = leaf;
	affineLeaf.type = 2;
	for (size_t k = 0; k < 32; k++) affineLeaf.controlPts[k] = kMotionDefaultControlPoints[k];

	item.surfaces.push_back(parent);
	item.surfaces.push_back(leaf);
	synthetic.renderMethodIR.push_back(item);

	std::vector<MotionRendererCommandEntryInfo> commands =
		MotionBuildRendererCommandEntriesFromDrawItems(&synthetic, 2.0);
	MotionStagingArtifact staging;
	MotionRendererUploadPrepSummary upload =
		MotionPrepareRendererUploadFromCommands(commands, &synthetic, 2.0, &staging);
	MotionShadowDrawDiagnostic shadow =
		MotionConsumeRendererStagingForShadowDraw(commands, staging, upload);

	tjs_real deformedPx = 0.0, deformedPy = 0.0;
	tjs_real affinePx = 0.0, affinePy = 0.0;
	tjs_real deformedNdcX = 0.0, deformedNdcY = 0.0;
	tjs_real affineNdcX = 0.0, affineNdcY = 0.0;
	tjs_int deformedRemaps = 0;
	std::vector<MotionRenderSurface> affineSurfaces;
	affineSurfaces.push_back(parent);
	affineSurfaces.push_back(affineLeaf);
	bool deformedOk = MotionEvalSurfaceChainVertex(item.surfaces, 0.5, 0.5,
		(tjs_real)synthetic.width, (tjs_real)synthetic.height,
		deformedPx, deformedPy, &deformedNdcX, &deformedNdcY, &deformedRemaps);
	bool affineOk = MotionEvalSurfaceChainVertex(affineSurfaces, 0.5, 0.5,
		(tjs_real)synthetic.width, (tjs_real)synthetic.height,
		affinePx, affinePy, &affineNdcX, &affineNdcY);
	tjs_real deviationDist = std::sqrt(
		(deformedPx - affinePx) * (deformedPx - affinePx) +
		(deformedPy - affinePy) * (deformedPy - affinePy));

	tjs_real stagingDist = 0.0;
	bool stagingDeformOk = false;
	if (staging.position.size() >= 10) {
		tjs_int centerVertIdx = 4;
		tjs_real sx = staging.position[(size_t)(centerVertIdx * 2)];
		tjs_real sy = staging.position[(size_t)(centerVertIdx * 2) + 1];
		stagingDist = std::sqrt((sx - affinePx) * (sx - affinePx) +
			(sy - affinePy) * (sy - affinePy));
		stagingDeformOk =
			std::abs(sx - deformedPx) <= 0.001 &&
			std::abs(sy - deformedPy) <= 0.001 &&
			stagingDist > 0.05;
	}

	diag.m5Type1SurfaceCount = 1;
	diag.m5ControlPointSurfaceConsumed =
		(deformedOk && affineOk && stagingDeformOk && deviationDist > 0.05) ? 1 : 0;
	diag.m5SampleAffinePixelX = affinePx;
	diag.m5SampleAffinePixelY = affinePy;
	diag.m5SampleDeformedPixelX = deformedPx;
	diag.m5SampleDeformedPixelY = deformedPy;
	diag.m5SampleDeviationDist = deviationDist;
	diag.m5SampleControlPt1X = synthBp[2];
	diag.m5SampleControlPt1Y = synthBp[3];

	bool ok =
		commands.size() == 1 &&
		upload.prepared &&
		upload.stagingArtifactPopulated &&
		upload.perCommandOk &&
		upload.rangesOk &&
		upload.offsetsOk &&
		shadow.consumed &&
		shadow.indexInRange &&
		shadow.coveredCellCount == 4 &&
		shadow.coveredTriangleCount == 8 &&
		deformedOk &&
		affineOk &&
		deformedRemaps == 1 &&
		deviationDist > 0.05 &&
		stagingDeformOk;
	diag.m5SelfTestPassed = ok ? 1 : 0;
	return ok;
}

static bool MotionRunMeshDeformationSelfTest(MotionMeshDeformationDiag &diag)
{
	diag.meshDeformSelfTested = 1;

	tjs_real synthBp[32] = {
		0.0, 0.0, 0.2, 0.1, 0.6, -0.05, 1.0, 0.0,
		0.1, 0.25, 0.3, 0.3, 0.7, 0.3, 0.9, 0.25,
		-0.05, 0.6, 0.35, 0.7, 0.65, 0.65, 1.05, 0.6,
		0.0, 1.0, 0.25, 0.9, 0.75, 1.0, 1.0, 1.0
	};

	bool hasInteriorDeviation = false;
	tjs_real interiorAffineX = 0.0, interiorAffineY = 0.0;
	tjs_real interiorDeformedX = 0.0, interiorDeformedY = 0.0;
	tjs_real interiorDist = 0.0;

	{
		MotionAffine2D identAffine = MotionBuildIconDrawAffine(
			MotionAffineIdentity(), 0.0, 0.0, 200.0, 100.0, 0.0, 0.0);
		tjs_real du, dv;
		MotionEvalBezierSurface(synthBp, 0.5, 0.5, du, dv);
		tjs_real affineX, affineY, deformX, deformY;
		MotionApplyIconDrawAffineToPoint(identAffine, 0.5, 0.5, affineX, affineY);
		MotionApplyIconDrawAffineToPoint(identAffine, du, dv, deformX, deformY);
		tjs_real dist = std::sqrt((deformX - affineX) * (deformX - affineX) +
			(deformY - affineY) * (deformY - affineY));
		if (dist > 0.1) {
			hasInteriorDeviation = true;
			interiorAffineX = affineX;
			interiorAffineY = affineY;
			interiorDeformedX = deformX;
			interiorDeformedY = deformY;
			interiorDist = dist;
		}
	}
	diag.meshDeformInteriorDeviated = hasInteriorDeviation ? 1 : 0;
	diag.sampleInteriorAffineX = interiorAffineX;
	diag.sampleInteriorAffineY = interiorAffineY;
	diag.sampleInteriorDeformedX = interiorDeformedX;
	diag.sampleInteriorDeformedY = interiorDeformedY;
	diag.sampleInteriorDeviationDist = interiorDist;
	diag.sampleBp0 = synthBp[0];
	diag.sampleBp1 = synthBp[1];
	diag.sampleBp30 = synthBp[30];
	diag.sampleBp31 = synthBp[31];

	diag.meshDeformConsumerSelfTested = 1;
	MotionResource synthetic;
	synthetic.opened = true;
	synthetic.parsed = true;
	synthetic.storage = TJS_W("mesh-deform-selftest");
	synthetic.width = 320;
	synthetic.height = 240;

	MotionLayoutInfo layout;
	layout.name = TJS_W("mesh/deform-selftest");
	layout.sourceKey = TJS_W("src/mesh-deform-selftest");
	layout.objectName = TJS_W("mesh_deform_selftest");
	layout.motionName = TJS_W("show");
	layout.hitObjectName = TJS_W("mesh_deform_selftest");
	layout.hitObjectPath = TJS_W("mesh_deform_selftest");
	layout.hitStateName = TJS_W("show");
	layout.hitStateClass = TJS_W("show");
	layout.hitStateGroup = TJS_W("active");
	layout.hitStatePath = TJS_W("mesh_deform_selftest/show");
	layout.hitPartName = TJS_W("front");
	layout.hitRole = TJS_W("control");
	layout.frontDrawCandidate = true;
	layout.drawCount = 1;
	layout.opacityMin = 1.0;
	layout.opacityMax = 1.0;
	layout.left = 30.0;
	layout.top = 20.0;
	layout.width = 200.0;
	layout.height = 100.0;
	layout.centerX = layout.left + layout.width * 0.5;
	layout.centerY = layout.top + layout.height * 0.5;
	layout.affineValid = true;
	layout.affine = MotionBuildIconDrawAffine(
		MotionAffineIdentity(), 0.0, 0.0, 200.0, 100.0, 0.0, 0.0);
	layout.hasControlPoints = true;
	for (size_t k = 0; k < 32; k++) layout.controlPts[k] = synthBp[k];
	synthetic.layouts.push_back(layout);

	std::vector<MotionRendererCommandEntryInfo> commands =
		MotionBuildRendererCommandEntriesFromDrawItems(&synthetic, 2.0);
	MotionStagingArtifact staging;
	MotionRendererUploadPrepSummary upload =
		MotionPrepareRendererUploadFromCommands(commands, &synthetic, 2.0, &staging);
	MotionShadowDrawDiagnostic shadow =
		MotionConsumeRendererStagingForShadowDraw(commands, staging, upload);

	diag.meshDeformConsumerCommandCount = (tjs_int)commands.size();
	diag.meshDeformConsumerVertexCount = upload.vertexBufferCount;
	diag.meshDeformConsumerIndexCount = upload.indexBufferCount;
	diag.meshDeformConsumerCellCount = upload.cellBufferCount;
	diag.meshDeformConsumerTriangleCount = shadow.coveredTriangleCount;
	diag.meshDeformConsumerUploadPrepared = upload.prepared ? 1 : 0;
	diag.meshDeformConsumerStagingPopulated = upload.stagingArtifactPopulated ? 1 : 0;
	diag.meshDeformConsumerShadowConsumed = shadow.consumed ? 1 : 0;

	bool consumerDeformOk = false;
	tjs_real interiorStagingAffineX = 0.0, interiorStagingAffineY = 0.0;
	tjs_real interiorStagingDeformX = 0.0, interiorStagingDeformY = 0.0;
	if (staging.position.size() >= 18) {
		tjs_int centerVertIdx = 4;
		if (centerVertIdx * 2 + 1 < (tjs_int)staging.position.size()) {
			interiorStagingDeformX = staging.position[(size_t)(centerVertIdx * 2)];
			interiorStagingDeformY = staging.position[(size_t)(centerVertIdx * 2) + 1];
			MotionApplyIconDrawAffineToPoint(layout.affine,
				0.5, 0.5, interiorStagingAffineX, interiorStagingAffineY);
			tjs_real sDist = std::sqrt(
				(interiorStagingDeformX - interiorStagingAffineX) * (interiorStagingDeformX - interiorStagingAffineX) +
				(interiorStagingDeformY - interiorStagingAffineY) * (interiorStagingDeformY - interiorStagingAffineY));
			consumerDeformOk = sDist > 0.05;
		}
	}
	bool consumerOk =
		commands.size() == 1 &&
		upload.prepared &&
		upload.stagingArtifactPopulated &&
		upload.perCommandOk &&
		upload.rangesOk &&
		upload.offsetsOk &&
		shadow.consumed &&
		shadow.indexInRange &&
		shadow.coveredCellCount == 4 &&
		shadow.coveredTriangleCount == 8 &&
		hasInteriorDeviation &&
		consumerDeformOk;
	diag.meshDeformConsumerSelfTestPassed = consumerOk ? 1 : 0;
	if (!consumerOk) {
		ttstr uploadKind = upload.mismatchKind.IsEmpty() ?
			ttstr(TJS_W("empty")) : upload.mismatchKind;
		ttstr details = TJS_W("commands=") + ttstr((tjs_int)commands.size()) +
			TJS_W(" upload=") + uploadKind +
			TJS_W(" interiorDeviated=") + ttstr(hasInteriorDeviation ? 1 : 0) +
			TJS_W(" consumerDeformOk=") + ttstr(consumerDeformOk ? 1 : 0);
		diag.consumerSelfTestMismatchKind =
			(uploadKind == TJS_W("none")) ?
			std::string("consumer-deform-not-ok") : uploadKind.AsNarrowStdString();
		diag.consumerSelfTestDetails = details.AsNarrowStdString();
	}

	diag.meshDeformSelfTestPassed =
		(hasInteriorDeviation && consumerOk) ? 1 : 0;
	return diag.meshDeformSelfTestPassed == 1;
}

static MotionOffscreenDrawDiagnostic MotionRasterizeRendererOffscreenDrawDiagnostic(
	const std::vector<MotionRendererCommandEntryInfo> &rendererCommandEntries,
	const MotionStagingArtifact &stagingArtifact,
	const MotionShadowDrawDiagnostic &shadowDiag)
{
	MotionOffscreenDrawDiagnostic diag;
	diag.ran = true;
	diag.source = TJS_W("rendererCommandEntries+stagingArtifact");
	diag.consumedCommandCount = (tjs_int)rendererCommandEntries.size();

	if (stagingArtifact.position.empty() || stagingArtifact.index.empty() ||
		rendererCommandEntries.empty() || !shadowDiag.consumed) {
		diag.consumed = false;
		diag.mismatchKind = TJS_W("empty-staging");
		diag.mismatchIndex = 0;
		return diag;
	}

	const tjs_int TILE_W = 20;
	const tjs_int TILE_H = 20;
	const tjs_int DEST_W = 1280;
	const tjs_int DEST_H = 720;
	tjs_int gridW = (DEST_W + TILE_W - 1) / TILE_W;
	tjs_int gridH = (DEST_H + TILE_H - 1) / TILE_H;
	tjs_int tileCount = gridW * gridH;

	std::vector<tjs_real> tileAlpha(tileCount, 0.0);
	std::vector<tjs_int> tileCoverage(tileCount, 0);

	diag.width = DEST_W;
	diag.height = DEST_H;
	diag.tileGridW = gridW;
	diag.tileGridH = gridH;

	tjs_int maxVertIdx = (tjs_int)(stagingArtifact.position.size() / 2) - 1;
	bool boundsInit = false;
	tjs_uint64 coverageHash = (tjs_uint64)1469598103934665701ULL;
	tjs_uint64 alphaHashAccum = (tjs_uint64)1469598103934665707ULL;
	tjs_int rasterizedCellCount = 0;
	tjs_int rasterizedTriangleCount = 0;

	tjs_int firstTileX = -1;
	tjs_int firstTileY = -1;
	tjs_int lastTileX = -1;
	tjs_int lastTileY = -1;

	auto setMismatch = [&](tjs_int index, const ttstr &kind, const ttstr &details)
	{
		if (diag.mismatchIndex >= 0) return;
		diag.mismatchIndex = index;
		diag.mismatchKind = kind;
		diag.mismatchDetails = details;
	};

	for (tjs_int ci = 0; ci < (tjs_int)rendererCommandEntries.size(); ci++) {
		const MotionRendererCommandEntryInfo &cmd = rendererCommandEntries[(size_t)ci];
		if (cmd.indexCount <= 0 || cmd.indexOffset < 0) {
			setMismatch(ci, TJS_W("invalid-command-slice"),
				ttstr(TJS_W("cmd ")) + ttstr(ci) +
				TJS_W(" indexOffset:") + ttstr(cmd.indexOffset) +
				TJS_W(" indexCount:") + ttstr(cmd.indexCount));
			continue;
		}
		if ((cmd.indexCount % 6) != 0) {
			setMismatch(ci, TJS_W("invalid-command-index-count"),
				ttstr(TJS_W("cmd ")) + ttstr(ci) +
				TJS_W(" indexCount:") + ttstr(cmd.indexCount));
		}

		tjs_int idxEnd = cmd.indexOffset + cmd.indexCount;
		if (idxEnd > (tjs_int)stagingArtifact.index.size()) {
			setMismatch(ci, TJS_W("idx-out-of-bounds"),
				ttstr(TJS_W("cmd ")) + ttstr(ci) +
				TJS_W(" indexRange:") + ttstr(cmd.indexOffset) +
				TJS_W("-") + ttstr(idxEnd - 1) +
				TJS_W(" bufSize:") + ttstr((tjs_int)stagingArtifact.index.size()));
		}

		tjs_int quads = cmd.indexCount / 6;
		if (quads != cmd.cellCount) {
			setMismatch(ci, TJS_W("command-cell-count-mismatch"),
				ttstr(TJS_W("cmd ")) + ttstr(ci) +
				TJS_W(" quads:") + ttstr(quads) +
				TJS_W(" cellCount:") + ttstr(cmd.cellCount));
		}
		for (tjs_int qi = 0; qi < quads; qi++) {
			tjs_int i0 = cmd.indexOffset + qi * 6;
			if (i0 + 5 >= (tjs_int)stagingArtifact.index.size()) {
				setMismatch(ci, TJS_W("idx-oob-read"),
					ttstr(TJS_W("cmd ")) + ttstr(ci) +
					TJS_W(" quad:") + ttstr(qi) +
					TJS_W(" indexRange:") + ttstr(i0) +
					TJS_W("-") + ttstr(i0 + 5) +
					TJS_W(" bufSize:") + ttstr((tjs_int)stagingArtifact.index.size()));
				break;
			}
			tjs_int v0 = stagingArtifact.index[(size_t)i0];
			tjs_int v1 = stagingArtifact.index[(size_t)i0 + 1];
			tjs_int v2 = stagingArtifact.index[(size_t)i0 + 2];
			tjs_int v4 = stagingArtifact.index[(size_t)i0 + 4];
			tjs_int v5 = stagingArtifact.index[(size_t)i0 + 5];

			if (v0 < 0 || v1 < 0 || v2 < 0 || v4 < 0 || v5 < 0 ||
				v0 > maxVertIdx || v1 > maxVertIdx || v2 > maxVertIdx ||
				v4 > maxVertIdx || v5 > maxVertIdx) {
				setMismatch(ci, TJS_W("idx-out-of-vertex-range"),
					ttstr(TJS_W("cmd ")) + ttstr(ci) +
					TJS_W(" quad:") + ttstr(qi) +
					TJS_W(" idx:") + ttstr(v0) + TJS_W(",") + ttstr(v1) +
					TJS_W(",") + ttstr(v2) + TJS_W(",") + ttstr(v4) +
				TJS_W(",") + ttstr(v5) +
				TJS_W(" maxVert:") + ttstr(maxVertIdx));
				continue;
			}
			if ((size_t)v0 >= stagingArtifact.alpha.size() ||
				(size_t)v1 >= stagingArtifact.alpha.size() ||
				(size_t)v2 >= stagingArtifact.alpha.size() ||
				(size_t)v4 >= stagingArtifact.alpha.size() ||
				(size_t)v5 >= stagingArtifact.alpha.size()) {
				setMismatch(ci, TJS_W("alpha-out-of-range"),
					ttstr(TJS_W("cmd ")) + ttstr(ci) +
					TJS_W(" quad:") + ttstr(qi) +
					TJS_W(" idx:") + ttstr(v0) + TJS_W(",") + ttstr(v1) +
					TJS_W(",") + ttstr(v2) + TJS_W(",") + ttstr(v4) +
					TJS_W(",") + ttstr(v5) +
					TJS_W(" alphaSize:") + ttstr((tjs_int)stagingArtifact.alpha.size()));
				continue;
			}

			float px0 = stagingArtifact.position[(size_t)v0 * 2];
			float py0 = stagingArtifact.position[(size_t)v0 * 2 + 1];
			float px1 = stagingArtifact.position[(size_t)v1 * 2];
			float py1 = stagingArtifact.position[(size_t)v1 * 2 + 1];
			float px2 = stagingArtifact.position[(size_t)v2 * 2];
			float py2 = stagingArtifact.position[(size_t)v2 * 2 + 1];
			float px4 = stagingArtifact.position[(size_t)v4 * 2];
			float py4 = stagingArtifact.position[(size_t)v4 * 2 + 1];
			float px5 = stagingArtifact.position[(size_t)v5 * 2];
			float py5 = stagingArtifact.position[(size_t)v5 * 2 + 1];

			tjs_int a0 = stagingArtifact.alpha[(size_t)v0];
			tjs_int a1 = stagingArtifact.alpha[(size_t)v1];
			tjs_int a2 = stagingArtifact.alpha[(size_t)v2];
			tjs_int a4 = stagingArtifact.alpha[(size_t)v4];
			tjs_int a5 = stagingArtifact.alpha[(size_t)v5];

			tjs_int triAvgAlpha[2];
			triAvgAlpha[0] = (tjs_int)std::lround((double)(a0 + a1 + a2) / 3.0);
			triAvgAlpha[1] = (tjs_int)std::lround((double)(a2 + a4 + a5) / 3.0);

			float tri1X[3] = { px0, px1, px2 };
			float tri1Y[3] = { py0, py1, py2 };
			float tri2X[3] = { px2, px4, px5 };
			float tri2Y[3] = { py2, py4, py5 };
			for (int t = 0; t < 2; t++) {
				const float *tx = (t == 0) ? tri1X : tri2X;
				const float *ty = (t == 0) ? tri1Y : tri2Y;

				float tMinX = std::min(std::min(tx[0], tx[1]), tx[2]);
				float tMaxX = std::max(std::max(tx[0], tx[1]), tx[2]);
				float tMinY = std::min(std::min(ty[0], ty[1]), ty[2]);
				float tMaxY = std::max(std::max(ty[0], ty[1]), ty[2]);

				if (!boundsInit) {
					boundsInit = true;
					diag.boundsMinX = tMinX;
					diag.boundsMinY = tMinY;
					diag.boundsMaxX = tMaxX;
					diag.boundsMaxY = tMaxY;
				} else {
					if (tMinX < diag.boundsMinX) diag.boundsMinX = tMinX;
					if (tMinY < diag.boundsMinY) diag.boundsMinY = tMinY;
					if (tMaxX > diag.boundsMaxX) diag.boundsMaxX = tMaxX;
					if (tMaxY > diag.boundsMaxY) diag.boundsMaxY = tMaxY;
				}

				tjs_int tx0 = std::max<tjs_int>(0, std::min<tjs_int>(gridW - 1, (tjs_int)(tMinX / (tjs_real)TILE_W)));
				tjs_int ty0 = std::max<tjs_int>(0, std::min<tjs_int>(gridH - 1, (tjs_int)(tMinY / (tjs_real)TILE_H)));
				tjs_int tx1 = std::max<tjs_int>(0, std::min<tjs_int>(gridW - 1, (tjs_int)(tMaxX / (tjs_real)TILE_W)));
				tjs_int ty1 = std::max<tjs_int>(0, std::min<tjs_int>(gridH - 1, (tjs_int)(tMaxY / (tjs_real)TILE_H)));

				tjs_real triAlpha = (tjs_real)triAvgAlpha[t] / 255.0;
				for (tjs_int ty = ty0; ty <= ty1; ty++) {
					for (tjs_int tx = tx0; tx <= tx1; tx++) {
						tjs_int tileId = ty * gridW + tx;
						tileAlpha[tileId] += triAlpha;
						tileCoverage[tileId] = 1;
					}
				}
				rasterizedTriangleCount++;
			}
			rasterizedCellCount++;
		}
	}

	diag.consumedCellCount = rasterizedCellCount;
	diag.consumedTriangleCount = rasterizedTriangleCount;
	tjs_int coveredCount = 0;
	tjs_int alphaNonZeroCount = 0;
	tjs_real alphaSum = 0.0;
	for (tjs_int i = 0; i < tileCount; i++) {
		if (tileCoverage[i] > 0) {
			coveredCount++;
			if (tileAlpha[i] > 0.0) alphaNonZeroCount++;
			alphaSum += tileAlpha[i];
			tjs_int tx = i % gridW;
			tjs_int ty = i / gridW;
			if (firstTileX < 0) { firstTileX = tx; firstTileY = ty; }
			lastTileX = tx;
			lastTileY = ty;
		}
		MotionHashPlanDouble(coverageHash, (tjs_real)tileCoverage[i]);
		MotionHashPlanDouble(alphaHashAccum, tileAlpha[i]);
	}

	diag.coveredTileCount = coveredCount;
	diag.alphaNonZeroTileCount = alphaNonZeroCount;
	diag.alphaAccum = alphaSum;
	diag.coverageHash = coverageHash;
	diag.alphaHash = alphaHashAccum;
	diag.compositeHash = (coverageHash * (tjs_uint64)1099511628211ULL) ^
		(alphaHashAccum * (tjs_uint64)1099511628217ULL);

	if (firstTileX >= 0) {
		tjs_int firstTileId = firstTileY * gridW + firstTileX;
		diag.firstOccupiedTileSample = ttstr(TJS_W("tile(")) +
			ttstr(firstTileX) + TJS_W(",") + ttstr(firstTileY) +
			TJS_W(") cov=") + ttstr(tileCoverage[firstTileId]) +
			TJS_W(" alpha=") + ttstr((tjs_int)std::lround(tileAlpha[firstTileId] * 1000.0));
		diag.firstAlphaTileSample = ttstr(TJS_W("tile(")) +
			ttstr(firstTileX) + TJS_W(",") + ttstr(firstTileY) +
			TJS_W(") alpha=") + ttstr((tjs_int)std::lround(tileAlpha[firstTileId] * 1000.0));
	}
	if (lastTileX >= 0) {
		tjs_int lastTileId = lastTileY * gridW + lastTileX;
		diag.lastOccupiedTileSample = ttstr(TJS_W("tile(")) +
			ttstr(lastTileX) + TJS_W(",") + ttstr(lastTileY) +
			TJS_W(") cov=") + ttstr(tileCoverage[lastTileId]) +
			TJS_W(" alpha=") + ttstr((tjs_int)std::lround(tileAlpha[lastTileId] * 1000.0));
	}

	diag.consumed = boundsInit && coveredCount > 0 && rasterizedTriangleCount > 0;
	if (diag.mismatchIndex < 0 &&
		diag.consumedCellCount != shadowDiag.consumedCellCount) {
		setMismatch(0, TJS_W("cell-count-mismatch"),
			ttstr(TJS_W("offscreen:")) + ttstr(diag.consumedCellCount) +
			TJS_W(" shadow:") + ttstr(shadowDiag.consumedCellCount));
	}
	if (diag.mismatchIndex < 0 &&
		diag.consumedTriangleCount != shadowDiag.coveredTriangleCount) {
		setMismatch(0, TJS_W("triangle-count-mismatch"),
			ttstr(TJS_W("offscreen:")) + ttstr(diag.consumedTriangleCount) +
			TJS_W(" shadow:") + ttstr(shadowDiag.coveredTriangleCount));
	}
	if (!diag.consumed && diag.mismatchIndex < 0) {
		diag.mismatchKind = TJS_W("no-coverage");
		diag.mismatchIndex = 0;
	} else if (diag.mismatchIndex < 0) {
		diag.mismatchKind = TJS_W("none");
		diag.mismatchIndex = -1;
	}

	return diag;
}

static MotionDiagnosticRenderBridge MotionBuildDiagnosticRenderBridge(
	const std::vector<MotionRendererCommandEntryInfo> &rendererCommandEntries,
	const MotionStagingArtifact &stagingArtifact,
	const MotionOffscreenDrawDiagnostic &offscreenDiag,
	const MotionShadowDrawDiagnostic &shadowDiag,
	bool hadTarget,
	bool usedFallback)
{
	MotionDiagnosticRenderBridge bridge;
	bridge.built = true;
	bridge.source = TJS_W("rendererCommandEntries+stagingArtifact+offscreenDraw");
	bridge.targetWidth = offscreenDiag.width;
	bridge.targetHeight = offscreenDiag.height;
	bridge.fallbackStillUsed = usedFallback;

	bridge.compatibleWithLayerTarget = hadTarget;
	bridge.compatibleWithLayerTargetDetail = hadTarget
		? TJS_W("draw-has-layer-target-")
			+ ttstr(bridge.targetWidth) + TJS_W("x") + ttstr(bridge.targetHeight)
		: TJS_W("no-layer-target");
	bridge.bridgeWouldDraw = hadTarget && offscreenDiag.consumed &&
		!rendererCommandEntries.empty();

	bridge.commandCount = (tjs_int)rendererCommandEntries.size();
	bridge.cellCount = offscreenDiag.consumedCellCount;
	bridge.triangleCount = offscreenDiag.consumedTriangleCount;
	bridge.coveredTileCount = offscreenDiag.coveredTileCount;
	bridge.alphaNonZeroTileCount = offscreenDiag.alphaNonZeroTileCount;
	bridge.alphaAccum = offscreenDiag.alphaAccum;
	bridge.coverageHash = offscreenDiag.coverageHash;
	bridge.alphaHash = offscreenDiag.alphaHash;
	bridge.compositeHash = offscreenDiag.compositeHash;
	bridge.boundsMinX = offscreenDiag.boundsMinX;
	bridge.boundsMinY = offscreenDiag.boundsMinY;
	bridge.boundsMaxX = offscreenDiag.boundsMaxX;
	bridge.boundsMaxY = offscreenDiag.boundsMaxY;
	bridge.tileGridW = offscreenDiag.tileGridW;
	bridge.tileGridH = offscreenDiag.tileGridH;
	bridge.firstTileSample = offscreenDiag.firstOccupiedTileSample;
	bridge.lastTileSample = offscreenDiag.lastOccupiedTileSample;
	bridge.firstAlphaTileSample = offscreenDiag.firstAlphaTileSample;

	bridge.blendModeSummary = TJS_W("identity-blend");
	bridge.alphaModeSummary = TJS_W("vertex-alpha+distance-per-draw");
	bridge.pixelCoverageSummary = ttstr(TJS_W("tiles:"))
		+ ttstr(bridge.coveredTileCount) + TJS_W("/")
		+ ttstr(bridge.tileGridW * bridge.tileGridH)
		+ TJS_W(" alpha-tiles:") + ttstr(bridge.alphaNonZeroTileCount)
		+ TJS_W(" accum:") + ttstr((tjs_int)std::lround(bridge.alphaAccum * 100.0));

	bridge.drawTargetCompatNote = bridge.bridgeWouldDraw
		? (usedFallback ?
			TJS_W("bridge-would-draw-coverage-nonzero-fallback-remains") :
			TJS_W("bridge-would-draw-coverage-nonzero"))
		: TJS_W("bridge-cannot-draw-no-target-or-no-coverage");

	if (offscreenDiag.mismatchKind != TJS_W("none")) {
		bridge.mismatchKind = offscreenDiag.mismatchKind;
		bridge.mismatchIndex = offscreenDiag.mismatchIndex;
		bridge.mismatchDetails = offscreenDiag.mismatchDetails;
	} else if (!hadTarget) {
		bridge.mismatchKind = TJS_W("no-layer-target");
		bridge.mismatchIndex = 0;
		bridge.mismatchDetails = TJS_W("draw() missing layer target");
	} else if (!offscreenDiag.consumed) {
		bridge.mismatchKind = TJS_W("offscreen-not-consumed");
		bridge.mismatchIndex = 0;
		bridge.mismatchDetails = TJS_W("offscreen draw diagnostic reports not consumed");
	} else {
		bridge.mismatchKind = TJS_W("none");
		bridge.mismatchIndex = -1;
	}

	return bridge;
}

static bool IsLogoPath(const ttstr &path)
{
	std::string p = path.AsNarrowStdString();
	std::transform(p.begin(), p.end(), p.begin(),
		[](unsigned char c) { return (char)std::tolower(c); });
	return p.find("yuzulogo") != std::string::npos ||
		p.find("m2logo") != std::string::npos;
}

static void MotionExportVisibleRenderBridgeToJs(
	const MotionVisibleRenderBridge &bridge, const ttstr &path)
{
#ifdef __EMSCRIPTEN__
	// `draw()` may be invoked from Module.evalTJS while the runtime is already
	// inside a JS->WASM call.  A nested emscripten_run_script() here can abort
	// the whole diagnostic page on pthread builds, so visible bridge evidence is
	// exported through getRendererFrontState() instead.
	(void)bridge;
	(void)path;
#else
	(void)bridge;
	(void)path;
#endif
}

static bool MotionGetLayerWriteBuffer(iTJSDispatch2 *target,
	tTJSNI_BaseLayer *layer, tjs_int height,
	unsigned char *&row0, tjs_int &rowStride, tjs_int &pitchAbs)
{
	row0 = layer ? (unsigned char *)layer->GetMainImagePixelBufferForWrite() : nullptr;
	tjs_int pitch = layer ? layer->GetMainImagePixelBufferPitch() : 0;
	if ((!row0 || pitch == 0) && target) {
		tTJSVariant val;
		if (TJS_SUCCEEDED(target->PropGet(0,
			TJS_W("mainImageBufferPitch"), nullptr, &val, target))) {
			pitch = (tjs_int)val;
		}
		if (TJS_SUCCEEDED(target->PropGet(0,
			TJS_W("mainImageBufferForWrite"), nullptr, &val, target))) {
			row0 = reinterpret_cast<unsigned char*>((tjs_intptr_t)(tjs_int64)val);
		}
	}
	if (!row0 || pitch == 0 || height <= 0) return false;
	pitchAbs = pitch > 0 ? pitch : -pitch;
	rowStride = pitch > 0 ? -pitch : pitch;
	return pitchAbs > 0;
}

static tTJSNI_BaseLayer *MotionGetNativeLayer(iTJSDispatch2 *target)
{
	if (!target) return nullptr;
	tTJSNI_BaseLayer *layer = nullptr;
	if (TJS_FAILED(target->NativeInstanceSupport(
		TJS_NIS_GETINSTANCE, tTJSNC_Layer::ClassID,
		(iTJSNativeInstance**)&layer))) {
		return nullptr;
	}
	return layer;
}

static bool MotionIsFiniteReal(tjs_real value)
{
	return std::isfinite((double)value);
}

static bool MotionIsFiniteTriangle(const tjs_real *xs, const tjs_real *ys)
{
	for (int i = 0; i < 3; i++) {
		if (!MotionIsFiniteReal(xs[i]) || !MotionIsFiniteReal(ys[i])) return false;
	}
	return true;
}

static void MotionHashPlanInt(tjs_uint64 &hash, tjs_int value);

static const TVPStaticMotionBridgeItem *MotionFindSnapshotItemForSourceKey(
	const TVPStaticMotionBridgeSnapshot *snapshot,
	const std::string &key)
{
	if (!snapshot) return nullptr;
	if (key.empty()) return nullptr;
	std::vector<std::string> candidates;
	auto addCandidate = [&](const std::string &value) {
		if (value.empty()) return;
		if (std::find(candidates.begin(), candidates.end(), value) == candidates.end()) {
			candidates.push_back(value);
		}
	};
	addCandidate(key);
	if (MotionStartsWith(key, "src/")) {
		std::string stripped = key.substr(4);
		addCandidate(stripped);
		addCandidate(std::string("source/") + stripped);
		std::vector<std::string> parts = MotionSplitPath(key);
		if (parts.size() >= 3) {
			std::string source = parts[1];
			std::string icon = parts[2];
			for (size_t i = 3; i < parts.size(); i++) icon += "/" + parts[i];
			addCandidate(source + "/icon/" + icon);
			addCandidate("source/" + source + "/icon/" + icon);
		}
	} else if (MotionStartsWith(key, "source/")) {
		std::string stripped = key.substr(7);
		addCandidate(stripped);
		addCandidate(std::string("src/") + stripped);
	}
	for (size_t ci = 0; ci < candidates.size(); ci++) {
		for (size_t i = 0; i < snapshot->items.size(); i++) {
			if (snapshot->items[i].ref == candidates[ci]) return &snapshot->items[i];
		}
	}
	for (size_t ci = 0; ci < candidates.size(); ci++) {
		const std::string &candidate = candidates[ci];
		size_t slash = candidate.find_last_of('/');
		if (slash == std::string::npos || slash + 1 >= candidate.size()) continue;
		std::string leaf = candidate.substr(slash + 1);
		const TVPStaticMotionBridgeItem *match = nullptr;
		for (size_t i = 0; i < snapshot->items.size(); i++) {
			const std::string &ref = snapshot->items[i].ref;
			size_t refSlash = ref.find_last_of('/');
			std::string refLeaf =
				refSlash == std::string::npos ? ref : ref.substr(refSlash + 1);
			if (refLeaf == leaf) {
				if (match) {
					match = nullptr;
					break;
				}
				match = &snapshot->items[i];
			}
		}
		if (match) return match;
	}
	return nullptr;
}

static const TVPStaticMotionBridgeItem *MotionFindSnapshotItemForLayout(
	const TVPStaticMotionBridgeSnapshot *snapshot,
	const MotionLayoutInfo *layout)
{
	if (!layout) return nullptr;
	return MotionFindSnapshotItemForSourceKey(
		snapshot, layout->sourceKey.AsNarrowStdString());
}

static void MotionAccumulateTextureColorStats(
	const TVPStaticMotionBridgeItem &item,
	tjs_int &totalPixelCount,
	tjs_int &alphaNonZeroPixelCount,
	tjs_int &redPixelCount,
	tjs_int &whitePixelCount,
	tjs_int &blackPixelCount,
	tjs_uint64 &colorHash)
{
	if (item.width <= 0 || item.height <= 0) return;
	size_t expected = (size_t)item.width * (size_t)item.height * 4u;
	if (item.pixelsBGRA.size() < expected) return;
	for (tjs_int y = 0; y < item.height; y++) {
		for (tjs_int x = 0; x < item.width; x++) {
			size_t off = ((size_t)y * (size_t)item.width + (size_t)x) * 4u;
			tjs_int b = item.pixelsBGRA[off + 0];
			tjs_int g = item.pixelsBGRA[off + 1];
			tjs_int r = item.pixelsBGRA[off + 2];
			tjs_int a = item.pixelsBGRA[off + 3];
			totalPixelCount++;
			if (a > 0) alphaNonZeroPixelCount++;
			if (r >= 140 && g <= 110 && b <= 110 && a >= 160) redPixelCount++;
			if (r >= 245 && g >= 245 && b >= 245 && a >= 245) whitePixelCount++;
			if (r <= 24 && g <= 24 && b <= 24 && a >= 160) blackPixelCount++;
			MotionHashPlanInt(colorHash, r);
			MotionHashPlanInt(colorHash, g);
			MotionHashPlanInt(colorHash, b);
			MotionHashPlanInt(colorHash, a);
		}
	}
}

static MotionVisibleRenderBridge MotionBuildVisibleRenderBridge(
	iTJSDispatch2 *target,
	const std::vector<MotionRendererCommandEntryInfo> &rendererCommandEntries,
	const MotionStagingArtifact &stagingArtifact,
	const MotionResource *resource,
	const MotionOffscreenDrawDiagnostic &offscreenDiag,
	const ttstr &path,
	bool usedFallback)
{
	MotionVisibleRenderBridge bridge;
	bridge.built = true;
	bridge.fallbackStillUsed = usedFallback;
	bridge.visibleBridgePath = path;

	bridge.source = TJS_W("runtime-command-staging+triangle-raster");
	bridge.commandCount = (tjs_int)rendererCommandEntries.size();

	bool doRaster = true;
	bool rendererDiagnostics = MotionRendererDiagnosticsWanted();

	if (!target) {
		bridge.mismatchKind = TJS_W("no-target");
		doRaster = false;
	} else if (stagingArtifact.position.empty() || stagingArtifact.index.empty() ||
		stagingArtifact.alpha.empty() || rendererCommandEntries.empty()) {
		bridge.mismatchKind = TJS_W("empty-staging");
		doRaster = false;
	} else {
		tTJSNI_BaseLayer *layer = nullptr;
		if (TJS_FAILED(target->NativeInstanceSupport(
			TJS_NIS_GETINSTANCE, tTJSNC_Layer::ClassID,
			(iTJSNativeInstance**)&layer)) || !layer) {
			bridge.mismatchKind = TJS_W("no-layer-native");
			doRaster = false;
		} else {
			// ... rasterization code follows
		}
	}

	if (!doRaster) {
		MotionExportVisibleRenderBridgeToJs(bridge, path);
		return bridge;
	}

	tTJSNI_BaseLayer *layer = nullptr;
	TJS_FAILED(target->NativeInstanceSupport(
		TJS_NIS_GETINSTANCE, tTJSNC_Layer::ClassID,
		(iTJSNativeInstance**)&layer));
	if (!layer) {
		bridge.mismatchKind = TJS_W("no-layer-native");
		MotionExportVisibleRenderBridgeToJs(bridge, path);
		return bridge;
	}

	tjs_int lw = (tjs_int)layer->GetWidth();
	tjs_int lh = (tjs_int)layer->GetHeight();
	if (lw <= 0 || lh <= 0) {
		bridge.mismatchKind = TJS_W("invalid-layer-size");
		MotionExportVisibleRenderBridgeToJs(bridge, path);
		return bridge;
	}
	bridge.targetWidth = lw;
	bridge.targetHeight = lh;

	unsigned char *buf = nullptr;
	tjs_int rowStride = 0;
	tjs_int pitch = 0;
	if (!MotionGetLayerWriteBuffer(target, layer, lh, buf, rowStride, pitch)) {
		bridge.mismatchKind = TJS_W("no-pixel-buffer");
		MotionExportVisibleRenderBridgeToJs(bridge, path);
		return bridge;
	}
	if (pitch < lw * 4 || pitch > 65536 || lh > 8192 || lw > 8192) {
		bridge.mismatchKind = TJS_W("invalid-layer-buffer");
		bridge.mismatchDetails =
			ttstr(TJS_W("pitch:")) + ttstr(pitch) +
			TJS_W(" size:") + ttstr(lw) + TJS_W("x") + ttstr(lh);
		MotionExportVisibleRenderBridgeToJs(bridge, path);
		return bridge;
	}
	auto layerRow = [&](tjs_int y) -> unsigned char* {
		if (!buf || y < 0 || y >= lh) return nullptr;
		return buf + (ptrdiff_t)y * (ptrdiff_t)rowStride;
	};
	auto layerConstRow = [&](tjs_int y) -> const unsigned char* {
		return layerRow(y);
	};
	bool fillWhiteBackground = false;
	if (resource) {
		fillWhiteBackground = IsLogoPath(path) &&
			(resource->motion.AsLowerCase().AsNarrowStdString().find("white") != std::string::npos);
	}

	const TVPStaticMotionBridgeSnapshot *staticSnapshot =
		MotionResolveStaticBridgeSnapshot(path, resource);
	bool useStaticDrawReplay = false;
	tjs_real staticReplayScaleX =
		(staticSnapshot && staticSnapshot->canvasWidth > 0) ?
		((tjs_real)lw / (tjs_real)staticSnapshot->canvasWidth) : 1.0;
	tjs_real staticReplayScaleY =
		(staticSnapshot && staticSnapshot->canvasHeight > 0) ?
		((tjs_real)lh / (tjs_real)staticSnapshot->canvasHeight) : 1.0;
	if (staticSnapshot) {
		bridge.staticSnapshotCanvasWidth = staticSnapshot->canvasWidth;
		bridge.staticSnapshotCanvasHeight = staticSnapshot->canvasHeight;
		bridge.staticSnapshotOffsetX = staticSnapshot->offsetX;
		bridge.staticSnapshotOffsetY = staticSnapshot->offsetY;
		bridge.staticSnapshotUsedMotionLayout = staticSnapshot->usedMotionLayout;
		bridge.staticSnapshotItemCount = (tjs_int)staticSnapshot->items.size();
		bridge.staticReplayScaleX = staticReplayScaleX;
		bridge.staticReplayScaleY = staticReplayScaleY;
		bridge.staticReplayTargetMatchesCanvas =
			staticSnapshot->canvasWidth == lw &&
			staticSnapshot->canvasHeight == lh;
	}
	std::set<std::string> textureRefsSeen;
	auto formatSurfaceSummary = [](const MotionRenderSurface &surface,
		const tjs_char *prefix) -> ttstr {
		return ttstr(prefix) +
			TJS_W(":type=") + ttstr(surface.type) +
			TJS_W(":label=") + (surface.label.GetLen() > 0 ? surface.label : TJS_W("void")) +
			TJS_W(":origin=") + ttstr((tjs_int)std::lround(surface.originX)) +
			TJS_W(",") + ttstr((tjs_int)std::lround(surface.originY)) +
			TJS_W(",") + ttstr((tjs_int)std::lround(surface.originW)) +
			TJS_W("x") + ttstr((tjs_int)std::lround(surface.originH)) +
			TJS_W(":opa1000=") + ttstr((tjs_int)std::lround(surface.opa * 1000.0)) +
			TJS_W(":m00k=") + ttstr((tjs_int)std::lround(surface.matTrans.m[0] * 1000.0)) +
			TJS_W(":m01k=") + ttstr((tjs_int)std::lround(surface.matTrans.m[1] * 1000.0)) +
			TJS_W(":m03k=") + ttstr((tjs_int)std::lround(surface.matTrans.m[3] * 1000.0)) +
			TJS_W(":m10k=") + ttstr((tjs_int)std::lround(surface.matTrans.m[4] * 1000.0)) +
			TJS_W(":m11k=") + ttstr((tjs_int)std::lround(surface.matTrans.m[5] * 1000.0)) +
			TJS_W(":m13k=") + ttstr((tjs_int)std::lround(surface.matTrans.m[7] * 1000.0));
	};

	for (tjs_int py = 0; py < lh; py++) {
		unsigned char *row = layerRow(py);
		if (!row) continue;
		if (fillWhiteBackground) {
			memset(row, 255, (size_t)lw * 4);
		} else {
			memset(row, 0, (size_t)lw * 4);
		}
	}

	auto clampInt = [](tjs_int value, tjs_int lo, tjs_int hi) -> tjs_int {
		if (value < lo) return lo;
		if (value > hi) return hi;
		return value;
	};
	auto extendBounds =
		[](bool &valid, tjs_real &minX, tjs_real &minY,
			tjs_real &maxX, tjs_real &maxY,
			tjs_real left, tjs_real top, tjs_real right, tjs_real bottom) {
			if (!valid) {
				minX = left;
				minY = top;
				maxX = right;
				maxY = bottom;
				valid = true;
				return;
			}
			minX = std::min(minX, left);
			minY = std::min(minY, top);
			maxX = std::max(maxX, right);
			maxY = std::max(maxY, bottom);
		};
	auto buildStaticItemSample =
		[](const TVPStaticMotionBridgeItem &item, tjs_int index) -> ttstr {
			return ttstr(TJS_W("#")) + ttstr(index) +
				TJS_W(":") + MotionString(item.ref) +
				TJS_W("@(") + ttstr(item.dstX) + TJS_W(",") + ttstr(item.dstY) +
				TJS_W(")") +
				TJS_W(" size=") + ttstr(item.width) + TJS_W("x") + ttstr(item.height) +
				TJS_W(" scale=") +
				ttstr((tjs_int)std::lround(item.scaleX * 1000.0)) + TJS_W(",") +
				ttstr((tjs_int)std::lround(item.scaleY * 1000.0)) +
				TJS_W(" alpha=") +
				ttstr((tjs_int)std::lround(item.opacity * 1000.0));
		};
	auto rasterStaticTriangle =
		[&](const TVPStaticMotionBridgeItem &item,
			const tjs_real *tx, const tjs_real *ty,
			const tjs_real *tu, const tjs_real *tv,
			tjs_real opacity, bool &coverageBoundsValid,
			tjs_real &coverageMinX, tjs_real &coverageMinY,
			tjs_real &coverageMaxX, tjs_real &coverageMaxY,
			tjs_uint64 &pixelHash, tjs_int &pixelCount,
			tjs_int &alphaPixelCount, tjs_real &alphaSum) -> bool {
			tjs_real triMinX = std::min(tx[0], std::min(tx[1], tx[2]));
			tjs_real triMinY = std::min(ty[0], std::min(ty[1], ty[2]));
			tjs_real triMaxX = std::max(tx[0], std::max(tx[1], tx[2]));
			tjs_real triMaxY = std::max(ty[0], std::max(ty[1], ty[2]));
			tjs_int minX = (tjs_int)std::max(0.0, std::floor(triMinX));
			tjs_int minY = (tjs_int)std::max(0.0, std::floor(triMinY));
			tjs_int maxX = (tjs_int)std::min((tjs_real)(lw - 1), std::ceil(triMaxX));
			tjs_int maxY = (tjs_int)std::min((tjs_real)(lh - 1), std::ceil(triMaxY));
			if (minX > maxX || minY > maxY) return false;
			tjs_real denom =
				(ty[1] - ty[2]) * (tx[0] - tx[2]) +
				(tx[2] - tx[1]) * (ty[0] - ty[2]);
			if (std::abs(denom) <= 1e-8) return false;
			bool touched = false;
			for (tjs_int py = minY; py <= maxY; py++) {
				for (tjs_int px = minX; px <= maxX; px++) {
					tjs_real sxp = (tjs_real)px + 0.5;
					tjs_real syp = (tjs_real)py + 0.5;
					tjs_real w0 =
						((ty[1] - ty[2]) * (sxp - tx[2]) +
						 (tx[2] - tx[1]) * (syp - ty[2])) / denom;
					tjs_real w1 =
						((ty[2] - ty[0]) * (sxp - tx[2]) +
						 (tx[0] - tx[2]) * (syp - ty[2])) / denom;
					tjs_real w2 = 1.0 - w0 - w1;
					if (w0 < -1e-5 || w1 < -1e-5 || w2 < -1e-5) continue;
					tjs_real u = w0 * tu[0] + w1 * tu[1] + w2 * tu[2];
					tjs_real v = w0 * tv[0] + w1 * tv[1] + w2 * tv[2];
					tjs_int srcX = clampInt(
						(tjs_int)std::floor(u * (tjs_real)item.width),
						0, item.width - 1);
					tjs_int srcY = clampInt(
						(tjs_int)std::floor(v * (tjs_real)item.height),
						0, item.height - 1);
					const unsigned char *sp = item.pixelsBGRA.data() +
						((size_t)srcY * (size_t)item.width + (size_t)srcX) * 4u;
					tjs_int sa = sp[3];
					if (opacity < 1.0) {
						sa = (tjs_int)std::lround((tjs_real)sa * opacity);
					}
					if (sa <= 0) continue;
					unsigned char *dp = layerRow(py);
					if (!dp) continue;
					dp += px * 4;
					tjs_int existR = (tjs_int)dp[2];
					tjs_int existG = (tjs_int)dp[1];
					tjs_int existB = (tjs_int)dp[0];
					tjs_int existA = (tjs_int)dp[3];
					tjs_int srcR = sp[2];
					tjs_int srcG = sp[1];
					tjs_int srcB = sp[0];
					tjs_int outA = sa + existA * (255 - sa) / 255;
					if (outA <= 0) continue;
					tjs_int outR = (srcR * sa + existR * existA * (255 - sa) / 255) / outA;
					tjs_int outG = (srcG * sa + existG * existA * (255 - sa) / 255) / outA;
					tjs_int outB = (srcB * sa + existB * existA * (255 - sa) / 255) / outA;
					dp[2] = (tjs_uint8)clampInt(outR, 0, 255);
					dp[1] = (tjs_uint8)clampInt(outG, 0, 255);
					dp[0] = (tjs_uint8)clampInt(outB, 0, 255);
					dp[3] = (tjs_uint8)clampInt(outA, 0, 255);

					MotionHashPlanDouble(pixelHash, (tjs_real)px);
					MotionHashPlanDouble(pixelHash, (tjs_real)py);
					MotionHashPlanInt(pixelHash, sa);
					MotionHashPlanInt(pixelHash, dp[2]);
					MotionHashPlanInt(pixelHash, dp[1]);
					MotionHashPlanInt(pixelHash, dp[0]);

					pixelCount++;
					alphaPixelCount++;
					alphaSum += (tjs_real)sa / 255.0;
					touched = true;
					extendBounds(coverageBoundsValid, coverageMinX, coverageMinY,
						coverageMaxX, coverageMaxY,
						(tjs_real)px, (tjs_real)py,
						(tjs_real)px + 1.0, (tjs_real)py + 1.0);
				}
			}
			return touched;
		};

	tjs_uint64 pixelHash = 1469598103934665711ULL;
	tjs_int pixelCount = 0;
	tjs_int alphaPixelCount = 0;
	tjs_real alphaSum = 0.0;
	tjs_int rasterTriangleCount = 0;
	tjs_int skippedBadSlice = 0;
	tjs_int skippedBadIndex = 0;
	tjs_int skippedMissingTexture = 0;
	tjs_int firstBadIndex = -1;
	ttstr firstBadDetails;
	tjs_int firstMissingTextureIndex = -1;
	ttstr firstMissingTextureDetails;
	tjs_int textureTotalPixelCount = 0;
	tjs_int textureAlphaNonZeroPixelCount = 0;
	tjs_int textureRedPixelCount = 0;
	tjs_int textureWhitePixelCount = 0;
	tjs_int textureBlackPixelCount = 0;
	tjs_uint64 textureColorHash = 1469598103934665711ULL;
	tjs_int sampledSourcePixelCount = 0;
	tjs_int sampledSourceAlphaNonZeroPixelCount = 0;
	tjs_int sampledSourceRedPixelCount = 0;
	tjs_int sampledSourceWhitePixelCount = 0;
	tjs_int sampledSourceBlackPixelCount = 0;
	tjs_uint64 sampledSourceColorHash = 1469598103934665711ULL;
	bool sampledUvBoundsValid = false;
	tjs_real sampledUvMinU = 0.0;
	tjs_real sampledUvMaxU = 0.0;
	tjs_real sampledUvMinV = 0.0;
	tjs_real sampledUvMaxV = 0.0;
	tjs_int maxCommandCoveragePixels = -1;
	tjs_int maxNonBackgroundCommandCoveragePixels = -1;
	tjs_int maxBlackCommandPixelCount = -1;
	ttstr maxCommandCoverageSummary;
	ttstr maxCommandIRSummary;
	ttstr maxNonBackgroundCommandCoverageSummary;
	ttstr maxNonBackgroundCommandIRSummary;
	ttstr maxBlackCommandCoverageSummary;
	ttstr maxBlackCommandIRSummary;
	auto describeCommandCoverage =
		[&](tjs_int ci, const MotionRendererCommandEntryInfo &cmd,
			const MotionRenderMethodItem *irItem, const MotionLayoutInfo *layout,
			const TVPStaticMotionBridgeItem *texture,
			tjs_int commandCoveragePixels,
			bool commandBoundsValid,
			tjs_real commandMinX, tjs_real commandMinY,
			tjs_real commandMaxX, tjs_real commandMaxY) -> ttstr {
			return TJS_W("ci=") + ttstr((tjs_int)ci) +
				TJS_W(" rm=") + ttstr(cmd.renderMethodItemIndex) +
				TJS_W(" src=") +
				(irItem ? MotionString(irItem->srcKey) :
					(layout && layout->sourceKey.GetLen() > 0 ? layout->sourceKey : TJS_W("void"))) +
				TJS_W(" ref=") + MotionString(texture ? texture->ref : std::string()) +
				TJS_W(" px=") + ttstr(commandCoveragePixels) +
				TJS_W(" bounds=") +
				(commandBoundsValid ?
					(ttstr((tjs_int)std::lround(commandMinX)) + TJS_W(",") +
					 ttstr((tjs_int)std::lround(commandMinY)) + TJS_W("-") +
					 ttstr((tjs_int)std::lround(commandMaxX)) + TJS_W(",") +
					 ttstr((tjs_int)std::lround(commandMaxY))) :
					ttstr(TJS_W("void")));
		};
		auto describeCommandIR =
			[&](const MotionRendererCommandEntryInfo &cmd,
				const MotionRenderMethodItem *irItem, const MotionLayoutInfo *layout) -> ttstr {
			if (irItem) {
				ttstr summary =
					TJS_W("rm=") + ttstr(cmd.renderMethodItemIndex) +
					TJS_W(" label=") + (irItem->label.GetLen() > 0 ? irItem->label : TJS_W("void")) +
					TJS_W(" src=") + MotionString(irItem->srcKey) +
					TJS_W(" type=") + ttstr(irItem->type) +
					TJS_W(" layoutIndex=") + ttstr(irItem->layoutIndex) +
					TJS_W(" sourceNode=") + ttstr(irItem->sourceNodeIndex) +
					TJS_W(" linked=") + ttstr((tjs_int)(irItem->linkedToLayout ? 1 : 0)) +
					TJS_W(" surfaces=") + ttstr((tjs_int)irItem->surfaces.size()) +
					TJS_W(" itemOrigin=") + ttstr((tjs_int)std::lround(irItem->originX)) +
					TJS_W(",") + ttstr((tjs_int)std::lround(irItem->originY)) +
					TJS_W(",") + ttstr((tjs_int)std::lround(irItem->width)) +
					TJS_W("x") + ttstr((tjs_int)std::lround(irItem->height));
					if (!irItem->surfaces.empty()) {
						summary += TJS_W(" ");
						summary += formatSurfaceSummary(irItem->surfaces.front(), TJS_W("first"));
						summary += TJS_W(" ");
						summary += formatSurfaceSummary(irItem->surfaces.back(), TJS_W("last"));
						summary += TJS_W(" chain=");
						for (size_t si = 0; si < irItem->surfaces.size(); si++) {
							if (si > 0) summary += TJS_W("|");
							summary += formatSurfaceSummary(irItem->surfaces[si], TJS_W("s"));
						}
					}
					return summary;
				}
			if (layout) {
				return TJS_W("layoutOnly sourceIndex=") + ttstr(cmd.sourceIndex) +
					TJS_W(" name=") + layout->name +
					TJS_W(" src=") + (layout->sourceKey.GetLen() > 0 ? layout->sourceKey : TJS_W("void")) +
					TJS_W(" bounds=") + ttstr((tjs_int)std::lround(layout->left)) +
					TJS_W(",") + ttstr((tjs_int)std::lround(layout->top)) +
					TJS_W(",") + ttstr((tjs_int)std::lround(layout->width)) +
					TJS_W("x") + ttstr((tjs_int)std::lround(layout->height));
			}
			return TJS_W("void");
		};
	auto captureFinalLayerColorStats = [&]()
	{
		tjs_int totalPixelCount = 0;
		tjs_int alphaNonZeroPixelCount = 0;
		tjs_int redPixelCount = 0;
		tjs_int whitePixelCount = 0;
		tjs_int blackPixelCount = 0;
		tjs_uint64 finalHash = 1469598103934665711ULL;
		std::set<tjs_uint32> buckets;
		for (tjs_int py = 0; py < lh; py++) {
			const unsigned char *row = layerConstRow(py);
			if (!row) continue;
			for (tjs_int px = 0; px < lw; px++) {
				const unsigned char *p = row + px * 4;
				tjs_int b = p[0];
				tjs_int g = p[1];
				tjs_int r = p[2];
				tjs_int a = p[3];
				totalPixelCount++;
				if (a > 0) alphaNonZeroPixelCount++;
				if (r >= 140 && g <= 110 && b <= 110 && a >= 160) redPixelCount++;
				if (r >= 245 && g >= 245 && b >= 245 && a >= 245) whitePixelCount++;
				if (r <= 24 && g <= 24 && b <= 24 && a >= 160) blackPixelCount++;
				tjs_uint32 bucket =
					((tjs_uint32)(r >> 4) << 12) |
					((tjs_uint32)(g >> 4) << 8) |
					((tjs_uint32)(b >> 4) << 4) |
					((tjs_uint32)(a >> 6));
				buckets.insert(bucket);
				MotionHashPlanInt(finalHash, r);
				MotionHashPlanInt(finalHash, g);
				MotionHashPlanInt(finalHash, b);
				MotionHashPlanInt(finalHash, a);
			}
		}
		bridge.finalLayerPixelCount = totalPixelCount;
		bridge.finalLayerAlphaNonZeroPixelCount = alphaNonZeroPixelCount;
		bridge.finalLayerRedPixelCount = redPixelCount;
		bridge.finalLayerWhitePixelCount = whitePixelCount;
		bridge.finalLayerBlackPixelCount = blackPixelCount;
		bridge.finalLayerColorBucketCount = (tjs_int)buckets.size();
		bridge.finalLayerColorHash = finalHash;
		bridge.finalLayerColorSummary =
			TJS_W("pixels=") + ttstr(totalPixelCount) +
			TJS_W(" alpha=") + ttstr(alphaNonZeroPixelCount) +
			TJS_W(" red=") + ttstr(redPixelCount) +
			TJS_W(" white=") + ttstr(whitePixelCount) +
			TJS_W(" black=") + ttstr(blackPixelCount) +
			TJS_W(" buckets=") + ttstr((tjs_int)buckets.size());
	};
	auto publishLayerUpdate = [&]()
	{
		try {
			layer->Update();
#ifdef __EMSCRIPTEN__
			ForceMotionLayerUpdate();
#endif
		} catch (...) {
		}
	};

	auto markBadSlice = [&](tjs_int index, const ttstr &details)
	{
		skippedBadSlice++;
		if (firstBadIndex < 0) {
			firstBadIndex = index;
			firstBadDetails = details;
		}
	};
	auto markBadIndex = [&](tjs_int index, const ttstr &details)
	{
		skippedBadIndex++;
		if (firstBadIndex < 0) {
			firstBadIndex = index;
			firstBadDetails = details;
		}
	};
	auto markMissingTexture = [&](tjs_int index, const ttstr &details)
	{
		skippedMissingTexture++;
		if (firstMissingTextureIndex < 0) {
			firstMissingTextureIndex = index;
			firstMissingTextureDetails = details;
		}
	};

	tjs_int maxVertIdx = (tjs_int)(stagingArtifact.position.size() / 2) - 1;
	tjs_int maxAlphaIdx = (tjs_int)stagingArtifact.alpha.size() - 1;

	if (useStaticDrawReplay) {
		bridge.source = TJS_W("static-motion-draw-items+blit");
		bool replayBoundsValid = false;
		tjs_real replayMinX = 0.0;
		tjs_real replayMinY = 0.0;
		tjs_real replayMaxX = 0.0;
		tjs_real replayMaxY = 0.0;
		bool coverageBoundsValid = false;
		tjs_real coverageMinX = 0.0;
		tjs_real coverageMinY = 0.0;
		tjs_real coverageMaxX = 0.0;
		tjs_real coverageMaxY = 0.0;
		tjs_int clippedItemCount = 0;
		tjs_int fullyOutsideItemCount = 0;
		tjs_int firstSkippedItemIndex = -1;
		tjs_int firstClippedItemIndex = -1;
		ttstr firstSkippedItemDetail;
		ttstr firstClippedItemDetail;
		for (size_t si = 0; si < staticSnapshot->items.size(); si++) {
			const TVPStaticMotionBridgeItem &item = staticSnapshot->items[si];
			ttstr itemSample = buildStaticItemSample(item, (tjs_int)si);
			if (item.width <= 0 || item.height <= 0 ||
				item.pixelsBGRA.size() < (size_t)item.width * (size_t)item.height * 4u ||
				item.scaleX <= 0.0 || item.scaleY <= 0.0 || item.opacity <= 0.001 ||
				staticReplayScaleX <= 0.0 || staticReplayScaleY <= 0.0) {
				skippedBadSlice++;
				if (firstSkippedItemIndex < 0) {
					firstSkippedItemIndex = (tjs_int)si;
					firstSkippedItemDetail = itemSample;
				}
				continue;
			}
			if (bridge.staticFirstItemSample.IsEmpty()) {
				bridge.staticFirstItemSample = itemSample;
			}
			bridge.staticLastItemSample = itemSample;
			tjs_real quadX[4];
			tjs_real quadY[4];
			if (item.hasQuad) {
				for (int qi = 0; qi < 4; qi++) {
					quadX[qi] = (tjs_real)item.quadX[qi] * staticReplayScaleX;
					quadY[qi] = (tjs_real)item.quadY[qi] * staticReplayScaleY;
				}
			} else {
				tjs_real dstLeftF = (tjs_real)item.dstX * staticReplayScaleX;
				tjs_real dstTopF = (tjs_real)item.dstY * staticReplayScaleY;
				tjs_real scaledItemW = (tjs_real)item.width * item.scaleX * staticReplayScaleX;
				tjs_real scaledItemH = (tjs_real)item.height * item.scaleY * staticReplayScaleY;
				quadX[0] = dstLeftF;
				quadY[0] = dstTopF;
				quadX[1] = dstLeftF + scaledItemW;
				quadY[1] = dstTopF;
				quadX[2] = quadX[1];
				quadY[2] = dstTopF + scaledItemH;
				quadX[3] = dstLeftF;
				quadY[3] = quadY[2];
			}
			tjs_real dstLeftF = std::min(quadX[0], std::min(quadX[1], std::min(quadX[2], quadX[3])));
			tjs_real dstTopF = std::min(quadY[0], std::min(quadY[1], std::min(quadY[2], quadY[3])));
			tjs_real dstRightF = std::max(quadX[0], std::max(quadX[1], std::max(quadX[2], quadX[3])));
			tjs_real dstBottomF = std::max(quadY[0], std::max(quadY[1], std::max(quadY[2], quadY[3])));
			extendBounds(replayBoundsValid, replayMinX, replayMinY,
				replayMaxX, replayMaxY,
				dstLeftF, dstTopF, dstRightF, dstBottomF);
			bool itemOutOfBounds =
				dstLeftF < -0.01 || dstTopF < -0.01 ||
				dstRightF > (tjs_real)lw + 0.01 ||
				dstBottomF > (tjs_real)lh + 0.01;
			bool itemFullyOutside =
				dstRightF <= 0.0 || dstBottomF <= 0.0 ||
				dstLeftF >= (tjs_real)lw || dstTopF >= (tjs_real)lh;
			if (itemOutOfBounds) {
				clippedItemCount++;
				if (itemFullyOutside) fullyOutsideItemCount++;
				if (firstClippedItemIndex < 0) {
					firstClippedItemIndex = (tjs_int)si;
					firstClippedItemDetail = itemSample +
						TJS_W(" target=") + ttstr(lw) + TJS_W("x") + ttstr(lh);
				}
			}
			tjs_real opacity = std::max<tjs_real>(0.0, std::min<tjs_real>(1.0, item.opacity));
			tjs_real tri0x[3] = {quadX[0], quadX[1], quadX[2]};
			tjs_real tri0y[3] = {quadY[0], quadY[1], quadY[2]};
			tjs_real tri0u[3] = {0.0, 1.0, 1.0};
			tjs_real tri0v[3] = {0.0, 0.0, 1.0};
			if (rasterStaticTriangle(item, tri0x, tri0y, tri0u, tri0v,
				opacity, coverageBoundsValid, coverageMinX, coverageMinY,
				coverageMaxX, coverageMaxY, pixelHash, pixelCount,
				alphaPixelCount, alphaSum)) {
				bridge.triangleCount++;
			}
			tjs_real tri1x[3] = {quadX[0], quadX[2], quadX[3]};
			tjs_real tri1y[3] = {quadY[0], quadY[2], quadY[3]};
			tjs_real tri1u[3] = {0.0, 1.0, 0.0};
			tjs_real tri1v[3] = {0.0, 1.0, 1.0};
			if (rasterStaticTriangle(item, tri1x, tri1y, tri1u, tri1v,
				opacity, coverageBoundsValid, coverageMinX, coverageMinY,
				coverageMaxX, coverageMaxY, pixelHash, pixelCount,
				alphaPixelCount, alphaSum)) {
				bridge.triangleCount++;
			}
		}
		bridge.staticReplayBoundsValid = replayBoundsValid;
		bridge.staticReplayBoundsMinX = replayMinX;
		bridge.staticReplayBoundsMinY = replayMinY;
		bridge.staticReplayBoundsMaxX = replayMaxX;
		bridge.staticReplayBoundsMaxY = replayMaxY;
		bridge.staticReplayCoverageValid = coverageBoundsValid;
		bridge.staticReplayCoverageMinX = coverageMinX;
		bridge.staticReplayCoverageMinY = coverageMinY;
		bridge.staticReplayCoverageMaxX = coverageMaxX;
		bridge.staticReplayCoverageMaxY = coverageMaxY;
			bridge.staticReplayInBounds =
				replayBoundsValid &&
				replayMinX >= -0.01 && replayMinY >= -0.01 &&
				replayMaxX <= (tjs_real)lw + 0.01 &&
				replayMaxY <= (tjs_real)lh + 0.01;
			bridge.staticReplayCoverageInsideTarget =
				coverageBoundsValid &&
				coverageMinX >= -0.01 && coverageMinY >= -0.01 &&
				coverageMaxX <= (tjs_real)lw + 0.01 &&
				coverageMaxY <= (tjs_real)lh + 0.01;
			bool replayIntersectsTarget =
				replayBoundsValid &&
				replayMaxX > 0.0 && replayMaxY > 0.0 &&
				replayMinX < (tjs_real)lw && replayMinY < (tjs_real)lh;
			bridge.staticReplayViewportClippingUsed = clippedItemCount > 0;
			bridge.staticReplayViewportClipOk =
				replayIntersectsTarget &&
				coverageBoundsValid &&
				bridge.staticReplayCoverageInsideTarget &&
				pixelCount > 0;
			bridge.staticReplayCoverageMatchesBounds =
				bridge.staticReplayInBounds ? (
					replayBoundsValid && coverageBoundsValid &&
					coverageMinX + 1.0 >= std::floor(replayMinX) &&
					coverageMinY + 1.0 >= std::floor(replayMinY) &&
					coverageMaxX <= std::ceil(replayMaxX) + 1.0 &&
					coverageMaxY <= std::ceil(replayMaxY) + 1.0) :
					bridge.staticReplayViewportClipOk;
			bridge.staticReplaySkippedItemCount = skippedBadSlice;
			bridge.staticReplayClippedItemCount = clippedItemCount;
			bridge.staticReplayFullyOutsideItemCount = fullyOutsideItemCount;
			bridge.pixelCount = pixelCount;
			bridge.alphaNonZeroPixelCount = alphaPixelCount;
			bridge.alphaAccum = alphaSum;
			bridge.pixelHash = pixelHash;
			bridge.rendered = pixelCount > 0;
			bridge.rendererPathUsed = pixelCount > 0;
			if (rendererDiagnostics) {
				captureFinalLayerColorStats();
			}
			publishLayerUpdate();
		if (!bridge.staticSnapshotUsedMotionLayout) {
			bridge.mismatchKind = TJS_W("static-snapshot-not-motion-layout");
			bridge.mismatchIndex = 0;
			bridge.mismatchDetails = TJS_W("snapshot usedMotionLayout=0");
		} else if (bridge.staticSnapshotItemCount <= 0) {
			bridge.mismatchKind = TJS_W("static-snapshot-empty");
			bridge.mismatchIndex = 0;
			bridge.mismatchDetails = TJS_W("snapshot items=0");
		} else if (bridge.staticReplayScaleX <= 0.0 || bridge.staticReplayScaleY <= 0.0) {
			bridge.mismatchKind = TJS_W("invalid-static-replay-scale");
			bridge.mismatchIndex = 0;
			bridge.mismatchDetails =
				TJS_W("scale=") +
				ttstr((tjs_int)std::lround(bridge.staticReplayScaleX * 1000.0)) +
				TJS_W(",") +
				ttstr((tjs_int)std::lround(bridge.staticReplayScaleY * 1000.0));
		} else if (skippedBadSlice > 0) {
			bridge.mismatchKind = TJS_W("bad-static-draw-item");
			bridge.mismatchIndex = firstSkippedItemIndex;
			bridge.mismatchDetails = firstSkippedItemDetail;
		} else if (!bridge.staticReplayBoundsValid) {
			bridge.mismatchKind = TJS_W("static-replay-bounds-empty");
			bridge.mismatchIndex = 0;
			bridge.mismatchDetails =
				TJS_W("items=") + ttstr(bridge.staticSnapshotItemCount);
		} else if (!bridge.staticReplayInBounds && !bridge.staticReplayViewportClipOk) {
			bridge.mismatchKind = TJS_W("static-replay-out-of-bounds");
			bridge.mismatchIndex = firstClippedItemIndex;
			bridge.mismatchDetails = firstClippedItemDetail;
		} else if (!bridge.staticReplayCoverageValid || pixelCount == 0) {
			bridge.mismatchKind = TJS_W("no-coverage");
			bridge.mismatchIndex = 0;
			bridge.mismatchDetails =
				TJS_W("items=") + ttstr(bridge.staticSnapshotItemCount) +
				TJS_W(" pixels=") + ttstr(pixelCount);
		} else if (!bridge.staticReplayCoverageMatchesBounds) {
			bridge.mismatchKind = TJS_W("static-replay-coverage-bounds-mismatch");
			bridge.mismatchIndex = 0;
			bridge.mismatchDetails =
				TJS_W("coverage=(") +
				ttstr((tjs_int)std::lround(coverageMinX)) + TJS_W(",") +
				ttstr((tjs_int)std::lround(coverageMinY)) + TJS_W(")-(") +
				ttstr((tjs_int)std::lround(coverageMaxX)) + TJS_W(",") +
				ttstr((tjs_int)std::lround(coverageMaxY)) + TJS_W(")");
		} else {
			bridge.mismatchKind = TJS_W("none");
			bridge.mismatchIndex = -1;
		}
		if (MotionVerboseLogWanted()) {
			TVPAddLog(ttstr(TJS_W("[emoteplayer] visibleRenderBridge rasterized path=")) + path
				+ TJS_W(" pixels=") + ttstr(pixelCount)
				+ TJS_W(" triangles=") + ttstr(bridge.triangleCount)
				+ TJS_W(" commands=") + ttstr(bridge.commandCount)
				+ TJS_W(" staticItems=") + ttstr(bridge.staticSnapshotItemCount)
				+ TJS_W(" clipped=") + ttstr(bridge.staticReplayClippedItemCount)
				+ TJS_W(" mismatch=") + bridge.mismatchKind
				+ TJS_W(" source=") + bridge.source);
		}
		MotionExportVisibleRenderBridgeToJs(bridge, path);
		return bridge;
	}

	for (size_t ci = 0; ci < rendererCommandEntries.size(); ci++) {
		const auto &cmd = rendererCommandEntries[ci];
		const MotionLayoutInfo *layout = nullptr;
		const MotionRenderMethodItem *irItem = nullptr;
		if (resource && cmd.renderMethodItemIndex >= 0 &&
			cmd.renderMethodItemIndex < (tjs_int)resource->renderMethodIR.size()) {
			irItem = &resource->renderMethodIR[(size_t)cmd.renderMethodItemIndex];
		}
		if (resource && cmd.sourceIndex >= 0 &&
			cmd.sourceIndex < (tjs_int)resource->layouts.size()) {
			layout = &resource->layouts[(size_t)cmd.sourceIndex];
		}
		if (irItem && (!MotionRenderMethodItemRenderableForRuntimeDraw(*irItem))) {
			markBadIndex((tjs_int)ci,
				ttstr(TJS_W("ir item missing/empty renderMethodItemIndex=")) +
				ttstr(cmd.renderMethodItemIndex));
			continue;
		}
		if (!irItem && (!layout || layout->width <= 0.0 || layout->height <= 0.0)) {
			markBadIndex((tjs_int)ci,
				ttstr(TJS_W("layout missing/empty sourceIndex=")) +
				ttstr(cmd.sourceIndex));
			continue;
		}
		const TVPStaticMotionBridgeItem *texture =
			irItem ?
			MotionFindSnapshotItemForSourceKey(staticSnapshot, irItem->srcKey) :
			MotionFindSnapshotItemForLayout(staticSnapshot, layout);
		if (!texture || texture->width <= 0 || texture->height <= 0 ||
			texture->pixelsBGRA.size() <
				(size_t)texture->width * (size_t)texture->height * 4u) {
			markMissingTexture((tjs_int)ci,
				ttstr(TJS_W("texture missing/empty sourceKey=")) +
				(irItem ? MotionString(irItem->srcKey) :
					(layout && layout->sourceKey.GetLen() > 0 ? layout->sourceKey : TJS_W("void"))) +
				TJS_W(" staticItems=") + ttstr(bridge.staticSnapshotItemCount));
			continue;
		}
		if (rendererDiagnostics && textureRefsSeen.insert(texture->ref).second) {
			MotionAccumulateTextureColorStats(
				*texture,
				textureTotalPixelCount,
				textureAlphaNonZeroPixelCount,
				textureRedPixelCount,
				textureWhitePixelCount,
				textureBlackPixelCount,
				textureColorHash);
		}
		if (bridge.staticFirstItemSample.IsEmpty()) {
			bridge.staticFirstItemSample =
				ttstr(TJS_W("#")) + ttstr((tjs_int)ci) +
				TJS_W(":") + MotionString(texture->ref) +
				TJS_W(" size=") + ttstr(texture->width) +
				TJS_W("x") + ttstr(texture->height);
		}
		bridge.staticLastItemSample =
			ttstr(TJS_W("#")) + ttstr((tjs_int)ci) +
			TJS_W(":") + MotionString(texture->ref) +
			TJS_W(" size=") + ttstr(texture->width) +
			TJS_W("x") + ttstr(texture->height);
		if (cmd.indexCount < 6 || cmd.indexOffset < 0 ||
			(cmd.indexCount % 6) != 0) {
			markBadSlice((tjs_int)ci,
				ttstr(TJS_W("bad command slice offset=")) +
				ttstr(cmd.indexOffset) + TJS_W(" count=") +
				ttstr(cmd.indexCount));
			continue;
		}
		tjs_int idxEnd = cmd.indexOffset + cmd.indexCount;
		if (idxEnd > (tjs_int)stagingArtifact.index.size()) {
			markBadSlice((tjs_int)ci,
				ttstr(TJS_W("index slice oob end=")) + ttstr(idxEnd) +
				TJS_W(" size=") + ttstr((tjs_int)stagingArtifact.index.size()));
			continue;
		}

		tjs_int quads = cmd.indexCount / 6;
		tjs_int commandCoveragePixels = 0;
		tjs_int commandBlackPixels = 0;
		tjs_real commandMinX = 0.0;
		tjs_real commandMinY = 0.0;
		tjs_real commandMaxX = 0.0;
		tjs_real commandMaxY = 0.0;
		bool commandBoundsValid = false;
		for (tjs_int qi = 0; qi < quads; qi++) {
			tjs_int base = cmd.indexOffset + qi * 6;
			if (base + 5 >= (tjs_int)stagingArtifact.index.size()) {
				markBadSlice((tjs_int)ci,
					ttstr(TJS_W("quad slice oob q=")) + ttstr(qi));
				break;
			}

			for (int t = 0; t < 2; t++) {
				tjs_int iA = stagingArtifact.index[base + (t == 0 ? 0 : 2)];
				tjs_int iB = stagingArtifact.index[base + (t == 0 ? 1 : 4)];
				tjs_int iC = stagingArtifact.index[base + (t == 0 ? 2 : 5)];

				if (iA < 0 || iA > maxVertIdx ||
					iB < 0 || iB > maxVertIdx ||
					iC < 0 || iC > maxVertIdx ||
					iA > maxAlphaIdx || iB > maxAlphaIdx || iC > maxAlphaIdx) {
					markBadIndex((tjs_int)ci,
						ttstr(TJS_W("vertex/alpha index oob tri=")) + ttstr(t) +
						TJS_W(" idx=") + ttstr(iA) + TJS_W(",") + ttstr(iB) +
						TJS_W(",") + ttstr(iC));
					continue;
				}

				tjs_real ax, ay, bx, by, cx, cy;
				tjs_real uA = 0.0, vA = 0.0;
				tjs_real uB = 0.0, vB = 0.0;
				tjs_real uC = 0.0, vC = 0.0;
				tjs_int aA = 255, aB = 255, aC = 255;

				if (iA * 2 + 1 < (tjs_int)stagingArtifact.position.size()) {
					ax = stagingArtifact.position[iA * 2];
					ay = stagingArtifact.position[iA * 2 + 1];
				} else {
					markBadIndex((tjs_int)ci, TJS_W("position A oob"));
					continue;
				}
				if (iA * 2 + 1 < (tjs_int)stagingArtifact.uv.size()) {
					uA = stagingArtifact.uv[iA * 2];
					vA = stagingArtifact.uv[iA * 2 + 1];
				}
				if (iB * 2 + 1 < (tjs_int)stagingArtifact.position.size()) {
					bx = stagingArtifact.position[iB * 2];
					by = stagingArtifact.position[iB * 2 + 1];
				} else {
					markBadIndex((tjs_int)ci, TJS_W("position B oob"));
					continue;
				}
				if (iB * 2 + 1 < (tjs_int)stagingArtifact.uv.size()) {
					uB = stagingArtifact.uv[iB * 2];
					vB = stagingArtifact.uv[iB * 2 + 1];
				}
				if (iC * 2 + 1 < (tjs_int)stagingArtifact.position.size()) {
					cx = stagingArtifact.position[iC * 2];
					cy = stagingArtifact.position[iC * 2 + 1];
				} else {
					markBadIndex((tjs_int)ci, TJS_W("position C oob"));
					continue;
				}
				if (iC * 2 + 1 < (tjs_int)stagingArtifact.uv.size()) {
					uC = stagingArtifact.uv[iC * 2];
					vC = stagingArtifact.uv[iC * 2 + 1];
				}

				aA = stagingArtifact.alpha[iA];
				aB = stagingArtifact.alpha[iB];
				aC = stagingArtifact.alpha[iC];

				tjs_real txs[3] = { ax, bx, cx };
				tjs_real tys[3] = { ay, by, cy };
				if (!MotionIsFiniteTriangle(txs, tys) ||
					!MotionIsFiniteReal(uA) || !MotionIsFiniteReal(vA) ||
					!MotionIsFiniteReal(uB) || !MotionIsFiniteReal(vB) ||
					!MotionIsFiniteReal(uC) || !MotionIsFiniteReal(vC)) {
					markBadIndex((tjs_int)ci,
						ttstr(TJS_W("non-finite triangle/uv tri=")) +
						ttstr(t));
					continue;
				}

				rasterTriangleCount++;

				tjs_int minX = (tjs_int)std::max(0.0, std::floor(std::min({ax, bx, cx})));
				tjs_int maxX = (tjs_int)std::min(static_cast<double>(lw - 1), std::ceil(std::max({ax, bx, cx})));
				tjs_int minY = (tjs_int)std::max(0.0, std::floor(std::min({ay, by, cy})));
				tjs_int maxY = (tjs_int)std::min(static_cast<double>(lh - 1), std::ceil(std::max({ay, by, cy})));

				if (maxX < 0 || maxY < 0 || minX >= lw || minY >= lh) continue;

				tjs_real denom =
					(by - cy) * (ax - cx) + (cx - bx) * (ay - cy);
				if (std::abs(denom) <= 1e-6) continue;

				for (tjs_int py = minY; py <= maxY; py++) {
					for (tjs_int px = minX; px <= maxX; px++) {
						tjs_real sx = (tjs_real)px + 0.5f;
						tjs_real sy = (tjs_real)py + 0.5f;
						tjs_real wA =
							((by - cy) * (sx - cx) + (cx - bx) * (sy - cy)) / denom;
						tjs_real wB =
							((cy - ay) * (sx - cx) + (ax - cx) * (sy - cy)) / denom;
						tjs_real wC = 1.0 - wA - wB;
						if (wA < -1e-4 || wB < -1e-4 || wC < -1e-4) continue;

						tjs_real localU = wA * uA + wB * uB + wC * uC;
						tjs_real localV = wA * vA + wB * vB + wC * vC;
						tjs_real alphaF = wA * (tjs_real)aA +
							wB * (tjs_real)aB + wC * (tjs_real)aC;
						tjs_int alpha = clampInt((tjs_int)std::lround(alphaF), 0, 255);
						if (alpha <= 0) continue;

						if (!MotionIsFiniteReal(localU) || !MotionIsFiniteReal(localV)) {
							continue;
						}
						tjs_int sampleX =
							clampInt((tjs_int)std::floor(localU * (tjs_real)texture->width),
								0, texture->width - 1);
						tjs_int sampleY =
							clampInt((tjs_int)std::floor(localV * (tjs_real)texture->height),
								0, texture->height - 1);
						size_t sampleOffset =
							((size_t)sampleY * (size_t)texture->width +
							 (size_t)sampleX) * 4u;
						if (sampleOffset + 3 >= texture->pixelsBGRA.size()) continue;

						tjs_int sampleA = texture->pixelsBGRA[sampleOffset + 3];
						tjs_int srcB = texture->pixelsBGRA[sampleOffset + 0];
						tjs_int srcG = texture->pixelsBGRA[sampleOffset + 1];
						tjs_int srcR = texture->pixelsBGRA[sampleOffset + 2];
						if (rendererDiagnostics) {
							sampledSourcePixelCount++;
							if (sampleA > 0) sampledSourceAlphaNonZeroPixelCount++;
							if (srcR >= 140 && srcG <= 110 && srcB <= 110 && sampleA >= 160) {
								sampledSourceRedPixelCount++;
							}
							if (srcR >= 245 && srcG >= 245 && srcB >= 245 && sampleA >= 245) {
								sampledSourceWhitePixelCount++;
							}
							if (srcR <= 24 && srcG <= 24 && srcB <= 24 && sampleA >= 160) {
								sampledSourceBlackPixelCount++;
							}
							MotionHashPlanInt(sampledSourceColorHash, srcR);
							MotionHashPlanInt(sampledSourceColorHash, srcG);
							MotionHashPlanInt(sampledSourceColorHash, srcB);
							MotionHashPlanInt(sampledSourceColorHash, sampleA);
							if (!sampledUvBoundsValid) {
								sampledUvMinU = sampledUvMaxU = localU;
								sampledUvMinV = sampledUvMaxV = localV;
								sampledUvBoundsValid = true;
							} else {
								sampledUvMinU = std::min(sampledUvMinU, localU);
								sampledUvMaxU = std::max(sampledUvMaxU, localU);
								sampledUvMinV = std::min(sampledUvMinV, localV);
								sampledUvMaxV = std::max(sampledUvMaxV, localV);
							}
						}
						tjs_int srcAlpha = (sampleA * alpha) / 255;
						if (srcAlpha <= 0) continue;

						unsigned char *dp = layerRow(py);
						if (!dp) continue;
						dp += px * 4;
						tjs_int existR = (tjs_int)dp[2];
						tjs_int existG = (tjs_int)dp[1];
						tjs_int existB = (tjs_int)dp[0];
						tjs_int existA = (tjs_int)dp[3];
						tjs_int outA = srcAlpha + existA * (255 - srcAlpha) / 255;
						if (outA <= 0) continue;
						tjs_int outR = (srcR * srcAlpha + existR * existA * (255 - srcAlpha) / 255) / outA;
						tjs_int outG = (srcG * srcAlpha + existG * existA * (255 - srcAlpha) / 255) / outA;
						tjs_int outB = (srcB * srcAlpha + existB * existA * (255 - srcAlpha) / 255) / outA;
						dp[2] = (tjs_uint8)clampInt(outR, 0, 255);
						dp[1] = (tjs_uint8)clampInt(outG, 0, 255);
						dp[0] = (tjs_uint8)clampInt(outB, 0, 255);
						dp[3] = (tjs_uint8)clampInt(outA, 0, 255);
						if (outR <= 24 && outG <= 24 && outB <= 24 && outA >= 160) {
							commandBlackPixels++;
						}

						MotionHashPlanDouble(pixelHash, (tjs_real)px);
						MotionHashPlanDouble(pixelHash, (tjs_real)py);
						MotionHashPlanInt(pixelHash, srcAlpha);
						MotionHashPlanInt(pixelHash, dp[2]);
						MotionHashPlanInt(pixelHash, dp[1]);
						MotionHashPlanInt(pixelHash, dp[0]);

						pixelCount++;
						commandCoveragePixels++;
						if (!commandBoundsValid) {
							commandMinX = (tjs_real)px;
							commandMinY = (tjs_real)py;
							commandMaxX = (tjs_real)px + 1.0;
							commandMaxY = (tjs_real)py + 1.0;
							commandBoundsValid = true;
						} else {
							if ((tjs_real)px < commandMinX) commandMinX = (tjs_real)px;
							if ((tjs_real)py < commandMinY) commandMinY = (tjs_real)py;
							if ((tjs_real)px + 1.0 > commandMaxX) commandMaxX = (tjs_real)px + 1.0;
							if ((tjs_real)py + 1.0 > commandMaxY) commandMaxY = (tjs_real)py + 1.0;
						}
						if (srcAlpha > 0) {
							alphaPixelCount++;
							alphaSum += (tjs_real)srcAlpha / 255.0;
						}
					}
				}
			}
		}
		if (commandCoveragePixels > maxCommandCoveragePixels) {
			maxCommandCoveragePixels = commandCoveragePixels;
			maxCommandCoverageSummary = describeCommandCoverage(
				(tjs_int)ci, cmd, irItem, layout, texture,
				commandCoveragePixels, commandBoundsValid,
				commandMinX, commandMinY, commandMaxX, commandMaxY);
			maxCommandIRSummary = describeCommandIR(cmd, irItem, layout);
		}
		bool isBackgroundCommand =
			texture && texture->ref.find("icon50") != std::string::npos;
		if (!isBackgroundCommand &&
			commandCoveragePixels > maxNonBackgroundCommandCoveragePixels) {
			maxNonBackgroundCommandCoveragePixels = commandCoveragePixels;
			maxNonBackgroundCommandCoverageSummary = describeCommandCoverage(
				(tjs_int)ci, cmd, irItem, layout, texture,
				commandCoveragePixels, commandBoundsValid,
				commandMinX, commandMinY, commandMaxX, commandMaxY);
			maxNonBackgroundCommandIRSummary = describeCommandIR(cmd, irItem, layout);
		}
		if (commandBlackPixels > maxBlackCommandPixelCount) {
			maxBlackCommandPixelCount = commandBlackPixels;
			maxBlackCommandCoverageSummary = describeCommandCoverage(
				(tjs_int)ci, cmd, irItem, layout, texture,
				commandCoveragePixels, commandBoundsValid,
				commandMinX, commandMinY, commandMaxX, commandMaxY) +
				TJS_W(" black=") + ttstr(commandBlackPixels);
			maxBlackCommandIRSummary = describeCommandIR(cmd, irItem, layout);
		}
	}

	bridge.triangleCount = rasterTriangleCount;
	bridge.pixelCount = pixelCount;
	bridge.alphaNonZeroPixelCount = alphaPixelCount;
	bridge.alphaAccum = alphaSum;
	bridge.pixelHash = pixelHash;
	bridge.rendered = pixelCount > 0;
	bridge.rendererPathUsed = pixelCount > 0;
	bridge.textureUniqueRefCount = (tjs_int)textureRefsSeen.size();
	bridge.textureTotalPixelCount = textureTotalPixelCount;
	bridge.textureAlphaNonZeroPixelCount = textureAlphaNonZeroPixelCount;
	bridge.textureRedPixelCount = textureRedPixelCount;
	bridge.textureWhitePixelCount = textureWhitePixelCount;
	bridge.textureBlackPixelCount = textureBlackPixelCount;
	bridge.textureColorHash = textureColorHash;
	bridge.textureColorSummary =
		TJS_W("refs=") + ttstr(bridge.textureUniqueRefCount) +
		TJS_W(" pixels=") + ttstr(textureTotalPixelCount) +
		TJS_W(" alpha=") + ttstr(textureAlphaNonZeroPixelCount) +
		TJS_W(" red=") + ttstr(textureRedPixelCount) +
		TJS_W(" white=") + ttstr(textureWhitePixelCount) +
		TJS_W(" black=") + ttstr(textureBlackPixelCount);
	bridge.sampledSourcePixelCount = sampledSourcePixelCount;
	bridge.sampledSourceAlphaNonZeroPixelCount = sampledSourceAlphaNonZeroPixelCount;
	bridge.sampledSourceRedPixelCount = sampledSourceRedPixelCount;
	bridge.sampledSourceWhitePixelCount = sampledSourceWhitePixelCount;
	bridge.sampledSourceBlackPixelCount = sampledSourceBlackPixelCount;
	bridge.sampledSourceColorHash = sampledSourceColorHash;
	bridge.sampledUvBoundsValid = sampledUvBoundsValid;
	bridge.sampledUvMinU = sampledUvMinU;
	bridge.sampledUvMaxU = sampledUvMaxU;
	bridge.sampledUvMinV = sampledUvMinV;
	bridge.sampledUvMaxV = sampledUvMaxV;
	bridge.maxCommandCoverageSummary =
		maxCommandCoverageSummary.IsEmpty() ? TJS_W("void") : maxCommandCoverageSummary;
	bridge.maxCommandIRSummary =
		maxCommandIRSummary.IsEmpty() ? TJS_W("void") : maxCommandIRSummary;
	bridge.maxNonBackgroundCommandCoverageSummary =
		maxNonBackgroundCommandCoverageSummary.IsEmpty() ?
			TJS_W("void") : maxNonBackgroundCommandCoverageSummary;
	bridge.maxNonBackgroundCommandIRSummary =
		maxNonBackgroundCommandIRSummary.IsEmpty() ?
			TJS_W("void") : maxNonBackgroundCommandIRSummary;
	bridge.maxBlackCommandCoverageSummary =
		maxBlackCommandCoverageSummary.IsEmpty() ?
			TJS_W("void") : maxBlackCommandCoverageSummary;
	bridge.maxBlackCommandIRSummary =
		maxBlackCommandIRSummary.IsEmpty() ?
			TJS_W("void") : maxBlackCommandIRSummary;
	bridge.sampledSourceColorSummary =
		TJS_W("samples=") + ttstr(sampledSourcePixelCount) +
		TJS_W(" alpha=") + ttstr(sampledSourceAlphaNonZeroPixelCount) +
		TJS_W(" red=") + ttstr(sampledSourceRedPixelCount) +
		TJS_W(" white=") + ttstr(sampledSourceWhitePixelCount) +
		TJS_W(" black=") + ttstr(sampledSourceBlackPixelCount) +
		TJS_W(" uv=") +
		(sampledUvBoundsValid ?
			(ttstr((tjs_int)std::lround(sampledUvMinU * 1000.0)) +
			 TJS_W(",") +
			 ttstr((tjs_int)std::lround(sampledUvMinV * 1000.0)) +
			 TJS_W("-") +
			 ttstr((tjs_int)std::lround(sampledUvMaxU * 1000.0)) +
			 TJS_W(",") +
			 ttstr((tjs_int)std::lround(sampledUvMaxV * 1000.0))) :
			ttstr(TJS_W("void")));
	if (rendererDiagnostics) {
		captureFinalLayerColorStats();
	}
	publishLayerUpdate();

	if (skippedMissingTexture > 0) {
		bridge.mismatchKind = TJS_W("texture-missing");
		bridge.mismatchIndex = firstMissingTextureIndex;
		bridge.mismatchDetails = firstMissingTextureDetails;
	} else if (skippedBadSlice > 0) {
		bridge.mismatchKind = TJS_W("bad-index-slice");
		bridge.mismatchIndex = firstBadIndex;
		bridge.mismatchDetails = firstBadDetails;
	} else if (skippedBadIndex > 0) {
		bridge.mismatchKind = TJS_W("bad-vertex-or-alpha-index");
		bridge.mismatchIndex = firstBadIndex;
		bridge.mismatchDetails = firstBadDetails;
	} else if (offscreenDiag.consumed &&
		rasterTriangleCount != offscreenDiag.consumedTriangleCount) {
		bridge.mismatchKind = TJS_W("visible-triangle-count-mismatch");
		bridge.mismatchIndex = 0;
		bridge.mismatchDetails =
			ttstr(TJS_W("visible:")) + ttstr(rasterTriangleCount) +
			TJS_W(" offscreen:") + ttstr(offscreenDiag.consumedTriangleCount);
	} else if (pixelCount == 0) {
		bridge.mismatchKind = TJS_W("no-coverage");
		bridge.mismatchIndex = 0;
	} else {
		bridge.mismatchKind = TJS_W("none");
		bridge.mismatchIndex = -1;
	}

	if (MotionVerboseLogWanted()) {
		TVPAddLog(ttstr(TJS_W("[emoteplayer] visibleRenderBridge rasterized path=")) + path
			+ TJS_W(" pixels=") + ttstr(pixelCount)
			+ TJS_W(" triangles=") + ttstr(rasterTriangleCount)
			+ TJS_W(" skippedMissingTexture=") + ttstr(skippedMissingTexture)
			+ TJS_W(" skippedBadIndex=") + ttstr(skippedBadIndex)
			+ TJS_W(" alphaPixels=") + ttstr(alphaPixelCount));
	}

	MotionExportVisibleRenderBridgeToJs(bridge, path);

	return bridge;
}

