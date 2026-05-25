/* SPDX-License-Identifier: MIT */
/* Copyright (c) Kirikiri SDL2 Developers */

#include "tjsCommHead.h"
#include "LogFilter.h"
#include "WindowImpl.h"
#include "VirtualKey.h"
#include "Application.h"
#include "SystemImpl.h"
#include "TVPWindow.h"
#include "SysInitIntf.h"
#include "SysInitImpl.h"
#include "CharacterSet.h"
#include "WaveImpl.h"
#include "TimerThread.h"
#include "MsgIntf.h"
#include "DebugIntf.h"
#include "tjsArray.h"
#include "StorageIntf.h"
#include "SDLBitmapCompletion.h"
#include "ScriptMgnIntf.h"
#include "SystemControl.h"
#include "PluginImpl.h"
#ifdef KRKRZ_ENABLE_CANVAS
#include "OpenGLScreenSDL2.h"
#endif
#ifdef _WIN32
#include <SDL_syswm.h>
#endif
#include <SDL.h>
#ifdef _WIN32
#include <shellapi.h>
#include <stdlib.h>
#endif

#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#include <vector>
#endif

#ifdef __SWITCH__
#include <switch.h>
#endif

#ifdef __EMSCRIPTEN__
EM_JS_DEPS(main, "$FS,$IDBFS");
extern void TVPArmMotionClickBurstTrace(const char *reason, double x, double y);
extern void TVPArmMotionHostBurstTrace(const char *reason);
extern void TVPLogMotionHostBurstTrace(const char *tag, double a, double b, double c, double d);

static double krkr_perf_now_ms()
{
	return emscripten_get_now();
}

static bool krkr_emscripten_force_full_present()
{
	static int force_full_present = -1;
	if (force_full_present < 0) {
		force_full_present = MAIN_THREAD_EM_ASM_INT({
		try {
			var params = new URLSearchParams(location.search || "");
			var param = params.get("krkr_full_present");
			var moduleFlag = (typeof Module !== "undefined") &&
				(Module["krkr_full_present"] || Module["krkrForceFullPresent"]);
			var globalFlag = !!globalThis.__krkr_force_full_present;
			return (param === "1" || param === "true" || param === "yes" ||
				param === "on" || moduleFlag || globalFlag) ? 1 : 0;
		} catch (e) {
			return 0;
		}
		});
	}
	return force_full_present != 0;
}

static bool krkr_emscripten_multi_dirty_upload_enabled()
{
	static int multi_dirty_upload = -1;
	if (multi_dirty_upload < 0) {
		multi_dirty_upload = MAIN_THREAD_EM_ASM_INT({
		try {
			function disabled(value) {
				return value === 0 || value === false ||
					value === "0" || value === "false" ||
					value === "no" || value === "off";
			}
			var params = new URLSearchParams(location.search || "");
			var param = params.get("krkr_multi_dirty_upload");
			if (disabled(param)) return 0;
			var moduleValue;
			if (typeof Module !== "undefined") {
				moduleValue = Module["krkr_multi_dirty_upload"];
				if (moduleValue === undefined) moduleValue = Module["krkrMultiDirtyUpload"];
			}
			if (disabled(moduleValue)) return 0;
			if (disabled(globalThis.__krkr_multi_dirty_upload)) return 0;
			return 1;
		} catch (e) {
			return 1;
		}
		});
	}
	return multi_dirty_upload != 0;
}

static bool krkr_sdl_rects_touch_or_overlap(const SDL_Rect &a, const SDL_Rect &b)
{
	if (a.w <= 0 || a.h <= 0 || b.w <= 0 || b.h <= 0)
	{
		return false;
	}
	return !(a.x + a.w < b.x || b.x + b.w < a.x ||
		a.y + a.h < b.y || b.y + b.h < a.y);
}

static void krkr_sdl_add_dirty_upload_rect(std::vector<SDL_Rect> &rects,
	const SDL_Rect &source, int surface_w, int surface_h)
{
	int left = source.x < 0 ? 0 : source.x;
	int top = source.y < 0 ? 0 : source.y;
	int right = source.x + source.w;
	int bottom = source.y + source.h;
	if (right > surface_w) right = surface_w;
	if (bottom > surface_h) bottom = surface_h;
	SDL_Rect rect;
	rect.x = left;
	rect.y = top;
	rect.w = right - left;
	rect.h = bottom - top;
	if (rect.w <= 0 || rect.h <= 0)
	{
		return;
	}
	bool changed = true;
	while (changed)
	{
		changed = false;
		for (std::vector<SDL_Rect>::iterator it = rects.begin(); it != rects.end(); ++it)
		{
			if (!krkr_sdl_rects_touch_or_overlap(rect, *it))
			{
				continue;
			}
			int merged_left = rect.x < it->x ? rect.x : it->x;
			int merged_top = rect.y < it->y ? rect.y : it->y;
			int merged_right = rect.x + rect.w > it->x + it->w ? rect.x + rect.w : it->x + it->w;
			int merged_bottom = rect.y + rect.h > it->y + it->h ? rect.y + rect.h : it->y + it->h;
			rect.x = merged_left;
			rect.y = merged_top;
			rect.w = merged_right - merged_left;
			rect.h = merged_bottom - merged_top;
			rects.erase(it);
			changed = true;
			break;
		}
	}
	rects.push_back(rect);
}

static int krkr_emscripten_update_texture_dirty(SDL_Texture *texture, SDL_Surface *surface,
	TVPSDLBitmapCompletion *completion, const SDL_Rect &union_rect, bool force_full_present,
	int bytes_per_pixel, int *mode_out, int *reason_out, int *uploaded_rects_out,
	unsigned long long *dirty_area_out, unsigned long long *uploaded_bytes_out)
{
	const int max_upload_rects = 64;
	const unsigned long long area_fallback_percent = 70;
	int mode = 1;
	int reason = 0;
	int uploaded_rects = 0;
	unsigned long long dirty_area = 0;
	unsigned long long uploaded_bytes = 0;
	unsigned long long surface_area = surface
		? (unsigned long long)surface->w * (unsigned long long)surface->h
		: 0;
	auto finish = [&](int result) -> int {
		if (mode_out) *mode_out = mode;
		if (reason_out) *reason_out = reason;
		if (uploaded_rects_out) *uploaded_rects_out = uploaded_rects;
		if (dirty_area_out) *dirty_area_out = dirty_area;
		if (uploaded_bytes_out) *uploaded_bytes_out = uploaded_bytes;
		return result;
	};
	auto full_upload = [&](int full_mode, int full_reason, bool preserve_dirty_area) -> int {
		mode = full_mode;
		reason = full_reason;
		uploaded_rects = 1;
		if (!preserve_dirty_area || dirty_area == 0)
		{
			dirty_area = surface_area;
		}
		uploaded_bytes = surface
			? (unsigned long long)surface->pitch * (unsigned long long)surface->h
			: 0;
		return finish(SDL_UpdateTexture(texture, nullptr, surface->pixels, surface->pitch));
	};

	if (!texture || !surface || !surface->pixels || surface->w <= 0 || surface->h <= 0)
	{
		return finish(-1);
	}
	if (force_full_present)
	{
		return full_upload(3, 3, false);
	}

	std::vector<SDL_Rect> upload_rects;
	if (completion && !completion->update_rects.empty())
	{
		for (std::vector<tTVPRect>::const_iterator it = completion->update_rects.begin();
			it != completion->update_rects.end(); ++it)
		{
			SDL_Rect source;
			source.x = it->left;
			source.y = it->top;
			source.w = it->get_width();
			source.h = it->get_height();
			krkr_sdl_add_dirty_upload_rect(upload_rects, source, surface->w, surface->h);
		}
	}
	if (upload_rects.empty())
	{
		krkr_sdl_add_dirty_upload_rect(upload_rects, union_rect, surface->w, surface->h);
	}
	if (upload_rects.empty())
	{
		return finish(0);
	}

	if (!krkr_emscripten_multi_dirty_upload_enabled())
	{
		SDL_Rect single = union_rect;
		std::vector<SDL_Rect> single_rects;
		krkr_sdl_add_dirty_upload_rect(single_rects, single, surface->w, surface->h);
		if (single_rects.empty())
		{
			return finish(0);
		}
		single = single_rects[0];
		mode = 1;
		reason = 8;
		uploaded_rects = 1;
		dirty_area = (unsigned long long)single.w * (unsigned long long)single.h;
		uploaded_bytes = dirty_area * (unsigned long long)bytes_per_pixel;
		if (dirty_area >= surface_area)
		{
			return full_upload(3, 4, false);
		}
		const Uint8 *pixels = (const Uint8*)surface->pixels +
			single.y * surface->pitch + single.x * bytes_per_pixel;
		return finish(SDL_UpdateTexture(texture, &single, pixels, surface->pitch));
	}

	for (std::vector<SDL_Rect>::const_iterator it = upload_rects.begin();
		it != upload_rects.end(); ++it)
	{
		dirty_area += (unsigned long long)it->w * (unsigned long long)it->h;
	}

	if (upload_rects.size() > (size_t)max_upload_rects)
	{
		return full_upload(4, 6, true);
	}
	if (dirty_area >= surface_area)
	{
		int full_reason = completion && completion->update_rect_full_count > 0 ? 1 :
			((completion && completion->update_rect_union_full) ? 2 : 4);
		return full_upload(3, full_reason, false);
	}
	if (surface_area > 0 && dirty_area * 100ULL > surface_area * area_fallback_percent)
	{
		int full_reason = completion && completion->update_rect_full_count > 0 ? 1 :
			((completion && completion->update_rect_union_full) ? 2 : 5);
		int full_mode = full_reason == 5 ? 4 : 3;
		return full_upload(full_mode, full_reason, true);
	}

	mode = upload_rects.size() > 1 ? 2 : 1;
	reason = 0;
	uploaded_rects = (int)upload_rects.size();
	uploaded_bytes = dirty_area * (unsigned long long)bytes_per_pixel;
	for (std::vector<SDL_Rect>::const_iterator it = upload_rects.begin();
		it != upload_rects.end(); ++it)
	{
		const SDL_Rect &upload_rect = *it;
		const Uint8 *pixels = (const Uint8*)surface->pixels +
			upload_rect.y * surface->pitch + upload_rect.x * bytes_per_pixel;
		int update_result = SDL_UpdateTexture(texture, &upload_rect, pixels, surface->pitch);
		if (update_result != 0)
		{
			return full_upload(4, 7, true);
		}
	}
	return finish(0);
}

static void krkr_emscripten_get_canvas_box(double *paddingLeft, double *paddingTop,
	double *paddingRight, double *paddingBottom, double *width, double *height,
	double *canvasWidth, double *canvasHeight)
{
	MAIN_THREAD_EM_ASM({
		var paddingLeftOut = $0;
		var paddingTopOut = $1;
		var paddingRightOut = $2;
		var paddingBottomOut = $3;
		var widthOut = $4;
		var heightOut = $5;
		var canvasWidthOut = $6;
		var canvasHeightOut = $7;
		var canvas = Module["canvas"] || document.getElementById("canvas");
		if (!canvas) {
			HEAPF64[paddingLeftOut >> 3] = 0;
			HEAPF64[paddingTopOut >> 3] = 0;
			HEAPF64[paddingRightOut >> 3] = 0;
			HEAPF64[paddingBottomOut >> 3] = 0;
			HEAPF64[widthOut >> 3] = 0;
			HEAPF64[heightOut >> 3] = 0;
			HEAPF64[canvasWidthOut >> 3] = 0;
			HEAPF64[canvasHeightOut >> 3] = 0;
			return;
		}
		var rect = canvas.getBoundingClientRect();
		var style = getComputedStyle(canvas);
		HEAPF64[paddingLeftOut >> 3] = parseFloat(style.paddingLeft) || 0;
		HEAPF64[paddingTopOut >> 3] = parseFloat(style.paddingTop) || 0;
		HEAPF64[paddingRightOut >> 3] = parseFloat(style.paddingRight) || 0;
		HEAPF64[paddingBottomOut >> 3] = parseFloat(style.paddingBottom) || 0;
		HEAPF64[widthOut >> 3] = Math.max(1, rect.width);
		HEAPF64[heightOut >> 3] = Math.max(1, rect.height);
		HEAPF64[canvasWidthOut >> 3] = Math.max(1, canvas.width || 0);
		HEAPF64[canvasHeightOut >> 3] = Math.max(1, canvas.height || 0);
	}, paddingLeft, paddingTop, paddingRight, paddingBottom, width, height,
		canvasWidth, canvasHeight);
}

static void krkr_emscripten_record_mouse_input(const char *phase,
	int rawX, int rawY, int drawX, int drawY, int button, int clicks, tjs_uint32 shift)
{
	MAIN_THREAD_EM_ASM({
		var phase = UTF8ToString($0);
		var item = {};
		item.time = Date.now();
		item.phase = phase;
		item.rawX = $1;
		item.rawY = $2;
		item.drawX = $3;
		item.drawY = $4;
		item.button = $5;
		item.clicks = $6;
		item.shift = $7;
		globalThis.__krkr_last_sdl_input = item;
		if (!globalThis.__krkr_sdl_inputs) globalThis.__krkr_sdl_inputs = [];
		globalThis.__krkr_sdl_inputs.push(globalThis.__krkr_last_sdl_input);
		if (globalThis.__krkr_sdl_inputs.length > 64) globalThis.__krkr_sdl_inputs.shift();
	}, phase, rawX, rawY, drawX, drawY, button, clicks, shift);
}

static Uint32 krkr_read_surface_pixel(const Uint8 *src, int bytes_per_pixel) {
	switch (bytes_per_pixel) {
	case 1:
		return *src;
	case 2:
		return *(const Uint16*)src;
	case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		return (src[0] << 16) | (src[1] << 8) | src[2];
#else
		return src[0] | (src[1] << 8) | (src[2] << 16);
#endif
	case 4:
		return *(const Uint32*)src;
	default:
		return 0;
	}
}

static void krkr_emscripten_put_surface(SDL_Surface *surface) {
	if (!surface || !surface->pixels) {
		fprintf(stderr, "[PUT_SURFACE] SKIP: surface=%p pixels=%p\n",
			(void*)surface, surface ? surface->pixels : nullptr);
		return;
	}
	int w = surface->w;
	int h = surface->h;
	SDL_PixelFormat *format = surface->format;
	int bytes_per_pixel = format ? format->BytesPerPixel : 4;
	bool perf_enabled = TVPPerfEnabled();
	double perf_start_ms = perf_enabled ? krkr_perf_now_ms() : 0.0;

	static void* rgba_buffer = nullptr;
	static int rgba_buffer_size = 0;
	int byte_count = w * h * 4;
	if (rgba_buffer_size < byte_count) {
		if (rgba_buffer) free(rgba_buffer);
		rgba_buffer = malloc(byte_count);
		rgba_buffer_size = byte_count;
	}
	if (!rgba_buffer) {
		fprintf(stderr, "[PUT_SURFACE] SKIP: rgba buffer alloc failed size=%d\n", byte_count);
		return;
	}

	bool locked = false;
	if (SDL_MUSTLOCK(surface)) {
		if (SDL_LockSurface(surface) != 0) {
			fprintf(stderr, "[PUT_SURFACE] SKIP: SDL_LockSurface failed: %s\n", SDL_GetError());
			return;
		}
		locked = true;
	}

	const Uint8 *src_pixels = (const Uint8*)surface->pixels;
	Uint8 *dst_pixels = (Uint8*)rgba_buffer;
	bool direct_rgba_copy =
		bytes_per_pixel == 4 &&
		surface->pitch == w * 4 &&
		format &&
		format->Rmask == 0x000000ff &&
		format->Gmask == 0x0000ff00 &&
		format->Bmask == 0x00ff0000 &&
		format->Amask == 0xff000000;
	bool direct_xrgb_copy =
		bytes_per_pixel == 4 &&
		surface->pitch == w * 4 &&
		format &&
		format->Rmask == 0x00ff0000 &&
		format->Gmask == 0x0000ff00 &&
		format->Bmask == 0x000000ff &&
		format->Amask == 0x00000000;
	bool copied_direct = false;
	if (direct_rgba_copy)
	{
		SDL_memcpy(dst_pixels, src_pixels, byte_count);
		copied_direct = true;
	}
	else if (direct_xrgb_copy)
	{
		for (int y = 0; y < h; y++) {
			const Uint8 *src_row = src_pixels + y * surface->pitch;
			Uint8 *dst_row = dst_pixels + y * w * 4;
			for (int x = 0; x < w; x++) {
				Uint32 pixel = *(const Uint32*)(src_row + x * 4);
				Uint8 *dp = dst_row + x * 4;
				dp[0] = (pixel >> 16) & 0xff;
				dp[1] = (pixel >> 8) & 0xff;
				dp[2] = pixel & 0xff;
				dp[3] = 255;
			}
		}
		copied_direct = true;
	}
	if (!copied_direct)
	{
	for (int y = 0; y < h; y++) {
		const Uint8 *src_row = src_pixels + y * surface->pitch;
		Uint8 *dst_row = dst_pixels + y * w * 4;
		for (int x = 0; x < w; x++) {
			Uint32 pixel = krkr_read_surface_pixel(src_row + x * bytes_per_pixel, bytes_per_pixel);
			Uint8 r = 0, g = 0, b = 0, a = 255;
			if (format) {
				SDL_GetRGBA(pixel, format, &r, &g, &b, &a);
				if (format->Amask == 0) a = 255;
			} else {
				r = (pixel >> 16) & 0xff;
				g = (pixel >> 8) & 0xff;
				b = pixel & 0xff;
				a = 255;
			}
			Uint8 *dp = dst_row + x * 4;
			dp[0] = r;
			dp[1] = g;
			dp[2] = b;
			dp[3] = a;
		}
	}
	}
	double perf_after_convert_ms = perf_enabled ? krkr_perf_now_ms() : 0.0;

	{
		static int call_count = 0;
		call_count++;
		if (TVPLogL3() && (call_count <= 5 || call_count % 100 == 0)) {
			tjs_uint32 *px = (tjs_uint32*)surface->pixels;
			const Uint8 *rgba = (const Uint8*)rgba_buffer;
			int total = w * h;
			int nonzero = 0;
			int sample = total < 500 ? total : 500;
			for (int i = 0; i < sample && sample > 0; i++) {
				if (px[i * (total / sample)] != 0) nonzero++;
			}
			fprintf(stderr, "[PUT_SURFACE] #%d: %dx%d pixels=%p pitch=%d fmt=%s masks=[%08x,%08x,%08x,%08x] nonzero=%d/%d first4=[%08x,%08x,%08x,%08x] rgba0=[%02x,%02x,%02x,%02x]\n",
				call_count, w, h, surface->pixels, surface->pitch,
				format ? SDL_GetPixelFormatName(format->format) : "(null)",
				format ? format->Rmask : 0, format ? format->Gmask : 0,
				format ? format->Bmask : 0, format ? format->Amask : 0,
				nonzero, sample,
				total > 0 ? px[0] : 0, total > 1 ? px[1] : 0,
				total > 2 ? px[2] : 0, total > 3 ? px[3] : 0,
				byte_count >= 4 ? rgba[0] : 0, byte_count >= 4 ? rgba[1] : 0,
				byte_count >= 4 ? rgba[2] : 0, byte_count >= 4 ? rgba[3] : 0);
		}
	}
	if (locked) SDL_UnlockSurface(surface);

	MAIN_THREAD_EM_ASM({
		var w = $0; var h = $1; var pixels = $2;
		var bytesPerPixel = $3;
		var rMask = $4 >>> 0;
		var gMask = $5 >>> 0;
		var bMask = $6 >>> 0;
		var aMask = $7 >>> 0;
		var pitch = $8;
		var sourceFormat = $9 >>> 0;
		var perfEnabled = !!$10;
		var convertMs = +$11;
		var copiedDirect = !!$12;
		if (!Module["SDL2"]) Module["SDL2"] = {};
		var SDL2 = Module["SDL2"];
		var canvas = Module["canvas"];
		if (!canvas) return;
		var now = (typeof performance != "undefined" && performance.now)
			? function() { return performance.now(); }
			: function() { return Date.now(); };
		if (canvas.width !== w || canvas.height !== h) {
			console.log("[PUT_SURFACE JS] resizing canvas from " + canvas.width + "x" + canvas.height + " to " + w + "x" + h);
			canvas.width = w;
			canvas.height = h;
		}
		if (SDL2.ctxCanvas !== canvas) {
			SDL2.ctx = canvas.getContext("2d");
			SDL2.ctxCanvas = canvas;
		}
		if (!SDL2.ctx) return;
		if (SDL2.w !== w || SDL2.h !== h || SDL2.imageCtx !== SDL2.ctx) {
			SDL2.image = SDL2.ctx.createImageData(w, h);
			SDL2.w = w;
			SDL2.h = h;
			SDL2.imageCtx = SDL2.ctx;
		}
		var byteCount = (w * h * 4) | 0;
		var setStart = perfEnabled ? now() : 0;
		SDL2.image.data.set(HEAPU8.subarray(pixels, pixels + byteCount));
		var afterSet = perfEnabled ? now() : 0;
		SDL2.ctx.putImageData(SDL2.image, 0, 0);
		var afterPut = perfEnabled ? now() : 0;
		var r = globalThis.__krkr_render || {};
		r.w = w; r.h = h; r.pixels = pixels;
		r.pixelFormat = "CanvasRGBA";
		r.surface = r.surface || {};
		r.surface.sourceFormat = sourceFormat;
		r.surface.bytesPerPixel = bytesPerPixel;
		r.surface.pitch = pitch;
		r.surface.rMask = rMask;
		r.surface.gMask = gMask;
		r.surface.bMask = bMask;
		r.surface.aMask = aMask;
		r.frameCount = (r.frameCount || 0) + 1;
		var wallNow = Date.now();
		var prevFrame = r.lastFrame || 0;
		r.lastFrame = wallNow;
		var intervalMs = prevFrame ? wallNow - prevFrame : 0;
		if (perfEnabled) {
			var perf = r.perf || {};
			perf.canvas = perf.canvas || {};
			var canvasPerf = perf.canvas;
			canvasPerf.frameCount = (canvasPerf.frameCount || 0) + 1;
			canvasPerf.totalBytes = (canvasPerf.totalBytes || 0) + byteCount;
			canvasPerf.lastBytes = byteCount;
			canvasPerf.lastConvertMs = convertMs;
			canvasPerf.lastSetMs = afterSet - setStart;
			canvasPerf.lastPutMs = afterPut - afterSet;
			canvasPerf.lastTotalJsMs = afterPut - setStart;
			canvasPerf.lastIntervalMs = intervalMs;
			canvasPerf.lastFullFrame = 1;
			canvasPerf.lastCopiedDirect = copiedDirect ? 1 : 0;
			canvasPerf.maxSetMs = Math.max(canvasPerf.maxSetMs || 0, canvasPerf.lastSetMs);
			canvasPerf.maxPutMs = Math.max(canvasPerf.maxPutMs || 0, canvasPerf.lastPutMs);
			canvasPerf.maxTotalJsMs = Math.max(canvasPerf.maxTotalJsMs || 0, canvasPerf.lastTotalJsMs);
			canvasPerf.maxConvertMs = Math.max(canvasPerf.maxConvertMs || 0, convertMs);
			canvasPerf.maxIntervalMs = Math.max(canvasPerf.maxIntervalMs || 0, intervalMs);
			if (intervalMs > 33.4) canvasPerf.longFrameCount = (canvasPerf.longFrameCount || 0) + 1;
			if (intervalMs > 50) canvasPerf.droppedFrameCount = (canvasPerf.droppedFrameCount || 0) + 1;
			canvasPerf.samples = canvasPerf.samples || [];
			canvasPerf.samples.push({
				t: wallNow,
				frame: r.frameCount,
				bytes: byteCount,
				convertMs: convertMs,
				setMs: canvasPerf.lastSetMs,
				putMs: canvasPerf.lastPutMs,
				totalJsMs: canvasPerf.lastTotalJsMs,
				intervalMs: intervalMs,
				fullFrame: 1,
				copiedDirect: copiedDirect ? 1 : 0
			});
			if (canvasPerf.samples.length > 240) canvasPerf.samples.shift();
			perf.canvas = canvasPerf;
			r.perf = perf;
			if (canvasPerf.frameCount <= 10 || canvasPerf.frameCount % 60 === 0 ||
				canvasPerf.lastTotalJsMs >= 8 || convertMs >= 8 || intervalMs >= 50) {
				console.log("[PERF] present.canvas frame=" + r.frameCount +
					" size=" + w + "x" + h +
					" bytes=" + byteCount +
					" full=1" +
					" copied_direct=" + (copiedDirect ? 1 : 0) +
					" convert_ms=" + convertMs.toFixed(3) +
					" image_set_ms=" + canvasPerf.lastSetMs.toFixed(3) +
					" put_ms=" + canvasPerf.lastPutMs.toFixed(3) +
					" js_ms=" + canvasPerf.lastTotalJsMs.toFixed(3) +
					" interval_ms=" + intervalMs.toFixed(1));
			}
		}
		globalThis.__krkr_render = r;
	}, w, h, rgba_buffer, bytes_per_pixel,
		format ? format->Rmask : 0,
		format ? format->Gmask : 0,
		format ? format->Bmask : 0,
		format ? format->Amask : 0,
		surface->pitch,
		format ? format->format : 0,
		perf_enabled ? 1 : 0,
		perf_enabled ? (perf_after_convert_ms - perf_start_ms) : 0.0,
		copied_direct ? 1 : 0);
}

