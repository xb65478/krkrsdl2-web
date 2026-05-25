//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// "Plugins" class implementation / Service for plug-ins
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include <algorithm>
#include <functional>
#include "ScriptMgnIntf.h"
#include "PluginImpl.h"
#include "StorageImpl.h"
#include "GraphicsLoaderImpl.h"

#include "MsgImpl.h"
#include "SysInitIntf.h"

#include "tjsHashSearch.h"
#include "EventIntf.h"
#include "TransIntf.h"
#include "tjsArray.h"
#include "tjsDictionary.h"
#include "DebugIntf.h"
#ifdef KRKRSDL2_ENABLE_PLUGINS
#include "FuncStubs.h"
#endif
#include "tjs.h"

#ifdef _WIN32
#if 0
#ifdef TVP_SUPPORT_OLD_WAVEUNPACKER
	#include "oldwaveunpacker.h"
#endif
#endif

#pragma pack(push, 8)
	///  tvpsnd.h needs packing size of 8
	#include "tvpsnd.h"
#pragma pack(pop)

#if 0
#ifdef TVP_SUPPORT_KPI
	#include "kmp_pi.h"
#endif
#endif
#endif

#include "FilePathUtil.h"
#include "Application.h"
#include "SysInitImpl.h"
#include "LogFilter.h"
#include <set>
#include <string>


//---------------------------------------------------------------------------
// export table
//---------------------------------------------------------------------------
#ifdef KRKRSDL2_ENABLE_PLUGINS
static tTJSHashTable<ttstr, void *> TVPExportFuncs;
static bool TVPExportFuncsInit = false;
#endif
void TVPAddExportFunction(const char *name, void *ptr)
{
#ifdef KRKRSDL2_ENABLE_PLUGINS
	TVPExportFuncs.Add(name, ptr);
#endif
}
void TVPAddExportFunction(const tjs_char *name, void *ptr)
{
#ifdef KRKRSDL2_ENABLE_PLUGINS
	TVPExportFuncs.Add(name, ptr);
#endif
}
#ifdef KRKRSDL2_ENABLE_PLUGINS
static void TVPInitExportFuncs()
{
	if(TVPExportFuncsInit) return;
	TVPExportFuncsInit = true;


	// Export functions
	TVPExportFunctions();
}
//---------------------------------------------------------------------------
struct tTVPFunctionExporter : iTVPFunctionExporter
{
	bool TJS_INTF_METHOD QueryFunctions(const tjs_char **name, void **function,
		tjs_uint count);
	bool TJS_INTF_METHOD QueryFunctionsByNarrowString(const char **name,
		void **function, tjs_uint count);
} static TVPFunctionExporter;
//---------------------------------------------------------------------------
bool TJS_INTF_METHOD tTVPFunctionExporter::QueryFunctions(const tjs_char **name, void **function,
		tjs_uint count)
{
	// retrieve function table by given name table.
	// return false if any function is missing.
	bool ret = true;
	ttstr tname;
	for(tjs_uint i = 0; i<count; i++)
	{
		tname = name[i];
		void ** ptr = TVPExportFuncs.Find(tname);
		if(ptr)
			function[i] = *ptr;
		else
			function[i] = NULL, ret= false;
	}
	return ret;
}
//---------------------------------------------------------------------------
bool TJS_INTF_METHOD tTVPFunctionExporter::QueryFunctionsByNarrowString(
	const char **name, void **function, tjs_uint count)
{
	// retrieve function table by given name table.
	// return false if any function is missing.
	bool ret = true;
	ttstr tname;
	for(tjs_uint i = 0; i<count; i++)
	{
		tname = name[i];
		void ** ptr = TVPExportFuncs.Find(tname);
		if(ptr)
			function[i] = *ptr;
		else
			function[i] = NULL, ret= false;
	}
	return ret;
}
//---------------------------------------------------------------------------
extern "C" iTVPFunctionExporter * TVPGetFunctionExporter()
{
	// for external applications
	TVPInitExportFuncs();
    return &TVPFunctionExporter;
}
//---------------------------------------------------------------------------
#endif


