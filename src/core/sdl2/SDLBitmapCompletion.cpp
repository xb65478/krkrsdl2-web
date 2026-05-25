/* SPDX-License-Identifier: MIT */
/* Copyright (c) Kirikiri SDL2 Developers */

#include "SDLBitmapCompletion.h"
#include "LogFilter.h"
#include "DebugIntf.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

static const size_t KRKR_EMSCRIPTEN_DIRTY_RECT_KEEP_LIMIT = 256;

static bool krkr_dirty_rects_touch_or_overlap(const tTVPRect &a, const tTVPRect &b)
{
	if (a.is_empty() || b.is_empty())
	{
		return false;
	}
	return !(a.right < b.left || b.right < a.left ||
		a.bottom < b.top || b.bottom < a.top);
}

static void krkr_add_dirty_rect(std::vector<tTVPRect> &rects, const tTVPRect &rect)
{
	if (rect.is_empty())
	{
		return;
	}
	tTVPRect merged = rect;
	bool changed = true;
	while (changed)
	{
		changed = false;
		for (std::vector<tTVPRect>::iterator it = rects.begin(); it != rects.end(); ++it)
		{
			if (!krkr_dirty_rects_touch_or_overlap(merged, *it))
			{
				continue;
			}
			merged.do_union(*it);
			rects.erase(it);
			changed = true;
			break;
		}
	}
	if (rects.size() >= KRKR_EMSCRIPTEN_DIRTY_RECT_KEEP_LIMIT)
	{
		if (rects.empty())
		{
			rects.push_back(merged);
		}
		else
		{
			rects[0].do_union(merged);
		}
		return;
	}
	rects.push_back(merged);
}

static void krkr_write_surface_pixel(Uint8 *dst, int bytes_per_pixel, Uint32 pixel)
{
	switch (bytes_per_pixel)
	{
	case 1:
		*dst = (Uint8)pixel;
		break;
	case 2:
		*(Uint16*)dst = (Uint16)pixel;
		break;
	case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		dst[0] = (pixel >> 16) & 0xff;
		dst[1] = (pixel >> 8) & 0xff;
		dst[2] = pixel & 0xff;
#else
		dst[0] = pixel & 0xff;
		dst[1] = (pixel >> 8) & 0xff;
		dst[2] = (pixel >> 16) & 0xff;
#endif
		break;
	case 4:
		*(Uint32*)dst = pixel;
		break;
	}
}

static bool krkr_can_memcpy_tvp_argb_to_surface(SDL_Surface *surface)
{
	SDL_PixelFormat *format = surface ? surface->format : nullptr;
	if (!surface || !format || format->BytesPerPixel != 4)
	{
		return false;
	}
	return format->Rmask == 0x00ff0000 &&
		format->Gmask == 0x0000ff00 &&
		format->Bmask == 0x000000ff &&
		(format->Amask == 0x00000000 || format->Amask == 0xff000000);
}

static void krkr_record_bitmap_complete_perf(SDL_Surface *surface, long x, long y,
	long width, long rows, unsigned long long bytes, bool fast_memcpy, double copy_ms)
{
	SDL_PixelFormat *format = surface ? surface->format : nullptr;
	static unsigned long long call_count = 0;
	static unsigned long long total_bytes = 0;
	static unsigned long long fast_count = 0;
	static unsigned long long slow_count = 0;
	static double total_copy_ms = 0.0;
	static double max_copy_ms = 0.0;
	static double last_summary_ms = 0.0;
	double now_ms = emscripten_get_now();
	call_count++;
	total_bytes += bytes;
	if (fast_memcpy) fast_count++;
	else slow_count++;
	total_copy_ms += copy_ms;
	if (copy_ms > max_copy_ms) max_copy_ms = copy_ms;
	bool print_slow_sample = copy_ms >= 2.0 || (!fast_memcpy && copy_ms >= 0.5);
	bool print_periodic = call_count <= 4 || (call_count % 2000 == 0) ||
		(last_summary_ms > 0.0 && now_ms - last_summary_ms >= 1000.0);
	if (!print_slow_sample && !print_periodic)
	{
		return;
	}
	last_summary_ms = now_ms;
	fprintf(stderr,
		"[PERF] bitmap.complete count=%llu rect=%ld,%ld,%ld,%ld bytes=%llu total_bytes=%llu fast=%llu slow=%llu last_fast=%d last_ms=%.3f total_ms=%.3f max_ms=%.3f surface=%dx%d pitch=%d bpp=%d masks=%08x,%08x,%08x,%08x\n",
		call_count,
		x, y, width, rows,
		bytes,
		total_bytes,
		fast_count,
		slow_count,
		fast_memcpy ? 1 : 0,
		copy_ms,
		total_copy_ms,
		max_copy_ms,
		surface ? surface->w : 0,
		surface ? surface->h : 0,
		surface ? surface->pitch : 0,
		format ? format->BytesPerPixel : 0,
		format ? format->Rmask : 0,
		format ? format->Gmask : 0,
		format ? format->Bmask : 0,
		format ? format->Amask : 0);
}
#endif

