#include "tjsCommHead.h"
#include "LogFilter.h"
#include "GraphicsLoaderIntf.h"
#include "MsgIntf.h"
#include "StorageIntf.h"
#include "SysInitIntf.h"
#include "TickCount.h"
#include "psb.hpp"
#include "psb_static_motion_bridge.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <vector>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

extern "C" {
#include "compress.h"
}

struct PsbImageInfo {
	int width;
	int height;
	int align;
	int left;
	int top;
	int originX;
	int originY;
	unsigned char *pixel_data;
	uint32_t pixel_length;
	unsigned char *pal_data = nullptr;
	uint32_t pal_length = 0;
	std::string name;
	std::string keys;
	std::string format;
	bool recoverAlphaFromRGB = false;
	bool rawRGBA = false;
	bool paletteIndexed = false;
	bool originFromOriginFields = false;
};

struct PsbDrawInfo {
	PsbImageInfo image;
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
	double scaleX = 1.0;
	double scaleY = 1.0;
	double opacity = 1.0;
	bool hasQuad = false;
	double quadX[4] = {0.0, 0.0, 0.0, 0.0};
	double quadY[4] = {0.0, 0.0, 0.0, 0.0};
	std::string ref;
};

struct PsbDrawBounds {
	int minX = 0;
	int minY = 0;
	int maxX = 0;
	int maxY = 0;
	bool valid = false;
};

struct PsbStaticTransform {
	double a = 1.0;
	double b = 0.0;
	double c = 0.0;
	double d = 1.0;
	double tx = 0.0;
	double ty = 0.0;
	double x = 0.0;
	double y = 0.0;
	double scaleX = 1.0;
	double scaleY = 1.0;
	double opacity = 1.0;
};

struct PsbFrameState {
	bool hasContent = false;
	bool hasCoord = false;
	int type = 0;
	double frameTime = 0.0;
	std::string src;
	double coordX = 0.0;
	double coordY = 0.0;
	double coordZ = 0.0;
	double angle = 0.0;
	double shearX = 0.0;
	double shearY = 0.0;
	double ox = 0.0;
	double oy = 0.0;
	double scaleX = 1.0;
	double scaleY = 1.0;
	double opacity = 1.0;
	double timeOffset = 0.0;
};

struct PsbStaticMotionHint {
	bool active = false;
	std::string storage;
	std::string objectName;
	std::string motionName;
	double frameTime = 0.0;
};

static PsbStaticMotionHint g_staticMotionHint;
static TVPStaticMotionBridgeSnapshot g_lastStaticMotionBridgeSnapshot;
static std::vector<TVPStaticMotionBridgeSnapshot> g_staticMotionBridgeSnapshotCache;

static int psb_source_row(const PsbImageInfo &c, int y);

#ifdef __EMSCRIPTEN__
static bool psb_gfx_verbose_log_enabled()
{
	static int cached = -1;
	if (cached >= 0) return cached != 0;
	if (!TVPLogL2()) {
		cached = 0;
		return false;
	}
	bool enabled = TVPGetCommandLine(TJS_W("-krkr-psb-gfx-verbose-log"));
	if (!enabled) {
		enabled = MAIN_THREAD_EM_ASM_INT({
			var enabled = false;
			try {
				var params = new URLSearchParams(location.search || "");
				enabled = params.has("psb_gfx_verbose") ||
					params.get("psb_gfx_verbose") === "1" ||
					params.get("psb_gfx_verbose") === "true" ||
					localStorage.getItem("krkr_psb_gfx_verbose_log") === "1";
			} catch (e) {}
			if (globalThis.__krkrPsbGfxVerboseLog === 1 ||
				globalThis.__krkrPsbGfxVerboseLog === "1" ||
				globalThis.__krkrPsbGfxVerboseLog === true) {
				enabled = true;
			}
			return enabled ? 1 : 0;
		}) != 0;
	}
	cached = enabled ? 1 : 0;
	return enabled;
}
#endif

const TVPStaticMotionBridgeSnapshot *TVPGetLastPSBStaticMotionBridgeSnapshot()
{
	return &g_lastStaticMotionBridgeSnapshot;
}

static std::string psb_snapshot_lower_string(std::string value)
{
	std::transform(value.begin(), value.end(), value.begin(),
		[](unsigned char c) { return (char)std::tolower(c); });
	return value;
}

static std::string psb_snapshot_key(const std::string &storage,
	const std::string &objectName, const std::string &motionName)
{
	return psb_snapshot_lower_string(storage) + "|" +
		psb_snapshot_lower_string(objectName) + "|" +
		psb_snapshot_lower_string(motionName);
}

const TVPStaticMotionBridgeSnapshot *TVPFindPSBStaticMotionBridgeSnapshot(
	const char *storage, const char *objectName, const char *motionName)
{
	std::string key = psb_snapshot_key(
		storage ? storage : "",
		objectName ? objectName : "",
		motionName ? motionName : "");
	for (size_t i = 0; i < g_staticMotionBridgeSnapshotCache.size(); i++) {
		const TVPStaticMotionBridgeSnapshot &snapshot =
			g_staticMotionBridgeSnapshotCache[i];
		if (psb_snapshot_key(snapshot.storage, snapshot.objectName,
			snapshot.motionName) == key) {
			return &snapshot;
		}
	}
	return nullptr;
}

void TVPClearLastPSBStaticMotionBridgeSnapshot()
{
	g_lastStaticMotionBridgeSnapshot = TVPStaticMotionBridgeSnapshot();
}

static bool psb_draw_bounds(const PsbDrawInfo &draw,
	double &minX, double &minY, double &maxX, double &maxY)
{
	if (draw.opacity <= 0.001) return false;
	if (draw.hasQuad) {
		minX = maxX = draw.quadX[0];
		minY = maxY = draw.quadY[0];
		for (int i = 1; i < 4; i++) {
			minX = std::min(minX, draw.quadX[i]);
			minY = std::min(minY, draw.quadY[i]);
			maxX = std::max(maxX, draw.quadX[i]);
			maxY = std::max(maxY, draw.quadY[i]);
		}
		return true;
	}
	if (draw.scaleX <= 0.0 || draw.scaleY <= 0.0) return false;
	minX = draw.x;
	minY = draw.y;
	maxX = draw.x + draw.image.width * draw.scaleX;
	maxY = draw.y + draw.image.height * draw.scaleY;
	return true;
}

static bool psb_bounds_from_draws(const std::vector<PsbDrawInfo> &draws, PsbDrawBounds &bounds)
{
	bounds = PsbDrawBounds();
	for (size_t i = 0; i < draws.size(); i++) {
		const PsbDrawInfo &d = draws[i];
		double drawMinX = 0.0;
		double drawMinY = 0.0;
		double drawMaxX = 0.0;
		double drawMaxY = 0.0;
		if (!psb_draw_bounds(d, drawMinX, drawMinY, drawMaxX, drawMaxY)) continue;
		int left = (int)std::floor(drawMinX);
		int top = (int)std::floor(drawMinY);
		int right = (int)std::ceil(drawMaxX);
		int bottom = (int)std::ceil(drawMaxY);
		if (!bounds.valid) {
			bounds.minX = left;
			bounds.minY = top;
			bounds.maxX = right;
			bounds.maxY = bottom;
			bounds.valid = true;
		} else {
			bounds.minX = std::min(bounds.minX, left);
			bounds.minY = std::min(bounds.minY, top);
			bounds.maxX = std::max(bounds.maxX, right);
			bounds.maxY = std::max(bounds.maxY, bottom);
		}
	}
	return bounds.valid;
}

static std::string psb_hint_string(const tjs_char *value)
{
	return value ? ttstr(value).AsNarrowStdString() : std::string();
}

static std::string psb_lower_string(std::string value)
{
	std::transform(value.begin(), value.end(), value.begin(),
		[](unsigned char c) { return (char)std::tolower(c); });
	return value;
}

static bool psb_static_hint_prefers_final_frame()
{
	if (!g_staticMotionHint.active) return false;
	std::string motion = psb_lower_string(g_staticMotionHint.motionName);
	return motion == "show" || motion == "show2" ||
		motion == "menubar_on" || motion == "menubar";
}

static bool psb_should_capture_static_motion_bridge_snapshot()
{
	if (!g_staticMotionHint.active) return false;
	std::string storage = psb_lower_string(g_staticMotionHint.storage);
	return storage.find(".psb") != std::string::npos;
}

static double psb_lerp(double a, double b, double t)
{
	return a + (b - a) * t;
}

static void psb_apply_point(const PsbStaticTransform &m,
	double x, double y, double &outX, double &outY)
{
	outX = m.a * x + m.c * y + m.tx;
	outY = m.b * x + m.d * y + m.ty;
}

static PsbStaticTransform psb_transform_multiply(
	const PsbStaticTransform &parent,
	const PsbStaticTransform &child)
{
	PsbStaticTransform out;
	out.a = parent.a * child.a + parent.c * child.b;
	out.b = parent.b * child.a + parent.d * child.b;
	out.c = parent.a * child.c + parent.c * child.d;
	out.d = parent.b * child.c + parent.d * child.d;
	out.tx = parent.a * child.tx + parent.c * child.ty + parent.tx;
	out.ty = parent.b * child.tx + parent.d * child.ty + parent.ty;
	out.x = out.tx;
	out.y = out.ty;
	out.scaleX = std::sqrt(out.a * out.a + out.b * out.b);
	out.scaleY = std::sqrt(out.c * out.c + out.d * out.d);
	out.opacity = parent.opacity * child.opacity;
	return out;
}

static PsbStaticTransform psb_transform_translate(double x, double y)
{
	PsbStaticTransform out;
	out.tx = x;
	out.ty = y;
	out.x = x;
	out.y = y;
	return out;
}

static PsbStaticTransform psb_transform_scale(double sx, double sy)
{
	PsbStaticTransform out;
	out.a = sx;
	out.d = sy;
	out.scaleX = sx;
	out.scaleY = sy;
	return out;
}

static PsbStaticTransform psb_transform_rotate(double degrees)
{
	PsbStaticTransform out;
	double radians = degrees * 3.14159265358979323846 / 180.0;
	double cs = std::cos(radians);
	double sn = std::sin(radians);
	out.a = cs;
	out.b = sn;
	out.c = -sn;
	out.d = cs;
	return out;
}

static PsbStaticTransform psb_transform_shear(double sx, double sy)
{
	PsbStaticTransform out;
	out.a = 1.0;
	out.b = sy;
	out.c = sx;
	out.d = 1.0;
	return out;
}

static void psb_capture_static_motion_bridge_item(
	TVPStaticMotionBridgeSnapshot &snapshot,
	const PsbDrawInfo &draw, const PsbImageInfo &image,
	const unsigned char *pixels, double opacity, int dstX, int dstY)
{
	if (!pixels || image.width <= 0 || image.height <= 0) return;
	TVPStaticMotionBridgeItem item;
	item.ref = draw.ref.empty() ? image.name : draw.ref;
	item.dstX = dstX;
	item.dstY = dstY;
	item.width = image.width;
	item.height = image.height;
	item.scaleX = draw.scaleX;
	item.scaleY = draw.scaleY;
	item.opacity = opacity;
	item.hasQuad = draw.hasQuad;
	for (int i = 0; i < 4; i++) {
		item.quadX[i] = draw.quadX[i] - snapshot.offsetX;
		item.quadY[i] = draw.quadY[i] - snapshot.offsetY;
	}
	size_t pixelBytes = (size_t)image.width * (size_t)image.height * 4u;
	item.pixelsBGRA.resize(pixelBytes, 0);
	for (int sy = 0; sy < image.height; sy++) {
		int srcRow = psb_source_row(image, sy);
		for (int sx = 0; sx < image.width; sx++) {
			const unsigned char *sp = pixels + ((size_t)srcRow * (size_t)image.width + (size_t)sx) * 4u;
			unsigned char sb = image.rawRGBA ? sp[2] : sp[0];
			unsigned char sg = sp[1];
			unsigned char sr = image.rawRGBA ? sp[0] : sp[2];
			unsigned char sa = sp[3];
			if (image.recoverAlphaFromRGB && sa == 0 && (sb | sg | sr)) sa = 255;
			unsigned char *dp = item.pixelsBGRA.data() +
				((size_t)sy * (size_t)image.width + (size_t)sx) * 4u;
			dp[0] = sb;
			dp[1] = sg;
			dp[2] = sr;
			dp[3] = sa;
		}
	}
	snapshot.items.push_back(item);
}

static bool psb_snapshot_has_ref(const TVPStaticMotionBridgeSnapshot &snapshot,
	const std::string &ref)
{
	for (size_t i = 0; i < snapshot.items.size(); i++) {
		if (snapshot.items[i].ref == ref) return true;
	}
	return false;
}

void TVPSetPSBStaticMotionHint(const tjs_char *storage, const tjs_char *objectName,
	const tjs_char *motionName, double frameTime)
{
	g_staticMotionHint.active = true;
	g_staticMotionHint.storage = psb_hint_string(storage);
	g_staticMotionHint.objectName = psb_hint_string(objectName);
	g_staticMotionHint.motionName = psb_hint_string(motionName);
	g_staticMotionHint.frameTime = frameTime;
#ifdef __EMSCRIPTEN__
	if (psb_gfx_verbose_log_enabled() &&
		(g_staticMotionHint.storage.find("title.psb") != std::string::npos ||
		 g_staticMotionHint.storage.find("autoskip.psb") != std::string::npos ||
		 g_staticMotionHint.storage.find("m2logo.psb") != std::string::npos)) {
		emscripten_log(EM_LOG_CONSOLE,
			"[PSB-GFX] static motion hint received %s/%s for %s frame=%.3f",
			g_staticMotionHint.objectName.c_str(),
			g_staticMotionHint.motionName.c_str(),
			g_staticMotionHint.storage.c_str(),
			g_staticMotionHint.frameTime);
	}
#endif
}

