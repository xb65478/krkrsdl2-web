#define NCB_MODULE_NAME TJS_W("layerExRaster.dll")
#include "ncbind.hpp"
#define _USE_MATH_DEFINES
#include <math.h>
#include <cstring>

#include "layerExBase.hpp"

struct layerExRaster : public layerExBase
{
public:
	layerExRaster(DispatchT obj) : layerExBase(obj) {}

	void copyRaster(tTJSVariant layer, int maxh, int lines, int cycle, tjs_int64 time) {

		tjs_int width, height, pitch;
		unsigned char* buffer;
		{
			iTJSDispatch2 *layerobj = layer.AsObjectNoAddRef();
			tTJSVariant var;
			layerobj->PropGet(0, TJS_W("imageWidth"), NULL, &var, layerobj);
			width = (tjs_int)var;
			layerobj->PropGet(0, TJS_W("imageHeight"), NULL, &var, layerobj);
			height = (tjs_int)var;
			layerobj->PropGet(0, TJS_W("mainImageBuffer"), NULL, &var, layerobj);
			buffer = (unsigned char*)(tjs_intptr_t)(tjs_int64)var;
			layerobj->PropGet(0, TJS_W("mainImageBufferPitch"), NULL, &var, layerobj);
			pitch = (tjs_int)var;
		}

		if (_width != width || _height != height) {
			return;
		}

		double omega = 2 * M_PI / lines;
		tjs_int CurH = (tjs_int)maxh;
		double rad = - omega * time / cycle * (height/2);

		rad += omega * _clipTop;
		unsigned char *dstBuf = _buffer + _pitch * _clipTop + _clipLeft * 4;
		unsigned char *srcBuf = buffer  +  pitch * _clipTop + _clipLeft * 4;

		tjs_int n;
		for (n = 0; n < _clipHeight; n++, rad += omega) {
			tjs_int d = (tjs_int)(sin(rad) * CurH);
			if (d >= 0) {
				int w = _clipWidth - d;
				if (w > 0) {
					const tjs_uint32 *src = (const tjs_uint32*)(srcBuf + n * pitch);
					tjs_uint32 *dest = (tjs_uint32 *)(dstBuf + n * _pitch) + d;
					memcpy(dest, src, w * sizeof(tjs_uint32));
				}
			} else {
				int w = _clipWidth + d;
				if (w > 0) {
					const tjs_uint32 *src = (const tjs_uint32*)(srcBuf + n * pitch) - d;
					tjs_uint32 *dest = (tjs_uint32 *)(dstBuf + n * _pitch);
					memcpy(dest, src, w * sizeof(tjs_uint32));
				}
			}
		}

		redraw();
	}
};

NCB_GET_INSTANCE_HOOK(layerExRaster)
{
	NCB_INSTANCE_GETTER(objthis) {
		ClassT* obj = GetNativeInstance(objthis);
		if (!obj) {
			obj = new ClassT(objthis);
			SetNativeInstance(objthis, obj);
		}
		obj->reset();
		return obj;
	}
	~NCB_GET_INSTANCE_HOOK_CLASS () {
	}
};

NCB_ATTACH_CLASS_WITH_HOOK(layerExRaster, Layer) {
	NCB_METHOD(copyRaster);
}
