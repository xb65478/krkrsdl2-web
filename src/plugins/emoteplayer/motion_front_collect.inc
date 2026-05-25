static std::string MotionFrontObjectStateKey(const MotionLayoutInfo &layout);
static int MotionLayoutContextScore(const MotionLayoutInfo &layout,
	const ttstr &contextObject, const ttstr &contextMotion);
static tjs_int MotionFrontResolveContextScore(const MotionLayoutInfo &layout,
	const ttstr &contextObject, const ttstr &contextMotion);

static MotionRendererPrepInfo MotionBuildRendererPrepInfo(
	const MotionResource *resource, tjs_real meshDivisionRatio)
{
	MotionRendererPrepInfo info;
	info.meshDivisionRatio = meshDivisionRatio;
	tjs_int division = (tjs_int)std::ceil(std::max<tjs_real>(1.0, meshDivisionRatio));
	info.meshDivision = std::max<tjs_int>(1, division);
	if (!resource) return info;

	info.layoutCount = (tjs_int)resource->layouts.size();
	for (size_t i = 0; i < resource->layouts.size(); i++) {
		const MotionLayoutInfo &layout = resource->layouts[i];
		info.drawItemCount += std::max<tjs_int>(1, layout.drawCount);
		tjs_real right = layout.left + layout.width;
		tjs_real bottom = layout.top + layout.height;
		if (!info.boundsValid) {
			info.boundsValid = true;
			info.left = layout.left;
			info.top = layout.top;
			info.width = layout.width;
			info.height = layout.height;
		} else {
			tjs_real left = std::min(info.left, layout.left);
			tjs_real top = std::min(info.top, layout.top);
			right = std::max(info.left + info.width, right);
			bottom = std::max(info.top + info.height, bottom);
			info.left = left;
			info.top = top;
			info.width = right - left;
			info.height = bottom - top;
		}
	}
	info.meshPrepared = info.layoutCount > 0;
	info.estimatedQuadCount = info.drawItemCount *
		info.meshDivision * info.meshDivision;
	for (size_t i = 0; i < resource->layouts.size(); i++) {
		const MotionLayoutInfo &layout = resource->layouts[i];
		if (layout.width <= 0.0 || layout.height <= 0.0) continue;
		if (layout.frontDrawCandidate) info.frontDrawItemCount++;
		if (layout.frontHitCandidate) info.frontHitShapeCount++;
		if (layout.frontDrawCandidate &&
			(layout.opacityMin < 0.999 || layout.opacityMax < 0.999)) {
			info.alphaAwareDrawItemCount++;
		}
		if (layout.frontHitCandidate &&
			(layout.opacityMin < 0.999 || layout.opacityMax < 0.999)) {
			info.alphaAwareHitShapeCount++;
		}
	}
	std::set<std::string> drawObjectStates;
	std::set<std::string> hitObjectStates;
	for (size_t i = 0; i < resource->layouts.size(); i++) {
		const MotionLayoutInfo &layout = resource->layouts[i];
		if (layout.width <= 0.0 || layout.height <= 0.0) continue;
		if (layout.frontDrawCandidate) drawObjectStates.insert(
			MotionFrontObjectStateKey(layout));
		if (layout.frontHitCandidate) hitObjectStates.insert(
			MotionFrontObjectStateKey(layout));
	}
	info.frontObjectStateCount = (tjs_int)drawObjectStates.size();
	info.frontHitObjectStateCount = (tjs_int)hitObjectStates.size();
	info.frontMeshCellCount = info.frontDrawItemCount *
		info.meshDivision * info.meshDivision;
	info.estimatedVertexCount = info.estimatedQuadCount * 4;
	info.estimatedIndexCount = info.estimatedQuadCount * 6;
	return info;
}