void TVPClearPSBStaticMotionHint()
{
	g_staticMotionHint = PsbStaticMotionHint();
}

struct PsbNodeEntry {
	unsigned char *data = nullptr;
	int parent = -1;
};

static bool psb_should_recover_alpha_from_rgb(const unsigned char *pixels, int width, int height)
{
	if (!pixels || width <= 0 || height <= 0) return false;
	tjs_int64 totalPixels = (tjs_int64)width * (tjs_int64)height;
	tjs_int64 nonZeroRgbCount = 0;
	for (int y = 0; y < height; y++) {
		const unsigned char *row = pixels + (size_t)y * width * 4;
		for (int x = 0; x < width; x++) {
			const unsigned char *p = row + x * 4;
			if (p[3] != 0) return false;
			if (p[0] | p[1] | p[2]) nonZeroRgbCount++;
		}
	}
	tjs_real ratio = (tjs_real)nonZeroRgbCount / (tjs_real)totalPixels;
	return ratio >= 0.40;
}

static int psb_source_row(const PsbImageInfo &c, int y)
{
	(void)c;
	return y;
}

static const unsigned char *psb_palette_color(const unsigned char *pal, uint32_t palLength, unsigned char index)
{
	uint32_t entryOffset = (uint32_t)index * 4u;
	if (entryOffset + 4u <= palLength) return pal + entryOffset;
	if ((uint32_t)index + 4u <= palLength) return pal + index;
	return nullptr;
}

static bool psb_palette_uncompress(const PsbImageInfo &c, unsigned char *out, uint32_t outLength)
{
	if (!c.pixel_data || !c.pal_data || !out || c.pal_length < 4) return false;

	uint32_t srcPos = 0;
	uint32_t dstPos = 0;
	while (srcPos < c.pixel_length && dstPos + 4 <= outLength) {
		unsigned char cmd = c.pixel_data[srcPos++];
		int count = 0;
		if ((cmd & 0x80) != 0) {
			count = (cmd ^ 0x80) + 3;
			if (srcPos >= c.pixel_length) return false;
			const unsigned char *color = psb_palette_color(c.pal_data, c.pal_length, c.pixel_data[srcPos++]);
			if (!color) return false;
			for (int i = 0; i < count && dstPos + 4 <= outLength; i++) {
				memcpy(out + dstPos, color, 4);
				dstPos += 4;
			}
		} else {
			count = cmd + 1;
			for (int i = 0; i < count && dstPos + 4 <= outLength; i++) {
				if (srcPos >= c.pixel_length) return false;
				const unsigned char *color = psb_palette_color(c.pal_data, c.pal_length, c.pixel_data[srcPos++]);
				if (!color) return false;
				memcpy(out + dstPos, color, 4);
				dstPos += 4;
			}
		}
	}
	return dstPos == outLength;
}

#ifdef __EMSCRIPTEN__
static std::string psb_json_escape(const std::string &value)
{
	std::string out;
	out.reserve(value.size() + 8);
	for (size_t i = 0; i < value.size(); i++) {
		unsigned char ch = (unsigned char)value[i];
		switch (ch) {
		case '\\': out += "\\\\"; break;
		case '"': out += "\\\""; break;
		case '\b': out += "\\b"; break;
		case '\f': out += "\\f"; break;
		case '\n': out += "\\n"; break;
		case '\r': out += "\\r"; break;
		case '\t': out += "\\t"; break;
		default:
			if (ch < 0x20) {
				char buf[8];
				snprintf(buf, sizeof(buf), "\\u%04x", ch);
				out += buf;
			} else {
				out += value[i];
			}
			break;
		}
	}
	return out;
}

static bool psb_dump_enabled()
{
	return MAIN_THREAD_EM_ASM_INT({
		if (typeof globalThis.__krkr_psb_dump_enabled === "undefined") {
			var enabled = false;
			try {
				var params = new URLSearchParams(location.search || "");
				enabled = params.has("psb_dump") ||
					params.get("psb_dump") === "1" ||
					params.get("psb_dump") === "true" ||
					localStorage.getItem("krkr_psb_dump") === "1";
			} catch (e) {}
			if (globalThis.__krkr_psb_dump === true) enabled = true;
			globalThis.__krkr_psb_dump_enabled = !!enabled;
		}
		return globalThis.__krkr_psb_dump_enabled ? 1 : 0;
	}) != 0;
}

static void psb_append_sample_json(std::string &json, const char *label, int x, int y, const unsigned char *pixels, int width)
{
	const unsigned char *p = pixels + ((size_t)y * width + x) * 4;
	char buf[160];
	snprintf(buf, sizeof(buf),
		"%s{\"label\":\"%s\",\"x\":%d,\"y\":%d,\"bytes\":[%u,%u,%u,%u]}",
		json.empty() ? "" : ",",
		label, x, y,
		(unsigned)p[0], (unsigned)p[1], (unsigned)p[2], (unsigned)p[3]);
	json += buf;
}

static std::string psb_make_samples_json(const unsigned char *pixels, int width, int height)
{
	std::string json;
	if (!pixels || width <= 0 || height <= 0) return json;
	psb_append_sample_json(json, "top-left", 0, 0, pixels, width);
	psb_append_sample_json(json, "center", width / 2, height / 2, pixels, width);
	psb_append_sample_json(json, "bottom-right", width - 1, height - 1, pixels, width);
	return json;
}

static std::string psb_make_chunk_meta_json(const PsbImageInfo &c, int dstX, int dstY, const unsigned char *decodedPixels)
{
	std::string samples = psb_make_samples_json(decodedPixels, c.width, c.height);
	char buf[768];
	snprintf(buf, sizeof(buf),
		"{\"format\":\"%s\",\"width\":%d,\"height\":%d,\"left\":%d,\"top\":%d,"
		"\"originX\":%d,\"originY\":%d,\"dstX\":%d,\"dstY\":%d,"
		"\"rawBytes\":%u,\"orientation\":\"top-down\",\"recoverAlphaFromRGB\":%s,\"samples\":[",
		psb_json_escape(c.format).c_str(), c.width, c.height, c.left, c.top,
		c.originX, c.originY, dstX, dstY, (unsigned)c.pixel_length,
		c.recoverAlphaFromRGB ? "true" : "false");
	std::string json = buf;
	json += samples;
	json += "],\"keys\":\"";
	json += psb_json_escape(c.keys);
	json += "\"}";
	return json;
}

static std::string psb_make_composite_meta_json(int chunkCount, int width, int height, int offsetX, int offsetY)
{
	char buf[256];
	snprintf(buf, sizeof(buf),
		"{\"format\":\"compositeBGRA\",\"chunkCount\":%d,\"width\":%d,\"height\":%d,"
		"\"offsetX\":%d,\"offsetY\":%d}",
		chunkCount, width, height, offsetX, offsetY);
	return std::string(buf);
}

static unsigned char *psb_make_chunk_rgba_dump(const PsbImageInfo &c, const unsigned char *pixels)
{
	uint32_t chunkBytes = (uint32_t)c.width * (uint32_t)c.height * 4;
	unsigned char *rgba = new unsigned char[chunkBytes];
	for (int sy = 0; sy < c.height; sy++) {
		int srcRow = psb_source_row(c, sy);
		for (int sx = 0; sx < c.width; sx++) {
			const unsigned char *sp = pixels + ((size_t)srcRow * c.width + sx) * 4;
			unsigned char r = c.rawRGBA ? sp[0] : sp[2];
			unsigned char g = sp[1];
			unsigned char b = c.rawRGBA ? sp[2] : sp[0];
			unsigned char a = sp[3];
			if (c.recoverAlphaFromRGB && a == 0 && (r | g | b)) {
				a = 255;
			}
			unsigned char *dp = rgba + ((size_t)sy * c.width + sx) * 4;
			dp[0] = r;
			dp[1] = g;
			dp[2] = b;
			dp[3] = a;
		}
	}
	return rgba;
}

static unsigned char *psb_make_bgra_canvas_rgba_dump(const unsigned char *canvas, int width, int height)
{
	uint32_t bytes = (uint32_t)width * (uint32_t)height * 4;
	unsigned char *rgba = new unsigned char[bytes];
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			const unsigned char *sp = canvas + ((size_t)y * width + x) * 4;
			unsigned char *dp = rgba + ((size_t)y * width + x) * 4;
			dp[0] = sp[2];
			dp[1] = sp[1];
			dp[2] = sp[0];
			dp[3] = sp[3];
		}
	}
	return rgba;
}

static void psb_dump_browser_image(const char *kind, const char *name,
	const unsigned char *rgba, int width, int height,
	const unsigned char *rawBytes, uint32_t rawLength,
	const std::string &metaJson)
{
	if (!rgba || width <= 0 || height <= 0) return;
	MAIN_THREAD_EM_ASM({
		var kind = UTF8ToString($0);
		var name = UTF8ToString($1);
		var ptr = $2;
		var width = $3;
		var height = $4;
		var rawPtr = $5;
		var rawLength = $6 >>> 0;
		var metaText = UTF8ToString($7);
		var meta = {};
		try {
			meta = JSON.parse(metaText);
		} catch (e) {
			meta = {};
			meta.parseError = e.message;
			meta.raw = metaText;
		}
		var dump = globalThis.__krkr_psb_dumps;
		if (!dump) {
			dump = globalThis.__krkr_psb_dumps = {};
			dump.enabled = true;
			dump.items = [];
			dump.byName = Object.create(null);
			dump.nextId = 1;
			dump.createdAt = Date.now();
		}
		var maxItems = 0;
		try {
			var params = new URLSearchParams(location.search || "");
			maxItems = parseInt(params.get("psb_dump_max") || "0", 10) || 0;
		} catch (e) {}
		if (maxItems > 0 && dump.items.length >= maxItems) {
			if (!dump.truncated) {
				dump.truncated = true;
				console.warn("[PSB-DUMP] psb_dump_max reached; further dumps are skipped");
			}
			return;
		}
		var total = width * height * 4;
		var imageBytes = new Uint8ClampedArray(HEAPU8.subarray(ptr, ptr + total));
		var dataUrl = null;
		try {
			var canvas = document.createElement("canvas");
			canvas.width = width;
			canvas.height = height;
			var ctx = canvas.getContext("2d");
			ctx.putImageData(new ImageData(imageBytes, width, height), 0, 0);
			dataUrl = canvas.toDataURL("image/png");
		} catch (e) {
			console.error("[PSB-DUMP] PNG encode failed:", e);
		}
		var rawBytesUrl = null;
		var rawBytesOmitted = false;
		var maxRawBytes = 32 * 1024 * 1024;
		if (rawPtr && rawLength > 0) {
			if (rawLength <= maxRawBytes) {
				try {
					var rawCopy = HEAPU8.slice(rawPtr, rawPtr + rawLength);
					rawBytesUrl = URL.createObjectURL(new Blob([rawCopy], { type: "application/octet-stream" }));
				} catch (e) {
					rawBytesOmitted = true;
					console.warn("[PSB-DUMP] raw byte capture failed:", e);
				}
			} else {
				rawBytesOmitted = true;
			}
		}
		var item = {};
		item.id = dump.nextId++;
		item.kind = kind;
		item.name = name;
		item.width = width;
		item.height = height;
		item.meta = meta;
		item.dataUrl = dataUrl;
		item.rawBytesUrl = rawBytesUrl;
		item.rawBytesLength = rawLength;
		item.rawBytesOmitted = rawBytesOmitted;
		item.createdAt = Date.now();
		dump.items.push(item);
		if (!dump.byName[name]) dump.byName[name] = [];
		dump.byName[name].push(item);
		console.error("[PSB-DUMP] " + kind + " #" + item.id + " " + name + " " + width + "x" + height, meta);
		if (typeof globalThis.__krkr_psb_dump_update === "function") {
			try { globalThis.__krkr_psb_dump_update(item, dump); } catch (e) { console.error("[PSB-DUMP] panel update failed:", e); }
		}
	}, kind, name, rgba, width, height, rawBytes, rawLength, metaJson.c_str());
}

static void psb_dump_chunk_to_browser(const PsbImageInfo &c, const unsigned char *pixels, int dstX, int dstY)
{
	if (!psb_dump_enabled()) return;
	std::string meta = psb_make_chunk_meta_json(c, dstX, dstY, pixels);
	unsigned char *rgba = psb_make_chunk_rgba_dump(c, pixels);
	psb_dump_browser_image("chunk", c.name.c_str(), rgba, c.width, c.height,
		c.pixel_data, c.pixel_length, meta);
	delete[] rgba;
	if (psb_gfx_verbose_log_enabled()) {
		emscripten_log(EM_LOG_CONSOLE,
			"[PSB-GFX] dump chunk name=%s format=%s size=%dx%d left/top=(%d,%d) origin=(%d,%d) dst=(%d,%d) rawBytes=%u keys=[%s]",
			c.name.c_str(), c.format.c_str(), c.width, c.height,
			c.left, c.top, c.originX, c.originY, dstX, dstY,
			(unsigned)c.pixel_length, c.keys.c_str());
	}
}

static void psb_dump_composite_to_browser(const unsigned char *canvas, int width, int height, int offsetX, int offsetY, int chunkCount)
{
	if (!psb_dump_enabled()) return;
	std::string meta = psb_make_composite_meta_json(chunkCount, width, height, offsetX, offsetY);
	unsigned char *rgba = psb_make_bgra_canvas_rgba_dump(canvas, width, height);
	psb_dump_browser_image("composite", "composite", rgba, width, height,
		nullptr, 0, meta);
	delete[] rgba;
}
#endif

static std::string psb_key_list(const psb_objects_t *obj)
{
	std::string keys;
	if (!obj) return keys;
	for (uint32_t k = 0; k < obj->size(); k++) {
		if (!keys.empty()) keys += ",";
		keys += obj->get_name(k);
	}
	return keys;
}

