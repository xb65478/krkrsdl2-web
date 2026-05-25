/* SPDX-License-Identifier: MIT */
/* Copyright (c) Kirikiri SDL2 Developers */

#include "tjsCommHead.h"
#include "OpenALWaveMixer.h"
#include "WaveImpl.h"
#include "DebugIntf.h"
#include "SysInitIntf.h"
#include "LogFilter.h"
#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <unordered_set>
#include <vector>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifndef TVP_FAUDIO_IMPLEMENT
class tTVPAudioRenderer;
static tTVPAudioRenderer *TVPAudioRenderer;
static tjs_uint64 TVPAudioDiagOpenALCreateStreamCount = 0;
static tjs_uint64 TVPAudioDiagOpenALAppendBufferCount = 0;
static tjs_uint64 TVPAudioDiagOpenALAppendBytes = 0;
static tjs_uint64 TVPAudioDiagOpenALPlayCount = 0;
static tjs_uint64 TVPAudioDiagOpenALAppendBufferMsX1000 = 0;
static tjs_uint64 TVPAudioDiagOpenALLowQueuedCount = 0;
static tjs_int TVPAudioDiagOpenALMinQueuedBuffers = 0x7fffffff;

enum class tTVPAudioSinkState
{
	Closed,
	Opened,
	Prefilling,
	Playing,
	Paused,
	Stopping,
	Stopped,
	Draining,
	Error
};

static const char *TVPAudioSinkStateName(tTVPAudioSinkState state)
{
	switch(state)
	{
	case tTVPAudioSinkState::Closed: return "Closed";
	case tTVPAudioSinkState::Opened: return "Opened";
	case tTVPAudioSinkState::Prefilling: return "Prefilling";
	case tTVPAudioSinkState::Playing: return "Playing";
	case tTVPAudioSinkState::Paused: return "Paused";
	case tTVPAudioSinkState::Stopping: return "Stopping";
	case tTVPAudioSinkState::Stopped: return "Stopped";
	case tTVPAudioSinkState::Draining: return "Draining";
	case tTVPAudioSinkState::Error: return "Error";
	default: return "unknown";
	}
}

#ifdef __EMSCRIPTEN__
static double TVPAudioDiagOpenALLastLogMs = 0.0;
static tjs_uint64 TVPAudioDiagOpenALLastLoggedLowQueuedCount = 0;
static tjs_uint64 TVPAudioDiagWebAudioCreateStreamCount = 0;
static tjs_uint64 TVPAudioDiagWebAudioAppendBufferCount = 0;
static tjs_uint64 TVPAudioDiagWebAudioAppendBytes = 0;
static tjs_uint64 TVPAudioDiagWebAudioAppendMsX1000 = 0;
static double TVPAudioDiagWebAudioLastLogMs = 0.0;
static int TVPAudioDiagNextWebAudioSinkId = 1;

enum
{
	TVP_WA_CTRL_WRITE_POS = 0,
	TVP_WA_CTRL_READ_POS = 1,
	TVP_WA_CTRL_FILL_BYTES = 2,
	TVP_WA_CTRL_PLAYING = 3,
	TVP_WA_CTRL_VOLUME_X1000 = 4,
	TVP_WA_CTRL_PAN_X1000 = 5,
	TVP_WA_CTRL_UNDERRUNS = 6,
	TVP_WA_CTRL_DROPPED_BYTES = 7,
	TVP_WA_CTRL_PLAYED_LO = 8,
	TVP_WA_CTRL_PLAYED_HI = 9,
	TVP_WA_CTRL_GENERATION = 10,
	TVP_WA_CTRL_DESTROYED = 11,
	TVP_WA_CTRL_READY = 12,
	TVP_WA_CTRL_FAILED = 13,
	TVP_WA_CTRL_COUNT = 16
};

static int TVPAtomicLoad(const int32_t *ptr)
{
	return __atomic_load_n(ptr, __ATOMIC_SEQ_CST);
}

static void TVPAtomicStore(int32_t *ptr, int32_t value)
{
	__atomic_store_n(ptr, value, __ATOMIC_SEQ_CST);
}

static int TVPAtomicAdd(int32_t *ptr, int32_t value)
{
	return __atomic_add_fetch(ptr, value, __ATOMIC_SEQ_CST);
}

static bool TVPWasmOpenALFastAppendEnabled()
{
	static int enabled = -1;
	if (enabled < 0)
	{
		enabled = MAIN_THREAD_EM_ASM_INT({
			try {
				function disabled(value) {
					return value === 0 || value === false ||
						value === "0" || value === "false" ||
						value === "no" || value === "off";
				}
				var params = new URLSearchParams(location.search || "");
				if (disabled(params.get("krkr_audio_openal_fast_append"))) return 0;
				var moduleValue;
				if (typeof Module !== "undefined") {
					moduleValue = Module["krkr_audio_openal_fast_append"];
					if (moduleValue === undefined) moduleValue = Module["krkrAudioOpenALFastAppend"];
				}
				if (disabled(moduleValue)) return 0;
				if (disabled(globalThis.__krkr_audio_openal_fast_append)) return 0;
				return 1;
			} catch (e) {
				return 1;
			}
		});
	}
	return enabled != 0;
}

static bool TVPWasmOpenALBatchEnabled()
{
	static int enabled = -1;
	if (enabled < 0)
	{
		enabled = MAIN_THREAD_EM_ASM_INT({
			try {
				function disabled(value) {
					return value === 0 || value === false ||
						value === "0" || value === "false" ||
						value === "no" || value === "off";
				}
				var params = new URLSearchParams(location.search || "");
				if (disabled(params.get("krkr_audio_openal_batch"))) return 0;
				var moduleValue;
				if (typeof Module !== "undefined") {
					moduleValue = Module["krkr_audio_openal_batch"];
					if (moduleValue === undefined) moduleValue = Module["krkrAudioOpenALBatch"];
				}
				if (disabled(moduleValue)) return 0;
				if (disabled(globalThis.__krkr_audio_openal_batch)) return 0;
				return 1;
			} catch (e) {
				return 1;
			}
		});
	}
	return enabled != 0;
}

static bool TVPWasmDirectWebAudioEnabled()
{
	static int enabled = -1;
	if (enabled < 0)
	{
		enabled = MAIN_THREAD_EM_ASM_INT({
			try {
				function disabled(value) {
					return value === 0 || value === false ||
						value === "0" || value === "false" ||
						value === "no" || value === "off";
				}
				var params = new URLSearchParams(location.search || "");
				var backend = params.get("krkr_audio_backend");
				var direct = params.get("krkr_audio_direct_webaudio");
				if (backend && /openal/i.test(backend)) return 0;
				if (disabled(direct)) return 0;
				var moduleValue;
				if (typeof Module !== "undefined") {
					moduleValue = Module["krkr_audio_direct_webaudio"];
					if (moduleValue === undefined) moduleValue = Module["krkrAudioDirectWebAudio"];
					if (disabled(Module["krkr_audio_backend"]) ||
						(Module["krkr_audio_backend"] && /openal/i.test(String(Module["krkr_audio_backend"])))) return 0;
				}
				if (disabled(moduleValue)) return 0;
				if (disabled(globalThis.__krkr_audio_direct_webaudio)) return 0;
				if (globalThis.__krkr_audio_backend && /openal/i.test(String(globalThis.__krkr_audio_backend))) return 0;
				return 1;
			} catch (e) {
				return 1;
			}
		});
	}
	return enabled != 0;
}

