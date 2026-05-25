struct MotionHitRect {
	tjs_real l = 0.0;
	tjs_real t = 0.0;
	tjs_real w = 0.0;
	tjs_real h = 0.0;
	tjs_real x = 0.0;
	tjs_real y = 0.0;
	tjs_int priority = 0;
	bool interactive = false;
	ttstr objectName;
	ttstr stateName;
	ttstr stateClass;
};

class MotionHitShape {
public:
	MotionHitShape() = default;
	MotionHitShape(tjs_real left, tjs_real top, tjs_real width, tjs_real height,
		tjs_real centerX, tjs_real centerY, const ttstr &scope = ttstr())
		: layoutScope(scope)
	{
		addRect(left, top, width, height, centerX, centerY);
	}
	~MotionHitShape()
	{
		if (motionObject) motionObject->Release();
	}
	void setMotionObject(iTJSDispatch2 *object)
	{
		if (object) object->AddRef();
		if (motionObject) motionObject->Release();
		motionObject = object;
	}
	void setLayoutScope(const ttstr &scope) { layoutScope = scope; }
	void setHitSemantics(const MotionLayoutInfo &layout)
	{
		hitObjectName = layout.hitObjectName;
		hitObjectPath = layout.hitObjectPath;
		hitStateName = layout.hitStateName;
		hitStateClass = layout.hitStateClass;
		hitStateGroup = layout.hitStateGroup;
		hitStatePath = layout.hitStatePath;
		hitPriority = layout.hitPriority;
		hitInteractive = layout.hitInteractive;
	}

	bool contains(tjs_real px, tjs_real py) const
	{
		for (size_t i = 0; i < rects.size(); i++) {
			const MotionHitRect &rect = rects[i];
			if (px >= rect.l && py >= rect.t &&
				px < rect.l + rect.w && py < rect.t + rect.h) {
				return true;
			}
		}
		return false;
	}
	static tjs_error TJS_INTF_METHOD containsRaw(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, MotionHitShape *nativeInstance)
	{
		if (!nativeInstance) {
			if (result) *result = tTJSVariant((tjs_int)0);
			return TJS_S_OK;
		}
		if (numparams < 2) return TJS_E_BADPARAMCOUNT;
		bool hit = nativeInstance->contains(
			(tjs_real)*param[0], (tjs_real)*param[1]);
		if (result) *result = tTJSVariant((tjs_int)(hit ? 1 : 0));
		return TJS_S_OK;
	}
	static tjs_error TJS_INTF_METHOD getLayerMotionRaw(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, MotionHitShape *nativeInstance)
	{
		if (!nativeInstance) {
			if (result) result->Clear();
			return TJS_S_OK;
		}
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;
		tTJSVariant getter;
		tjs_error hr = getLayerGetterRaw(&getter, numparams, param, nativeInstance);
		if (TJS_FAILED(hr)) {
			if (result) result->Clear();
			return TJS_S_OK;
		}
		ExtractLayerGetterMotion(getter, result);
		return TJS_S_OK;
	}
	static tjs_error TJS_INTF_METHOD getLayerGetterRaw(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, MotionHitShape *nativeInstance)
	{
		if (!nativeInstance) {
			if (result) result->Clear();
			return TJS_S_OK;
		}
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;
		if (!nativeInstance->motionObject) {
			if (result) result->Clear();
			return TJS_S_OK;
		}
		ttstr layerName(*param[0]);
		tjs_error hr = TJS_S_OK;
		try {
		if (nativeInstance->layoutScope.GetLen() > 0 && layerName.GetLen() > 0) {
			layerName = nativeInstance->layoutScope + TJS_W("/") + layerName;
			tTJSVariant scopedParam(layerName);
			tTJSVariant *argv[] = { &scopedParam };
			hr = nativeInstance->motionObject->FuncCall(
				0, TJS_W("getLayerGetter"), nullptr, result,
				1, argv, nativeInstance->motionObject);
			if (TJS_SUCCEEDED(hr) && result && result->Type() == tvtVoid) {
				tTJSVariant leafParam(*param[0]);
				tTJSVariant *leafArgv[] = { &leafParam };
				hr = nativeInstance->motionObject->FuncCall(
					0, TJS_W("getLayerGetter"), nullptr, result,
					1, leafArgv, nativeInstance->motionObject);
			}
		} else {
			hr = nativeInstance->motionObject->FuncCall(
				0, TJS_W("getLayerGetter"), nullptr, result,
				numparams, param, nativeInstance->motionObject);
		}
		} catch (...) {
			hr = TJS_E_FAIL;
		}
		if (TJS_FAILED(hr) && result) result->Clear();
		return TJS_S_OK;
	}

	tjs_int get_type() const { return ShapeTypeRect; }
	tjs_real get_x() const { return rects.empty() ? 0.0 : rects[0].x; }
	tjs_real get_y() const { return rects.empty() ? 0.0 : rects[0].y; }
	tjs_real get_l() const { return rects.empty() ? 0.0 : rects[0].l; }
	tjs_real get_t() const { return rects.empty() ? 0.0 : rects[0].t; }
	tjs_real get_w() const { return rects.empty() ? 0.0 : rects[0].w; }
	tjs_real get_h() const { return rects.empty() ? 0.0 : rects[0].h; }
	tTJSString get_objectName() const { return hitObjectName; }
	tTJSString get_objectPath() const { return hitObjectPath; }
	tTJSString get_stateName() const { return hitStateName; }
	tTJSString get_stateClass() const { return hitStateClass; }
	tTJSString get_stateGroup() const { return hitStateGroup; }
	tTJSString get_statePath() const { return hitStatePath; }
	tjs_int get_hitPriority() const { return hitPriority; }
	tjs_int get_interactive() const { return hitInteractive ? 1 : 0; }