//---------------------------------------------------------------------------
void TVPThrowPluginUnboundFunctionError(const char *funcname)
{
	TVPThrowExceptionMessage(TVPPluginUnboundFunctionError, funcname);
}
//---------------------------------------------------------------------------
void TVPThrowPluginUnboundFunctionError(const tjs_char *funcname)
{
	TVPThrowExceptionMessage(TVPPluginUnboundFunctionError, funcname);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// implementation of IStorageProvider
//---------------------------------------------------------------------------
class tTVPStorageProvider : public ITSSStorageProvider
{
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **ppvObjOut)
	{
		if(!ppvObjOut) return E_INVALIDARG;

		*ppvObjOut = NULL;
		if(!memcmp(&iid, &IID_IUnknown, 16))
			*ppvObjOut = (IUnknown*)this;
		else if(!memcmp(&iid, &IID_ITSSStorageProvider, 16))
			*ppvObjOut = (ITSSStorageProvider*)this;

		if(*ppvObjOut)
		{
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE AddRef(void) { return 1; }
	ULONG STDMETHODCALLTYPE Release(void) { return 1; }

	HRESULT STDMETHODCALLTYPE GetStreamForRead(
		TSS_LPWSTR url,
		IUnknown * *stream);

	HRESULT STDMETHODCALLTYPE GetStreamForWrite(
		TSS_LPWSTR url,
		IUnknown * *stream) { return E_NOTIMPL; }

	HRESULT STDMETHODCALLTYPE GetStreamForUpdate(
		TSS_LPWSTR url,
		IUnknown * *stream) { return E_NOTIMPL; }
};
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE tTVPStorageProvider::GetStreamForRead(
		TSS_LPWSTR url,
		IUnknown * *stream)
{
	tTJSBinaryStream *stream0;
	try
	{
		stream0 = TVPCreateStream(url);
	}
	catch(...)
	{
		return E_FAIL;
	}

	IUnknown *istream = (IUnknown*)(IStream*)new tTVPIStreamAdapter(stream0);
	*stream = istream;

	return S_OK;
}
//---------------------------------------------------------------------------



#ifdef KRKRSDL2_ENABLE_PLUGINS
//---------------------------------------------------------------------------
// Plug-ins management
//---------------------------------------------------------------------------
struct tTVPPlugin
{
	ttstr Name;
	void *Instance = nullptr;

	tTVPPluginHolder *Holder = nullptr;

#ifdef _WIN32
	bool IsSusiePicturePlugin; // Susie picture plugins are managed in GraphicsLoaderImpl.cpp
	bool IsSusieArchivePlugin; // Susie archive plugins are managed in SusieArchive.cpp
#endif

	ITSSModule *TSSModule = nullptr;

#if 0
#ifdef TVP_SUPPORT_KPI
	KMPMODULE *KMPModule;
#endif
#endif

	tTVPV2LinkProc V2Link = nullptr;
	tTVPV2UnlinkProc V2Unlink = nullptr;


	tTVPGetModuleInstanceProc GetModuleInstance = nullptr;
#if 0
	tTVPGetModuleThreadModelProc GetModuleThreadModel = nullptr;
	tTVPShowConfigWindowProc ShowConfigWindow = nullptr;
	tTVPCanUnloadNowProc CanUnloadNow = nullptr;
#ifdef TVP_SUPPORT_OLD_WAVEUNPACKER
	tTVPCreateWaveUnpackerProc CreateWaveUnpacker = nullptr;
#endif

#ifdef TVP_SUPPORT_KPI
	pfnGetKMPModule GetKMPModule = nullptr
#endif
#endif

	std::vector<ttstr> SupportedExts;

	tTVPPlugin(const ttstr & name, ITSSStorageProvider *storageprovider);
	~tTVPPlugin();

	bool Uninit();
};
//---------------------------------------------------------------------------
tTVPPlugin::tTVPPlugin(const ttstr & name, ITSSStorageProvider *storageprovider)
{
	Name = name;

	Instance = NULL;
	Holder = new tTVPPluginHolder(name);
	std::string filename;
	if (TVPUtf16ToUtf8(filename, Holder->GetLocalName().AsStdString()))
	{
		if (TVPCheckExistentLocalFile(Holder->GetLocalName()))
		{
			Instance = SDL_LoadObject(filename.c_str());
		}
	}

	if(!Instance)
	{
		if (Holder != NULL)
		{
			delete Holder;
		}
		TVPThrowExceptionMessage(TVPCannotLoadPlugin, name);
	}

	try
	{
		// retrieve each functions
		V2Link = (tTVPV2LinkProc)
			SDL_LoadFunction(Instance, "V2Link");
		V2Unlink = (tTVPV2UnlinkProc)
			SDL_LoadFunction(Instance, "V2Unlink");

		GetModuleInstance = (tTVPGetModuleInstanceProc)
			SDL_LoadFunction(Instance, "GetModuleInstance");
#if 0
		GetModuleThreadModel = (tTVPGetModuleThreadModelProc)
			SDL_LoadFunction(Instance, "GetModuleThreadModel");
		ShowConfigWindow = (tTVPShowConfigWindowProc)
			SDL_LoadFunction(Instance, "ShowConfigWindow");
		CanUnloadNow = (tTVPCanUnloadNowProc)
			SDL_LoadFunction(Instance, "CanUnloadNow");
#ifdef TVP_SUPPORT_OLD_WAVEUNPACKER
		CreateWaveUnpacker = (tTVPCreateWaveUnpackerProc)
			SDL_LoadFunction(Instance, "CreateWaveUnpacker");
#endif

#ifdef TVP_SUPPORT_KPI
		GetKMPModule = (pfnGetKMPModule)
			SDL_LoadFunction(Instance, SZ_KMP_GETMODULE);
#endif
#endif

		// link
		if(V2Link)
		{
			V2Link(TVPGetFunctionExporter());
		}

#ifdef _WIN32
		// retrieve ModuleInstance
		// Susie Plug-in check
		if(SDL_LoadFunction(Instance, "GetPicture"))
		{
			IsSusiePicturePlugin = true;
			TVPLoadPictureSPI(Instance);
			return;
		}
		if(SDL_LoadFunction(Instance, "GetFile"))
		{
			IsSusieArchivePlugin = true;
			TVPLoadArchiveSPI(Instance);
			return;
		}
#endif

		if(GetModuleInstance)
		{
			TSS_HWND mainwin = NULL;
#ifdef _WIN32
			mainwin = Application->GetHandle();
#endif
			HRESULT hr = GetModuleInstance(&TSSModule, storageprovider,
				 NULL, mainwin);
			if(FAILED(hr) || TSSModule == NULL)
				TVPThrowExceptionMessage(TVPCannotLoadPlugin, name);

			// get supported extensions
			TSS_ULONG index = 0;
			while(true)
			{
				tjs_char mediashortname[33];
				tjs_char buf[256];
				HRESULT hr = TSSModule->GetSupportExts(index,
					mediashortname, buf, 255);
				if(hr == S_OK)
					SupportedExts.push_back(ttstr(buf).AsLowerCase());
				else
					break;
				index ++;
			}
		}


#if 0
#ifdef TVP_SUPPORT_KPI
		// retrieve KbMediaPlayer Plug-in module instance
		if(GetKMPModule)
		{
			KMPModule = GetKMPModule();
			if(KMPModule->dwVersion != 100)
				TVPThrowExceptionMessage(TVPCannotLoadPlugin, name +
					TJS_W(" (invalid version)"));
			if(!KMPModule->dwReentrant)
				TVPThrowExceptionMessage(TVPCannotLoadPlugin, name +
					TJS_W(" (is not re-entrant)"));

			if(KMPModule->Init) KMPModule->Init();
		}
#endif
#endif
	}
	catch(...)
	{
		if (Instance != NULL)
		{
			SDL_UnloadObject(Instance);
			Instance = NULL;
		}
		if (Holder != NULL)
		{
			delete Holder;
			Holder = NULL;
		}
		throw;
	}
}
//---------------------------------------------------------------------------
tTVPPlugin::~tTVPPlugin()
{
}
//---------------------------------------------------------------------------
bool tTVPPlugin::Uninit()
{
	tTJS *tjs = TVPGetScriptEngine();
	if(tjs) tjs->DoGarbageCollection(); // to release unused objects

	if(V2Unlink)
	{
 		if(TJS_FAILED(V2Unlink())) return false;
	}
#ifdef _WIN32
#if 0
#ifdef TVP_SUPPORT_KPI
	if(KMPModule) if(KMPModule->Deinit) KMPModule->Deinit();
#endif
#endif
	if(TSSModule) TSSModule->Release();
#ifdef _WIN32
	if(IsSusiePicturePlugin) TVPUnloadPictureSPI(Instance);
	if(IsSusieArchivePlugin) TVPUnloadArchiveSPI(Instance);
#endif
#endif

	if (Instance != NULL)
	{
		SDL_UnloadObject(Instance);
		Instance = NULL;
	}
	if (Holder != NULL)
	{
		delete Holder;
		Holder = NULL;
	}
	return true;
}
#endif
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
#ifdef KRKRSDL2_ENABLE_PLUGINS
bool TVPPluginUnloadedAtSystemExit = false;
typedef std::vector<tTVPPlugin*> tTVPPluginVectorType;
#endif
struct tTVPPluginVectorStruc
{
#ifdef KRKRSDL2_ENABLE_PLUGINS
	tTVPPluginVectorType Vector;
#endif
	tTVPStorageProvider StorageProvider;
} static TVPPluginVector;
#ifdef KRKRSDL2_ENABLE_PLUGINS
static void TVPDestroyPluginVector(void)
{
	// state all plugins are to be released
	TVPPluginUnloadedAtSystemExit = true;

	// delete all objects
	tTVPPluginVectorType::iterator i;
	while(TVPPluginVector.Vector.size())
	{
		i = TVPPluginVector.Vector.end() - 1;
		try
		{
			(*i)->Uninit();
			delete *i;
		}
		catch(...)
		{
		}
		TVPPluginVector.Vector.pop_back();
	}
}
tTVPAtExit TVPDestroyPluginVectorAtExit
	(TVP_ATEXIT_PRI_RELEASE, TVPDestroyPluginVector);
#endif
//---------------------------------------------------------------------------
static bool TVPPluginLoading = false;
extern iTJSDispatch2 * TVPCreateNativeClass_KAGParserEX();
extern "C" void TVPKAGParserEX_StaticInit();

#ifdef __EMSCRIPTEN__
struct tTVPPluginStubAudit
{
	const tjs_char *dll_name;
	const char *tag;
	const char *fallback_class;
	const char *missing_semantics;
	const char *target_specific;
};

static const tTVPPluginStubAudit *TVPFindPluginStubAudit(const ttstr &basename)
{
	static const tTVPPluginStubAudit audits[] = {
		{ TJS_W("motionplayer.dll"), "[STUB-P1]", "dangerous-noop", "motion-runtime/timeline/wait/hittest", "no" },
		{ TJS_W("psbfile.dll"), "[STUB-P1]", "internal-expected", "psbfile-native-registry", "no" },
		{ TJS_W("varfile.dll"), "[STUB-P1]", "internal-expected", "var-storage-native-registry", "no" },
		{ TJS_W("AlphaMovie.dll"), "[STUB-P1]", "dangerous-noop", "alpha-video-playback/wait", "no" },
		{ TJS_W("csvParser.dll"), "[STUB-P1]", "dangerous-fake-success", "csv-data", "no" },
		{ TJS_W("fstat.dll"), "[STUB-P1]", "dangerous-fake-empty", "dirlist/getTime", "no" },
		{ TJS_W("layerExDraw.dll"), "[STUB-P1]", "dangerous-noop", "software-drawing", "no" },
		{ TJS_W("layerExSave.dll"), "[STUB-P1]", "dangerous-fake-success", "image-save/diff-rect", "no" },
		{ TJS_W("layerExImage.dll"), "[STUB-P1]", "dangerous-noop", "image-effects", "no" },
		{ TJS_W("layerExRaster.dll"), "[STUB-P1]", "dangerous-noop", "raster-effects", "no" },
		{ TJS_W("squirrel.dll"), "[STUB-P1]", "dangerous-noop", "squirrel-script-runtime", "no" },
		{ TJS_W("windowEx.dll"), "[STUB-P1]", "api-shim", "window/input/draw-device-semantics", "no" },
		{ TJS_W("scriptsEx.dll"), "[STUB-P1]", "internal-expected", "scriptsEx-native-registry", "no" },
		{ TJS_W("shrinkCopy.dll"), "[STUB-P1]", "internal-expected", "shrinkCopy-native-registry", "no" },
		{ TJS_W("extNagano.dll"), "[STUB-P1]", "dangerous-noop", "vendor-extension", "unknown" },
		{ TJS_W("extrans.dll"), "[STUB-P1]", "api-shim", "transition-provider-semantics", "no" },
		{ TJS_W("kagexopt.dll"), "[STUB-P1]", "known-safe-noop", "kag-optimizer", "no" },
		{ TJS_W("KAGParserEx.dll"), "[NATIVE]", "static-replacement", "extended-kag-parser (replaces KAGParser)", "no" },
		{ TJS_W("krmovie.dll"), "[STUB-P1]", "dangerous-noop", "video-plugin/wait", "no" },
		{ TJS_W("multiimage.dll"), "[STUB-P1]", "dangerous-noop", "multi-image-composition", "unknown" },
		{ TJS_W("PackinOne.dll"), "[STUB-P1]", "known-safe-noop", "packaging-tool-runtime", "no" },
		{ TJS_W("yuzuex.dll"), "[STUB-P1]", "dangerous-noop", "vendor-extension", "unknown" },
		{ TJS_W("wuvorbis.dll"), "[STUB-P1]", "compat-noop", "plugin-audio-decoder/status", "no" },
		{ nullptr, nullptr, nullptr, nullptr, nullptr }
	};
	for (int i = 0; audits[i].dll_name != nullptr; i++) {
		if (basename == ttstr(audits[i].dll_name)) return &audits[i];
	}
	return nullptr;
}

static void TVPLogPluginStubAudit(const ttstr &basename, const char *action)
{
	const tTVPPluginStubAudit *audit = TVPFindPluginStubAudit(basename);
	std::string dll = basename.AsNarrowStdString();
	if (audit) {
		KRKR_LOG_L2(
			"%s dll=%s fallback=%s missing=%s target_specific=%s action=%s\n",
			audit->tag, dll.c_str(), audit->fallback_class,
			audit->missing_semantics, audit->target_specific, action);
	} else {
		KRKR_LOG_L2(
			"[STUB-P1] dll=%s fallback=unknown-ignored missing=all target_specific=unknown action=%s\n",
			dll.c_str(), action);
	}
}

static void TVPRegisterPluginStub(const ttstr & name)
{
	extern void TVPExecuteExpression(const ttstr &expr, tTJSVariant *result);

	struct PluginStub {
		const tjs_char *dll_name;
		const tjs_char *tjs_code;
	};

	static const PluginStub stubs[] = {
		{
			TJS_W("motionplayer.dll"),
			TJS_W(
				"if (typeof global.Motion === 'undefined') {"
				"  global.Motion = %[];"
				/* --- _MotionResourceManager --- */
				"  class _MotionResourceManager {"
				"    var _resources, _nextId, resourceManager, allplaying;"
				"    function _MotionResourceManager() {"
				"      _resources = %[]; _nextId = 1; resourceManager = this; allplaying = false;"
				"    }"
				"    function clone() { return new _MotionResourceManager(); }"
				"    function load(filename) { var id = _nextId++; Debug.message('[EMOTE-RM] load: ' + filename + ' -> id=' + id); return id; }"
				"    function loadPSB(filename) { var id = _nextId++; Debug.message('[EMOTE-RM] loadPSB: ' + filename + ' -> id=' + id); return id; }"
				"    function loadPSBFromMemory(buf) { var id = _nextId++; Debug.message('[EMOTE-RM] loadPSBFromMemory -> id=' + id); return id; }"
				"    function loadFromMemory(buf) { var id = _nextId++; return id; }"
				"    function unload(id) {}"
				"    function unloadAll() { _resources = %[]; }"
				"    function getModelCount() { return 0; }"
				"    function getModelWidth(id) { return 0; }"
				"    function getModelHeight(id) { return 0; }"
				"    function getModelFrameCount(id) { return 0; }"
				"    function getModelMotionCount(id) { return 0; }"
				"    function getModelMotionName(id, index) { return ''; }"
				"    function getModelVariableCount(id) { return 0; }"
				"    function getModelVariableName(id, index) { return ''; }"
				"    function getModelVariableMin(id, index) { return 0; }"
				"    function getModelVariableMax(id, index) { return 0; }"
				"    function isLoaded(id) { return true; }"
				"  }"
				/* --- _MotionPlayer --- */
				"  class _MotionPlayer {"
				"    var _playing, _paused, _visible, _left, _top, _width, _height;"
				"    var _opacity, _scale, _timeScale, _bustScale, _wind;"
				"    var _meshDivisionRatio, _transform, _color, _smoothing;"
				"    var allplaying, _resMgr, _motionId, _loading;"
				"    function _MotionPlayer(resMgr) {"
				"      _playing = false; _paused = false; _visible = false; _loading = false;"
				"      _left = 0; _top = 0; _width = 0; _height = 0;"
				"      _opacity = 255; _scale = 1.0; _timeScale = 1.0;"
				"      _bustScale = 1.0; _wind = 0; _meshDivisionRatio = 1.0;"
				"      _transform = void; _color = 0xFFFFFF; _smoothing = true;"
				"      allplaying = false; _resMgr = resMgr; _motionId = void;"
				"    }"
				"    function clone() { return new _MotionPlayer(void); }"
				/* resource/motion management */
				"    function setResourceManager(mgr) { _resMgr = mgr; }"
				"    function getResourceManager() { return _resMgr; }"
				"    function setMotion(id) { _motionId = id; }"
				"    function load(filename) { Debug.message('[EMOTE-PL] load: ' + filename); return 0; }"
				"    function loadPSB(filename) { Debug.message('[EMOTE-PL] loadPSB: ' + filename); return 0; }"
				"    function loadFromMemory(buf) { return 0; }"
				"    function isLoading() { return _loading; }"
				"    function isLoaded() { return true; }"
				/* playback control */
				"    function play() { _playing = true; _paused = false; }"
				"    function stop() { _playing = false; _paused = false; }"
				"    function pause() { _paused = true; }"
				"    function resume() { _paused = false; }"
				"    function isPlaying() { return _playing; }"
				"    function isPaused() { return _paused; }"
				"    function progress(interval) {}"
				"    function skip() {}"
				"    function skipToSync() {}"
				/* rendering */
				"    function draw(target) {}"
				"    function update() {}"
				"    function clear() {}"
				"    function render() {}"
				/* coordinate/transform */
				"    function setCoord(x, y) { _left = x; _top = y; }"
				"    function getCoord() { return %[x: _left, y: _top]; }"
				"    function setScale(v) { _scale = v; }"
				"    function getScale() { return _scale; }"
				"    function setTimeScale(v) { _timeScale = v; }"
				"    function getTimeScale() { return _timeScale; }"
				"    function setAttachRect(l,t,w,h) { _left=l; _top=t; _width=w; _height=h; }"
				"    function getAttachRect() { return %[left:_left,top:_top,width:_width,height:_height]; }"
				"    function setTransform(mtx) { _transform = mtx; }"
				"    function getTransform() { return _transform; }"
				/* position/size */
				"    function setLeft(v) { _left = v; }"
				"    function getLeft() { return _left; }"
				"    function setTop(v) { _top = v; }"
				"    function getTop() { return _top; }"
				"    function setWidth(v) { _width = v; }"
				"    function getWidth() { return _width; }"
				"    function setHeight(v) { _height = v; }"
				"    function getHeight() { return _height; }"
				/* visibility/opacity */
				"    function setVisible(v) { _visible = v; }"
				"    function getVisible() { return _visible; }"
				"    function setOpacity(v) { _opacity = v; }"
				"    function getOpacity() { return _opacity; }"
				"    function setSmoothing(v) { _smoothing = v; }"
				"    function getSmoothing() { return _smoothing; }"
				/* E-mote specific: physics/effects */
				"    function setBustScale(v) { _bustScale = v; }"
				"    function getBustScale() { return _bustScale; }"
				"    function setWind(v) { _wind = v; }"
				"    function getWind() { return _wind; }"
				"    function setMeshDivisionRatio(v) { _meshDivisionRatio = v; }"
				"    function getMeshDivisionRatio() { return _meshDivisionRatio; }"
				"    function setColor(v) { _color = v; }"
				"    function getColor() { return _color; }"
				"    function setPhysicsTimestep(v) {}"
				"    function getPhysicsTimestep() { return 0; }"
				"    function setPhysicsGravity(x, y) {}"
				"    function setPhysicsWind(x, y) {}"
				/* variable/command */
				"    function setVariable(name, val) {}"
				"    function getVariable(name) { return 0; }"
				"    function setVariableRange(name, min, max) {}"
				"    function addMotion(name) {}"
				"    function removeMotion(name) {}"
				"    function getMotionList() { return []; }"
				"    function getCommandList() { return []; }"
				"    function getLayerGetter(name) { return void; }"
				"    function getVariableList() { return []; }"
				"    function getVariableCount() { return 0; }"
				"    function getVariableName(index) { return ''; }"
				"    function getVariableMin(name) { return 0; }"
				"    function getVariableMax(name) { return 0; }"
				/* model info */
				"    function getModelWidth() { return 0; }"
				"    function getModelHeight() { return 0; }"
				"    function getFrameCount() { return 0; }"
				"    function getCurrentFrame() { return 0; }"
				"    function setCurrentFrame(f) {}"
				"    function getDuration() { return 0; }"
				/* properties */
				"    property playing { getter { return _playing; } }"
				"    property motionPlaying { getter { return _playing; } }"
				"    property motion { getter { return _motionId; } setter(v) { _motionId = v; } }"
				"    property visible { getter { return _visible; } setter(v) { _visible = v; } }"
				"    property opacity { getter { return _opacity; } setter(v) { _opacity = v; } }"
				"    property left { getter { return _left; } setter(v) { _left = v; } }"
				"    property top { getter { return _top; } setter(v) { _top = v; } }"
				"    property width { getter { return _width; } setter(v) { _width = v; } }"
				"    property height { getter { return _height; } setter(v) { _height = v; } }"
				"    property scale { getter { return _scale; } setter(v) { _scale = v; } }"
				"    property timeScale { getter { return _timeScale; } setter(v) { _timeScale = v; } }"
				"    property bustScale { getter { return _bustScale; } setter(v) { _bustScale = v; } }"
				"    property wind { getter { return _wind; } setter(v) { _wind = v; } }"
				"    property color { getter { return _color; } setter(v) { _color = v; } }"
				"    property smoothing { getter { return _smoothing; } setter(v) { _smoothing = v; } }"
				"    property paused { getter { return _paused; } }"
				"  }"
				/* --- _SeparateLayerAdaptor --- */
				"  class _SeparateLayerAdaptor {"
				"    var _layer, _visible, _opacity, _left, _top;"
				"    function _SeparateLayerAdaptor(layer) {"
				"      _layer = layer; _visible = true; _opacity = 255; _left = 0; _top = 0;"
				"    }"
				"    function clone() { return new _SeparateLayerAdaptor(void); }"
				"    function setCoord(x, y) { _left = x; _top = y; }"
				"    function setVisible(v) { _visible = v; }"
				"    function getVisible() { return _visible; }"
				"    function setOpacity(v) { _opacity = v; }"
				"    function getOpacity() { return _opacity; }"
				"    function setLeft(v) { _left = v; }"
				"    function setTop(v) { _top = v; }"
				"    function update() {}"
				"    function draw(target) {}"
				"    function clear() {}"
				"    property visible { getter { return _visible; } setter(v) { _visible = v; } }"
				"    property opacity { getter { return _opacity; } setter(v) { _opacity = v; } }"
				"  }"
				/* --- Registration --- */
				"  Motion.ResourceManager = _MotionResourceManager;"
				"  Motion.Player = _MotionPlayer;"
				"  Motion.SeparateLayerAdaptor = _SeparateLayerAdaptor;"
				"}"
				"try {"
				"  var _mgr = new Motion.ResourceManager();"
				"  global.__WindowMotionManager = _mgr;"
				"  if (typeof Window !== 'undefined' && typeof Window.mainWindow !== 'undefined'"
				"      && Window.mainWindow !== void) {"
				"    Window.mainWindow.motion_manager = _mgr;"
				"  }"
				"} catch(e) {"
				"  Debug.message('[Plugin Stub] motion_manager setup: ' + e.message);"
				"}"
			)
		},
		{
			TJS_W("psbfile.dll"),
			TJS_W("")
		},
		{
			TJS_W("varfile.dll"),
			TJS_W("global.varfileLoaded = true;")
		},
		{
			TJS_W("AlphaMovie.dll"),
			TJS_W(
				"if (typeof global.AlphaMovie === 'undefined') {"
				"  class _AlphaMovie {"
				"    var _visible, _loop, _left, _top, _width, _height;"
				"    function _AlphaMovie(window) {"
				"      _visible = false; _loop = false;"
				"      _left = 0; _top = 0; _width = 0; _height = 0;"
				"    }"
				"    function open(filename) { return true; }"
				"    function play() {}"
				"    function stop() {}"
				"    function setVisible(v) { _visible = v; }"
				"    function getVisible() { return _visible; }"
				"    function setLoop(v) { _loop = v; }"
				"    function getLoop() { return _loop; }"
				"    function setLeft(v) { _left = v; }"
				"    function setTop(v) { _top = v; }"
				"    function setWidth(v) { _width = v; }"
				"    function setHeight(v) { _height = v; }"
				"  }"
				"  global.AlphaMovie = _AlphaMovie;"
				"}"
			)
		},
		{
			TJS_W("csvParser.dll"),
			TJS_W(
				"if (typeof global.CSVParser === 'undefined') {"
				"  class _CSVParser {"
				"    var _data;"
				"    function _CSVParser() { _data = []; }"
				"    function load(filename) { return true; }"
				"    function getCount() { return _data.count; }"
				"    function getData(row, col) { return ''; }"
				"    function getRowCount() { return 0; }"
				"    function getColCount() { return 0; }"
				"  }"
				"  global.CSVParser = _CSVParser;"
				"}"
			)
		},
		{
			TJS_W("fstat.dll"),
			TJS_W(
				"if (typeof Storages.dirlist === 'undefined') {"
				"  Storages.dirlist = function(path) { return []; };"
				"}"
				"if (typeof Storages.getTime === 'undefined') {"
				"  Storages.getTime = function(filename) { return ''; };"
				"}"
			)
		},
		{
			TJS_W("layerExDraw.dll"),
			TJS_W(
				"Debug.message('[Plugin Stub] layerExDraw.dll: drawText=' + typeof Layer.drawText + ', drawImage=' + typeof Layer.drawImage);"
				"if (typeof Layer.drawImage === 'undefined') Layer.drawImage = function() {};"
				"if (typeof Layer.drawEllipse === 'undefined') Layer.drawEllipse = function() {};"
				"if (typeof Layer.drawRectangle === 'undefined') Layer.drawRectangle = function() {};"
				"if (typeof Layer.drawRoundRect === 'undefined') Layer.drawRoundRect = function() {};"
				"if (typeof Layer.drawArc === 'undefined') Layer.drawArc = function() {};"
				"if (typeof Layer.drawLine === 'undefined') Layer.drawLine = function() {};"
				"if (typeof Layer.drawBezier === 'undefined') Layer.drawBezier = function() {};"
				"if (typeof Layer.drawPie === 'undefined') Layer.drawPie = function() {};"
				"if (typeof Layer.drawPolygon === 'undefined') Layer.drawPolygon = function() {};"
				"if (typeof Layer.fillEllipse === 'undefined') Layer.fillEllipse = function() {};"
				"if (typeof Layer.fillRectangle === 'undefined') Layer.fillRectangle = function() {};"
				"if (typeof Layer.fillRoundRect === 'undefined') Layer.fillRoundRect = function() {};"
				"if (typeof Layer.fillPie === 'undefined') Layer.fillPie = function() {};"
				"if (typeof Layer.fillPolygon === 'undefined') Layer.fillPolygon = function() {};"
			)
		},
		{
			TJS_W("layerExSave.dll"),
			TJS_W(
				"if (typeof Layer.saveLayerImagePng === 'undefined') {"
				"  Layer.saveLayerImagePng = function(filename) { return true; };"
				"  Layer.saveLayerImageTlg5 = function(filename) { return true; };"
				"  Layer.getDiffRect = function(layer) { return %[left:0,top:0,right:0,bottom:0]; };"
				"}"
			)
		},
		{
			TJS_W("layerExImage.dll"),
			TJS_W(
				"if (typeof Layer.doDropShadow === 'undefined') {"
				"  Layer.doDropShadow = function() {};"
				"  Layer.doBlurLight = function() {};"
				"  Layer.doGlowShadow = function() {};"
				"}"
			)
		},
		{
			TJS_W("layerExRaster.dll"),
			TJS_W(
				"if (typeof Layer.copyRaster === 'undefined') {"
				"  Layer.copyRaster = function() {};"
				"}"
			)
		},
		{
			TJS_W("squirrel.dll"),
			TJS_W(
				"if (typeof Scripts.execStorageSQ === 'undefined') {"
				"  Scripts.execStorageSQ = function() {};"
				"  Scripts.forkStorageSQ = function() {};"
				"  Scripts.driveSQ = function() {};"
				"}"
			)
		},
		{
			TJS_W("windowEx.dll"),
			TJS_W(
				"if (typeof global.BasicDrawDevice !== 'undefined') {"
				"  if (typeof global.PassThroughDrawDevice === 'undefined') {"
				"    global.PassThroughDrawDevice = global.BasicDrawDevice;"
				"  }"
				"  if (typeof global.BasicDrawDevice.drawer === 'undefined') {"
				"    global.BasicDrawDevice.drawer = 0;"
				"  }"
				"} else {"
				"  class PassThroughDrawDevice {"
				"    var drawer;"
				"    function PassThroughDrawDevice() { drawer = 0; }"
				"  }"
				"}"
				"if (typeof Window !== 'undefined') {"
				"  if (typeof Window.PassThroughDrawDevice === 'undefined') {"
				"    if (typeof global.PassThroughDrawDevice !== 'undefined') {"
				"      Window.PassThroughDrawDevice = global.PassThroughDrawDevice;"
				"    } else if (typeof global.BasicDrawDevice !== 'undefined') {"
				"      Window.PassThroughDrawDevice = global.BasicDrawDevice;"
				"    }"
				"  }"
				"  if (typeof Window.registerExEvent === 'undefined') {"
				"    Window.registerExEvent = function() {};"
				"  }"
				"  if (typeof Window.getDisplayColorDepth === 'undefined') {"
				"    Window.getDisplayColorDepth = function() { return 32; };"
				"  }"
				"  if (typeof Window.getKeyState === 'undefined') {"
				"    Window.getKeyState = function(key) { return false; };"
				"  }"
				"  if (typeof Window.exSystemMenu === 'undefined') {"
				"    Window.exSystemMenu = void;"
				"  }"
				"  if (typeof Window.exMaximizeMenuItem === 'undefined') {"
				"    Window.exMaximizeMenuItem = void;"
				"  }"
				"  if (typeof Window.exMinimizeMenuItem === 'undefined') {"
				"    Window.exMinimizeMenuItem = void;"
				"  }"
				"  if (typeof Window.exCloseMenuItem === 'undefined') {"
				"    Window.exCloseMenuItem = void;"
				"  }"
				"  if (typeof Window.disableResize === 'undefined') {"
				"    Window.disableResize = void;"
				"  }"
				"  if (typeof Window.maximizeBox === 'undefined') {"
				"    Window.maximizeBox = void;"
				"  }"
				"  if (typeof Window.minimizeBox === 'undefined') {"
				"    Window.minimizeBox = void;"
				"  }"
				"  if (typeof Window.maximized === 'undefined') {"
				"    Window.maximized = false;"
				"  }"
				"  if (typeof Window.drawer === 'undefined') {"
				"    Window.drawer = void;"
				"  }"
				"  if (typeof Window.bringAfter === 'undefined') {"
				"    Window.bringAfter = function() {};"
				"  }"
				"  if (typeof Window.dtNone === 'undefined') {"
				"    Window.dtNone = -1;"
				"  }"
				"  if (typeof Window.dtDrawDib === 'undefined') {"
				"    Window.dtDrawDib = 0;"
				"  }"
				"  if (typeof Window.dtDBGDI === 'undefined') {"
				"    Window.dtDBGDI = 1;"
				"  }"
				"  if (typeof Window.dtDBDD === 'undefined') {"
				"    Window.dtDBDD = 2;"
				"  }"
				"  if (typeof Window.dtDBD3D === 'undefined') {"
				"    Window.dtDBD3D = 3;"
				"  }"
				"}"
			)
		},
		{
			TJS_W("scriptsEx.dll"),
			TJS_W("")
		},
		{
			TJS_W("shrinkCopy.dll"),
			TJS_W(
				"Debug.message('[Plugin Stub] shrinkCopy.dll loaded (no-op)');"
			)
		},
		{
			TJS_W("extNagano.dll"),
			TJS_W(
				"Debug.message('[Plugin Stub] extNagano.dll loaded (no-op)');"
			)
		},
		{
			TJS_W("extrans.dll"),
			TJS_W(
				"if (typeof global.GFTransHandlerProvider === 'undefined') {"
				"  class GFTransHandlerProvider {"
				"    function GFTransHandlerProvider() {}"
				"  }"
				"}"
				"Debug.message('[Plugin Stub] extrans.dll loaded (transition provider stub)');"
			)
		},
		{
			TJS_W("kagexopt.dll"),
			TJS_W(
				"Debug.message('[Plugin Stub] kagexopt.dll loaded (no-op)');"
			)
		},
		{
			TJS_W("KAGParserEx.dll"),
			TJS_W(
				"Debug.message('[Plugin Native] KAGParserEx.dll: statically linked (replaces KAGParser class)');"
			)
		},
		{
			TJS_W("krmovie.dll"),
			TJS_W(
				"Debug.message('[Plugin Stub] krmovie.dll loaded (no-op)');"
			)
		},
		{
			TJS_W("multiimage.dll"),
			TJS_W(
				"Debug.message('[Plugin Stub] multiimage.dll loaded (no-op)');"
			)
		},
		{
			TJS_W("PackinOne.dll"),
			TJS_W(
				"Debug.message('[Plugin Stub] PackinOne.dll loaded (no-op)');"
			)
		},
		{
			TJS_W("yuzuex.dll"),
			TJS_W(
				"Debug.message('[Plugin Stub] yuzuex.dll loaded (no-op)');"
			)
		},
		{
			TJS_W("wuvorbis.dll"),
			TJS_W(
				"Debug.message('[Plugin Stub] wuvorbis.dll loaded (no-op)');"
			)
		},
		{ nullptr, nullptr }
	};

	ttstr basename = name;
	const tjs_char *p = basename.c_str();
	const tjs_char *last_sep = nullptr;
	for (const tjs_char *c = p; *c; c++) {
		if (*c == TJS_W('/') || *c == TJS_W('\\')) last_sep = c;
	}
	if (last_sep) basename = ttstr(last_sep + 1);

	for (int i = 0; stubs[i].dll_name != nullptr; i++) {
		if (basename == ttstr(stubs[i].dll_name)) {
			try {
				TVPExecuteExpression(ttstr(stubs[i].tjs_code), nullptr);
				TVPLogPluginStubAudit(basename, "registered");
				if (TVPLogL2()) TVPAddLog(ttstr(TJS_W("[Plugin Stub] Registered stub for: ")) + basename);
			} catch (...) {
				TVPLogPluginStubAudit(basename, "registration-failed");
				TVPAddImportantLog(ttstr(TJS_W("[Plugin Stub] Failed to register stub for: ")) + basename);
			}
			return;
		}
	}
	TVPLogPluginStubAudit(basename, "ignored");
	if (TVPLogL2()) TVPAddLog(ttstr(TJS_W("[Plugin Stub] No stub for: ")) + basename + TJS_W(" (ignored)"));
}
#endif

void TVPLoadPlugin(const ttstr & name)
{
#ifdef __EMSCRIPTEN__
	KRKR_LOG_L2("[PLUGIN-LOAD] request: %s\n", name.AsNarrowStdString().c_str());
	if (name == TJS_W("KAGParser.dll") || name == TJS_W("KAGParserEx.dll")) {
		iTJSDispatch2 * global = TVPGetScriptDispatch();
		if (global) {
			tTJSVariant val;
			TVPKAGParserEX_StaticInit();
			iTJSDispatch2 * tjsclass = TVPCreateNativeClass_KAGParserEX();
			val = tTJSVariant(tjsclass);
			tjsclass->Release();
			global->PropSet( TJS_MEMBERENSURE, TJS_W("KAGParser"), nullptr, &val, global );
			global->Release();
		}
		TVPAddLog(ttstr(TJS_W("Statically linked KAGParserEx (replaces KAGParser/KAGParserEx DLLs) for Emscripten.")));
		return;
	}
		// Try loading via ncbind static plugin registry first
			extern bool TVPLoadInternalPlugin(const ttstr &);
			if (TVPLoadInternalPlugin(name)) {
				KRKR_LOG_L2("[PLUGIN-LOAD] internal hit: %s\n", name.AsNarrowStdString().c_str());
				KRKR_LOG_L2("[PLUGIN-INTERNAL] dll=%s result=internal-hit\n", name.AsNarrowStdString().c_str());
				TVPAddLog(ttstr(TJS_W("Loaded internal plugin: ")) + name);
				return;
			}
			KRKR_LOG_L2("[PLUGIN-LOAD] internal miss: %s\n", name.AsNarrowStdString().c_str());
		if (name == TJS_W("psbfile.dll")) {
			extern void TVPLoadPSBFilePlugin();
			static bool psbfile_loaded = false;
			if (!psbfile_loaded) {
				TVPLoadPSBFilePlugin();
				psbfile_loaded = true;
				}
				KRKR_LOG_L2("[PLUGIN-INTERNAL] dll=psbfile.dll result=special-internal-hit\n");
				TVPAddLog(TJS_W("Loaded internal plugin: psbfile.dll"));
				return;
			}
		// Fallback to TJS stub registration
		TVPRegisterPluginStub(name);
		return;
#endif

#ifdef KRKRSDL2_ENABLE_PLUGINS
	// load plugin
	if(TVPPluginLoading)
		TVPThrowExceptionMessage(TVPCannnotLinkPluginWhilePluginLinking);
			// linking plugin while other plugin is linking, is prohibited
			// by data security reason.

	// check whether the same plugin was already loaded
	tTVPPluginVectorType::iterator i;
	for(i = TVPPluginVector.Vector.begin();
		i != TVPPluginVector.Vector.end(); i++)
	{
		if((*i)->Name == name) return;
	}

	tTVPPlugin * p;

	try
	{
		TVPPluginLoading = true;
		p = new tTVPPlugin(name, &TVPPluginVector.StorageProvider);
		TVPPluginLoading = false;
	}
	catch(...)
	{
		TVPPluginLoading = false;
		throw;
	}

	TVPPluginVector.Vector.push_back(p);
#endif
}
//---------------------------------------------------------------------------
bool TVPUnloadPlugin(const ttstr & name)
{
	// unload plugin

#ifdef KRKRSDL2_ENABLE_PLUGINS
	tTVPPluginVectorType::iterator i;
	for(i = TVPPluginVector.Vector.begin();
		i != TVPPluginVector.Vector.end(); i++)
	{
		if((*i)->Name == name)
		{
			if(!(*i)->Uninit()) return false;
			delete *i;
			TVPPluginVector.Vector.erase(i);
			return true;
		}
	}
	TVPThrowExceptionMessage(TVPNotLoadedPlugin, name);
	return false;
#else
	return true;
#endif
}
//---------------------------------------------------------------------------





#if 0
//---------------------------------------------------------------------------
// plug-in autoload support
//---------------------------------------------------------------------------
struct tTVPFoundPlugin
{
	tjs_string Path;
	tjs_string Name;
	bool operator < (const tTVPFoundPlugin &rhs) const { return Name < rhs.Name; }
};
static tjs_int TVPAutoLoadPluginCount = 0;
static void TVPSearchPluginsAt(std::vector<tTVPFoundPlugin> &list, tjs_string folder)
{
	WIN32_FIND_DATA ffd;
	HANDLE handle = ::FindFirstFile((folder + TJS_W("*.tpm")).c_str(), &ffd);
	if(handle != INVALID_HANDLE_VALUE)
	{
		BOOL cont;
		do
		{
			if(!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				tTVPFoundPlugin fp;
				fp.Path = folder;
				fp.Name = ffd.cFileName;
				list.push_back(fp);
			}
			cont = FindNextFile(handle, &ffd);
		} while(cont);
		FindClose(handle);
	}
}
#endif

void TVPLoadPluigins(void)
{
#if 0
	// This function searches plugins which have an extension of ".tpm"
	// in the default path:
	//    1. a folder which holds kirikiri executable
	//    2. "plugin" folder of it
	// Plugin load order is to be decided using its name;
	// aaa.tpm is to be loaded before aab.tpm (sorted by ASCII order)

	// search plugins from path: (exepath), (exepath)\system, (exepath)\plugin
	std::vector<tTVPFoundPlugin> list;

	tjs_string exepath = IncludeTrailingBackslash(ExtractFileDir(ExePath()));

	TVPSearchPluginsAt(list, exepath);
	TVPSearchPluginsAt(list, exepath + TJS_W("system\\"));
#ifdef TJS_64BIT_OS
	TVPSearchPluginsAt(list, exepath + TJS_W("plugin64\\"));
#else
	TVPSearchPluginsAt(list, exepath + TJS_W("plugin\\"));
#endif

	// sort by filename
	std::sort(list.begin(), list.end());

	// load each plugin
	TVPAutoLoadPluginCount = (tjs_int)list.size();
	for(std::vector<tTVPFoundPlugin>::iterator i = list.begin();
		i != list.end();
		i++)
	{
		TVPAddImportantLog(ttstr(TJS_W("(info) Loading ")) + ttstr(i->Name));
		TVPLoadPlugin(i->Path + i->Name);
	}
#endif
}
//---------------------------------------------------------------------------
#if 0
tjs_int TVPGetAutoLoadPluginCount() { return TVPAutoLoadPluginCount; }
#else
tjs_int TVPGetAutoLoadPluginCount() { return 0; }
#endif
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// interface for built-in Wave decode plugins
//---------------------------------------------------------------------------
struct tTVPTSSModuleWrapper
{
	ITSSModule *TSSModule = nullptr;

	tTVPGetModuleInstanceProc GetModuleInstance = nullptr;

	std::vector<ttstr> SupportedExts;

	tTVPTSSModuleWrapper(tTVPGetModuleInstanceProc GetModuleInstanceProc, ITSSStorageProvider *storageprovider);
	~tTVPTSSModuleWrapper();
};
//---------------------------------------------------------------------------
tTVPTSSModuleWrapper::tTVPTSSModuleWrapper(tTVPGetModuleInstanceProc GetModuleInstanceProc, ITSSStorageProvider *storageprovider)
{
	GetModuleInstance = GetModuleInstanceProc;
	if(GetModuleInstance)
	{
		TSS_HWND mainwin = NULL;
#ifdef _WIN32
		mainwin = Application->GetHandle();
#endif
		HRESULT hr = GetModuleInstance(&TSSModule, storageprovider,
			 NULL, mainwin);
		if(FAILED(hr) || TSSModule == NULL)
			TVPThrowExceptionMessage(TJS_W("TSSModule retrieval failure"));

		// get supported extensions
		TSS_ULONG index = 0;
		while(true)
		{
			tjs_char mediashortname[33];
			tjs_char buf[256];
			HRESULT hr = TSSModule->GetSupportExts(index,
				mediashortname, buf, 255);
			if(hr == S_OK)
				SupportedExts.push_back(ttstr(buf).AsLowerCase());
			else
				break;
			index ++;
		}
	}
}
//---------------------------------------------------------------------------
tTVPTSSModuleWrapper::~tTVPTSSModuleWrapper()
{
	if(TSSModule) TSSModule->Release();
}
typedef std::vector<tTVPTSSModuleWrapper*> tTVPTSSModuleWrapperType;
struct tTVPTSSModuleWrapperVectorStruc
{
	tTVPTSSModuleWrapperType Vector;
} static TVPTSSModuleWrapperVector;

void TVPRegisterTSSWaveDecoder(tTVPGetModuleInstanceProc GetModuleInstance)
{
	tTVPTSSModuleWrapper * p;

	try
	{
		p = new tTVPTSSModuleWrapper(GetModuleInstance, &TVPPluginVector.StorageProvider);
	}
	catch(...)
	{
		throw;
	}

	TVPTSSModuleWrapperVector.Vector.push_back(p);
}
//---------------------------------------------------------------------------
// interface to Wave decode plugins
//---------------------------------------------------------------------------
ITSSWaveDecoder * TVPSearchAvailTSSWaveDecoder(const ttstr & storage, const ttstr & extension)
{
	{
		tTVPTSSModuleWrapperType::iterator i;
		for(i = TVPTSSModuleWrapperVector.Vector.begin();
			i != TVPTSSModuleWrapperVector.Vector.end(); i++)
		{
			if((*i)->TSSModule)
			{
				// check whether the plugin supports extension
				bool supported = false;
				std::vector<ttstr>::iterator ei;
				for(ei = (*i)->SupportedExts.begin(); ei != (*i)->SupportedExts.end(); ei++)
				{
					if(ei->GetLen() == 0) { supported = true; break; }
					if(extension == *ei) { supported = true; break; }
				}

				if(!supported) continue;

				// retrieve instance from (*i)->TSSModule
				IUnknown *intf = NULL;
				HRESULT hr = (*i)->TSSModule->GetMediaInstance(
					(tjs_char*)storage.c_str(), &intf);
				if(SUCCEEDED(hr))
				{
					try
					{
						// check  whether the instance has IID_ITSSWaveDecoder
						// interface.
						ITSSWaveDecoder * decoder;
						if(SUCCEEDED(intf->QueryInterface(IID_ITSSWaveDecoder,
							(void**) &decoder)))
						{
							intf->Release();
							return decoder; // OK
						}
					}
					catch(...)
					{
						intf->Release();
						throw;
					}
					intf->Release();
				}

			}
		}
	}
#ifdef KRKRSDL2_ENABLE_PLUGINS
	tTVPPluginVectorType::iterator i;
	for(i = TVPPluginVector.Vector.begin();
		i != TVPPluginVector.Vector.end(); i++)
	{
		if((*i)->TSSModule)
		{
			// check whether the plugin supports extension
			bool supported = false;
			std::vector<ttstr>::iterator ei;
			for(ei = (*i)->SupportedExts.begin(); ei != (*i)->SupportedExts.end(); ei++)
			{
				if(ei->GetLen() == 0) { supported = true; break; }
				if(extension == *ei) { supported = true; break; }
			}

			if(!supported) continue;

			// retrieve instance from (*i)->TSSModule
			IUnknown *intf = NULL;
			HRESULT hr = (*i)->TSSModule->GetMediaInstance(
				(tjs_char*)storage.c_str(), &intf);
			if(SUCCEEDED(hr))
			{
				try
				{
					// check  whether the instance has IID_ITSSWaveDecoder
					// interface.
					ITSSWaveDecoder * decoder;
					if(SUCCEEDED(intf->QueryInterface(IID_ITSSWaveDecoder,
						(void**) &decoder)))
					{
						intf->Release();
						return decoder; // OK
					}
				}
				catch(...)
				{
					intf->Release();
					throw;
				}
				intf->Release();
			}

		}
	}
#endif
	return NULL; // not found
}
//---------------------------------------------------------------------------
#if 0
//---------------------------------------------------------------------------
#ifdef TVP_SUPPORT_OLD_WAVEUNPACKER
IWaveUnpacker * TVPSearchAvailWaveUnpacker(const ttstr & storage, IStream **stream)
{
	tTVPPluginVectorType::iterator i;
	for(i = TVPPluginVector.Vector.begin();
		i != TVPPluginVector.Vector.end(); i++)
	{
		if((*i)->CreateWaveUnpacker) break;
	}
	if(i == TVPPluginVector.Vector.end()) return NULL; // KPI not found

	// retrieve IStream interface
	AnsiString ansiname = storage.AsAnsiString();

	tTJSBinaryStream *stream0 = NULL;
	long size;
	try
	{
		stream0 = TVPCreateStream(storage);
		size = (long)stream0->GetSize();
	}
	catch(...)
	{
		if(stream0) delete stream0;
		return NULL;
	}

	IStream *istream = new tTVPIStreamAdapter(stream0);

	try
	{

		for(i = TVPPluginVector.Vector.begin();
			i != TVPPluginVector.Vector.end(); i++)
		{
			if((*i)->CreateWaveUnpacker)
			{
				// call CreateWaveUnpacker to retrieve decoder instance
				IWaveUnpacker *out;
				HRESULT hr = (*i)->CreateWaveUnpacker(istream, size,
					ansiname.c_str(), &out);
				if(SUCCEEDED(hr))
				{
					*stream = istream;
					return out;
				}
			}
		}
	}
	catch(...)
	{
		istream->Release();
		return NULL;
	}
	istream->Release();
	return NULL; // not found
}
#endif
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#ifdef TVP_SUPPORT_KPI
void * TVPSearchAvailKMPWaveDecoder(const ttstr & storage, KMPMODULE ** module,
	SOUNDINFO * info)
{
	tTVPPluginVectorType::iterator i;
	for(i = TVPPluginVector.Vector.begin();
		i != TVPPluginVector.Vector.end(); i++)
	{
		if((*i)->KMPModule) break;
	}
	if(i == TVPPluginVector.Vector.end()) return NULL; // KPI not found

	AnsiString localname;

	if(TJS_strchr(storage.c_str(), TVPArchiveDelimiter)) return NULL;
		// in-archive storage is not supported

	try
	{
		ttstr ln(TVPSearchPlacedPath(storage));
		TVPGetLocalName(ln);
		localname  = ln.AsAnsiString();
	}
	catch(...)
	{
		return NULL;
	}

	AnsiString ext = TVPExtractStorageExt(storage).AsAnsiString();

	for(i = TVPPluginVector.Vector.begin();
		i != TVPPluginVector.Vector.end(); i++)
	{
		if((*i)->KMPModule)
		{
			// search over available extensions
			const char **module_ext = (*i)->KMPModule->ppszSupportExts;
			while(*module_ext)
			{
				if(!strcmpi(ext.c_str(), *module_ext)) break;
				module_ext ++;
			}
			if(!*module_ext) continue; // not found in this plug-in

			*module = (*i)->KMPModule;
			HKMP hkmp = (*i)->KMPModule->Open(localname.c_str(), info);
			if(hkmp)
				(*i)->KMPModule->SetPosition(hkmp, 0);
					// rewind; some plug-ins crash when the initial rewind is
					// not processed...
			return hkmp;
		}
	}
	return NULL; // not found
}
#endif
//---------------------------------------------------------------------------
#endif





//---------------------------------------------------------------------------
// some service functions for plugin
//---------------------------------------------------------------------------
#include "zlib/zlib.h"
int ZLIB_uncompress(unsigned char *dest, unsigned long *destlen,
	const unsigned char *source, unsigned long sourcelen)
{
	return uncompress(dest, destlen, source, sourcelen);
}
//---------------------------------------------------------------------------
int ZLIB_compress(unsigned char *dest, unsigned long *destlen,
	const unsigned char *source, unsigned long sourcelen)
{
	return compress(dest, destlen, source, sourcelen);
}
//---------------------------------------------------------------------------
int ZLIB_compress2(unsigned char *dest, unsigned long *destlen,
	const unsigned char *source, unsigned long sourcelen, int level)
{
	return compress2(dest, destlen, source, sourcelen, level);
}
//---------------------------------------------------------------------------
#include "md5.h"
static char TVP_assert_md5_state_t_size[
	 (sizeof(TVP_md5_state_t) >= sizeof(md5_state_t))];
	// if this errors, sizeof(TVP_md5_state_t) is not equal to sizeof(md5_state_t).
	// sizeof(TVP_md5_state_t) must be equal to sizeof(md5_state_t).
//---------------------------------------------------------------------------
void TVP_md5_init(TVP_md5_state_t *pms)
{
	md5_init((md5_state_t*)pms);
}
//---------------------------------------------------------------------------
void TVP_md5_append(TVP_md5_state_t *pms, const tjs_uint8 *data, int nbytes)
{
	md5_append((md5_state_t*)pms, (const md5_byte_t*)data, nbytes);
}
//---------------------------------------------------------------------------
void TVP_md5_finish(TVP_md5_state_t *pms, tjs_uint8 *digest)
{
	md5_finish((md5_state_t*)pms, digest);
}
//---------------------------------------------------------------------------
#ifdef _WIN32
HWND TVPGetApplicationWindowHandle()
{
	return Application->GetHandle();
}
#endif
//---------------------------------------------------------------------------
void TVPProcessApplicationMessages()
{
#if 0
	Application->ProcessMessages();
#endif
}
//---------------------------------------------------------------------------
void TVPHandleApplicationMessage()
{
#if 0
	Application->HandleMessage();
#endif
}
//---------------------------------------------------------------------------
bool TVPRegisterGlobalObject(const tjs_char *name, iTJSDispatch2 * dsp)
{
	// register given object to global object
	tTJSVariant val(dsp);
	iTJSDispatch2 *global = TVPGetScriptDispatch();
	tjs_error er;
	try
	{
		er = global->PropSet(TJS_MEMBERENSURE, name, NULL, &val, global);
	}
	catch(...)
	{
		global->Release();
		return false;
	}
	global->Release();
	return TJS_SUCCEEDED(er);
}
//---------------------------------------------------------------------------
bool TVPRemoveGlobalObject(const tjs_char *name)
{
	// remove registration of global object
	iTJSDispatch2 *global = TVPGetScriptDispatch();
	if(!global) return false;
	tjs_error er;
	try
	{
		er = global->DeleteMember(0, name, NULL, global);
	}
	catch(...)
	{
		global->Release();
		return false;
	}
	global->Release();
	return TJS_SUCCEEDED(er);
}
//---------------------------------------------------------------------------
void TVPDoTryBlock(
	tTVPTryBlockFunction tryblock,
	tTVPCatchBlockFunction catchblock,
	tTVPFinallyBlockFunction finallyblock,
	void *data)
{
	try
	{
		tryblock(data);
	}
	catch(const eTJS & e)
	{
		if(finallyblock) finallyblock(data);
		tTVPExceptionDesc desc;
		desc.type = TJS_W("eTJS");
		desc.message = e.GetMessage();
		if(catchblock(data, desc)) throw;
		return;
	}
	catch(...)
	{
		if(finallyblock) finallyblock(data);
		tTVPExceptionDesc desc;
		desc.type = TJS_W("unknown");
		if(catchblock(data, desc)) throw;
		return;
	}
	if(finallyblock) finallyblock(data);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPGetFileVersionOf
//---------------------------------------------------------------------------
#ifdef _WIN32
bool TVPGetFileVersionOf(const wchar_t* module_filename, tjs_int &major, tjs_int &minor, tjs_int &release, tjs_int &build)
{
	// retrieve file version
	major = minor = release = build = 0;

	VS_FIXEDFILEINFO *FixedFileInfo;
	BYTE *VersionInfo;
	bool got = false;

	UINT dum;
	DWORD dum2;

	tjs_char* filename = new tjs_char[TJS_strlen(module_filename) + 1];
	try
	{
		TJS_strcpy(filename, module_filename);

		DWORD size = ::GetFileVersionInfoSize (filename, &dum2);
		if(size)
		{
			VersionInfo = new BYTE[size + 2];
			try
			{
				if(::GetFileVersionInfo(filename, 0, size, (void*)VersionInfo))
				{
					if(::VerQueryValue((void*)VersionInfo, TJS_W("\\"), (void**)(&FixedFileInfo),
						&dum))
					{
						major   = FixedFileInfo->dwFileVersionMS >> 16;
						minor   = FixedFileInfo->dwFileVersionMS & 0xffff;
						release = FixedFileInfo->dwFileVersionLS >> 16;
						build   = FixedFileInfo->dwFileVersionLS & 0xffff;
						got = true;
					}
				}
			}
			catch(...)
			{
				delete [] VersionInfo;
				throw;
			}
			delete [] VersionInfo;
		}
	}
	catch(...)
	{
		delete [] filename;
		throw;
	}

	delete [] filename;

	return got;
}
//---------------------------------------------------------------------------
#endif



//---------------------------------------------------------------------------
// TVPCreateNativeClass_Plugins
//---------------------------------------------------------------------------
tTJSNativeClass * TVPCreateNativeClass_Plugins()
{
	tTJSNC_Plugins *cls = new tTJSNC_Plugins();


	// setup some platform-specific members
//---------------------------------------------------------------------------

//-- methods

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/link)
{
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr name = *param[0];

	TVPLoadPlugin(name);

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL_OUTER(/*object to register*/cls,
	/*func. name*/link)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/unlink)
{
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr name = *param[0];

	bool res = TVPUnloadPlugin(name);

	if(result) *result = (tjs_int)res;

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL_OUTER(/*object to register*/cls,
	/*func. name*/unlink)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(getList)
{
	iTJSDispatch2 * array = TJSCreateArrayObject();
	try
	{
#ifdef KRKRSDL2_ENABLE_PLUGINS
		tTVPPluginVectorType::iterator i;
		tjs_int idx = 0;
		for(i = TVPPluginVector.Vector.begin(); i != TVPPluginVector.Vector.end(); i++)
		{
			tTJSVariant val = (*i)->Name.c_str();
			array->PropSetByNum(TJS_MEMBERENSURE, idx++, &val, array);
		}
#endif

		if (result) *result = tTJSVariant(array, array);
	}
	catch(...)
	{
		array->Release();
		throw;
	}
	array->Release();
	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL_OUTER(cls, getList)
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
	return cls;
}
//---------------------------------------------------------------------------