static bool psb_is_number_type(psb_value_t *v)
{
	if (!v) return false;
	psb_value_t::type_t t = v->get_type();
	return (t >= psb_value_t::TYPE_NUMBER_N0 && t <= psb_value_t::TYPE_NUMBER_N8) ||
		t == psb_value_t::TYPE_FLOAT0 ||
		t == psb_value_t::TYPE_FLOAT ||
		t == psb_value_t::TYPE_DOUBLE;
}

static bool psb_is_string_type(psb_value_t *v)
{
	if (!v) return false;
	psb_value_t::type_t t = v->get_type();
	return t >= psb_value_t::TYPE_STRING_N1 && t <= psb_value_t::TYPE_STRING_N4;
}

static bool psb_is_resource_type(psb_value_t *v)
{
	if (!v) return false;
	psb_value_t::type_t t = v->get_type();
	return t >= psb_value_t::TYPE_RESOURCE_N1 && t <= psb_value_t::TYPE_RESOURCE_N4;
}

static bool psb_read_number_value(psb_t &psb, unsigned char *data, double &out)
{
	if (!data) return false;
	unsigned char *p = data;
	psb_value_t *v = psb.unpack(p);
	if (!psb_is_number_type(v)) {
		delete v;
		return false;
	}
	psb_number_t *n = static_cast<psb_number_t*>(v);
	switch (v->get_type()) {
	case psb_value_t::TYPE_FLOAT0:
	case psb_value_t::TYPE_FLOAT:
		out = n->get_float();
		break;
	case psb_value_t::TYPE_DOUBLE:
		out = n->get_double();
		break;
	default:
		out = (double)n->get_integer();
		break;
	}
	delete v;
	return true;
}

static void psb_capture_source_textures_for_bridge(
	TVPStaticMotionBridgeSnapshot &snapshot,
	std::vector<PsbImageInfo> &candidates)
{
	for (size_t i = 0; i < candidates.size(); i++) {
		PsbImageInfo &c = candidates[i];
		if (c.width <= 0 || c.height <= 0 || psb_snapshot_has_ref(snapshot, c.name)) {
			continue;
		}
		uint32_t chunkBytes = (uint32_t)c.width * (uint32_t)c.height * 4u;
		unsigned char *pixels = nullptr;
		if (c.rawRGBA) {
			pixels = c.pixel_data;
		} else {
			pixels = new unsigned char[chunkBytes];
			memset(pixels, 0, chunkBytes);
			if (c.paletteIndexed) {
				psb_palette_uncompress(c, pixels, chunkBytes);
			} else {
				psb_pixel_uncompress(c.pixel_data, pixels, c.pixel_length, 4);
			}
		}
		c.recoverAlphaFromRGB = psb_should_recover_alpha_from_rgb(
			pixels, c.width, c.height);
		PsbDrawInfo draw;
		draw.image = c;
		draw.ref = c.name;
		draw.opacity = 1.0;
		draw.x = 0.0;
		draw.y = 0.0;
		draw.hasQuad = false;
		psb_capture_static_motion_bridge_item(snapshot, draw, c, pixels, 1.0, 0, 0);
		if (!c.rawRGBA) delete[] pixels;
	}
}

static bool psb_read_int_value(psb_t &psb, unsigned char *data, int &out)
{
	double v = 0.0;
	if (!psb_read_number_value(psb, data, v)) return false;
	out = (int)std::lround(v);
	return true;
}

static bool psb_read_string_value(psb_t &psb, unsigned char *data, std::string &out)
{
	if (!data) return false;
	unsigned char *p = data;
	psb_value_t *v = psb.unpack(p);
	if (!psb_is_string_type(v)) {
		delete v;
		return false;
	}
	out = static_cast<psb_string_t*>(v)->get_string();
	delete v;
	return true;
}

static bool psb_read_resource_value(psb_t &psb, unsigned char *data, unsigned char *&buff, uint32_t &length)
{
	if (!data) return false;
	unsigned char *p = data;
	psb_value_t *v = psb.unpack(p);
	if (!psb_is_resource_type(v)) {
		delete v;
		return false;
	}
	psb_resource_t *res = static_cast<psb_resource_t*>(v);
	buff = res->get_buff();
	length = res->get_length();
	delete v;
	return true;
}

static bool psb_read_member_number(psb_t &psb, const psb_objects_t *obj, const char *key, double &out)
{
	return obj && psb_read_number_value(psb, obj->get_data(key), out);
}

static bool psb_read_member_int(psb_t &psb, const psb_objects_t *obj, const char *key, int &out)
{
	return obj && psb_read_int_value(psb, obj->get_data(key), out);
}

static bool psb_read_member_string(psb_t &psb, const psb_objects_t *obj, const char *key, std::string &out)
{
	return obj && psb_read_string_value(psb, obj->get_data(key), out);
}

static bool psb_read_collection_entries(psb_t &psb, unsigned char *data, std::vector<unsigned char*> &entries)
{
	entries.clear();
	if (!data) return false;
	unsigned char *p = data;
	psb_value_t *v = psb.unpack(p);
	if (!v || v->get_type() != psb_value_t::TYPE_COLLECTION) {
		delete v;
		return false;
	}
	psb_collection_t *list = static_cast<psb_collection_t*>(v);
	for (uint32_t i = 0; i < list->size(); i++) {
		entries.push_back(list->get(i));
	}
	delete v;
	return true;
}

static bool psb_read_member_number_list(psb_t &psb, const psb_objects_t *obj, const char *key, std::vector<double> &out)
{
	out.clear();
	if (!obj) return false;
	std::vector<unsigned char*> entries;
	if (!psb_read_collection_entries(psb, obj->get_data(key), entries)) return false;
	for (size_t i = 0; i < entries.size(); i++) {
		double v = 0.0;
		if (!psb_read_number_value(psb, entries[i], v)) return false;
		out.push_back(v);
	}
	return true;
}

static std::vector<std::string> psb_split_path(const std::string &path)
{
	std::vector<std::string> parts;
	size_t start = 0;
	while (start <= path.size()) {
		size_t slash = path.find('/', start);
		if (slash == std::string::npos) slash = path.size();
		if (slash > start) parts.push_back(path.substr(start, slash - start));
		if (slash == path.size()) break;
		start = slash + 1;
	}
	return parts;
}

static bool psb_string_starts_with(const std::string &s, const char *prefix)
{
	size_t n = strlen(prefix);
	return s.size() >= n && s.compare(0, n, prefix) == 0;
}

static void psb_find_all_images(psb_t &psb, const psb_objects_t *obj, std::vector<PsbImageInfo> &results, const std::string &path, int parentLeft = 0, int parentTop = 0, int parentOx = 0, int parentOy = 0) {

	auto readInt = [&](const char *key) -> int {
		int val = 0;
		psb_read_member_int(psb, obj, key, val);
		return val;
	};

	int myLeft = readInt("left") + parentLeft;
	int myTop = readInt("top") + parentTop;
	int myOx = readInt("ox") + parentOx;
	int myOy = readInt("oy") + parentOy;
	bool originFromOriginFields = false;
	int originX = 0;
	int originY = 0;
	if (psb_read_member_int(psb, obj, "originX", originX)) {
		myOx = originX + parentOx;
		originFromOriginFields = true;
	}
	if (psb_read_member_int(psb, obj, "originY", originY)) {
		myOy = originY + parentOy;
		originFromOriginFields = true;
	}

	unsigned char *p_pixel = obj->get_data("pixel");
	unsigned char *p_compress = obj->get_data("compress");
	unsigned char *p_width = obj->get_data("width");
	unsigned char *p_height = obj->get_data("height");
	unsigned char *p_pal = obj->get_data("pal");
	unsigned char *p_type = obj->get_data("type");

	if (p_pixel && p_width && p_height) {
		psb_value_t *v_compress = p_compress ? psb.unpack(p_compress) : nullptr;
		bool is_rl = false;
		if (v_compress) {
			if (psb_is_string_type(v_compress)) {
				psb_string_t *s = static_cast<psb_string_t*>(v_compress);
				is_rl = (s->get_string() == "RL");
			}
			delete v_compress;
		}
		if (is_rl) {
			PsbImageInfo info = {};
			info.name = path;
			info.keys = psb_key_list(obj);
			info.format = p_pal ? "RL-pal" : "RL";
			psb_read_int_value(psb, p_width, info.width);
			psb_read_int_value(psb, p_height, info.height);

			info.align = p_pal ? 1 : 4;
			info.left = myLeft;
			info.top = myTop;
			info.originX = myOx;
			info.originY = myOy;
			info.originFromOriginFields = originFromOriginFields;
			info.paletteIndexed = p_pal != nullptr;

			if (psb_read_resource_value(psb, p_pixel, info.pixel_data, info.pixel_length)) {
				bool paletteOK = true;
				if (info.paletteIndexed) {
					paletteOK = psb_read_resource_value(psb, p_pal, info.pal_data, info.pal_length) &&
						info.pal_data && info.pal_length >= 4;
				}
				if (info.width > 0 && info.height > 0 && info.pixel_data && paletteOK) {
#ifdef __EMSCRIPTEN__
					if (psb_gfx_verbose_log_enabled()) {
						KRKR_LOG_L3("[PSB-GFX] chunk: %s %dx%d at(%d,%d) format=%s pixel=%u pal=%u keys=[%s]\n",
							path.c_str(), info.width, info.height, info.left, info.top,
							info.format.c_str(), (unsigned)info.pixel_length, (unsigned)info.pal_length,
							info.keys.c_str());
						MAIN_THREAD_EM_ASM({
							console.error("[PSB-GFX] chunk: " + UTF8ToString($0) + " " + $1 + "x" + $2 + " at(" + $3 + "," + $4 + ") format=" + UTF8ToString($5) + " pixel=" + $6 + " pal=" + $7 + " keys=[" + UTF8ToString($8) + "]");
						}, path.c_str(), info.width, info.height, info.left, info.top,
							info.format.c_str(), (unsigned)info.pixel_length, (unsigned)info.pal_length,
							info.keys.c_str());
					}
#endif
					results.push_back(info);
				}
			}
			return;
		}

		std::string imageType;
		psb_value_t *v_type = p_type ? psb.unpack(p_type) : nullptr;
		if (psb_is_string_type(v_type)) {
			imageType = static_cast<psb_string_t*>(v_type)->get_string();
		}
		delete v_type;

		// E-mote "win"/"common" PSB files often store a raw RGBA8 texture
		// atlas under source/*/texture. The motion tree normally places
		// sub-rects from this atlas, but exposing the atlas as a still image is
		// a useful fallback for web builds without the full motion renderer.
		if (imageType == "RGBA8" || (!p_compress && imageType.empty())) {
			PsbImageInfo info = {};
			info.name = path;
			info.keys = psb_key_list(obj);
			info.format = imageType.empty() ? "rawRGBA" : imageType;
			psb_read_int_value(psb, p_width, info.width);
			psb_read_int_value(psb, p_height, info.height);

			info.align = 4;
			info.left = myLeft;
			info.top = myTop;
			info.originX = myOx;
			info.originY = myOy;
			info.originFromOriginFields = originFromOriginFields;
			info.rawRGBA = true;

			if (psb_read_resource_value(psb, p_pixel, info.pixel_data, info.pixel_length)) {
				if (info.width > 0 && info.height > 0 &&
					info.pixel_data && info.pixel_length >= (uint32_t)info.width * info.height * 4) {
#ifdef __EMSCRIPTEN__
					if (psb_gfx_verbose_log_enabled()) {
						KRKR_LOG_L3("[PSB-GFX] raw RGBA texture: %s %dx%d\n",
							path.c_str(), info.width, info.height);
						MAIN_THREAD_EM_ASM({
							console.error("[PSB-GFX] raw RGBA texture: " + UTF8ToString($0) + " " + $1 + "x" + $2);
						}, path.c_str(), info.width, info.height);
					}
#endif
					results.push_back(info);
				}
			}
			return;
		}
	}

	for (uint32_t i = 0; i < obj->size(); i++) {
		unsigned char *data = obj->get_data(i);
		psb_value_t *val = psb.unpack(data);
		if (val && val->get_type() == psb_value_t::TYPE_OBJECTS) {
			psb_objects_t *child = static_cast<psb_objects_t*>(val);
			std::string childpath = path.empty() ? obj->get_name(i) : path + "/" + obj->get_name(i);
			psb_find_all_images(psb, child, results, childpath, myLeft, myTop, myOx, myOy);
		}
		delete val;
	}
}

static void psb_add_source_key(std::map<std::string, PsbImageInfo*> &sourceMap,
	const std::string &key, PsbImageInfo *info)
{
	if (!key.empty() && sourceMap.find(key) == sourceMap.end()) {
		sourceMap[key] = info;
	}
}

static void psb_build_source_map(std::vector<PsbImageInfo> &images,
	std::map<std::string, PsbImageInfo*> &sourceMap)
{
	sourceMap.clear();
	for (size_t i = 0; i < images.size(); i++) {
		PsbImageInfo *info = &images[i];
		psb_add_source_key(sourceMap, info->name, info);
		std::string path = info->name;
		if (psb_string_starts_with(path, "source/")) {
			path = path.substr(7);
			psb_add_source_key(sourceMap, path, info);
		}
		size_t icon = path.find("/icon/");
		if (icon != std::string::npos) {
			std::string src = "src/" + path.substr(0, icon) + "/" + path.substr(icon + 6);
			psb_add_source_key(sourceMap, src, info);
		}
	}
}