	void addRect(tjs_real left, tjs_real top, tjs_real width, tjs_real height,
		tjs_real centerX, tjs_real centerY, const MotionLayoutInfo *layout = nullptr)
	{
		if (width <= 0.0 || height <= 0.0) return;
		MotionHitRect rect;
		rect.l = left;
		rect.t = top;
		rect.w = width;
		rect.h = height;
		rect.x = centerX;
		rect.y = centerY;
		if (layout) {
			rect.priority = layout->hitPriority;
			rect.interactive = layout->hitInteractive;
			rect.objectName = layout->hitObjectName;
			rect.stateName = layout->hitStateName;
			rect.stateClass = layout->hitStateClass;
		}
		rects.push_back(rect);
	}

private:
	std::vector<MotionHitRect> rects;
	iTJSDispatch2 *motionObject = nullptr;
	ttstr layoutScope;
	ttstr hitObjectName;
	ttstr hitObjectPath;
	ttstr hitStateName;
	ttstr hitStateClass;
	ttstr hitStateGroup;
	ttstr hitStatePath;
	tjs_int hitPriority = 0;
	bool hitInteractive = false;
};

static tTJSVariant MakeMotionHitShape(tjs_real left, tjs_real top,
	tjs_real width, tjs_real height, tjs_real centerX, tjs_real centerY,
	iTJSDispatch2 *motionObject = nullptr, const ttstr &layoutScope = ttstr())
{
	MotionHitShape *shape = new MotionHitShape(left, top, width, height,
		centerX, centerY, layoutScope);
	shape->setMotionObject(motionObject);
	iTJSDispatch2 *object = ncbInstanceAdaptor<MotionHitShape>::CreateAdaptor(shape);
	if (!object) {
		delete shape;
		TVPAddLog(TJS_W("[emoteplayer] failed to create MotionHitShape adaptor"));
		return tTJSVariant();
	}
	tTJSVariant result(object, object);
	object->Release();
	return result;
}

static tTJSVariant MakeMotionHitShape(const std::vector<const MotionLayoutInfo*> &layouts,
	const MotionLayoutInfo &fallback, iTJSDispatch2 *motionObject = nullptr)
{
	MotionHitShape *shape = new MotionHitShape();
	shape->setMotionObject(motionObject);
	shape->setLayoutScope(fallback.name);
	shape->setHitSemantics(fallback);
	for (size_t i = 0; i < layouts.size(); i++) {
		const MotionLayoutInfo *layout = layouts[i];
		if (!layout) continue;
		shape->addRect(layout->left, layout->top, layout->width, layout->height,
			layout->centerX, layout->centerY, layout);
	}
	if (layouts.empty()) {
		shape->addRect(fallback.left, fallback.top, fallback.width, fallback.height,
			fallback.centerX, fallback.centerY, &fallback);
	}
	iTJSDispatch2 *object = ncbInstanceAdaptor<MotionHitShape>::CreateAdaptor(shape);
	if (!object) {
		delete shape;
		TVPAddLog(TJS_W("[emoteplayer] failed to create MotionHitShape adaptor"));
		return tTJSVariant();
	}
	tTJSVariant result(object, object);
	object->Release();
	return result;
}

static bool MotionNameMatches(const ttstr &a, const ttstr &b)
{
	if (a == b) return true;
	return a.AsLowerCase() == b.AsLowerCase();
}

static bool MotionLayoutMatchesName(const MotionLayoutInfo &layout, const ttstr &name)
{
	return MotionNameMatches(layout.name, name) ||
		MotionNameMatches(layout.objectName, name);
}

static bool MotionLayoutNameHasPrefix(const ttstr &layoutName, const ttstr &prefix)
{
	std::string layout = layoutName.AsNarrowStdString();
	std::string prefixText = prefix.AsNarrowStdString();
	return layout.size() > prefixText.size() &&
		layout.compare(0, prefixText.size(), prefixText) == 0 &&
		layout[prefixText.size()] == '/';
}

static bool MotionLayoutNameMatchesScopedDescendantLeaf(const ttstr &layoutName,
	const ttstr &scopeName, const ttstr &leafName)
{
	if (scopeName.GetLen() <= 0 || leafName.GetLen() <= 0) return false;
	std::string layout = MotionLowerString(layoutName.AsNarrowStdString());
	std::string scope = MotionLowerString(scopeName.AsNarrowStdString());
	std::string leaf = MotionLowerString(leafName.AsNarrowStdString());
	if (layout.size() <= scope.size() + leaf.size() + 1) return false;
	if (layout.compare(0, scope.size(), scope) != 0 ||
		layout[scope.size()] != '/') {
		return false;
	}
	size_t leafStart = layout.size() - leaf.size();
	return leafStart > scope.size() + 1 &&
		layout[leafStart - 1] == '/' &&
		layout.compare(leafStart, leaf.size(), leaf) == 0;
}

