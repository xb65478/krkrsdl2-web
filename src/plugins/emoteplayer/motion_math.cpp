#include "motion_math.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace emoteplayer {

tjs_real MotionClamp01(tjs_real value)
{
	if (value < 0.0) return 0.0;
	if (value > 1.0) return 1.0;
	return value;
}

tjs_int MotionAlphaFromOpacity(tjs_real opacity)
{
	return (tjs_int)std::lround(MotionClamp01(opacity) * 255.0);
}

MotionAffine2D MotionAffineIdentity()
{
	return MotionAffine2D();
}

MotionMat4 MotionMat4Identity()
{
	return MotionMat4();
}

MotionMat4 MotionMat4Multiply(const MotionMat4 &lhs, const MotionMat4 &rhs)
{
	MotionMat4 out;
	for (int row = 0; row < 4; row++) {
		for (int col = 0; col < 4; col++) {
			tjs_real value = 0.0;
			for (int k = 0; k < 4; k++) {
				value += lhs.m[row * 4 + k] * rhs.m[k * 4 + col];
			}
			out.m[row * 4 + col] = value;
		}
	}
	return out;
}

void MotionMat4TransformPoint(const MotionMat4 &mat,
	tjs_real x, tjs_real y, tjs_real z, tjs_real w,
	tjs_real &outX, tjs_real &outY, tjs_real &outZ, tjs_real &outW)
{
	outX = mat.m[0] * x + mat.m[1] * y + mat.m[2] * z + mat.m[3] * w;
	outY = mat.m[4] * x + mat.m[5] * y + mat.m[6] * z + mat.m[7] * w;
	outZ = mat.m[8] * x + mat.m[9] * y + mat.m[10] * z + mat.m[11] * w;
	outW = mat.m[12] * x + mat.m[13] * y + mat.m[14] * z + mat.m[15] * w;
}

bool MotionMat4AlmostEqual(const MotionMat4 &a, const MotionMat4 &b,
	tjs_real epsilon, tjs_real &maxAbsError)
{
	bool ok = true;
	maxAbsError = 0.0;
	for (int i = 0; i < 16; i++) {
		tjs_real err = std::abs(a.m[i] - b.m[i]);
		if (err > maxAbsError) maxAbsError = err;
		if (err > epsilon) ok = false;
	}
	return ok;
}

MotionMat4 MotionMat4Translate(tjs_real tx, tjs_real ty, tjs_real tz)
{
	MotionMat4 out = MotionMat4Identity();
	out.m[3] = tx;
	out.m[7] = ty;
	out.m[11] = tz;
	return out;
}

MotionMat4 MotionMat4Scale(tjs_real sx, tjs_real sy, tjs_real sz)
{
	MotionMat4 out = MotionMat4Identity();
	out.m[0] = sx;
	out.m[5] = sy;
	out.m[10] = sz;
	return out;
}

MotionMat4 MotionMat4RotateZDeg(tjs_real angleDeg)
{
	MotionMat4 out = MotionMat4Identity();
	tjs_real rad = angleDeg * 3.141592653589793 / 180.0;
	tjs_real cosA = std::cos(rad);
	tjs_real sinA = std::sin(rad);
	out.m[0] = cosA;
	out.m[1] = -sinA;
	out.m[4] = sinA;
	out.m[5] = cosA;
	return out;
}

MotionMat4 MotionMat4Ortho(tjs_real left, tjs_real right,
	tjs_real bottom, tjs_real top, tjs_real nearZ, tjs_real farZ)
{
	MotionMat4 out = MotionMat4Identity();
	tjs_real rw = right - left;
	tjs_real th = top - bottom;
	tjs_real fn = farZ - nearZ;
	if (std::abs(rw) < 0.0000001 || std::abs(th) < 0.0000001 ||
		std::abs(fn) < 0.0000001) {
		return out;
	}
	out.m[0] = 2.0 / rw;
	out.m[5] = 2.0 / th;
	out.m[10] = -2.0 / fn;
	out.m[3] = -(right + left) / rw;
	out.m[7] = -(top + bottom) / th;
	out.m[11] = -(farZ + nearZ) / fn;
	return out;
}