static bool psb_parse_content_frame(psb_t &psb, const psb_objects_t *content, PsbFrameState &state)
{
	if (!content) return false;
	state = PsbFrameState();
	state.hasContent = true;
	std::string rawSrc;
	if (psb_read_member_string(psb, content, "src", rawSrc)) {
		if (rawSrc == "blank") {
			state.src.clear();
		} else {
			std::string icon;
			if (psb_read_member_string(psb, content, "icon", icon)) {
				bool isMotion = false;
				unsigned char *motionData = content->get_data("motion");
				if (motionData) {
					isMotion = true;
					unsigned char *p_motion = motionData;
					psb_value_t *v_motion = psb.unpack(p_motion);
					if (v_motion && v_motion->get_type() == psb_value_t::TYPE_OBJECTS) {
						psb_read_member_number(psb,
							static_cast<psb_objects_t*>(v_motion),
							"timeOffset", state.timeOffset);
					}
					delete v_motion;
				}
				state.src = std::string(isMotion ? "motion/" : "src/") +
					rawSrc + "/" + icon;
				state.src.erase(std::remove(state.src.begin(), state.src.end(), '\0'),
					state.src.end());
			} else {
				state.src = rawSrc;
			}
		}
	} else {
		state.src = "layout";
	}
	std::vector<double> coord;
	if (psb_read_member_number_list(psb, content, "coord", coord) && coord.size() >= 2) {
		state.hasCoord = true;
		state.coordX = coord[0];
		state.coordY = coord[1];
		if (coord.size() >= 3) state.coordZ = coord[2];
	}
	psb_read_member_number(psb, content, "ox", state.ox);
	psb_read_member_number(psb, content, "oy", state.oy);
	psb_read_member_number(psb, content, "angle", state.angle);
	psb_read_member_number(psb, content, "sx", state.shearX);
	psb_read_member_number(psb, content, "sy", state.shearY);
	psb_read_member_number(psb, content, "zx", state.scaleX);
	psb_read_member_number(psb, content, "zy", state.scaleY);
	double opacity = state.opacity;
	if (psb_read_member_number(psb, content, "opa", opacity)) {
		state.opacity = opacity > 1.0 ? opacity / 255.0 : opacity;
	}
	return true;
}

static bool psb_parse_frame(psb_t &psb, unsigned char *frameData, PsbFrameState &state, double &time)
{
	state = PsbFrameState();
	time = 0.0;
	if (!frameData) return false;
	unsigned char *p = frameData;
	psb_value_t *v_frame = psb.unpack(p);
	if (!v_frame || v_frame->get_type() != psb_value_t::TYPE_OBJECTS) {
		delete v_frame;
		return false;
	}
	psb_objects_t *frame = static_cast<psb_objects_t*>(v_frame);
	psb_read_member_number(psb, frame, "time", time);
	state.frameTime = time;
	psb_read_member_int(psb, frame, "type", state.type);
	unsigned char *contentData = frame->get_data("content");
	if (!contentData) {
		delete v_frame;
		return true;
	}
	p = contentData;
	psb_value_t *v_content = psb.unpack(p);
	if (!v_content || v_content->get_type() != psb_value_t::TYPE_OBJECTS) {
		delete v_content;
		delete v_frame;
		return false;
	}
	bool ok = psb_parse_content_frame(psb, static_cast<psb_objects_t*>(v_content), state);
	delete v_content;
	delete v_frame;
	return ok;
}

static bool psb_read_layer_frames(psb_t &psb, const psb_objects_t *layer,
	std::vector<PsbFrameState> &parsedFrames)
{
	parsedFrames.clear();
	if (!layer) return false;
	std::vector<unsigned char*> frames;
	if (!psb_read_collection_entries(psb, layer->get_data("frameList"), frames)) return false;

	parsedFrames.reserve(frames.size());
	for (size_t i = 0; i < frames.size(); i++) {
		PsbFrameState candidate;
		double time = 0.0;
		if (!psb_parse_frame(psb, frames[i], candidate, time)) continue;
		candidate.frameTime = time;
		parsedFrames.push_back(candidate);
	}
	return !parsedFrames.empty();
}

static double psb_normalize_angle(double angle)
{
	while (angle < 0.0) angle += 360.0;
	while (angle >= 360.0) angle -= 360.0;
	return angle;
}

static double psb_interpolate_angle(double a, double b, double t)
{
	if (b < 180.0 && a > 180.0) {
		return psb_normalize_angle(a - 360.0 + (b + 360.0 - a) * t);
	}
	if (b > 180.0 && a < 180.0) {
		return psb_normalize_angle(a + (b - 360.0 - a) * t);
	}
	return psb_normalize_angle(psb_lerp(a, b, t));
}

static PsbFrameState psb_interpolate_frames(
	const PsbFrameState &a, const PsbFrameState &b, double targetTime)
{
	double span = b.frameTime - a.frameTime;
	double t = span > 0.000001 ? (targetTime - a.frameTime) / span : 0.0;
	t = std::max(0.0, std::min(1.0, t));

	PsbFrameState out = a;
	out.frameTime = targetTime;
	out.coordX = psb_lerp(a.coordX, b.coordX, t);
	out.coordY = psb_lerp(a.coordY, b.coordY, t);
	out.coordZ = psb_lerp(a.coordZ, b.coordZ, t);
	out.angle = psb_interpolate_angle(a.angle, b.angle, t);
	out.shearX = psb_lerp(a.shearX, b.shearX, t);
	out.shearY = psb_lerp(a.shearY, b.shearY, t);
	out.scaleX = psb_lerp(a.scaleX, b.scaleX, t);
	out.scaleY = psb_lerp(a.scaleY, b.scaleY, t);
	out.ox = psb_lerp(a.ox, b.ox, t);
	out.oy = psb_lerp(a.oy, b.oy, t);
	out.opacity = psb_lerp(a.opacity, b.opacity, t);
	out.timeOffset = psb_lerp(a.timeOffset, b.timeOffset, t);
	out.hasCoord = a.hasCoord || b.hasCoord;
	return out;
}

static bool psb_select_frame_from_list(const std::vector<PsbFrameState> &parsedFrames,
	double desiredTime, PsbFrameState &state)
{
	state = PsbFrameState();
	if (parsedFrames.empty()) return false;

	bool foundFrame = false;
	bool foundAtZero = false;
	PsbFrameState firstContent;
	PsbFrameState lastContent;
	PsbFrameState selected;
	PsbFrameState selectedByTime;
	PsbFrameState nextByTime;
	bool foundByTime = false;
	bool foundNextByTime = false;
	double effectiveTargetTime = desiredTime;
	if (parsedFrames.size() == 2 &&
		parsedFrames[0].type == 2 &&
		parsedFrames[1].type == 0) {
		// krkrsdl3 keeps this motion node pinned to the first frame time instead
		// of letting the trailing blank frame swallow the draw state.
		effectiveTargetTime = parsedFrames[0].frameTime;
	}
	double bestSelectedTime = -1.0;
	for (size_t i = 0; i < parsedFrames.size(); i++) {
		const PsbFrameState &candidate = parsedFrames[i];
		double time = candidate.frameTime;
		if (!foundFrame && candidate.hasContent) {
			firstContent = candidate;
			foundFrame = true;
		}
		if (candidate.hasContent) {
			lastContent = candidate;
		}
		if (time <= 0.0001) {
			selected = candidate;
			foundAtZero = true;
		}
		if (effectiveTargetTime >= 0.0 && time <= effectiveTargetTime + 0.0001) {
			if (!foundByTime || time >= bestSelectedTime) {
				selectedByTime = candidate;
				bestSelectedTime = time;
				foundByTime = true;
			}
		}
		if (effectiveTargetTime >= 0.0 && time > effectiveTargetTime + 0.0001 &&
			(!foundNextByTime || time < nextByTime.frameTime)) {
			nextByTime = candidate;
			foundNextByTime = true;
		}
	}
	if (foundByTime) {
		if (selectedByTime.hasContent && foundNextByTime && nextByTime.hasContent &&
			(selectedByTime.type != 2 || nextByTime.type == 2)) {
			state = psb_interpolate_frames(selectedByTime, nextByTime, effectiveTargetTime);
		} else {
			state = selectedByTime;
		}
		return state.hasContent;
	}
	if (psb_static_hint_prefers_final_frame()) {
		if (foundFrame) {
			state = lastContent;
			return true;
		}
		state = PsbFrameState();
		return false;
	}
	if (foundAtZero) {
		state = selected;
		return state.hasContent;
	}
	if (foundFrame) {
		state = firstContent;
		return true;
	}
	return false;
}

static bool psb_select_layer_frame(psb_t &psb, const psb_objects_t *layer, PsbFrameState &state)
{
	std::vector<PsbFrameState> parsedFrames;
	if (!psb_read_layer_frames(psb, layer, parsedFrames)) {
		state = PsbFrameState();
		return false;
	}
	double targetTime = g_staticMotionHint.active ? g_staticMotionHint.frameTime : 0.0;
	return psb_select_frame_from_list(parsedFrames, targetTime, state);
}

static PsbStaticTransform psb_apply_frame_transform(const PsbStaticTransform &base,
	const PsbFrameState &frame)
{
	if (!frame.hasContent) return base;
	PsbStaticTransform local;
	local.opacity = frame.opacity;
	PsbStaticTransform model = psb_transform_translate(frame.coordX, frame.coordY);
	model = psb_transform_multiply(model, psb_transform_rotate(frame.angle));
	model = psb_transform_multiply(psb_transform_shear(frame.shearX, frame.shearY), model);
	model = psb_transform_multiply(model, psb_transform_scale(frame.scaleX, frame.scaleY));
	local = model;
	local.opacity = frame.opacity;
	return psb_transform_multiply(base, local);
}

static void psb_sample_image_bgra(const PsbImageInfo &image, const unsigned char *pixels,
	int sx, int sy,
	unsigned char &sb, unsigned char &sg, unsigned char &sr, unsigned char &sa)
{
	int srcRow = psb_source_row(image, sy);
	const unsigned char *sp = pixels +
		((size_t)srcRow * (size_t)image.width + (size_t)sx) * 4u;
	sb = image.rawRGBA ? sp[2] : sp[0];
	sg = sp[1];
	sr = image.rawRGBA ? sp[0] : sp[2];
	sa = sp[3];
	if (image.recoverAlphaFromRGB && sa == 0 && (sb | sg | sr)) sa = 255;
}

static void psb_blend_bgra(unsigned char *dp,
	unsigned char sb, unsigned char sg, unsigned char sr, int sa)
{
	if (sa <= 0) return;
	int da = dp[3];
	if (sa >= 255 && da == 0) {
		dp[0] = sb;
		dp[1] = sg;
		dp[2] = sr;
		dp[3] = 255;
		return;
	}
	int oa = sa + (da * (255 - sa) / 255);
	if (oa <= 0) return;
	dp[0] = (unsigned char)((sb * sa + dp[0] * da * (255 - sa) / 255) / oa);
	dp[1] = (unsigned char)((sg * sa + dp[1] * da * (255 - sa) / 255) / oa);
	dp[2] = (unsigned char)((sr * sa + dp[2] * da * (255 - sa) / 255) / oa);
	dp[3] = (unsigned char)oa;
}

static void psb_rasterize_triangle_to_canvas(
	const PsbImageInfo &image, const unsigned char *pixels,
	const double *qx, const double *qy,
	const double *qu, const double *qv,
	double opacity, unsigned char *canvas, int canvasW, int canvasH)
{
	double minX = std::min(qx[0], std::min(qx[1], qx[2]));
	double minY = std::min(qy[0], std::min(qy[1], qy[2]));
	double maxX = std::max(qx[0], std::max(qx[1], qx[2]));
	double maxY = std::max(qy[0], std::max(qy[1], qy[2]));
	int ix0 = std::max(0, (int)std::floor(minX));
	int iy0 = std::max(0, (int)std::floor(minY));
	int ix1 = std::min(canvasW - 1, (int)std::ceil(maxX));
	int iy1 = std::min(canvasH - 1, (int)std::ceil(maxY));
	if (ix0 > ix1 || iy0 > iy1) return;

	double denom =
		(qy[1] - qy[2]) * (qx[0] - qx[2]) +
		(qx[2] - qx[1]) * (qy[0] - qy[2]);
	if (std::abs(denom) <= 1e-8) return;

	for (int dy = iy0; dy <= iy1; dy++) {
		for (int dx = ix0; dx <= ix1; dx++) {
			double px = (double)dx + 0.5;
			double py = (double)dy + 0.5;
			double w0 =
				((qy[1] - qy[2]) * (px - qx[2]) +
				 (qx[2] - qx[1]) * (py - qy[2])) / denom;
			double w1 =
				((qy[2] - qy[0]) * (px - qx[2]) +
				 (qx[0] - qx[2]) * (py - qy[2])) / denom;
			double w2 = 1.0 - w0 - w1;
			if (w0 < -1e-5 || w1 < -1e-5 || w2 < -1e-5) continue;

			double u = w0 * qu[0] + w1 * qu[1] + w2 * qu[2];
			double v = w0 * qv[0] + w1 * qv[1] + w2 * qv[2];
			int sx = std::min(image.width - 1, std::max(0, (int)std::floor(u * image.width)));
			int sy = std::min(image.height - 1, std::max(0, (int)std::floor(v * image.height)));
			unsigned char sb = 0;
			unsigned char sg = 0;
			unsigned char sr = 0;
			unsigned char sa = 0;
			psb_sample_image_bgra(image, pixels, sx, sy, sb, sg, sr, sa);
			if (opacity < 1.0) {
				sa = (unsigned char)std::lround((double)sa * opacity);
			}
			unsigned char *dp =
				canvas + ((size_t)dy * (size_t)canvasW + (size_t)dx) * 4u;
			psb_blend_bgra(dp, sb, sg, sr, sa);
		}
	}
}