TVPSDLBitmapCompletion::TVPSDLBitmapCompletion()
{
	surface = nullptr;
	update_rect.clear();
#ifdef __EMSCRIPTEN__
	update_rect_count = 0;
	update_rect_full_count = 0;
	update_rect_empty_count = 0;
	update_rect_union_full = 0;
	update_rects.clear();
#endif
}

void TVPSDLBitmapCompletion::StartUpdateCycle()
{
	update_rect.clear();
#ifdef __EMSCRIPTEN__
	update_rect_count = 0;
	update_rect_full_count = 0;
	update_rect_empty_count = 0;
	update_rect_union_full = 0;
	update_rects.clear();
#endif
}

void TVPSDLBitmapCompletion::AddDirtyRect(const tTVPRect &rect)
{
#ifdef __EMSCRIPTEN__
	const bool was_empty = update_rect.is_empty();
	const bool rect_empty = rect.is_empty();
	bool rect_full = false;
	if (surface && !rect_empty) {
		rect_full = rect.left <= 0 && rect.top <= 0 &&
			rect.right >= surface->w && rect.bottom >= surface->h;
	}
	if (rect_empty) {
		update_rect_empty_count++;
		return;
	}
	update_rect_count++;
	if (rect_full) update_rect_full_count++;
#else
	if (rect.is_empty()) {
		return;
	}
#endif
	if (update_rect.is_empty()) {
		update_rect = rect;
	} else {
		update_rect.do_union(rect);
	}
#ifdef __EMSCRIPTEN__
	krkr_add_dirty_rect(update_rects, rect);
	if (surface && !update_rect.is_empty() && !rect_full && !was_empty &&
		update_rect.left <= 0 && update_rect.top <= 0 &&
		update_rect.right >= surface->w && update_rect.bottom >= surface->h) {
		update_rect_union_full = 1;
	}
#endif
}

