#define NCB_MODULE_NAME TJS_W("layerExSave.dll")
#include "ncbind.hpp"
#include <vector>
#include <cstring>

typedef unsigned char const *BufRefT;
typedef unsigned char       *WrtRefT;

static bool GetLayerSize(iTJSDispatch2 *lay, long &w, long &h, long &pitch)
{
	if (!lay || TJS_FAILED(lay->IsInstanceOf(0, 0, 0, TJS_W("Layer"), lay))) return false;
	tTJSVariant val;
	if (TJS_FAILED(lay->PropGet(0, TJS_W("hasImage"), 0, &val, lay)) || (val.AsInteger() == 0)) return false;
	val.Clear();
	if (TJS_FAILED(lay->PropGet(0, TJS_W("imageWidth"), 0, &val, lay))) return false;
	w = (long)val.AsInteger();
	val.Clear();
	if (TJS_FAILED(lay->PropGet(0, TJS_W("imageHeight"), 0, &val, lay))) return false;
	h = (long)val.AsInteger();
	val.Clear();
	if (TJS_FAILED(lay->PropGet(0, TJS_W("mainImageBufferPitch"), 0, &val, lay))) return false;
	pitch = (long)val.AsInteger();
	return (w > 0 && h > 0 && pitch != 0);
}

static bool GetLayerBufferAndSize(iTJSDispatch2 *lay, long &w, long &h, BufRefT &ptr, long &pitch)
{
	if (!GetLayerSize(lay, w, h, pitch)) return false;
	tTJSVariant val;
	if (TJS_FAILED(lay->PropGet(0, TJS_W("mainImageBuffer"), 0, &val, lay))) return false;
	ptr = reinterpret_cast<BufRefT>(val.AsInteger());
	return (ptr != 0);
}

static bool GetLayerBufferAndSize(iTJSDispatch2 *lay, long &w, long &h, WrtRefT &ptr, long &pitch)
{
	if (!GetLayerSize(lay, w, h, pitch)) return false;
	tTJSVariant val;
	if (TJS_FAILED(lay->PropGet(0, TJS_W("mainImageBufferForWrite"), 0, &val, lay))) return false;
	ptr = reinterpret_cast<WrtRefT>(val.AsInteger());
	return (ptr != 0);
}

static void MakeResult(tTJSVariant *result, long x, long y, long w, long h)
{
	ncbDictionaryAccessor dict;
	dict.SetValue(TJS_W("x"), x);
	dict.SetValue(TJS_W("y"), y);
	dict.SetValue(TJS_W("w"), w);
	dict.SetValue(TJS_W("h"), h);
	*result = dict;
}

static bool CheckTransp(BufRefT p, long next, long count)
{
	for (; count > 0; count--, p+=next) if (p[3] != 0) return true;
	return false;
}

static tjs_error TJS_INTF_METHOD
GetCropRect(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *lay)
{
	BufRefT p, r = 0;
	long w, h, nl, nc = 4;
	if (!GetLayerBufferAndSize(lay, w, h, r, nl))
		TVPThrowExceptionMessage(TJS_W("Invalid layer image."));

	long x1=0, y1=0, x2=w-1, y2=h-1;
	result->Clear();

	for (p=r;             x1 <  w; x1++,p+=nc) if (CheckTransp(p, nl,  h)) break;
	if (x1 >= w) return TJS_S_OK;
	for (p=r+x2*nc;       x2 >= 0; x2--,p-=nc) if (CheckTransp(p, nl,  h)) break;
	long rw = x2 - x1 + 1;
	for (p=r+x1*nc;       y1 <  h; y1++,p+=nl) if (CheckTransp(p, nc, rw)) break;
	for (p=r+x1*nc+y2*nl; y2 >= 0; y2--,p-=nl) if (CheckTransp(p, nc, rw)) break;

	MakeResult(result, x1, y1, rw, y2 - y1 + 1);
	return TJS_S_OK;
}
NCB_ATTACH_FUNCTION(getCropRect, Layer, GetCropRect);

static bool CheckZerop(BufRefT p, long next, long count)
{
	for (; count > 0; count--, p+=next) if (p[3] != 0 || p[2] != 0 || p[1] != 0 || p[0] != 0) return true;
	return false;
}