static int TVPWasmWebAudioCreateSink(tTVPWaveFormat &fmt, int capacityBytes)
{
	return MAIN_THREAD_EM_ASM_INT({
		try {
			if (!globalThis.__krkrDirectWebAudio) return 0;
			return globalThis.__krkrDirectWebAudio.create($0, $1, $2, $3) | 0;
		} catch (e) {
			try { console.warn("[KRKR] direct WebAudio create failed", e); } catch (_) {}
			return 0;
		}
	}, fmt.SamplesPerSec, fmt.Channels, fmt.BitsPerSample, capacityBytes);
}

static int TVPWasmWebAudioCreateWasmRingSink(int sinkId, int32_t *ctrl, tjs_uint8 *pcm,
	int capacityBytes, tTVPWaveFormat &fmt)
{
	return MAIN_THREAD_EM_ASM_INT({
		try {
			if (!globalThis.__krkrDirectWebAudio) return 0;
			return globalThis.__krkrDirectWebAudio.createWasmRing($0, HEAPU8, $1, $2, $3, $4, $5, $6) | 0;
		} catch (e) {
			try { console.warn("[KRKR] direct WebAudio wasm ring create failed", e); } catch (_) {}
			return 0;
		}
	}, sinkId, ctrl, pcm, capacityBytes, fmt.SamplesPerSec, fmt.Channels, fmt.BitsPerSample);
}

static int TVPWasmWebAudioAppendSink(int sinkId, const void *buf, unsigned int len)
{
	return MAIN_THREAD_EM_ASM_INT({
		try {
			if (!globalThis.__krkrDirectWebAudio) return -1;
			return globalThis.__krkrDirectWebAudio.append($0, HEAPU8, $1, $2) | 0;
		} catch (e) {
			try { console.warn("[KRKR] direct WebAudio append failed", e); } catch (_) {}
			return -1;
		}
	}, sinkId, buf, len);
}

static void TVPWasmWebAudioPlaySink(int sinkId)
{
	MAIN_THREAD_EM_ASM({
		try {
			if (globalThis.__krkrDirectWebAudio) globalThis.__krkrDirectWebAudio.play($0);
		} catch (e) {}
	}, sinkId);
}

static void TVPWasmWebAudioPauseSink(int sinkId)
{
	MAIN_THREAD_EM_ASM({
		try {
			if (globalThis.__krkrDirectWebAudio) globalThis.__krkrDirectWebAudio.pause($0);
		} catch (e) {}
	}, sinkId);
}

static void TVPWasmWebAudioResetSink(int sinkId, bool resetCounters)
{
	MAIN_THREAD_EM_ASM({
		try {
			if (globalThis.__krkrDirectWebAudio) globalThis.__krkrDirectWebAudio.reset($0, !!$1);
		} catch (e) {}
	}, sinkId, resetCounters ? 1 : 0);
}

static void TVPWasmWebAudioSetParamsSink(int sinkId, float volume, float pan)
{
	MAIN_THREAD_EM_ASM({
		try {
			if (globalThis.__krkrDirectWebAudio) globalThis.__krkrDirectWebAudio.setParams($0, $1, $2);
		} catch (e) {}
	}, sinkId, volume, pan);
}

static void TVPWasmWebAudioDestroySink(int sinkId)
{
	MAIN_THREAD_EM_ASM({
		try {
			if (globalThis.__krkrDirectWebAudio) globalThis.__krkrDirectWebAudio.destroy($0);
		} catch (e) {}
	}, sinkId);
}

static int TVPWasmWebAudioStatSink(int sinkId, int code)
{
	return MAIN_THREAD_EM_ASM_INT({
		try {
			if (!globalThis.__krkrDirectWebAudio) return -1;
			return globalThis.__krkrDirectWebAudio.stat($0, $1) | 0;
		} catch (e) {
			return -1;
		}
	}, sinkId, code);
}
#else
static bool TVPWasmOpenALFastAppendEnabled()
{
	return false;
}

static bool TVPWasmOpenALBatchEnabled()
{
	return false;
}
#endif

extern "C" tjs_uint64 KRKRSDL2AudioDiagGetOpenALCreateStreamCount()
{
	return TVPAudioDiagOpenALCreateStreamCount;
}

extern "C" tjs_uint64 KRKRSDL2AudioDiagGetOpenALAppendBufferCount()
{
	return TVPAudioDiagOpenALAppendBufferCount;
}

extern "C" tjs_uint64 KRKRSDL2AudioDiagGetOpenALAppendBytes()
{
	return TVPAudioDiagOpenALAppendBytes;
}

extern "C" tjs_uint64 KRKRSDL2AudioDiagGetOpenALPlayCount()
{
	return TVPAudioDiagOpenALPlayCount;
}

extern "C" tjs_uint64 KRKRSDL2AudioDiagGetOpenALAppendBufferMsX1000()
{
	return TVPAudioDiagOpenALAppendBufferMsX1000;
}

extern "C" tjs_uint64 KRKRSDL2AudioDiagGetOpenALLowQueuedCount()
{
	return TVPAudioDiagOpenALLowQueuedCount;
}

extern "C" tjs_int KRKRSDL2AudioDiagGetOpenALMinQueuedBuffers()
{
	return TVPAudioDiagOpenALMinQueuedBuffers == 0x7fffffff ? -1 : TVPAudioDiagOpenALMinQueuedBuffers;
}

#ifdef __EMSCRIPTEN__
class tTVPWebAudioSoundBuffer : public iTVPSoundBuffer
{
	int _sinkId = 0;
	tTVPWaveFormat _format;
	tTJSCriticalSection _mtx;
	bool _playing = false;
	float _volume = 1.0f;
	float _pan = 0.0f;
	int _frameSize = 0;
	unsigned int _accessUnitBytes = 0;
	tjs_uint64 _appendCount = 0;
	tjs_uint64 _appendBytes = 0;
	int _capacityBytes = 0;
	int32_t *_ctrl = nullptr;
	tjs_uint8 *_pcm = nullptr;
	bool _wasmRing = false;
	int _lastLoggedUnderruns = 0;
	tTVPAudioSinkState _state = tTVPAudioSinkState::Closed;
	tjs_uint64 _generation = 1;

	int AlignBytes(int bytes) const
	{
		if (_frameSize <= 0)
			return bytes;
		return (bytes / _frameSize) * _frameSize;
	}

	unsigned int DefaultAccessUnitBytes() const
	{
		if (_format.SamplesPerSec <= 0 || _frameSize <= 0)
			return _frameSize > 0 ? static_cast<unsigned int>(_frameSize) : 1;
		return static_cast<unsigned int>(_format.SamplesPerSec / 8) *
			static_cast<unsigned int>(_frameSize);
	}

	int FilledBytes() const
	{
		if (!_sinkId)
			return 0;
		if (_wasmRing && _ctrl)
			return TVPAtomicLoad(_ctrl + TVP_WA_CTRL_FILL_BYTES);
		int filled = TVPWasmWebAudioStatSink(_sinkId, 1);
		return filled < 0 ? 0 : filled;
	}

	int FreeBytes() const
	{
		if (!_sinkId)
			return 0;
		if (_wasmRing && _ctrl)
			return _capacityBytes - TVPAtomicLoad(_ctrl + TVP_WA_CTRL_FILL_BYTES);
		int freeBytes = TVPWasmWebAudioStatSink(_sinkId, 2);
		return freeBytes < 0 ? 0 : freeBytes;
	}

