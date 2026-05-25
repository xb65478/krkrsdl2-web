static bool ProbeStorage(const ttstr &path, tjs_uint64 *size)
{
	tTJSBinaryStream *stream = nullptr;
	try {
		stream = TVPCreateStream(path);
	} catch (...) {
		stream = nullptr;
	}
	if (!stream) return false;
	if (size) *size = stream->GetSize();
	delete stream;
	return true;
}

static bool CallTjsMethod(iTJSDispatch2 *target, const tjs_char *name, tjs_int argc = 0, tTJSVariant **argv = nullptr)
{
	if (!target) return false;
	tjs_error hr = TJS_S_OK;
	try {
		hr = target->FuncCall(0, name, nullptr, nullptr, argc, argv, target);
	} catch (...) {
		TVPAddLog(ttstr(TJS_W("[emoteplayer] method threw: ")) + ttstr(name));
		return false;
	}
	if (TJS_FAILED(hr)) {
		TVPAddLog(ttstr(TJS_W("[emoteplayer] method call failed: ")) + ttstr(name));
		return false;
	}
	return true;
}

class MotionScopedDrawableLayerType {
public:
	explicit MotionScopedDrawableLayerType(tTJSNI_BaseLayer *layer)
		: layer(layer)
	{
		if (!layer) return;
		originalType = layer->GetType();
		originalWidth = (tjs_int)layer->GetWidth();
		originalHeight = (tjs_int)layer->GetHeight();
		originalHasImage = layer->GetHasImage();
		if (originalHasImage) {
			originalImageWidth = (tjs_int)layer->GetImageWidth();
			originalImageHeight = (tjs_int)layer->GetImageHeight();
		}
		if (originalType == ltBinder || originalType == ltEffect ||
			originalType == ltFilter || !originalHasImage) {
			layer->SetType(ltAlpha);
			changed = true;
			tjs_int width = originalImageWidth > 0 ? originalImageWidth : originalWidth;
			tjs_int height = originalImageHeight > 0 ? originalImageHeight : originalHeight;
			if (!layer->GetHasImage()) {
				layer->SetHasImage(true);
			}
			if (width > 0 && height > 0) {
				tjs_int imageWidth = layer->GetHasImage() ? (tjs_int)layer->GetImageWidth() : 0;
				tjs_int imageHeight = layer->GetHasImage() ? (tjs_int)layer->GetImageHeight() : 0;
				if (imageWidth != width || imageHeight != height) {
					layer->SetImageSize((tjs_uint)width, (tjs_uint)height);
				}
			}
		}
	}

	~MotionScopedDrawableLayerType()
	{
		if (!layer || !changed) return;
		if (originalType == ltBinder || originalType == ltEffect ||
			originalType == ltFilter) {
			return;
		}
		layer->SetType(originalType);
	}

	bool changedType() const { return changed; }

private:
	tTJSNI_BaseLayer *layer = nullptr;
	tTVPLayerType originalType = ltAlpha;
	tjs_int originalWidth = 0;
	tjs_int originalHeight = 0;
	tjs_int originalImageWidth = 0;
	tjs_int originalImageHeight = 0;
	bool originalHasImage = false;
	bool changed = false;
};

static void SetTjsProperty(iTJSDispatch2 *target, const tjs_char *name, const tTJSVariant &value)
{
	if (!target) return;
	tTJSVariant v(value);
	target->PropSet(TJS_MEMBERENSURE, name, nullptr, &v, target);
}

