static void SetMotionFrontLayoutDumpFields(iTJSDispatch2 *dict,
	const MotionLayoutInfo &layout, tjs_int sourceIndex)
{
	SetTjsProperty(dict, TJS_W("sourceIndex"), tTJSVariant(sourceIndex));
	SetTjsProperty(dict, TJS_W("name"), tTJSVariant(layout.name));
	SetTjsProperty(dict, TJS_W("sourceKey"), tTJSVariant(layout.sourceKey.GetLen() > 0 ? layout.sourceKey : TJS_W("void")));
	SetTjsProperty(dict, TJS_W("objectName"), tTJSVariant(layout.objectName));
	SetTjsProperty(dict, TJS_W("motionName"), tTJSVariant(layout.motionName));
	SetTjsProperty(dict, TJS_W("contextObjectName"), tTJSVariant(layout.contextObjectName));
	SetTjsProperty(dict, TJS_W("contextMotionName"), tTJSVariant(layout.contextMotionName));
	SetTjsProperty(dict, TJS_W("hitObjectName"), tTJSVariant(layout.hitObjectName));
	SetTjsProperty(dict, TJS_W("hitObjectPath"), tTJSVariant(layout.hitObjectPath));
	SetTjsProperty(dict, TJS_W("hitStateName"), tTJSVariant(layout.hitStateName));
	SetTjsProperty(dict, TJS_W("hitStateClass"), tTJSVariant(layout.hitStateClass));
	SetTjsProperty(dict, TJS_W("hitStateGroup"), tTJSVariant(layout.hitStateGroup));
	SetTjsProperty(dict, TJS_W("hitStatePath"), tTJSVariant(layout.hitStatePath));
	SetTjsProperty(dict, TJS_W("hitPartName"), tTJSVariant(layout.hitPartName));
	SetTjsProperty(dict, TJS_W("hitRole"), tTJSVariant(layout.hitRole));
	SetTjsProperty(dict, TJS_W("hitPathDepth"), tTJSVariant(layout.hitPathDepth));
	SetTjsProperty(dict, TJS_W("layoutProxy"), tTJSVariant((tjs_int)(layout.hitLayoutProxy ? 1 : 0)));
	SetTjsProperty(dict, TJS_W("frontDrawCandidate"), tTJSVariant((tjs_int)(layout.frontDrawCandidate ? 1 : 0)));
	SetTjsProperty(dict, TJS_W("frontHitCandidate"), tTJSVariant((tjs_int)(layout.frontHitCandidate ? 1 : 0)));
	SetTjsProperty(dict, TJS_W("left"), tTJSVariant(layout.left));
	SetTjsProperty(dict, TJS_W("top"), tTJSVariant(layout.top));
	SetTjsProperty(dict, TJS_W("width"), tTJSVariant(layout.width));
	SetTjsProperty(dict, TJS_W("height"), tTJSVariant(layout.height));
	SetTjsProperty(dict, TJS_W("centerX"), tTJSVariant(layout.centerX));
	SetTjsProperty(dict, TJS_W("centerY"), tTJSVariant(layout.centerY));
	SetTjsProperty(dict, TJS_W("opacityMin"), tTJSVariant(layout.opacityMin));
	SetTjsProperty(dict, TJS_W("opacityMax"), tTJSVariant(layout.opacityMax));
	SetTjsProperty(dict, TJS_W("alphaMin"), tTJSVariant(MotionAlphaFromOpacity(layout.opacityMin)));
	SetTjsProperty(dict, TJS_W("alphaMax"), tTJSVariant(MotionAlphaFromOpacity(layout.opacityMax)));
	SetTjsProperty(dict, TJS_W("alphaHitEnabled"),
		tTJSVariant((tjs_int)(layout.opacityMax > 0.001 ? 1 : 0)));
	SetTjsProperty(dict, TJS_W("alphaAware"),
		tTJSVariant((tjs_int)(
			layout.opacityMin < 0.999 || layout.opacityMax < 0.999 ? 1 : 0)));
	SetTjsProperty(dict, TJS_W("hitArea"),
		tTJSVariant(layout.width * layout.height));
	SetTjsProperty(dict, TJS_W("hitCenterX"), tTJSVariant(layout.centerX));
	SetTjsProperty(dict, TJS_W("hitCenterY"), tTJSVariant(layout.centerY));
	SetTjsProperty(dict, TJS_W("drawCount"), tTJSVariant(layout.drawCount));
	SetTjsProperty(dict, TJS_W("hitPriority"), tTJSVariant(layout.hitPriority));
	SetTjsProperty(dict, TJS_W("hitInteractive"), tTJSVariant((tjs_int)(layout.hitInteractive ? 1 : 0)));
}

static void SetMotionFrontSemanticResolveDumpFields(iTJSDispatch2 *dict,
	const MotionLayoutInfo &layout, tjs_int sourceIndex,
	const tjs_char *mode, tjs_int sourceCount,
	tjs_int hitShapeCount, tjs_int drawItemCount,
	const ttstr &contextObject, const ttstr &contextMotion)
{
	SetTjsProperty(dict, TJS_W("semanticResolveMode"), tTJSVariant(mode));
	SetTjsProperty(dict, TJS_W("semanticSourceIndex"), tTJSVariant(sourceIndex));
	SetTjsProperty(dict, TJS_W("semanticSourceCount"), tTJSVariant(sourceCount));
	SetTjsProperty(dict, TJS_W("semanticHitShapeCount"), tTJSVariant(hitShapeCount));
	SetTjsProperty(dict, TJS_W("semanticDrawItemCount"), tTJSVariant(drawItemCount));
	SetTjsProperty(dict, TJS_W("semanticContextScore"),
		tTJSVariant(MotionFrontResolveContextScore(
			layout, contextObject, contextMotion)));
	SetTjsProperty(dict, TJS_W("semanticSpecificityScore"),
		tTJSVariant(MotionFrontResolveSemanticSpecificityScore(layout)));
	SetTjsProperty(dict, TJS_W("semanticName"), tTJSVariant(layout.name));
	SetTjsProperty(dict, TJS_W("semanticObjectName"), tTJSVariant(layout.hitObjectName));
	SetTjsProperty(dict, TJS_W("semanticObjectPath"), tTJSVariant(layout.hitObjectPath));
	SetTjsProperty(dict, TJS_W("semanticStateName"), tTJSVariant(layout.hitStateName));
	SetTjsProperty(dict, TJS_W("semanticStateClass"), tTJSVariant(layout.hitStateClass));
	SetTjsProperty(dict, TJS_W("semanticStateGroup"), tTJSVariant(layout.hitStateGroup));
	SetTjsProperty(dict, TJS_W("semanticStatePath"), tTJSVariant(layout.hitStatePath));
	SetTjsProperty(dict, TJS_W("semanticPartName"), tTJSVariant(layout.hitPartName));
	SetTjsProperty(dict, TJS_W("semanticRole"), tTJSVariant(layout.hitRole));
	SetTjsProperty(dict, TJS_W("semanticLayoutProxy"),
		tTJSVariant((tjs_int)(layout.hitLayoutProxy ? 1 : 0)));
	SetTjsProperty(dict, TJS_W("semanticHitInteractive"),
		tTJSVariant((tjs_int)(layout.hitInteractive ? 1 : 0)));
	SetTjsProperty(dict, TJS_W("semanticHitPriority"),
		tTJSVariant(layout.hitPriority));
	SetTjsProperty(dict, TJS_W("semanticLeft"), tTJSVariant(layout.left));
	SetTjsProperty(dict, TJS_W("semanticTop"), tTJSVariant(layout.top));
	SetTjsProperty(dict, TJS_W("semanticWidth"), tTJSVariant(layout.width));
	SetTjsProperty(dict, TJS_W("semanticHeight"), tTJSVariant(layout.height));
	SetTjsProperty(dict, TJS_W("semanticCenterX"), tTJSVariant(layout.centerX));
	SetTjsProperty(dict, TJS_W("semanticCenterY"), tTJSVariant(layout.centerY));
	SetTjsProperty(dict, TJS_W("semanticHitArea"),
		tTJSVariant(layout.width * layout.height));
}