	int DroppedBytes() const
	{
		if (!_sinkId)
			return 0;
		if (_wasmRing && _ctrl)
			return TVPAtomicLoad(_ctrl + TVP_WA_CTRL_DROPPED_BYTES);
		int dropped = TVPWasmWebAudioStatSink(_sinkId, 3);
		return dropped < 0 ? 0 : dropped;
	}

	int UnderrunCount() const
	{
		if (!_sinkId)
			return 0;
		if (_wasmRing && _ctrl)
			return TVPAtomicLoad(_ctrl + TVP_WA_CTRL_UNDERRUNS);
		int underrun = TVPWasmWebAudioStatSink(_sinkId, 4);
		return underrun < 0 ? 0 : underrun;
	}

	int ReadyState() const
	{
		if (!_sinkId)
			return 0;
		if (_wasmRing && _ctrl)
			return TVPAtomicLoad(_ctrl + TVP_WA_CTRL_READY);
		int ready = TVPWasmWebAudioStatSink(_sinkId, 7);
		return ready < 0 ? 0 : ready;
	}

	tjs_uint64 GenerationLocked() const
	{
		if (_wasmRing && _ctrl)
			return static_cast<tjs_uint64>(TVPAtomicLoad(_ctrl + TVP_WA_CTRL_GENERATION));
		return _generation;
	}

	tjs_uint64 PlayedSamplesLocked() const
	{
		if (!_sinkId)
			return 0;
		tjs_uint lo = static_cast<tjs_uint>(_wasmRing && _ctrl ?
			TVPAtomicLoad(_ctrl + TVP_WA_CTRL_PLAYED_LO) : TVPWasmWebAudioStatSink(_sinkId, 5));
		tjs_uint hi = static_cast<tjs_uint>(_wasmRing && _ctrl ?
			TVPAtomicLoad(_ctrl + TVP_WA_CTRL_PLAYED_HI) : TVPWasmWebAudioStatSink(_sinkId, 6));
		return (static_cast<tjs_uint64>(hi) << 32) | lo;
	}

	void SetStateLocked(tTVPAudioSinkState state, const char *action)
	{
		if (_state == state && (!TVPPerfEnabled() || !action))
			return;
		_state = state;
		if (TVPPerfEnabled())
			LogSinkStateLocked(action ? action : "state");
	}

	void LogSinkStateLocked(const char *action) const
	{
		if (!TVPPerfEnabled())
			return;
		fprintf(stderr,
			"[PERF] audio.sink action=%s sink_id=%d backend=%s state=%s generation=%llu playing=%d ring_fill_bytes=%d free_bytes=%d played_samples=%llu underrun=%d dropped=%d ready=%d rate=%d ch=%d\n",
			action ? action : "state",
			_sinkId,
			GetDebugBackendName(),
			TVPAudioSinkStateName(_state),
			(unsigned long long)GenerationLocked(),
			_playing ? 1 : 0,
			FilledBytes(),
			FreeBytes(),
			(unsigned long long)PlayedSamplesLocked(),
			UnderrunCount(),
			DroppedBytes(),
			ReadyState(),
			_format.SamplesPerSec,
			_format.Channels);
	}

	void ResetRingLocked(bool resetCounters)
	{
		if (_wasmRing && _ctrl)
		{
			TVPAtomicStore(_ctrl + TVP_WA_CTRL_PLAYING, 0);
			TVPAtomicStore(_ctrl + TVP_WA_CTRL_WRITE_POS, 0);
			TVPAtomicStore(_ctrl + TVP_WA_CTRL_READ_POS, 0);
			TVPAtomicStore(_ctrl + TVP_WA_CTRL_FILL_BYTES, 0);
			_generation = static_cast<tjs_uint64>(TVPAtomicAdd(_ctrl + TVP_WA_CTRL_GENERATION, 1));
			if (resetCounters)
			{
				TVPAtomicStore(_ctrl + TVP_WA_CTRL_UNDERRUNS, 0);
				TVPAtomicStore(_ctrl + TVP_WA_CTRL_DROPPED_BYTES, 0);
				TVPAtomicStore(_ctrl + TVP_WA_CTRL_PLAYED_LO, 0);
				TVPAtomicStore(_ctrl + TVP_WA_CTRL_PLAYED_HI, 0);
				_lastLoggedUnderruns = 0;
			}
			return;
		}
		if (_sinkId)
			TVPWasmWebAudioResetSink(_sinkId, resetCounters);
		_generation++;
	}

	int AppendWasmRingLocked(const void *buf, unsigned int len)
	{
		if (!_ctrl || !_pcm || _capacityBytes <= 0 || _frameSize <= 0)
			return -1;
		unsigned int alignedLen = static_cast<unsigned int>(AlignBytes(static_cast<int>(len)));
		if (alignedLen == 0)
			return FilledBytes();
		const tjs_uint8 *src = static_cast<const tjs_uint8*>(buf);
		if (static_cast<int>(alignedLen) > _capacityBytes)
		{
			unsigned int keep = static_cast<unsigned int>(AlignBytes(_capacityBytes));
			src += alignedLen - keep;
			alignedLen = keep;
		}

		int fill = TVPAtomicLoad(_ctrl + TVP_WA_CTRL_FILL_BYTES);
		int readPos = TVPAtomicLoad(_ctrl + TVP_WA_CTRL_READ_POS);
		int freeBytes = _capacityBytes - fill;
		if (static_cast<int>(alignedLen) > freeBytes)
		{
			int drop = AlignBytes(static_cast<int>(alignedLen) - freeBytes + _frameSize - 1);
			if (drop > fill)
				drop = fill;
			readPos = (readPos + drop) % _capacityBytes;
			fill -= drop;
			TVPAtomicStore(_ctrl + TVP_WA_CTRL_READ_POS, readPos);
			TVPAtomicStore(_ctrl + TVP_WA_CTRL_FILL_BYTES, fill);
			TVPAtomicAdd(_ctrl + TVP_WA_CTRL_DROPPED_BYTES, drop);
		}

		int writePos = TVPAtomicLoad(_ctrl + TVP_WA_CTRL_WRITE_POS);
		int first = std::min(static_cast<int>(alignedLen), _capacityBytes - writePos);
		memcpy(_pcm + writePos, src, first);
		if (first < static_cast<int>(alignedLen))
			memcpy(_pcm, src + first, alignedLen - first);
		TVPAtomicStore(_ctrl + TVP_WA_CTRL_WRITE_POS, (writePos + static_cast<int>(alignedLen)) % _capacityBytes);
		return TVPAtomicAdd(_ctrl + TVP_WA_CTRL_FILL_BYTES, static_cast<int>(alignedLen));
	}

public:
	tTVPWebAudioSoundBuffer(tTVPWaveFormat &desired, int bufcount)
	{
		_format = desired;
		_frameSize = desired.BytesPerSample * desired.Channels;
		if (_frameSize <= 0)
			return;
		_accessUnitBytes = DefaultAccessUnitBytes();
		int capacityBytes = static_cast<int>(_format.SamplesPerSec * _frameSize * 2);
		int minByBufferCount = static_cast<int>(_accessUnitBytes * (bufcount + 2));
		if (capacityBytes < minByBufferCount)
			capacityBytes = minByBufferCount;
		capacityBytes = AlignBytes(capacityBytes);
		_sinkId = TVPAudioDiagNextWebAudioSinkId++;
		_ctrl = static_cast<int32_t*>(calloc(TVP_WA_CTRL_COUNT, sizeof(int32_t)));
		_pcm = static_cast<tjs_uint8*>(malloc(capacityBytes));
		if (_ctrl && _pcm)
		{
			memset(_pcm, 0, capacityBytes);
			_capacityBytes = capacityBytes;
			TVPAtomicStore(_ctrl + TVP_WA_CTRL_VOLUME_X1000, 1000);
			TVPAtomicStore(_ctrl + TVP_WA_CTRL_PAN_X1000, 0);
			TVPAtomicStore(_ctrl + TVP_WA_CTRL_GENERATION, static_cast<int32_t>(_generation));
			int createdId = TVPWasmWebAudioCreateWasmRingSink(_sinkId, _ctrl, _pcm, _capacityBytes, _format);
			if (createdId == _sinkId)
				_wasmRing = true;
		}
		if (!_wasmRing)
		{
			if (_ctrl)
				free(_ctrl), _ctrl = nullptr;
			if (_pcm)
				free(_pcm), _pcm = nullptr;
			_capacityBytes = capacityBytes;
			_sinkId = TVPWasmWebAudioCreateSink(_format, capacityBytes);
		}
		if (_sinkId)
		{
			TVPAudioDiagWebAudioCreateStreamCount++;
			TVPWasmWebAudioSetParamsSink(_sinkId, _volume, _pan);
			_state = tTVPAudioSinkState::Opened;
			if (TVPPerfEnabled())
			{
				tTJSCriticalSectionHolder holder(_mtx);
				LogSinkStateLocked("create");
			}
		}
	}