static const char *krkr_sdl_present_full_reason_name(int full_reason)
{
	switch (full_reason)
	{
	case 1: return "dirty-full";
	case 2: return "dirty-union-full";
	case 3: return "renderer-full";
	case 4: return "rect-full";
	case 5: return "dirty-area";
	case 6: return "rect-limit";
	case 7: return "update-failed";
	case 8: return "multi-disabled";
	default: return "partial";
	}
}

static const char *krkr_sdl_present_mode_name(int mode)
{
	switch (mode)
	{
	case 1: return "single-rect";
	case 2: return "multi-rect";
	case 3: return "full";
	case 4: return "fallback";
	default: return "unknown";
	}
}

static void krkr_emscripten_record_sdl_present(SDL_Surface *surface, const SDL_Rect *rect,
	int full_frame, unsigned long long updated_bytes, int full_reason,
	int dirty_rect_count, int dirty_full_count, int dirty_empty_count, int dirty_union_full,
	int mode, int uploaded_rect_count, unsigned long long dirty_area, unsigned long long surface_area,
	double update_ms, double copy_ms, double present_ms, double total_ms)
{
	int w = surface ? surface->w : 0;
	int h = surface ? surface->h : 0;
	int rx = rect ? rect->x : 0;
	int ry = rect ? rect->y : 0;
	int rw = rect ? rect->w : w;
	int rh = rect ? rect->h : h;
	int bpp = surface && surface->format ? surface->format->BytesPerPixel : 0;
	int pitch = surface ? surface->pitch : 0;
	bool perf_enabled = TVPPerfEnabled();
	double args[20];
	args[0] = (double)w;
	args[1] = (double)h;
	args[2] = (double)rx;
	args[3] = (double)ry;
	args[4] = (double)rw;
	args[5] = (double)rh;
	args[6] = full_frame ? 1.0 : 0.0;
	args[7] = (double)updated_bytes;
	args[8] = (double)bpp;
	args[9] = (double)pitch;
	args[10] = perf_enabled ? 1.0 : 0.0;
	args[11] = (double)full_reason;
	args[12] = update_ms;
	args[13] = copy_ms;
	args[14] = present_ms;
	args[15] = total_ms;
	args[16] = (double)mode;
	args[17] = (double)uploaded_rect_count;
	args[18] = (double)dirty_area;
	args[19] = (double)surface_area;
	MAIN_THREAD_EM_ASM({
		var a = $0 >> 3;
		var w = HEAPF64[a + 0] | 0;
		var h = HEAPF64[a + 1] | 0;
		var rx = HEAPF64[a + 2] | 0;
		var ry = HEAPF64[a + 3] | 0;
		var rw = HEAPF64[a + 4] | 0;
		var rh = HEAPF64[a + 5] | 0;
		var fullFrame = HEAPF64[a + 6] | 0;
		var updatedBytes = Number(HEAPF64[a + 7]);
		var bpp = HEAPF64[a + 8] | 0;
		var pitch = HEAPF64[a + 9] | 0;
		var perfEnabled = !!(HEAPF64[a + 10] | 0);
		var reasonCode = HEAPF64[a + 11] | 0;
		var updateMs = +HEAPF64[a + 12];
		var copyMs = +HEAPF64[a + 13];
		var presentMs = +HEAPF64[a + 14];
		var totalMs = +HEAPF64[a + 15];
		var modeCode = HEAPF64[a + 16] | 0;
		var uploadedRectCount = HEAPF64[a + 17] | 0;
		var dirtyArea = Number(HEAPF64[a + 18]);
		var surfaceArea = Number(HEAPF64[a + 19]);
		var fullReason = reasonCode === 1 ? "dirty-full" :
			(reasonCode === 2 ? "dirty-union-full" :
			(reasonCode === 3 ? "renderer-full" :
			(reasonCode === 4 ? "rect-full" :
			(reasonCode === 5 ? "dirty-area" :
			(reasonCode === 6 ? "rect-limit" :
			(reasonCode === 7 ? "update-failed" :
			(reasonCode === 8 ? "multi-disabled" : "partial")))))));
		var mode = modeCode === 1 ? "single-rect" :
			(modeCode === 2 ? "multi-rect" :
			(modeCode === 3 ? "full" :
			(modeCode === 4 ? "fallback" : "unknown")));
		var r = globalThis.__krkr_render || {};
		r.w = w;
		r.h = h;
		r.pixelFormat = "SDL_Renderer";
		r.surface = r.surface || {};
		r.surface.bytesPerPixel = bpp;
		r.surface.pitch = pitch;
		r.surface.updateRect = ({ x: rx, y: ry, w: rw, h: rh });
		r.surface.fullReason = fullReason;
		r.surface.presentMode = mode;
		r.surface.uploadedRectCount = uploadedRectCount;
		r.surface.dirtyArea = dirtyArea;
		r.surface.surfaceArea = surfaceArea;
		r.frameCount = (r.frameCount || 0) + 1;
		var wallNow = Date.now();
		var prevFrame = r.lastFrame || 0;
		r.lastFrame = wallNow;
		var intervalMs = prevFrame ? wallNow - prevFrame : 0;
		if (perfEnabled) {
			var perf = r.perf || {};
			perf.sdl = perf.sdl || {};
			var sdlPerf = perf.sdl;
			sdlPerf.frameCount = (sdlPerf.frameCount || 0) + 1;
			sdlPerf.totalBytes = (sdlPerf.totalBytes || 0) + updatedBytes;
			sdlPerf.lastBytes = updatedBytes;
			sdlPerf.lastUpdateMs = updateMs;
			sdlPerf.lastCopyMs = copyMs;
			sdlPerf.lastPresentMs = presentMs;
			sdlPerf.lastTotalMs = totalMs;
			sdlPerf.lastIntervalMs = intervalMs;
			sdlPerf.lastFullFrame = fullFrame;
			sdlPerf.lastFullReason = fullReason;
			sdlPerf.lastMode = mode;
			sdlPerf.lastRectCount = uploadedRectCount;
			sdlPerf.lastDirtyArea = dirtyArea;
			sdlPerf.lastSurfaceArea = surfaceArea;
			if (fullFrame) {
				sdlPerf.fullFrameCount = (sdlPerf.fullFrameCount || 0) + 1;
				sdlPerf.fullReasons = sdlPerf.fullReasons || {};
				sdlPerf.fullReasons[fullReason || "unknown"] =
					(sdlPerf.fullReasons[fullReason || "unknown"] || 0) + 1;
			}
			sdlPerf.maxUpdateMs = Math.max(sdlPerf.maxUpdateMs || 0, updateMs);
			sdlPerf.maxCopyMs = Math.max(sdlPerf.maxCopyMs || 0, copyMs);
			sdlPerf.maxPresentMs = Math.max(sdlPerf.maxPresentMs || 0, presentMs);
			sdlPerf.maxTotalMs = Math.max(sdlPerf.maxTotalMs || 0, totalMs);
			sdlPerf.maxIntervalMs = Math.max(sdlPerf.maxIntervalMs || 0, intervalMs);
			if (intervalMs > 33.4) sdlPerf.longFrameCount = (sdlPerf.longFrameCount || 0) + 1;
			if (intervalMs > 50) sdlPerf.droppedFrameCount = (sdlPerf.droppedFrameCount || 0) + 1;
			sdlPerf.samples = sdlPerf.samples || [];
			sdlPerf.samples.push({
				t: wallNow,
				frame: r.frameCount,
				bytes: updatedBytes,
				updateMs: updateMs,
				copyMs: copyMs,
				presentMs: presentMs,
				totalMs: totalMs,
				intervalMs: intervalMs,
				fullFrame: fullFrame,
				fullReason: fullReason,
				mode: mode,
				rects: uploadedRectCount,
				dirtyArea: dirtyArea,
				surfaceArea: surfaceArea,
				rect: { x: rx, y: ry, w: rw, h: rh }
			});
			if (sdlPerf.samples.length > 240) sdlPerf.samples.shift();
			perf.sdl = sdlPerf;
			r.perf = perf;
		}
		globalThis.__krkr_render = r;
	}, args);
}

#endif

#if defined(__IPHONEOS__) || defined(__ANDROID__) || defined(__EMSCRIPTEN__) || defined(__vita__) || defined(__SWITCH__)
#define KRKRSDL2_WINDOW_SIZE_IS_LAYER_SIZE
#endif

#if defined(__linux__) && !defined(__EMSCRIPTEN__)
// By specification of SDL_RenderPresent, the backbuffer should be
// considered invalidated after each call. This is required for
// some renderers to be enabled.
#define KRKRSDL2_RENDERER_FULL_UPDATES
#endif

extern void TVPLoadMessage();

class TVPWindowWindow;
static TVPWindowWindow *_lastWindowWindow, *_currentWindowWindow;
static SDL_GameController **sdl_controllers = nullptr;
static int sdl_controller_num = 0;

#ifdef __EMSCRIPTEN__
static void process_events();
#else
static bool process_events();
#endif

#ifdef __EMSCRIPTEN__
static int sdl_event_watch(void *userdata, SDL_Event *in_event);
#endif

static void refresh_controllers()
{
#if defined(__IPHONEOS__) || defined(__ANDROID__)
	// TODO: check why invalid pointers get set in SDL's controller subsystem which causes segfault
	{
		return;
	}
#endif
	if (!SDL_WasInit(SDL_INIT_GAMECONTROLLER))
	{
		SDL_Init(SDL_INIT_GAMECONTROLLER);
	}
	if (sdl_controller_num && sdl_controllers)
	{
		for (int i = 0; i < sdl_controller_num; i += 1)
		{
			if (sdl_controllers[i])
			{
				SDL_GameControllerClose(sdl_controllers[i]);
				sdl_controllers[i] = nullptr;
			}
		}
		sdl_controller_num = 0;
		SDL_free(sdl_controllers);
		sdl_controllers = nullptr;
	}
	sdl_controller_num = SDL_NumJoysticks();
	if (sdl_controller_num)
	{
		sdl_controllers = (SDL_GameController**)SDL_malloc(sizeof(SDL_GameController*) * sdl_controller_num);
		if (!sdl_controllers)
		{
			sdl_controller_num = 0;
			TVPAddLog(ttstr("Could not allocate memory to store SDL controller information"));
			return;
		}
		for (int i = 0; i < sdl_controller_num; i += 1)
		{
			if (SDL_IsGameController(i))
			{
				sdl_controllers[i] = SDL_GameControllerOpen(i);
				if (!sdl_controllers[i])
				{
					TVPAddLog(ttstr("Could not open controller: ") + ttstr(SDL_GetError()));
				}
			}
		}
	}
}

static Uint8 vk_key_to_sdl_gamecontrollerbutton(tjs_uint key)
{
	switch (key)
	{
		case VK_PAD1: return SDL_CONTROLLER_BUTTON_A;
		case VK_PAD2: return SDL_CONTROLLER_BUTTON_B;
		case VK_PAD3: return SDL_CONTROLLER_BUTTON_X;
		case VK_PAD4: return SDL_CONTROLLER_BUTTON_Y;
		case VK_PAD7: return SDL_CONTROLLER_BUTTON_BACK;
		case VK_PAD8: return SDL_CONTROLLER_BUTTON_START;
		case VK_PAD9: return SDL_CONTROLLER_BUTTON_LEFTSTICK;
		case VK_PAD10: return SDL_CONTROLLER_BUTTON_RIGHTSTICK;
		case VK_PAD5: return SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
		case VK_PAD6: return SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
		case VK_PADUP: return SDL_CONTROLLER_BUTTON_DPAD_UP;
		case VK_PADDOWN: return SDL_CONTROLLER_BUTTON_DPAD_DOWN;
		case VK_PADLEFT: return SDL_CONTROLLER_BUTTON_DPAD_LEFT;
		case VK_PADRIGHT: return SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
		default: return 0;
	}
}

static tjs_uint sdl_gamecontrollerbutton_to_vk_key(Uint8 key)
{
	switch (key)
	{
		case SDL_CONTROLLER_BUTTON_A: return VK_PAD1;
		case SDL_CONTROLLER_BUTTON_B: return VK_PAD2;
		case SDL_CONTROLLER_BUTTON_X: return VK_PAD3;
		case SDL_CONTROLLER_BUTTON_Y: return VK_PAD4;
		case SDL_CONTROLLER_BUTTON_BACK: return VK_PAD7;
		case SDL_CONTROLLER_BUTTON_START: return VK_PAD8;
		case SDL_CONTROLLER_BUTTON_LEFTSTICK: return VK_PAD9;
		case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return VK_PAD10;
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return VK_PAD5;
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return VK_PAD6;
		case SDL_CONTROLLER_BUTTON_DPAD_UP: return VK_PADUP;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return VK_PADDOWN;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return VK_PADLEFT;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return VK_PADRIGHT;
		default: return 0;
	}
}

#ifdef MK_SHIFT
#undef MK_SHIFT
#endif
#ifdef MK_CONTROL
#undef MK_CONTROL
#endif
#ifdef MK_ALT
#undef MK_ALT
#endif
#define MK_SHIFT 4
#define MK_CONTROL 8
#define MK_ALT (0x20)

static SDL_Cursor *sdl_system_cursors[SDL_NUM_SYSTEM_CURSORS] = {0};

static SDL_Keycode vk_key_to_sdl_key(tjs_uint key)
{
	// This is generated using regex find replace
	switch (key)
	{
		case VK_BACK: return SDLK_BACKSPACE;
		case VK_TAB: return SDLK_TAB;
		case VK_CLEAR: return SDLK_CLEAR;
		case VK_RETURN: return SDLK_RETURN;
		case VK_PAUSE: return SDLK_PAUSE;
		case VK_CAPITAL: return SDLK_CAPSLOCK;
		case VK_ESCAPE: return SDLK_ESCAPE;
		case VK_SPACE: return SDLK_SPACE;
		case VK_PRIOR: return SDLK_PAGEUP;
		case VK_NEXT: return SDLK_PAGEDOWN;
		case VK_END: return SDLK_END;
		case VK_HOME: return SDLK_HOME;
		case VK_LEFT: return SDLK_LEFT;
		case VK_UP: return SDLK_UP;
		case VK_RIGHT: return SDLK_RIGHT;
		case VK_DOWN: return SDLK_DOWN;
		case VK_SELECT: return SDLK_SELECT;
		case VK_EXECUTE: return SDLK_EXECUTE;
		case VK_SNAPSHOT: return SDLK_PRINTSCREEN;
		case VK_INSERT: return SDLK_INSERT;
		case VK_DELETE: return SDLK_DELETE;
		case VK_HELP: return SDLK_HELP;
		case VK_0: return SDLK_0;
		case VK_1: return SDLK_1;
		case VK_2: return SDLK_2;
		case VK_3: return SDLK_3;
		case VK_4: return SDLK_4;
		case VK_5: return SDLK_5;
		case VK_6: return SDLK_6;
		case VK_7: return SDLK_7;
		case VK_8: return SDLK_8;
		case VK_9: return SDLK_9;
		case VK_A: return SDLK_a;
		case VK_B: return SDLK_b;
		case VK_C: return SDLK_c;
		case VK_D: return SDLK_d;
		case VK_E: return SDLK_e;
		case VK_F: return SDLK_f;
		case VK_G: return SDLK_g;
		case VK_H: return SDLK_h;
		case VK_I: return SDLK_i;
		case VK_J: return SDLK_j;
		case VK_K: return SDLK_k;
		case VK_L: return SDLK_l;
		case VK_M: return SDLK_m;
		case VK_N: return SDLK_n;
		case VK_O: return SDLK_o;
		case VK_P: return SDLK_p;
		case VK_Q: return SDLK_q;
		case VK_R: return SDLK_r;
		case VK_S: return SDLK_s;
		case VK_T: return SDLK_t;
		case VK_U: return SDLK_u;
		case VK_V: return SDLK_v;
		case VK_W: return SDLK_w;
		case VK_X: return SDLK_x;
		case VK_Y: return SDLK_y;
		case VK_Z: return SDLK_z;
		case VK_LWIN: return SDLK_LGUI;
		case VK_RWIN: return SDLK_RGUI;
		case VK_SLEEP: return SDLK_SLEEP;
		case VK_NUMPAD0: return SDLK_KP_0;
		case VK_NUMPAD1: return SDLK_KP_1;
		case VK_NUMPAD2: return SDLK_KP_2;
		case VK_NUMPAD3: return SDLK_KP_3;
		case VK_NUMPAD4: return SDLK_KP_4;
		case VK_NUMPAD5: return SDLK_KP_5;
		case VK_NUMPAD6: return SDLK_KP_6;
		case VK_NUMPAD7: return SDLK_KP_7;
		case VK_NUMPAD8: return SDLK_KP_8;
		case VK_NUMPAD9: return SDLK_KP_9;
		case VK_MULTIPLY: return SDLK_KP_MULTIPLY;
		case VK_ADD: return SDLK_KP_PLUS;
		case VK_SUBTRACT: return SDLK_KP_MINUS;
		case VK_DECIMAL: return SDLK_KP_PERIOD;
		case VK_DIVIDE: return SDLK_KP_DIVIDE;
		case VK_F1: return SDLK_F1;
		case VK_F2: return SDLK_F2;
		case VK_F3: return SDLK_F3;
		case VK_F4: return SDLK_F4;
		case VK_F5: return SDLK_F5;
		case VK_F6: return SDLK_F6;
		case VK_F7: return SDLK_F7;
		case VK_F8: return SDLK_F8;
		case VK_F9: return SDLK_F9;
		case VK_F10: return SDLK_F10;
		case VK_F11: return SDLK_F11;
		case VK_F12: return SDLK_F12;
		case VK_F13: return SDLK_F13;
		case VK_F14: return SDLK_F14;
		case VK_F15: return SDLK_F15;
		case VK_F16: return SDLK_F16;
		case VK_F17: return SDLK_F17;
		case VK_F18: return SDLK_F18;
		case VK_F19: return SDLK_F19;
		case VK_F20: return SDLK_F20;
		case VK_F21: return SDLK_F21;
		case VK_F22: return SDLK_F22;
		case VK_F23: return SDLK_F23;
		case VK_F24: return SDLK_F24;
		case VK_NUMLOCK: return SDLK_NUMLOCKCLEAR;
		case VK_SCROLL: return SDLK_SCROLLLOCK;
		case VK_LSHIFT: return SDLK_LSHIFT;
		case VK_RSHIFT: return SDLK_RSHIFT;
		case VK_LCONTROL: return SDLK_LCTRL;
		case VK_RCONTROL: return SDLK_RCTRL;
		case VK_LMENU: return SDLK_LALT;
		case VK_RMENU: return SDLK_RALT;
		case VK_BROWSER_BACK: return SDLK_AC_BACK;
		case VK_BROWSER_FORWARD: return SDLK_AC_FORWARD;
		case VK_BROWSER_REFRESH: return SDLK_AC_REFRESH;
		case VK_BROWSER_STOP: return SDLK_AC_STOP;
		case VK_BROWSER_SEARCH: return SDLK_AC_SEARCH;
		case VK_BROWSER_FAVORITES: return SDLK_AC_BOOKMARKS;
		case VK_BROWSER_HOME: return SDLK_AC_HOME;
		case VK_VOLUME_MUTE: return SDLK_MUTE;
		case VK_VOLUME_DOWN: return SDLK_VOLUMEDOWN;
		case VK_VOLUME_UP: return SDLK_VOLUMEUP;
		case VK_MEDIA_NEXT_TRACK: return SDLK_AUDIONEXT;
		case VK_MEDIA_PREV_TRACK: return SDLK_AUDIOPREV;
		case VK_MEDIA_STOP: return SDLK_AUDIOSTOP;
		case VK_MEDIA_PLAY_PAUSE: return SDLK_AUDIOPLAY;
		case VK_LAUNCH_MAIL: return SDLK_MAIL;
		case VK_LAUNCH_MEDIA_SELECT: return SDLK_MEDIASELECT;
		default: return 0;
	}
}