static bool MotionFrontLayoutPriorityLess(
	const std::pair<const MotionLayoutInfo*, tjs_int> &a,
	const std::pair<const MotionLayoutInfo*, tjs_int> &b)
{
	const MotionLayoutInfo *left = a.first;
	const MotionLayoutInfo *right = b.first;
	if (!left || !right) return left < right;
	if (left->hitInteractive != right->hitInteractive) return left->hitInteractive > right->hitInteractive;
	if (left->hitPriority != right->hitPriority) return left->hitPriority > right->hitPriority;
	if (left->hitLayoutProxy != right->hitLayoutProxy) return !left->hitLayoutProxy;
	if ((left->hitRole == TJS_W("hit-area")) != (right->hitRole == TJS_W("hit-area"))) {
		return right->hitRole == TJS_W("hit-area");
	}
	if (left->hitPathDepth != right->hitPathDepth) return left->hitPathDepth > right->hitPathDepth;
	if (left->opacityMax != right->opacityMax) return left->opacityMax > right->opacityMax;
	return a.second > b.second;
}

static bool MotionHitObjectNameIsGeneric(const ttstr &value)
{
	std::string lower = value.AsLowerCase().AsNarrowStdString();
	return lower.empty() ||
		lower == "area" ||
		lower == "offarea" ||
		lower == "bt" ||
		lower == "bg" ||
		lower == "base" ||
		lower == "on" ||
		lower == "off";
}

static tjs_int MotionFrontResolveSemanticSpecificityScore(
	const MotionLayoutInfo &layout)
{
	tjs_int score = 0;
	if (layout.hitRole == TJS_W("control")) score += 500;
	else if (layout.hitRole == TJS_W("visual")) score += 220;
	else if (layout.hitRole == TJS_W("hit-area")) score -= 80;
	if (layout.hitObjectPath.GetLen() > 0) {
		score += 140;
		if (layout.hitObjectPath.AsNarrowStdString().find('/') != std::string::npos) {
			score += 70;
		}
	}
	if (layout.hitObjectName.GetLen() > 0 &&
		!MotionHitObjectNameIsGeneric(layout.hitObjectName)) {
		score += 120;
	}
	if (!MotionHitObjectNameIsGeneric(layout.objectName)) score += 60;
	if (!MotionHitObjectNameIsGeneric(layout.hitPartName) &&
		layout.hitPartName.AsLowerCase() != TJS_W("area") &&
		layout.hitPartName.AsLowerCase() != TJS_W("offarea")) {
		score += 40;
	}
	if (layout.hitLayoutProxy) score -= 240;
	return score;
}

static bool MotionFrontResolveLayoutPriorityLess(
	const std::pair<const MotionLayoutInfo*, tjs_int> &a,
	const std::pair<const MotionLayoutInfo*, tjs_int> &b,
	const ttstr &contextObject, const ttstr &contextMotion)
{
	if (!a.first || !b.first) return a.first < b.first;
	tjs_int aContext =
		MotionFrontResolveContextScore(*a.first, contextObject, contextMotion);
	tjs_int bContext =
		MotionFrontResolveContextScore(*b.first, contextObject, contextMotion);
	if (aContext != bContext) return aContext > bContext;
	tjs_int aSpecific = MotionFrontResolveSemanticSpecificityScore(*a.first);
	tjs_int bSpecific = MotionFrontResolveSemanticSpecificityScore(*b.first);
	if (aSpecific != bSpecific) return aSpecific > bSpecific;
	return MotionFrontLayoutPriorityLess(a, b);
}

static void MotionResolveSemanticObjectPath(MotionLayoutInfo &layout)
{
	if (layout.hitObjectPath.GetLen() <= 0 &&
		MotionStringHasInteractivePrefix(layout.name)) {
		layout.hitObjectPath = layout.name;
	}
	if (layout.hitObjectPath.GetLen() <= 0 &&
		MotionStringHasInteractivePrefix(layout.objectName)) {
		layout.hitObjectPath = layout.objectName;
	}
	if ((layout.hitStatePath.GetLen() <= 0 ||
			layout.hitStatePath == layout.hitStateName) &&
		layout.hitObjectPath.GetLen() > 0 &&
		layout.hitStateName.GetLen() > 0) {
		layout.hitStatePath = layout.hitObjectPath;
		layout.hitStatePath += TJS_W("/");
		layout.hitStatePath += layout.hitStateName;
	}
}