	virtual ~tTVPWebAudioSoundBuffer()
	{
		if (_sinkId)
		{
			if (TVPPerfEnabled())
			{
				tTJSCriticalSectionHolder holder(_mtx);
				SetStateLocked(tTVPAudioSinkState::Closed, "destroy");
			}
			TVPWasmWebAudioDestroySink(_sinkId);
		}
		if (_ctrl)
			free(_ctrl);
		if (_pcm)
			free(_pcm);
	}

	bool IsAvailable() const
	{
		return _sinkId != 0;
	}

	virtual void Release() override
	{
		delete this;
	}

	virtual void Play() override
	{
		tTJSCriticalSectionHolder holder(_mtx);
		_playing = true;
		_state = tTVPAudioSinkState::Playing;
		if (_wasmRing && _ctrl)
		{
			TVPAtomicStore(_ctrl + TVP_WA_CTRL_PLAYING, 1);
			MAIN_THREAD_ASYNC_EM_ASM({
				try {
					if (globalThis.__krkrDirectWebAudio) globalThis.__krkrDirectWebAudio.play($0);
				} catch (e) {}
			}, _sinkId);
		}
		else if (_sinkId)
			TVPWasmWebAudioPlaySink(_sinkId);
		LogSinkStateLocked("play");
	}

	virtual void Pause() override
	{
		tTJSCriticalSectionHolder holder(_mtx);
		_playing = false;
		_state = tTVPAudioSinkState::Paused;
		if (_wasmRing && _ctrl)
			TVPAtomicStore(_ctrl + TVP_WA_CTRL_PLAYING, 0);
		else if (_sinkId)
			TVPWasmWebAudioPauseSink(_sinkId);
		LogSinkStateLocked("pause");
	}

	virtual void Stop() override
	{
		tTJSCriticalSectionHolder holder(_mtx);
		_playing = false;
		_state = tTVPAudioSinkState::Stopping;
		ResetRingLocked(true);
		_state = tTVPAudioSinkState::Stopped;
		LogSinkStateLocked("stop");
	}

	virtual void Reset() override
	{
		tTJSCriticalSectionHolder holder(_mtx);
		_playing = false;
		ResetRingLocked(true);
		_state = tTVPAudioSinkState::Stopped;
		LogSinkStateLocked("reset");
	}

	virtual bool IsPlaying() override
	{
		return _playing;
	}

	virtual void SetVolume(float volume) override
	{
		tTJSCriticalSectionHolder holder(_mtx);
		_volume = volume;
		if (_wasmRing && _ctrl)
			TVPAtomicStore(_ctrl + TVP_WA_CTRL_VOLUME_X1000, static_cast<int32_t>(volume * 1000.0f));
		else if (_sinkId)
			TVPWasmWebAudioSetParamsSink(_sinkId, _volume, _pan);
	}

	virtual float GetVolume() override
	{
		return _volume;
	}

	virtual void SetPan(float pan) override
	{
		tTJSCriticalSectionHolder holder(_mtx);
		_pan = pan;
		if (_wasmRing && _ctrl)
			TVPAtomicStore(_ctrl + TVP_WA_CTRL_PAN_X1000, static_cast<int32_t>(pan * 1000.0f));
		else if (_sinkId)
			TVPWasmWebAudioSetParamsSink(_sinkId, _volume, _pan);
	}

	virtual float GetPan() override
	{
		return _pan;
	}

	virtual void SetPosition(float x, float y, float z) override
	{
		(void)y;
		(void)z;
		SetPan(x);
	}

	virtual bool IsBufferValid() override
	{
		tTJSCriticalSectionHolder holder(_mtx);
		if (!_sinkId)
			return false;
		unsigned int unitBytes = _accessUnitBytes ? _accessUnitBytes : DefaultAccessUnitBytes();
		return FreeBytes() >= static_cast<int>(unitBytes);
	}

	virtual void AppendBuffer(const void *buf, unsigned int len) override
	{
		if (!_sinkId || !buf || len == 0 || _frameSize <= 0)
			return;
		double startMs = TVPPerfEnabled() ? emscripten_get_now() : 0.0;
		int filled = 0;
		int dropped = 0;
		int underruns = 0;
		int underrunDelta = 0;
		int ready = 0;
		{
			tTJSCriticalSectionHolder holder(_mtx);
			if (!_playing && (_state == tTVPAudioSinkState::Opened ||
				_state == tTVPAudioSinkState::Stopped ||
				_state == tTVPAudioSinkState::Paused))
				_state = tTVPAudioSinkState::Prefilling;
			if (len > _accessUnitBytes)
				_accessUnitBytes = len;
			filled = _wasmRing ? AppendWasmRingLocked(buf, len) : TVPWasmWebAudioAppendSink(_sinkId, buf, len);
			_appendCount++;
			_appendBytes += len;
			TVPAudioDiagWebAudioAppendBufferCount++;
			TVPAudioDiagWebAudioAppendBytes += len;
			if (TVPPerfEnabled())
			{
				dropped = DroppedBytes();
				underruns = UnderrunCount();
				underrunDelta = underruns - _lastLoggedUnderruns;
				_lastLoggedUnderruns = underruns;
				ready = ReadyState();
			}
		}
		if (TVPPerfEnabled())
		{
			double endMs = emscripten_get_now();
			double totalMs = endMs - startMs;
			TVPAudioDiagWebAudioAppendMsX1000 += (tjs_uint64)(totalMs * 1000.0);
			bool printSample = totalMs >= 2.0 ||
				TVPAudioDiagWebAudioAppendBufferCount <= 4 ||
				(TVPAudioDiagWebAudioLastLogMs > 0.0 && endMs - TVPAudioDiagWebAudioLastLogMs >= 1000.0);
			if (printSample)
			{
				TVPAudioDiagWebAudioLastLogMs = endMs;
				fprintf(stderr,
					"[PERF] audio.webaudio_append sink_id=%d mode=%s bytes=%u ring_fill_bytes=%d free_bytes=%d copy_ms=%.3f dropped=%d underrun=%d underrun_delta=%d total_ms=%.3f ready=%d rate=%d ch=%d\n",
					_sinkId,
					_wasmRing ? "wasm-ring" : "js-sab",
					len,
					filled,
					FreeBytes(),
					totalMs,
					dropped,
					underruns,
					underrunDelta,
					totalMs,
					ready,
					_format.SamplesPerSec,
					_format.Channels);
			}
			if (underrunDelta != 0)
			{
				tTJSCriticalSectionHolder holder(_mtx);
				LogSinkStateLocked("underrun");
			}
		}
	}

