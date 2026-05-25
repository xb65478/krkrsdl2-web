#include "ncbind/ncbind.hpp"

#define NCB_MODULE_NAME TJS_W("addFont.dll")

struct FontEx
{
	static tjs_error TJS_INTF_METHOD addFont(tTJSVariant *result,
											 tjs_int numparams,
											 tTJSVariant **param,
											 iTJSDispatch2 *objthis) {
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;
		ttstr filename = *param[0];
		TVPAddLog(ttstr(TJS_W("[addFont] Font loading not supported in WASM: ")) + filename);
		if (result) *result = (int)0;
		return TJS_S_OK;
	}
};

NCB_ATTACH_CLASS(FontEx, System) {
	RawCallback("addFont", &FontEx::addFont, TJS_STATICMEMBER);
}