static tTJSVariant MakeMotionRenderSurfaceDumpArray(const std::vector<MotionRenderSurface> &surfaces)
{
	iTJSDispatch2 *array = TJSCreateArrayObject();
	for (size_t i = 0; i < surfaces.size(); i++) {
		size_t renderIndex = surfaces.size() - 1 - i;
		const MotionRenderSurface &surface = surfaces[renderIndex];
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		SetTjsProperty(dict, TJS_W("surfaceIndex"), tTJSVariant((tjs_int)i));
		SetTjsProperty(dict, TJS_W("renderIndex"), tTJSVariant((tjs_int)renderIndex));
		SetTjsProperty(dict, TJS_W("type"), tTJSVariant(surface.type));
		SetTjsProperty(dict, TJS_W("opacity"), tTJSVariant(surface.opa));
		SetTjsProperty(dict, TJS_W("originX"), tTJSVariant(surface.originX));
		SetTjsProperty(dict, TJS_W("originY"), tTJSVariant(surface.originY));
		SetTjsProperty(dict, TJS_W("width"), tTJSVariant(surface.originW));
		SetTjsProperty(dict, TJS_W("height"), tTJSVariant(surface.originH));
		SetTjsProperty(dict, TJS_W("hasStencil"), tTJSVariant((tjs_int)(surface.hasStencil ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("currCoordz"), tTJSVariant(surface.currCoordz));
		SetTjsProperty(dict, TJS_W("label"), tTJSVariant(surface.label));
		SetTjsProperty(dict, TJS_W("m00"), tTJSVariant(surface.matTrans.m[0]));
		SetTjsProperty(dict, TJS_W("m01"), tTJSVariant(surface.matTrans.m[1]));
		SetTjsProperty(dict, TJS_W("m02"), tTJSVariant(surface.matTrans.m[2]));
		SetTjsProperty(dict, TJS_W("m03"), tTJSVariant(surface.matTrans.m[3]));
		SetTjsProperty(dict, TJS_W("m10"), tTJSVariant(surface.matTrans.m[4]));
		SetTjsProperty(dict, TJS_W("m11"), tTJSVariant(surface.matTrans.m[5]));
		SetTjsProperty(dict, TJS_W("m12"), tTJSVariant(surface.matTrans.m[6]));
		SetTjsProperty(dict, TJS_W("m13"), tTJSVariant(surface.matTrans.m[7]));
		SetTjsProperty(dict, TJS_W("m20"), tTJSVariant(surface.matTrans.m[8]));
		SetTjsProperty(dict, TJS_W("m21"), tTJSVariant(surface.matTrans.m[9]));
		SetTjsProperty(dict, TJS_W("m22"), tTJSVariant(surface.matTrans.m[10]));
		SetTjsProperty(dict, TJS_W("m23"), tTJSVariant(surface.matTrans.m[11]));
		SetTjsProperty(dict, TJS_W("m30"), tTJSVariant(surface.matTrans.m[12]));
		SetTjsProperty(dict, TJS_W("m31"), tTJSVariant(surface.matTrans.m[13]));
		SetTjsProperty(dict, TJS_W("m32"), tTJSVariant(surface.matTrans.m[14]));
		SetTjsProperty(dict, TJS_W("m33"), tTJSVariant(surface.matTrans.m[15]));
		iTJSDispatch2 *controlArray = TJSCreateArrayObject();
		for (size_t ci = 0; ci < 32; ci++) {
			tTJSVariant cp(surface.controlPts[ci]);
			controlArray->PropSetByNum(TJS_MEMBERENSURE, (tjs_int)ci, &cp, controlArray);
		}
		tTJSVariant controlArrayVal(controlArray, controlArray);
		controlArray->Release();
		SetTjsProperty(dict, TJS_W("controlPts"), controlArrayVal);
		tTJSVariant dictVal(dict, dict);
		dict->Release();
		array->PropSetByNum(TJS_MEMBERENSURE, (tjs_int)i, &dictVal, array);
	}
	tTJSVariant arrayVal(array, array);
	array->Release();
	return arrayVal;
}

static bool GetTjsProperty(iTJSDispatch2 *target, const tjs_char *name, tTJSVariant &value)
{
	if (!target) return false;
	try {
		return TJS_SUCCEEDED(target->PropGet(TJS_IGNOREPROP, name, nullptr, &value, target));
	} catch (...) {
		value.Clear();
		return false;
	}
}

static bool MotionReadTargetRealProperty(iTJSDispatch2 *target,
	const tjs_char *name, tjs_real &out)
{
	tTJSVariant value;
	if (!GetTjsProperty(target, name, value) || value.Type() == tvtVoid) {
		return false;
	}
	try {
		tjs_real number = (tjs_real)value;
		if (!std::isfinite((double)number)) return false;
		out = number;
		return true;
	} catch (...) {
		return false;
	}
}

static void MotionResolveRuntimeDrawLimit(iTJSDispatch2 *target,
	tjs_real fallbackWidth, tjs_real fallbackHeight,
	tjs_real &originX, tjs_real &originY,
	tjs_real &width, tjs_real &height,
	tjs_real &zMax)
{
	originX = 0.0;
	originY = 0.0;
	width = fallbackWidth > 0.0 ? fallbackWidth : 1280.0;
	height = fallbackHeight > 0.0 ? fallbackHeight : 720.0;
	zMax = kMotionDefaultLimitZMax;

	tTJSNI_BaseLayer *nativeLayer = MotionGetNativeLayer(target);
	if (nativeLayer) {
		tjs_real layerWidth = (tjs_real)nativeLayer->GetWidth();
		tjs_real layerHeight = (tjs_real)nativeLayer->GetHeight();
		if (nativeLayer->GetHasImage()) {
			tjs_real imageWidth = (tjs_real)nativeLayer->GetImageWidth();
			tjs_real imageHeight = (tjs_real)nativeLayer->GetImageHeight();
			if (imageWidth > 0.0) width = imageWidth;
			else if (layerWidth > 0.0) width = layerWidth;
			if (imageHeight > 0.0) height = imageHeight;
			else if (layerHeight > 0.0) height = layerHeight;
		} else {
			if (layerWidth > 0.0) width = layerWidth;
			if (layerHeight > 0.0) height = layerHeight;
		}
	}

	tjs_real prop = 0.0;
	if (MotionReadTargetRealProperty(target, TJS_W("imageWidth"), prop) && prop > 0.0) width = prop;
	if (MotionReadTargetRealProperty(target, TJS_W("imageHeight"), prop) && prop > 0.0) height = prop;
	if (MotionReadTargetRealProperty(target, TJS_W("width"), prop) && width <= 0.0 && prop > 0.0) width = prop;
	if (MotionReadTargetRealProperty(target, TJS_W("height"), prop) && height <= 0.0 && prop > 0.0) height = prop;
}

static bool GetTjsVariantObjectProperty(tTJSVariant object,
	const tjs_char *name, tTJSVariant &value)
{
	if (object.Type() != tvtObject) return false;
	tTJSVariantClosure &closure = object.AsObjectClosureNoAddRef();
	if (!closure.Object) return false;
	return TJS_SUCCEEDED(closure.PropGet(
		TJS_IGNOREPROP, name, nullptr, &value, closure.ObjThis));
}

static bool GetTjsVariantArrayElement(tTJSVariant object,
	tjs_int index, tTJSVariant &value)
{
	if (object.Type() != tvtObject) return false;
	tTJSVariantClosure &closure = object.AsObjectClosureNoAddRef();
	if (!closure.Object) return false;
	return TJS_SUCCEEDED(closure.PropGetByNum(
		TJS_IGNOREPROP, index, &value, closure.ObjThis));
}