static tTJSVariant MakeMotionFrontDrawItemDumpArray(const MotionResource *resource,
	tjs_real meshDivisionRatio)
{
	iTJSDispatch2 *array = TJSCreateArrayObject();
	std::vector<std::pair<const MotionLayoutInfo*, tjs_int> > items =
		MotionCollectFrontLayouts(resource, false);
	tjs_int division = (tjs_int)std::ceil(std::max<tjs_real>(1.0, meshDivisionRatio));
	division = std::max<tjs_int>(1, division);
	tjs_int runningCellIndex = 0;
	tjs_int runningVertexOffset = 0;
	tjs_int runningIndexOffset = 0;
	for (size_t i = 0; i < items.size(); i++) {
		const MotionLayoutInfo *layout = items[i].first;
		if (!layout) continue;
			tjs_int itemQuadCount = division * division;
			tjs_int itemVertexCount = (division + 1) * (division + 1);
			tjs_int itemIndexCount = itemQuadCount * 6;
			iTJSDispatch2 *dict = TJSCreateDictionaryObject();
			SetTjsProperty(dict, TJS_W("index"), tTJSVariant((tjs_int)i));
			SetMotionFrontLayoutDumpFields(dict, *layout, items[i].second);
			SetTjsProperty(dict, TJS_W("source"), tTJSVariant(TJS_W("layout-front")));
			SetTjsProperty(dict, TJS_W("quadCount"), tTJSVariant(itemQuadCount));
			SetTjsProperty(dict, TJS_W("vertexCount"), tTJSVariant(itemVertexCount));
			SetTjsProperty(dict, TJS_W("indexCount"), tTJSVariant(itemIndexCount));
			SetTjsProperty(dict, TJS_W("vertexOffset"), tTJSVariant(runningVertexOffset));
			SetTjsProperty(dict, TJS_W("indexOffset"), tTJSVariant(runningIndexOffset));
			SetTjsProperty(dict, TJS_W("meshDivision"), tTJSVariant(division));
			SetTjsProperty(dict, TJS_W("firstCellIndex"), tTJSVariant(runningCellIndex));
			SetTjsProperty(dict, TJS_W("cellCount"), tTJSVariant(itemQuadCount));
			runningCellIndex += itemQuadCount;
			runningVertexOffset += itemVertexCount;
			runningIndexOffset += itemIndexCount;
				{
					bool useAffine = MotionLayoutNeedsAffinePath(layout);
					if (useAffine) {
						tjs_real q0x, q0y, q1x, q1y, q2x, q2y, q3x, q3y;
						MotionApplyIconDrawAffineToPoint(layout->affine, 0.0, 0.0, q0x, q0y);
						MotionApplyIconDrawAffineToPoint(layout->affine, 1.0, 0.0, q1x, q1y);
						MotionApplyIconDrawAffineToPoint(layout->affine, 1.0, 1.0, q2x, q2y);
						MotionApplyIconDrawAffineToPoint(layout->affine, 0.0, 1.0, q3x, q3y);
					SetTjsProperty(dict, TJS_W("v0x"), tTJSVariant(q0x));
					SetTjsProperty(dict, TJS_W("v0y"), tTJSVariant(q0y));
					SetTjsProperty(dict, TJS_W("v1x"), tTJSVariant(q1x));
					SetTjsProperty(dict, TJS_W("v1y"), tTJSVariant(q1y));
					SetTjsProperty(dict, TJS_W("v2x"), tTJSVariant(q2x));
					SetTjsProperty(dict, TJS_W("v2y"), tTJSVariant(q2y));
					SetTjsProperty(dict, TJS_W("v3x"), tTJSVariant(q3x));
					SetTjsProperty(dict, TJS_W("v3y"), tTJSVariant(q3y));
				} else {
					SetTjsProperty(dict, TJS_W("v0x"), tTJSVariant(layout->left));
					SetTjsProperty(dict, TJS_W("v0y"), tTJSVariant(layout->top));
					SetTjsProperty(dict, TJS_W("v1x"), tTJSVariant(layout->left + layout->width));
					SetTjsProperty(dict, TJS_W("v1y"), tTJSVariant(layout->top));
					SetTjsProperty(dict, TJS_W("v2x"), tTJSVariant(layout->left + layout->width));
					SetTjsProperty(dict, TJS_W("v2y"), tTJSVariant(layout->top + layout->height));
					SetTjsProperty(dict, TJS_W("v3x"), tTJSVariant(layout->left));
					SetTjsProperty(dict, TJS_W("v3y"), tTJSVariant(layout->top + layout->height));
				}
			}
			tTJSVariant value(dict, dict);
			dict->Release();
			array->PropSetByNum(TJS_MEMBERENSURE, (tjs_int)i, &value, array);
	}
	tTJSVariant result(array, array);
	array->Release();
	return result;
}

static tTJSVariant MakeMotionFrontMeshCellDumpArray(
	const MotionResource *resource, tjs_real meshDivisionRatio)
{
	iTJSDispatch2 *array = TJSCreateArrayObject();
	std::vector<std::pair<const MotionLayoutInfo*, tjs_int> > items =
		MotionCollectFrontLayouts(resource, false);
	if (!items.empty()) {
		tjs_int division = (tjs_int)std::ceil(std::max<tjs_real>(1.0, meshDivisionRatio));
		division = std::max<tjs_int>(1, division);
		tjs_int index = 0;
		for (size_t i = 0; i < items.size(); i++) {
			const MotionLayoutInfo *layout = items[i].first;
			if (!layout) continue;
			tjs_real cellWidth = layout->width / (tjs_real)division;
			tjs_real cellHeight = layout->height / (tjs_real)division;
			tjs_real alphaMin = MotionAlphaFromOpacity(layout->opacityMin);
			tjs_real alphaMax = MotionAlphaFromOpacity(layout->opacityMax);
			tjs_real cellAlpha = alphaMax;
			tjs_real uvStep = 1.0 / (tjs_real)division;
			bool useAffine = MotionLayoutNeedsAffinePath(layout);
			bool useControlPts = layout->hasControlPoints && !MotionControlPtsAreIdentity(layout->controlPts);
			auto computePoint = [&](tjs_real u, tjs_real v, tjs_real &ox, tjs_real &oy) {
				tjs_real du = u, dv = v;
				if (useControlPts) {
					MotionEvalBezierSurface(layout->controlPts, u, v, du, dv);
				}
				if (useAffine) {
					MotionApplyIconDrawAffineToPoint(layout->affine, du, dv, ox, oy);
				} else {
					ox = layout->left + layout->width * du;
					oy = layout->top + layout->height * dv;
				}
			};
			for (tjs_int y = 0; y < division; y++) {
				for (tjs_int x = 0; x < division; x++) {
					tjs_real cx0, cy0, cx1a, cy1a, cx2, cy2, cx3, cy3;
					tjs_real u0 = uvStep * (tjs_real)x;
					tjs_real v0 = uvStep * (tjs_real)y;
					tjs_real u1 = uvStep * (tjs_real)(x + 1);
					tjs_real v1 = uvStep * (tjs_real)(y + 1);
					computePoint(u0, v0, cx0, cy0);
					computePoint(u1, v0, cx1a, cy1a);
					computePoint(u1, v1, cx2, cy2);
					computePoint(u0, v1, cx3, cy3);
					iTJSDispatch2 *dict = TJSCreateDictionaryObject();
					SetTjsProperty(dict, TJS_W("index"), tTJSVariant(index));
					SetTjsProperty(dict, TJS_W("drawItemIndex"), tTJSVariant((tjs_int)i));
					SetTjsProperty(dict, TJS_W("sourceIndex"), tTJSVariant(items[i].second));
					SetTjsProperty(dict, TJS_W("name"), tTJSVariant(layout->name));
					SetTjsProperty(dict, TJS_W("hitObjectName"), tTJSVariant(layout->hitObjectName));
					SetTjsProperty(dict, TJS_W("hitObjectPath"), tTJSVariant(layout->hitObjectPath));
					SetTjsProperty(dict, TJS_W("hitStateName"), tTJSVariant(layout->hitStateName));
					SetTjsProperty(dict, TJS_W("hitStateClass"), tTJSVariant(layout->hitStateClass));
					SetTjsProperty(dict, TJS_W("hitStateGroup"), tTJSVariant(layout->hitStateGroup));
					SetTjsProperty(dict, TJS_W("hitStatePath"), tTJSVariant(layout->hitStatePath));
					SetTjsProperty(dict, TJS_W("hitPartName"), tTJSVariant(layout->hitPartName));
					SetTjsProperty(dict, TJS_W("hitRole"), tTJSVariant(layout->hitRole));
					SetTjsProperty(dict, TJS_W("gridX"), tTJSVariant(x));
					SetTjsProperty(dict, TJS_W("gridY"), tTJSVariant(y));
					SetTjsProperty(dict, TJS_W("left"), tTJSVariant(cx0));
					SetTjsProperty(dict, TJS_W("top"), tTJSVariant(cy0));
					SetTjsProperty(dict, TJS_W("width"), tTJSVariant(cx2 - cx0));
					SetTjsProperty(dict, TJS_W("height"), tTJSVariant(cy2 - cy0));
					SetTjsProperty(dict, TJS_W("alphaMin"), tTJSVariant(alphaMin));
					SetTjsProperty(dict, TJS_W("alphaMax"), tTJSVariant(alphaMax));
					SetTjsProperty(dict, TJS_W("v0x"), tTJSVariant(cx0));
					SetTjsProperty(dict, TJS_W("v0y"), tTJSVariant(cy0));
					SetTjsProperty(dict, TJS_W("v1x"), tTJSVariant(cx1a));
					SetTjsProperty(dict, TJS_W("v1y"), tTJSVariant(cy1a));
					SetTjsProperty(dict, TJS_W("v2x"), tTJSVariant(cx2));
					SetTjsProperty(dict, TJS_W("v2y"), tTJSVariant(cy2));
					SetTjsProperty(dict, TJS_W("v3x"), tTJSVariant(cx3));
					SetTjsProperty(dict, TJS_W("v3y"), tTJSVariant(cy3));
					SetTjsProperty(dict, TJS_W("uv0x"), tTJSVariant(u0));
					SetTjsProperty(dict, TJS_W("uv0y"), tTJSVariant(v0));
					SetTjsProperty(dict, TJS_W("uv1x"), tTJSVariant(u1));
					SetTjsProperty(dict, TJS_W("uv1y"), tTJSVariant(v0));
					SetTjsProperty(dict, TJS_W("uv2x"), tTJSVariant(u1));
					SetTjsProperty(dict, TJS_W("uv2y"), tTJSVariant(v1));
					SetTjsProperty(dict, TJS_W("uv3x"), tTJSVariant(u0));
					SetTjsProperty(dict, TJS_W("uv3y"), tTJSVariant(v1));
					SetTjsProperty(dict, TJS_W("alpha0"), tTJSVariant(cellAlpha));
					SetTjsProperty(dict, TJS_W("alpha1"), tTJSVariant(cellAlpha));
					SetTjsProperty(dict, TJS_W("alpha2"), tTJSVariant(cellAlpha));
					SetTjsProperty(dict, TJS_W("alpha3"), tTJSVariant(cellAlpha));
					SetTjsProperty(dict, TJS_W("hitPriority"), tTJSVariant(layout->hitPriority));
					tTJSVariant value(dict, dict);
					dict->Release();
					array->PropSetByNum(TJS_MEMBERENSURE, index, &value, array);
					index++;
				}
			}
		}
	}
	tTJSVariant result(array, array);
	array->Release();
	return result;
}

