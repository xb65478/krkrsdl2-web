#define NCB_MODULE_NAME TJS_W("extrans.dll")
#include "ncbind.hpp"

#include "wave.h"
#include "mosaic.h"
#include "turn.h"
#include "rotatetrans.h"
#include "ripple.h"

static void extrans_startup()
{
	RegisterWaveTransHandlerProvider();
	RegisterMosaicTransHandlerProvider();
	RegisterTurnTransHandlerProvider();
	RegisterRotateTransHandlerProvider();
	RegisterRippleTransHandlerProvider();
}

static void extrans_cleanup()
{
	UnregisterWaveTransHandlerProvider();
	UnregisterMosaicTransHandlerProvider();
	UnregisterTurnTransHandlerProvider();
	UnregisterRotateTransHandlerProvider();
	UnregisterRippleTransHandlerProvider();
}

NCB_PRE_REGIST_CALLBACK(extrans_startup);
NCB_POST_UNREGIST_CALLBACK(extrans_cleanup);