static bool MotionLayoutMatchesScopedDescendantLeaf(const MotionLayoutInfo &layout,
	const ttstr &scopeName, const ttstr &leafName)
{
	if (MotionLayoutNameMatchesScopedDescendantLeaf(
		layout.name, scopeName, leafName)) {
		return true;
	}
	if (MotionLayoutNameMatchesScopedDescendantLeaf(
		layout.objectName, scopeName, leafName)) {
		return true;
	}
	return false;
}

static bool MotionSplitScopedLeafName(const ttstr &layerName,
	ttstr &scopeName, ttstr &leafName)
{
	std::string name = layerName.AsNarrowStdString();
	size_t slash = name.rfind('/');
	if (slash == std::string::npos || slash == 0 || slash + 1 >= name.size()) {
		return false;
	}
	scopeName = MotionString(name.substr(0, slash));
	leafName = MotionString(name.substr(slash + 1));
	return scopeName.GetLen() > 0 && leafName.GetLen() > 0;
}

static void CollectMotionScopedDescendantLeafLayouts(
	const MotionResource &resource, const ttstr &scopeName, const ttstr &leafName,
	const ttstr &contextObject, const ttstr &contextMotion,
	std::vector<const MotionLayoutInfo*> &layouts)
{
	for (size_t i = 0; i < resource.layouts.size(); i++) {
		const MotionLayoutInfo &layout = resource.layouts[i];
		if (layout.width <= 0.0 || layout.height <= 0.0) continue;
		if (!MotionLayoutMatchesScopedDescendantLeaf(layout, scopeName, leafName)) {
			continue;
		}
		if (contextObject.GetLen() > 0 &&
			layout.contextObjectName.GetLen() > 0 &&
			!MotionNameMatches(layout.contextObjectName, contextObject)) {
			continue;
		}
		if (contextMotion.GetLen() > 0 &&
			layout.contextMotionName.GetLen() > 0 &&
			!MotionNameMatches(layout.contextMotionName, contextMotion)) {
			continue;
		}
		layouts.push_back(&layout);
	}
}

static void SortMotionScopedDescendantLeafLayouts(
	std::vector<const MotionLayoutInfo*> &layouts)
{
	std::stable_sort(layouts.begin(), layouts.end(),
		[](const MotionLayoutInfo *left, const MotionLayoutInfo *right) {
			if (!left || !right) return left < right;
			return MotionFrontLayoutPriorityLess(
				std::make_pair(left, (tjs_int)0),
				std::make_pair(right, (tjs_int)0));
		});
}

static bool MotionLayoutPathHasSegment(const ttstr &layoutName, const ttstr &segmentName)
{
	if (segmentName.GetLen() <= 0) return false;
	std::vector<std::string> parts = MotionSplitPath(layoutName.AsNarrowStdString());
	std::string segment = MotionLowerString(segmentName.AsNarrowStdString());
	if (segment.empty()) return false;
	for (size_t i = 0; i < parts.size(); i++) {
		if (MotionLowerString(parts[i]) == segment) return true;
	}
	return false;
}

static bool MotionLayoutMatchesDescendantName(const MotionLayoutInfo &layout,
	const ttstr &name)
{
	if (name.GetLen() <= 0) return false;
	if (MotionLayoutNameHasPrefix(layout.name, name)) return true;
	if (MotionStringHasInteractivePrefix(name) &&
		(MotionLayoutPathHasSegment(layout.name, name) ||
		 MotionLayoutPathHasSegment(layout.objectName, name))) {
		return true;
	}

	std::string narrowName = name.AsNarrowStdString();
	if (!MotionStartsWith(narrowName, "slot/")) {
		ttstr slotScoped = TJS_W("slot/");
		slotScoped += name;
		if (MotionLayoutNameHasPrefix(layout.name, slotScoped)) return true;
	}
	if (!MotionStartsWith(narrowName, "page/")) {
		ttstr pageScoped = TJS_W("page/");
		pageScoped += name;
		if (MotionLayoutNameHasPrefix(layout.name, pageScoped)) return true;
	}
	return false;
}

static int MotionLayoutContextScore(const MotionLayoutInfo &layout,
	const ttstr &contextObject, const ttstr &contextMotion)
{
	int score = 0;
	if (contextObject.GetLen() > 0) {
		if (MotionNameMatches(layout.contextObjectName, contextObject)) score += 1000;
		else if (layout.contextObjectName.GetLen() > 0) score -= 100;
	}
	if (contextMotion.GetLen() > 0) {
		if (MotionNameMatches(layout.contextMotionName, contextMotion)) score += 500;
		else if (layout.contextMotionName.GetLen() > 0) score -= 20;
	}
	return score;
}

