#ifndef MenuItemIntfH
#define MenuItemIntfH

#include "tjsNative.h"

class tTJSNI_MenuItem : public tTJSNativeInstance
{
	typedef tTJSNativeInstance inherited;

public:
	tTJSNI_MenuItem();
	tjs_error TJS_INTF_METHOD
		Construct(tjs_int numparams, tTJSVariant **param,
			iTJSDispatch2 *tjs_obj);
	void TJS_INTF_METHOD Invalidate();

	void PopupEx(tjs_int x, tjs_int y);
	void Add(tTJSNI_MenuItem* item);
	void Remove(tTJSNI_MenuItem* item);
};


class tTJSNC_MenuItem : public tTJSNativeClass
{
	typedef tTJSNativeClass inherited;

public:
	tTJSNC_MenuItem();
	static tjs_uint32 ClassID;

protected:
	tTJSNativeInstance *CreateNativeInstance();
};

extern iTJSDispatch2 * TVPCreateMenuItemObject();
extern tTJSNativeClass * TVPCreateNativeClass_MenuItem();

#endif // MenuItemIntfH