static tjs_error TJS_INTF_METHOD
GetCropRectZero(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *lay)
{
	BufRefT p, r = 0;
	long w, h, nl, nc = 4;
	if (!GetLayerBufferAndSize(lay, w, h, r, nl))
		TVPThrowExceptionMessage(TJS_W("Invalid layer image."));

	long x1=0, y1=0, x2=w-1, y2=h-1;
	result->Clear();

	for (p=r;             x1 <  w; x1++,p+=nc) if (CheckZerop(p, nl,  h)) break;
	if (x1 >= w) return TJS_S_OK;
	for (p=r+x2*nc;       x2 >= 0; x2--,p-=nc) if (CheckZerop(p, nl,  h)) break;
	long rw = x2 - x1 + 1;
	for (p=r+x1*nc;       y1 <  h; y1++,p+=nl) if (CheckZerop(p, nc, rw)) break;
	for (p=r+x1*nc+y2*nl; y2 >= 0; y2--,p-=nl) if (CheckZerop(p, nc, rw)) break;

	MakeResult(result, x1, y1, rw, y2 - y1 + 1);
	return TJS_S_OK;
}
NCB_ATTACH_FUNCTION(getCropRectZero, Layer, GetCropRectZero);

#define IS_SAME_COLOR(A1,R1,G1,B1, A2,R2,G2,B2) \
	(((A1)==(A2)) && ((A1)==0 || ((R1)==(R2) && (G1)==(G2) && (B1)==(B2))))

static bool CheckDiff(BufRefT p1, long p1n, BufRefT p2, long p2n, long count)
{
	for (; count > 0; count--, p1+=p1n, p2+=p2n)
		if (!IS_SAME_COLOR( p1[3],p1[2],p1[1],p1[0],  p2[3],p2[2],p2[1],p2[0] )) return true;
	return false;
}

static tjs_error TJS_INTF_METHOD
GetDiffRect(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *lay)
{
	if (numparams < 1) return TJS_E_BADPARAMCOUNT;
	iTJSDispatch2 *base = param[0]->AsObjectNoAddRef();

	BufRefT fp, tp, fr = 0, tr = 0;
	long w, h, tnl, fw, fh, fnl, nc = 4;
	if (!GetLayerBufferAndSize(lay,   w,  h, tr, tnl) ||
		!GetLayerBufferAndSize(base, fw, fh, fr, fnl))
		TVPThrowExceptionMessage(TJS_W("Invalid layer image."));

	if (w != fw || h != fh)
		TVPThrowExceptionMessage(TJS_W("Different layer size."));

	long x1=0, y1=0, x2=w-1, y2=h-1;
	result->Clear();

	for (fp=fr,tp=tr;                           x1< w; x1++,fp+=nc,tp+=nc)   if (CheckDiff(fp,fnl,tp,tnl, h)) break;
	if (x1 >= w) return TJS_S_OK;
	for (fp=fr+x2*nc,tp=tr+x2*nc;              x2>=0; x2--,fp-=nc,tp-=nc)   if (CheckDiff(fp,fnl,tp,tnl, h)) break;
	long rw = x2 - x1 + 1;
	for (fp=fr+x1*nc,tp=tr+x1*nc;              y1< h; y1++,fp+=fnl,tp+=tnl) if (CheckDiff(fp,nc,tp,nc,  rw)) break;
	for (fp=fr+x1*nc+y2*fnl,tp=tr+x1*nc+y2*tnl; y2>=0; y2--,fp-=fnl,tp-=tnl) if (CheckDiff(fp,nc,tp,nc,  rw)) break;

	MakeResult(result, x1, y1, rw, y2 - y1 + 1);
	return TJS_S_OK;
}
NCB_ATTACH_FUNCTION(getDiffRect, Layer, GetDiffRect);

static tjs_error TJS_INTF_METHOD
GetDiffPixel(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *lay)
{
	tjs_uint32 scol = 0, dcol = 0;
	bool sfill = false, dfill = false;
	tTVInteger count = 0;

	if (numparams < 1) return TJS_E_BADPARAMCOUNT;

	if (numparams >= 2 && param[1]->Type() != tvtVoid) {
		scol = (tjs_uint32)(param[1]->AsInteger());
		sfill = true;
	}
	if (numparams >= 3 && param[2]->Type() != tvtVoid) {
		dcol = (tjs_uint32)(param[2]->AsInteger());
		dfill = true;
	}

	iTJSDispatch2 *base = param[0]->AsObjectNoAddRef();

	BufRefT fp, fr = 0;
	WrtRefT tp, tr = 0;
	long w, h, tnl, fw, fh, fnl, nc = 4;
	if (!GetLayerBufferAndSize(lay,   w,  h, tr, tnl) ||
		!GetLayerBufferAndSize(base, fw, fh, fr, fnl))
		TVPThrowExceptionMessage(TJS_W("Invalid layer image."));

	if (w != fw || h != fh)
		TVPThrowExceptionMessage(TJS_W("Different layer size."));

	for (long y = 0; (fp=fr, tp=tr, y < h); y++, fr+=fnl, tr+=tnl) {
		for (long x = 0; x < w; x++, fp+=nc, tp+=nc) {
			bool same = IS_SAME_COLOR(fp[3],fp[2],fp[1],fp[0], tp[3],tp[2],tp[1],tp[0]);
			if (      same &&     sfill) *(tjs_uint32*)tp = scol;
			else if (!same) { if (dfill) *(tjs_uint32*)tp = dcol; count++; }
		}
	}
	if (result) *result = count;
	return TJS_S_OK;
}
NCB_ATTACH_FUNCTION(getDiffPixel, Layer, GetDiffPixel);

