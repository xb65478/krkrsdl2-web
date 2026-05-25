#include "ncbind/ncbind.hpp"
#include "tjsArray.h"
#include "tjsDictionary.h"
#include "StorageIntf.h"
#include "MsgIntf.h"
#include "LayerIntf.h"
#include "LayerBitmapIntf.h"
#include "CharacterSet.h"
#include "GraphicsLoaderIntf.h"
#include "LogFilter.h"
#include "TickCount.h"
#include "psb.hpp"
#include "../psb/psb_static_motion_bridge.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include "ScriptMgnIntf.h"
#include "SysInitIntf.h"
#include "WindowIntf.h"
#include "DrawDevice.h"
#endif
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <limits>
#include <map>
#include <memory>
#include <stdio.h>
#include <string>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

#include "motion_math.h"
#include "motion_types.h"

#define NCB_MODULE_NAME TJS_W("emoteplayer.dll")

extern void TVPLoadPSBFilePlugin();
extern void TVPSetPSBStaticMotionHint(const tjs_char *storage, const tjs_char *objectName,
	const tjs_char *motionName, double frameTime);
extern void TVPClearPSBStaticMotionHint();

namespace emoteplayer {

static bool MotionRunM2ProgressSurfaceSelfTest(MotionRenderMethodSemanticsDiag &diag);
static bool MotionRunM3DemuxSurfaceSelfTest(MotionRenderMethodSemanticsDiag &diag);
static bool MotionRunM4RendererUploadSelfTest(MotionRenderMethodSemanticsDiag &diag);
static bool MotionRunM5BezierSurfaceChainSelfTest(MotionRenderMethodSemanticsDiag &diag);
static const TVPStaticMotionBridgeItem *MotionFindSnapshotItemForSourceKey(
	const TVPStaticMotionBridgeSnapshot *snapshot,
	const std::string &key);
static tTJSNI_BaseLayer *MotionGetNativeLayer(iTJSDispatch2 *target);

// This file intentionally remains the only translation unit for the WASM
// Emote runtime.  The .inc files below are logical slices ordered by their
// dependency flow; do not include them independently.
#include "motion_trace_bootstrap.inc"
#include "motion_psb_parse.inc"
#include "motion_layout_runtime.inc"
#include "motion_tjs_core.inc"
#include "motion_front_collect.inc"
#include "motion_renderer.inc"
#include "motion_tjs_dump.inc"
#include "emoteplayer_resource_runtime.inc"
#include "motion_front_hit.inc"
#include "emoteplayer_runtime.inc"

} // namespace emoteplayer

// Web exports and ncbind registration need the complete class definitions.
#include "emoteplayer_web_bridge.inc"
#include "emoteplayer_registration.inc"
