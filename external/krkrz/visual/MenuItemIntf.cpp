#include "tjsCommHead.h"

#include "MenuItemIntf.h"
#include "MsgIntf.h"
#include "EventIntf.h"

tTJSNI_MenuItem::tTJSNI_MenuItem()
{
}

tjs_error TJS_INTF_METHOD
	tTJSNI_MenuItem::Construct(tjs_int numparams, tTJSVariant **param,
	iTJSDispatch2 *tjs_obj) {

	return TJS_S_OK;
}
void TJS_INTF_METHOD tTJSNI_MenuItem::Invalidate() {
}

void tTJSNI_MenuItem::PopupEx(tjs_int x, tjs_int y) {
}

void tTJSNI_MenuItem::Add(tTJSNI_MenuItem* item) {
}

void tTJSNI_MenuItem::Remove(tTJSNI_MenuItem* item) {
}

tjs_uint32 tTJSNC_MenuItem::ClassID = -1;
tTJSNC_MenuItem::tTJSNC_MenuItem() : inherited(TJS_W("MenuItem")) {
	// registration of native members

	TJS_BEGIN_NATIVE_MEMBERS(MenuItem) // constructor
	TJS_DECL_EMPTY_FINALIZE_METHOD
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var.name*/_this, /*var.type*/tTJSNI_MenuItem,
	/*TJS class name*/MenuItem)
{
	return TJS_S_OK;
}
TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/MenuItem)
//----------------------------------------------------------------------

//-- methods

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/popupEx)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
	if(numparams < 2) return TJS_E_BADPARAMCOUNT;
	_this->PopupEx(*param[0], *param[1]);
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/popupEx)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/add)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
	tTJSNI_MenuItem* item = NULL;
	if( clo.Object ) {
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
			tTJSNC_MenuItem::ClassID, (iTJSNativeInstance**)&item))) {

			return TJS_E_INVALIDPARAM;
		}
		_this->Add(item);
	}
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/add)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/remove)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
	tTJSNI_MenuItem* item = NULL;
	if( clo.Object ) {
		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
			tTJSNC_MenuItem::ClassID, (iTJSNativeInstance**)&item))) {
			return TJS_E_INVALIDPARAM;
		}
		_this->Remove(item);
	}
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/remove)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/fireClick)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_MenuItem);
	(void)_this;
	bool delivered = false;
	if(objthis)
	{
		tTJSVariant click;
		if(TJS_SUCCEEDED(objthis->PropGet(0, TJS_W("click"), NULL, &click, objthis)) &&
			click.Type() == tvtObject)
		{
			tTJSVariantClosure clo = click.AsObjectClosureNoAddRef();
			if(clo.Object)
			{
				clo.FuncCall(0, NULL, NULL, NULL, 0, NULL, objthis);
				delivered = true;
			}
		}
		if(!delivered)
		{
			tTJSVariant onClick;
			if(TJS_SUCCEEDED(objthis->PropGet(0, TJS_W("onClick"), NULL, &onClick, objthis)) &&
				onClick.Type() == tvtObject)
			{
				tTJSVariantClosure clo = onClick.AsObjectClosureNoAddRef();
				if(clo.Object)
				{
					clo.FuncCall(0, NULL, NULL, NULL, 0, NULL, objthis);
				}
			}
		}
	}
	TVPRequestSaveSystemVariables("MenuItem.fireClick");
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/fireClick)
//----------------------------------------------------------------------

//-- properties (none for now)

	TJS_END_NATIVE_MEMBERS
}

tTJSNativeInstance *tTJSNC_MenuItem::CreateNativeInstance() {
	return new tTJSNI_MenuItem();
}

//---------------------------------------------------------------------------
iTJSDispatch2 * TVPCreateMenuItemObject()
{
	struct tHolder
	{
		iTJSDispatch2 * Obj;
		tHolder() { Obj = new tTJSNC_MenuItem(); }
		~tHolder() { Obj->Release(); }
	} static menuitemclass;

	iTJSDispatch2 *out;
	tjs_error hr = menuitemclass.Obj->CreateNew(0, NULL, NULL, &out, 0, NULL, menuitemclass.Obj);
	if(TJS_FAILED(hr)) TVPThrowInternalError;

	return out;
}

tTJSNativeClass * TVPCreateNativeClass_MenuItem()
{
	tTJSNativeClass *cls = new tTJSNC_MenuItem();
	return cls;
}