void TVPSDLBitmapCompletion::NotifyBitmapCompleted(iTVPLayerManager * manager,
	tjs_int x, tjs_int y, const void * bits, const class BitmapInfomation * bmpinfo,
	const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity)
{
	if (!surface)
	{
		KRKR_LOG_L3("[NBC] SKIP: surface is null\n");
		return;
	}
	const TVPBITMAPINFO *bitmapinfo = bmpinfo->GetBITMAPINFO();
	tjs_int w = 0;
	tjs_int h = 0;
	if (!manager)
	{
		KRKR_LOG_L3("[NBC] SKIP: manager is null\n");
		return;
	}
	if (!manager->GetPrimaryLayerSize(w, h))
	{
		w = 0;
		h = 0;
	}
	KRKR_LOG_L3("[NBC] Called: dest=(%d,%d) clip=(%d,%d,%d,%d) primaryLayer=%dx%d surface=%dx%d bmp=%dx%d bits=%p\n",
		(int)x, (int)y,
		(int)cliprect.left, (int)cliprect.top, (int)cliprect.right, (int)cliprect.bottom,
		(int)w, (int)h, surface->w, surface->h,
		(int)bitmapinfo->bmiHeader.biWidth, (int)bitmapinfo->bmiHeader.biHeight,
		bits);
	bool boundsOk1 = !(x < 0 || y < 0 ||
			x + cliprect.get_width() > w ||
			y + cliprect.get_height() > h);
	bool boundsOk2 = !(cliprect.left < 0 || cliprect.top < 0 ||
			cliprect.right > bitmapinfo->bmiHeader.biWidth ||
			cliprect.bottom > bitmapinfo->bmiHeader.biHeight);
	if (!boundsOk1 || !boundsOk2) {
		KRKR_LOG_L3("[NBC] BOUNDS REJECT: boundsOk1=%d boundsOk2=%d x+cw=%d>w=%d? y+ch=%d>h=%d? cr=%d>bw=%d? cb=%d>bh=%d?\n",
			boundsOk1, boundsOk2,
			(int)(x + cliprect.get_width()), (int)w,
			(int)(y + cliprect.get_height()), (int)h,
			(int)cliprect.right, (int)bitmapinfo->bmiHeader.biWidth,
			(int)cliprect.bottom, (int)bitmapinfo->bmiHeader.biHeight);
	}
	if(boundsOk1 && boundsOk2)
	{
		// Clamp to actual surface dimensions to prevent buffer overflow
		long surface_w = surface->w;
		long surface_h = surface->h;
		
		// bitmapinfo で表された cliprect の領域を x,y にコピーする
		long src_y       = cliprect.top;
		long src_y_limit = cliprect.bottom;
		long src_x       = cliprect.left;
		long width_bytes   = cliprect.get_width() * sizeof(tjs_uint32); // 32bit
		long dest_y      = y;
		long dest_x      = x;
		
		// Clamp dest region to surface bounds
		if(dest_x < 0) { src_x -= dest_x; dest_x = 0; }
		if(dest_y < 0) { src_y -= dest_y; dest_y = 0; }
		long dest_right = dest_x + cliprect.get_width();
		long dest_bottom = dest_y + (src_y_limit - src_y);
		long clipped_width = cliprect.get_width();
		if(dest_right > surface_w) { clipped_width -= (dest_right - surface_w); }
		if(dest_bottom > surface_h) { src_y_limit -= (dest_bottom - surface_h); }
		if(clipped_width <= 0 || src_y >= src_y_limit) {
			KRKR_LOG_L3("[NBC] CLAMP REJECT: clipped_width=%ld src_y=%ld src_y_limit=%ld\n", clipped_width, src_y, src_y_limit);
			return; // Entirely outside surface
		}
		width_bytes = clipped_width * sizeof(tjs_uint32);
		long dirty_left = dest_x;
		long dirty_top = dest_y;
		long dirty_rows = src_y_limit - src_y;
		const tjs_uint8 * src_p = (const tjs_uint8 *)bits;
		long src_pitch;

		if (bitmapinfo->bmiHeader.biHeight < 0)
		{
			// bottom-down
			src_pitch = bitmapinfo->bmiHeader.biWidth * sizeof(tjs_uint32);
			//src_pitch = -bitmapinfo->bmiHeader.biWidth * sizeof(tjs_uint32);
			//src_p += bitmapinfo->bmiHeader.biWidth * sizeof(tjs_uint32) * (bitmapinfo->bmiHeader.biHeight - 1);
		}
		else
		{
			// bottom-up
			src_pitch = -bitmapinfo->bmiHeader.biWidth * sizeof(tjs_uint32);
			src_p += bitmapinfo->bmiHeader.biWidth * sizeof(tjs_uint32) * (bitmapinfo->bmiHeader.biHeight - 1);
			//src_pitch = bitmapinfo->bmiHeader.biWidth * sizeof(tjs_uint32);
		}

			if (TVPLogL3())
			{
				static int nbc_diag_count = 0;
				nbc_diag_count++;
				if (nbc_diag_count <= 5 || nbc_diag_count % 50 == 0) {
				const tjs_uint32 *src_sample = (const tjs_uint32*)(src_p + src_pitch * src_y + src_x * sizeof(tjs_uint32));
				int sample_count = clipped_width < 8 ? (int)clipped_width : 8;
				KRKR_LOG_L3("[NBC] SRC BITS #%d: biHeight=%d src_pitch=%ld src_y=%ld src_x=%ld first%dpx=[",
					nbc_diag_count, (int)bitmapinfo->bmiHeader.biHeight, src_pitch, src_y, src_x, sample_count);
				for (int i = 0; i < sample_count; i++) {
					fprintf(stderr, "%s%08x", i > 0 ? "," : "", src_sample[i]);
				}
				fprintf(stderr, "] type=%d opacity=%d\n", (int)type, (int)opacity);
			}
		}

			if (surface)
			{
#ifdef __EMSCRIPTEN__
				bool perf_enabled = TVPPerfEnabled();
				double perf_start_ms = perf_enabled ? emscripten_get_now() : 0.0;
				bool fast_memcpy = krkr_can_memcpy_tvp_argb_to_surface(surface);
				unsigned long long total_bytes = (unsigned long long)width_bytes * (unsigned long long)dirty_rows;
#endif
				SDL_LockSurface(surface);
				for (; src_y < src_y_limit; src_y++, dest_y++)
				{
					const void *srcp = src_p + src_pitch * src_y + src_x * sizeof(tjs_uint32);
					void *destp = (tjs_uint8*)surface->pixels + surface->pitch * dest_y + dest_x * sizeof(tjs_uint32);
#ifdef __EMSCRIPTEN__
					if (fast_memcpy)
					{
						SDL_memcpy(destp, srcp, width_bytes);
					}
					else
					{
						const tjs_uint32 *src32 = (const tjs_uint32*)srcp;
						SDL_PixelFormat *format = surface->format;
						int bytes_per_pixel = format ? format->BytesPerPixel : 4;
						Uint8 *dst8 = (Uint8*)surface->pixels + surface->pitch * dest_y + dest_x * bytes_per_pixel;
						for (long sx = 0; sx < clipped_width; sx++)
						{
							tjs_uint32 tvp = src32[sx];
							Uint8 r = (tvp >> 16) & 0xff;
							Uint8 g = (tvp >> 8) & 0xff;
							Uint8 b = tvp & 0xff;
							Uint8 a = (tvp >> 24) & 0xff;
							Uint32 mapped = format
								? (format->Amask ? SDL_MapRGBA(format, r, g, b, a) : SDL_MapRGB(format, r, g, b))
								: tvp;
							krkr_write_surface_pixel(dst8 + sx * bytes_per_pixel, bytes_per_pixel, mapped);
						}
					}
#else
					SDL_memcpy(destp, srcp, width_bytes);
#endif
				}
				SDL_UnlockSurface(surface);
	#ifdef __EMSCRIPTEN__
				if (perf_enabled)
				{
					double perf_end_ms = emscripten_get_now();
					krkr_record_bitmap_complete_perf(surface,
						dirty_left, dirty_top, (long)clipped_width, dirty_rows,
						total_bytes,
						fast_memcpy,
						perf_end_ms - perf_start_ms);
				}
	#endif
			// Sample pixels to verify data was written
			if (TVPLogL3())
			{
				tjs_uint32 *px = (tjs_uint32*)surface->pixels;
				int total = surface->w * surface->h;
				int nonzero = 0;
				int sample = total < 1000 ? total : 1000;
				for (int i = 0; i < sample; i++) {
					if (px[i * (total / sample)] != 0) nonzero++;
				}
				KRKR_LOG_L3("[NBC] After copy: surface=%p pixels=%p %dx%d, nonzero=%d/%d, first4px=[%08x,%08x,%08x,%08x]\n",
					(void*)surface, (void*)surface->pixels, surface->w, surface->h,
					nonzero, sample,
					total > 0 ? px[0] : 0, total > 1 ? px[1] : 0,
					total > 2 ? px[2] : 0, total > 3 ? px[3] : 0);
			}
		}
		tTVPRect r;
#ifdef __EMSCRIPTEN__
		r.set_offsets((tjs_int)dirty_left, (tjs_int)dirty_top);
		r.set_size((tjs_int)clipped_width, (tjs_int)dirty_rows);
#else
		r.set_offsets(x, y);
		r.set_size(cliprect.get_width(), cliprect.get_height());
#endif
		AddDirtyRect(r);
	}

}

TVPSDLBitmapCompletion::~TVPSDLBitmapCompletion()
{
}