static bool BuildMotionGroupLayout(const MotionResource &resource,
	const ttstr &name, const ttstr &contextObject, const ttstr &contextMotion,
	MotionLayoutInfo &group, std::vector<const MotionLayoutInfo*> *hitLayouts = nullptr)
{
	MotionLayoutBounds bounds;
	int bestScore = -1000000;
	const MotionLayoutInfo *best = nullptr;
	if (hitLayouts) hitLayouts->clear();
	for (size_t i = 0; i < resource.layouts.size(); i++) {
		const MotionLayoutInfo &layout = resource.layouts[i];
		if (!MotionLayoutMatchesDescendantName(layout, name)) continue;
		if (layout.width <= 0.0 || layout.height <= 0.0) continue;
		int score = MotionLayoutContextScore(layout, contextObject, contextMotion);
		if (contextObject.GetLen() > 0 &&
			layout.contextObjectName.GetLen() > 0 &&
			!MotionNameMatches(layout.contextObjectName, contextObject)) {
			continue;
		}
		if (contextMotion.GetLen() > 0 &&
			layout.contextMotionName.GetLen() > 0 &&
			!MotionNameMatches(layout.contextMotionName, contextMotion)) {
			continue;
		}
		MotionAddLayoutBounds(bounds, layout.left, layout.top, layout.width, layout.height,
			layout.opacityMax);
		if (hitLayouts) hitLayouts->push_back(&layout);
		if (!best || score > bestScore) {
			best = &layout;
			bestScore = score;
		}
	}

	if (!bounds.valid) return false;
	group = MotionLayoutInfo();
	group.name = name;
	group.objectName = name;
	group.motionName = best ? best->motionName : ttstr();
	group.contextObjectName = best ? best->contextObjectName : contextObject;
	group.contextMotionName = best ? best->contextMotionName : contextMotion;
	group.hitObjectName = best ? best->hitObjectName : name;
	group.hitStateName = best ? best->hitStateName : ttstr();
	group.hitStateClass = best ? best->hitStateClass : ttstr();
	group.left = bounds.minX;
	group.top = bounds.minY;
	group.width = bounds.maxX - bounds.minX;
	group.height = bounds.maxY - bounds.minY;
	group.centerX = group.left + group.width / 2.0;
	group.centerY = group.top + group.height / 2.0;
	if (bounds.opacityValid) {
		group.opacityMin = bounds.opacityMin;
		group.opacityMax = bounds.opacityMax;
	}
	group.drawCount = bounds.drawCount;
	group.hitPriority = best ? best->hitPriority : 0;
	group.hitInteractive = best ? best->hitInteractive : false;
	if (group.hitStateClass.GetLen() <= 0) MotionFinalizeLayoutHitSemantics(group);
	return true;
}

static const MotionLayoutInfo *FindMotionLayoutByName(const MotionResource &resource,
	const ttstr &name, const ttstr &contextObject, const ttstr &contextMotion)
{
	const MotionLayoutInfo *best = nullptr;
	int bestScore = -1000000;
	for (size_t i = 0; i < resource.layouts.size(); i++) {
		const MotionLayoutInfo &layout = resource.layouts[i];
		if (!MotionLayoutMatchesName(layout, name)) continue;
		int score = 0;
		if (MotionNameMatches(layout.name, name)) score += 10000;
		else if (MotionNameMatches(layout.objectName, name)) score += 100;
		score += MotionLayoutContextScore(layout, contextObject, contextMotion);
		if (layout.contextObjectName.GetLen() > 0 &&
			!MotionNameMatches(layout.contextObjectName, layout.objectName)) {
			score += 100;
		}
		if (resource.width > 0 && resource.height > 0) {
			bool intersectsCanvas = layout.left < resource.width &&
				layout.top < resource.height &&
				layout.left + layout.width > 0.0 &&
				layout.top + layout.height > 0.0;
			if (intersectsCanvas) score += 200;
			else score -= 1000;
		}
		if (layout.motionName.GetLen() > 0) score += 10;
		tjs_real area = layout.width * layout.height;
		if (area > 0.0 && resource.width > 0 && resource.height > 0 &&
			area < (tjs_real)resource.width * (tjs_real)resource.height) {
			score += 5;
		}
		if (!best || score > bestScore) {
			best = &layout;
			bestScore = score;
		}
	}
	return best;
}

static const MotionLayoutInfo *FindMotionLayout(const MotionResource &resource,
	const ttstr &name, const ttstr &contextObject = ttstr(),
	const ttstr &contextMotion = ttstr())
{
	const MotionLayoutInfo *layout =
		FindMotionLayoutByName(resource, name, contextObject, contextMotion);
	if (layout) return layout;
	return nullptr;
}

static void CollectMotionLayoutsByName(const MotionResource &resource,
	const ttstr &name, std::vector<const MotionLayoutInfo*> &layouts)
{
	for (size_t i = 0; i < resource.layouts.size(); i++) {
		const MotionLayoutInfo &layout = resource.layouts[i];
		if (!MotionLayoutMatchesName(layout, name)) continue;
		if (layout.width <= 0.0 || layout.height <= 0.0) continue;
		layouts.push_back(&layout);
	}
}

static void CollectMotionDescendantLayoutsByName(const MotionResource &resource,
	const ttstr &name, std::vector<const MotionLayoutInfo*> &layouts)
{
	for (size_t i = 0; i < resource.layouts.size(); i++) {
		const MotionLayoutInfo &layout = resource.layouts[i];
		if (!MotionLayoutMatchesDescendantName(layout, name)) continue;
		if (layout.width <= 0.0 || layout.height <= 0.0) continue;
		layouts.push_back(&layout);
	}
}

static std::vector<const MotionLayoutInfo*> CollectMotionHitLayouts(
	const MotionResource &resource, const ttstr &name)
{
	std::vector<const MotionLayoutInfo*> layouts;
	CollectMotionLayoutsByName(resource, name, layouts);
	CollectMotionDescendantLayoutsByName(resource, name, layouts);
	return layouts;
}

