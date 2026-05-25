#pragma once

#include "motion_types.h"

#include <vector>

namespace emoteplayer {

tjs_real MotionClamp01(tjs_real value);
tjs_int MotionAlphaFromOpacity(tjs_real opacity);

MotionAffine2D MotionAffineIdentity();
MotionAffine2D MotionAffineMultiply(const MotionAffine2D &lhs, const MotionAffine2D &rhs);
MotionAffine2D MotionAffineTranslate(tjs_real tx, tjs_real ty);
MotionAffine2D MotionAffineScale(tjs_real sx, tjs_real sy);
MotionAffine2D MotionAffineRotateDeg(tjs_real angleDeg);
MotionAffine2D MotionAffineShear(tjs_real sx, tjs_real sy);
MotionAffine2D MotionBuildLayoutLocalAffine(const MotionFrameLayoutState &frame);
void MotionAffineApply(const MotionAffine2D &affine,
	tjs_real x, tjs_real y, tjs_real &outX, tjs_real &outY);
void MotionApplyAffineToPoint(
	tjs_real u, tjs_real v,
	tjs_real ox, tjs_real oy,
	tjs_real iconW, tjs_real iconH,
	tjs_real iconOriginX, tjs_real iconOriginY,
	tjs_real scaleX, tjs_real scaleY,
	tjs_real sx, tjs_real sy,
	tjs_real angleDeg,
	tjs_real coordX, tjs_real coordY,
	tjs_real &outX, tjs_real &outY);
bool MotionAffineIsIdentity(
	tjs_real angle, tjs_real sx, tjs_real sy, tjs_real ox, tjs_real oy);
bool MotionAffineHasNonAxisAlignedShape(const MotionAffine2D &affine);
bool MotionLayoutNeedsAffinePath(const MotionLayoutInfo *layout);
MotionAffine2D MotionBuildIconDrawAffine(const MotionAffine2D &nodeAffine,
	tjs_real ox, tjs_real oy,
	tjs_real iconW, tjs_real iconH,
	tjs_real iconOriginX, tjs_real iconOriginY);
void MotionApplyIconDrawAffineToPoint(const MotionAffine2D &draw,
	tjs_real u, tjs_real v, tjs_real &outX, tjs_real &outY);
void MotionCollectAffineQuadCorners(
	tjs_real ox, tjs_real oy,
	tjs_real iconW, tjs_real iconH,
	tjs_real iconOriginX, tjs_real iconOriginY,
	tjs_real scaleX, tjs_real scaleY,
	tjs_real sx, tjs_real sy,
	tjs_real angleDeg,
	tjs_real coordX, tjs_real coordY,
	tjs_real &minX, tjs_real &minY, tjs_real &maxX, tjs_real &maxY);
void MotionCollectAffineQuadCorners(const MotionAffine2D &draw,
	tjs_real &minX, tjs_real &minY, tjs_real &maxX, tjs_real &maxY);

MotionMat4 MotionMat4Identity();
MotionMat4 MotionMat4Multiply(const MotionMat4 &lhs, const MotionMat4 &rhs);
void MotionMat4TransformPoint(const MotionMat4 &mat,
	tjs_real x, tjs_real y, tjs_real z, tjs_real w,
	tjs_real &outX, tjs_real &outY, tjs_real &outZ, tjs_real &outW);
bool MotionMat4AlmostEqual(const MotionMat4 &a, const MotionMat4 &b,
	tjs_real epsilon, tjs_real &maxAbsError);
MotionMat4 MotionMat4Translate(tjs_real tx, tjs_real ty, tjs_real tz = 0.0);
MotionMat4 MotionMat4Scale(tjs_real sx, tjs_real sy, tjs_real sz = 1.0);
MotionMat4 MotionMat4RotateZDeg(tjs_real angleDeg);
MotionMat4 MotionMat4Ortho(tjs_real left, tjs_real right,
	tjs_real bottom, tjs_real top, tjs_real nearZ = -1.0, tjs_real farZ = 1.0);
bool MotionMat4Inverse(const MotionMat4 &in, MotionMat4 &out);
MotionMat4 MotionMat4FromAffine2D(const MotionAffine2D &affine);
MotionMat4 MotionMat4BuildModelFromFrame(const MotionFrameLayoutState &frame);
MotionMat4 MotionMat4ApplyIconLocal(const MotionMat4 &base,
	tjs_real originX, tjs_real originY,
	tjs_real offsetX, tjs_real offsetY,
	tjs_real width, tjs_real height);

MotionRenderSurface MotionBuildSingleRenderSurface(
	const MotionRenderMethodItem &item);
MotionRenderSurface MotionBuildLayoutRenderSurface(const MotionMat4 &matTrans,
	const ttstr &label,
	tjs_real originX, tjs_real originY,
	tjs_real width, tjs_real height,
	tjs_real coordZ = 0.0);
MotionRenderSurface MotionBuildDrawRenderSurface(const MotionMat4 &matTrans,
	const MotionRenderMethodItem &item,
	tjs_real coordZ = 0.0);
MotionMat4 MotionBuildDemuxMatFromSurface(
	const MotionRenderSurface &parentSurface,
	bool includeInverseProjection,
	tjs_real limOriginX, tjs_real limOriginY,
	tjs_real limWidth, tjs_real limHeight,
	tjs_real limZMax = kMotionDefaultLimitZMax);
void MotionRefreshRenderMethodItemSurface(MotionRenderMethodItem &item);

void MotionEvalBezierSurface(const tjs_real controlPts[32],
	tjs_real u, tjs_real v, tjs_real &outU, tjs_real &outV);
bool MotionControlPtsAreIdentity(const tjs_real controlPts[32]);
bool MotionEvalSurfaceChainVertex(
	const std::vector<MotionRenderSurface> &surfaces,
	tjs_real u, tjs_real v,
	tjs_real viewportWidth, tjs_real viewportHeight,
	tjs_real &outPixelX, tjs_real &outPixelY,
	tjs_real *outNdcX = nullptr, tjs_real *outNdcY = nullptr,
	tjs_int *outRemapCount = nullptr,
	tjs_real *outNdcZ = nullptr, tjs_real *outPixelZ = nullptr);

} // namespace emoteplayer