static inline void AddColor(tjs_uint32 &r, tjs_uint32 &g, tjs_uint32 &b, BufRefT p) {
	r += p[2]; g += p[1]; b += p[0];
}

static tjs_error TJS_INTF_METHOD
OozeColor(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *lay)
{
	if (numparams < 1) return TJS_E_BADPARAMCOUNT;
	int level = (int)(param[0]->AsInteger());
	if (level <= 0)
		TVPThrowExceptionMessage(TJS_W("Invalid level count."));
	unsigned char threshold = (unsigned char)(numparams > 1 ? param[1]->AsInteger() : 1);
	unsigned long fillColor = (unsigned long)(numparams > 2 ? param[2]->AsInteger() : 0);
	unsigned char fillR = (unsigned char)((fillColor >> 16) & 0xff);
	unsigned char fillG = (unsigned char)((fillColor >> 8) & 0xff);
	unsigned char fillB = (unsigned char)((fillColor) & 0xff);
	if (threshold < 1) threshold = 1;

	WrtRefT p, r = 0;
	long x, y, w, h, nl, nc = 4, ow, oh;
	if (!GetLayerBufferAndSize(lay, w, h, r, nl))
		TVPThrowExceptionMessage(TJS_W("Invalid layer image."));

	ow = w+2; oh = h+2;
	char *o, *otop, *oozed = new char[ow*oh];
	otop = oozed + ow + 1;
	memset(oozed, 0, ow*oh);
	try {
		for (y = 0; y < h; y++) {
			o = otop + y*ow;
			p = r    + y*nl;
			for (x = 0; x < w; x++, o++, p+=nc) {
				if (p[3] >= threshold) *o = -1;
				else { p[2] = fillR; p[1] = fillG; p[0] = fillB; }
			}
		}
		for (int i = 0; i < level; i++) {
			bool L, R, U, D;
			for (y = 0; y < h; y++) {
				o = otop + y*ow;
				p = r    + y*nl;
				for (x = 0; x < w; x++, p+=nc, o++) {
					if (!*o) {
						tjs_uint32 cr = 0, cg = 0, cb = 0;
						U=o[-ow]<0; D=o[ow]<0; L=o[-1]<0; R=o[1]<0;
						if (U || D || L || R) {
							int cnt = 0;
							if (U) { AddColor(cr, cg, cb, p-nl); cnt++; }
							if (D) { AddColor(cr, cg, cb, p+nl); cnt++; }
							if (L) { AddColor(cr, cg, cb, p-nc); cnt++; }
							if (R) { AddColor(cr, cg, cb, p+nc); cnt++; }
							p[2] = (unsigned char)(cr / cnt);
							p[1] = (unsigned char)(cg / cnt);
							p[0] = (unsigned char)(cb / cnt);
							*o = 1;
						}
					}
				}
			}
			for (y = 0; y < h; y++) {
				o = otop + y*ow;
				for (x = 0; x < w; x++, o++) if (*o>0) *o=-1;
			}
		}
	} catch (...) {
		delete[] oozed;
		throw;
	}
	delete[] oozed;
	return TJS_S_OK;
}
NCB_ATTACH_FUNCTION(oozeColor, Layer, OozeColor);

static tjs_error TJS_INTF_METHOD
CopyBlueToAlpha(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *lay)
{
	if (numparams < 1) return TJS_E_BADPARAMCOUNT;

	BufRefT sbuf = 0;
	long sw, sh, spitch;
	if (!GetLayerBufferAndSize(param[0]->AsObjectNoAddRef(), sw, sh, sbuf, spitch))
		TVPThrowExceptionMessage(TJS_W("src must be Layer."));

	WrtRefT dbuf = 0;
	long dw, dh, dpitch;
	if (!GetLayerBufferAndSize(lay, dw, dh, dbuf, dpitch))
		TVPThrowExceptionMessage(TJS_W("dest must be Layer."));

	int w = (sw < dw ? sw : dw);
	int h = sh < dh ? sh : dh;
	for (int i=0; i<h; i++) {
		BufRefT p = sbuf;
		WrtRefT q = dbuf+3;
		for (int j=0; j<w; j++) { *q = *p; p += 4; q += 4; }
		sbuf += spitch;
		dbuf += dpitch;
	}
	return TJS_S_OK;
}
NCB_ATTACH_FUNCTION(copyBlueToAlpha, Layer, CopyBlueToAlpha);