static void psb_composite_draw_to_canvas(const PsbDrawInfo &draw,
	int offsetX, int offsetY,
	const unsigned char *pixels, unsigned char *canvas,
	int canvasW, int canvasH)
{
	if (!pixels || !canvas || canvasW <= 0 || canvasH <= 0 || draw.opacity <= 0.001) return;

	double quadX[4] = {0.0, 0.0, 0.0, 0.0};
	double quadY[4] = {0.0, 0.0, 0.0, 0.0};
	if (draw.hasQuad) {
		for (int i = 0; i < 4; i++) {
			quadX[i] = draw.quadX[i] - offsetX;
			quadY[i] = draw.quadY[i] - offsetY;
		}
	} else {
		quadX[0] = draw.x - offsetX;
		quadY[0] = draw.y - offsetY;
		quadX[1] = quadX[0] + draw.image.width * draw.scaleX;
		quadY[1] = quadY[0];
		quadX[2] = quadX[1];
		quadY[2] = quadY[0] + draw.image.height * draw.scaleY;
		quadX[3] = quadX[0];
		quadY[3] = quadY[2];
	}

	double tri0x[3] = {quadX[0], quadX[1], quadX[2]};
	double tri0y[3] = {quadY[0], quadY[1], quadY[2]};
	double tri0u[3] = {0.0, 1.0, 1.0};
	double tri0v[3] = {0.0, 0.0, 1.0};
	psb_rasterize_triangle_to_canvas(
		draw.image, pixels, tri0x, tri0y, tri0u, tri0v,
		draw.opacity, canvas, canvasW, canvasH);

	double tri1x[3] = {quadX[0], quadX[2], quadX[3]};
	double tri1y[3] = {quadY[0], quadY[2], quadY[3]};
	double tri1u[3] = {0.0, 1.0, 0.0};
	double tri1v[3] = {0.0, 1.0, 1.0};
	psb_rasterize_triangle_to_canvas(
		draw.image, pixels, tri1x, tri1y, tri1u, tri1v,
		draw.opacity, canvas, canvasW, canvasH);
}

static void psb_collect_node_entries(psb_t &psb, unsigned char *nodeData,
	int parent, std::vector<PsbNodeEntry> &nodes, int depth)
{
	if (!nodeData || depth > 64) return;
	int myIndex = (int)nodes.size();
	PsbNodeEntry entry;
	entry.data = nodeData;
	entry.parent = parent;
	nodes.push_back(entry);

	unsigned char *p = nodeData;
	psb_value_t *v_node = psb.unpack(p);
	if (!v_node || v_node->get_type() != psb_value_t::TYPE_OBJECTS) {
		delete v_node;
		return;
	}
	psb_objects_t *node = static_cast<psb_objects_t*>(v_node);
	std::vector<unsigned char*> children;
	if (psb_read_collection_entries(psb, node->get_data("children"), children)) {
		for (size_t i = 0; i < children.size(); i++) {
			psb_collect_node_entries(psb, children[i], myIndex, nodes, depth + 1);
		}
	}
	delete v_node;
}

static bool psb_unpack_node_object(psb_t &psb, unsigned char *nodeData, psb_objects_t *&node)
{
	node = nullptr;
	if (!nodeData) return false;
	unsigned char *p = nodeData;
	psb_value_t *v_node = psb.unpack(p);
	if (!v_node || v_node->get_type() != psb_value_t::TYPE_OBJECTS) {
		delete v_node;
		return false;
	}
	node = static_cast<psb_objects_t*>(v_node);
	return true;
}

static bool psb_evaluate_node_frame(psb_t &psb, unsigned char *nodeData,
	double targetTime, PsbFrameState &frame)
{
	frame = PsbFrameState();
	psb_objects_t *node = nullptr;
	if (!psb_unpack_node_object(psb, nodeData, node)) return false;
	std::vector<PsbFrameState> parsedFrames;
	bool ok = false;
	if (psb_read_layer_frames(psb, node, parsedFrames)) {
		ok = psb_select_frame_from_list(parsedFrames, targetTime, frame);
	}
	delete node;
	return ok;
}

static double psb_compute_motion_self_sync_time(
	psb_t &psb, const std::vector<PsbNodeEntry> &nodes)
{
	double maxTime = 0.0;
	for (size_t i = 0; i < nodes.size(); i++) {
		psb_objects_t *node = nullptr;
		if (!psb_unpack_node_object(psb, nodes[i].data, node)) continue;
		std::vector<PsbFrameState> parsedFrames;
		if (psb_read_layer_frames(psb, node, parsedFrames)) {
			for (size_t fi = 0; fi < parsedFrames.size(); fi++) {
				if (parsedFrames[fi].hasContent &&
					parsedFrames[fi].frameTime > maxTime) {
					maxTime = parsedFrames[fi].frameTime;
				}
			}
		}
		delete node;
	}
	return maxTime;
}

static double psb_get_motion_sync_time(psb_t &psb,
	const psb_objects_t *rootObjects,
	const std::string &objectName, const std::string &motionName)
{
	if (!rootObjects) return 0.0;
	unsigned char *p_object = rootObjects->get_data(objectName);
	if (!p_object) return 0.0;
	unsigned char *p = p_object;
	psb_value_t *v_object = psb.unpack(p);
	if (!v_object || v_object->get_type() != psb_value_t::TYPE_OBJECTS) {
		delete v_object;
		return 0.0;
	}
	psb_objects_t *object = static_cast<psb_objects_t*>(v_object);
	unsigned char *p_motionRoot = object->get_data("motion");
	double result = 0.0;
	if (p_motionRoot) {
		p = p_motionRoot;
		psb_value_t *v_motionRoot = psb.unpack(p);
		if (v_motionRoot && v_motionRoot->get_type() == psb_value_t::TYPE_OBJECTS) {
			psb_objects_t *motionRoot = static_cast<psb_objects_t*>(v_motionRoot);
			unsigned char *p_motion = motionRoot->get_data(motionName);
			if (p_motion) {
				p = p_motion;
				psb_value_t *v_motion = psb.unpack(p);
				if (v_motion && v_motion->get_type() == psb_value_t::TYPE_OBJECTS) {
					psb_objects_t *motion = static_cast<psb_objects_t*>(v_motion);
					std::vector<unsigned char*> layers;
					if (psb_read_collection_entries(psb, motion->get_data("layer"), layers)) {
						std::vector<PsbNodeEntry> nodes;
						for (size_t i = 0; i < layers.size(); i++)
							psb_collect_node_entries(psb, layers[i], -1, nodes, 0);
						result = psb_compute_motion_self_sync_time(psb, nodes);
					}
				}
				delete v_motion;
			}
		}
		delete v_motionRoot;
	}
	delete v_object;
	return result;
}

static bool psb_parse_node_frame(psb_t &psb, unsigned char *nodeData, PsbFrameState &frame)
{
	double targetTime = g_staticMotionHint.active ? g_staticMotionHint.frameTime : 0.0;
	return psb_evaluate_node_frame(psb, nodeData, targetTime, frame);
}

static bool psb_parse_node_frame_at_time(psb_t &psb, unsigned char *nodeData,
	double targetTime, PsbFrameState &frame)
{
	return psb_evaluate_node_frame(psb, nodeData, targetTime, frame);
}

static bool psb_read_motion_priority(psb_t &psb, const psb_objects_t *motion, std::vector<int> &priority)
{
	priority.clear();
	if (!motion) return false;
	std::vector<unsigned char*> frames;
	if (!psb_read_collection_entries(psb, motion->get_data("priority"), frames)) return false;
	for (size_t i = 0; i < frames.size(); i++) {
		unsigned char *p = frames[i];
		psb_value_t *v_frame = psb.unpack(p);
		if (!v_frame || v_frame->get_type() != psb_value_t::TYPE_OBJECTS) {
			delete v_frame;
			continue;
		}
		psb_objects_t *frame = static_cast<psb_objects_t*>(v_frame);
		std::vector<unsigned char*> entries;
		if (psb_read_collection_entries(psb, frame->get_data("content"), entries) && !entries.empty()) {
			for (size_t k = 0; k < entries.size(); k++) {
				int idx = 0;
				if (psb_read_int_value(psb, entries[k], idx)) priority.push_back(idx);
			}
			delete v_frame;
			return !priority.empty();
		}
		delete v_frame;
	}
	return false;
}

static bool psb_collect_named_motion_draws(psb_t &psb,
	const psb_objects_t *rootObjects,
	const std::map<std::string, PsbImageInfo*> &sourceMap,
	const std::string &objectName,
	const std::string &motionName,
	const PsbStaticTransform &baseTransform,
	double frameTime,
	std::vector<PsbDrawInfo> &draws,
	int canvasW,
	int canvasH,
	std::set<std::string> &motionStack,
	int depth);

static void psb_add_static_image_draw(const PsbImageInfo &image,
	const PsbFrameState &frame,
	const PsbStaticTransform &transform,
	int canvasW,
	int canvasH,
	std::vector<PsbDrawInfo> &draws)
{
	if (transform.opacity <= 0.001) return;
	double originX = image.originX;
	double originY = image.originY;
	bool fullScreenNoCoord = !frame.hasCoord && canvasW > 0 && canvasH > 0 &&
		image.width >= canvasW && image.height >= canvasH;
	PsbDrawInfo draw;
	draw.image = image;
	draw.z = frame.coordZ;
	draw.scaleX = transform.scaleX;
	draw.scaleY = transform.scaleY;
	draw.opacity = transform.opacity;
	draw.ref = frame.src;
	draw.hasQuad = true;

	if (fullScreenNoCoord) {
		draw.x = 0.0;
		draw.y = 0.0;
		draw.scaleX = 1.0;
		draw.scaleY = 1.0;
		draw.quadX[0] = 0.0;
		draw.quadY[0] = 0.0;
		draw.quadX[1] = (double)image.width;
		draw.quadY[1] = 0.0;
		draw.quadX[2] = (double)image.width;
		draw.quadY[2] = (double)image.height;
		draw.quadX[3] = 0.0;
		draw.quadY[3] = (double)image.height;
		draws.push_back(draw);
		return;
	}

	PsbStaticTransform itemTransform = psb_transform_multiply(
		transform, psb_transform_translate(-(originX + frame.ox), -(originY + frame.oy)));
	psb_apply_point(itemTransform, 0.0, 0.0, draw.quadX[0], draw.quadY[0]);
	psb_apply_point(itemTransform, (double)image.width, 0.0, draw.quadX[1], draw.quadY[1]);
	psb_apply_point(itemTransform, (double)image.width, (double)image.height, draw.quadX[2], draw.quadY[2]);
	psb_apply_point(itemTransform, 0.0, (double)image.height, draw.quadX[3], draw.quadY[3]);
	draw.x = draw.quadX[0];
	draw.y = draw.quadY[0];
	draws.push_back(draw);
}

static bool psb_collect_motion_draws(psb_t &psb,
	const psb_objects_t *motion,
	const psb_objects_t *rootObjects,
	const std::map<std::string, PsbImageInfo*> &sourceMap,
	const PsbStaticTransform &baseTransform,
	double frameTime,
	std::vector<PsbDrawInfo> &draws,
	int canvasW,
	int canvasH,
	std::set<std::string> &motionStack,
	int depth)
{
	if (!motion || depth > 16) return false;
	std::vector<unsigned char*> layers;
	if (!psb_read_collection_entries(psb, motion->get_data("layer"), layers) || layers.empty()) {
		return false;
	}

	std::vector<PsbNodeEntry> nodes;
	for (size_t i = 0; i < layers.size(); i++) {
		psb_collect_node_entries(psb, layers[i], -1, nodes, 0);
	}
	if (nodes.empty()) return false;

	double motionSelfSyncTime = psb_compute_motion_self_sync_time(psb, nodes);
	double evalTime = frameTime;
	if (motionSelfSyncTime > 0.0 && evalTime > motionSelfSyncTime) {
		evalTime = motionSelfSyncTime;
	}
#ifdef __EMSCRIPTEN__
	if (psb_gfx_verbose_log_enabled() &&
		g_staticMotionHint.active &&
		(g_staticMotionHint.motionName == "menubar_on" ||
		 g_staticMotionHint.motionName == "menubar")) {
		emscripten_log(EM_LOG_CONSOLE,
			"[PSB-GFX] menubar_on debug: selfSyncTime=%.1f evalTime=%.1f nodes=%d",
			motionSelfSyncTime, evalTime, (int)nodes.size());
		// print all frames of first node
		if (!nodes.empty()) {
			psb_objects_t *dbgNode = nullptr;
			if (psb_unpack_node_object(psb, nodes[0].data, dbgNode)) {
				std::vector<PsbFrameState> dbgFrames;
				if (psb_read_layer_frames(psb, dbgNode, dbgFrames)) {
					for (size_t fi = 0; fi < dbgFrames.size() && fi < 10; fi++) {
						emscripten_log(EM_LOG_CONSOLE,
							"[PSB-GFX] frame[%d] t=%.1f hasContent=%d coordY=%.1f opa=%.3f src=%s",
							(int)fi, dbgFrames[fi].frameTime, dbgFrames[fi].hasContent ? 1 : 0,
							dbgFrames[fi].coordY, dbgFrames[fi].opacity,
							dbgFrames[fi].src.c_str());
					}
				}
				delete dbgNode;
			}
		}
	}
#endif

	std::vector<PsbFrameState> frames(nodes.size());
	std::vector<PsbStaticTransform> transforms(nodes.size());
	for (size_t i = 0; i < nodes.size(); i++) {
		psb_parse_node_frame_at_time(psb, nodes[i].data, evalTime, frames[i]);
		PsbStaticTransform parentTransform = baseTransform;
		if (nodes[i].parent >= 0 && nodes[i].parent < (int)i) {
			parentTransform = transforms[(size_t)nodes[i].parent];
		}
		transforms[i] = frames[i].hasContent ?
			psb_apply_frame_transform(parentTransform, frames[i]) :
			parentTransform;
	}

	std::vector<int> drawOrder;
	if (psb_read_motion_priority(psb, motion, drawOrder) && !drawOrder.empty()) {
		std::reverse(drawOrder.begin(), drawOrder.end());
	} else {
		for (int i = 0; i < (int)nodes.size(); i++) drawOrder.push_back(i);
	}

	size_t before = draws.size();
	for (size_t i = 0; i < drawOrder.size(); i++) {
		int idx = drawOrder[i];
		if (idx < 0 || idx >= (int)nodes.size()) continue;
		const PsbFrameState &frame = frames[(size_t)idx];
		if (!frame.hasContent || frame.src.empty()) continue;
		if (psb_string_starts_with(frame.src, "src/")) {
			std::map<std::string, PsbImageInfo*>::const_iterator it = sourceMap.find(frame.src);
			if (it != sourceMap.end() && it->second) {
				psb_add_static_image_draw(*it->second, frame, transforms[(size_t)idx],
					canvasW, canvasH, draws);
			}
		} else if (psb_string_starts_with(frame.src, "motion/")) {
			std::vector<std::string> parts = psb_split_path(frame.src);
			if (parts.size() >= 3) {
				psb_collect_named_motion_draws(psb, rootObjects, sourceMap,
					parts[1], parts[2], transforms[(size_t)idx],
					frameTime + frame.timeOffset, draws,
					canvasW, canvasH, motionStack, depth + 1);
			}
		}
	}
	return draws.size() > before;
}

