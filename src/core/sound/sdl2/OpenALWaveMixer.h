/* SPDX-License-Identifier: MIT */
/* Copyright (c) Kirikiri SDL2 Developers */

#pragma once
#include "WaveIntf.h"

#ifdef __EMSCRIPTEN__
#define TVPAL_BUFFER_COUNT 16
#else
#define TVPAL_BUFFER_COUNT 4
#endif

struct ALSoundImpl;
void TVPInitDirectSound();
void TVPUninitDirectSound();

class iTVPSoundBuffer {
public:
	virtual ~iTVPSoundBuffer() {}
	virtual void Release() = 0;
	virtual void Play() = 0;
	virtual void Pause() = 0;
	virtual void Stop() = 0;
	virtual void Reset() = 0;
	virtual bool IsPlaying() = 0;
	virtual void SetVolume(float v) = 0;
	virtual float GetVolume() = 0;
	virtual void SetPan(float v) = 0;
	virtual float GetPan() = 0;
	virtual void AppendBuffer(const void *buf, unsigned int len) = 0;
	virtual bool IsBufferValid() = 0;
	virtual tjs_uint GetCurrentPlaySamples() = 0;
	virtual tjs_uint GetLatencySamples() = 0;
	virtual int GetRemainBuffers() = 0;
	virtual void SetPosition(float x, float y, float z) {}
	virtual int GetDebugSinkId() const { return 0; }
	virtual const char *GetDebugBackendName() const { return "unknown"; }
	virtual const char *GetDebugStateName() const { return "unknown"; }
	virtual tjs_uint64 GetDebugGeneration() const { return 0; }
	virtual tjs_uint64 GetDebugPlayedSamples64() { return GetCurrentPlaySamples(); }
	virtual tjs_int GetDebugRingFillBytes() const { return -1; }
	virtual tjs_int GetDebugUnderrunCount() const { return -1; }
	virtual bool WantsStartPrefill() const { return false; }
};
iTVPSoundBuffer* TVPCreateSoundBuffer(tTVPWaveFormat &fmt, int bufcount);