static tjs_uint sdl_key_to_vk_key(SDL_Keycode key)
{
	// This is generated using regex find replace
	switch (key)
	{
		case SDLK_BACKSPACE: return VK_BACK;
		case SDLK_TAB: return VK_TAB;
		case SDLK_CLEAR: return VK_CLEAR;
		case SDLK_RETURN: return VK_RETURN;
		case SDLK_PAUSE: return VK_PAUSE;
		case SDLK_CAPSLOCK: return VK_CAPITAL;
		case SDLK_ESCAPE: return VK_ESCAPE;
		case SDLK_SPACE: return VK_SPACE;
		case SDLK_PAGEUP: return VK_PRIOR;
		case SDLK_PAGEDOWN: return VK_NEXT;
		case SDLK_END: return VK_END;
		case SDLK_HOME: return VK_HOME;
		case SDLK_LEFT: return VK_LEFT;
		case SDLK_UP: return VK_UP;
		case SDLK_RIGHT: return VK_RIGHT;
		case SDLK_DOWN: return VK_DOWN;
		case SDLK_SELECT: return VK_SELECT;
		case SDLK_EXECUTE: return VK_EXECUTE;
		case SDLK_PRINTSCREEN: return VK_SNAPSHOT;
		case SDLK_INSERT: return VK_INSERT;
		case SDLK_DELETE: return VK_DELETE;
		case SDLK_HELP: return VK_HELP;
		case SDLK_0: return VK_0;
		case SDLK_1: return VK_1;
		case SDLK_2: return VK_2;
		case SDLK_3: return VK_3;
		case SDLK_4: return VK_4;
		case SDLK_5: return VK_5;
		case SDLK_6: return VK_6;
		case SDLK_7: return VK_7;
		case SDLK_8: return VK_8;
		case SDLK_9: return VK_9;
		case SDLK_a: return VK_A;
		case SDLK_b: return VK_B;
		case SDLK_c: return VK_C;
		case SDLK_d: return VK_D;
		case SDLK_e: return VK_E;
		case SDLK_f: return VK_F;
		case SDLK_g: return VK_G;
		case SDLK_h: return VK_H;
		case SDLK_i: return VK_I;
		case SDLK_j: return VK_J;
		case SDLK_k: return VK_K;
		case SDLK_l: return VK_L;
		case SDLK_m: return VK_M;
		case SDLK_n: return VK_N;
		case SDLK_o: return VK_O;
		case SDLK_p: return VK_P;
		case SDLK_q: return VK_Q;
		case SDLK_r: return VK_R;
		case SDLK_s: return VK_S;
		case SDLK_t: return VK_T;
		case SDLK_u: return VK_U;
		case SDLK_v: return VK_V;
		case SDLK_w: return VK_W;
		case SDLK_x: return VK_X;
		case SDLK_y: return VK_Y;
		case SDLK_z: return VK_Z;
		case SDLK_LGUI: return VK_LWIN;
		case SDLK_RGUI: return VK_RWIN;
		case SDLK_SLEEP: return VK_SLEEP;
		case SDLK_KP_0: return VK_NUMPAD0;
		case SDLK_KP_1: return VK_NUMPAD1;
		case SDLK_KP_2: return VK_NUMPAD2;
		case SDLK_KP_3: return VK_NUMPAD3;
		case SDLK_KP_4: return VK_NUMPAD4;
		case SDLK_KP_5: return VK_NUMPAD5;
		case SDLK_KP_6: return VK_NUMPAD6;
		case SDLK_KP_7: return VK_NUMPAD7;
		case SDLK_KP_8: return VK_NUMPAD8;
		case SDLK_KP_9: return VK_NUMPAD9;
		case SDLK_KP_MULTIPLY: return VK_MULTIPLY;
		case SDLK_KP_PLUS: return VK_ADD;
		case SDLK_KP_MINUS: return VK_SUBTRACT;
		case SDLK_KP_PERIOD: return VK_DECIMAL;
		case SDLK_KP_DIVIDE: return VK_DIVIDE;
		case SDLK_F1: return VK_F1;
		case SDLK_F2: return VK_F2;
		case SDLK_F3: return VK_F3;
		case SDLK_F4: return VK_F4;
		case SDLK_F5: return VK_F5;
		case SDLK_F6: return VK_F6;
		case SDLK_F7: return VK_F7;
		case SDLK_F8: return VK_F8;
		case SDLK_F9: return VK_F9;
		case SDLK_F10: return VK_F10;
		case SDLK_F11: return VK_F11;
		case SDLK_F12: return VK_F12;
		case SDLK_F13: return VK_F13;
		case SDLK_F14: return VK_F14;
		case SDLK_F15: return VK_F15;
		case SDLK_F16: return VK_F16;
		case SDLK_F17: return VK_F17;
		case SDLK_F18: return VK_F18;
		case SDLK_F19: return VK_F19;
		case SDLK_F20: return VK_F20;
		case SDLK_F21: return VK_F21;
		case SDLK_F22: return VK_F22;
		case SDLK_F23: return VK_F23;
		case SDLK_F24: return VK_F24;
		case SDLK_NUMLOCKCLEAR: return VK_NUMLOCK;
		case SDLK_SCROLLLOCK: return VK_SCROLL;
		case SDLK_LSHIFT: return VK_LSHIFT;
		case SDLK_RSHIFT: return VK_RSHIFT;
		case SDLK_LCTRL: return VK_LCONTROL;
		case SDLK_RCTRL: return VK_RCONTROL;
		case SDLK_MENU: return VK_LMENU;
		case SDLK_LALT: return VK_LMENU;
		case SDLK_RALT: return VK_RMENU;
		case SDLK_AC_BACK: return VK_BROWSER_BACK;
		case SDLK_AC_FORWARD: return VK_BROWSER_FORWARD;
		case SDLK_AC_REFRESH: return VK_BROWSER_REFRESH;
		case SDLK_AC_STOP: return VK_BROWSER_STOP;
		case SDLK_AC_SEARCH: return VK_BROWSER_SEARCH;
		case SDLK_AC_BOOKMARKS: return VK_BROWSER_FAVORITES;
		case SDLK_AC_HOME: return VK_BROWSER_HOME;
		case SDLK_MUTE: return VK_VOLUME_MUTE;
		case SDLK_VOLUMEDOWN: return VK_VOLUME_DOWN;
		case SDLK_VOLUMEUP: return VK_VOLUME_UP;
		case SDLK_AUDIONEXT: return VK_MEDIA_NEXT_TRACK;
		case SDLK_AUDIOPREV: return VK_MEDIA_PREV_TRACK;
		case SDLK_AUDIOSTOP: return VK_MEDIA_STOP;
		case SDLK_AUDIOPLAY: return VK_MEDIA_PLAY_PAUSE;
		case SDLK_MAIL: return VK_LAUNCH_MAIL;
		case SDLK_MEDIASELECT: return VK_LAUNCH_MEDIA_SELECT;
		default: return 0;
	}
}

static int GetShiftState()
{
	int s = 0;
	if (TVPGetAsyncKeyState(VK_MENU)) s |= MK_ALT;
	if (TVPGetAsyncKeyState(VK_LMENU)) s |= MK_ALT;
	if (TVPGetAsyncKeyState(VK_RMENU)) s |= MK_ALT;
	if (TVPGetAsyncKeyState(VK_SHIFT)) s |= MK_SHIFT;
	if (TVPGetAsyncKeyState(VK_LSHIFT)) s |= MK_SHIFT;
	if (TVPGetAsyncKeyState(VK_RCONTROL)) s |= MK_SHIFT;
	if (TVPGetAsyncKeyState(VK_CONTROL)) s |= MK_CONTROL;
	if (TVPGetAsyncKeyState(VK_LCONTROL)) s |= MK_CONTROL;
	if (TVPGetAsyncKeyState(VK_RCONTROL)) s |= MK_CONTROL;
	return s;
}
static int GetMouseButtonState()
{
	int s = 0;
	if (TVPGetAsyncKeyState(VK_LBUTTON)) s |= ssLeft;
	if (TVPGetAsyncKeyState(VK_RBUTTON)) s |= ssRight;
	if (TVPGetAsyncKeyState(VK_MBUTTON)) s |= ssMiddle;
	if (TVPGetAsyncKeyState(VK_XBUTTON1)) s |= ssX1;
	if (TVPGetAsyncKeyState(VK_XBUTTON2)) s |= ssX2;
	return s;
}

#ifdef _WIN32
struct tTVPMessageReceiverRecord
{
	tTVPWindowMessageReceiver Proc;
	const void *UserData;
	bool Deliver(tTVPWindowMessage *Message)
	{
		return Proc(const_cast<void*>(UserData), Message);
	}
};
#endif

class TVPWindowWindow : public TTVPWindowForm
{
protected:
	SDL_Window *window;

	TVPWindowWindow *_prevWindow;
	TVPWindowWindow *_nextWindow;
	SDL_Texture *texture;
	SDL_Renderer *renderer;
	SDL_Surface *surface;
#ifdef KRKRZ_ENABLE_CANVAS
	SDL_GLContext context;
#endif
	tTJSNI_Window *TJSNativeInstance;
	bool hasDrawn = false;
	bool isVisible = true;
	bool visibilityHasInitialized = false;
	bool needsGraphicUpdate = false;
	bool isBeingDeleted = false;
	bool cursorTemporaryHidden = false;
	char *imeCompositionStr;
	size_t imeCompositionCursor;
	size_t imeCompositionLen;
	size_t imeCompositionSelection;
	SDL_Rect attentionPointRect;
	iTJSDispatch2 *fileDropArray;
	tjs_int fileDropArrayCount;
	TVPSDLBitmapCompletion *bitmapCompletion;
#ifdef KRKRZ_ENABLE_CANVAS
	tTVPOpenGLScreen *openGlScreen;
#endif
	int lastMouseX;
	int lastMouseY;

#ifdef KRKRSDL2_ENABLE_ZOOM
	tTVPRect FullScreenDestRect;
	tTVPRect LastSentDrawDeviceDestRect;

	//-- layer position / size
	tjs_int LayerLeft = 0;
	tjs_int LayerTop = 0;
	tjs_int LayerWidth = 32;
	tjs_int LayerHeight = 32;
	tjs_int ZoomDenom = 1; // Zooming factor denominator (setting)
	tjs_int ZoomNumer = 1; // Zooming factor numerator (setting)
	tjs_int ActualZoomDenom = 1; // Zooming factor denominator (actual)
	tjs_int ActualZoomNumer = 1; // Zooming factor numerator (actual)
	tjs_int InnerWidth = 32;
	tjs_int InnerHeight = 32;
#endif
#ifdef _WIN32
	tObjectList<tTVPMessageReceiverRecord> WindowMessageReceivers;
#endif

public:
	TVPWindowWindow(tTJSNI_Window *w);
	virtual ~TVPWindowWindow();
	/* Called from tTJSNI_Window */
	virtual void SetPaintBoxSize(tjs_int w, tjs_int h) override;
	void TranslateWindowToDrawArea(int &x, int &y);
	void TranslateDrawAreaToWindow(int &x, int &y);
#ifdef __EMSCRIPTEN__
	void TranslateEmscriptenCanvasToLogical(int &x, int &y);
#endif
	/* Called from tTJSNI_Window */
	virtual bool GetFormEnabled() override;
	/* Called from tTJSNI_Window */
	virtual void SetDefaultMouseCursor() override;
	/* Called from tTJSNI_Window */
	virtual void SetMouseCursor(tjs_int handle) override;
	/* Called from tTJSNI_Window */
	virtual void SetMouseCursorState(tTVPMouseCursorState mcs) override;
	/* Called from tTJSNI_Window */
	virtual tTVPMouseCursorState GetMouseCursorState() const override;
	void RestoreMouseCursor();
	/* Called from tTJSNI_Window */
	virtual void HideMouseCursor() override;
	/* Called from tTJSNI_Window */
	virtual void GetCursorPos(tjs_int &x, tjs_int &y) override;
	/* Called from tTJSNI_Window */
	virtual void SetCursorPos(tjs_int x, tjs_int y) override;
	/* Called from tTJSNI_Window */
	virtual void SetAttentionPoint(tjs_int left, tjs_int top, const struct tTVPFont * font) override;
	/* Called from tTJSNI_Window */
	virtual void BringToFront() override;
	/* Called from tTJSNI_Window */
	virtual void ShowWindowAsModal() override;
	/* Called from tTJSNI_Window */
	virtual bool GetVisible() override;
	/* Called from member SetVisibleFromScript */
	virtual void SetVisible(bool visible) override;
	/* Called from tTJSNI_Window */
	virtual void SetFullScreenMode(bool fullscreen) override;
	/* Called from tTJSNI_Window */
	virtual bool GetFullScreenMode() override;
	/* Called from tTJSNI_Window */
	virtual void SetBorderStyle(tTVPBorderStyle bs) override;
	/* Called from tTJSNI_Window */
	virtual tTVPBorderStyle GetBorderStyle() const override;
	/* Called from tTJSNI_Window */
	virtual tjs_string GetCaption() override;
	virtual void GetCaption(tjs_string & v) const override;
	/* Called from tTJSNI_Window */
	virtual void SetCaption(const tjs_string & v) override;
	/* Called from tTJSNI_Window */
	virtual void SetWidth(tjs_int w) override;
	/* Called from tTJSNI_Window */
	virtual void SetHeight(tjs_int h) override;
	/* Called from tTJSNI_Window */
	virtual void SetSize(tjs_int w, tjs_int h) override;
	virtual void GetSize(tjs_int &w, tjs_int &h) override;
	/* Called from tTJSNI_Window */
	virtual tjs_int GetWidth() const override;
	/* Called from tTJSNI_Window */
	virtual tjs_int GetHeight() const override;
	/* Called from tTJSNI_Window */
	virtual void SetMinWidth(tjs_int w) override;
	/* Called from tTJSNI_Window */
	virtual void SetMaxWidth(tjs_int w) override;
	/* Called from tTJSNI_Window */
	virtual void SetMinHeight(tjs_int h) override;
	/* Called from tTJSNI_Window */
	virtual void SetMaxHeight(tjs_int h) override;
	/* Called from tTJSNI_Window */
	virtual void SetMinSize(tjs_int w, tjs_int h) override;
	/* Called from tTJSNI_Window */
	virtual void SetMaxSize(tjs_int w, tjs_int h) override;
	/* Called from tTJSNI_Window */
	virtual tjs_int GetMinWidth() override;
	/* Called from tTJSNI_Window */
	virtual tjs_int GetMaxWidth() override;
	/* Called from tTJSNI_Window */
	virtual tjs_int GetMinHeight() override;
	/* Called from tTJSNI_Window */
	virtual tjs_int GetMaxHeight() override;
	/* Called from tTJSNI_Window */
	virtual tjs_int GetLeft() override;
	/* Called from tTJSNI_Window */
	virtual void SetLeft(tjs_int l) override;
	/* Called from tTJSNI_Window */
	virtual tjs_int GetTop() override;
	/* Called from tTJSNI_Window */
	virtual void SetTop(tjs_int t) override;
	/* Called from tTJSNI_Window */
	virtual void SetPosition(tjs_int l, tjs_int t) override;
	virtual TVPSDLBitmapCompletion *GetTVPSDLBitmapCompletion() override;
#ifdef KRKRZ_ENABLE_CANVAS
	virtual void SetOpenGLScreen(tTVPOpenGLScreen *s) override;
	virtual void SetSwapInterval(int interval) override;
	virtual void GetDrawableSize(tjs_int &w, tjs_int &h) override;
	virtual void Swap() override;
#endif
	virtual void Show() override;
	/* Called from tTJSNI_Window */
	virtual void TickBeat() override;
	/* Called from tTJSNI_Window */
	virtual void InvalidateClose() override;
	/* Called from tTJSNI_Window */
	virtual bool GetWindowActive() override;
	bool Closing = false;
	bool ProgramClosing = false;
	bool CanCloseWork = false;
	bool in_mode_ = false; // is modal
	int modal_result_ = 0;
	enum CloseAction
	{
		caNone,
		caHide,
		caFree,
		caMinimize
	};
	void OnClose(CloseAction& action);
	bool OnCloseQuery();
	/* Called from tTJSNI_Window */
	virtual void Close() override;
	/* Called from tTJSNI_Window */
	virtual void OnCloseQueryCalled(bool b) override;
	/* Called from tTJSNI_Window */
	virtual void SetImeMode(tTVPImeMode mode) override;
	/* Called from tTJSNI_Window */
	virtual void ResetImeMode() override;
	/* Called from tTJSNI_Window */
	virtual void UpdateWindow(tTVPUpdateType type) override;
	/* Called from tTJSNI_Window */
	virtual void InternalKeyDown(tjs_uint16 key, tjs_uint32 shift) override;
	/* Called from tTJSNI_Window */
	virtual void OnKeyUp(tjs_uint16 vk, int shift) override;
	/* Called from tTJSNI_Window */
	virtual void OnKeyPress(tjs_uint16 vk, int repeat, bool prevkeystate, bool convertkey) override;
	void UpdateActualZoom(void);
	void SetDrawDeviceDestRect(void);
	/* Called from tTJSNI_Window */
	virtual void SetZoom(tjs_int numer, tjs_int denom, bool set_logical = true) override;
	/* Called from tTJSNI_Window */
	virtual void SetZoomNumer(tjs_int n) override;
	/* Called from tTJSNI_Window */
	virtual tjs_int GetZoomNumer() const override;
	/* Called from tTJSNI_Window */
	virtual void SetZoomDenom(tjs_int d) override;
	/* Called from tTJSNI_Window */
	virtual tjs_int GetZoomDenom() const override;
	/* Called from tTJSNI_Window */
	virtual void SetInnerWidth(tjs_int v) override;
	/* Called from tTJSNI_Window */
	virtual void SetInnerHeight(tjs_int v) override;
	/* Called from tTJSNI_Window */
	virtual void SetInnerSize(tjs_int w, tjs_int h) override;
	/* Called from tTJSNI_Window */
	virtual tjs_int GetInnerWidth() override;
	/* Called from tTJSNI_Window */
	virtual tjs_int GetInnerHeight() override;
#ifdef _WIN32
	virtual void RegisterWindowMessageReceiver(tTVPWMRRegMode mode, void *proc, const void *userdata) override;
	bool InternalDeliverMessageToReceiver(tTVPWindowMessage &msg);
	virtual HWND GetHandle() const override;
#endif
	bool should_try_parent_window(SDL_Event event);
	void window_receive_event(SDL_Event event);
	bool window_receive_event_input(SDL_Event event);
};

TVPWindowWindow::TVPWindowWindow(tTJSNI_Window *w)
{
	this->window = nullptr;
	this->ResetImeMode();
	this->fileDropArray = nullptr;
	this->fileDropArrayCount = 0;
	this->lastMouseX = 0;
	this->lastMouseY = 0;
	this->_nextWindow = nullptr;
	this->_prevWindow = _lastWindowWindow;
	_lastWindowWindow = this;
	if (this->_prevWindow)
	{
		this->_prevWindow->_nextWindow = this;
	}
	if (!_currentWindowWindow)
	{
		_currentWindowWindow = this;
	}
	this->TJSNativeInstance = w;

	if (!SDL_WasInit(SDL_INIT_VIDEO))
	{
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			TVPThrowExceptionMessage(TJS_W("Cannot initialize SDL video subsystem: %1"), ttstr(SDL_GetError()));
		}
		refresh_controllers();
	}

	int new_window_x = SDL_WINDOWPOS_UNDEFINED;
	int new_window_y = SDL_WINDOWPOS_UNDEFINED;
	int new_window_w = 640;
	int new_window_h = 480;
	Uint32 window_flags = 0;

#ifdef SDL_HINT_RENDER_SCALE_QUALITY
	SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, "2", SDL_HINT_DEFAULT);
#endif

#ifdef KRKRZ_ENABLE_CANVAS
	if (!TVPIsEnableDrawDevice())
	{
#ifdef SDL_HINT_OPENGL_ES_DRIVER
		SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");
#endif
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		window_flags |= SDL_WINDOW_OPENGL;
	}
#endif

#ifdef SDL_HINT_TOUCH_MOUSE_EVENTS
	SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "1");
#endif
#ifdef SDL_HINT_MOUSE_TOUCH_EVENTS
	SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
#endif

#ifdef KRKRSDL2_WINDOW_SIZE_IS_LAYER_SIZE
	window_flags |= SDL_WINDOW_RESIZABLE;
#ifndef __EMSCRIPTEN__
	window_flags |= SDL_WINDOW_ALLOW_HIGHDPI;
