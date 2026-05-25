#ifndef _layExBase_hpp_
#define _layExBase_hpp_

#include "tjsCommHead.h"
#include "tjsNative.h"
#include "ScriptMgnIntf.h"
#include "MsgIntf.h"
#include "LayerIntf.h"

struct layerExBase
{
    typedef iTJSDispatch2* DispatchT;
    typedef tjs_int        GeometryT;
    typedef tjs_int        PitchT;
    typedef unsigned char* BufferT;

    tTJSNI_Layer *_this;

    GeometryT _width, _height;
    BufferT   _buffer;
    PitchT    _pitch;

    GeometryT _clipLeft, _clipTop, _clipWidth, _clipHeight;
    DispatchT _obj;

    layerExBase(DispatchT obj)
        : _obj(obj), _buffer(nullptr), _pitch(0)
    {
        tjs_error hr;
        hr = obj->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
            tTJSNC_Layer::ClassID, (iTJSNativeInstance**)&_this);
        if(TJS_FAILED(hr)) TVPThrowExceptionMessage(TJS_W("Not Layer"));
        reset();
    }

    virtual ~layerExBase() {}

    virtual void reset() {
        _width  = (GeometryT)_this->GetWidth();
        _height = (GeometryT)_this->GetHeight();
        _buffer = (BufferT)_this->GetMainImagePixelBufferForWrite();
        _pitch  = (PitchT)_this->GetMainImagePixelBufferPitch();
        _clipLeft   = (GeometryT)_this->GetClipLeft();
        _clipTop    = (GeometryT)_this->GetClipTop();
        _clipWidth  = (GeometryT)_this->GetClipWidth();
        _clipHeight = (GeometryT)_this->GetClipHeight();
    }

    virtual void redraw() {
        tTVPRect rc(_clipLeft, _clipTop, _clipLeft+_clipWidth, _clipTop+_clipHeight);
        _this->Update(rc);
    }
};

#endif