static std::vector<std::pair<const MotionLayoutInfo*, tjs_int> >
MotionCollectFrontLayouts(const MotionResource *resource, bool hitOnly);

struct MotionFrontObjectStateInfo {
	MotionLayoutInfo layout;
	tjs_int sourceIndex = -1;
	tjs_int sourceCount = 0;
	tjs_int drawItemCount = 0;
	tjs_int hitShapeCount = 0;
	tjs_int contextScore = 0;
};

static std::string MotionFrontObjectStateKey(const MotionLayoutInfo &layout)
{
	std::string key = layout.hitObjectPath.GetLen() > 0 ?
		layout.hitObjectPath.AsNarrowStdString() :
		layout.hitObjectName.AsNarrowStdString();
	key += "\n";
	key += layout.hitStateGroup.GetLen() > 0 ?
		layout.hitStateGroup.AsNarrowStdString() :
		layout.hitStateClass.AsNarrowStdString();
	key += "\n";
	key += layout.contextObjectName.AsNarrowStdString();
	key += "\n";
	key += layout.contextMotionName.AsNarrowStdString();
	return key;
}

static std::vector<MotionFrontObjectStateInfo> MotionCollectFrontObjectStates(
	const MotionResource *resource, bool hitOnly)
{
	std::vector<MotionFrontObjectStateInfo> states;
	std::map<std::string, size_t> indices;
	std::vector<std::pair<const MotionLayoutInfo*, tjs_int> > items =
		MotionCollectFrontLayouts(resource, hitOnly);
	for (size_t i = 0; i < items.size(); i++) {
		const MotionLayoutInfo *layout = items[i].first;
		if (!layout) continue;
		std::string key = MotionFrontObjectStateKey(*layout);
		std::map<std::string, size_t>::iterator found = indices.find(key);
		if (found == indices.end()) {
			MotionFrontObjectStateInfo state;
			state.layout = *layout;
			state.sourceIndex = items[i].second;
			state.sourceCount = 1;
			state.drawItemCount = layout->frontDrawCandidate ? 1 : 0;
			state.hitShapeCount = layout->frontHitCandidate ? 1 : 0;
			indices[key] = states.size();
			states.push_back(state);
			continue;
		}

		MotionFrontObjectStateInfo &state = states[found->second];
		MotionLayoutInfo &merged = state.layout;
		tjs_real right = std::max(merged.left + merged.width,
			layout->left + layout->width);
		tjs_real bottom = std::max(merged.top + merged.height,
			layout->top + layout->height);
		merged.left = std::min(merged.left, layout->left);
		merged.top = std::min(merged.top, layout->top);
		merged.width = right - merged.left;
		merged.height = bottom - merged.top;
		merged.centerX = merged.left + merged.width / 2.0;
		merged.centerY = merged.top + merged.height / 2.0;
		merged.opacityMin = std::min(merged.opacityMin, layout->opacityMin);
		merged.opacityMax = std::max(merged.opacityMax, layout->opacityMax);
		merged.drawCount += layout->drawCount;
		state.sourceCount++;
		if (layout->frontDrawCandidate) state.drawItemCount++;
		if (layout->frontHitCandidate) state.hitShapeCount++;

		std::pair<const MotionLayoutInfo*, tjs_int> current(&merged, state.sourceIndex);
		if (MotionFrontLayoutPriorityLess(items[i], current)) {
			ttstr objectPath = merged.hitObjectPath;
			ttstr stateGroup = merged.hitStateGroup;
			ttstr statePath = merged.hitStatePath;
			tjs_real l = merged.left;
			tjs_real t = merged.top;
			tjs_real w = merged.width;
			tjs_real h = merged.height;
			tjs_real cx = merged.centerX;
			tjs_real cy = merged.centerY;
			tjs_real opacityMin = merged.opacityMin;
			tjs_real opacityMax = merged.opacityMax;
			tjs_int drawCount = merged.drawCount;
			merged = *layout;
			merged.hitObjectPath = objectPath;
			merged.hitStateGroup = stateGroup;
			merged.hitStatePath = statePath;
			merged.left = l;
			merged.top = t;
			merged.width = w;
			merged.height = h;
			merged.centerX = cx;
			merged.centerY = cy;
			merged.opacityMin = opacityMin;
			merged.opacityMax = opacityMax;
			merged.drawCount = drawCount;
			state.sourceIndex = items[i].second;
		}
	}
	std::stable_sort(states.begin(), states.end(),
		[](const MotionFrontObjectStateInfo &a,
			const MotionFrontObjectStateInfo &b) {
			return MotionFrontLayoutPriorityLess(
				std::make_pair(&a.layout, a.sourceIndex),
				std::make_pair(&b.layout, b.sourceIndex));
		});
	return states;
}