static tTJSVariant MakeMotionFrontHitShapeDumpArray(const MotionResource *resource)
{
	iTJSDispatch2 *array = TJSCreateArrayObject();
	std::vector<std::pair<const MotionLayoutInfo*, tjs_int> > items =
		MotionCollectFrontLayouts(resource, true);
	for (size_t i = 0; i < items.size(); i++) {
		const MotionLayoutInfo *layout = items[i].first;
		if (!layout) continue;
			iTJSDispatch2 *dict = TJSCreateDictionaryObject();
			SetTjsProperty(dict, TJS_W("index"), tTJSVariant((tjs_int)i));
			SetTjsProperty(dict, TJS_W("type"), tTJSVariant(TJS_W("rect")));
			SetMotionFrontLayoutDumpFields(dict, *layout, items[i].second);
			SetTjsProperty(dict, TJS_W("alphaThreshold"), tTJSVariant(MotionAlphaFromOpacity(layout->opacityMin)));
			SetTjsProperty(dict, TJS_W("visible"), tTJSVariant((tjs_int)(layout->opacityMax > 0.001 ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("vertexCount"), tTJSVariant((tjs_int)4));
			SetTjsProperty(dict, TJS_W("v0x"), tTJSVariant(layout->left));
			SetTjsProperty(dict, TJS_W("v0y"), tTJSVariant(layout->top));
			SetTjsProperty(dict, TJS_W("v1x"), tTJSVariant(layout->left + layout->width));
			SetTjsProperty(dict, TJS_W("v1y"), tTJSVariant(layout->top));
			SetTjsProperty(dict, TJS_W("v2x"), tTJSVariant(layout->left + layout->width));
			SetTjsProperty(dict, TJS_W("v2y"), tTJSVariant(layout->top + layout->height));
			SetTjsProperty(dict, TJS_W("v3x"), tTJSVariant(layout->left));
			SetTjsProperty(dict, TJS_W("v3y"), tTJSVariant(layout->top + layout->height));
			tTJSVariant value(dict, dict);
			dict->Release();
			array->PropSetByNum(TJS_MEMBERENSURE, (tjs_int)i, &value, array);
	}
	tTJSVariant result(array, array);
	array->Release();
	return result;
}

static tTJSVariant MakeMotionFrontObjectStateDumpArray(
	const MotionResource *resource, bool hitOnly)
{
	iTJSDispatch2 *array = TJSCreateArrayObject();
	std::vector<MotionFrontObjectStateInfo> items =
		MotionCollectFrontObjectStates(resource, hitOnly);
	for (size_t i = 0; i < items.size(); i++) {
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		SetTjsProperty(dict, TJS_W("index"), tTJSVariant((tjs_int)i));
		SetMotionFrontLayoutDumpFields(dict, items[i].layout, items[i].sourceIndex);
		SetTjsProperty(dict, TJS_W("source"), tTJSVariant(
			hitOnly ? TJS_W("object-state-hit") : TJS_W("object-state-draw")));
		SetTjsProperty(dict, TJS_W("sourceCount"), tTJSVariant(items[i].sourceCount));
		SetTjsProperty(dict, TJS_W("drawItemCount"), tTJSVariant(items[i].drawItemCount));
		SetTjsProperty(dict, TJS_W("hitShapeCount"), tTJSVariant(items[i].hitShapeCount));
		tTJSVariant value(dict, dict);
		dict->Release();
		array->PropSetByNum(TJS_MEMBERENSURE, (tjs_int)i, &value, array);
	}
	tTJSVariant result(array, array);
	array->Release();
	return result;
}

static tTJSVariant MakeMotionFrontObjectStateDumpArray(
	const std::vector<MotionFrontObjectStateInfo> &items,
	const tjs_char *source)
{
	iTJSDispatch2 *array = TJSCreateArrayObject();
	for (size_t i = 0; i < items.size(); i++) {
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		SetTjsProperty(dict, TJS_W("index"), tTJSVariant((tjs_int)i));
		SetMotionFrontLayoutDumpFields(dict, items[i].layout, items[i].sourceIndex);
		SetTjsProperty(dict, TJS_W("source"), tTJSVariant(source));
		SetTjsProperty(dict, TJS_W("sourceCount"), tTJSVariant(items[i].sourceCount));
		SetTjsProperty(dict, TJS_W("drawItemCount"), tTJSVariant(items[i].drawItemCount));
		SetTjsProperty(dict, TJS_W("hitShapeCount"), tTJSVariant(items[i].hitShapeCount));
		SetTjsProperty(dict, TJS_W("contextScore"), tTJSVariant(items[i].contextScore));
		SetTjsProperty(dict, TJS_W("semanticSpecificityScore"),
			tTJSVariant(MotionFrontResolveSemanticSpecificityScore(items[i].layout)));
		tTJSVariant value(dict, dict);
		dict->Release();
		array->PropSetByNum(TJS_MEMBERENSURE, (tjs_int)i, &value, array);
	}
	tTJSVariant result(array, array);
	array->Release();
	return result;
}

static tTJSVariant MakeMotionRendererPrepDump(const MotionResource *resource,
	tjs_real meshDivisionRatio, bool includeFrontArrays = false)
{
	MotionRendererPrepInfo info =
		MotionBuildRendererPrepInfo(resource, meshDivisionRatio);
	iTJSDispatch2 *dict = TJSCreateDictionaryObject();
	SetTjsProperty(dict, TJS_W("backend"),
		tTJSVariant(MotionRendererBackendName(g_motionRendererBackend)));
	SetTjsProperty(dict, TJS_W("meshPrepared"), tTJSVariant((tjs_int)(info.meshPrepared ? 1 : 0)));
	SetTjsProperty(dict, TJS_W("boundsValid"), tTJSVariant((tjs_int)(info.boundsValid ? 1 : 0)));
	SetTjsProperty(dict, TJS_W("meshDivisionRatio"), tTJSVariant(info.meshDivisionRatio));
	SetTjsProperty(dict, TJS_W("meshDivision"), tTJSVariant(info.meshDivision));
	SetTjsProperty(dict, TJS_W("layoutCount"), tTJSVariant(info.layoutCount));
	SetTjsProperty(dict, TJS_W("drawItemCount"), tTJSVariant(info.drawItemCount));
	SetTjsProperty(dict, TJS_W("frontDrawItemCount"), tTJSVariant(info.frontDrawItemCount));
	SetTjsProperty(dict, TJS_W("frontMeshCellCount"), tTJSVariant(info.frontMeshCellCount));
	SetTjsProperty(dict, TJS_W("frontHitShapeCount"), tTJSVariant(info.frontHitShapeCount));
	SetTjsProperty(dict, TJS_W("frontObjectStateCount"), tTJSVariant(info.frontObjectStateCount));
	SetTjsProperty(dict, TJS_W("frontHitObjectStateCount"), tTJSVariant(info.frontHitObjectStateCount));
	SetTjsProperty(dict, TJS_W("alphaAwareDrawItemCount"), tTJSVariant(info.alphaAwareDrawItemCount));
	SetTjsProperty(dict, TJS_W("alphaAwareHitShapeCount"), tTJSVariant(info.alphaAwareHitShapeCount));
	SetTjsProperty(dict, TJS_W("estimatedQuadCount"), tTJSVariant(info.estimatedQuadCount));
	SetTjsProperty(dict, TJS_W("estimatedVertexCount"), tTJSVariant(info.estimatedVertexCount));
	SetTjsProperty(dict, TJS_W("estimatedIndexCount"), tTJSVariant(info.estimatedIndexCount));
	SetTjsProperty(dict, TJS_W("left"), tTJSVariant(info.left));
	SetTjsProperty(dict, TJS_W("top"), tTJSVariant(info.top));
	SetTjsProperty(dict, TJS_W("width"), tTJSVariant(info.width));
	SetTjsProperty(dict, TJS_W("height"), tTJSVariant(info.height));
	if (resource) {
		SetTjsProperty(dict, TJS_W("resourceId"), tTJSVariant((tjs_int)resource->id));
		SetTjsProperty(dict, TJS_W("storage"), tTJSVariant(resource->storage));
		SetTjsProperty(dict, TJS_W("parsed"), tTJSVariant((tjs_int)(resource->parsed ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("valid"), tTJSVariant((tjs_int)(resource->opened ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("widthPx"), tTJSVariant(resource->width));
		SetTjsProperty(dict, TJS_W("heightPx"), tTJSVariant(resource->height));
		SetTjsProperty(dict, TJS_W("chara"), tTJSVariant(resource->chara));
		SetTjsProperty(dict, TJS_W("motion"), tTJSVariant(resource->motion));
		if (includeFrontArrays) {
			SetTjsProperty(dict, TJS_W("frontDrawItems"),
				MakeMotionFrontDrawItemDumpArray(resource, meshDivisionRatio));
			SetTjsProperty(dict, TJS_W("frontObjectStates"),
				MakeMotionFrontObjectStateDumpArray(resource, false));
			SetTjsProperty(dict, TJS_W("frontMeshCells"),
				MakeMotionFrontMeshCellDumpArray(resource, meshDivisionRatio));
			SetTjsProperty(dict, TJS_W("frontHitShapes"),
				MakeMotionFrontHitShapeDumpArray(resource));
			SetTjsProperty(dict, TJS_W("frontHitObjectStates"),
				MakeMotionFrontObjectStateDumpArray(resource, true));
		}
	} else {
		SetTjsProperty(dict, TJS_W("resourceId"), tTJSVariant((tjs_int)-1));
		SetTjsProperty(dict, TJS_W("storage"), tTJSVariant(TJS_W("")));
		SetTjsProperty(dict, TJS_W("parsed"), tTJSVariant((tjs_int)0));
		SetTjsProperty(dict, TJS_W("valid"), tTJSVariant((tjs_int)0));
		SetTjsProperty(dict, TJS_W("widthPx"), tTJSVariant((tjs_int)0));
		SetTjsProperty(dict, TJS_W("heightPx"), tTJSVariant((tjs_int)0));
		SetTjsProperty(dict, TJS_W("chara"), tTJSVariant(TJS_W("")));
		SetTjsProperty(dict, TJS_W("motion"), tTJSVariant(TJS_W("")));
		if (includeFrontArrays) {
			SetTjsProperty(dict, TJS_W("frontDrawItems"),
				MakeMotionFrontDrawItemDumpArray(nullptr, meshDivisionRatio));
			SetTjsProperty(dict, TJS_W("frontObjectStates"),
				MakeMotionFrontObjectStateDumpArray(nullptr, false));
			SetTjsProperty(dict, TJS_W("frontMeshCells"),
				MakeMotionFrontMeshCellDumpArray(nullptr, meshDivisionRatio));
			SetTjsProperty(dict, TJS_W("frontHitShapes"),
				MakeMotionFrontHitShapeDumpArray(nullptr));
			SetTjsProperty(dict, TJS_W("frontHitObjectStates"),
				MakeMotionFrontObjectStateDumpArray(nullptr, true));
		}
	}
	tTJSVariant result(dict, dict);
	dict->Release();
	return result;
}

static tjs_int GetTjsVariantArrayCount(tTJSVariant object)
{
	if (object.Type() != tvtObject) return 0;
	tTJSVariantClosure &closure = object.AsObjectClosureNoAddRef();
	if (!closure.Object) return 0;
	tTJSVariant count;
	if (TJS_FAILED(closure.PropGet(0, TJS_W("count"),
		nullptr, &count, closure.ObjThis)) || count.Type() == tvtVoid) {
		return 0;
	}
	try {
		return std::max<tjs_int>(0, (tjs_int)count);
	} catch (...) {
		return 0;
	}
}

static bool MotionReadVariantReal(tTJSVariant object, const tjs_char *name,
	tjs_real &target)
{
	tTJSVariant value;
	if (!GetTjsVariantObjectProperty(object, name, value) || value.Type() == tvtVoid) {
		return false;
	}
	try {
		tjs_real number = (tjs_real)value;
		if (!std::isfinite((double)number)) return false;
		target = number;
		return true;
	} catch (...) {
		return false;
	}
}

static bool MotionReadVariantInt(tTJSVariant object, const tjs_char *name,
	tjs_int &target)
{
	tTJSVariant value;
	if (!GetTjsVariantObjectProperty(object, name, value) || value.Type() == tvtVoid) {
		return false;
	}
	try {
		target = (tjs_int)value;
		return true;
	} catch (...) {
		return false;
	}
}

static bool MotionReadVariantBool(tTJSVariant object, const tjs_char *name,
	bool &target)
{
	tTJSVariant value;
	if (!GetTjsVariantObjectProperty(object, name, value) || value.Type() == tvtVoid) {
		return false;
	}
	try {
		target = ((tjs_int)value) != 0;
		return true;
	} catch (...) {
		return false;
	}
}

static bool MotionReadVariantString(tTJSVariant object, const tjs_char *name,
	ttstr &target)
{
	tTJSVariant value;
	if (!GetTjsVariantObjectProperty(object, name, value) || value.Type() == tvtVoid) {
		return false;
	}
	try {
		target = ttstr(value);
		return true;
	} catch (...) {
		return false;
	}
}

static bool ExtractLayerGetterMotion(tTJSVariant getter, tTJSVariant *result)
{
	tTJSVariant motion;
	if (!GetTjsVariantObjectProperty(getter, TJS_W("motion"), motion)) {
		if (result) result->Clear();
		return false;
	}
	if (result) *result = motion;
	return true;
}

static std::vector<std::weak_ptr<MotionResource>> &MotionGlobalResourceRegistry()
{
	static std::vector<std::weak_ptr<MotionResource>> resources;
	return resources;
}

static void RegisterMotionResource(const std::shared_ptr<MotionResource> &resource)
{
	if (!resource) return;
	std::vector<std::weak_ptr<MotionResource>> &registry = MotionGlobalResourceRegistry();
	for (auto it = registry.begin(); it != registry.end();) {
		std::shared_ptr<MotionResource> item = it->lock();
		if (!item) {
			it = registry.erase(it);
			continue;
		}
		if (item == resource) return;
		++it;
	}
	registry.push_back(resource);
}

static ttstr MotionTjsStringLiteral(const ttstr &value)
{
	ttstr out = TJS_W("\"");
	const tjs_char *p = value.c_str();
	for (tjs_int i = 0; i < value.GetLen(); i++) {
		tjs_char ch = p[i];
		switch (ch) {
		case TJS_W('\\'):
			out += TJS_W("\\\\");
			break;
		case TJS_W('"'):
			out += TJS_W("\\\"");
			break;
		case TJS_W('\n'):
			out += TJS_W("\\n");
			break;
		case TJS_W('\r'):
			out += TJS_W("\\r");
			break;
		case TJS_W('\t'):
			out += TJS_W("\\t");
			break;
		default:
			out += ch;
			break;
		}
	}
	out += TJS_W("\"");
	return out;
}

static bool WarmMotionTextureSnapshot(const ttstr &path,
	const ttstr &objectName, const ttstr &motionName, tjs_real frameTime)
{
#ifdef __EMSCRIPTEN__
	if (MotionHasStaticBridgeTextureSnapshot(path, objectName, motionName)) {
		return true;
	}
	EnsurePSBFilePlugin();
	try {
		TVPSetPSBStaticMotionHint(path.c_str(), objectName.c_str(),
			motionName.c_str(), (double)frameTime);
		tTVPBaseBitmap scratch(1, 1, 32);
		ttstr provincename;
		iTJSDispatch2 *metainfo = nullptr;
		TVPLoadGraphic(&scratch, path, 0x1fffffff, 0, 0,
			glmNormal, &provincename, &metainfo);
		if (metainfo) metainfo->Release();
	} catch (...) {
		TVPClearPSBStaticMotionHint();
		return false;
	}
	TVPClearPSBStaticMotionHint();
	return MotionHasStaticBridgeTextureSnapshot(path, objectName, motionName);
#else
	(void)path;
	(void)objectName;
	(void)motionName;
	(void)frameTime;
	return false;
#endif
}

static bool LoadImageIntoLayer(iTJSDispatch2 *target, const ttstr &path,
	const ttstr &objectName = ttstr(), const ttstr &motionName = ttstr(),
	tjs_real frameTime = 0.0)
{
	if (!target) return false;
	TVPClearLastPSBStaticMotionBridgeSnapshot();
	tTJSNI_BaseLayer *layer = nullptr;
	if (TJS_FAILED(target->NativeInstanceSupport(
		TJS_NIS_GETINSTANCE, tTJSNC_Layer::ClassID,
		(iTJSNativeInstance**)&layer)) || !layer) {
		return false;
	}

	iTJSDispatch2 *metainfo = nullptr;
		try {
			ttstr hintObject = objectName;
			ttstr hintMotion = motionName;
			if (hintObject.GetLen() > 0 && hintMotion.GetLen() > 0) {
				TVPSetPSBStaticMotionHint(path.c_str(), hintObject.c_str(), hintMotion.c_str(),
					(double)frameTime);
			}
			if (IsLogoPath(path)) TVPClearGraphicCache();
			metainfo = layer->LoadImages(path, 0x1fffffff);
			layer->Update();
			ForceMotionLayerUpdate();
	} catch (...) {
		TVPClearPSBStaticMotionHint();
		if (metainfo) metainfo->Release();
		TVPAddLog(ttstr(TJS_W("[emoteplayer] direct LoadImages failed: ")) + path);
		return false;
	}
	TVPClearPSBStaticMotionHint();
	if (metainfo) metainfo->Release();
	return true;
}

enum MaskMode {
	MaskModeNone,
	MaskModeAlpha
};

enum PlayFlag {
	PlayFlagNone,
	PlayFlagForce
};

enum ShapeType {
	ShapeTypePoint,
	ShapeTypeCircle,
	ShapeTypeRect,
	ShapeTypeQuad
};

static tTJSVariant MakeEmptyArray()
{
	iTJSDispatch2 *array = TJSCreateArrayObject();
	tTJSVariant result(array, array);
	array->Release();
	return result;
}

static tTJSVariant MakeStringArray(const std::vector<ttstr> &items)
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

static tTJSVariant MakeEmptyDictionary()
{
	iTJSDispatch2 *dict = TJSCreateDictionaryObject();
	tTJSVariant result(dict, dict);
	dict->Release();
	return result;
}

static tTJSVariant MakeStringArray(const std::vector<std::string> &items)
{
	std::vector<ttstr> values;
	for (size_t i = 0; i < items.size(); i++) values.push_back(MotionString(items[i]));
	return MakeStringArray(values);
}

static tTJSVariant MakeMotionVariableDumpArray(const std::vector<MotionVariableInfo> &items)
{
	iTJSDispatch2 *array = TJSCreateArrayObject();
	for (size_t i = 0; i < items.size(); i++) {
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		SetTjsProperty(dict, TJS_W("name"), tTJSVariant(items[i].name));
		SetTjsProperty(dict, TJS_W("min"), tTJSVariant(items[i].minValue));
		SetTjsProperty(dict, TJS_W("max"), tTJSVariant(items[i].maxValue));
		SetTjsProperty(dict, TJS_W("division"), tTJSVariant(items[i].division));
		tTJSVariant value(dict, dict);
		dict->Release();
		array->PropSetByNum(TJS_MEMBERENSURE, (tjs_int)i, &value, array);
	}
	tTJSVariant result(array, array);
	array->Release();
	return result;
}

static tTJSVariant MakeMotionSelectorOptionDumpArray(
	const std::vector<MotionSelectorOptionInfo> &items)
{
	iTJSDispatch2 *array = TJSCreateArrayObject();
	for (size_t i = 0; i < items.size(); i++) {
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		SetTjsProperty(dict, TJS_W("label"), tTJSVariant(items[i].label));
		SetTjsProperty(dict, TJS_W("offValue"), tTJSVariant(items[i].offValue));
		SetTjsProperty(dict, TJS_W("onValue"), tTJSVariant(items[i].onValue));
		tTJSVariant value(dict, dict);
		dict->Release();
		array->PropSetByNum(TJS_MEMBERENSURE, (tjs_int)i, &value, array);
	}
	tTJSVariant result(array, array);
	array->Release();
	return result;
}

static tTJSVariant MakeMotionSelectorDumpArray(const std::vector<MotionSelectorInfo> &items)
{
	iTJSDispatch2 *array = TJSCreateArrayObject();
	for (size_t i = 0; i < items.size(); i++) {
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		SetTjsProperty(dict, TJS_W("label"), tTJSVariant(items[i].label));
		SetTjsProperty(dict, TJS_W("optionCount"), tTJSVariant((tjs_int)items[i].options.size()));
		SetTjsProperty(dict, TJS_W("options"), MakeMotionSelectorOptionDumpArray(items[i].options));
		tTJSVariant value(dict, dict);
		dict->Release();
		array->PropSetByNum(TJS_MEMBERENSURE, (tjs_int)i, &value, array);
	}
	tTJSVariant result(array, array);
	array->Release();
	return result;
}

static tTJSVariant MakeMotionNodeDumpArray(const std::vector<MotionNodeInfo> &items)
{
	iTJSDispatch2 *array = TJSCreateArrayObject();
	for (size_t i = 0; i < items.size(); i++) {
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		SetTjsProperty(dict, TJS_W("label"), tTJSVariant(items[i].label));
		SetTjsProperty(dict, TJS_W("type"), tTJSVariant(items[i].type));
		SetTjsProperty(dict, TJS_W("parameterIdx"), tTJSVariant(items[i].parameterIdx));
		SetTjsProperty(dict, TJS_W("frameCount"), tTJSVariant(items[i].frameCount));
		SetTjsProperty(dict, TJS_W("childCount"), tTJSVariant(items[i].childCount));
		SetTjsProperty(dict, TJS_W("motionRefs"), MakeStringArray(items[i].motionRefs));
		tTJSVariant value(dict, dict);
		dict->Release();
		array->PropSetByNum(TJS_MEMBERENSURE, (tjs_int)i, &value, array);
	}
	tTJSVariant result(array, array);
	array->Release();
	return result;
}

static tTJSVariant MakeMotionAttachmentRefDumpArray(
	const std::vector<MotionAttachmentRefInfo> &items)
{
	iTJSDispatch2 *array = TJSCreateArrayObject();
	for (size_t i = 0; i < items.size(); i++) {
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		SetTjsProperty(dict, TJS_W("ownerObjectName"), tTJSVariant(items[i].ownerObjectName));
		SetTjsProperty(dict, TJS_W("ownerMotionName"), tTJSVariant(items[i].ownerMotionName));
		SetTjsProperty(dict, TJS_W("nodeLabel"), tTJSVariant(items[i].nodeLabel));
		SetTjsProperty(dict, TJS_W("refPath"), tTJSVariant(items[i].refPath));
		SetTjsProperty(dict, TJS_W("refObjectName"), tTJSVariant(items[i].refObjectName));
		SetTjsProperty(dict, TJS_W("refMotionName"), tTJSVariant(items[i].refMotionName));
		SetTjsProperty(dict, TJS_W("refIconName"), tTJSVariant(items[i].refIconName));
		SetTjsProperty(dict, TJS_W("frameTime"), tTJSVariant(items[i].frameTime));
		SetTjsProperty(dict, TJS_W("resolved"), tTJSVariant((tjs_int)(items[i].resolved ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("external"), tTJSVariant((tjs_int)(items[i].external ? 1 : 0)));
		tTJSVariant value(dict, dict);
		dict->Release();
		array->PropSetByNum(TJS_MEMBERENSURE, (tjs_int)i, &value, array);
	}
	tTJSVariant result(array, array);
	array->Release();
	return result;
}

static tTJSVariant MakeMotionDumpArray(const std::vector<MotionInfo> &items)
{
	iTJSDispatch2 *array = TJSCreateArrayObject();
	for (size_t i = 0; i < items.size(); i++) {
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		SetTjsProperty(dict, TJS_W("objectName"), tTJSVariant(items[i].objectName));
		SetTjsProperty(dict, TJS_W("name"), tTJSVariant(items[i].name));
		SetTjsProperty(dict, TJS_W("parameterIdx"), tTJSVariant(items[i].parameterIdx));
		SetTjsProperty(dict, TJS_W("lastTime"), tTJSVariant(items[i].lastTime));
		SetTjsProperty(dict, TJS_W("loopTime"), tTJSVariant(items[i].loopTime));
		SetTjsProperty(dict, TJS_W("selfSyncTime"), tTJSVariant(items[i].selfSyncTime));
		SetTjsProperty(dict, TJS_W("syncTime"), tTJSVariant(items[i].syncTime));
		SetTjsProperty(dict, TJS_W("frameCount"), tTJSVariant(items[i].frameCount));
		SetTjsProperty(dict, TJS_W("motionRefs"), MakeStringArray(items[i].motionRefs));
		SetTjsProperty(dict, TJS_W("attachmentRefCount"),
			tTJSVariant((tjs_int)items[i].attachmentRefs.size()));
		SetTjsProperty(dict, TJS_W("attachmentRefs"),
			MakeMotionAttachmentRefDumpArray(items[i].attachmentRefs));
		SetTjsProperty(dict, TJS_W("variables"), MakeMotionVariableDumpArray(items[i].variables));
		SetTjsProperty(dict, TJS_W("nodes"), MakeMotionNodeDumpArray(items[i].nodes));
		tTJSVariant value(dict, dict);
		dict->Release();
		array->PropSetByNum(TJS_MEMBERENSURE, (tjs_int)i, &value, array);
	}
	tTJSVariant result(array, array);
	array->Release();
	return result;
}

static tTJSVariant MakeMotionObjectDumpArray(const std::vector<MotionObjectInfo> &items)
{
	iTJSDispatch2 *array = TJSCreateArrayObject();
	for (size_t i = 0; i < items.size(); i++) {
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		SetTjsProperty(dict, TJS_W("name"), tTJSVariant(items[i].name));
		SetTjsProperty(dict, TJS_W("type"), tTJSVariant(items[i].type));
		SetTjsProperty(dict, TJS_W("motionCount"), tTJSVariant((tjs_int)items[i].motions.size()));
		SetTjsProperty(dict, TJS_W("motions"), MakeMotionDumpArray(items[i].motions));
		tTJSVariant value(dict, dict);
		dict->Release();
		array->PropSetByNum(TJS_MEMBERENSURE, (tjs_int)i, &value, array);
	}
	tTJSVariant result(array, array);
	array->Release();
	return result;
}

static tTJSVariant MakeMotionSourceIconDumpArray(
	const std::vector<MotionSourceIconInfo> &items)
{
	iTJSDispatch2 *array = TJSCreateArrayObject();
	for (size_t i = 0; i < items.size(); i++) {
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		SetTjsProperty(dict, TJS_W("key"), tTJSVariant(MotionString(items[i].key)));
		SetTjsProperty(dict, TJS_W("canonicalKey"), tTJSVariant(MotionString(items[i].canonicalKey)));
		SetTjsProperty(dict, TJS_W("sourceName"), tTJSVariant(MotionString(items[i].sourceName)));
		SetTjsProperty(dict, TJS_W("iconName"), tTJSVariant(MotionString(items[i].iconName)));
		SetTjsProperty(dict, TJS_W("name"), tTJSVariant(MotionString(items[i].name)));
		SetTjsProperty(dict, TJS_W("originX"), tTJSVariant(items[i].originX));
		SetTjsProperty(dict, TJS_W("originY"), tTJSVariant(items[i].originY));
		SetTjsProperty(dict, TJS_W("width"), tTJSVariant(items[i].width));
		SetTjsProperty(dict, TJS_W("height"), tTJSVariant(items[i].height));
		SetTjsProperty(dict, TJS_W("hasPixel"), tTJSVariant((tjs_int)(items[i].hasPixel ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("hasPalette"), tTJSVariant((tjs_int)(items[i].hasPalette ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("pixelLength"), tTJSVariant((tjs_int)items[i].pixelLength));
		SetTjsProperty(dict, TJS_W("paletteLength"), tTJSVariant((tjs_int)items[i].paletteLength));
		SetTjsProperty(dict, TJS_W("decoderSupported"), tTJSVariant((tjs_int)(items[i].decoderSupported ? 1 : 0)));
		tTJSVariant value(dict, dict);
		dict->Release();
		array->PropSetByNum(TJS_MEMBERENSURE, (tjs_int)i, &value, array);
	}
	tTJSVariant result(array, array);
	array->Release();
	return result;
}

static tTJSVariant MakeMotionTimelineFrameDumpArray(const std::vector<MotionTimelineFrameInfo> &items)
{
	iTJSDispatch2 *array = TJSCreateArrayObject();
	for (size_t i = 0; i < items.size(); i++) {
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		SetTjsProperty(dict, TJS_W("time"), tTJSVariant(items[i].time));
		SetTjsProperty(dict, TJS_W("type"), tTJSVariant(items[i].type));
		SetTjsProperty(dict, TJS_W("value"), tTJSVariant(items[i].value));
		SetTjsProperty(dict, TJS_W("easing"), tTJSVariant(items[i].easing));
		SetTjsProperty(dict, TJS_W("hasContent"), tTJSVariant((tjs_int)(items[i].hasContent ? 1 : 0)));
		tTJSVariant value(dict, dict);
		dict->Release();
		array->PropSetByNum(TJS_MEMBERENSURE, (tjs_int)i, &value, array);
	}
	tTJSVariant result(array, array);
	array->Release();
	return result;
}

static tTJSVariant MakeMotionTimelineVariableDumpArray(const std::vector<MotionTimelineVariableInfo> &items)
{
	iTJSDispatch2 *array = TJSCreateArrayObject();
	for (size_t i = 0; i < items.size(); i++) {
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		SetTjsProperty(dict, TJS_W("label"), tTJSVariant(items[i].label));
		SetTjsProperty(dict, TJS_W("frameCount"), tTJSVariant((tjs_int)items[i].frames.size()));
		SetTjsProperty(dict, TJS_W("frames"), MakeMotionTimelineFrameDumpArray(items[i].frames));
		tTJSVariant value(dict, dict);
		dict->Release();
		array->PropSetByNum(TJS_MEMBERENSURE, (tjs_int)i, &value, array);
	}
	tTJSVariant result(array, array);
	array->Release();
	return result;
}

static tTJSVariant MakeMotionTimelineDumpArray(const std::vector<MotionTimelineInfo> &items)
{
	iTJSDispatch2 *array = TJSCreateArrayObject();
	for (size_t i = 0; i < items.size(); i++) {
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		SetTjsProperty(dict, TJS_W("label"), tTJSVariant(items[i].label));
		SetTjsProperty(dict, TJS_W("lastTime"), tTJSVariant(items[i].lastTime));
		SetTjsProperty(dict, TJS_W("loopBegin"), tTJSVariant(items[i].loopBegin));
		SetTjsProperty(dict, TJS_W("loopEnd"), tTJSVariant(items[i].loopEnd));
		SetTjsProperty(dict, TJS_W("diff"), tTJSVariant(items[i].diff));
		SetTjsProperty(dict, TJS_W("variableCount"), tTJSVariant((tjs_int)items[i].variables.size()));
		SetTjsProperty(dict, TJS_W("variables"), MakeMotionTimelineVariableDumpArray(items[i].variables));
		tTJSVariant value(dict, dict);
		dict->Release();
		array->PropSetByNum(TJS_MEMBERENSURE, (tjs_int)i, &value, array);
	}
	tTJSVariant result(array, array);
	array->Release();
	return result;
}

static tTJSVariant MakeMotionLayoutDumpArray(const std::vector<MotionLayoutInfo> &items)
{
	iTJSDispatch2 *array = TJSCreateArrayObject();
	for (size_t i = 0; i < items.size(); i++) {
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		SetTjsProperty(dict, TJS_W("name"), tTJSVariant(items[i].name));
		SetTjsProperty(dict, TJS_W("sourceKey"), tTJSVariant(items[i].sourceKey.GetLen() > 0 ? items[i].sourceKey : TJS_W("void")));
		SetTjsProperty(dict, TJS_W("objectName"), tTJSVariant(items[i].objectName));
		SetTjsProperty(dict, TJS_W("motionName"), tTJSVariant(items[i].motionName));
		SetTjsProperty(dict, TJS_W("contextObjectName"), tTJSVariant(items[i].contextObjectName));
		SetTjsProperty(dict, TJS_W("contextMotionName"), tTJSVariant(items[i].contextMotionName));
		SetTjsProperty(dict, TJS_W("hitObjectName"), tTJSVariant(items[i].hitObjectName));
		SetTjsProperty(dict, TJS_W("hitObjectPath"), tTJSVariant(items[i].hitObjectPath));
		SetTjsProperty(dict, TJS_W("hitStateName"), tTJSVariant(items[i].hitStateName));
		SetTjsProperty(dict, TJS_W("hitStateClass"), tTJSVariant(items[i].hitStateClass));
		SetTjsProperty(dict, TJS_W("hitStateGroup"), tTJSVariant(items[i].hitStateGroup));
		SetTjsProperty(dict, TJS_W("hitStatePath"), tTJSVariant(items[i].hitStatePath));
		SetTjsProperty(dict, TJS_W("hitPartName"), tTJSVariant(items[i].hitPartName));
		SetTjsProperty(dict, TJS_W("hitRole"), tTJSVariant(items[i].hitRole));
		SetTjsProperty(dict, TJS_W("hitPathDepth"), tTJSVariant(items[i].hitPathDepth));
		SetTjsProperty(dict, TJS_W("layoutProxy"), tTJSVariant((tjs_int)(items[i].hitLayoutProxy ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("frontDrawCandidate"), tTJSVariant((tjs_int)(items[i].frontDrawCandidate ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("frontHitCandidate"), tTJSVariant((tjs_int)(items[i].frontHitCandidate ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("left"), tTJSVariant(items[i].left));
		SetTjsProperty(dict, TJS_W("top"), tTJSVariant(items[i].top));
		SetTjsProperty(dict, TJS_W("width"), tTJSVariant(items[i].width));
		SetTjsProperty(dict, TJS_W("height"), tTJSVariant(items[i].height));
		SetTjsProperty(dict, TJS_W("centerX"), tTJSVariant(items[i].centerX));
		SetTjsProperty(dict, TJS_W("centerY"), tTJSVariant(items[i].centerY));
		SetTjsProperty(dict, TJS_W("opacityMin"), tTJSVariant(items[i].opacityMin));
		SetTjsProperty(dict, TJS_W("opacityMax"), tTJSVariant(items[i].opacityMax));
		SetTjsProperty(dict, TJS_W("alphaMin"), tTJSVariant(MotionAlphaFromOpacity(items[i].opacityMin)));
		SetTjsProperty(dict, TJS_W("alphaMax"), tTJSVariant(MotionAlphaFromOpacity(items[i].opacityMax)));
		SetTjsProperty(dict, TJS_W("drawCount"), tTJSVariant(items[i].drawCount));
		SetTjsProperty(dict, TJS_W("renderMethodItemIndex"), tTJSVariant(items[i].renderMethodItemIndex));
		SetTjsProperty(dict, TJS_W("hitPriority"), tTJSVariant(items[i].hitPriority));
		SetTjsProperty(dict, TJS_W("hitInteractive"), tTJSVariant((tjs_int)(items[i].hitInteractive ? 1 : 0)));
		tTJSVariant value(dict, dict);
		dict->Release();
		array->PropSetByNum(TJS_MEMBERENSURE, (tjs_int)i, &value, array);
	}
	tTJSVariant result(array, array);
	array->Release();
	return result;
}

static tTJSVariant MakeMotionRenderMethodDumpArray(
	const std::vector<MotionRenderMethodItem> &items)
{
	iTJSDispatch2 *array = TJSCreateArrayObject();
	for (size_t i = 0; i < items.size(); i++) {
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		SetTjsProperty(dict, TJS_W("index"), tTJSVariant((tjs_int)i));
		SetTjsProperty(dict, TJS_W("type"), tTJSVariant(items[i].type));
		SetTjsProperty(dict, TJS_W("label"), tTJSVariant(items[i].label));
		SetTjsProperty(dict, TJS_W("sourceKey"), tTJSVariant(MotionString(items[i].srcKey)));
		SetTjsProperty(dict, TJS_W("originX"), tTJSVariant(items[i].originX));
		SetTjsProperty(dict, TJS_W("originY"), tTJSVariant(items[i].originY));
		SetTjsProperty(dict, TJS_W("width"), tTJSVariant(items[i].width));
		SetTjsProperty(dict, TJS_W("height"), tTJSVariant(items[i].height));
		SetTjsProperty(dict, TJS_W("opacity"), tTJSVariant(items[i].opacity));
		SetTjsProperty(dict, TJS_W("isFromLayout"), tTJSVariant((tjs_int)(items[i].isFromLayout ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("isFromNestedMotion"), tTJSVariant((tjs_int)(items[i].isFromNestedMotion ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("linkedToLayout"), tTJSVariant((tjs_int)(items[i].linkedToLayout ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("layoutIndex"), tTJSVariant(items[i].layoutIndex));
		SetTjsProperty(dict, TJS_W("sourceNodeIndex"), tTJSVariant(items[i].sourceNodeIndex));
		SetTjsProperty(dict, TJS_W("drawPriority"), tTJSVariant(items[i].drawPriority));
		SetTjsProperty(dict, TJS_W("surfaceCount"), tTJSVariant((tjs_int)items[i].surfaces.size()));
		tTJSVariant value(dict, dict);
		dict->Release();
		array->PropSetByNum(TJS_MEMBERENSURE, (tjs_int)i, &value, array);
	}
	tTJSVariant result(array, array);
	array->Release();
	return result;
}

static tTJSVariant MakeMotionObjectGraphDiagDump(const MotionObjectGraphDiag &diag)
{
	iTJSDispatch2 *dict = TJSCreateDictionaryObject();
	SetTjsProperty(dict, TJS_W("graphBuilt"), tTJSVariant((tjs_int)(diag.graphBuilt ? 1 : 0)));
	SetTjsProperty(dict, TJS_W("objectCount"), tTJSVariant(diag.objectCount));
	SetTjsProperty(dict, TJS_W("motionCount"), tTJSVariant(diag.motionCount));
	SetTjsProperty(dict, TJS_W("nodeCount"), tTJSVariant(diag.nodeCount));
	SetTjsProperty(dict, TJS_W("motionRefCount"), tTJSVariant(diag.motionRefCount));
	SetTjsProperty(dict, TJS_W("resolvedMotionRefCount"), tTJSVariant(diag.resolvedMotionRefCount));
	SetTjsProperty(dict, TJS_W("unresolvedMotionRefCount"), tTJSVariant(diag.unresolvedMotionRefCount));
	SetTjsProperty(dict, TJS_W("externalMotionRefCount"), tTJSVariant(diag.externalMotionRefCount));
	SetTjsProperty(dict, TJS_W("sourceIconCount"), tTJSVariant(diag.sourceIconCount));
	SetTjsProperty(dict, TJS_W("sourceAliasCount"), tTJSVariant(diag.sourceAliasCount));
	SetTjsProperty(dict, TJS_W("sourceKeysUsedCount"), tTJSVariant(diag.sourceKeysUsedCount));
	SetTjsProperty(dict, TJS_W("sourceKeysResolvedCount"), tTJSVariant(diag.sourceKeysResolvedCount));
	SetTjsProperty(dict, TJS_W("sourceKeysUnresolvedCount"), tTJSVariant(diag.sourceKeysUnresolvedCount));
	SetTjsProperty(dict, TJS_W("layoutCount"), tTJSVariant(diag.layoutCount));
	SetTjsProperty(dict, TJS_W("layoutWithObjectCount"), tTJSVariant(diag.layoutWithObjectCount));
	SetTjsProperty(dict, TJS_W("layoutWithMotionCount"), tTJSVariant(diag.layoutWithMotionCount));
	SetTjsProperty(dict, TJS_W("layoutWithSourceKeyCount"), tTJSVariant(diag.layoutWithSourceKeyCount));
	SetTjsProperty(dict, TJS_W("layoutWithRenderMethodLinkCount"), tTJSVariant(diag.layoutWithRenderMethodLinkCount));
	SetTjsProperty(dict, TJS_W("renderMethodCount"), tTJSVariant(diag.renderMethodCount));
	SetTjsProperty(dict, TJS_W("renderMethodWithSourceKeyCount"), tTJSVariant(diag.renderMethodWithSourceKeyCount));
	SetTjsProperty(dict, TJS_W("renderMethodLinkedLayoutCount"), tTJSVariant(diag.renderMethodLinkedLayoutCount));
	SetTjsProperty(dict, TJS_W("skipReason"), tTJSVariant(MotionString(diag.skipReason)));
	tTJSVariant result(dict, dict);
	dict->Release();
	return result;
}

static tTJSVariant MakeMotionResourceDump(const MotionResource &resource)
{
	iTJSDispatch2 *dict = TJSCreateDictionaryObject();
	SetTjsProperty(dict, TJS_W("id"), tTJSVariant((tjs_int)resource.id));
	SetTjsProperty(dict, TJS_W("storage"), tTJSVariant(resource.storage));
	SetTjsProperty(dict, TJS_W("byteSize"), tTJSVariant((tjs_int64)resource.byteSize));
	SetTjsProperty(dict, TJS_W("valid"), tTJSVariant((tjs_int)(resource.opened ? 1 : 0)));
	SetTjsProperty(dict, TJS_W("parsed"), tTJSVariant((tjs_int)(resource.parsed ? 1 : 0)));
	SetTjsProperty(dict, TJS_W("isMotion"), tTJSVariant((tjs_int)(resource.isMotion ? 1 : 0)));
	SetTjsProperty(dict, TJS_W("hasMetadata"), tTJSVariant((tjs_int)(resource.hasMetadata ? 1 : 0)));
	SetTjsProperty(dict, TJS_W("spec"), tTJSVariant(resource.spec));
	SetTjsProperty(dict, TJS_W("label"), tTJSVariant(resource.label));
	SetTjsProperty(dict, TJS_W("chara"), tTJSVariant(resource.chara));
	SetTjsProperty(dict, TJS_W("motion"), tTJSVariant(resource.motion));
	SetTjsProperty(dict, TJS_W("width"), tTJSVariant(resource.width));
	SetTjsProperty(dict, TJS_W("height"), tTJSVariant(resource.height));
	SetTjsProperty(dict, TJS_W("objectCount"), tTJSVariant((tjs_int)resource.objects.size()));
	SetTjsProperty(dict, TJS_W("motionCount"), tTJSVariant((tjs_int)resource.motions.size()));
	SetTjsProperty(dict, TJS_W("variableCount"), tTJSVariant((tjs_int)resource.variables.size()));
	SetTjsProperty(dict, TJS_W("selectorCount"), tTJSVariant((tjs_int)resource.selectors.size()));
	SetTjsProperty(dict, TJS_W("timelineCount"), tTJSVariant((tjs_int)resource.timelines.size()));
	SetTjsProperty(dict, TJS_W("layoutCount"), tTJSVariant((tjs_int)resource.layouts.size()));
	SetTjsProperty(dict, TJS_W("sourceIconCount"), tTJSVariant((tjs_int)resource.sourceIcons.size()));
	SetTjsProperty(dict, TJS_W("sourceAliasCount"), tTJSVariant(resource.sourceAliasCount));
	SetTjsProperty(dict, TJS_W("attachmentRefCount"), tTJSVariant((tjs_int)resource.attachmentRefs.size()));
	SetTjsProperty(dict, TJS_W("renderMethodCount"), tTJSVariant((tjs_int)resource.renderMethodIR.size()));
	SetTjsProperty(dict, TJS_W("objects"), MakeMotionObjectDumpArray(resource.objects));
	SetTjsProperty(dict, TJS_W("motions"), MakeMotionDumpArray(resource.motions));
	SetTjsProperty(dict, TJS_W("variables"), MakeMotionVariableDumpArray(resource.variables));
	SetTjsProperty(dict, TJS_W("selectors"), MakeMotionSelectorDumpArray(resource.selectors));
	SetTjsProperty(dict, TJS_W("timelines"), MakeMotionTimelineDumpArray(resource.timelines));
	SetTjsProperty(dict, TJS_W("layouts"), MakeMotionLayoutDumpArray(resource.layouts));
	SetTjsProperty(dict, TJS_W("sourceIcons"), MakeMotionSourceIconDumpArray(resource.sourceIcons));
	SetTjsProperty(dict, TJS_W("attachmentRefs"), MakeMotionAttachmentRefDumpArray(resource.attachmentRefs));
	SetTjsProperty(dict, TJS_W("renderMethods"), MakeMotionRenderMethodDumpArray(resource.renderMethodIR));
	SetTjsProperty(dict, TJS_W("objectGraph"), MakeMotionObjectGraphDiagDump(resource.objectGraphDiag));
	SetTjsProperty(dict, TJS_W("rendererPrep"),
		MakeMotionRendererPrepDump(&resource, 1.0, false));
	tTJSVariant result(dict, dict);
	dict->Release();
	return result;
}