	virtual tjs_uint GetCurrentPlaySamples() override
	{
		tjs_uint64 samples = PlayedSamplesLocked();
		return static_cast<tjs_uint>(samples & 0xffffffffu);
	}

	virtual tjs_uint GetLatencySamples() override
	{
		if (_frameSize <= 0)
			return 0;
		return static_cast<tjs_uint>(FilledBytes() / _frameSize);
	}

	virtual int GetRemainBuffers() override
	{
		tTJSCriticalSectionHolder holder(_mtx);
		unsigned int unitBytes = _accessUnitBytes ? _accessUnitBytes : DefaultAccessUnitBytes();
		if (!unitBytes)
			unitBytes = 1;
		return FilledBytes() / static_cast<int>(unitBytes);
	}

	virtual int GetDebugSinkId() const override
	{
		return _sinkId;
	}

	virtual const char *GetDebugBackendName() const override
	{
		return _wasmRing ? "webaudio-wasm-ring" : "webaudio-js-sab";
	}

	virtual const char *GetDebugStateName() const override
	{
		return TVPAudioSinkStateName(_state);
	}

	virtual tjs_uint64 GetDebugGeneration() const override
	{
		return GenerationLocked();
	}

	virtual tjs_uint64 GetDebugPlayedSamples64() override
	{
		return PlayedSamplesLocked();
	}

	virtual tjs_int GetDebugRingFillBytes() const override
	{
		return FilledBytes();
	}

	virtual tjs_int GetDebugUnderrunCount() const override
	{
		return UnderrunCount();
	}

	virtual bool WantsStartPrefill() const override
	{
		return true;
	}
};
#endif

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#ifndef AL_FORMAT_MONO_FLOAT32
#define AL_FORMAT_MONO_FLOAT32   0x10010
#endif

#ifndef AL_FORMAT_STEREO_FLOAT32
#define AL_FORMAT_STEREO_FLOAT32 0x10011
#endif

class tTVPSoundBuffer : public iTVPSoundBuffer
{
	ALuint _alSource;
	ALenum _alFormat;
	ALuint *_bufferIds;
	ALuint *_bufferIds2;
	tjs_uint *_bufferSize;
	tjs_uint _bufferCount;
	int _bufferIdx = -1;
	tTVPWaveFormat _format;
	tTJSCriticalSection _buffer_mtx;
	tjs_uint _sentSamples = 0;
	bool _playing = false;
	int _frame_size = 0;
	bool has_format = false;
	ALfloat _volume;
	ALfloat _pan;
	ALfloat _sourcePos[3];
	bool _gainDirty = true;
	bool _positionDirty = true;
	bool _playStateDirty = true;
	unsigned int _lastAppendBytes = 0;
	tTVPAudioSinkState _state = tTVPAudioSinkState::Opened;
	tjs_uint64 _generation = 1;
#ifdef __EMSCRIPTEN__
	std::vector<tjs_uint8> _pendingPcm;
#endif
	void DrainProcessedBuffersLocked()
	{
		ALint processed = 0;
		alGetSourcei(_alSource, AL_BUFFERS_PROCESSED, &processed);
		if (processed <= 0)
		{
			return;
		}

		alSourceUnqueueBuffers(_alSource, processed, _bufferIds2);
		CheckALError("alSourceUnqueueBuffers");
		for (int i = 0; i < processed; i += 1)
		{
			for (int j = 0; j < static_cast<int>(_bufferCount); j += 1)
			{
				if (_bufferIds[j] == _bufferIds2[i])
				{
					_sentSamples += _bufferSize[j] / _frame_size;
					_bufferSize[j] = 0;
					break;
				}
			}
		}
	}

	void ClearPendingBufferLocked()
	{
#ifdef __EMSCRIPTEN__
		_pendingPcm.clear();
#endif
	}

	void LogSinkStateLocked(const char *action)
	{
		if (!TVPPerfEnabled())
			return;
		ALint processed = 0;
		ALint queued = 0;
		ALint offset = 0;
		alGetSourcei(_alSource, AL_BUFFERS_PROCESSED, &processed);
		alGetSourcei(_alSource, AL_BUFFERS_QUEUED, &queued);
		alGetSourcei(_alSource, AL_SAMPLE_OFFSET, &offset);
		fprintf(stderr,
			"[PERF] audio.sink action=%s sink_id=%d backend=%s state=%s generation=%llu playing=%d queued=%d processed=%d played_samples=%llu underrun=%d ring_fill_bytes=%d rate=%d ch=%d\n",
			action ? action : "state",
			GetDebugSinkId(),
			GetDebugBackendName(),
			TVPAudioSinkStateName(_state),
			(unsigned long long)_generation,
			_playing ? 1 : 0,
			(int)queued,
			(int)processed,
			(unsigned long long)(_sentSamples + offset),
			-1,
			-1,
			_format.SamplesPerSec,
			_format.Channels);
	}

	unsigned int GetBatchTargetBytes(unsigned int appendLen) const
	{
#ifdef __EMSCRIPTEN__
		if (!TVPWasmOpenALFastAppendEnabled() || !TVPWasmOpenALBatchEnabled())
		{
			return 0;
		}
		if (_format.SamplesPerSec <= 0 || _frame_size <= 0)
		{
			return 0;
		}
		unsigned int target = static_cast<unsigned int>(_format.SamplesPerSec / 4) *
			static_cast<unsigned int>(_frame_size);
		unsigned int minimum = appendLen * 2;
		if (target < minimum)
		{
			target = minimum;
		}
		return target;
#else
		(void)appendLen;
		return 0;
#endif
	}

	ALint GetQueueLimitLocked() const
	{
#ifdef __EMSCRIPTEN__
		if (TVPWasmOpenALFastAppendEnabled() && TVPWasmOpenALBatchEnabled())
		{
			ALint limit = static_cast<ALint>(_bufferCount / 2);
			return limit < 2 ? 2 : limit;
		}
#endif
		return static_cast<ALint>(_bufferCount);
	}