static std::vector<MotionFrontObjectStateInfo> MotionCollectPointFrontObjectStates(
	const std::vector<std::pair<const MotionLayoutInfo*, tjs_int> > &matches,
	const ttstr &contextObject, const ttstr &contextMotion)
{
	std::vector<MotionFrontObjectStateInfo> states;
	std::map<std::string, size_t> indices;
	for (size_t i = 0; i < matches.size(); i++) {
		const MotionLayoutInfo *layout = matches[i].first;
		if (!layout) continue;
		std::string key = MotionFrontObjectStateKey(*layout);
		std::map<std::string, size_t>::iterator found = indices.find(key);
		if (found == indices.end()) {
			MotionFrontObjectStateInfo state;
			state.layout = *layout;
			state.sourceIndex = matches[i].second;
			state.sourceCount = 1;
			state.drawItemCount = layout->frontDrawCandidate ? 1 : 0;
			state.hitShapeCount = layout->frontHitCandidate ? 1 : 0;
			state.contextScore =
				MotionFrontResolveContextScore(*layout, contextObject, contextMotion);
			indices[key] = states.size();
			states.push_back(state);
			continue;
		}

		MotionFrontObjectStateInfo &state = states[found->second];
		MotionLayoutInfo &merged = state.layout;
		tjs_real right = std::max(merged.left + merged.width,
			layout->left + layout->width);
		tjs_real bottom = std::max(merged.top + merged.height,
			layout->top + layout->height);
		merged.left = std::min(merged.left, layout->left);
		merged.top = std::min(merged.top, layout->top);
		merged.width = right - merged.left;
		merged.height = bottom - merged.top;
		merged.centerX = merged.left + merged.width / 2.0;
		merged.centerY = merged.top + merged.height / 2.0;
		merged.opacityMin = std::min(merged.opacityMin, layout->opacityMin);
		merged.opacityMax = std::max(merged.opacityMax, layout->opacityMax);
		merged.drawCount += layout->drawCount;
		state.sourceCount++;
		if (layout->frontDrawCandidate) state.drawItemCount++;
		if (layout->frontHitCandidate) state.hitShapeCount++;
		state.contextScore = std::max<tjs_int>(state.contextScore,
			MotionFrontResolveContextScore(*layout, contextObject, contextMotion));

		std::pair<const MotionLayoutInfo*, tjs_int> current(&merged, state.sourceIndex);
		if (MotionFrontResolveLayoutPriorityLess(
			matches[i], current, contextObject, contextMotion)) {
			ttstr objectPath = merged.hitObjectPath;
			ttstr stateGroup = merged.hitStateGroup;
			ttstr statePath = merged.hitStatePath;
			tjs_real l = merged.left;
			tjs_real t = merged.top;
			tjs_real w = merged.width;
			tjs_real h = merged.height;
			tjs_real cx = merged.centerX;
			tjs_real cy = merged.centerY;
			tjs_real opacityMin = merged.opacityMin;
			tjs_real opacityMax = merged.opacityMax;
			tjs_int drawCount = merged.drawCount;
			merged = *layout;
			merged.hitObjectPath = objectPath;
			merged.hitStateGroup = stateGroup;
			merged.hitStatePath = statePath;
			merged.left = l;
			merged.top = t;
			merged.width = w;
			merged.height = h;
			merged.centerX = cx;
			merged.centerY = cy;
			merged.opacityMin = opacityMin;
			merged.opacityMax = opacityMax;
			merged.drawCount = drawCount;
			state.sourceIndex = matches[i].second;
		}
	}
	for (size_t i = 0; i < states.size(); i++) {
		MotionResolveSemanticObjectPath(states[i].layout);
	}
	std::stable_sort(states.begin(), states.end(),
		[&contextObject, &contextMotion](const MotionFrontObjectStateInfo &a,
			const MotionFrontObjectStateInfo &b) {
			if (a.contextScore != b.contextScore) return a.contextScore > b.contextScore;
			tjs_int aSpecific = MotionFrontResolveSemanticSpecificityScore(a.layout);
			tjs_int bSpecific = MotionFrontResolveSemanticSpecificityScore(b.layout);
			if (aSpecific != bSpecific) return aSpecific > bSpecific;
			if (a.hitShapeCount != b.hitShapeCount) return a.hitShapeCount > b.hitShapeCount;
			if (a.sourceCount != b.sourceCount) return a.sourceCount > b.sourceCount;
			return MotionFrontResolveLayoutPriorityLess(
				std::make_pair(&a.layout, a.sourceIndex),
				std::make_pair(&b.layout, b.sourceIndex),
				contextObject, contextMotion);
		});
	return states;
}