bool MotionMat4Inverse(const MotionMat4 &in, MotionMat4 &out)
{
	tjs_real a[4][8];
	for (int row = 0; row < 4; row++) {
		for (int col = 0; col < 4; col++) {
			a[row][col] = in.m[row * 4 + col];
			a[row][col + 4] = row == col ? 1.0 : 0.0;
		}
	}

	for (int col = 0; col < 4; col++) {
		int pivot = col;
		tjs_real best = std::abs(a[col][col]);
		for (int row = col + 1; row < 4; row++) {
			tjs_real candidate = std::abs(a[row][col]);
			if (candidate > best) {
				best = candidate;
				pivot = row;
			}
		}
		if (best < 0.0000001) {
			out = MotionMat4Identity();
			return false;
		}
		if (pivot != col) {
			for (int k = 0; k < 8; k++) std::swap(a[col][k], a[pivot][k]);
		}
		tjs_real div = a[col][col];
		for (int k = 0; k < 8; k++) a[col][k] /= div;
		for (int row = 0; row < 4; row++) {
			if (row == col) continue;
			tjs_real factor = a[row][col];
			if (std::abs(factor) < 0.0000001) continue;
			for (int k = 0; k < 8; k++) a[row][k] -= factor * a[col][k];
		}
	}

	for (int row = 0; row < 4; row++) {
		for (int col = 0; col < 4; col++) {
			out.m[row * 4 + col] = a[row][col + 4];
		}
	}
	return true;
}

MotionMat4 MotionMat4FromAffine2D(const MotionAffine2D &affine)
{
	MotionMat4 out = MotionMat4Identity();
	out.m[0] = affine.m00;
	out.m[1] = affine.m01;
	out.m[3] = affine.tx;
	out.m[4] = affine.m10;
	out.m[5] = affine.m11;
	out.m[7] = affine.ty;
	return out;
}

MotionMat4 MotionMat4BuildModelFromFrame(const MotionFrameLayoutState &frame)
{
	MotionMat4 model = MotionMat4Identity();
	model = MotionMat4Multiply(model,
		MotionMat4Translate(frame.coordX, frame.coordY, frame.coordZ));
	model = MotionMat4Multiply(model, MotionMat4RotateZDeg(frame.angle));
	MotionMat4 shear = MotionMat4Identity();
	shear.m[1] = frame.sx;
	shear.m[4] = frame.sy;
	model = MotionMat4Multiply(shear, model);
	model = MotionMat4Multiply(model,
		MotionMat4Scale(frame.scaleX, frame.scaleY, 1.0));
	return model;
}

MotionMat4 MotionMat4ApplyIconLocal(const MotionMat4 &base,
	tjs_real originX, tjs_real originY,
	tjs_real offsetX, tjs_real offsetY,
	tjs_real width, tjs_real height)
{
	MotionMat4 out = MotionMat4Multiply(base,
		MotionMat4Translate(-originX - offsetX, -originY - offsetY, 0.0));
	out = MotionMat4Multiply(out, MotionMat4Scale(width, height, 1.0));
	return out;
}

MotionAffine2D MotionAffineMultiply(const MotionAffine2D &lhs, const MotionAffine2D &rhs)
{
	MotionAffine2D out;
	out.m00 = lhs.m00 * rhs.m00 + lhs.m01 * rhs.m10;
	out.m01 = lhs.m00 * rhs.m01 + lhs.m01 * rhs.m11;
	out.m10 = lhs.m10 * rhs.m00 + lhs.m11 * rhs.m10;
	out.m11 = lhs.m10 * rhs.m01 + lhs.m11 * rhs.m11;
	out.tx = lhs.m00 * rhs.tx + lhs.m01 * rhs.ty + lhs.tx;
	out.ty = lhs.m10 * rhs.tx + lhs.m11 * rhs.ty + lhs.ty;
	return out;
}

