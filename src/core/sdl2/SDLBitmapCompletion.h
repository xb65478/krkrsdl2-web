/* SPDX-License-Identifier: MIT */
/* Copyright (c) Kirikiri SDL2 Developers */

#pragma once
#include "tjsCommHead.h"
#include "WindowImpl.h"
#include <SDL.h>
#ifdef __EMSCRIPTEN__
#include <vector>
#endif

class TVPSDLBitmapCompletion
{
public:
	SDL_Surface* surface;
	tTVPRect update_rect;
#ifdef __EMSCRIPTEN__
	std::vector<tTVPRect> update_rects;
	int update_rect_count;
	int update_rect_full_count;
	int update_rect_empty_count;
	int update_rect_union_full;
#endif
	TVPSDLBitmapCompletion();
	~TVPSDLBitmapCompletion();
	void StartUpdateCycle();
	void AddDirtyRect(const tTVPRect &rect);
	void NotifyBitmapCompleted(iTVPLayerManager * manager,
		tjs_int x, tjs_int y, const void * bits, const class BitmapInfomation * bmpinfo,
		const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity);
};