static std::vector<std::pair<const MotionLayoutInfo*, tjs_int> >
MotionCollectFrontLayouts(const MotionResource *resource, bool hitOnly)
{
	std::vector<std::pair<const MotionLayoutInfo*, tjs_int> > items;
	if (!resource) return items;
	for (size_t i = 0; i < resource->layouts.size(); i++) {
		const MotionLayoutInfo &layout = resource->layouts[i];
		if (layout.width <= 0.0 || layout.height <= 0.0) continue;
		if (hitOnly) {
			if (!layout.frontHitCandidate) continue;
		} else {
			if (!layout.frontDrawCandidate) continue;
		}
		items.push_back(std::make_pair(&layout, (tjs_int)i));
	}
	std::stable_sort(items.begin(), items.end(), MotionFrontLayoutPriorityLess);
	return items;
}

static std::vector<std::pair<const MotionLayoutInfo*, tjs_int> >
MotionCollectRuntimeDrawLayouts(const MotionResource *resource)
{
	std::vector<std::pair<const MotionLayoutInfo*, tjs_int> > items;
	if (!resource) return items;
	for (size_t i = 0; i < resource->layouts.size(); i++) {
		const MotionLayoutInfo &layout = resource->layouts[i];
		if (!MotionLayoutRenderableForRuntimeDraw(layout)) continue;
		if (!MotionLayoutIsRuntimeTextureLeaf(resource, layout)) continue;
		items.push_back(std::make_pair(&layout, (tjs_int)i));
	}
	if (!items.empty()) {
		std::stable_sort(items.begin(), items.end(),
			[](const std::pair<const MotionLayoutInfo*, tjs_int> &a,
			   const std::pair<const MotionLayoutInfo*, tjs_int> &b) {
				const MotionLayoutInfo *left = a.first;
				const MotionLayoutInfo *right = b.first;
				if (!left || !right) return left < right;
				if (left->renderMethodItemIndex != right->renderMethodItemIndex) {
					return left->renderMethodItemIndex < right->renderMethodItemIndex;
				}
				return a.second < b.second;
			});
		return items;
	}
	for (size_t i = 0; i < resource->layouts.size(); i++) {
		const MotionLayoutInfo &layout = resource->layouts[i];
		if (!MotionLayoutRenderableForRuntimeDraw(layout)) continue;
		items.push_back(std::make_pair(&layout, (tjs_int)i));
	}
	std::stable_sort(items.begin(), items.end(), MotionFrontLayoutPriorityLess);
	return items;
}