	bool SubmitBufferLocked(const void *buf, unsigned int len, ALint &queued,
		double *bufferDataMs, double *queueMs)
	{
		if (queued >= static_cast<ALint>(_bufferCount))
		{
			return false;
		}
		_bufferIdx += 1;
		if (_bufferIdx >= static_cast<int>(_bufferCount))
		{
			_bufferIdx = 0;
		}
		ALuint bufid = _bufferIds[_bufferIdx];
		_bufferSize[_bufferIdx] = len;
#ifdef __EMSCRIPTEN__
		double step = TVPPerfEnabled() ? emscripten_get_now() : 0.0;
#endif
		alBufferData(bufid, _alFormat, buf, len, _format.SamplesPerSec);
		CheckALError("alBufferData");
#ifdef __EMSCRIPTEN__
		if (TVPPerfEnabled() && bufferDataMs)
		{
			double now = emscripten_get_now();
			*bufferDataMs += now - step;
			step = now;
		}
#else
		(void)bufferDataMs;
#endif
		alSourceQueueBuffers(_alSource, 1, &bufid);
		CheckALError("alSourceQueueBuffers");
#ifdef __EMSCRIPTEN__
		if (TVPPerfEnabled() && queueMs)
		{
			*queueMs += emscripten_get_now() - step;
		}
#else
		(void)queueMs;
#endif
		queued += 1;
		return true;
	}

	bool FlushPendingBufferLocked(ALint &queued, double *bufferDataMs, double *queueMs)
	{
#ifdef __EMSCRIPTEN__
		if (_pendingPcm.empty())
		{
			return false;
		}
		if (!SubmitBufferLocked(_pendingPcm.data(), static_cast<unsigned int>(_pendingPcm.size()),
			queued, bufferDataMs, queueMs))
		{
			return false;
		}
		_pendingPcm.clear();
		return true;
#else
		(void)queued;
		(void)bufferDataMs;
		(void)queueMs;
		return false;
#endif
	}

	void EnsurePlayStateLocked(bool queryState)
	{
		ALenum state = AL_INITIAL;
		if (queryState || _playStateDirty)
		{
			alGetSourcei(_alSource, AL_SOURCE_STATE, &state);
			CheckALError("alGetSourcei AL_SOURCE_STATE");
		}
		if (_playing)
		{
			if (_gainDirty)
			{
				alSourcef(_alSource, AL_GAIN, _volume);
				CheckALError("alSourcef AL_GAIN");
				_gainDirty = false;
			}
			if (_positionDirty)
			{
				alSourcefv(_alSource, AL_POSITION, _sourcePos);
				CheckALError("alSourcefv AL_POSITION");
				_positionDirty = false;
			}
			if ((queryState || _playStateDirty) && state != AL_PLAYING)
			{
				alSourcePlay(_alSource);
				CheckALError("alSourcePlay");
			}
		}
		else
		{
			if (_gainDirty)
			{
				alSourcef(_alSource, AL_GAIN, 0.0f);
				CheckALError("alSourcef AL_GAIN");
				_gainDirty = false;
			}
			if ((queryState || _playStateDirty) && state == AL_PLAYING)
			{
				alSourcePause(_alSource);
				CheckALError("alSourcePause");
				alGetSourcei(_alSource, AL_SOURCE_STATE, &state);
				CheckALError("alGetSourcei AL_SOURCE_STATE");
			}
			if (_bufferIdx == -1 && (queryState || _playStateDirty))
			{
				if (state != AL_STOPPED)
				{
					alSourceStop(_alSource);
					CheckALError("alSourceStop");
					if (_sentSamples == 0)
					{
						alSourceRewind(_alSource);
						alSourcei(_alSource, AL_BUFFER, 0);
					}
				}
			}
		}
		_playStateDirty = false;
	}
public:
	tTVPSoundBuffer(tTVPWaveFormat &desired, int bufcount)
	{
		_bufferCount = bufcount;
		_frame_size = desired.BytesPerSample * desired.Channels;
		_bufferIds = new ALuint[bufcount];
		_bufferIds2 = new ALuint[bufcount];
		_bufferSize = new tjs_uint[bufcount];
		memset(_bufferIds2, 0, sizeof(ALuint) * bufcount);
		memset(_bufferSize, 0, sizeof(tjs_uint) * bufcount);
		_format = desired;
		_volume = 1.0f;
		_sourcePos[0] = 0.0f;
		_sourcePos[1] = 0.0f;
		_sourcePos[2] = 0.0f;
		alGenSources(1, &_alSource);
		alGenBuffers(_bufferCount, _bufferIds);
		alSourcef(_alSource, AL_GAIN, 0.0f);
		if (TVPPerfEnabled())
			LogSinkStateLocked("create");
		has_format = true;
		switch (desired.Channels)
		{
			case 1:
			{
				switch (desired.BitsPerSample)
				{
					case 8:
					{
						_alFormat = AL_FORMAT_MONO8;
						break;
					}
					case 16:
					{
						_alFormat = AL_FORMAT_MONO16;
						break;
					}
					case 32:
					{
						_alFormat = AL_FORMAT_MONO_FLOAT32;
						break;
					}
					default:
					{
						has_format = false;
						break;
					}
				}
				break;
			}
			case 2:
			{
				switch (desired.BitsPerSample)
				{
					case 8:
					{
						_alFormat = AL_FORMAT_STEREO8;
						break;
					}
					case 16:
					{
						_alFormat = AL_FORMAT_STEREO16;
						break;
					}
					case 32:
					{
						_alFormat = AL_FORMAT_STEREO_FLOAT32;
						break;
					}
					default:
					{
						has_format = false;
						break;
					}
				}
				break;
			}
			default:
			{
				has_format = false;
				break;
			}
		}
	}

	virtual ~tTVPSoundBuffer()
	{
		if (TVPPerfEnabled())
		{
			tTJSCriticalSectionHolder holder(_buffer_mtx);
			_state = tTVPAudioSinkState::Closed;
			LogSinkStateLocked("destroy");
		}
		alDeleteBuffers(_bufferCount, _bufferIds);
		alDeleteSources(1, &_alSource);
		delete[] _bufferIds;
		delete[] _bufferIds2;
		delete[] _bufferSize;
	}

	virtual void Release() override
	{
		delete this;
	}

	bool IsBufferValid() override
	{
		tTJSCriticalSectionHolder holder(_buffer_mtx);
		DrainProcessedBuffersLocked();
		ALint queued = 0;
		alGetSourcei(_alSource, AL_BUFFERS_QUEUED, &queued);
		return queued < GetQueueLimitLocked();
	}