MotionAffine2D MotionAffineTranslate(tjs_real tx, tjs_real ty)
{
	MotionAffine2D out = MotionAffineIdentity();
	out.tx = tx;
	out.ty = ty;
	return out;
}

MotionAffine2D MotionAffineScale(tjs_real sx, tjs_real sy)
{
	MotionAffine2D out = MotionAffineIdentity();
	out.m00 = sx;
	out.m11 = sy;
	return out;
}

MotionAffine2D MotionAffineRotateDeg(tjs_real angleDeg)
{
	MotionAffine2D out = MotionAffineIdentity();
	tjs_real rad = angleDeg * 3.141592653589793 / 180.0;
	tjs_real cosA = std::cos(rad);
	tjs_real sinA = std::sin(rad);
	out.m00 = cosA;
	out.m01 = -sinA;
	out.m10 = sinA;
	out.m11 = cosA;
	return out;
}

MotionAffine2D MotionAffineShear(tjs_real sx, tjs_real sy)
{
	MotionAffine2D out = MotionAffineIdentity();
	out.m01 = sx;
	out.m10 = sy;
	return out;
}

MotionAffine2D MotionBuildLayoutLocalAffine(const MotionFrameLayoutState &frame)
{
	MotionAffine2D model = MotionAffineIdentity();
	model = MotionAffineMultiply(model, MotionAffineTranslate(frame.coordX, frame.coordY));
	model = MotionAffineMultiply(model, MotionAffineRotateDeg(frame.angle));
	model = MotionAffineMultiply(MotionAffineShear(frame.sx, frame.sy), model);
	model = MotionAffineMultiply(model, MotionAffineScale(frame.scaleX, frame.scaleY));
	return model;
}

void MotionAffineApply(const MotionAffine2D &affine,
	tjs_real x, tjs_real y, tjs_real &outX, tjs_real &outY)
{
	outX = affine.m00 * x + affine.m01 * y + affine.tx;
	outY = affine.m10 * x + affine.m11 * y + affine.ty;
}

void MotionApplyAffineToPoint(
	tjs_real u, tjs_real v,
	tjs_real ox, tjs_real oy,
	tjs_real iconW, tjs_real iconH,
	tjs_real iconOriginX, tjs_real iconOriginY,
	tjs_real scaleX, tjs_real scaleY,
	tjs_real sx, tjs_real sy,
	tjs_real angleDeg,
	tjs_real coordX, tjs_real coordY,
	tjs_real &outX, tjs_real &outY)
{
	tjs_real px = u * iconW;
	tjs_real py = v * iconH;
	px += (-iconOriginX - ox);
	py += (-iconOriginY - oy);
	MotionFrameLayoutState frame;
	frame.hasContent = true;
	frame.coordX = coordX;
	frame.coordY = coordY;
	frame.scaleX = scaleX;
	frame.scaleY = scaleY;
	frame.sx = sx;
	frame.sy = sy;
	frame.angle = angleDeg;
	MotionAffine2D model = MotionBuildLayoutLocalAffine(frame);
	MotionAffineApply(model, px, py, outX, outY);
}

bool MotionAffineIsIdentity(
	tjs_real angle, tjs_real sx, tjs_real sy, tjs_real ox, tjs_real oy)
{
	return std::abs(angle) < 0.0001 &&
		std::abs(sx) < 0.0001 &&
		std::abs(sy) < 0.0001 &&
		std::abs(ox) < 0.0001 &&
		std::abs(oy) < 0.0001;
}

bool MotionAffineHasNonAxisAlignedShape(const MotionAffine2D &affine)
{
	return std::abs(affine.m01) > 0.0001 || std::abs(affine.m10) > 0.0001;
}

bool MotionLayoutNeedsAffinePath(const MotionLayoutInfo *layout)
{
	if (!layout) return false;
	if (!layout->affineValid) return false;
	if (std::abs(layout->angle) > 0.0001) return true;
	if (std::abs(layout->sx) > 0.0001 || std::abs(layout->sy) > 0.0001) return true;
	if (std::abs(layout->ox) > 0.0001 || std::abs(layout->oy) > 0.0001) return true;
	if (layout->iconWidth > 0.0 &&
		(std::abs(layout->iconOriginX) > 0.0001 || std::abs(layout->iconOriginY) > 0.0001)) {
		return true;
	}
	return false;
}