static bool psb_collect_named_motion_draws(psb_t &psb,
	const psb_objects_t *rootObjects,
	const std::map<std::string, PsbImageInfo*> &sourceMap,
	const std::string &objectName,
	const std::string &motionName,
	const PsbStaticTransform &baseTransform,
	double frameTime,
	std::vector<PsbDrawInfo> &draws,
	int canvasW,
	int canvasH,
	std::set<std::string> &motionStack,
	int depth)
{
	if (!rootObjects) return false;
	std::string key = objectName + "/" + motionName;
	if (motionStack.find(key) != motionStack.end()) return false;
	motionStack.insert(key);

	bool ok = false;
	unsigned char *p_object = rootObjects->get_data(objectName);
	if (p_object) {
		unsigned char *p = p_object;
		psb_value_t *v_object = psb.unpack(p);
		if (v_object && v_object->get_type() == psb_value_t::TYPE_OBJECTS) {
			psb_objects_t *object = static_cast<psb_objects_t*>(v_object);
			unsigned char *p_motionRoot = object->get_data("motion");
			if (p_motionRoot) {
				p = p_motionRoot;
				psb_value_t *v_motionRoot = psb.unpack(p);
				if (v_motionRoot && v_motionRoot->get_type() == psb_value_t::TYPE_OBJECTS) {
					psb_objects_t *motionRoot = static_cast<psb_objects_t*>(v_motionRoot);
					unsigned char *p_motion = motionRoot->get_data(motionName);
					if (p_motion) {
						p = p_motion;
						psb_value_t *v_motion = psb.unpack(p);
						if (v_motion && v_motion->get_type() == psb_value_t::TYPE_OBJECTS) {
							PsbStaticMotionHint savedHint = g_staticMotionHint;
							g_staticMotionHint.active = true;
							g_staticMotionHint.frameTime = frameTime;
							ok = psb_collect_motion_draws(psb, static_cast<psb_objects_t*>(v_motion),
								rootObjects, sourceMap, baseTransform, frameTime, draws,
								canvasW, canvasH, motionStack, depth);
							g_staticMotionHint = savedHint;
						}
						delete v_motion;
					}
				}
				delete v_motionRoot;
			}
		}
		delete v_object;
	}
	motionStack.erase(key);
	return ok;
}

static bool psb_collect_object_motion_names(psb_t &psb,
	const psb_objects_t *rootObjects,
	const std::string &objectName,
	std::vector<std::string> &motionNames)
{
	motionNames.clear();
	if (!rootObjects) return false;
	unsigned char *p_object = rootObjects->get_data(objectName);
	if (!p_object) return false;

	unsigned char *p = p_object;
	psb_value_t *v_object = psb.unpack(p);
	if (!v_object || v_object->get_type() != psb_value_t::TYPE_OBJECTS) {
		delete v_object;
		return false;
	}

	psb_objects_t *object = static_cast<psb_objects_t*>(v_object);
	unsigned char *p_motionRoot = object->get_data("motion");
	if (!p_motionRoot) {
		delete v_object;
		return false;
	}

	p = p_motionRoot;
	psb_value_t *v_motionRoot = psb.unpack(p);
	if (!v_motionRoot || v_motionRoot->get_type() != psb_value_t::TYPE_OBJECTS) {
		delete v_motionRoot;
		delete v_object;
		return false;
	}

	psb_objects_t *motionRoot = static_cast<psb_objects_t*>(v_motionRoot);
	for (uint32_t i = 0; i < motionRoot->size(); i++) {
		motionNames.push_back(motionRoot->get_name(i));
	}

	delete v_motionRoot;
	delete v_object;
	return !motionNames.empty();
}

static bool psb_read_screen_size(psb_t &psb, const psb_objects_t *root,
	int &width, int &height, int *originX = nullptr, int *originY = nullptr)
{
	width = 0;
	height = 0;
	if (originX) *originX = 0;
	if (originY) *originY = 0;
	if (!root) return false;
	unsigned char *p_screen = root->get_data("screenSize");
	if (!p_screen) return false;
	unsigned char *p = p_screen;
	psb_value_t *v_screen = psb.unpack(p);
	if (!v_screen || v_screen->get_type() != psb_value_t::TYPE_OBJECTS) {
		delete v_screen;
		return false;
	}
	psb_objects_t *screen = static_cast<psb_objects_t*>(v_screen);
	psb_read_member_int(psb, screen, "width", width);
	psb_read_member_int(psb, screen, "height", height);
	if (originX) psb_read_member_int(psb, screen, "originX", *originX);
	if (originY) psb_read_member_int(psb, screen, "originY", *originY);
	delete v_screen;
	return width > 0 && height > 0;
}

static bool psb_infer_screen_origin_from_images(
	const std::vector<PsbImageInfo> &images, int canvasW, int canvasH,
	int &originX, int &originY)
{
	if (canvasW <= 0 || canvasH <= 0) return false;
	double bestScore = -1.0;
	int bestX = 0;
	int bestY = 0;
	for (size_t i = 0; i < images.size(); i++) {
		const PsbImageInfo &image = images[i];
		if (image.originX == 0 && image.originY == 0) continue;
		if (image.width <= 0 || image.height <= 0) continue;
		if (image.originX < 0 || image.originY < 0 ||
			image.originX > image.width || image.originY > image.height) {
			continue;
		}
		if (image.width < canvasW * 0.75 || image.height < canvasH * 0.50) {
			continue;
		}
		if (image.width > canvasW * 2.0 || image.height > canvasH * 2.0) {
			continue;
		}
		double widthFit = 1.0 - std::min(1.0,
			std::abs((double)image.width - (double)canvasW) /
			std::max(1.0, (double)canvasW));
		double heightFit = 1.0 - std::min(1.0,
			std::abs((double)image.height - (double)canvasH) /
			std::max(1.0, (double)canvasH));
		double originCenterFit = 1.0 - std::min(1.0,
			std::abs((double)image.originX - (double)image.width / 2.0) /
			std::max(1.0, (double)image.width / 2.0));
		double score = widthFit * 4.0 + heightFit * 2.0 + originCenterFit;
		if (score > bestScore) {
			bestScore = score;
			bestX = image.originX;
			bestY = image.originY;
		}
	}
	if (bestScore < 0.0) return false;
	originX = bestX;
	originY = bestY;
	return true;
}