static bool MotionLayoutContains(const MotionLayoutInfo &layout, tjs_real x, tjs_real y);

static bool MotionLayoutAnyContains(const MotionResource &resource,
	const ttstr &name, const MotionLayoutInfo &fallback, tjs_real x, tjs_real y)
{
	std::vector<const MotionLayoutInfo*> layouts = CollectMotionHitLayouts(resource, name);
	for (size_t i = 0; i < layouts.size(); i++) {
		if (layouts[i] && MotionLayoutContains(*layouts[i], x, y)) return true;
	}
	return MotionLayoutContains(fallback, x, y);
}

static bool MotionLayoutAnyContains(
	const std::vector<const MotionLayoutInfo*> &layouts,
	const MotionLayoutInfo &fallback, tjs_real x, tjs_real y)
{
	for (size_t i = 0; i < layouts.size(); i++) {
		if (layouts[i] && MotionLayoutContains(*layouts[i], x, y)) return true;
	}
	return MotionLayoutContains(fallback, x, y);
}

static bool MotionLayoutContains(const MotionLayoutInfo &layout, tjs_real x, tjs_real y)
{
	return x >= layout.left && y >= layout.top &&
		x < layout.left + layout.width && y < layout.top + layout.height;
}

static tjs_int MotionFrontResolveContextScore(const MotionLayoutInfo &layout,
	const ttstr &contextObject, const ttstr &contextMotion)
{
	tjs_int score = MotionLayoutContextScore(layout, contextObject, contextMotion);
	if (contextMotion.GetLen() > 0 && layout.motionName.GetLen() > 0 &&
		MotionNameMatches(layout.motionName, contextMotion)) {
		score += 40;
	}
	return score;
}