MotionAffine2D MotionBuildIconDrawAffine(const MotionAffine2D &nodeAffine,
	tjs_real ox, tjs_real oy,
	tjs_real iconW, tjs_real iconH,
	tjs_real iconOriginX, tjs_real iconOriginY)
{
	MotionAffine2D draw = nodeAffine;
	draw = MotionAffineMultiply(draw,
		MotionAffineTranslate(-iconOriginX - ox, -iconOriginY - oy));
	draw = MotionAffineMultiply(draw, MotionAffineScale(iconW, iconH));
	return draw;
}

void MotionApplyIconDrawAffineToPoint(const MotionAffine2D &draw,
	tjs_real u, tjs_real v, tjs_real &outX, tjs_real &outY)
{
	MotionAffineApply(draw, u, v, outX, outY);
}

MotionRenderSurface MotionBuildSingleRenderSurface(
	const MotionRenderMethodItem &item)
{
	MotionRenderSurface surface;
	surface.matTrans = item.affineValid ?
		MotionMat4FromAffine2D(item.affine) : MotionMat4Identity();
	surface.originX = item.originX;
	surface.originY = item.originY;
	surface.originW = item.width;
	surface.originH = item.height;
	surface.opa = item.opacity;
	surface.type = item.type;
	for (size_t k = 0; k < 32; k++) surface.controlPts[k] = item.controlPts[k];
	surface.hasStencil = item.type == 12;
	surface.currCoordz = (tjs_real)item.depth;
	surface.label = item.label;
	return surface;
}

MotionRenderSurface MotionBuildLayoutRenderSurface(const MotionMat4 &matTrans,
	const ttstr &label,
	tjs_real originX, tjs_real originY,
	tjs_real width, tjs_real height,
	tjs_real coordZ)
{
	MotionRenderSurface surface;
	surface.matTrans = matTrans;
	surface.originX = originX;
	surface.originY = originY;
	surface.originW = width;
	surface.originH = height;
	surface.opa = 1.0;
	surface.type = 3;
	surface.hasStencil = false;
	surface.currCoordz = coordZ;
	surface.label = label;
	return surface;
}

MotionRenderSurface MotionBuildDrawRenderSurface(const MotionMat4 &matTrans,
	const MotionRenderMethodItem &item,
	tjs_real coordZ)
{
	MotionRenderSurface surface;
	surface.matTrans = matTrans;
	surface.originX = item.originX;
	surface.originY = item.originY;
	surface.originW = item.width;
	surface.originH = item.height;
	surface.opa = item.opacity;
	surface.type = item.type;
	for (size_t k = 0; k < 32; k++) surface.controlPts[k] = item.controlPts[k];
	surface.hasStencil = item.type == 12;
	surface.currCoordz = coordZ;
	surface.label = item.label;
	return surface;
}

MotionMat4 MotionBuildDemuxMatFromSurface(
	const MotionRenderSurface &parentSurface,
	bool includeInverseProjection,
	tjs_real limOriginX, tjs_real limOriginY,
	tjs_real limWidth, tjs_real limHeight,
	tjs_real limZMax)
{
	MotionMat4 demuxMat = MotionMat4Multiply(parentSurface.matTrans,
		MotionMat4Scale(
			std::abs(parentSurface.originW) > 0.0000001 ? 1.0 / parentSurface.originW : 1.0,
			std::abs(parentSurface.originH) > 0.0000001 ? 1.0 / parentSurface.originH : 1.0,
			1.0));
	demuxMat = MotionMat4Multiply(demuxMat,
		MotionMat4Translate(parentSurface.originX, parentSurface.originY, 0.0));
	if (includeInverseProjection) {
		MotionMat4 projection = MotionMat4Ortho(-limOriginX, limWidth - limOriginX,
			limHeight - limOriginY, -limOriginY, limZMax, -limZMax);
		MotionMat4 inverseProjection;
		if (MotionMat4Inverse(projection, inverseProjection)) {
			demuxMat = MotionMat4Multiply(inverseProjection, demuxMat);
		}
	}
	return demuxMat;
}