static tjs_error TJS_INTF_METHOD
isBlank(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	if (numparams < 4) return TJS_E_BADPARAMCOUNT;

	BufRefT sbuf = 0;
	long sw, sh, spitch;
	if (!GetLayerBufferAndSize(objthis, sw, sh, sbuf, spitch))
		TVPThrowExceptionMessage(TJS_W("src must be Layer."));

	tjs_int left   = *param[0];
	tjs_int top    = *param[1];
	tjs_int width  = *param[2];
	tjs_int height = *param[3];

	if (left < 0) { width += left; left = 0; }
	if (top  < 0) { height += top; top  = 0; }
	tjs_int cut;
	if ((cut = left + width  - sw) > 0) width  -= cut;
	if ((cut = top  + height - sh) > 0) height -= cut;

	if (width >= 0 && height >= 0) {
		for (tjs_int y = top; y < top + height; y++) {
			BufRefT buffer = sbuf + left * 4 + spitch * y;
			for (tjs_int x = left; x < left + width; x++, buffer += 4) {
				if (*buffer) {
					if (result) *result = 0;
					return TJS_S_OK;
				}
			}
		}
	}
	if (result) *result = 1;
	return TJS_S_OK;
}
NCB_ATTACH_FUNCTION(isBlank, Layer, isBlank);

static tjs_error TJS_INTF_METHOD
clearAlpha(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *lay)
{
	int threthold = numparams <= 0 ? 0 : (int)param[0]->AsInteger();
	unsigned long fillColor = (unsigned long)((numparams > 1 ? param[1]->AsInteger() : 0) & 0xffffff);

	WrtRefT dbuf = 0;
	long w, h, pitch;
	if (!GetLayerBufferAndSize(lay, w, h, dbuf, pitch))
		TVPThrowExceptionMessage(TJS_W("dest must be Layer."));

	for (int i=0; i<h; i++) {
		WrtRefT q = dbuf;
		for (int j=0; j<w; j++) {
			if (q[3] <= threthold) *(unsigned long*)q = fillColor;
			q += 4;
		}
		dbuf += pitch;
	}
	return TJS_S_OK;
}
NCB_ATTACH_FUNCTION(clearAlpha, Layer, clearAlpha);

static tjs_error TJS_INTF_METHOD
getAverageColor(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	if (numparams < 4) return TJS_E_BADPARAMCOUNT;

	BufRefT sbuf = 0;
	long sw, sh, spitch;
	if (!GetLayerBufferAndSize(objthis, sw, sh, sbuf, spitch))
		TVPThrowExceptionMessage(TJS_W("src must be Layer."));

	tjs_int left   = *param[0];
	tjs_int top    = *param[1];
	tjs_int width  = *param[2];
	tjs_int height = *param[3];

	if (left < 0) { width += left; left = 0; }
	if (top  < 0) { height += top; top  = 0; }
	tjs_int cut;
	if ((cut = left + width  - sw) > 0) width  -= cut;
	if ((cut = top  + height - sh) > 0) height -= cut;

	if (width <= 0 || height <= 0)
		TVPThrowExceptionMessage(TJS_W("invalid layer range"));

	double a = 0, r = 0, g = 0, b = 0;
	double size = width * height;

	for (tjs_int y = top; y < top + height; y++) {
		BufRefT buffer = sbuf + left * 4 + spitch * y;
		for (tjs_int x = left; x < left + width; x++, buffer += 4) {
			a += buffer[0]; r += buffer[1]; g += buffer[2]; b += buffer[3];
		}
	}
	a /= size; r /= size; g /= size; b /= size;
	tjs_uint32 color = (((tjs_uint32)a & 0xff) << 24 |
						((tjs_uint32)r & 0xff) << 16 |
						((tjs_uint32)g & 0xff) << 8 |
						((tjs_uint32)b & 0xff));
	if (result) *result = (tjs_int)color;
	return TJS_S_OK;
}
NCB_ATTACH_FUNCTION(getAverageColor, Layer, getAverageColor);