static tTJSVariant MakeMotionFrontResolveDump(const MotionResource *resource,
	const ttstr &contextObject, const ttstr &contextMotion,
	tjs_real x, tjs_real y, bool playerVisible, tjs_int playerOpacity,
	tjs_real playerWidth, tjs_real playerHeight)
{
	iTJSDispatch2 *dict = TJSCreateDictionaryObject();
	SetTjsProperty(dict, TJS_W("backend"),
		tTJSVariant(MotionRendererBackendName(g_motionRendererBackend)));
	SetTjsProperty(dict, TJS_W("rendererStage"), tTJSVariant(TJS_W("front-hit-resolve")));
	SetTjsProperty(dict, TJS_W("x"), tTJSVariant(x));
	SetTjsProperty(dict, TJS_W("y"), tTJSVariant(y));
	SetTjsProperty(dict, TJS_W("resourceValid"),
		tTJSVariant((tjs_int)(resource && resource->opened ? 1 : 0)));
	SetTjsProperty(dict, TJS_W("resourceParsed"),
		tTJSVariant((tjs_int)(resource && resource->parsed ? 1 : 0)));
	SetTjsProperty(dict, TJS_W("resourceStorage"),
		tTJSVariant(resource ? resource->storage : ttstr()));
	SetTjsProperty(dict, TJS_W("contextObjectName"), tTJSVariant(contextObject));
	SetTjsProperty(dict, TJS_W("contextMotionName"), tTJSVariant(contextMotion));
	SetTjsProperty(dict, TJS_W("playerVisible"), tTJSVariant((tjs_int)(playerVisible ? 1 : 0)));
	SetTjsProperty(dict, TJS_W("playerOpacity"), tTJSVariant(playerOpacity));
	SetTjsProperty(dict, TJS_W("playerWidth"), tTJSVariant(playerWidth));
	SetTjsProperty(dict, TJS_W("playerHeight"), tTJSVariant(playerHeight));
	bool pointInPlayerBounds =
		x >= 0.0 && y >= 0.0 && x < playerWidth && y < playerHeight;
	bool playerHitEnabled = playerVisible && playerOpacity > 0;
	SetTjsProperty(dict, TJS_W("pointInPlayerBounds"),
		tTJSVariant((tjs_int)(pointInPlayerBounds ? 1 : 0)));
	SetTjsProperty(dict, TJS_W("playerHitEnabled"),
		tTJSVariant((tjs_int)(playerHitEnabled ? 1 : 0)));

	std::vector<std::pair<const MotionLayoutInfo*, tjs_int> > items =
		MotionCollectFrontLayouts(resource, true);
	SetTjsProperty(dict, TJS_W("frontHitShapeCount"), tTJSVariant((tjs_int)items.size()));
	SetTjsProperty(dict, TJS_W("frontHitObjectStateCount"),
		tTJSVariant((tjs_int)MotionCollectFrontObjectStates(resource, true).size()));

	std::vector<std::pair<const MotionLayoutInfo*, tjs_int> > matches;
	std::vector<std::pair<const MotionLayoutInfo*, tjs_int> > drawMatches;
	std::vector<MotionFrontObjectStateInfo> pointHitStates;
	std::vector<MotionFrontObjectStateInfo> pointDrawStates;
	if (playerHitEnabled && pointInPlayerBounds) {
		for (size_t i = 0; i < items.size(); i++) {
			const MotionLayoutInfo *layout = items[i].first;
			if (layout && MotionLayoutContains(*layout, x, y)) {
				matches.push_back(items[i]);
			}
		}
		std::stable_sort(matches.begin(), matches.end(),
			[&contextObject, &contextMotion](
				const std::pair<const MotionLayoutInfo*, tjs_int> &a,
				const std::pair<const MotionLayoutInfo*, tjs_int> &b) {
				if (!a.first || !b.first) return a.first < b.first;
				tjs_int aContext =
					MotionFrontResolveContextScore(*a.first, contextObject, contextMotion);
				tjs_int bContext =
					MotionFrontResolveContextScore(*b.first, contextObject, contextMotion);
				if (aContext != bContext) return aContext > bContext;
				return MotionFrontLayoutPriorityLess(a, b);
			});
		pointHitStates =
			MotionCollectPointFrontObjectStates(matches, contextObject, contextMotion);

		std::vector<std::pair<const MotionLayoutInfo*, tjs_int> > drawItems =
			MotionCollectFrontLayouts(resource, false);
		for (size_t i = 0; i < drawItems.size(); i++) {
			const MotionLayoutInfo *layout = drawItems[i].first;
			if (layout && MotionLayoutContains(*layout, x, y)) {
				drawMatches.push_back(drawItems[i]);
			}
		}
		std::stable_sort(drawMatches.begin(), drawMatches.end(),
			[&contextObject, &contextMotion](
				const std::pair<const MotionLayoutInfo*, tjs_int> &a,
				const std::pair<const MotionLayoutInfo*, tjs_int> &b) {
				return MotionFrontResolveLayoutPriorityLess(
					a, b, contextObject, contextMotion);
			});
		pointDrawStates =
			MotionCollectPointFrontObjectStates(drawMatches, contextObject, contextMotion);
	}

	SetTjsProperty(dict, TJS_W("hit"), tTJSVariant((tjs_int)(!matches.empty() ? 1 : 0)));
	SetTjsProperty(dict, TJS_W("overlapCount"), tTJSVariant((tjs_int)matches.size()));
	SetTjsProperty(dict, TJS_W("candidateCount"), tTJSVariant((tjs_int)matches.size()));
	SetTjsProperty(dict, TJS_W("occludedCount"),
		tTJSVariant((tjs_int)(matches.size() > 0 ? matches.size() - 1 : 0)));
	SetTjsProperty(dict, TJS_W("pointHitObjectStateCount"),
		tTJSVariant((tjs_int)pointHitStates.size()));
	SetTjsProperty(dict, TJS_W("pointDrawObjectStateCount"),
		tTJSVariant((tjs_int)pointDrawStates.size()));
	SetTjsProperty(dict, TJS_W("pointHitObjectStates"),
		MakeMotionFrontObjectStateDumpArray(pointHitStates,
			TJS_W("point-object-state-hit")));
	SetTjsProperty(dict, TJS_W("pointDrawObjectStates"),
		MakeMotionFrontObjectStateDumpArray(pointDrawStates,
			TJS_W("point-object-state-draw")));

	const tjs_int sampleLimit = 6;
	iTJSDispatch2 *array = TJSCreateArrayObject();
	tjs_int sampleCount =
		std::min<tjs_int>(sampleLimit, (tjs_int)matches.size());
	for (tjs_int i = 0; i < sampleCount; i++) {
		const MotionLayoutInfo *layout = matches[(size_t)i].first;
		if (!layout) continue;
		iTJSDispatch2 *candidate = TJSCreateDictionaryObject();
		SetTjsProperty(candidate, TJS_W("rank"), tTJSVariant(i));
		SetTjsProperty(candidate, TJS_W("contextScore"),
			tTJSVariant(MotionFrontResolveContextScore(
				*layout, contextObject, contextMotion)));
		SetTjsProperty(candidate, TJS_W("containsPoint"), tTJSVariant((tjs_int)1));
		SetMotionFrontLayoutDumpFields(candidate, *layout, matches[(size_t)i].second);
		tTJSVariant value(candidate, candidate);
		candidate->Release();
		array->PropSetByNum(TJS_MEMBERENSURE, i, &value, array);
	}
	tTJSVariant candidates(array, array);
	array->Release();
	SetTjsProperty(dict, TJS_W("candidates"), candidates);
	SetTjsProperty(dict, TJS_W("candidateSampleLimit"), tTJSVariant(sampleLimit));
	SetTjsProperty(dict, TJS_W("candidateSampleCount"), tTJSVariant(sampleCount));
	SetTjsProperty(dict, TJS_W("candidateListTruncated"),
		tTJSVariant((tjs_int)(matches.size() > (size_t)sampleLimit ? 1 : 0)));

	const MotionLayoutInfo *semanticLayout = nullptr;
	tjs_int semanticSourceIndex = -1;
	const tjs_char *semanticMode = TJS_W("none");
	tjs_int semanticSourceCount = 0;
	tjs_int semanticHitShapeCount = 0;
	tjs_int semanticDrawItemCount = 0;
	if (!pointHitStates.empty()) {
		semanticLayout = &pointHitStates[0].layout;
		semanticSourceIndex = pointHitStates[0].sourceIndex;
		semanticMode = TJS_W("hit-object-state");
		semanticSourceCount = pointHitStates[0].sourceCount;
		semanticHitShapeCount = pointHitStates[0].hitShapeCount;
		semanticDrawItemCount = pointHitStates[0].drawItemCount;
	}
	if (!pointDrawStates.empty()) {
		bool preferDraw = !semanticLayout ||
			(MotionFrontResolveSemanticSpecificityScore(pointDrawStates[0].layout) >
				MotionFrontResolveSemanticSpecificityScore(*semanticLayout) + 40);
		if (preferDraw) {
			semanticLayout = &pointDrawStates[0].layout;
			semanticSourceIndex = pointDrawStates[0].sourceIndex;
			semanticMode = TJS_W("draw-object-state");
			semanticSourceCount = pointDrawStates[0].sourceCount;
			semanticHitShapeCount = pointDrawStates[0].hitShapeCount;
			semanticDrawItemCount = pointDrawStates[0].drawItemCount;
		}
	}
	const tjs_char *semanticSelection = TJS_W("primary");
	if (semanticLayout && semanticLayout->hitRole == TJS_W("hit-area") &&
		MotionHitObjectNameIsGeneric(semanticLayout->hitObjectName)) {
		const MotionLayoutInfo *upgradeLayout = nullptr;
		const MotionFrontObjectStateInfo *upgradeState = nullptr;
		const tjs_char *upgradeMode = nullptr;
		for (size_t i = 0; i < pointHitStates.size(); i++) {
			const MotionLayoutInfo &cand = pointHitStates[i].layout;
			if (cand.hitRole != TJS_W("hit-area") &&
				!MotionHitObjectNameIsGeneric(cand.hitObjectName)) {
				upgradeLayout = &cand;
				upgradeState = &pointHitStates[i];
				upgradeMode = TJS_W("hit-object-state");
				break;
			}
		}
		if (!upgradeLayout) {
			for (size_t i = 0; i < pointDrawStates.size(); i++) {
				const MotionLayoutInfo &cand = pointDrawStates[i].layout;
				if (cand.hitRole != TJS_W("hit-area") &&
					!MotionHitObjectNameIsGeneric(cand.hitObjectName)) {
					upgradeLayout = &cand;
					upgradeState = &pointDrawStates[i];
					upgradeMode = TJS_W("draw-object-state");
					break;
				}
			}
		}
		if (upgradeLayout && upgradeState) {
			semanticLayout = upgradeLayout;
			semanticSourceIndex = upgradeState->sourceIndex;
			semanticMode = upgradeMode;
			semanticSourceCount = upgradeState->sourceCount;
			semanticHitShapeCount = upgradeState->hitShapeCount;
			semanticDrawItemCount = upgradeState->drawItemCount;
			semanticSelection = TJS_W("upgraded-from-generic-hit-area");
		}
	}
	if (!semanticLayout && !matches.empty()) {
		semanticLayout = matches[0].first;
		semanticSourceIndex = matches[0].second;
		semanticMode = TJS_W("layout-hit-shape");
		semanticSelection = TJS_W("fallback-raw-match");
	}
	if (!matches.empty() && matches[0].first) {
		const MotionLayoutInfo &layout = *matches[0].first;
		SetTjsProperty(dict, TJS_W("contextScore"),
			tTJSVariant(MotionFrontResolveContextScore(
				layout, contextObject, contextMotion)));
		SetMotionFrontLayoutDumpFields(dict, layout, matches[0].second);
	}
	if (semanticLayout) {
		const MotionLayoutInfo &layout = *semanticLayout;
		SetMotionFrontSemanticResolveDumpFields(dict, layout, semanticSourceIndex,
			semanticMode, semanticSourceCount, semanticHitShapeCount,
			semanticDrawItemCount, contextObject, contextMotion);
		SetTjsProperty(dict, TJS_W("semanticSelection"), tTJSVariant(semanticSelection));
	} else {
		SetTjsProperty(dict, TJS_W("semanticResolveMode"), tTJSVariant(TJS_W("none")));
	}

	tTJSVariant result(dict, dict);
	dict->Release();
	return result;
}