void MotionRefreshRenderMethodItemSurface(MotionRenderMethodItem &item)
{
	item.surfaces.clear();
	item.surfaces.push_back(MotionBuildSingleRenderSurface(item));
}

tjs_real MotionBezierBasis0(tjs_real t)
{
	return (1.0 - t) * (1.0 - t) * (1.0 - t);
}
tjs_real MotionBezierBasis1(tjs_real t)
{
	return 3.0 * t * (1.0 - t) * (1.0 - t);
}
tjs_real MotionBezierBasis2(tjs_real t)
{
	return 3.0 * t * t * (1.0 - t);
}
tjs_real MotionBezierBasis3(tjs_real t)
{
	return t * t * t;
}


void MotionEvalBezierSurface(const tjs_real controlPts[32],
	tjs_real u, tjs_real v, tjs_real &outU, tjs_real &outV)
{
	tjs_real bu[4];
	bu[0] = MotionBezierBasis0(u);
	bu[1] = MotionBezierBasis1(u);
	bu[2] = MotionBezierBasis2(u);
	bu[3] = MotionBezierBasis3(u);
	tjs_real bv[4];
	bv[0] = MotionBezierBasis0(v);
	bv[1] = MotionBezierBasis1(v);
	bv[2] = MotionBezierBasis2(v);
	bv[3] = MotionBezierBasis3(v);
	outU = 0.0;
	outV = 0.0;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			tjs_real basis = bu[i] * bv[j];
			int idx = (i * 4 + j) * 2;
			outU += controlPts[idx] * basis;
			outV += controlPts[idx + 1] * basis;
		}
	}
}

bool MotionControlPtsAreIdentity(const tjs_real controlPts[32])
{
	for (int i = 0; i < 32; i++) {
		if (std::abs(controlPts[i] - kMotionDefaultControlPoints[i]) > 0.0001)
			return false;
	}
	return true;
}

bool MotionEvalSurfaceChainVertex(
	const std::vector<MotionRenderSurface> &surfaces,
	tjs_real u, tjs_real v,
	tjs_real viewportWidth, tjs_real viewportHeight,
	tjs_real &outPixelX, tjs_real &outPixelY,
	tjs_real *outNdcX, tjs_real *outNdcY,
	tjs_int *outRemapCount,
	tjs_real *outNdcZ, tjs_real *outPixelZ)
{
	if (surfaces.empty()) return false;
	tjs_real lastX = u;
	tjs_real lastY = v;
	tjs_real lastZ = 0.0;
	tjs_real lastW = 0.0;
	tjs_int remaps = 0;
	for (std::vector<MotionRenderSurface>::const_reverse_iterator it = surfaces.rbegin();
		it != surfaces.rend(); ++it) {
		if (it != surfaces.rbegin()) {
			tjs_real remapX = 1.0 - (lastY / 2.0 + 0.5);
			tjs_real remapY = 0.5 + lastX / 2.0;
			lastX = remapX;
			lastY = remapY;
			remaps++;
		}
		tjs_real surfaceX = lastX;
		tjs_real surfaceY = lastY;
		const tjs_real *controlPts =
			(it->type == 1) ? it->controlPts : kMotionDefaultControlPoints;
		MotionEvalBezierSurface(controlPts, lastX, lastY, surfaceX, surfaceY);
		tjs_real tx, ty, tz, tw;
		MotionMat4TransformPoint(it->matTrans, surfaceX, surfaceY, 0.0, 1.0,
			tx, ty, tz, tw);
		lastX = tx;
		lastY = ty;
		lastZ = tz;
		lastW = tw;
	}
	if (std::abs(lastW) > 0.0000001 && std::abs(lastW - 1.0) > 0.0000001) {
		lastX /= lastW;
		lastY /= lastW;
		lastZ /= lastW;
	}
	tjs_real ndcX = lastX;
	tjs_real ndcY = -lastY;
	tjs_real ndcZ = lastZ;
	if (!std::isfinite((double)ndcX) || !std::isfinite((double)ndcY)) return false;
	if (outNdcX) *outNdcX = ndcX;
	if (outNdcY) *outNdcY = ndcY;
	if (outNdcZ) *outNdcZ = ndcZ;
	if (outRemapCount) *outRemapCount = remaps;
	tjs_real vw = viewportWidth > 0.0 ? viewportWidth : 1280.0;
	tjs_real vh = viewportHeight > 0.0 ? viewportHeight : 720.0;
	outPixelX = (ndcX * 0.5 + 0.5) * vw;
	outPixelY = (ndcY * 0.5 + 0.5) * vh;
	if (outPixelZ) *outPixelZ = ndcZ;
	return std::isfinite((double)outPixelX) && std::isfinite((double)outPixelY);
}