#endif
#ifndef __EMSCRIPTEN__
	window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
#endif
	new_window_w = 0;
	new_window_h = 0;
#endif

#ifdef _WIN32
	window_flags |= SDL_WINDOW_HIDDEN;
#endif

	this->window = SDL_CreateWindow("krkrsdl2", new_window_x, new_window_y, new_window_w, new_window_h, window_flags);
	if (!this->window)
	{
		TVPThrowExceptionMessage(TJS_W("Cannot create SDL window: %1"), ttstr(SDL_GetError()));
	}
#ifdef KRKRZ_ENABLE_CANVAS
	this->context = nullptr;
	if (!TVPIsEnableDrawDevice())
	{
		this->context = SDL_GL_CreateContext(this->window);
		if (!this->context)
		{
			TVPThrowExceptionMessage(TJS_W("Cannot create SDL context: %1"), ttstr(SDL_GetError()));
		}
		SDL_GL_MakeCurrent(this->window, this->context);
	}
#endif
	this->renderer = nullptr;
	this->bitmapCompletion = nullptr;
#ifdef KRKRZ_ENABLE_CANVAS
	this->openGlScreen = nullptr;
#endif
	this->surface = nullptr;
#ifdef KRKRZ_ENABLE_CANVAS
	if (TVPIsEnableDrawDevice())
#endif
	{
#if !defined(__EMSCRIPTEN__) || !defined(__EMSCRIPTEN_PTHREADS__) || !defined(__EMSCRIPTEN_PROXY_TO_PTHREAD__)
#ifdef __EMSCRIPTEN__
		this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_ACCELERATED);
#else
		this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
#endif
		if (!this->renderer)
		{
			TVPAddLog(ttstr("Cannot create SDL renderer: ") + ttstr(SDL_GetError()));
		}
#endif

#ifdef __EMSCRIPTEN__
		// move the event watch to after the SDL_RendererEventWatch to ensure transformed values are received
		SDL_DelEventWatch(sdl_event_watch, nullptr);
		SDL_AddEventWatch(sdl_event_watch, nullptr);
#endif

		this->bitmapCompletion = new TVPSDLBitmapCompletion();
		if (!this->renderer)
		{
			this->surface = SDL_GetWindowSurface(this->window);
			if (!this->surface)
			{
				TVPAddLog(ttstr("Cannot get surface from SDL window: ") + ttstr(SDL_GetError()));
			}
			this->bitmapCompletion->surface = this->surface;
		}
		if (!this->renderer && !this->surface)
		{
			TVPThrowExceptionMessage(TJS_W("Cannot get surface or renderer from SDL window"));
		}
		this->texture = nullptr;
		if (this->renderer)
		{
			SDL_SetRenderDrawColor(this->renderer, 0x00, 0x00, 0x00, 0xFF);
		}
	}
#ifdef _WIN32
	::SetWindowLongPtr(this->GetHandle(), GWLP_USERDATA, (LONG_PTR)this);
#endif
	Application->AddWindow(this);
}

TVPWindowWindow::~TVPWindowWindow()
{
	if (_lastWindowWindow == this)
	{
		_lastWindowWindow = this->_prevWindow;
	}
	if (this->_nextWindow)
	{
		this->_nextWindow->_prevWindow = this->_prevWindow;
	}
	if (this->_prevWindow)
	{
		this->_prevWindow->_nextWindow = this->_nextWindow;
	}
	if (_currentWindowWindow == this)
	{
		_currentWindowWindow = _lastWindowWindow;
	}
	if (this->bitmapCompletion)
	{
		delete this->bitmapCompletion;
		this->bitmapCompletion = nullptr;
	}
#ifdef KRKRZ_ENABLE_CANVAS
	if (this->context)
	{
		SDL_GL_DeleteContext(this->context);
		this->context = nullptr;
	}
#endif
	if (this->texture && this->surface)
	{
		SDL_DestroyTexture(this->texture);
		this->texture = nullptr;
		SDL_FreeSurface(this->surface);
		this->surface = nullptr;
	}
	if (this->renderer)
	{
		SDL_DestroyRenderer(this->renderer);
		this->renderer = nullptr;
	}
	if (this->window)
	{
		SDL_DestroyWindow(this->window);
		this->window = nullptr;
	}

#ifdef _WIN32
	tjs_int count = this->WindowMessageReceivers.GetCount();
	for (tjs_int i = 0; i < count; i += 1)
	{
		tTVPMessageReceiverRecord *item = this->WindowMessageReceivers[i];
		if (!item)
		{
			continue;
		}
		delete item;
		this->WindowMessageReceivers.Remove(i);
	}
#endif

	Application->RemoveWindow(this);
}

void TVPWindowWindow::SetPaintBoxSize(tjs_int w, tjs_int h)
{
#ifdef KRKRSDL2_ENABLE_ZOOM
	this->LayerWidth = w;
	this->LayerHeight = h;
#endif
	if (this->renderer)
	{
#ifdef __EMSCRIPTEN__
		if (this->window)
		{
			SDL_SetWindowSize(this->window, w, h);
		}
#endif
		if (this->texture)
		{
			SDL_DestroyTexture(this->texture);
			this->texture = nullptr;
		}
		this->texture = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, w, h);
		if (!this->texture)
		{
			TVPThrowExceptionMessage(TJS_W("Cannot create texture texture: %1"), ttstr(SDL_GetError()));
		}
		this->bitmapCompletion->surface = nullptr;
		if (this->surface)
		{
			SDL_FreeSurface(this->surface);
			this->surface = nullptr;
		}
		this->surface = SDL_CreateRGBSurface(0, w, h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0);
		if (!this->surface)
		{
			TVPThrowExceptionMessage(TJS_W("Cannot create surface: %1"), ttstr(SDL_GetError()));
		}
		this->bitmapCompletion->surface = this->surface;
	}
	else if (this->window && this->bitmapCompletion)
	{
		// No renderer path (e.g. Emscripten with pthreads):
		// Resize the canvas/window to match the game layer size,
		// then re-acquire the window surface at the new dimensions.
		SDL_Surface *oldSurface = this->surface;
		KRKR_LOG_L2("[SPB] SetPaintBoxSize no-renderer: requested=%dx%d old_surface=%p old_pixels=%p old_size=%dx%d\n",
			w, h, (void*)oldSurface, oldSurface ? oldSurface->pixels : nullptr,
			oldSurface ? oldSurface->w : 0, oldSurface ? oldSurface->h : 0);
		// Skip surface replacement if dimensions already match — avoids
		// destroying pixel data that was just composited.
		if (oldSurface && oldSurface->w == w && oldSurface->h == h)
		{
			KRKR_LOG_L2("[SPB] Dimensions unchanged, skipping surface replacement\n");
		}
		else
		{
#ifdef __EMSCRIPTEN__
			emscripten_set_canvas_element_size("#canvas", w, h);
#endif
			SDL_SetWindowSize(this->window, w, h);
			this->bitmapCompletion->surface = nullptr;
			this->surface = SDL_GetWindowSurface(this->window);
			if (!this->surface)
			{
				TVPAddLog(ttstr("SetPaintBoxSize: Cannot get resized window surface: ") + ttstr(SDL_GetError()));
			}
			this->bitmapCompletion->surface = this->surface;
			KRKR_LOG_L2("[SPB] SetPaintBoxSize no-renderer: new_surface=%p new_pixels=%p new_size=%dx%d same_ptr=%d\n",
				(void*)this->surface, this->surface ? this->surface->pixels : nullptr,
				this->surface ? this->surface->w : 0, this->surface ? this->surface->h : 0,
				(oldSurface == this->surface) ? 1 : 0);
		}
	}
#ifndef KRKRSDL2_ENABLE_ZOOM
	SDL_Rect cliprect;
	cliprect.x = 0;
	cliprect.y = 0;
	cliprect.w = w;
	cliprect.h = h;
#endif
	if (this->renderer)
	{
#ifdef KRKRSDL2_ENABLE_ZOOM
		this->UpdateActualZoom();
#else
		SDL_RenderSetLogicalSize(this->renderer, w, h);
#endif
	}
	if (this->TJSNativeInstance)
	{
		tTVPRect r;
		r.left = 0;
		r.top = 0;
		r.right = w;
		r.bottom = h;
		this->TJSNativeInstance->NotifyWindowExposureToLayer(r);
		this->TJSNativeInstance->GetDrawDevice()->SetClipRectangle(r);
		this->TJSNativeInstance->GetDrawDevice()->SetDestRectangle(r);
	}
}

#ifndef _WIN32
static int MulDiv(int nNumber, int nNumerator, int nDenominator)
{
	return (int)(((int64_t)nNumber * (int64_t)nNumerator) / nDenominator);
}
#endif

void TVPWindowWindow::TranslateWindowToDrawArea(int &x, int &y)
{
#ifdef KRKRSDL2_ENABLE_ZOOM
#ifdef KRKRZ_ENABLE_CANVAS
	if (this->context)
	{
		return;
	}
#endif
	x -= this->LastSentDrawDeviceDestRect.left;
	y -= this->LastSentDrawDeviceDestRect.top;
	x = MulDiv(x, this->GetInnerWidth(), this->LastSentDrawDeviceDestRect.get_width());
	y = MulDiv(y, this->GetInnerHeight(), this->LastSentDrawDeviceDestRect.get_height());
#endif
}

#ifdef __EMSCRIPTEN__
void TVPWindowWindow::TranslateEmscriptenCanvasToLogical(int &x, int &y)
{
	int logicalW = 0;
	int logicalH = 0;
	if (this->renderer)
	{
		SDL_RenderGetLogicalSize(this->renderer, &logicalW, &logicalH);
	}
	if ((logicalW <= 0 || logicalH <= 0) && this->surface)
	{
		logicalW = this->surface->w;
		logicalH = this->surface->h;
	}
	if (logicalW <= 0 || logicalH <= 0)
	{
		return;
	}

	double paddingLeft = 0.0;
	double paddingTop = 0.0;
	double paddingRight = 0.0;
	double paddingBottom = 0.0;
	double rectW = 0.0;
	double rectH = 0.0;
	double canvasW = 0.0;
	double canvasH = 0.0;
	krkr_emscripten_get_canvas_box(&paddingLeft, &paddingTop,
		&paddingRight, &paddingBottom, &rectW, &rectH, &canvasW, &canvasH);
	double contentW = rectW - paddingLeft - paddingRight;
	double contentH = rectH - paddingTop - paddingBottom;
	if (rectW <= 0.0 || rectH <= 0.0 || contentW <= 0.0 || contentH <= 0.0)
	{
		return;
	}
	if (canvasW <= 0.0)
	{
		canvasW = logicalW;
	}
	if (canvasH <= 0.0)
	{
		canvasH = logicalH;
	}

	int rawX = x;
	int rawY = y;
	double rawPaddingLeft = paddingLeft * (double)logicalW / rectW;
	double rawPaddingTop = paddingTop * (double)logicalH / rectH;
	double rawContentW = contentW * (double)logicalW / rectW;
	double rawContentH = contentH * (double)logicalH / rectH;
	double scaledX = ((double)x - rawPaddingLeft) * (double)logicalW / rawContentW;
	double scaledY = ((double)y - rawPaddingTop) * (double)logicalH / rawContentH;
	double canvasScaledX = ((double)x - paddingLeft) * canvasW / contentW;
	double canvasScaledY = ((double)y - paddingTop) * canvasH / contentH;
	bool canvasDiffersFromLogical =
		canvasW > (double)logicalW + 1.0 || canvasW < (double)logicalW - 1.0 ||
		canvasH > (double)logicalH + 1.0 || canvasH < (double)logicalH - 1.0;
	bool canvasCandidateInLogical =
		canvasScaledX >= -1.0 && canvasScaledY >= -1.0 &&
		canvasScaledX <= (double)logicalW + 1.0 &&
		canvasScaledY <= (double)logicalH + 1.0;
	if (canvasDiffersFromLogical && canvasCandidateInLogical)
	{
		scaledX = canvasScaledX;
		scaledY = canvasScaledY;
	}
	x = (int)(scaledX + (scaledX >= 0.0 ? 0.5 : -0.5));
	y = (int)(scaledY + (scaledY >= 0.0 ? 0.5 : -0.5));

	static int scale_log_count = 0;
	scale_log_count++;
	if (TVPLogL3() && scale_log_count <= 80 &&
		(rawX != x || rawY != y || (int)(canvasW + 0.5) != logicalW || (int)(canvasH + 0.5) != logicalH))
	{
		fprintf(stderr,
			"[INPUT-SCALE] raw=(%d,%d) logical=(%d,%d) canvas=%.1fx%.1f padding=(%.1f,%.1f,%.1f,%.1f) rect=%.1fx%.1f content=%.1fx%.1f old=(%.1f,%.1f) canvasCandidate=(%.1f,%.1f) useCanvas=%d -> (%d,%d)\n",
			rawX, rawY, logicalW, logicalH, canvasW, canvasH, paddingLeft, paddingTop,
			paddingRight, paddingBottom, rectW, rectH, contentW, contentH,
			((double)rawX - rawPaddingLeft) * (double)logicalW / rawContentW,
			((double)rawY - rawPaddingTop) * (double)logicalH / rawContentH,
			canvasScaledX, canvasScaledY,
			(canvasDiffersFromLogical && canvasCandidateInLogical) ? 1 : 0, x, y);
	}
}
#endif

void TVPWindowWindow::TranslateDrawAreaToWindow(int &x, int &y)
{
#ifdef KRKRSDL2_ENABLE_ZOOM
#ifdef KRKRZ_ENABLE_CANVAS
	if (this->context)
	{
		return;
	}
#endif
	x = MulDiv(x, this->LastSentDrawDeviceDestRect.get_width(), this->GetInnerWidth());
	y = MulDiv(y, this->LastSentDrawDeviceDestRect.get_height(), this->GetInnerHeight());
	x += this->LastSentDrawDeviceDestRect.left;
	y += this->LastSentDrawDeviceDestRect.top;
#endif
}