	virtual void AppendBuffer(const void *buf, unsigned int len) override
	{
#ifdef __EMSCRIPTEN__
		double perf_start_ms = TVPPerfEnabled() ? emscripten_get_now() : 0.0;
		double perf_after_drain_ms = perf_start_ms;
		double perf_after_queued_ms = perf_start_ms;
		double perf_buffer_data_ms = 0.0;
		double perf_queue_ms = 0.0;
		double perf_ensure_play_ms = 0.0;
		unsigned int submittedBytes = 0;
		unsigned int pendingBytes = 0;
		bool didSubmit = false;
		bool batched = false;
		bool flushedPending = false;
		bool fastAppend = TVPWasmOpenALFastAppendEnabled();
		bool batchEnabled = fastAppend && TVPWasmOpenALBatchEnabled();
#endif
		if (len <= 0)
		{
			return;
		}
		if (!has_format)
		{
			return;
		}
		TVPAudioDiagOpenALAppendBufferCount++;
		TVPAudioDiagOpenALAppendBytes += len;
		tTJSCriticalSectionHolder holder(_buffer_mtx);
		DrainProcessedBuffersLocked();
#ifdef __EMSCRIPTEN__
		if (TVPPerfEnabled())
			perf_after_drain_ms = emscripten_get_now();
#endif

		ALint queued = 0;
		alGetSourcei(_alSource, AL_BUFFERS_QUEUED, &queued);
		ALint queuedBefore = queued;
#ifdef __EMSCRIPTEN__
		if (TVPPerfEnabled())
			perf_after_queued_ms = emscripten_get_now();
#endif
		if (queued < TVPAudioDiagOpenALMinQueuedBuffers)
			TVPAudioDiagOpenALMinQueuedBuffers = queued;
		if (queued <= 1)
			TVPAudioDiagOpenALLowQueuedCount++;

		if (queued >= GetQueueLimitLocked())
		{
			return;
		}
#ifdef __EMSCRIPTEN__
		bool shortFinalLikeBlock = _lastAppendBytes > 0 && len < _lastAppendBytes;
		if (len > _lastAppendBytes)
			_lastAppendBytes = len;
		unsigned int batchTargetBytes = batchEnabled ? GetBatchTargetBytes(len) : 0;
		if (batchTargetBytes > 0 && queued > 1 && !shortFinalLikeBlock)
		{
			_pendingPcm.insert(_pendingPcm.end(),
				static_cast<const tjs_uint8*>(buf),
				static_cast<const tjs_uint8*>(buf) + len);
			pendingBytes = static_cast<unsigned int>(_pendingPcm.size());
			batched = pendingBytes < batchTargetBytes;
			if (!batched)
			{
				flushedPending = FlushPendingBufferLocked(queued, &perf_buffer_data_ms, &perf_queue_ms);
				didSubmit = flushedPending;
				submittedBytes = flushedPending ? pendingBytes : 0;
			}
		}
		else
		{
			FlushPendingBufferLocked(queued, &perf_buffer_data_ms, &perf_queue_ms);
			didSubmit = SubmitBufferLocked(buf, len, queued, &perf_buffer_data_ms, &perf_queue_ms);
			submittedBytes = didSubmit ? len : 0;
		}
		bool shouldEnsurePlay = !fastAppend || _playStateDirty || _gainDirty ||
			_positionDirty || queued <= 1 || didSubmit;
		if (shouldEnsurePlay)
		{
			double ensure_start_ms = TVPPerfEnabled() ? emscripten_get_now() : 0.0;
			EnsurePlayStateLocked(!fastAppend || queued <= 1 || _playStateDirty);
			if (TVPPerfEnabled())
				perf_ensure_play_ms = emscripten_get_now() - ensure_start_ms;
		}
#else
		bool didSubmit = SubmitBufferLocked(buf, len, queued, nullptr, nullptr);
		(void)didSubmit;
		EnsurePlayStateLocked(true);
#endif
#ifdef __EMSCRIPTEN__
		if (TVPPerfEnabled())
		{
			double perf_end_ms = emscripten_get_now();
			TVPAudioDiagOpenALAppendBufferMsX1000 += (tjs_uint64)((perf_end_ms - perf_start_ms) * 1000.0);
			bool lowQueuedChanged = TVPAudioDiagOpenALLowQueuedCount != TVPAudioDiagOpenALLastLoggedLowQueuedCount;
			bool printSample = perf_end_ms - perf_start_ms >= 4.0 ||
				TVPAudioDiagOpenALAppendBufferCount <= 4 ||
				(lowQueuedChanged && TVPAudioDiagOpenALLowQueuedCount <= 4) ||
				(lowQueuedChanged && TVPAudioDiagOpenALLowQueuedCount % 128 == 0) ||
				(TVPAudioDiagOpenALLastLogMs > 0.0 && perf_end_ms - TVPAudioDiagOpenALLastLogMs >= 1000.0);
			if (printSample)
			{
				TVPAudioDiagOpenALLastLogMs = perf_end_ms;
				TVPAudioDiagOpenALLastLoggedLowQueuedCount = TVPAudioDiagOpenALLowQueuedCount;
				fprintf(stderr,
					"[PERF] audio.openal_append len=%u submitted=%u pending=%u queued_before=%d queued_after=%d buffer_count=%u ms=%.3f min_queued=%d low_queued_count=%llu fast=%d batch=%d batched=%d flushed=%d\n",
					len,
					submittedBytes,
					pendingBytes,
					(int)queuedBefore,
					(int)queued,
					(unsigned)_bufferCount,
					perf_end_ms - perf_start_ms,
					(int)KRKRSDL2AudioDiagGetOpenALMinQueuedBuffers(),
					(unsigned long long)TVPAudioDiagOpenALLowQueuedCount,
					fastAppend ? 1 : 0,
					batchEnabled ? 1 : 0,
					batched ? 1 : 0,
					flushedPending ? 1 : 0);
				fprintf(stderr,
					"[PERF] audio.openal_append_detail drain_ms=%.3f queued_query_ms=%.3f buffer_data_ms=%.3f queue_ms=%.3f ensure_play_ms=%.3f total_ms=%.3f submitted=%d\n",
					perf_after_drain_ms - perf_start_ms,
					perf_after_queued_ms - perf_after_drain_ms,
					perf_buffer_data_ms,
					perf_queue_ms,
					perf_ensure_play_ms,
					perf_end_ms - perf_start_ms,
					didSubmit ? 1 : 0);
			}
		}
#endif
	}

	void EnsurePlayState()
	{
		tTJSCriticalSectionHolder holder(_buffer_mtx);
		EnsurePlayStateLocked(true);
	}

	void Reset() override
	{
		tTJSCriticalSectionHolder holder(_buffer_mtx);
		_playing = false;
		_bufferIdx = -1;
		_sentSamples = 0;
		_generation++;
		_lastAppendBytes = 0;
		memset(_bufferSize, 0, sizeof(tjs_uint) * _bufferCount);
		ClearPendingBufferLocked();
		_gainDirty = true;
		_playStateDirty = true;
		_state = tTVPAudioSinkState::Stopped;
		EnsurePlayStateLocked(true);
		LogSinkStateLocked("reset");
	}

	void Pause() override
	{
		tTJSCriticalSectionHolder holder(_buffer_mtx);
		_playing = false;
		_gainDirty = true;
		_playStateDirty = true;
		_state = tTVPAudioSinkState::Paused;
		EnsurePlayStateLocked(true);
		LogSinkStateLocked("pause");
	}

	void Play() override
	{
		tTJSCriticalSectionHolder holder(_buffer_mtx);
		_playing = true;
		TVPAudioDiagOpenALPlayCount++;
		_gainDirty = true;
		_positionDirty = true;
		_playStateDirty = true;
		_state = tTVPAudioSinkState::Playing;
		EnsurePlayStateLocked(true);
		LogSinkStateLocked("play");
	}

	void Stop() override
	{
		tTJSCriticalSectionHolder holder(_buffer_mtx);
		_playing = false;
		_bufferIdx = -1;
		_sentSamples = 0;
		_generation++;
		_lastAppendBytes = 0;
		memset(_bufferSize, 0, sizeof(tjs_uint) * _bufferCount);
		ClearPendingBufferLocked();
		_gainDirty = true;
		_playStateDirty = true;
		_state = tTVPAudioSinkState::Stopping;
		EnsurePlayStateLocked(true);
		_state = tTVPAudioSinkState::Stopped;
		LogSinkStateLocked("stop");
	}

	void SetVolume(float volume) override
	{
		if (_volume == volume)
			return;
		_volume = volume;
		_gainDirty = true;
		if (!TVPWasmOpenALFastAppendEnabled())
			EnsurePlayState();
	}

	float GetVolume() override
	{
		return _volume;
	}