static bool psb_build_static_motion_draws(psb_t &psb,
	const psb_objects_t *root,
	std::vector<PsbImageInfo> &images,
	std::vector<PsbDrawInfo> &draws,
	int &canvasW,
	int &canvasH)
{
	draws.clear();
	canvasW = 0;
	canvasH = 0;
	if (!root) return false;

	std::string spec;
	if (!psb_read_member_string(psb, root, "spec", spec) || spec != "krkr") {
		return false;
	}
	int screenOriginX = 0;
	int screenOriginY = 0;
	psb_read_screen_size(psb, root, canvasW, canvasH, &screenOriginX, &screenOriginY);
	std::string label;
	psb_read_member_string(psb, root, "label", label);

	std::map<std::string, PsbImageInfo*> sourceMap;
	psb_build_source_map(images, sourceMap);
	if (sourceMap.empty()) return false;
	bool looksMainUi = false;
	for (size_t i = 0; i < images.size(); i++) {
		if (psb_string_starts_with(images[i].name, "main/icon/")) {
			looksMainUi = true;
			break;
		}
	}
	if (screenOriginX == 0 && screenOriginY == 0 && canvasW > 0 && canvasH > 0) {
		for (size_t i = 0; i < images.size(); i++) {
			const PsbImageInfo &image = images[i];
			if (image.width >= canvasW && image.height >= canvasH &&
				(image.originX != 0 || image.originY != 0)) {
				screenOriginX = image.originX;
				screenOriginY = image.originY;
				break;
			}
		}
	}
	if (screenOriginX == 0 && screenOriginY == 0) {
		psb_infer_screen_origin_from_images(
			images, canvasW, canvasH, screenOriginX, screenOriginY);
	}
	if ((screenOriginX == 0 || screenOriginY == 0) && canvasW > 0 && canvasH > 0) {
		for (size_t i = 0; i < images.size(); i++) {
			const PsbImageInfo &image = images[i];
			if (image.width < canvasW || image.height < canvasH) continue;
			if (screenOriginX == 0 && image.originX != 0) screenOriginX = image.originX;
			if (screenOriginY == 0 && image.originY != 0) screenOriginY = image.originY;
			if (screenOriginX != 0 && screenOriginY != 0) break;
		}
	}
	if (screenOriginX == 0 || screenOriginY == 0) {
		int inferredX = 0;
		int inferredY = 0;
		if (psb_infer_screen_origin_from_images(
			images, canvasW, canvasH, inferredX, inferredY)) {
			if (screenOriginX == 0) screenOriginX = inferredX;
			if (screenOriginY == 0) screenOriginY = inferredY;
		}
	}
	// Motion-style PSBs (m2logo, yuzulogo, character emote) author their layout
	// nodes in a coordinate system centered on the screen. They typically have no
	// canvas-sized icon (so origin inference fails) and don't carry an explicit
	// screenSize.originX/Y. krkrsdl3's emote runtime maps this via the per-motion
	// `lim` defaulting to canvas center. Replicate that here: if none of the
	// images are canvas-sized AND no explicit origin was found, default to canvas
	// center so layout coords like (-132, -2) land on screen instead of off the
	// negative side.
	if (canvasW > 0 && canvasH > 0 && (screenOriginX == 0 || screenOriginY == 0)) {
		bool hasCanvasSizedImage = false;
		for (size_t i = 0; i < images.size(); i++) {
			if (images[i].width >= canvasW * 3 / 4 &&
				images[i].height >= canvasH * 3 / 4) {
				hasCanvasSizedImage = true;
				break;
			}
		}
		if (!hasCanvasSizedImage) {
			if (screenOriginX == 0) screenOriginX = canvasW / 2;
			if (screenOriginY == 0) screenOriginY = canvasH / 2;
		}
	}
	std::string lowerLabel = psb_lower_string(label);
#ifdef __EMSCRIPTEN__
	if (psb_gfx_verbose_log_enabled() &&
		(looksMainUi || lowerLabel == "main")) {
		emscripten_log(EM_LOG_CONSOLE,
			"[PSB-GFX] screenOrigin label=%s looksMainUi=%d fromPsb=(%d,%d) canvas=%dx%d",
			label.c_str(), looksMainUi ? 1 : 0,
			screenOriginX, screenOriginY, canvasW, canvasH);
	}
#endif
	if (canvasW > 0 && canvasH > 0) {
		if (screenOriginX == 0 && lowerLabel == "main") {
			screenOriginX = canvasW / 2;
		}
		if (screenOriginY == 0 &&
			(lowerLabel == "main" || lowerLabel == "autoskip")) {
			screenOriginY = canvasH / 2;
		}
	}

	unsigned char *p_objects = root->get_data("object");
	if (!p_objects) return false;
	unsigned char *p = p_objects;
	psb_value_t *v_objects = psb.unpack(p);
	if (!v_objects || v_objects->get_type() != psb_value_t::TYPE_OBJECTS) {
		delete v_objects;
		return false;
	}
	psb_objects_t *objects = static_cast<psb_objects_t*>(v_objects);

	std::vector<std::string> preferredObjects;
	if (label == "title") {
		preferredObjects.push_back("TITLE");
		preferredObjects.push_back("TITLE_TRIAL");
		preferredObjects.push_back("TITLE2");
	}
	preferredObjects.push_back("TITLE");
	preferredObjects.push_back("MONO_LOOP");
	preferredObjects.push_back("mono_loop");
	preferredObjects.push_back("mono");

	PsbStaticTransform base;
	base.tx = screenOriginX;
	base.ty = screenOriginY;
	base.x = screenOriginX;
	base.y = screenOriginY;
	std::set<std::string> motionStack;
	bool ok = false;
	std::string selectedObject;
	std::string selectedMotion;
	if (g_staticMotionHint.active &&
		!g_staticMotionHint.objectName.empty() &&
		!g_staticMotionHint.motionName.empty()) {
		std::vector<PsbDrawInfo> temp;
		bool hintOk = psb_collect_named_motion_draws(psb, objects, sourceMap,
			g_staticMotionHint.objectName, g_staticMotionHint.motionName,
			base, g_staticMotionHint.frameTime, temp, canvasW, canvasH, motionStack, 0);
		if (hintOk && !temp.empty()) {
			draws = temp;
			ok = true;
			selectedObject = g_staticMotionHint.objectName;
			selectedMotion = g_staticMotionHint.motionName;
#ifdef __EMSCRIPTEN__
			if (psb_gfx_verbose_log_enabled()) {
				emscripten_log(EM_LOG_CONSOLE,
					"[PSB-GFX] static motion hint selected %s/%s for %s",
					g_staticMotionHint.objectName.c_str(),
					g_staticMotionHint.motionName.c_str(),
					g_staticMotionHint.storage.c_str());
			}
#endif
		} else if (g_staticMotionHint.storage.find("title.psb") != std::string::npos) {
#ifdef __EMSCRIPTEN__
			std::vector<std::string> motionNames;
			std::string names;
			if (psb_collect_object_motion_names(psb, objects,
				g_staticMotionHint.objectName, motionNames)) {
				for (size_t n = 0; n < motionNames.size(); n++) {
					if (!names.empty()) names += ",";
					names += motionNames[n];
				}
			}
			if (psb_gfx_verbose_log_enabled()) {
				emscripten_log(EM_LOG_CONSOLE,
					"[PSB-GFX] static motion hint missed %s/%s for %s ok=%d draws=%d motions=[%s]",
					g_staticMotionHint.objectName.c_str(),
					g_staticMotionHint.motionName.c_str(),
					g_staticMotionHint.storage.c_str(),
					hintOk ? 1 : 0, (int)temp.size(), names.c_str());
			}
#endif
		}
	}
	for (size_t i = 0; i < preferredObjects.size() && !ok; i++) {
		std::vector<PsbDrawInfo> temp;
		if (psb_collect_named_motion_draws(psb, objects, sourceMap, preferredObjects[i],
			"normal", base, 0.0, temp, canvasW, canvasH, motionStack, 0) && !temp.empty()) {
			draws = temp;
			ok = true;
			selectedObject = preferredObjects[i];
			selectedMotion = "normal";
		}
	}

	if (!ok) {
		tjs_int bestScore = std::numeric_limits<tjs_int>::min();
		std::vector<PsbDrawInfo> best;
		for (uint32_t i = 0; i < objects->size(); i++) {
			std::string objectName = objects->get_name(i);
			if (psb_string_starts_with(objectName, "bt_")) continue;
			std::vector<std::string> motionNames;
			if (!psb_collect_object_motion_names(psb, objects, objectName, motionNames)) continue;
			for (size_t m = 0; m < motionNames.size(); m++) {
				std::vector<PsbDrawInfo> temp;
				std::set<std::string> stack;
				const std::string &motionName = motionNames[m];
				bool prefersEnd = (looksMainUi || label == "main") &&
					objectName == "MSGWIN" &&
					(motionName == "menubar_on" || motionName == "menubar" ||
					 motionName == "menubar_over");
				double evalFrameTime = 0.0;
				if (prefersEnd) {
					evalFrameTime = psb_get_motion_sync_time(psb, objects, objectName, motionName);
#ifdef __EMSCRIPTEN__
				if (psb_gfx_verbose_log_enabled()) {
					emscripten_log(EM_LOG_CONSOLE,
						"[PSB-GFX] prefersEnd %s/%s syncTime=%.1f",
						objectName.c_str(), motionName.c_str(), evalFrameTime);
				}
#endif
				}
				if (!psb_collect_named_motion_draws(psb, objects, sourceMap, objectName,
					motionName, base, evalFrameTime, temp, canvasW, canvasH, stack, 0) || temp.empty()) {
					continue;
				}
				tjs_int score = (tjs_int)temp.size();
				if (motionName == "normal") score += 200;
				if (motionName == "loop") score += 100;
				if (looksMainUi || label == "main") {
					if (objectName == "MSGWIN" &&
						(motionName == "menubar" || motionName == "menubar_on" ||
						 motionName == "menubar_over" || motionName == "menubg" ||
						 motionName == "menubg2")) {
						score += 5000;
						if (motionName == "menubar_on") score += 100;
					}
					if (objectName == "STATUS" &&
						(motionName == "normal" || motionName == "show" ||
						 motionName == "text" || motionName == "wait" ||
						 motionName == "waiton" || motionName == "auto")) {
						score -= 400;
					}
				}
				for (size_t j = 0; j < temp.size(); j++) {
					if (canvasW > 0 && canvasH > 0 &&
						temp[j].image.width >= canvasW / 2 &&
						temp[j].image.height >= canvasH / 2) {
						score += 1000;
					}
				}
#ifdef __EMSCRIPTEN__
				if (psb_gfx_verbose_log_enabled() &&
					(looksMainUi || label == "main")) {
					PsbDrawBounds bounds;
					psb_bounds_from_draws(temp, bounds);
					const char *firstRef = temp.empty() ? "" : temp[0].image.name.c_str();
					emscripten_log(EM_LOG_CONSOLE,
						"[PSB-GFX] main candidate %s/%s score=%d draws=%d bounds=(%d,%d)-(%d,%d) first=%s",
						objectName.c_str(), motionName.c_str(), (int)score,
						(int)temp.size(), bounds.minX, bounds.minY,
						bounds.maxX, bounds.maxY, firstRef);
				}
#endif
				if (score > bestScore) {
					bestScore = score;
					best = temp;
					selectedObject = objectName;
					selectedMotion = motionName;
#ifdef __EMSCRIPTEN__
					if (psb_gfx_verbose_log_enabled() &&
						(looksMainUi || label == "main")) {
						PsbDrawBounds bounds;
						psb_bounds_from_draws(temp, bounds);
						emscripten_log(EM_LOG_CONSOLE,
							"[PSB-GFX] main candidate best %s/%s score=%d draws=%d bounds=(%d,%d)-(%d,%d)",
							objectName.c_str(), motionName.c_str(), (int)score,
							(int)temp.size(), bounds.minX, bounds.minY,
							bounds.maxX, bounds.maxY);
					}
#endif
				}
			}
		}
		if (!best.empty()) {
			draws = best;
			ok = true;
		}
	}

	delete v_objects;
	if (!ok || draws.empty()) return false;
	if (canvasW <= 0 || canvasH <= 0) return false;
#ifdef __EMSCRIPTEN__
	if (psb_gfx_verbose_log_enabled() &&
		(looksMainUi || label == "main")) {
		PsbDrawBounds bounds;
		psb_bounds_from_draws(draws, bounds);
		emscripten_log(EM_LOG_CONSOLE,
			"[PSB-GFX] main static motion selected %s/%s label=%s draws=%d bounds=(%d,%d)-(%d,%d)",
			selectedObject.c_str(), selectedMotion.c_str(), label.c_str(),
			(int)draws.size(), bounds.minX, bounds.minY, bounds.maxX, bounds.maxY);
	}
	if (psb_gfx_verbose_log_enabled()) {
		emscripten_log(EM_LOG_CONSOLE,
			"[PSB-GFX] static motion layout selected %d draw items on %dx%d canvas screenOrigin=(%d,%d)",
			(int)draws.size(), canvasW, canvasH, (int)base.tx, (int)base.ty);
		if (!draws.empty()) {
			emscripten_log(EM_LOG_CONSOLE,
				"[PSB-GFX] first draw item x=%.1f y=%.1f img=%s imgW=%d imgH=%d",
				draws[0].x, draws[0].y,
				draws[0].image.name.c_str(), draws[0].image.width, draws[0].image.height);
			for (size_t _di = 0; _di < draws.size() && _di < 20; _di++) {
				emscripten_log(EM_LOG_CONSOLE,
					"[PSB-GFX] draw[%d] img=%s opa=%.3f x=%.1f y=%.1f",
					(int)_di, draws[_di].image.name.c_str(), draws[_di].opacity,
					draws[_di].x, draws[_di].y);
			}
		}
	}
#endif
	return true;
}

static std::vector<PsbDrawInfo> psb_make_fallback_draws(const std::vector<PsbImageInfo> &candidates)
{
	std::vector<PsbDrawInfo> draws;
	for (size_t i = 0; i < candidates.size(); i++) {
		const PsbImageInfo &c = candidates[i];
		double fallbackOriginX = c.originFromOriginFields ? 0.0 : (double)c.originX;
		double fallbackOriginY = c.originFromOriginFields ? 0.0 : (double)c.originY;
		PsbDrawInfo draw;
		draw.image = c;
		draw.x = (double)c.left - fallbackOriginX;
		draw.y = (double)c.top - fallbackOriginY;
		draw.ref = c.name;
		draws.push_back(draw);
	}
	return draws;
}