static bool MotionResourceAnyLayoutContains(const MotionResource &resource,
	tjs_real x, tjs_real y, bool *hadLayout = nullptr)
{
	bool any = false;
	for (size_t i = 0; i < resource.layouts.size(); i++) {
		const MotionLayoutInfo &layout = resource.layouts[i];
		if (layout.width <= 0.0 || layout.height <= 0.0) continue;
		any = true;
		if (MotionLayoutContains(layout, x, y)) {
			if (hadLayout) *hadLayout = true;
			return true;
		}
	}
	if (hadLayout) *hadLayout = any;
	return false;
}

static bool MotionResourceLayoutsContain(const MotionResource &resource,
	const ttstr &contextObject, const ttstr &contextMotion, tjs_real x, tjs_real y,
	bool *hadCandidate = nullptr)
{
	bool any = false;
	for (size_t i = 0; i < resource.layouts.size(); i++) {
		const MotionLayoutInfo &layout = resource.layouts[i];
		if (layout.width <= 0.0 || layout.height <= 0.0) continue;
		if (contextObject.GetLen() > 0 &&
			layout.contextObjectName.GetLen() > 0 &&
			!MotionNameMatches(layout.contextObjectName, contextObject)) {
			continue;
		}
		if (contextMotion.GetLen() > 0 &&
			layout.contextMotionName.GetLen() > 0 &&
			!MotionNameMatches(layout.contextMotionName, contextMotion)) {
			continue;
		}
		any = true;
		if (MotionLayoutContains(layout, x, y)) {
			if (hadCandidate) *hadCandidate = true;
			return true;
		}
	}
	if (hadCandidate) *hadCandidate = any;
	return false;
}