bool TVPWindowWindow::GetFormEnabled()
{
	return (!!this->window && !!(SDL_GetWindowFlags(this->window) & SDL_WINDOW_SHOWN));
}
void TVPWindowWindow::SetDefaultMouseCursor()
{
	if (!sdl_system_cursors[0])
	{
		for (int i = 0; i < SDL_NUM_SYSTEM_CURSORS; i += 1)
		{
			sdl_system_cursors[i] = SDL_CreateSystemCursor((SDL_SystemCursor)i);
		}
	}
	SDL_SetCursor(sdl_system_cursors[SDL_SYSTEM_CURSOR_ARROW]);
}
void TVPWindowWindow::SetMouseCursor(tjs_int handle)
{
	if (!sdl_system_cursors[0])
	{
		for (int i = 0; i < SDL_NUM_SYSTEM_CURSORS; i += 1)
		{
			sdl_system_cursors[i] = SDL_CreateSystemCursor((SDL_SystemCursor)i);
		}
	}
	switch (handle)
	{
		case -2: // crArrow
			SDL_SetCursor(sdl_system_cursors[SDL_SYSTEM_CURSOR_ARROW]);
			break;
		case -3: // crCross
			SDL_SetCursor(sdl_system_cursors[SDL_SYSTEM_CURSOR_CROSSHAIR]);
			break;
		case -4: // crIBeam
			SDL_SetCursor(sdl_system_cursors[SDL_SYSTEM_CURSOR_IBEAM]);
			break;
		case -5: // crSize
			SDL_SetCursor(sdl_system_cursors[SDL_SYSTEM_CURSOR_SIZEALL]);
			break;
		case -6: // crSizeNESW
			SDL_SetCursor(sdl_system_cursors[SDL_SYSTEM_CURSOR_SIZENESW]);
			break;
		case -7: // crSizeNS
			SDL_SetCursor(sdl_system_cursors[SDL_SYSTEM_CURSOR_SIZENS]);
			break;
		case -8: // crSizeNWSE
			SDL_SetCursor(sdl_system_cursors[SDL_SYSTEM_CURSOR_SIZENWSE]);
			break;
		case -9: // crSizeWE
			SDL_SetCursor(sdl_system_cursors[SDL_SYSTEM_CURSOR_SIZEWE]);
			break;
		case -11: // crHourGlass
			SDL_SetCursor(sdl_system_cursors[SDL_SYSTEM_CURSOR_WAIT]);
			break;
		case -18: // crNo
			SDL_SetCursor(sdl_system_cursors[SDL_SYSTEM_CURSOR_NO]);
			break;
		case -19: // crAppStart
			SDL_SetCursor(sdl_system_cursors[SDL_SYSTEM_CURSOR_WAITARROW]);
			break;
		case -21: // crHandPoint
			SDL_SetCursor(sdl_system_cursors[SDL_SYSTEM_CURSOR_HAND]);
			break;
		case -22: // crSizeAll
			SDL_SetCursor(sdl_system_cursors[SDL_SYSTEM_CURSOR_SIZEALL]);
			break;
		default:
			SDL_SetCursor(sdl_system_cursors[SDL_SYSTEM_CURSOR_ARROW]);
			break;
	}
}
void TVPWindowWindow::SetMouseCursorState(tTVPMouseCursorState mcs)
{
	this->cursorTemporaryHidden = (mcs == mcsTempHidden);
	SDL_ShowCursor((mcs == mcsVisible) ? SDL_ENABLE : SDL_DISABLE);
}
tTVPMouseCursorState TVPWindowWindow::GetMouseCursorState() const
{
	return this->cursorTemporaryHidden ? mcsTempHidden : ((SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE) ? mcsVisible : mcsHidden);
}
void TVPWindowWindow::HideMouseCursor()
{
	this->SetMouseCursorState(mcsTempHidden);
}
void TVPWindowWindow::RestoreMouseCursor()
{
	if (this->cursorTemporaryHidden)
	{
		this->SetMouseCursorState(mcsVisible);
	}
}
void TVPWindowWindow::GetCursorPos(tjs_int &x, tjs_int &y)
{
	x = this->lastMouseX;
	y = this->lastMouseY;
	if (this->window != SDL_GetMouseFocus())
	{
		return;
	}
	tjs_int new_x = 0;
	tjs_int new_y = 0;
	SDL_GetMouseState(&new_x, &new_y);
	if (this->renderer)
	{
#ifdef KRKRSDL2_ENABLE_ZOOM
		this->TranslateWindowToDrawArea(new_x, new_y);
#else
		float scale_x, scale_y;
		SDL_Rect viewport;
		int window_w, window_h;
		int output_w, output_h;
		SDL_RenderGetScale(this->renderer, &scale_x, &scale_y);
		SDL_RenderGetViewport(this->renderer, &viewport);
		SDL_GetWindowSize(this->window, &window_w, &window_h);
		SDL_GetRendererOutputSize(this->renderer, &output_w, &output_h);
		float dpi_scale_x = (float)window_w / output_w;
		float dpi_scale_y = (float)window_h / output_h;
		new_x -= (int)(viewport.x * dpi_scale_x);
		new_y -= (int)(viewport.y * dpi_scale_y);
		new_x = (int)(new_x / (scale_x * dpi_scale_x));
		new_y = (int)(new_y / (scale_x * dpi_scale_y));
#endif
	}
	x = new_x;
	y = new_y;
}
void TVPWindowWindow::SetCursorPos(tjs_int x, tjs_int y)
{
	this->RestoreMouseCursor();
	if (!this->window)
	{
		return;
	}
	tjs_int new_x = x;
	tjs_int new_y = y;

	if (this->renderer)
	{
#ifdef KRKRSDL2_ENABLE_ZOOM
		this->TranslateDrawAreaToWindow(new_x, new_y);
#else
		float scale_x, scale_y;
		SDL_Rect viewport;
		int window_w, window_h;
		int output_w, output_h;
		SDL_RenderGetScale(this->renderer, &scale_x, &scale_y);
		SDL_RenderGetViewport(this->renderer, &viewport);
		SDL_GetWindowSize(this->window, &window_w, &window_h);
		SDL_GetRendererOutputSize(this->renderer, &output_w, &output_h);
		float dpi_scale_x = (float)window_w / output_w;
		float dpi_scale_y = (float)window_h / output_h;
		new_x = (int)(new_x * (scale_x * dpi_scale_x));
		new_y = (int)(new_y * (scale_x * dpi_scale_y));
		new_x += (int)(viewport.x * dpi_scale_x);
		new_y += (int)(viewport.y * dpi_scale_y);
#endif
	}
	SDL_WarpMouseInWindow(this->window, new_x, new_y);
}
void TVPWindowWindow::SetAttentionPoint(tjs_int left, tjs_int top, const struct tTVPFont *font)
{
	if (!font)
	{
		return;
	}
	this->attentionPointRect.x = left;
	this->attentionPointRect.y = top;
	this->attentionPointRect.w = 0;
	this->attentionPointRect.h = font->Height;
	this->TranslateDrawAreaToWindow(this->attentionPointRect.x, this->attentionPointRect.y);
	SDL_SetTextInputRect(&this->attentionPointRect);
}
void TVPWindowWindow::BringToFront()
{
	if (_currentWindowWindow != this)
	{
		if (_currentWindowWindow)
		{
			_currentWindowWindow->TJSNativeInstance->OnReleaseCapture();
		}
		_currentWindowWindow = this;
	}
	if (this->window)
	{
		SDL_RaiseWindow(this->window);
	}
}
void TVPWindowWindow::ShowWindowAsModal()
{
#if defined(KRKRSDL2_WINDOW_SIZE_IS_LAYER_SIZE)
	TVPThrowExceptionMessage(TJS_W("Showing window as modal is not supported"));
#else
	this->in_mode_ = true;
	this->BringToFront();
	this->modal_result_ = 0;
	while (this == _currentWindowWindow && !this->modal_result_)
	{
		process_events();
		if (::Application->IsTarminate())
		{
			this->modal_result_ = mrCancel;
		}
		else if (this->modal_result_)
		{
			break;
		}
	}
	this->in_mode_ = false;
#endif
}
bool TVPWindowWindow::GetVisible()
{
	return (!this->visibilityHasInitialized) ? this->isVisible : (!!this->window && !!(SDL_GetWindowFlags(this->window) & SDL_WINDOW_SHOWN));
}
void TVPWindowWindow::SetVisible(bool visible)
{
	this->isVisible = visible;
	if (!this->visibilityHasInitialized)
	{
		return;
	}
	if (this->window)
	{
#ifndef KRKRSDL2_WINDOW_SIZE_IS_LAYER_SIZE
		if (visible)
		{
			SDL_ShowWindow(this->window);
		}
		else
		{
			SDL_HideWindow(this->window);
		}
#endif
	}
	if (visible)
	{
		this->BringToFront();
	}
	else if (_currentWindowWindow == this)
	{
		_currentWindowWindow = this->_prevWindow ? this->_prevWindow : this->_nextWindow;
		if (_currentWindowWindow)
		{
			_currentWindowWindow->BringToFront();
		}
	}
}
void TVPWindowWindow::SetFullScreenMode(bool fullscreen)
{
#ifndef KRKRSDL2_WINDOW_SIZE_IS_LAYER_SIZE
	if (this->window)
	{
		SDL_SetWindowFullscreen(this->window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	}
	this->UpdateWindow(utNormal);
#endif
}
bool TVPWindowWindow::GetFullScreenMode()
{
#ifndef KRKRSDL2_WINDOW_SIZE_IS_LAYER_SIZE
	return !!this->window && !!(SDL_GetWindowFlags(this->window) & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP));
#else
	return false;
#endif
}
void TVPWindowWindow::SetBorderStyle(tTVPBorderStyle bs)
{
#ifndef KRKRSDL2_WINDOW_SIZE_IS_LAYER_SIZE
	if (this->window)
	{
		SDL_SetWindowBordered(this->window, (bs == bsNone) ? SDL_FALSE : SDL_TRUE);
		SDL_SetWindowResizable(this->window, (bs == bsSizeable || bs == bsSizeToolWin) ? SDL_TRUE : SDL_FALSE);
	}
#endif
}
tTVPBorderStyle TVPWindowWindow::GetBorderStyle() const
{
#ifndef KRKRSDL2_WINDOW_SIZE_IS_LAYER_SIZE
	if (this->window)
	{
		Uint32 flags = SDL_GetWindowFlags(this->window);
		if (flags & SDL_WINDOW_BORDERLESS)
		{
			return bsNone;
		}
		else if (flags & SDL_WINDOW_RESIZABLE)
		{
			return bsSizeable;
		}
	}
#endif
	return bsSingle;
}
tjs_string TVPWindowWindow::GetCaption()
{
	std::string v_utf8 = this->window ? SDL_GetWindowTitle(this->window) : "";
	tjs_string v_utf16;
	TVPUtf8ToUtf16(v_utf16, v_utf8);
	return v_utf16;
}
void TVPWindowWindow::GetCaption(tjs_string & v) const
{
	v.clear();
	std::string v_utf8 = this->window ? SDL_GetWindowTitle(this->window) : "";
	TVPUtf8ToUtf16(v, v_utf8);
}
void TVPWindowWindow::SetCaption(const tjs_string & v)
{
	if (!this->window)
	{
		return;
	}
	std::string v_utf8;
	if (!TVPUtf16ToUtf8(v_utf8, v))
	{
		return;
	}
	SDL_SetWindowTitle(this->window, v_utf8.c_str());
}
void TVPWindowWindow::SetWidth(tjs_int w)
{
#ifndef KRKRSDL2_WINDOW_SIZE_IS_LAYER_SIZE
	if (this->window)
	{
		int h;
		SDL_GetWindowSize(this->window, nullptr, &h);
		SDL_SetWindowSize(this->window, w, h);
		if (!this->renderer && this->surface)
		{
			this->bitmapCompletion->surface = nullptr;
			this->surface = SDL_GetWindowSurface(this->window);
			if (!this->surface)
			{
				TVPThrowExceptionMessage(TJS_W("Cannot get surface from SDL window: %1"), ttstr(SDL_GetError()));
			}
			this->bitmapCompletion->surface = this->surface;
		}
	}
#endif
#ifdef KRKRSDL2_ENABLE_ZOOM
	this->UpdateActualZoom();
#else
	this->UpdateWindow(utNormal);
#endif
}
void TVPWindowWindow::SetHeight(tjs_int h)
{
#ifndef KRKRSDL2_WINDOW_SIZE_IS_LAYER_SIZE
	if (this->window)
	{
		int w;
		SDL_GetWindowSize(this->window, &w, nullptr);
		SDL_SetWindowSize(this->window, w, h);
		if (!this->renderer && this->surface)
		{
			this->bitmapCompletion->surface = nullptr;
			this->surface = SDL_GetWindowSurface(this->window);
			if (!this->surface)
			{
				TVPThrowExceptionMessage(TJS_W("Cannot get surface from SDL window: %1"), ttstr(SDL_GetError()));
			}
			this->bitmapCompletion->surface = this->surface;
		}
	}
#endif
#ifdef KRKRSDL2_ENABLE_ZOOM
	this->UpdateActualZoom();
#else
	this->UpdateWindow(utNormal);
#endif
}
void TVPWindowWindow::SetSize(tjs_int w, tjs_int h)
{
#ifndef KRKRSDL2_WINDOW_SIZE_IS_LAYER_SIZE
	if (this->window)
	{
		SDL_SetWindowSize(this->window, w, h);
		if (!this->renderer && this->surface)
		{
			this->bitmapCompletion->surface = nullptr;
			this->surface = SDL_GetWindowSurface(this->window);
			if (!this->surface)
			{
				TVPThrowExceptionMessage(TJS_W("Cannot get surface from SDL window: %1"), ttstr(SDL_GetError()));
			}
			this->bitmapCompletion->surface = this->surface;
		}
	}
#endif
#ifdef KRKRSDL2_ENABLE_ZOOM
	this->UpdateActualZoom();
#else
	this->UpdateWindow(utNormal);
#endif
}
void TVPWindowWindow::GetSize(tjs_int &w, tjs_int &h)
{
	w = 0;
	h = 0;
#ifndef KRKRSDL2_WINDOW_SIZE_IS_LAYER_SIZE
	if (this->window)
	{
		SDL_GetWindowSize(this->window, &w, &h);
		return;
	}
#endif
	if (this->renderer)
	{
#ifdef KRKRSDL2_ENABLE_ZOOM
		SDL_GetRendererOutputSize(this->renderer, &w, &h);
#else
		SDL_RenderGetLogicalSize(this->renderer, &w, &h);
#endif
		return;
	}
}
tjs_int TVPWindowWindow::GetWidth() const
{
	int w = 0;
#ifndef KRKRSDL2_WINDOW_SIZE_IS_LAYER_SIZE
	if (this->window)
	{
		SDL_GetWindowSize(this->window, &w, nullptr);
		return w;
	}
#endif
	if (this->renderer)
	{
		int h;
#ifdef KRKRSDL2_ENABLE_ZOOM
		SDL_GetRendererOutputSize(this->renderer, &w, &h);
#else
		SDL_RenderGetLogicalSize(this->renderer, &w, &h);
#endif
	}
	return w;
}
tjs_int TVPWindowWindow::GetHeight() const
{
	int h = 0;
#ifndef KRKRSDL2_WINDOW_SIZE_IS_LAYER_SIZE
	if (this->window)
	{
		SDL_GetWindowSize(this->window, nullptr, &h);
		return h;
	}
#endif
	if (this->renderer)
	{
		int w;
#ifdef KRKRSDL2_ENABLE_ZOOM
		SDL_GetRendererOutputSize(this->renderer, &w, &h);
#else
		SDL_RenderGetLogicalSize(this->renderer, &w, &h);
#endif
	}
	return h;
}
void TVPWindowWindow::SetMinWidth(tjs_int w)
{
	this->SetMinSize(w, this->GetMinHeight());
}
void TVPWindowWindow::SetMaxWidth(tjs_int w)
{
	this->SetMaxSize(w, this->GetMaxHeight());
}
void TVPWindowWindow::SetMinHeight(tjs_int h)
{
	this->SetMinSize(this->GetMinWidth(), h);
}
void TVPWindowWindow::SetMaxHeight(tjs_int h)
{
	this->SetMaxSize(this->GetMaxWidth(), h);
}
void TVPWindowWindow::SetMinSize(tjs_int w, tjs_int h)
{
#ifndef KRKRSDL2_WINDOW_SIZE_IS_LAYER_SIZE
	if (this->window)
	{
		SDL_SetWindowMinimumSize(this->window, w, h);
	}
#endif
}
void TVPWindowWindow::SetMaxSize(tjs_int w, tjs_int h)
{
#ifndef KRKRSDL2_WINDOW_SIZE_IS_LAYER_SIZE
	if (this->window)
	{
		SDL_SetWindowMaximumSize(this->window, w, h);
	}
#endif
}
tjs_int TVPWindowWindow::GetMinWidth()
{
	int w = 0;
#ifndef KRKRSDL2_WINDOW_SIZE_IS_LAYER_SIZE
	if (this->window)
	{
		SDL_GetWindowMinimumSize(this->window, &w, nullptr);
		return w;
	}
#endif
	if (this->renderer)
	{
		int h;
#ifdef KRKRSDL2_ENABLE_ZOOM
		SDL_GetRendererOutputSize(this->renderer, &w, &h);
#else
		SDL_RenderGetLogicalSize(this->renderer, &w, &h);
#endif
	}
	return w;
}
tjs_int TVPWindowWindow::GetMaxWidth()
{
	int w = 0;
#ifndef KRKRSDL2_WINDOW_SIZE_IS_LAYER_SIZE
	if (this->window)
	{
		SDL_GetWindowMaximumSize(this->window, &w, nullptr);
		return w;
	}
#endif
	if (this->renderer)
	{
		int h;
#ifdef KRKRSDL2_ENABLE_ZOOM
		SDL_GetRendererOutputSize(this->renderer, &w, &h);
#else
		SDL_RenderGetLogicalSize(this->renderer, &w, &h);
#endif
	}
	return w;
}
tjs_int TVPWindowWindow::GetMinHeight()
{
	int h = 0;
#ifndef KRKRSDL2_WINDOW_SIZE_IS_LAYER_SIZE
	if (this->window)
	{
		SDL_GetWindowMinimumSize(this->window, &h, nullptr);
		return h;
	}
#endif
	if (this->renderer)
	{
		int w;
#ifdef KRKRSDL2_ENABLE_ZOOM
		SDL_GetRendererOutputSize(this->renderer, &w, &h);
#else
		SDL_RenderGetLogicalSize(this->renderer, &w, &h);
#endif
	}
	return h;
}
tjs_int TVPWindowWindow::GetMaxHeight()
{
	int h = 0;
#ifndef KRKRSDL2_WINDOW_SIZE_IS_LAYER_SIZE
	if (this->window)
	{
		SDL_GetWindowMaximumSize(this->window, &h, nullptr);
		return h;
	}
#endif
	if (this->renderer)
	{
		int w;
#ifdef KRKRSDL2_ENABLE_ZOOM
		SDL_GetRendererOutputSize(this->renderer, &w, &h);
#else
		SDL_RenderGetLogicalSize(this->renderer, &w, &h);
#endif
	}
	return h;
}
tjs_int TVPWindowWindow::GetLeft()
{
	int x = 0;
#ifndef KRKRSDL2_WINDOW_SIZE_IS_LAYER_SIZE
	if (this->window)
	{
		SDL_GetWindowPosition(this->window, &x, nullptr);
	}
#endif
	return x;
}
void TVPWindowWindow::SetLeft(tjs_int l)
{
	this->SetPosition(l, this->GetTop());
}
tjs_int TVPWindowWindow::GetTop()
{
	int y = 0;
#ifndef KRKRSDL2_WINDOW_SIZE_IS_LAYER_SIZE
	if (this->window)
	{
		SDL_GetWindowPosition(this->window, nullptr, &y);
	}
#endif
	return y;
}
void TVPWindowWindow::SetTop(tjs_int t)
{
	this->SetPosition(this->GetLeft(), t);
}
void TVPWindowWindow::SetPosition(tjs_int l, tjs_int t)
{
#ifndef KRKRSDL2_WINDOW_SIZE_IS_LAYER_SIZE
	if (this->window)
	{
		SDL_SetWindowPosition(this->window, l, t);
	}
#endif
}
TVPSDLBitmapCompletion *TVPWindowWindow::GetTVPSDLBitmapCompletion()
{
	this->needsGraphicUpdate = true;
	KRKR_LOG_L3("[GTSBC] this=%p needsGraphicUpdate=true surface=%p pixels=%p bc=%p bc_surface=%p\n",
		(void*)this,
		(void*)this->surface, this->surface ? this->surface->pixels : nullptr,
		(void*)this->bitmapCompletion,
		this->bitmapCompletion ? (void*)this->bitmapCompletion->surface : nullptr);
	return this->bitmapCompletion;
}
#ifdef KRKRZ_ENABLE_CANVAS
void TVPWindowWindow::SetOpenGLScreen(tTVPOpenGLScreen *s)
{
	this->openGlScreen = s;
}
void TVPWindowWindow::SetSwapInterval(int interval)
{
	if (this->context)
	{
#ifndef __EMSCRIPTEN__
		SDL_GL_SetSwapInterval(interval);
#endif
	}
}
void TVPWindowWindow::GetDrawableSize(tjs_int &w, tjs_int &h)
{
	w = 0;
	h = 0;
	if (this->context)
	{
		SDL_GL_GetDrawableSize(this->window, &w, &h);
	}
}
void TVPWindowWindow::Swap()
{
	if (this->context)
	{
		SDL_GL_SwapWindow(this->window);
	}
}
#endif
void TVPWindowWindow::Show()
{
}
void TVPWindowWindow::TickBeat()
{
	if (!this->visibilityHasInitialized)
	{
		this->visibilityHasInitialized = true;
		this->SetVisible(this->isVisible);
	}
#ifdef __EMSCRIPTEN__
	static int tb_call_count = 0;
	static int frames_without_update = 0;
	tb_call_count++;
	if (!this->needsGraphicUpdate) {
		frames_without_update++;
		if (TVPLogL3() && frames_without_update == 120) {
			if (this->TJSNativeInstance) {
				try {
					tTJSVariant result;
					TVPExecuteExpression(ttstr(TJS_W(
						"(function() {"
						"  var win = Window.mainWindow;"
						"  if (win === void) return 'no mainWindow';"
						"  var fl = win.fore.base;"
						"  var s = 'fore children=' + fl.children.count;"
						"  for (var i = 0; i < fl.children.count && i < 10; i++) {"
						"    var c = fl.children[i];"
						"    var p1 = 'na', p2 = 'na', p3 = 'na';"
						"    try { p1 = c.getMainPixel(805,523); } catch(e) { p1 = 'err'; }"
						"    try { p2 = c.getMainPixel(1065,682); } catch(e) { p2 = 'err'; }"
						"    try { p3 = c.getMainPixel(1140,650); } catch(e) { p3 = 'err'; }"
						"    s += ' | child[' + i + '] ' + c.name"
						"      + ' pos=' + c.left + ',' + c.top"
						"      + ' vis=' + c.visible + ' opacity=' + c.opacity"
						"      + ' ' + c.width + 'x' + c.height"
						"      + ' type=' + c.type"
						"      + ' img=' + c.imageWidth + 'x' + c.imageHeight"
						"      + ' children=' + c.children.count"
						"      + ' pxTop=' + p1 + ' pxBottom=' + p2 + ' pxOrb=' + p3;"
						"    for (var j = 0; j < c.children.count && j < 4; j++) {"
						"      var g = c.children[j];"
						"      var gp1 = 'na', gp2 = 'na', gp3 = 'na';"
						"      try { gp1 = g.getMainPixel(805,523); } catch(e) { gp1 = 'err'; }"
						"      try { gp2 = g.getMainPixel(1065,682); } catch(e) { gp2 = 'err'; }"
						"      try { gp3 = g.getMainPixel(1140,650); } catch(e) { gp3 = 'err'; }"
						"      s += ' | child[' + i + '.' + j + '] ' + g.name"
						"        + ' pos=' + g.left + ',' + g.top"
						"        + ' vis=' + g.visible + ' opacity=' + g.opacity"
						"        + ' ' + g.width + 'x' + g.height"
						"        + ' type=' + g.type"
						"        + ' img=' + g.imageWidth + 'x' + g.imageHeight"
						"        + ' children=' + g.children.count"
						"        + ' pxTop=' + gp1 + ' pxBottom=' + gp2 + ' pxOrb=' + gp3;"
						"    }"
						"  }"
						"  return s;"
						"})()"
					)), &result);
					if (result.Type() == tvtString) {
						std::string rs = ttstr(result.GetString()).AsNarrowStdString();
						fprintf(stderr, "[LAYER-STATE] %s\n", rs.c_str());
					}
				} catch(...) {
					fprintf(stderr, "[LAYER-STATE] Script execution failed\n");
				}
			}
		}
	} else {
		frames_without_update = 0;
	}
	if (TVPLogL3() && (tb_call_count <= 5 || tb_call_count % 500 == 0)) {
		fprintf(stderr, "[DIAG-TB] this=%p TickBeat #%d needsGU=%d bc=%p renderer=%p surface=%p window=%p\n",
			(void*)this,
			tb_call_count, (int)this->needsGraphicUpdate,
			(void*)this->bitmapCompletion, (void*)this->renderer,
			(void*)this->surface, (void*)this->window);
	}
#endif
	if (this->needsGraphicUpdate)
	{
#ifdef __EMSCRIPTEN__
		KRKR_LOG_L3("[TB-ENTER] this=%p needsGU=true bc=%p renderer=%p window=%p surface=%p\n",
			(void*)this, (void*)this->bitmapCompletion, (void*)this->renderer,
			(void*)this->window, (void*)this->surface);
#endif
		if (this->bitmapCompletion)
		{
			SDL_Rect rect;
			rect.x = this->bitmapCompletion->update_rect.left;
			rect.y = this->bitmapCompletion->update_rect.top;
			rect.w = this->bitmapCompletion->update_rect.get_width();
			rect.h = this->bitmapCompletion->update_rect.get_height();
			if (this->renderer)
			{
#ifdef __EMSCRIPTEN__
				bool forceFullPresent = krkr_emscripten_force_full_present();
				bool renderCopyFull = true;
#else
				bool forceFullPresent = false;
				bool renderCopyFull = false;
#endif
#if defined(KRKRSDL2_ENABLE_ZOOM) || defined(KRKRSDL2_RENDERER_FULL_UPDATES)
				SDL_RenderFillRect(this->renderer, nullptr);
#else
				SDL_Rect logical_rect;
				SDL_RenderGetLogicalSize(this->renderer, &(logical_rect.w), &(logical_rect.h));
				if (logical_rect.w == rect.w && logical_rect.h == rect.h)
				{
					// Clear extra artifacts
					SDL_RenderSetLogicalSize(this->renderer, 0, 0);
					SDL_RenderFillRect(this->renderer, nullptr);
					SDL_RenderSetLogicalSize(this->renderer, logical_rect.w, logical_rect.h);
				}
#endif
				int updateResult = 0;
				int copyResult = 0;
				const char *renderError = "";
				int fullReason = 0;
				int presentMode = 0;
				int uploadedRectCount = 0;
				int dirtyRectCount = 0;
				int dirtyFullCount = 0;
				int dirtyEmptyCount = 0;
				int dirtyUnionFull = 0;
				unsigned long long dirtyArea = 0;
				unsigned long long surfaceArea = 0;
				unsigned long long updatedBytes = 0;
				if (this->texture)
				{
					if (this->surface)
					{
						surfaceArea = (unsigned long long)this->surface->w * (unsigned long long)this->surface->h;
						dirtyRectCount = this->bitmapCompletion ? this->bitmapCompletion->update_rect_count : 0;
						dirtyFullCount = this->bitmapCompletion ? this->bitmapCompletion->update_rect_full_count : 0;
						dirtyEmptyCount = this->bitmapCompletion ? this->bitmapCompletion->update_rect_empty_count : 0;
						dirtyUnionFull = this->bitmapCompletion ? this->bitmapCompletion->update_rect_union_full : 0;
						if ((rect.w + rect.x) > this->surface->w)
						{
							rect.w = this->surface->w;
						}
						if ((rect.h + rect.y) > this->surface->h)
						{
							rect.h = this->surface->h;
						}
						if (rect.w > 0 && rect.h > 0)
						{
							SDL_PixelFormat *format = this->surface->format;
							int bytesPerPixel = format ? format->BytesPerPixel : 4;
							bool perfEnabled = TVPPerfEnabled();
							double updateStartMs = perfEnabled ? krkr_perf_now_ms() : 0.0;
#if defined(KRKRSDL2_RENDERER_FULL_UPDATES)
							updateResult = SDL_UpdateTexture(this->texture, nullptr, this->surface->pixels, this->surface->pitch);
							fullReason = 3;
							presentMode = 3;
							uploadedRectCount = 1;
							dirtyArea = surfaceArea;
							updatedBytes = (unsigned long long)this->surface->pitch * (unsigned long long)this->surface->h;
#elif defined(__EMSCRIPTEN__)
							updateResult = krkr_emscripten_update_texture_dirty(this->texture,
								this->surface,
								this->bitmapCompletion,
								rect,
								forceFullPresent,
								bytesPerPixel,
								&presentMode,
								&fullReason,
								&uploadedRectCount,
								&dirtyArea,
								&updatedBytes);
#else
							if (forceFullPresent)
							{
								updateResult = SDL_UpdateTexture(this->texture, nullptr, this->surface->pixels, this->surface->pitch);
								fullReason = 3;
								presentMode = 3;
								uploadedRectCount = 1;
								dirtyArea = surfaceArea;
								updatedBytes = (unsigned long long)this->surface->pitch * (unsigned long long)this->surface->h;
							}
							else
							{
								const Uint8 *pixels = (const Uint8*)this->surface->pixels + rect.y * this->surface->pitch + rect.x * bytesPerPixel;
								updateResult = SDL_UpdateTexture(this->texture, &rect, pixels, this->surface->pitch);
								presentMode = 1;
								uploadedRectCount = 1;
								dirtyArea = (unsigned long long)rect.w * (unsigned long long)rect.h;
								updatedBytes = dirtyArea * (unsigned long long)bytesPerPixel;
							}
#endif
							double afterUpdateMs = perfEnabled ? krkr_perf_now_ms() : 0.0;
							if (updateResult != 0) renderError = SDL_GetError();
							if (perfEnabled)
							{
								double copyStartMs = krkr_perf_now_ms();
#if defined(KRKRSDL2_ENABLE_ZOOM)
								SDL_Rect destrect;
								destrect.x = this->LastSentDrawDeviceDestRect.left;
								destrect.y = this->LastSentDrawDeviceDestRect.top;
								destrect.w = this->LastSentDrawDeviceDestRect.get_width();
								destrect.h = this->LastSentDrawDeviceDestRect.get_height();
								SDL_Rect srcrect;
								srcrect.x = 0;
								srcrect.y = 0;
								srcrect.w = this->GetInnerWidth();
								srcrect.h = this->GetInnerHeight();
								copyResult = SDL_RenderCopy(this->renderer, this->texture, &srcrect, &destrect);
#elif defined(KRKRSDL2_RENDERER_FULL_UPDATES)
								copyResult = SDL_RenderCopy(this->renderer, this->texture, nullptr, nullptr);
#else
								copyResult = (forceFullPresent || renderCopyFull)
									? SDL_RenderCopy(this->renderer, this->texture, nullptr, nullptr)
									: SDL_RenderCopy(this->renderer, this->texture, &rect, &rect);
#endif
								double afterCopyMs = krkr_perf_now_ms();
								if (copyResult != 0) renderError = SDL_GetError();
								double presentStartMs = krkr_perf_now_ms();
								SDL_RenderPresent(this->renderer);
								double afterPresentMs = krkr_perf_now_ms();
								const int fullFrame =
#if defined(KRKRSDL2_RENDERER_FULL_UPDATES) || defined(KRKRSDL2_ENABLE_ZOOM)
									1;
#else
									(this->surface && updatedBytes >= (unsigned long long)this->surface->pitch * (unsigned long long)this->surface->h) ? 1 : 0;
#endif
								if (fullFrame && fullReason == 0) {
									fullReason = dirtyFullCount > 0 ? 1 :
										(dirtyUnionFull ? 2 : 4);
								}
								static unsigned long long perfPresentCount = 0;
								static double perfPresentLastSummaryMs = 0.0;
								perfPresentCount++;
								double perfPresentTotalMs = afterPresentMs - updateStartMs;
								bool printPresentPerf = perfPresentCount <= 5 ||
									(perfPresentCount % 120 == 0) ||
									perfPresentTotalMs >= 8.0 ||
									(perfPresentLastSummaryMs > 0.0 && afterPresentMs - perfPresentLastSummaryMs >= 1000.0);
								if (printPresentPerf)
								{
									perfPresentLastSummaryMs = afterPresentMs;
									fprintf(stderr,
										"[PERF] present.sdl count=%llu mode=%s rects=%d rect=%d,%d,%d,%d surface=%dx%d dirty_area=%llu surface_area=%llu uploaded_bytes=%llu full=%d fallback_reason=%s dirty_rects=%d dirty_full=%d dirty_empty=%d dirty_union_full=%d update_ms=%.3f copy_ms=%.3f present_ms=%.3f total_ms=%.3f update_result=%d copy_result=%d err=%s\n",
										perfPresentCount,
										krkr_sdl_present_mode_name(presentMode),
										uploadedRectCount,
										rect.x, rect.y, rect.w, rect.h,
										this->surface ? this->surface->w : 0,
										this->surface ? this->surface->h : 0,
										dirtyArea,
										surfaceArea,
										updatedBytes,
										fullFrame,
										krkr_sdl_present_full_reason_name(fullReason),
										dirtyRectCount,
										dirtyFullCount,
										dirtyEmptyCount,
										dirtyUnionFull,
										afterUpdateMs - updateStartMs,
										afterCopyMs - copyStartMs,
										afterPresentMs - presentStartMs,
										perfPresentTotalMs,
										updateResult,
										copyResult,
										renderError ? renderError : "");
								}
#ifdef __EMSCRIPTEN__
								krkr_emscripten_record_sdl_present(this->surface, &rect, fullFrame,
									updatedBytes,
									fullReason,
									dirtyRectCount, dirtyFullCount, dirtyEmptyCount, dirtyUnionFull,
									presentMode, uploadedRectCount, dirtyArea, surfaceArea,
									afterUpdateMs - updateStartMs,
									afterCopyMs - copyStartMs,
									afterPresentMs - presentStartMs,
									afterPresentMs - updateStartMs);
#endif
								goto krkr_sdl_present_done;
							}
							else
							{
#if defined(KRKRSDL2_ENABLE_ZOOM)
								SDL_Rect destrect;
								destrect.x = this->LastSentDrawDeviceDestRect.left;
								destrect.y = this->LastSentDrawDeviceDestRect.top;
								destrect.w = this->LastSentDrawDeviceDestRect.get_width();
								destrect.h = this->LastSentDrawDeviceDestRect.get_height();
								SDL_Rect srcrect;
								srcrect.x = 0;
								srcrect.y = 0;
								srcrect.w = this->GetInnerWidth();
								srcrect.h = this->GetInnerHeight();
								copyResult = SDL_RenderCopy(this->renderer, this->texture, &srcrect, &destrect);
#elif defined(KRKRSDL2_RENDERER_FULL_UPDATES)
								copyResult = SDL_RenderCopy(this->renderer, this->texture, nullptr, nullptr);
#else
								copyResult = (forceFullPresent || renderCopyFull)
									? SDL_RenderCopy(this->renderer, this->texture, nullptr, nullptr)
									: SDL_RenderCopy(this->renderer, this->texture, &rect, &rect);
#endif
								if (copyResult != 0) renderError = SDL_GetError();
							}
						}
					}
#ifdef __EMSCRIPTEN__
					if (TVPLogL3() && this->surface && this->surface->pixels)
					{
						static int sdl_render_diag_count = 0;
						sdl_render_diag_count++;
						if (sdl_render_diag_count <= 5 || sdl_render_diag_count % 50 == 0) {
							tjs_uint32 *px = (tjs_uint32*)this->surface->pixels;
							int total = this->surface->w * this->surface->h;
							int nonzero = 0;
							int sample = total < 1000 ? total : 1000;
							for (int i = 0; i < sample && sample > 0; i++) {
								if (px[i * (total / sample)] != 0) nonzero++;
							}
							fprintf(stderr, "[SDL-RENDER] #%d rect=(%d,%d,%d,%d) update=%d copy=%d err=%s surface=%dx%d nonzero=%d/%d first4=[%08x,%08x,%08x,%08x]\n",
								sdl_render_diag_count,
								rect.x, rect.y, rect.w, rect.h,
								updateResult, copyResult, renderError ? renderError : "",
								this->surface ? this->surface->w : 0,
								this->surface ? this->surface->h : 0,
								nonzero, sample,
								total > 0 ? px[0] : 0, total > 1 ? px[1] : 0,
								total > 2 ? px[2] : 0, total > 3 ? px[3] : 0);
						}
					}
#endif
				}
				SDL_RenderPresent(this->renderer);
				#ifdef __EMSCRIPTEN__
					if (this->surface)
					{
						const int fullFrame =
#if defined(KRKRSDL2_RENDERER_FULL_UPDATES) || defined(KRKRSDL2_ENABLE_ZOOM)
							1;
#else
							(this->surface && updatedBytes >= (unsigned long long)this->surface->pitch * (unsigned long long)this->surface->h) ? 1 : 0;
#endif
#if defined(KRKRSDL2_RENDERER_FULL_UPDATES) || defined(KRKRSDL2_ENABLE_ZOOM)
						fullReason = 3;
						presentMode = 3;
#else
						if (fullFrame && fullReason == 0)
						{
							fullReason = forceFullPresent ? 3 :
								((this->bitmapCompletion && this->bitmapCompletion->update_rect_full_count > 0)
									? 1
								: ((this->bitmapCompletion && this->bitmapCompletion->update_rect_union_full)
									? 2
									: 4));
						}
#endif
					krkr_emscripten_record_sdl_present(this->surface, &rect, fullFrame, updatedBytes,
						fullReason,
						this->bitmapCompletion ? this->bitmapCompletion->update_rect_count : 0,
						this->bitmapCompletion ? this->bitmapCompletion->update_rect_full_count : 0,
						this->bitmapCompletion ? this->bitmapCompletion->update_rect_empty_count : 0,
						this->bitmapCompletion ? this->bitmapCompletion->update_rect_union_full : 0,
						presentMode, uploadedRectCount, dirtyArea, surfaceArea,
						0.0, 0.0, 0.0, 0.0);
				}
	#endif
krkr_sdl_present_done:
#if !defined(KRKRSDL2_ENABLE_ZOOM) && !defined(KRKRSDL2_RENDERER_FULL_UPDATES)
				if (!forceFullPresent && !renderCopyFull && logical_rect.w == rect.w && logical_rect.h == rect.h)
				{
					// Clear extra artifacts (for the back buffer)
					SDL_RenderSetLogicalSize(this->renderer, 0, 0);
					SDL_RenderFillRect(this->renderer, nullptr);
					SDL_RenderSetLogicalSize(this->renderer, logical_rect.w, logical_rect.h);
				}
				if (!forceFullPresent && !renderCopyFull && this->texture)
				{
					SDL_RenderCopy(this->renderer, this->texture, &rect, &rect);
				}
#endif
				this->hasDrawn = true;
			}
		else if (this->window && this->surface)
		{
#ifdef __EMSCRIPTEN__
			{
				tjs_uint32 *px = (tjs_uint32*)this->surface->pixels;
				int total = this->surface->w * this->surface->h;
				int nonzero = 0;
				int sample = total < 1000 ? total : 1000;
				for (int i = 0; i < sample && sample > 0; i++) {
					if (px[i * (total / sample)] != 0) nonzero++;
				}
				KRKR_LOG_L3("[TB] pre-put: surface=%p pixels=%p %dx%d rect=(%d,%d,%d,%d) nonzero=%d/%d first4=[%08x,%08x,%08x,%08x] bc_rect=(%d,%d,%d,%d)\n",
					(void*)this->surface, (void*)this->surface->pixels,
					this->surface->w, this->surface->h,
					rect.x, rect.y, rect.w, rect.h,
					nonzero, sample,
					total > 0 ? px[0] : 0, total > 1 ? px[1] : 0,
					total > 2 ? px[2] : 0, total > 3 ? px[3] : 0,
					this->bitmapCompletion ? this->bitmapCompletion->update_rect.left : -1,
					this->bitmapCompletion ? this->bitmapCompletion->update_rect.top : -1,
					this->bitmapCompletion ? this->bitmapCompletion->update_rect.right : -1,
					this->bitmapCompletion ? this->bitmapCompletion->update_rect.bottom : -1);
			}
			krkr_emscripten_put_surface(this->surface);
#else
			int ret = SDL_UpdateWindowSurfaceRects(this->window, &rect, 1);
#endif
			this->hasDrawn = true;
		}
			this->needsGraphicUpdate = false;
			if (this->bitmapCompletion) this->bitmapCompletion->StartUpdateCycle();
		}
	}
#ifdef KRKRZ_ENABLE_CANVAS
	else if (this->context && this->TJSNativeInstance)
	{
		this->TJSNativeInstance->StartDrawing();
		this->hasDrawn = true;
	}
#endif
}
void TVPWindowWindow::InvalidateClose()
{
	this->TJSNativeInstance = nullptr;
	this->SetVisible(false);
	delete this;
}
bool TVPWindowWindow::GetWindowActive()
{
	return _currentWindowWindow == this && SDL_GetWindowFlags(this->window) & SDL_WINDOW_INPUT_FOCUS;
}
void TVPWindowWindow::OnClose(CloseAction& action)
{
	action = (!this->modal_result_) ? caNone : caHide;

	if (!this->ProgramClosing || !this->TJSNativeInstance)
	{
		return;
	}
	if (!this->TJSNativeInstance->IsMainWindow())
	{
		action = caFree;
	}
	iTJSDispatch2 *obj = this->TJSNativeInstance->GetOwnerNoAddRef();
	this->TJSNativeInstance->NotifyWindowClose();
	obj->Invalidate(0, nullptr, nullptr, obj);
	this->TJSNativeInstance = nullptr;
	this->SetVisible(false);
}
bool TVPWindowWindow::OnCloseQuery()
{
	// closing actions are 3 patterns;
	// 1. closing action by the user
	// 2. "close" method
	// 3. object invalidation

	if (TVPGetBreathing())
	{
		return false;
	}

	// the default event handler will invalidate this object when an onCloseQuery
	// event reaches the handler.
	if (!this->TJSNativeInstance)
	{
		return true;
	}
	if (this->modal_result_ && this->modal_result_ != mrCancel/* mrCancel=when close button is pushed in modal window */)
	{
		return true;
	}
	iTJSDispatch2 *obj = this->TJSNativeInstance->GetOwnerNoAddRef();
	if (!obj)
	{
		return true;
	}
	tTJSVariant arg[1] = { true };
	static ttstr eventname(TJS_W("onCloseQuery"));

	if (!this->ProgramClosing)
	{
		// close action does not happen immediately
		if (this->TJSNativeInstance)
		{
			TVPPostInputEvent(new tTVPOnCloseInputEvent(this->TJSNativeInstance));
		}

		this->Closing = true; // waiting closing...
	//	TVPSystemControl->NotifyCloseClicked();
		return false;
	}
	else
	{
		this->CanCloseWork = true;
		TVPPostEvent(obj, obj, eventname, 0, TVP_EPT_IMMEDIATE, sizeof(arg)/sizeof(arg[0]), arg);
		process_events(); // for post event
		// this event happens immediately
		// and does not return until done
		return this->CanCloseWork; // CanCloseWork is set by the event handler
	}
}
void TVPWindowWindow::Close()
{
	// closing action by "close" method
	if (this->Closing)
	{
		return; // already waiting closing...
	}

	this->ProgramClosing = true;
	try
	{
		//tTVPWindow::Close();
		if (this->in_mode_)
		{
			this->modal_result_ = mrCancel;
		}
		else if (this->OnCloseQuery())
		{
			CloseAction action = caFree;
			this->OnClose(action);
			switch (action)
			{
				case caNone:
					break;
				case caHide:
					this->SetVisible(false);
					break;
				case caMinimize:
					if (this->window)
					{
						SDL_MinimizeWindow(this->window);
					}
					break;
				case caFree:
				default:
					this->isBeingDeleted = true;
					//::PostMessage(this->GetHandle(), TVP_EV_WINDOW_RELEASE, 0, 0);
					break;
			}
		}
	}
	catch (...)
	{
		this->ProgramClosing = false;
		throw;
	}
	this->ProgramClosing = false;
}
void TVPWindowWindow::OnCloseQueryCalled(bool b)
{
	// closing is allowed by onCloseQuery event handler
	if (!this->ProgramClosing)
	{
		// closing action by the user
		if (b)
		{
			if (this->in_mode_)
			{
				this->modal_result_ = 1; // when modal
			}
			else
			{
				this->SetVisible(false);  // just hide
			}

			this->Closing = false;
			if (this->TJSNativeInstance)
			{
				if (this->TJSNativeInstance->IsMainWindow())
				{
					// this is the main window
					iTJSDispatch2 *obj = this->TJSNativeInstance->GetOwnerNoAddRef();
					obj->Invalidate(0, nullptr, nullptr, obj);
				}
			}
			else
			{
				delete this;
			}
		}
		else
		{
			this->Closing = false;
		}
	}
	else
	{
		// closing action by the program
		this->CanCloseWork = b;
	}
}
void TVPWindowWindow::SetImeMode(tTVPImeMode mode)
{
	if (!this->window || mode == ::imDisable || mode == ::imClose)
	{
		this->ResetImeMode();
	}
	else if (!SDL_IsTextInputActive())
	{
		SDL_SetTextInputRect(&this->attentionPointRect);
		SDL_StartTextInput();
	}
}
void TVPWindowWindow::ResetImeMode()
{
	this->imeCompositionStr = nullptr;
	this->imeCompositionLen = 0;
	this->imeCompositionCursor = 0;
	this->imeCompositionSelection = 0;
	this->attentionPointRect.x = 0;
	this->attentionPointRect.y = 0;
	this->attentionPointRect.w = 0;
	this->attentionPointRect.h = 0;
	if (this->window && SDL_IsTextInputActive())
	{
		SDL_SetTextInputRect(&this->attentionPointRect);
		SDL_StopTextInput();
	}
}
void TVPWindowWindow::UpdateWindow(tTVPUpdateType type)
{
	if (!this->TJSNativeInstance)
	{
		return;
	}
	tTVPRect r;
#ifdef KRKRSDL2_ENABLE_ZOOM
	r.left = 0;
	r.top = 0;
	r.right = this->LayerWidth;
	r.bottom = this->LayerHeight;
#else
	r.clear();
	if (this->renderer)
	{
		SDL_RenderGetLogicalSize(this->renderer, &(r.right), &(r.bottom));
		SDL_RenderSetLogicalSize(this->renderer, r.right, r.bottom);
	}
	else if (this->window)
	{
		SDL_GetWindowSize(this->window, &(r.right), &(r.bottom));
	}
#endif
	this->TJSNativeInstance->NotifyWindowExposureToLayer(r);
	TVPDeliverWindowUpdateEvents();
}
void TVPWindowWindow::InternalKeyDown(tjs_uint16 key, tjs_uint32 shift)
{
	TVPPostInputEvent(new tTVPOnKeyDownInputEvent(this->TJSNativeInstance, key, shift));
}
void TVPWindowWindow::OnKeyUp(tjs_uint16 vk, int shift)
{
	TVPPostInputEvent(new tTVPOnKeyUpInputEvent(this->TJSNativeInstance, vk, shift));
}
void TVPWindowWindow::OnKeyPress(tjs_uint16 vk, int repeat, bool prevkeystate, bool convertkey)
{
	TVPPostInputEvent(new tTVPOnKeyPressInputEvent(this->TJSNativeInstance, vk));
}

