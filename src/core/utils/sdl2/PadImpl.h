//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000-2007 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Pad stub for non-Win32 platforms
//---------------------------------------------------------------------------
#ifndef PadImplH
#define PadImplH

#include "tjsNative.h"

class tTJSNI_Pad : public tTJSNativeInstance
{
public:
	virtual tjs_error TJS_INTF_METHOD
		Construct(tjs_int numparams, tTJSVariant **param,
		iTJSDispatch2 *dsp) { return TJS_S_OK; }
	virtual void TJS_INTF_METHOD Invalidate() {}
};

#endif