	void SetPan(float pan) override
	{
		if (_sourcePos[0] == pan && _sourcePos[1] == 0.0f && _sourcePos[2] == 0.0f)
			return;
		_sourcePos[0] = pan;
		_sourcePos[1] = 0.0f;
		_sourcePos[2] = 0.0f;
		_positionDirty = true;
		if (!TVPWasmOpenALFastAppendEnabled())
			EnsurePlayState();
	}

	float GetPan() override
	{
		return _sourcePos[0];
	}

	bool IsPlaying() override
	{
		return _playing;
	}

	void SetPosition(float x, float y, float z) override
	{
		if (_sourcePos[0] == x && _sourcePos[1] == y && _sourcePos[2] == z)
			return;
		_sourcePos[0] = x;
		_sourcePos[1] = y;
		_sourcePos[2] = z;
		_positionDirty = true;
		if (!TVPWasmOpenALFastAppendEnabled())
			EnsurePlayState();
	}

	int GetRemainBuffers() override
	{
		tTJSCriticalSectionHolder holder(_buffer_mtx);
		DrainProcessedBuffersLocked();
		ALint processed = 0;
		ALint queued = 0;
		alGetSourcei(_alSource, AL_BUFFERS_PROCESSED, &processed);
		alGetSourcei(_alSource, AL_BUFFERS_QUEUED, &queued);
		FlushPendingBufferLocked(queued, nullptr, nullptr);
		return queued - processed;
	}

	tjs_uint GetLatencySamples() override
	{
		tTJSCriticalSectionHolder holder(_buffer_mtx);
		DrainProcessedBuffersLocked();
		ALint offset = 0;
		ALint queued = 0;
		alGetSourcei(_alSource, AL_BYTE_OFFSET, &offset);
		alGetSourcei(_alSource, AL_BUFFERS_QUEUED, &queued);
		int remainBuffers = queued;
		if (remainBuffers == 0)
		{
			return 0;
		}
		tjs_int total = -offset;
		for (int i = 0; i < remainBuffers; i += 1)
		{
			int idx = _bufferIdx + 1 - remainBuffers + i;
			if (idx >= _bufferCount)
			{
				idx -= _bufferCount;
			}
			else if (idx < 0)
			{
				idx += _bufferCount;
			}
			total += _bufferSize[idx];
		}
		return total / _frame_size;
	}

	virtual tjs_uint GetCurrentPlaySamples() override
	{
		tTJSCriticalSectionHolder holder(_buffer_mtx);
		DrainProcessedBuffersLocked();
		ALint offset = 0;
		alGetSourcei(_alSource, AL_SAMPLE_OFFSET, &offset);
		return _sentSamples + offset;
	}

	virtual int GetDebugSinkId() const override
	{
		return static_cast<int>(_alSource);
	}

	virtual const char *GetDebugBackendName() const override
	{
		return "openal";
	}

	virtual const char *GetDebugStateName() const override
	{
		return TVPAudioSinkStateName(_state);
	}

	virtual tjs_uint64 GetDebugGeneration() const override
	{
		return _generation;
	}

	virtual tjs_uint64 GetDebugPlayedSamples64() override
	{
		return GetCurrentPlaySamples();
	}

	static void CheckALError(const char *funcname);
};

class tTVPAudioRenderer
{
	ALCdevice *_device = nullptr;
	ALCcontext *_context = nullptr;
	std::unordered_set<tTVPSoundBuffer*> _streams;
public:
	virtual ~tTVPAudioRenderer()
	{
		if (_context)
		{
			alcMakeContextCurrent(nullptr);
			alcDestroyContext(_context);
		}
		if (_device)
		{
			alcCloseDevice(_device);
		}
	}
	bool Init()
	{
		_device = alcOpenDevice(nullptr);
		if (!_device)
		{
			return false;
		}

		_context = alcCreateContext(_device, nullptr);
		alcMakeContextCurrent(_context);

		return true;
	}

	virtual tTVPSoundBuffer* CreateStream(tTVPWaveFormat &fmt, int bufcount)
	{
		TVPAudioDiagOpenALCreateStreamCount++;
		tTVPSoundBuffer* s = new tTVPSoundBuffer(fmt, bufcount);
		_streams.emplace(s);
		return s;
	}

	ALCcontext *GetContext()
	{
		return _context;
	}
};

void tTVPSoundBuffer::CheckALError(const char *funcname)
{
	ALCcontext *ctx = ((tTVPAudioRenderer*)TVPAudioRenderer)->GetContext();
	if (alcGetCurrentContext() != ctx)
	{
		alcMakeContextCurrent(ctx);
	}
	ALenum err = alGetError();
	if (AL_NO_ERROR == err)
	{
		return;
	}
	TVPAddImportantLog(ttstr(funcname) + ttstr(": OpenAL Error ") + TJSInt32ToHex(err, 0));
}
#endif

#ifndef TVP_FAUDIO_IMPLEMENT
static tTVPAudioRenderer *CreateAudioRenderer()
{
	tTVPAudioRenderer *renderer = nullptr;
	renderer = new tTVPAudioRenderer;
	renderer->Init();
	return renderer;
}
#endif

void TVPInitDirectSound()
{
#ifndef TVP_FAUDIO_IMPLEMENT
#ifdef __EMSCRIPTEN__
	if (TVPWasmDirectWebAudioEnabled())
	{
		return;
	}
#endif
	if (!TVPAudioRenderer)
	{
		TVPAudioRenderer = CreateAudioRenderer();
	}
#endif
}

void TVPUninitDirectSound()
{
}

iTVPSoundBuffer* TVPCreateSoundBuffer(tTVPWaveFormat &fmt, int bufcount)
{
	iTVPSoundBuffer* stream = nullptr;
#ifdef __EMSCRIPTEN__
	if (TVPWasmDirectWebAudioEnabled())
	{
		tTVPWebAudioSoundBuffer *webAudioStream = new tTVPWebAudioSoundBuffer(fmt, bufcount);
		if (webAudioStream->IsAvailable())
		{
			if (TVPPerfEnabled())
			{
				fprintf(stderr,
					"[PERF] audio.backend selected=webaudio fallback=0 sample_rate=%d channels=%d bits=%d buffer_count=%d\n",
					fmt.SamplesPerSec,
					fmt.Channels,
					fmt.BitsPerSample,
					bufcount);
			}
			return webAudioStream;
		}
		delete webAudioStream;
		if (TVPPerfEnabled())
		{
			fprintf(stderr,
				"[PERF] audio.backend selected=openal fallback=1 reason=webaudio-unavailable sample_rate=%d channels=%d bits=%d buffer_count=%d\n",
				fmt.SamplesPerSec,
				fmt.Channels,
				fmt.BitsPerSample,
				bufcount);
		}
	}
	else if (TVPPerfEnabled())
	{
		fprintf(stderr,
			"[PERF] audio.backend selected=openal fallback=0 reason=disabled sample_rate=%d channels=%d bits=%d buffer_count=%d\n",
			fmt.SamplesPerSec,
			fmt.Channels,
			fmt.BitsPerSample,
			bufcount);
	}
#endif
#ifndef TVP_FAUDIO_IMPLEMENT
	if (!TVPAudioRenderer)
	{
		TVPAudioRenderer = CreateAudioRenderer();
	}
	stream = TVPAudioRenderer->CreateStream(fmt, bufcount);
#endif
	return stream;
}