static tTJSVariant MakeMotionLayoutLayerGetter(const MotionLayoutInfo &layout,
	const std::vector<const MotionLayoutInfo*> &hitLayouts =
		std::vector<const MotionLayoutInfo*>(),
	iTJSDispatch2 *motionObject = nullptr)
{
	iTJSDispatch2 *dict = TJSCreateDictionaryObject();
	tTJSVariant x(layout.centerX), y(layout.centerY);
	tTJSVariant l(layout.left), t(layout.top);
	tTJSVariant w(layout.width), h(layout.height);
	dict->PropSet(TJS_MEMBERENSURE, TJS_W("x"), nullptr, &x, dict);
	dict->PropSet(TJS_MEMBERENSURE, TJS_W("y"), nullptr, &y, dict);
	dict->PropSet(TJS_MEMBERENSURE, TJS_W("left"), nullptr, &l, dict);
	dict->PropSet(TJS_MEMBERENSURE, TJS_W("top"), nullptr, &t, dict);
	dict->PropSet(TJS_MEMBERENSURE, TJS_W("width"), nullptr, &w, dict);
	dict->PropSet(TJS_MEMBERENSURE, TJS_W("height"), nullptr, &h, dict);
	SetTjsProperty(dict, TJS_W("visible"), tTJSVariant(true));
	SetTjsProperty(dict, TJS_W("originX"), tTJSVariant((tjs_real)0.0));
	SetTjsProperty(dict, TJS_W("originY"), tTJSVariant((tjs_real)0.0));
	SetTjsProperty(dict, TJS_W("flipX"), tTJSVariant(false));
	SetTjsProperty(dict, TJS_W("flipY"), tTJSVariant(false));
	SetTjsProperty(dict, TJS_W("zoomX"), tTJSVariant((tjs_real)1.0));
	SetTjsProperty(dict, TJS_W("zoomY"), tTJSVariant((tjs_real)1.0));
	SetTjsProperty(dict, TJS_W("slantX"), tTJSVariant((tjs_real)0.0));
	SetTjsProperty(dict, TJS_W("slantY"), tTJSVariant((tjs_real)0.0));
	SetTjsProperty(dict, TJS_W("angleDeg"), tTJSVariant((tjs_real)0.0));
	SetTjsProperty(dict, TJS_W("opacity"), tTJSVariant((tjs_int)255));
	SetTjsProperty(dict, TJS_W("name"), tTJSVariant(layout.name));
	SetTjsProperty(dict, TJS_W("objectName"), tTJSVariant(layout.objectName));
	SetTjsProperty(dict, TJS_W("motionName"), tTJSVariant(layout.motionName));
	SetTjsProperty(dict, TJS_W("contextObjectName"), tTJSVariant(layout.contextObjectName));
	SetTjsProperty(dict, TJS_W("contextMotionName"), tTJSVariant(layout.contextMotionName));
	SetTjsProperty(dict, TJS_W("hitObjectName"), tTJSVariant(layout.hitObjectName));
	SetTjsProperty(dict, TJS_W("hitStateName"), tTJSVariant(layout.hitStateName));
	SetTjsProperty(dict, TJS_W("hitStateClass"), tTJSVariant(layout.hitStateClass));
	SetTjsProperty(dict, TJS_W("hitPartName"), tTJSVariant(layout.hitPartName));
	SetTjsProperty(dict, TJS_W("hitRole"), tTJSVariant(layout.hitRole));
	SetTjsProperty(dict, TJS_W("hitPathDepth"), tTJSVariant(layout.hitPathDepth));
	SetTjsProperty(dict, TJS_W("layoutProxy"), tTJSVariant((tjs_int)(layout.hitLayoutProxy ? 1 : 0)));
	SetTjsProperty(dict, TJS_W("frontDrawCandidate"), tTJSVariant((tjs_int)(layout.frontDrawCandidate ? 1 : 0)));
	SetTjsProperty(dict, TJS_W("frontHitCandidate"), tTJSVariant((tjs_int)(layout.frontHitCandidate ? 1 : 0)));
	SetTjsProperty(dict, TJS_W("hitPriority"), tTJSVariant(layout.hitPriority));
	SetTjsProperty(dict, TJS_W("hitInteractive"), tTJSVariant((tjs_int)(layout.hitInteractive ? 1 : 0)));

	tTJSVariant shape = MakeMotionHitShape(hitLayouts, layout, motionObject);
	SetTjsProperty(dict, TJS_W("motion"), shape);
	SetTjsProperty(dict, TJS_W("shape"), shape);

	tTJSVariant result(dict, dict);
	dict->Release();
	return result;
}

static iTJSDispatch2 *ResolveLayerTarget(iTJSDispatch2 *target)
{
	SeparateLayerAdaptor *adaptor =
		ncbInstanceAdaptor<SeparateLayerAdaptor>::GetNativeInstance(target, false);
	if (adaptor && adaptor->getLayer()) return adaptor->getLayer();
	return target;
}