#ifdef KRKRSDL2_ENABLE_ZOOM
//---------------------------------------------------------------------------
//! @brief	do reduction for numer over denom
static void TVPDoReductionNumerAndDenom(tjs_int &n, tjs_int &d)
{
	tjs_int a = n;
	tjs_int b = d;
	while (b)
	{
		tjs_int t = b;
		b = a % b;
		a = t;
	}
	n = n / a;
	d = d / a;
}
#endif

void TVPWindowWindow::UpdateActualZoom(void)
{
#ifdef KRKRSDL2_ENABLE_ZOOM
	if (!this->renderer)
	{
		return;
	}
#ifdef KRKRZ_ENABLE_CANVAS
	if (this->context)
	{
		return;
	}
#endif
	// determine fullscreen zoom factor and client size
	int sb_w, sb_h, zoom_d, zoom_n, output_w, output_h;
	SDL_GetRendererOutputSize(this->renderer, &output_w, &output_h);

	float layer_aspect = (float)this->GetInnerWidth() / this->GetInnerHeight();
	float output_aspect = (float)output_w / output_h;

	// 0=letterbox, 1=crop
	int scale_policy = 0;
	SDL_Rect viewport;
	if (SDL_fabs(layer_aspect - output_aspect) < 0.0001)
	{
		zoom_n = 1;
		zoom_d = 1;
		viewport.x = 0;
		viewport.y = 0;
		viewport.w = this->GetInnerWidth();
		viewport.h = this->GetInnerHeight();
	}
	else if (layer_aspect > output_aspect)
	{
		if (scale_policy == 1)
		{
			// Crop left and right
			zoom_n = output_h;
			zoom_d = this->GetInnerHeight();
			TVPDoReductionNumerAndDenom(zoom_n, zoom_d);
			viewport.y = 0;
			viewport.h = output_h;
			viewport.w = MulDiv(this->GetInnerWidth(), zoom_n, zoom_d);
			viewport.x = (output_w - viewport.w) / 2;
		}
		else
		{
			// Top and bottom black bars (letterbox)
			zoom_n = output_w;
			zoom_d = this->GetInnerWidth();
			TVPDoReductionNumerAndDenom(zoom_n, zoom_d);
			viewport.x = 0;
			viewport.w = output_w;
			viewport.h = MulDiv(this->GetInnerHeight(), zoom_n, zoom_d);
			viewport.y = (output_h - viewport.h) / 2;
		}
	}
	else
	{
		if (scale_policy == 1)
		{
			// Crop top and bottom
			zoom_n = output_w;
			zoom_d = this->GetInnerWidth();
			TVPDoReductionNumerAndDenom(zoom_n, zoom_d);
			viewport.x = 0;
			viewport.w = output_w;
			viewport.h = MulDiv(this->GetInnerHeight(), zoom_n, zoom_d);
			viewport.y = (output_h - viewport.h) / 2;
		}
		else
		{
			// Left and right black bars (letterbox)
			zoom_n = output_h;
			zoom_d = this->GetInnerHeight();
			TVPDoReductionNumerAndDenom(zoom_n, zoom_d);
			viewport.y = 0;
			viewport.h = output_h;
			viewport.w = MulDiv(this->GetInnerWidth(), zoom_n, zoom_d);
			viewport.x = (output_w - viewport.w) / 2;
		}
	}
	this->FullScreenDestRect.set_size(viewport.w, viewport.h);
	this->FullScreenDestRect.set_offsets(viewport.x, viewport.y);

	this->ActualZoomNumer = zoom_n;
	this->ActualZoomDenom = zoom_d;
	this->SetDrawDeviceDestRect();
#endif
}