static void TVPLoadPSB(void* formatdata, void *callbackdata,
	tTVPGraphicSizeCallback sizecallback,
	tTVPGraphicScanLineCallback scanlinecallback,
	tTVPMetaInfoPushCallback metainfopushcallback,
	tTJSBinaryStream *src, tjs_int32 keyidx, tTVPGraphicLoadMode mode)
{
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfStart = TVPGetTickCount();
#endif
	tjs_uint64 filesize = src->GetSize();
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfAfterSize = TVPGetTickCount();
	tjs_uint64 perfReadStart = perfAfterSize;
#endif
	unsigned char *buf = new unsigned char[filesize];
	tjs_uint read = src->Read(buf, (tjs_uint)filesize);
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfAfterRead = TVPGetTickCount();
#endif
#ifdef __EMSCRIPTEN__
	if (psb_gfx_verbose_log_enabled()) {
		KRKR_LOG_L3("[PSB-GFX] LoadPSB size=%llu read=%u sig=%02x %02x %02x %02x\n",
			(unsigned long long)filesize, (unsigned)read,
			filesize > 0 ? buf[0] : 0, filesize > 1 ? buf[1] : 0,
			filesize > 2 ? buf[2] : 0, filesize > 3 ? buf[3] : 0);
	}
#endif

	if (filesize < 4 || buf[0] != 'P' || buf[1] != 'S' || buf[2] != 'B') {
#ifdef __EMSCRIPTEN__
		tjs_uint64 perfEnd = TVPGetTickCount();
		fprintf(stderr,
			"[PERF] psb.gfx.load ok=0 reason=invalid-header filesize=%llu read=%u keyidx=%d mode=%d size_ms=%llu read_ms=%llu total_ms=%llu\n",
			(unsigned long long)filesize,
			(unsigned)read,
			(int)keyidx,
			(int)mode,
			(unsigned long long)(perfAfterSize - perfStart),
			(unsigned long long)(perfAfterRead - perfReadStart),
			(unsigned long long)(perfEnd - perfStart));
#endif
		delete[] buf;
		TVPThrowExceptionMessage(TJS_W("Not a valid PSB file"));
		return;
	}

	psb_t *psb = nullptr;
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfParseStart = TVPGetTickCount();
	tjs_uint64 perfAfterParse = perfParseStart;
#endif
	try {
		psb = new psb_t(buf);
#ifdef __EMSCRIPTEN__
		perfAfterParse = TVPGetTickCount();
#endif
	} catch (...) {
#ifdef __EMSCRIPTEN__
		tjs_uint64 perfEnd = TVPGetTickCount();
		fprintf(stderr,
			"[PERF] psb.gfx.load ok=0 reason=parse-exception filesize=%llu read=%u keyidx=%d mode=%d size_ms=%llu read_ms=%llu parse_ms=%llu total_ms=%llu\n",
			(unsigned long long)filesize,
			(unsigned)read,
			(int)keyidx,
			(int)mode,
			(unsigned long long)(perfAfterSize - perfStart),
			(unsigned long long)(perfAfterRead - perfReadStart),
			(unsigned long long)(perfEnd - perfParseStart),
			(unsigned long long)(perfEnd - perfStart));
#endif
		delete[] buf;
#ifdef __EMSCRIPTEN__
		if (psb_gfx_verbose_log_enabled()) {
			KRKR_LOG_L3("[PSB-GFX] Failed to parse PSB after header check\n");
		}
#endif
		TVPThrowExceptionMessage(TJS_W("Failed to parse PSB file"));
		return;
	}
#ifdef __EMSCRIPTEN__
	if (psb_gfx_verbose_log_enabled()) {
		KRKR_LOG_L3("[PSB-GFX] Header ver=%u enc=%u entries=%u names=%u strings=%u chunks=%u chunkData=%u\n",
			(unsigned)psb->hdr->version, (unsigned)psb->hdr->encrypt,
			(unsigned)psb->hdr->offset_entries, (unsigned)psb->hdr->offset_names,
			(unsigned)psb->hdr->offset_strings, (unsigned)psb->hdr->offset_chunk_offsets,
			(unsigned)psb->hdr->offset_chunk_data);
	}
#endif

	std::vector<PsbImageInfo> candidates;
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfDiscoverStart = TVPGetTickCount();
#endif
	const psb_objects_t *root = psb->get_objects();
	if (root) {
#ifdef __EMSCRIPTEN__
		if (psb_gfx_verbose_log_enabled()) {
			std::string rootKeys = psb_key_list(root);
			KRKR_LOG_L3("[PSB-GFX] root keys=[%s]\n", rootKeys.c_str());
		}
#endif
		unsigned char *p_source = root->get_data("source");
		if (p_source) {
			psb_value_t *v_source = psb->unpack(p_source);
#ifdef __EMSCRIPTEN__
			if (psb_gfx_verbose_log_enabled()) {
				KRKR_LOG_L3("[PSB-GFX] source value type=%s\n",
					v_source ? v_source->get_type_string().c_str() : "<null>");
			}
#endif
			if (v_source && v_source->get_type() == psb_value_t::TYPE_OBJECTS) {
				psb_find_all_images(*psb, static_cast<psb_objects_t*>(v_source), candidates, "");
			}
			delete v_source;
		}
		if (candidates.empty()) {
			psb_find_all_images(*psb, root, candidates, "");
		}
	}
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfAfterDiscover = TVPGetTickCount();
#endif
#ifdef __EMSCRIPTEN__
	if (psb_gfx_verbose_log_enabled()) {
		KRKR_LOG_L3("[PSB-GFX] candidates=%d\n", (int)candidates.size());
	}
#endif

	if (candidates.empty()) {
		delete psb;
		delete[] buf;
#ifdef __EMSCRIPTEN__
		tjs_uint64 perfEnd = TVPGetTickCount();
		fprintf(stderr,
			"[PERF] psb.gfx.load ok=0 reason=no-candidates filesize=%llu read=%u keyidx=%d mode=%d size_ms=%llu read_ms=%llu parse_ms=%llu discover_ms=%llu total_ms=%llu\n",
			(unsigned long long)filesize,
			(unsigned)read,
			(int)keyidx,
			(int)mode,
			(unsigned long long)(perfAfterSize - perfStart),
			(unsigned long long)(perfAfterRead - perfReadStart),
			(unsigned long long)(perfAfterParse - perfParseStart),
			(unsigned long long)(perfAfterDiscover - perfDiscoverStart),
			(unsigned long long)(perfEnd - perfStart));
		emscripten_log(EM_LOG_CONSOLE | EM_LOG_ERROR, "[PSB-GFX] No image found in PSB");
#else
		KRKR_LOG_L3("[PSB-GFX] No image found in PSB\n");
#endif
		TVPThrowExceptionMessage(TJS_W("No image resource found in PSB file"));
		return;
	}

#ifdef __EMSCRIPTEN__
	tjs_uint64 perfLayoutStart = TVPGetTickCount();
#endif
	std::vector<PsbDrawInfo> draws;
	int canvasW = 0;
	int canvasH = 0;
	int minX = 0;
	int minY = 0;
	bool usedMotionLayout = root &&
		psb_build_static_motion_draws(*psb, root, candidates, draws, canvasW, canvasH);
	if (!usedMotionLayout) {
		draws = psb_make_fallback_draws(candidates);
		// Compute bounding box of all fallback chunks.
		int bboxMinX = 0x7fffffff, bboxMinY = 0x7fffffff;
		int bboxMaxX = -0x7fffffff, bboxMaxY = -0x7fffffff;
		for (size_t i = 0; i < draws.size(); i++) {
			PsbDrawInfo &d = draws[i];
			double drawMinX = 0.0;
			double drawMinY = 0.0;
			double drawMaxX = 0.0;
			double drawMaxY = 0.0;
			if (!psb_draw_bounds(d, drawMinX, drawMinY, drawMaxX, drawMaxY)) continue;
			int cx = (int)std::floor(drawMinX);
			int cy = (int)std::floor(drawMinY);
			int cr = (int)std::ceil(drawMaxX);
			int cb = (int)std::ceil(drawMaxY);
			if (cx < bboxMinX) bboxMinX = cx;
			if (cy < bboxMinY) bboxMinY = cy;
			if (cr > bboxMaxX) bboxMaxX = cr;
			if (cb > bboxMaxY) bboxMaxY = cb;
		}
		canvasW = bboxMaxX - bboxMinX;
		canvasH = bboxMaxY - bboxMinY;
		minX = bboxMinX;
		minY = bboxMinY;
		if (canvasW <= 0 || canvasH <= 0) {
			// Fallback: use largest chunk
			canvasW = 0; canvasH = 0;
			for (size_t i = 0; i < candidates.size(); i++) {
				if (candidates[i].width * candidates[i].height > canvasW * canvasH) {
					canvasW = candidates[i].width;
					canvasH = candidates[i].height;
				}
			}
			minX = 0; minY = 0;
		}
	}
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfAfterLayout = TVPGetTickCount();
#endif

#ifdef __EMSCRIPTEN__
	if (psb_gfx_verbose_log_enabled()) {
		emscripten_log(EM_LOG_CONSOLE, "[PSB-GFX] Compositing %d draw items into %dx%d canvas (offset %d,%d, motionLayout=%d)",
			(int)draws.size(), canvasW, canvasH, minX, minY, usedMotionLayout ? 1 : 0);
	}
#else
	KRKR_LOG_L3("[PSB-GFX] Compositing %d draw items into %dx%d canvas (offset %d,%d, motionLayout=%d)\n",
		(int)draws.size(), canvasW, canvasH, minX, minY, usedMotionLayout ? 1 : 0);
#endif

	bool captureBridgeSnapshot = psb_should_capture_static_motion_bridge_snapshot();
	TVPStaticMotionBridgeSnapshot bridgeSnapshot;
	if (captureBridgeSnapshot) {
		bridgeSnapshot.storage = g_staticMotionHint.storage;
		bridgeSnapshot.objectName = g_staticMotionHint.objectName;
		bridgeSnapshot.motionName = g_staticMotionHint.motionName;
		bridgeSnapshot.frameTime = g_staticMotionHint.frameTime;
		bridgeSnapshot.canvasWidth = canvasW;
		bridgeSnapshot.canvasHeight = canvasH;
		bridgeSnapshot.offsetX = minX;
		bridgeSnapshot.offsetY = minY;
		bridgeSnapshot.usedMotionLayout = usedMotionLayout;
	}

	// Allocate canvas (BGRA)
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfCanvasAllocStart = TVPGetTickCount();
#endif
	uint32_t canvasBytes = canvasW * canvasH * 4;
	unsigned char *canvas = new unsigned char[canvasBytes];
	memset(canvas, 0, canvasBytes);
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfAfterCanvasAlloc = TVPGetTickCount();
	tjs_uint64 perfDecodeMs = 0;
	tjs_uint64 perfAlphaMs = 0;
	tjs_uint64 perfCompositeMs = 0;
	tjs_uint64 perfBridgeMs = 0;
	tjs_uint64 perfDumpMs = 0;
#endif

	// Decompress and blit each draw item
	for (size_t i = 0; i < draws.size(); i++) {
		PsbDrawInfo &draw = draws[i];
		PsbImageInfo &c = draw.image;
		if (draw.opacity <= 0.001) continue;
		uint32_t chunkBytes = c.width * c.height * 4;
		unsigned char *pixels = nullptr;
#ifdef __EMSCRIPTEN__
		tjs_uint64 perfDecodeStart = TVPGetTickCount();
#endif
		if (c.rawRGBA) {
			pixels = c.pixel_data;
		} else {
			pixels = new unsigned char[chunkBytes];
			memset(pixels, 0, chunkBytes);
			bool decoded = c.paletteIndexed ?
				psb_palette_uncompress(c, pixels, chunkBytes) :
				(psb_pixel_uncompress(c.pixel_data, pixels, c.pixel_length, 4), true);
#ifdef __EMSCRIPTEN__
			if (!decoded) {
				emscripten_log(EM_LOG_CONSOLE | EM_LOG_ERROR,
					"[PSB-GFX] failed to decode chunk: %s format=%s",
					c.name.c_str(), c.format.c_str());
			}
#endif
		}
#ifdef __EMSCRIPTEN__
		perfDecodeMs += TVPGetTickCount() - perfDecodeStart;
#endif

		int dstX = (int)std::floor(draw.x - minX);
		int dstY = (int)std::floor(draw.y - minY);
#ifdef __EMSCRIPTEN__
		tjs_uint64 perfAlphaStart = TVPGetTickCount();
#endif
		c.recoverAlphaFromRGB = psb_should_recover_alpha_from_rgb(pixels, c.width, c.height);
#ifdef __EMSCRIPTEN__
		perfAlphaMs += TVPGetTickCount() - perfAlphaStart;
#endif
#ifdef __EMSCRIPTEN__
		// Sample a non-corner pixel from the largest chunk to verify pixel
		// format. yuzu_logo is 479x277 and the visible artwork sits roughly
		// in the middle, so sample near (w/2, h/2).
		if (psb_gfx_verbose_log_enabled() &&
			c.width >= 100 && c.height >= 100) {
			int mx = c.width / 2;
			int my = c.height / 2;
			unsigned char *mid = pixels + my * c.width * 4 + mx * 4;
			emscripten_log(EM_LOG_CONSOLE,
				"[PSB-GFX] mid-sample %s (%d,%d): bytes=[%02x %02x %02x %02x]",
				c.name.c_str(), mx, my,
				mid[0], mid[1], mid[2], mid[3]);
		}
		tjs_uint64 perfDumpStart = TVPGetTickCount();
		psb_dump_chunk_to_browser(c, pixels, dstX, dstY);
		perfDumpMs += TVPGetTickCount() - perfDumpStart;
#endif

		double opacity = draw.opacity;
		if (opacity < 0.0) opacity = 0.0;
		if (opacity > 1.0) opacity = 1.0;
		if (captureBridgeSnapshot) {
#ifdef __EMSCRIPTEN__
			tjs_uint64 perfBridgeStart = TVPGetTickCount();
#endif
			psb_capture_static_motion_bridge_item(
				bridgeSnapshot, draw, c, pixels, opacity, dstX, dstY);
#ifdef __EMSCRIPTEN__
			perfBridgeMs += TVPGetTickCount() - perfBridgeStart;
#endif
		}
#ifdef __EMSCRIPTEN__
		tjs_uint64 perfCompositeStart = TVPGetTickCount();
#endif
		psb_composite_draw_to_canvas(draw, minX, minY, pixels, canvas, canvasW, canvasH);
#ifdef __EMSCRIPTEN__
		perfCompositeMs += TVPGetTickCount() - perfCompositeStart;
#endif
		if (!c.rawRGBA) delete[] pixels;
	}
	if (captureBridgeSnapshot) {
#ifdef __EMSCRIPTEN__
		tjs_uint64 perfBridgeStart = TVPGetTickCount();
#endif
		psb_capture_source_textures_for_bridge(bridgeSnapshot, candidates);
		g_lastStaticMotionBridgeSnapshot = bridgeSnapshot;
		std::string newKey = psb_snapshot_key(
			bridgeSnapshot.storage, bridgeSnapshot.objectName,
			bridgeSnapshot.motionName);
		bool replaced = false;
		for (size_t i = 0; i < g_staticMotionBridgeSnapshotCache.size(); i++) {
			TVPStaticMotionBridgeSnapshot &existing =
				g_staticMotionBridgeSnapshotCache[i];
			if (psb_snapshot_key(existing.storage, existing.objectName,
				existing.motionName) == newKey) {
				existing = bridgeSnapshot;
				replaced = true;
				break;
			}
		}
		if (!replaced) {
			g_staticMotionBridgeSnapshotCache.push_back(bridgeSnapshot);
			if (g_staticMotionBridgeSnapshotCache.size() > 32) {
				g_staticMotionBridgeSnapshotCache.erase(
					g_staticMotionBridgeSnapshotCache.begin());
			}
		}
#ifdef __EMSCRIPTEN__
		perfBridgeMs += TVPGetTickCount() - perfBridgeStart;
#endif
	}

#ifdef __EMSCRIPTEN__
	{
		tjs_uint64 perfDumpStart = TVPGetTickCount();
		psb_dump_composite_to_browser(canvas, canvasW, canvasH, minX, minY, (int)draws.size());
		perfDumpMs += TVPGetTickCount() - perfDumpStart;
	}
#endif

	// Output composited canvas
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfOutputStart = TVPGetTickCount();
#endif
	sizecallback(callbackdata, canvasW, canvasH);
	for (int y = 0; y < canvasH; y++) {
		void *scanline = scanlinecallback(callbackdata, y);
		if (!scanline) break;
		unsigned char *row = canvas + y * canvasW * 4;
		tjs_uint32 *dst = (tjs_uint32*)scanline;
		for (int x = 0; x < canvasW; x++) {
			unsigned char b = row[x * 4 + 0];
			unsigned char g = row[x * 4 + 1];
			unsigned char r = row[x * 4 + 2];
			unsigned char a = row[x * 4 + 3];
			dst[x] = (a << 24) | (r << 16) | (g << 8) | b;
		}
		scanlinecallback(callbackdata, -1);
	}
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfAfterOutput = TVPGetTickCount();
#endif

	delete[] canvas;
	delete psb;
	delete[] buf;
#ifdef __EMSCRIPTEN__
	tjs_uint64 perfEnd = TVPGetTickCount();
	fprintf(stderr,
		"[PERF] psb.gfx.load ok=1 filesize=%llu read=%u keyidx=%d mode=%d candidates=%zu draws=%zu canvas=%dx%d offset=%d,%d motion_layout=%d capture_bridge=%d size_ms=%llu read_ms=%llu parse_ms=%llu discover_ms=%llu layout_ms=%llu canvas_alloc_ms=%llu decode_ms=%llu alpha_ms=%llu composite_ms=%llu bridge_ms=%llu dump_ms=%llu output_ms=%llu cleanup_ms=%llu total_ms=%llu\n",
		(unsigned long long)filesize,
		(unsigned)read,
		(int)keyidx,
		(int)mode,
		candidates.size(),
		draws.size(),
		canvasW,
		canvasH,
		minX,
		minY,
		usedMotionLayout ? 1 : 0,
		captureBridgeSnapshot ? 1 : 0,
		(unsigned long long)(perfAfterSize - perfStart),
		(unsigned long long)(perfAfterRead - perfReadStart),
		(unsigned long long)(perfAfterParse - perfParseStart),
		(unsigned long long)(perfAfterDiscover - perfDiscoverStart),
		(unsigned long long)(perfAfterLayout - perfLayoutStart),
		(unsigned long long)(perfAfterCanvasAlloc - perfCanvasAllocStart),
		(unsigned long long)perfDecodeMs,
		(unsigned long long)perfAlphaMs,
		(unsigned long long)perfCompositeMs,
		(unsigned long long)perfBridgeMs,
		(unsigned long long)perfDumpMs,
		(unsigned long long)(perfAfterOutput - perfOutputStart),
		(unsigned long long)(perfEnd - perfAfterOutput),
		(unsigned long long)(perfEnd - perfStart));
#endif
}

void TVPRegisterPSBGraphicLoader()
{
	TVPRegisterGraphicLoadingHandler(ttstr(TJS_W(".psb")),
		TVPLoadPSB, nullptr, nullptr, nullptr, nullptr);
	KRKR_LOG_L3("[PSB-GFX] Registered .psb graphic loader\n");
}