void MotionCollectAffineQuadCorners(
	tjs_real ox, tjs_real oy,
	tjs_real iconW, tjs_real iconH,
	tjs_real iconOriginX, tjs_real iconOriginY,
	tjs_real scaleX, tjs_real scaleY,
	tjs_real sx, tjs_real sy,
	tjs_real angleDeg,
	tjs_real coordX, tjs_real coordY,
	tjs_real &minX, tjs_real &minY, tjs_real &maxX, tjs_real &maxY)
{
	tjs_real qx[4], qy[4];
	MotionApplyAffineToPoint(0.0, 0.0, ox, oy, iconW, iconH,
		iconOriginX, iconOriginY, scaleX, scaleY, sx, sy, angleDeg,
		coordX, coordY, qx[0], qy[0]);
	MotionApplyAffineToPoint(1.0, 0.0, ox, oy, iconW, iconH,
		iconOriginX, iconOriginY, scaleX, scaleY, sx, sy, angleDeg,
		coordX, coordY, qx[1], qy[1]);
	MotionApplyAffineToPoint(1.0, 1.0, ox, oy, iconW, iconH,
		iconOriginX, iconOriginY, scaleX, scaleY, sx, sy, angleDeg,
		coordX, coordY, qx[2], qy[2]);
	MotionApplyAffineToPoint(0.0, 1.0, ox, oy, iconW, iconH,
		iconOriginX, iconOriginY, scaleX, scaleY, sx, sy, angleDeg,
		coordX, coordY, qx[3], qy[3]);
	minX = qx[0]; maxX = qx[0];
	minY = qy[0]; maxY = qy[0];
	for (int qi = 1; qi < 4; qi++) {
		if (qx[qi] < minX) minX = qx[qi];
		if (qx[qi] > maxX) maxX = qx[qi];
		if (qy[qi] < minY) minY = qy[qi];
		if (qy[qi] > maxY) maxY = qy[qi];
	}
}

void MotionCollectAffineQuadCorners(const MotionAffine2D &draw,
	tjs_real &minX, tjs_real &minY, tjs_real &maxX, tjs_real &maxY)
{
	tjs_real qx[4], qy[4];
	MotionApplyIconDrawAffineToPoint(draw, 0.0, 0.0, qx[0], qy[0]);
	MotionApplyIconDrawAffineToPoint(draw, 1.0, 0.0, qx[1], qy[1]);
	MotionApplyIconDrawAffineToPoint(draw, 1.0, 1.0, qx[2], qy[2]);
	MotionApplyIconDrawAffineToPoint(draw, 0.0, 1.0, qx[3], qy[3]);
	minX = qx[0]; maxX = qx[0];
	minY = qy[0]; maxY = qy[0];
	for (int qi = 1; qi < 4; qi++) {
		if (qx[qi] < minX) minX = qx[qi];
		if (qx[qi] > maxX) maxX = qx[qi];
		if (qy[qi] < minY) minY = qy[qi];
		if (qy[qi] > maxY) maxY = qy[qi];
	}
}

} // namespace emoteplayer