void TVPWindowWindow::SetDrawDeviceDestRect(void)
{
#ifdef KRKRSDL2_ENABLE_ZOOM
	tTVPRect destrect;
	tjs_int w = MulDiv(this->GetInnerWidth(),  this->ActualZoomNumer, this->ActualZoomDenom);
	tjs_int h = MulDiv(this->GetInnerHeight(), this->ActualZoomNumer, this->ActualZoomDenom);
	if (w < 1)
	{
		w = 1;
	}
	if (h < 1)
	{
		h = 1;
	}
	{
		destrect.left = this->FullScreenDestRect.left;
		destrect.top = this->FullScreenDestRect.top;
		destrect.right = destrect.left + w;
		destrect.bottom = destrect.top + h;
	}

	if (this->LastSentDrawDeviceDestRect != destrect)
	{
		this->LastSentDrawDeviceDestRect = destrect;
		this->UpdateWindow(utNormal);
	}
#endif
}

void TVPWindowWindow::SetZoom(tjs_int numer, tjs_int denom, bool set_logical)
{
#ifdef KRKRSDL2_ENABLE_ZOOM
	bool ischanged = false;
	// set layer zooming factor;
	// the zooming factor is passed in numerator/denoiminator style.
	// we must find GCM to optimize numer/denium via Euclidean algorithm.
	TVPDoReductionNumerAndDenom(numer, denom);
	if (set_logical)
	{
		if (this->ZoomNumer != numer || this->ZoomDenom != denom)
		{
			ischanged = true;
		}
		this->ZoomNumer = numer;
		this->ZoomDenom = denom;
	}
	this->UpdateActualZoom();
#endif
}

void TVPWindowWindow::SetZoomNumer(tjs_int n)
{
	this->SetZoom(n, this->GetZoomDenom());
}

tjs_int TVPWindowWindow::GetZoomNumer() const
{
#ifdef KRKRSDL2_ENABLE_ZOOM
	return this->ZoomNumer;
#else
	return 1;
#endif
}

void TVPWindowWindow::SetZoomDenom(tjs_int d)
{
	this->SetZoom(this->GetZoomNumer(), d);
}

tjs_int TVPWindowWindow::GetZoomDenom() const
{
#ifdef KRKRSDL2_ENABLE_ZOOM
	return this->ZoomDenom;
#else
	return 1;
#endif
}

void TVPWindowWindow::SetInnerWidth(tjs_int v)
{
#ifdef KRKRSDL2_ENABLE_ZOOM
	this->SetInnerSize(v, this->GetInnerHeight());
#else
	this->SetWidth(v);
#endif
}

void TVPWindowWindow::SetInnerHeight(tjs_int v)
{
#ifdef KRKRSDL2_ENABLE_ZOOM
	this->SetInnerSize(this->GetInnerWidth(), v);
#else
	this->SetHeight(v);
#endif
}

void TVPWindowWindow::SetInnerSize(tjs_int w, tjs_int h)
{
#ifdef KRKRSDL2_ENABLE_ZOOM
	this->InnerWidth = w;
	this->InnerHeight = h;
	this->UpdateActualZoom();
#endif
	this->SetSize(w, h);
}

tjs_int TVPWindowWindow::GetInnerWidth()
{
#ifdef KRKRSDL2_ENABLE_ZOOM
	return this->InnerWidth;
#else
	return this->GetWidth();
#endif
}

tjs_int TVPWindowWindow::GetInnerHeight()
{
#ifdef KRKRSDL2_ENABLE_ZOOM
	return this->InnerHeight;
#else
	return this->GetHeight();
#endif
}

#ifdef _WIN32
void TVPWindowWindow::RegisterWindowMessageReceiver(tTVPWMRRegMode mode, void *proc, const void *userdata)
{
	switch (mode)
	{
		case wrmRegister:
		case wrmUnregister:
		{
			tjs_int count = this->WindowMessageReceivers.GetCount();
			tjs_int i;
			for (i = 0; i < count; i += 1)
			{
				tTVPMessageReceiverRecord *item = this->WindowMessageReceivers[i];
				if (!item)
				{
					continue;
				}
				if ((void*)item->Proc == proc)
				{
					if (mode == wrmRegister)
					{
						break; // have already registered
					}
					if (mode == wrmUnregister)
					{
						// found
						this->WindowMessageReceivers.Remove(i);
						delete item;
					}
				}
			}
			if (mode == wrmRegister)
			{
				if (i == count)
				{
					// not have registered
					tTVPMessageReceiverRecord *item = new tTVPMessageReceiverRecord();
					item->Proc = (tTVPWindowMessageReceiver)proc;
					item->UserData = userdata;
					this->WindowMessageReceivers.Add(item);
				}
			}
			if (mode == wrmUnregister)
			{
				this->WindowMessageReceivers.Compact();
			}
			break;
		}
		default:
			break;
	}
}

bool TVPWindowWindow::InternalDeliverMessageToReceiver(tTVPWindowMessage &msg)
{
	if (!this->WindowMessageReceivers.GetCount() || !this->TJSNativeInstance)
	{
		return false;
	}
#ifdef KRKRSDL2_ENABLE_PLUGINS
	if (TVPPluginUnloadedAtSystemExit)
	{
		return false;
	}
#endif

	tObjectListSafeLockHolder<tTVPMessageReceiverRecord> holder(this->WindowMessageReceivers);
	tjs_int count = this->WindowMessageReceivers.GetSafeLockedObjectCount();

	bool block = false;
	for (tjs_int i = 0; i < count; i += 1)
	{
		tTVPMessageReceiverRecord *item = this->WindowMessageReceivers.GetSafeLockedObjectAt(i);
		if (!item)
		{
			continue;
		}
		bool b = item->Deliver(&msg);
		block = block || b;
	}
	return block;
}

HWND TVPWindowWindow::GetHandle() const
{
	SDL_SysWMinfo syswminfo;
	SDL_VERSION(&syswminfo.version);
	return SDL_GetWindowWMInfo(this->window, &syswminfo) ? syswminfo.info.win.window : nullptr;
}
#endif

bool TVPWindowWindow::should_try_parent_window(SDL_Event event)
{
	bool tryParentWindow = false;
	if (this->window && this->_prevWindow)
	{
		uint32_t windowID = SDL_GetWindowID(this->window);
		switch (event.type)
		{
			case SDL_DROPFILE:
			case SDL_DROPTEXT:
			case SDL_DROPBEGIN:
			case SDL_DROPCOMPLETE:
				tryParentWindow = event.drop.windowID != windowID;
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				tryParentWindow = event.key.windowID != windowID;
				break;
			case SDL_MOUSEMOTION:
				tryParentWindow = event.motion.windowID != windowID;
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				tryParentWindow = event.button.windowID != windowID;
				break;
			case SDL_MOUSEWHEEL:
				tryParentWindow = event.wheel.windowID != windowID;
				break;
			case SDL_TEXTEDITING:
				tryParentWindow = event.edit.windowID != windowID;
				break;
			case SDL_TEXTINPUT:
				tryParentWindow = event.text.windowID != windowID;
				break;
			case SDL_WINDOWEVENT:
				tryParentWindow = event.window.windowID != windowID;
				break;
			default:
				break;
		}
	}
	return tryParentWindow;
}

void TVPWindowWindow::window_receive_event(SDL_Event event)
{
	if (this->isBeingDeleted)
	{
		delete this;
		return;
	}
	if (this->should_try_parent_window(event))
	{
		if (!this->in_mode_)
		{
			this->_prevWindow->window_receive_event(event);
		}
		return;
	}
	if (this->window && this->hasDrawn)
	{
		tjs_uint32 s = TVP_TShiftState_To_uint32(GetShiftState());
		s |= GetMouseButtonState();
		if (this->TJSNativeInstance->CanDeliverEvents())
		{
			switch (event.type)
			{
				case SDL_TEXTINPUT:
				case SDL_TEXTEDITING:
				{
					if (!SDL_IsTextInputActive())
					{
						return;
					}
					// TODO: figure out vertical edit
					for (size_t i = 0; i < this->imeCompositionSelection; i += 1)
					{
						TVPPostInputEvent(new tTVPOnKeyDownInputEvent(this->TJSNativeInstance, VK_LEFT, TVP_SS_SHIFT));
						TVPPostInputEvent(new tTVPOnKeyUpInputEvent(this->TJSNativeInstance, VK_LEFT, TVP_SS_SHIFT));
#if 0
						TVPPostInputEvent(new tTVPOnKeyDownInputEvent(this->TJSNativeInstance, VK_DOWN, TVP_SS_SHIFT));
						TVPPostInputEvent(new tTVPOnKeyUpInputEvent(this->TJSNativeInstance, VK_DOWN, TVP_SS_SHIFT));
#endif
					}
					for (size_t i = 0; i < this->imeCompositionLen - this->imeCompositionCursor; i += 1)
					{
						TVPPostInputEvent(new tTVPOnKeyDownInputEvent(this->TJSNativeInstance, VK_RIGHT, 0));
						TVPPostInputEvent(new tTVPOnKeyUpInputEvent(this->TJSNativeInstance, VK_RIGHT, 0));
#if 0
						TVPPostInputEvent(new tTVPOnKeyDownInputEvent(this->TJSNativeInstance, VK_DOWN, 0));
						TVPPostInputEvent(new tTVPOnKeyUpInputEvent(this->TJSNativeInstance, VK_DOWN, 0));
#endif
					}
					for (size_t i = 0; i < this->imeCompositionLen; i += 1)
					{
						TVPPostInputEvent(new tTVPOnKeyDownInputEvent(this->TJSNativeInstance, VK_BACK, 0));
						TVPPostInputEvent(new tTVPOnKeyUpInputEvent(this->TJSNativeInstance, VK_BACK, 0));
					}
					switch (event.type)
					{
						case SDL_TEXTINPUT:
							this->imeCompositionStr = event.text.text;
							this->imeCompositionCursor = 0;
							this->imeCompositionSelection = 0;
							break;
						case SDL_TEXTEDITING:
							this->imeCompositionStr = event.edit.text;
							this->imeCompositionCursor = event.edit.start;
							this->imeCompositionSelection = event.edit.length;
							break;
					}
					size_t buffer_len = TVPUtf8ToWideCharString((const char*)(this->imeCompositionStr), nullptr);
					if (buffer_len == (size_t)-1)
					{
						return;
					}
					if (buffer_len)
					{
						this->imeCompositionLen = buffer_len;
						tjs_char *buffer = new tjs_char[buffer_len + 1];
						TVPUtf8ToWideCharString((const char*)(this->imeCompositionStr), buffer);
						for (size_t i = 0; i < buffer_len; i += 1)
						{
							TVPPostInputEvent(new tTVPOnKeyPressInputEvent(this->TJSNativeInstance, buffer[i]));
						}
						delete[] buffer;
					}
					else
					{
						this->imeCompositionStr = nullptr;
						this->imeCompositionLen = 0;
						this->imeCompositionCursor = 0;
						this->imeCompositionSelection = 0;
					}
					if (event.type == SDL_TEXTEDITING)
					{
						for (size_t i = 0; i < this->imeCompositionLen - this->imeCompositionCursor; i += 1)
						{
							TVPPostInputEvent(new tTVPOnKeyDownInputEvent(this->TJSNativeInstance, VK_LEFT, 0));
							TVPPostInputEvent(new tTVPOnKeyUpInputEvent(this->TJSNativeInstance, VK_LEFT, 0));
#if 0
							TVPPostInputEvent(new tTVPOnKeyDownInputEvent(this->TJSNativeInstance, VK_UP, 0));
							TVPPostInputEvent(new tTVPOnKeyUpInputEvent(this->TJSNativeInstance, VK_UP, 0));
#endif
						}
						for (size_t i = 0; i < this->imeCompositionSelection; i += 1)
						{
							TVPPostInputEvent(new tTVPOnKeyDownInputEvent(this->TJSNativeInstance, VK_RIGHT, TVP_SS_SHIFT));
							TVPPostInputEvent(new tTVPOnKeyUpInputEvent(this->TJSNativeInstance, VK_RIGHT, TVP_SS_SHIFT));
#if 0
							TVPPostInputEvent(new tTVPOnKeyDownInputEvent(this->TJSNativeInstance, VK_DOWN, TVP_SS_SHIFT));
							TVPPostInputEvent(new tTVPOnKeyUpInputEvent(this->TJSNativeInstance, VK_DOWN, TVP_SS_SHIFT));
#endif
						}
					}
					if (event.type == SDL_TEXTINPUT)
					{
						this->imeCompositionStr = nullptr;
						this->imeCompositionLen = 0;
					}
					return;
				}
				case SDL_DROPBEGIN:
				{
					if (!this->fileDropArray)
					{
						this->fileDropArray = TJSCreateArrayObject();
					}
					return;
				}
				case SDL_DROPCOMPLETE:
				{
					if (this->fileDropArray)
					{
						tTJSVariant arg(this->fileDropArray, this->fileDropArray);
						TVPPostInputEvent(new tTVPOnFileDropInputEvent(this->TJSNativeInstance, arg));
						this->fileDropArray->Release();
						this->fileDropArray = nullptr;
						this->fileDropArrayCount = 0;
					}
					return;
				}
				case SDL_DROPFILE:
				case SDL_DROPTEXT:
				{
					if (event.drop.file)
					{
						std::string f_utf8 = event.drop.file;
						tjs_string f_utf16;
						TVPUtf8ToUtf16(f_utf16, f_utf8);
						SDL_free(event.drop.file);
						if (TVPIsExistentStorageNoSearch(f_utf16))
						{
							tTJSVariant val = TVPNormalizeStorageName(ttstr(f_utf16));
							if (this->fileDropArray)
							{
								this->fileDropArray->PropSetByNum(TJS_MEMBERENSURE|TJS_IGNOREPROP, this->fileDropArrayCount, &val, this->fileDropArray);
								this->fileDropArrayCount += 1;
							}
							else
							{
								iTJSDispatch2 *file_drop_array_single = TJSCreateArrayObject();
								file_drop_array_single->PropSetByNum(TJS_MEMBERENSURE|TJS_IGNOREPROP, 0, &val, file_drop_array_single);
								{
									tTJSVariant arg(file_drop_array_single, file_drop_array_single);
									TVPPostInputEvent(new tTVPOnFileDropInputEvent(this->TJSNativeInstance, arg));
								}
								file_drop_array_single->Release();
							}
						}
					}
					return;
				}
				case SDL_CONTROLLERDEVICEADDED:
				case SDL_CONTROLLERDEVICEREMOVED:
				case SDL_CONTROLLERDEVICEREMAPPED:
				{
					refresh_controllers();
					return;
				}
				case SDL_WINDOWEVENT:
				{
					switch (event.window.event)
					{
						case SDL_WINDOWEVENT_EXPOSED:
						{
							this->UpdateWindow(utNormal);
							return;
						}
						case SDL_WINDOWEVENT_MINIMIZED:
						case SDL_WINDOWEVENT_MAXIMIZED:
						case SDL_WINDOWEVENT_RESTORED:
						case SDL_WINDOWEVENT_RESIZED:
						case SDL_WINDOWEVENT_SIZE_CHANGED:
						{
#ifdef KRKRSDL2_ENABLE_ZOOM
							this->UpdateActualZoom();
#else
							this->UpdateWindow(utNormal);
#endif
							TVPPostInputEvent(new tTVPOnResizeInputEvent(this->TJSNativeInstance), TVP_EPT_REMOVE_POST);
							return;
						}
						case SDL_WINDOWEVENT_ENTER:
						{
							TVPPostInputEvent(new tTVPOnMouseEnterInputEvent(this->TJSNativeInstance));
							return;
						}
						case SDL_WINDOWEVENT_LEAVE:
						{
							TVPPostInputEvent(new tTVPOnMouseOutOfWindowInputEvent(this->TJSNativeInstance));
							TVPPostInputEvent(new tTVPOnMouseLeaveInputEvent(this->TJSNativeInstance));
							return;
						}
						case SDL_WINDOWEVENT_FOCUS_GAINED:
						case SDL_WINDOWEVENT_FOCUS_LOST:
						{
							TVPPostInputEvent(new tTVPOnWindowActivateEvent(this->TJSNativeInstance, event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED), TVP_EPT_REMOVE_POST);
							return;
						}
						case SDL_WINDOWEVENT_CLOSE:
						{
							TVPPostInputEvent(new tTVPOnCloseInputEvent(this->TJSNativeInstance));
							return;
						}
						default:
						{
							return;
						}
					}
				}
				case SDL_QUIT:
				{
					TVPPostInputEvent(new tTVPOnCloseInputEvent(this->TJSNativeInstance));
					return;
				}
				default:
				{
#ifndef __EMSCRIPTEN__
					this->window_receive_event_input(event);
#endif
					return;
				}
			}
		}
	}
}

bool TVPWindowWindow::window_receive_event_input(SDL_Event event)
{
	if (this->isBeingDeleted)
	{
		delete this;
		return false;
	}
	if (this->should_try_parent_window(event))
	{
		if (!this->in_mode_)
		{
			return this->_prevWindow->window_receive_event_input(event);
		}
		return false;
	}
	if (this->window && this->hasDrawn)
	{
		tjs_uint32 s = TVP_TShiftState_To_uint32(GetShiftState());
		s |= GetMouseButtonState();
		if (this->TJSNativeInstance->CanDeliverEvents())
		{
			switch (event.type)
			{
				case SDL_MOUSEMOTION:
				{
#ifdef __EMSCRIPTEN__
					// Browser/mobile builds are click-first targets, but drag controls
					// such as config sliders still need move events while a button is held.
					int rawX = event.motion.x;
					int rawY = event.motion.y;
					this->lastMouseX = event.motion.x;
					this->lastMouseY = event.motion.y;
					this->TranslateEmscriptenCanvasToLogical(this->lastMouseX, this->lastMouseY);
					this->TranslateWindowToDrawArea(this->lastMouseX, this->lastMouseY);
					if (event.motion.state != 0)
					{
						this->RestoreMouseCursor();
						TVPPostInputEvent(new tTVPOnMouseMoveInputEvent(this->TJSNativeInstance, this->lastMouseX, this->lastMouseY, s));
						{
							static int drag_motion_log_count = 0;
							drag_motion_log_count++;
							if (TVPLogL3() && drag_motion_log_count <= 80) {
								fprintf(stderr, "[INPUT-SDL] move drag raw=(%d,%d) draw=(%d,%d) buttons=%u shift=%u\n",
									rawX, rawY, this->lastMouseX, this->lastMouseY, (unsigned)event.motion.state, (unsigned)s);
							}
						}
						return true;
					}
					{
						static int motion_log_count = 0;
						motion_log_count++;
						if (TVPLogL3() && motion_log_count <= 40) {
							fprintf(stderr, "[INPUT-SDL] move ignored raw=(%d,%d) draw=(%d,%d) shift=%u\n",
								rawX, rawY, this->lastMouseX, this->lastMouseY, (unsigned)s);
						}
					}
					return true;
#else
					this->RestoreMouseCursor();
					int rawX = event.motion.x;
					int rawY = event.motion.y;
					this->lastMouseX = event.motion.x;
					this->lastMouseY = event.motion.y;
					this->TranslateWindowToDrawArea(this->lastMouseX, this->lastMouseY);
					TVPPostInputEvent(new tTVPOnMouseMoveInputEvent(this->TJSNativeInstance, this->lastMouseX, this->lastMouseY, s));
					return true;
#endif
				}
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
				{
					if (SDL_IsTextInputActive() && this->imeCompositionStr)
					{
						return false;
					}
					tTVPMouseButton btn;
					bool hasbtn = true;
					switch (event.button.button)
					{
						case SDL_BUTTON_RIGHT:
							btn = tTVPMouseButton::mbRight;
							break;
						case SDL_BUTTON_MIDDLE:
							btn = tTVPMouseButton::mbMiddle;
							break;
						case SDL_BUTTON_LEFT:
							btn = tTVPMouseButton::mbLeft;
							break;
						case SDL_BUTTON_X1:
							btn = tTVPMouseButton::mbX1;
							break;
						case SDL_BUTTON_X2:
							btn = tTVPMouseButton::mbX2;
							break;
						default:
							hasbtn = false;
							break;
					}
					if (hasbtn)
					{
						int rawX = event.button.x;
						int rawY = event.button.y;
						this->lastMouseX = event.button.x;
						this->lastMouseY = event.button.y;
#ifdef __EMSCRIPTEN__
						this->TranslateEmscriptenCanvasToLogical(this->lastMouseX, this->lastMouseY);
#endif
						this->TranslateWindowToDrawArea(this->lastMouseX, this->lastMouseY);
#ifdef __EMSCRIPTEN__
						{
							if (event.type == SDL_MOUSEBUTTONUP &&
								event.button.button == SDL_BUTTON_LEFT &&
								event.button.clicks < 2) {
								TVPArmMotionClickBurstTrace(
									"sdl-left-up",
									(double)this->lastMouseX,
									(double)this->lastMouseY);
								TVPArmMotionHostBurstTrace("sdl-left-up");
							}
							krkr_emscripten_record_mouse_input(
								event.type == SDL_MOUSEBUTTONDOWN ? "down" : "up",
								rawX, rawY, this->lastMouseX, this->lastMouseY,
								(int)event.button.button, (int)event.button.clicks, s);
							static int mouse_log_count = 0;
							mouse_log_count++;
							if (TVPLogL3() && mouse_log_count <= 80) {
								fprintf(stderr, "[INPUT-SDL] %s raw=(%d,%d) draw=(%d,%d) btn=%d clicks=%d shift=%u\n",
									event.type == SDL_MOUSEBUTTONDOWN ? "down" : "up",
									rawX, rawY, this->lastMouseX, this->lastMouseY,
									(int)event.button.button, (int)event.button.clicks, (unsigned)s);
							}
						}
#endif
#ifndef __EMSCRIPTEN__
						TVPPostInputEvent(new tTVPOnMouseMoveInputEvent(this->TJSNativeInstance, this->lastMouseX, this->lastMouseY, s));
#endif
						switch (event.type)
						{
							case SDL_MOUSEBUTTONDOWN:
								TVPPostInputEvent(new tTVPOnMouseDownInputEvent(this->TJSNativeInstance, this->lastMouseX, this->lastMouseY, btn, s));
								break;
							case SDL_MOUSEBUTTONUP:
								if (event.button.clicks >= 2)
								{
									TVPPostInputEvent(new tTVPOnDoubleClickInputEvent(this->TJSNativeInstance, this->lastMouseX, this->lastMouseY));
								}
								else
								{
									TVPPostInputEvent(new tTVPOnClickInputEvent(this->TJSNativeInstance, this->lastMouseX, this->lastMouseY));
								}
								TVPPostInputEvent(new tTVPOnMouseUpInputEvent(this->TJSNativeInstance, this->lastMouseX, this->lastMouseY, btn, s));
								break;
						}
						return true;
					}
					return false;
				}
				case SDL_MOUSEWHEEL:
				{
					this->TranslateWindowToDrawArea(this->lastMouseX, this->lastMouseY);
					TVPPostInputEvent(new tTVPOnMouseWheelInputEvent(this->TJSNativeInstance, event.wheel.x, event.wheel.y, this->lastMouseX, this->lastMouseY));
					return true;
				}
				case SDL_FINGERMOTION:
				{
					TVPPostInputEvent(new tTVPOnTouchMoveInputEvent(this->TJSNativeInstance, event.tfinger.x, event.tfinger.y, 1, 1, event.tfinger.fingerId));
					return true;
				}
				case SDL_FINGERDOWN:
				case SDL_FINGERUP:
				{
					switch (event.tfinger.type)
					{
						case SDL_FINGERDOWN:
							TVPPostInputEvent(new tTVPOnTouchDownInputEvent(this->TJSNativeInstance, event.tfinger.x, event.tfinger.y, 1, 1, event.tfinger.fingerId));
							break;
						case SDL_FINGERUP:
							TVPPostInputEvent(new tTVPOnTouchUpInputEvent(this->TJSNativeInstance, event.tfinger.x, event.tfinger.y, 1, 1, event.tfinger.fingerId));
							break;
					}
					return true;
				}
				case SDL_MULTIGESTURE:
				{
					TVPPostInputEvent(new tTVPOnTouchScalingInputEvent(this->TJSNativeInstance, 0, event.mgesture.dDist, event.mgesture.x, event.mgesture.y, 0));
					TVPPostInputEvent(new tTVPOnTouchRotateInputEvent(this->TJSNativeInstance, 0, event.mgesture.dTheta, event.mgesture.dDist, event.mgesture.x, event.mgesture.y, 0));
					return true;
				}
				case SDL_CONTROLLERBUTTONDOWN:
				case SDL_CONTROLLERBUTTONUP:
				{
					switch (event.cbutton.state)
					{
						case SDL_PRESSED:
							TVPPostInputEvent(new tTVPOnKeyDownInputEvent(this->TJSNativeInstance, sdl_gamecontrollerbutton_to_vk_key(event.cbutton.button), s));
							break;
						case SDL_RELEASED:
							if (!SDL_IsTextInputActive())
							{
								TVPPostInputEvent(new tTVPOnKeyPressInputEvent(this->TJSNativeInstance, sdl_gamecontrollerbutton_to_vk_key(event.cbutton.button)));
							}
							TVPPostInputEvent(new tTVPOnKeyUpInputEvent(this->TJSNativeInstance, sdl_gamecontrollerbutton_to_vk_key(event.cbutton.button), s));
							break;
					}
					return true;
				}
				case SDL_KEYDOWN:
				{
					if (SDL_IsTextInputActive())
					{
						if (this->imeCompositionStr)
						{
							return false;
						}
					}
					if (event.key.repeat)
					{
						s |= TVP_SS_REPEAT;
					}
					tjs_uint unified_vk_key = 0;
					switch (event.key.keysym.sym)
					{
						case SDLK_LSHIFT:
						case SDLK_RSHIFT:
							unified_vk_key = VK_SHIFT;
							break;
						case SDLK_LCTRL:
						case SDLK_RCTRL:
							unified_vk_key = VK_CONTROL;
							break;
						case SDLK_LALT:
						case SDLK_RALT:
							unified_vk_key = VK_MENU;
							break;
					}
					TVPPostInputEvent(new tTVPOnKeyDownInputEvent(this->TJSNativeInstance, sdl_key_to_vk_key(event.key.keysym.sym), s));
					if (unified_vk_key)
					{
						TVPPostInputEvent(new tTVPOnKeyDownInputEvent(this->TJSNativeInstance, unified_vk_key, s));
					}
					SDL_SetTextInputRect(&this->attentionPointRect);
					return true;
				}
				case SDL_KEYUP:
				{
					if (SDL_IsTextInputActive())
					{
						if (this->imeCompositionStr)
						{
							return false;
						}
					}
					tjs_uint unified_vk_key = 0;
					switch (event.key.keysym.sym)
					{
						case SDLK_LSHIFT:
						case SDLK_RSHIFT:
							unified_vk_key = VK_SHIFT;
							break;
						case SDLK_LCTRL:
						case SDLK_RCTRL:
							unified_vk_key = VK_CONTROL;
							break;
						case SDLK_LALT:
						case SDLK_RALT:
							unified_vk_key = VK_MENU;
							break;
					}
					if (!SDL_IsTextInputActive())
					{
						TVPPostInputEvent(new tTVPOnKeyPressInputEvent(this->TJSNativeInstance, sdl_key_to_vk_key(event.key.keysym.sym)));
						if (unified_vk_key)
						{
							TVPPostInputEvent(new tTVPOnKeyPressInputEvent(this->TJSNativeInstance, unified_vk_key));
						}
					}
					TVPPostInputEvent(new tTVPOnKeyUpInputEvent(this->TJSNativeInstance, sdl_key_to_vk_key(event.key.keysym.sym), s));
					if (unified_vk_key)
					{
						TVPPostInputEvent(new tTVPOnKeyUpInputEvent(this->TJSNativeInstance, unified_vk_key, s));
					}
					SDL_SetTextInputRect(&this->attentionPointRect);
					return true;
				}
				default:
				{
					return false;
				}
			}
		}
	}
	return false;
}

void sdl_process_events()
{
	if (!SDL_WasInit(SDL_INIT_EVENTS))
	{
		return;
	}
#if defined(__EMSCRIPTEN__) && defined(__EMSCRIPTEN_PTHREADS__)
	NativeEventQueueImplement::DrainNativeEventQueue();
#endif
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
#if defined(_WIN32) && defined(KRKRSDL2_USE_WIN32_EVENT_QUEUE)
#else
		if (event.type == NativeEventQueueImplement::native_event_queue_custom_event_type)
		{
			((NativeEvent*)event.user.data2)->HandleEvent();
		}
		else
#endif
		if (_currentWindowWindow)
		{
			_currentWindowWindow->window_receive_event(event);
		}
		else if (event.type == SDL_QUIT)
		{
			Application->Terminate();
		}
	}
}

#ifdef _WIN32
static void sdl_windows_message_hook(void *userdata, void *hWnd, unsigned int message, Uint64 wParam, Sint64 lParam)
{
	TVPWindowWindow *win = reinterpret_cast<TVPWindowWindow*>(::GetWindowLongPtr((HWND)hWnd, GWLP_USERDATA));
	tTVPWindowMessage Message;
	Message.LParam = lParam;
	Message.WParam = wParam;
	Message.Msg = message;
	Message.Result = 0;
	if (win && win->InternalDeliverMessageToReceiver(Message))
	{
		// TODO: return Message.result and block
	}
}
#endif

#ifdef __EMSCRIPTEN__
static int sdl_event_watch(void *userdata, SDL_Event *in_event)
{
	SDL_Event event;
	SDL_memcpy(&event, in_event, sizeof(SDL_Event));
#if defined(_WIN32) && defined(KRKRSDL2_USE_WIN32_EVENT_QUEUE)
#else
	if (event.type == NativeEventQueueImplement::native_event_queue_custom_event_type)
	{
		return 1;
	}
#endif
	if (_currentWindowWindow && _currentWindowWindow->window_receive_event_input(event) && TVPSystemControl)
	{
		// process events now
		// Some JS functions will only work in e.g. mouse down callback due to browser restrictions
		TVPSystemControl->ApplicationIdle();
	}
	return 1;
}
#endif

#ifdef __EMSCRIPTEN__
static void process_events()
#else
static bool process_events()
#endif
{
#ifdef __EMSCRIPTEN__
	double __krkr_process_start = emscripten_get_now();
	static double __krkr_last_process_start = 0.0;
	double __krkr_process_gap = __krkr_last_process_start > 0.0 ?
		(__krkr_process_start - __krkr_last_process_start) : 0.0;
	__krkr_last_process_start = __krkr_process_start;
#endif
  try {
	try
		{
			try
			{
#ifdef __EMSCRIPTEN__
#ifdef __EMSCRIPTEN_PTHREADS__
				NativeEventQueueImplement::DrainNativeEventQueue();
#endif
				tTJSNI_WaveSoundBuffer::Trigger();
#ifndef __EMSCRIPTEN_PTHREADS__
				tTVPTimerThread::Trigger();
#endif
#if KRKRSDL2_FFMPEG_ENABLED
				extern void TVPTickVideoOverlays();
				TVPTickVideoOverlays();
#endif
#endif
			::Application->Run();
#ifdef __EMSCRIPTEN__
			TVPLogMotionHostBurstTrace("process",
				__krkr_process_gap,
				emscripten_get_now() - __krkr_process_start,
				0.0,
				0.0);
#endif
			if (::Application->IsTarminate())
			{
				TVPSystemUninit();
				if (TVPSystemControl)
				{
					delete TVPSystemControl;
					TVPSystemControl = nullptr;
				}
#ifdef __EMSCRIPTEN__
				emscripten_cancel_main_loop();
#else
				return false;
#endif
			}
		}
		TJS_CONVERT_TO_TJS_EXCEPTION
	}
	TVP_CATCH_AND_SHOW_SCRIPT_EXCEPTION(TJS_W("SDL event processing"));
  } catch(...) {
	fprintf(stderr, "[FATAL] Unhandled exception escaped process_events(), recovering\n");
  }

#ifndef __EMSCRIPTEN__
	return true;
#endif
}

void krkrsdl2_pre_init_platform(void)
{
	TVPTerminateCode = 0;
#ifdef _WIN32
	_set_error_mode(_OUT_TO_STDERR);
#endif
}

void krkrsdl2_set_args(int argc, tjs_char **argv)
{
	_argc = argc;
	_wargv = argv;
}

void krkrsdl2_convert_set_args(int argc, char **argv)
{
	// Initialize log verbosity filter as early as possible so subsequent
	// startup traces respect the requested level.
	TVPLogFilterInitFromArgs(argc, argv);
	_argc = argc;
	_wargv = new tjs_char*[argc];

	for (int i = 0; i < argc; i += 1)
	{
		const char *narg;
#if !defined(__EMSCRIPTEN__) && !defined(__vita__) && !defined(__SWITCH__) && !defined(_WIN32)
		if (!i)
		{
			narg = realpath(argv[i], nullptr);
		}
		else
#endif
		{
			narg = argv[i];
		}
		if (!narg)
		{
			tjs_char *warg_copy = new tjs_char[1];
			warg_copy[0] = '\0';
			_wargv[i] = warg_copy;
			continue;
		}
		std::string v_utf8 = narg;
		tjs_string v_utf16;
		TVPUtf8ToUtf16(v_utf16, v_utf8);
#if !defined(__EMSCRIPTEN__) && !defined(__vita__) && !defined(__SWITCH__) && !defined(_WIN32)
		if (!i)
		{
			free((void*)narg);
		}
#endif
		tjs_char *warg_copy = new tjs_char[v_utf16.length() + 1];
		SDL_memcpy(warg_copy, v_utf16.c_str(), sizeof(tjs_char) * (v_utf16.length()));
		warg_copy[v_utf16.length()] = '\0';
		_wargv[i] = warg_copy;
	}
}

bool krkrsdl2_init_platform(void)
{
#ifdef __SWITCH__
	romfsInit();
	socketInitializeDefault();
	nxlinkStdio();
#endif

	SDL_setenv("VITA_DISABLE_TOUCH_BACK", "1", 1);
	SDL_setenv("DBUS_FATAL_WARNINGS", "0", 0);

#ifdef _WIN32
#ifdef SDL_HINT_AUDIODRIVER
	SDL_SetHintWithPriority(SDL_HINT_AUDIODRIVER, "directsound", SDL_HINT_DEFAULT);
#endif
#ifdef SDL_HINT_IME_SHOW_UI
	SDL_SetHintWithPriority(SDL_HINT_IME_SHOW_UI, "1", SDL_HINT_DEFAULT);
#endif
#endif

#ifdef TVP_LOG_TO_COMMANDLINE_CONSOLE
	SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE);
#endif

	TVPLoadMessage();

#ifdef _WIN32
	SDL_SetWindowsMessageHook(sdl_windows_message_hook, nullptr);
#endif
#ifdef __EMSCRIPTEN__
	SDL_AddEventWatch(sdl_event_watch, nullptr);
#endif
	::Application = new tTVPApplication();
	return !!::Application->StartApplication(_argc, _wargv);
}

void krkrsdl2_run_main_loop(void)
{
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(process_events, 0, 1);
#else
	while (process_events());
#endif
}

void krkrsdl2_cleanup(void)
{
	// delete application and exit forcely
	// this prevents ugly exception message on exit
	delete ::Application;
	::Application = nullptr;
}

bool TVPGetKeyMouseAsyncState(tjs_uint keycode, bool getcurrent)
{
	if (keycode >= VK_LBUTTON && keycode <= VK_XBUTTON2 && keycode != VK_CANCEL)
	{
		Uint32 state = SDL_GetMouseState(nullptr, nullptr);
		switch (keycode)
		{
			case VK_LBUTTON:
				return !!(state & SDL_BUTTON(SDL_BUTTON_LEFT));
			case VK_RBUTTON:
				return !!(state & SDL_BUTTON(SDL_BUTTON_RIGHT));
			case VK_MBUTTON:
				return !!(state & SDL_BUTTON(SDL_BUTTON_MIDDLE));
			case VK_XBUTTON1:
				return !!(state & SDL_BUTTON(SDL_BUTTON_X1));
			case VK_XBUTTON2:
				return !!(state & SDL_BUTTON(SDL_BUTTON_X2));
			default:
				return false;
		}
	}
	if (keycode >= VK_SHIFT && keycode <= VK_MENU)
	{
		Uint32 state = SDL_GetModState();
		switch (keycode)
		{
			case VK_SHIFT:
				return !!(state & KMOD_SHIFT);
			case VK_MENU:
				return !!(state & KMOD_ALT);
			case VK_CONTROL:
				return !!(state & KMOD_CTRL);
			default:
				return false;
		}
	}
	const Uint8 *state = SDL_GetKeyboardState(nullptr);
	return !!(state[SDL_GetScancodeFromKey(vk_key_to_sdl_key(keycode))]);
}

bool TVPGetJoyPadAsyncState(tjs_uint keycode, bool getcurrent)
{
	bool is_pressed = false;
	if (sdl_controllers)
	{
		for (int i = 0; i < sdl_controller_num; i += 1)
		{
			if (sdl_controllers[i])
			{
				is_pressed = is_pressed || !!SDL_GameControllerGetButton(sdl_controllers[i], (SDL_GameControllerButton)vk_key_to_sdl_gamecontrollerbutton(keycode));
			}
		}
	}
	return is_pressed;
}

TTVPWindowForm *TVPCreateAndAddWindow(tTJSNI_Window *w)
{
	return new TVPWindowWindow(w);
}

tjs_uint32 TVPGetCurrentShiftKeyState()
{
	tjs_uint32 f = 0;
	if (TVPGetAsyncKeyState(VK_SHIFT)) f |= TVP_SS_SHIFT;
	if (TVPGetAsyncKeyState(VK_MENU)) f |= TVP_SS_ALT;
	if (TVPGetAsyncKeyState(VK_CONTROL)) f |= TVP_SS_CTRL;
	if (TVPGetAsyncKeyState(VK_LBUTTON)) f |= TVP_SS_LEFT;
	if (TVPGetAsyncKeyState(VK_RBUTTON)) f |= TVP_SS_RIGHT;
	if (TVPGetAsyncKeyState(VK_MBUTTON)) f |= TVP_SS_MIDDLE;
	return f;
}

ttstr TVPGetPlatformName()
{
	return SDL_GetPlatform();
}

ttstr TVPGetOSName()
{
	return TVPGetPlatformName();
}

tjs_uint32 TVP_TShiftState_To_uint32(TShiftState state)
{
	tjs_uint32 result = 0;
	if (state & MK_SHIFT) result |= ssShift;
	if (state & MK_CONTROL) result |= ssCtrl;
	if (state & MK_ALT) result |= ssAlt;
	return result;
}
TShiftState TVP_TShiftState_From_uint32(tjs_uint32 state)
{
	TShiftState result = 0;
	if (state & ssShift) result |= MK_SHIFT;
	if (state & ssCtrl) result |= MK_CONTROL;
	if (state & ssAlt) result |= MK_ALT;
	return result;
}

void TVPGetAllFontList(std::vector<tjs_string>& list) {}

const tjs_char *TVPGetDefaultFontName()
{
	if (!TVPGetCommandLine(TJS_W("-deffont"), nullptr))
	{
		TVPSetCommandLine(TJS_W("-deffont"), TJS_W("Noto Sans CJK JP"));
	}
	static tjs_int ArgumentGeneration = 0;
	if (ArgumentGeneration != TVPGetCommandLineArgumentGeneration())
	{
		ArgumentGeneration = TVPGetCommandLineArgumentGeneration();
		// Use that font, if specified on the command line
		tTJSVariant opt;
		if (TVPGetCommandLine(TJS_W("-deffont"), &opt))
		{
			ttstr str(opt);
			TVPDefaultFontName.AssignMessage(str.c_str());
		}
	}
	return TVPDefaultFontName;
}

void TVPSetDefaultFontName(const tjs_char *name)
{
	TVPSetCommandLine(TJS_W("-deffont"), name);
}

static ttstr TVPDefaultFaceNames;
const ttstr &TVPGetDefaultFaceNames()
{
	static tjs_int ArgumentGeneration = 0;
	if (ArgumentGeneration != TVPGetCommandLineArgumentGeneration())
	{
		ArgumentGeneration = TVPGetCommandLineArgumentGeneration();
		TVPDefaultFaceNames = TJS_W("");
	}
	if (TVPDefaultFaceNames.IsEmpty())
	{
		TVPDefaultFaceNames = ttstr(TVPGetDefaultFontName());
	}
	return TVPDefaultFaceNames;
}

#if defined(__vita__)
#define KRKRSDL2_OVERRIDE_NEW_ALLOCATOR_FUNCTIONS
#endif

// Override allocation functions by removing the std::bad_alloc throw and doing garbage collection.
#ifdef KRKRSDL2_OVERRIDE_NEW_ALLOCATOR_FUNCTIONS
void *operator new(std::size_t size) noexcept
{
	bool has_gced = false;

	if (!size)
		size = 1;
	void *p;
	while ((p = ::malloc(size)) == nullptr)
	{
		// If malloc fails, try to free up memory.
		if (has_gced)
			break;
		else
		{
			TVPDeliverCompactEvent(TVP_COMPACT_LEVEL_MAX);
			has_gced = true;
		}
	}
	return p;
}

void *operator new(size_t size, const std::nothrow_t&) noexcept
{
	void *p = 0;
	p = ::operator new(size);
	return p;
}

void *operator new[](size_t size) noexcept
{
	return ::operator new(size);
}

void *operator new[](size_t size, const std::nothrow_t&) noexcept
{
	void *p = 0;
	p = ::operator new[](size);
	return p;
}

void operator delete(void *ptr) noexcept
{
	::free(ptr);
}

void operator delete(void *ptr, const std::nothrow_t&) noexcept
{
	::operator delete(ptr);
}

void operator delete(void *ptr, size_t) noexcept
{
	::operator delete(ptr);
}

void operator delete[] (void *ptr) noexcept
{
	::operator delete(ptr);
}

void operator delete[] (void *ptr, const std::nothrow_t&) noexcept
{
	::operator delete[](ptr);
}

void operator delete[] (void *ptr, size_t) noexcept
{
	::operator delete[](ptr);
}

#ifdef __cpp_aligned_new
void *operator new(std::size_t size, std::align_val_t alignment) noexcept
{
	bool has_gced = false;

	if (!size)
		size = 1;
	if (static_cast<size_t>(alignment) < sizeof(void*))
		alignment = std::align_val_t(sizeof(void*));
	void *p;
	while (::posix_memalign(&p, static_cast<size_t>(alignment), size))
	{
		// If posix_memalign fails, try to free up memory.
		if (has_gced)
		{
			p = nullptr; // posix_memalign doesn't initialize 'p' on failure
			break;
		}
		else
		{
			TVPDeliverCompactEvent(TVP_COMPACT_LEVEL_MAX);
			has_gced = true;
		}
	}
	return p;
}

void *operator new(size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept
{
	void *p = nullptr;
	p = ::operator new(size, alignment);
	return p;
}

void *operator new[](size_t size, std::align_val_t alignment) noexcept
{
	return ::operator new(size, alignment);
}

void *operator new[](size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept
{
	void *p = nullptr;
	p = ::operator new[](size, alignment);
	return p;
}

void operator delete(void *ptr, std::align_val_t) noexcept
{
	::free(ptr);
}

void operator delete(void *ptr, std::align_val_t alignment, const std::nothrow_t&) noexcept
{
	::operator delete(ptr, alignment);
}

void operator delete(void *ptr, size_t, std::align_val_t alignment) noexcept
{
	::operator delete(ptr, alignment);
}

void operator delete[] (void *ptr, std::align_val_t alignment) noexcept
{
	::operator delete(ptr, alignment);
}

void operator delete[] (void *ptr, std::align_val_t alignment, const std::nothrow_t&) noexcept
{
	::operator delete[](ptr, alignment);
}

void operator delete[] (void *ptr, size_t, std::align_val_t alignment) noexcept
{
	::operator delete[](ptr, alignment);
}
#endif

#endif
