#pragma once

#include "tjsString.h"

#include <string>
#include <vector>

namespace emoteplayer {

struct MotionVariableInfo {
	ttstr name;
	tjs_real minValue = 0.0;
	tjs_real maxValue = 0.0;
	tjs_real division = 0.0;
};

struct MotionSelectorOptionInfo {
	ttstr label;
	tjs_real offValue = 0.0;
	tjs_real onValue = 1.0;
};

struct MotionSelectorInfo {
	ttstr label;
	std::vector<MotionSelectorOptionInfo> options;
};

struct MotionNodeInfo {
	ttstr label;
	tjs_int type = -1;
	tjs_int parameterIdx = -1;
	tjs_int frameCount = 0;
	tjs_int childCount = 0;
	std::vector<std::string> motionRefs;
};

struct MotionAttachmentRefInfo {
	ttstr ownerObjectName;
	ttstr ownerMotionName;
	ttstr nodeLabel;
	ttstr refPath;
	ttstr refObjectName;
	ttstr refMotionName;
	ttstr refIconName;
	tjs_real frameTime = 0.0;
	bool resolved = false;
	bool external = false;
};

struct MotionTimelineFrameInfo {
	tjs_real time = 0.0;
	tjs_int type = -1;
	tjs_real value = 0.0;
	tjs_real easing = 0.0;
	bool hasContent = false;
};

struct MotionTimelineVariableInfo {
	ttstr label;
	std::vector<MotionTimelineFrameInfo> frames;
};

struct MotionTimelineInfo {
	ttstr label;
	tjs_int lastTime = -1;
	tjs_int loopBegin = -1;
	tjs_int loopEnd = -1;
	tjs_int diff = 0;
	std::vector<MotionTimelineVariableInfo> variables;
};

struct MotionTimelineRuntimeState {
	ttstr label;
	tjs_int flags = 0;
	tjs_real blendRatio = 1.0;
	tjs_real blendStartRatio = 1.0;
	tjs_real blendTargetRatio = 1.0;
	tjs_real blendTime = 0.0;
	tjs_real easing = 0.0;
	tjs_real startedTick = 0.0;
	tjs_real blendStartedTick = 0.0;
	bool stopWhenBlendDone = false;
	bool active = true;
};

struct MotionWindState {
	bool active = false;
	tjs_real start = 0.0;
	tjs_real goal = 0.0;
	tjs_real speed = 0.0;
	tjs_real powerMin = 0.0;
	tjs_real powerMax = 0.0;
	tjs_real progress = 0.0;
};

struct MotionOuterForceState {
	ttstr name;
	tjs_real x = 0.0;
	tjs_real y = 0.0;
};

struct MotionAffine2D {
	tjs_real m00 = 1.0;
	tjs_real m01 = 0.0;
	tjs_real m10 = 0.0;
	tjs_real m11 = 1.0;
	tjs_real tx = 0.0;
	tjs_real ty = 0.0;
};

struct MotionMat4 {
	tjs_real m[16] = {
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	};
};

static constexpr tjs_real kMotionDefaultLimitZMax = 30.0;

struct MotionLayoutInfo {
	ttstr name;
	ttstr sourceKey;
	ttstr objectName;
	ttstr motionName;
	ttstr contextObjectName;
	ttstr contextMotionName;
	ttstr hitObjectName;
	ttstr hitObjectPath;
	ttstr hitStateName;
	ttstr hitStateClass;
	ttstr hitStateGroup;
	ttstr hitStatePath;
	ttstr hitPartName;
	ttstr hitRole;
	tjs_real left = 0.0;
	tjs_real top = 0.0;
	tjs_real width = 0.0;
	tjs_real height = 0.0;
	tjs_real centerX = 0.0;
	tjs_real centerY = 0.0;
	tjs_real opacityMin = 1.0;
	tjs_real opacityMax = 1.0;
	tjs_int drawCount = 0;
	tjs_int renderMethodItemIndex = -1;
	tjs_int hitPriority = 0;
	tjs_int hitPathDepth = 0;
	bool hitInteractive = false;
	bool hitLayoutProxy = false;
	bool frontDrawCandidate = false;
	bool frontHitCandidate = false;
	tjs_real angle = 0.0;
	tjs_real sx = 0.0;
	tjs_real sy = 0.0;
	tjs_real ox = 0.0;
	tjs_real oy = 0.0;
	tjs_real iconOriginX = 0.0;
	tjs_real iconOriginY = 0.0;
	tjs_real iconWidth = 0.0;
	tjs_real iconHeight = 0.0;
	tjs_real accumX = 0.0;
	tjs_real accumY = 0.0;
	tjs_real accumScaleX = 1.0;
	tjs_real accumScaleY = 1.0;
	bool affineValid = false;
	MotionAffine2D affine;
	bool hasControlPoints = false;
	tjs_real controlPts[32] = {
		0.0, 0.0, 0.333333, 0.0, 0.666667, 0.0, 1.0, 0.0,
		0.0, 0.333333, 0.333333, 0.333333, 0.666667, 0.333333, 1.0, 0.333333,
		0.0, 0.666667, 0.333333, 0.666667, 0.666667, 0.666667, 1.0, 0.666667,
		0.0, 1.0, 0.333333, 1.0, 0.666667, 1.0, 1.0, 1.0
	};
};

struct MotionInfo {
	ttstr objectName;
	ttstr name;
	tjs_int parameterIdx = -1;
	tjs_real lastTime = 0.0;
	tjs_real loopTime = 0.0;
	tjs_real selfSyncTime = 0.0;
	tjs_real syncTime = 0.0;
	tjs_int frameCount = 0;
	tjs_int motionRefCount = 0;
	tjs_int timeOffsetNonzeroRefCount = 0;
	std::vector<std::string> motionRefs;
	std::vector<MotionVariableInfo> variables;
	std::vector<MotionNodeInfo> nodes;
	std::vector<MotionAttachmentRefInfo> attachmentRefs;
};

struct MotionObjectInfo {
	ttstr name;
	ttstr type;
	std::vector<MotionInfo> motions;
};

struct MotionTimeSemanticsDiag {
	tjs_int nestedMotionCount = 0;
	tjs_int timeOffsetNonzeroCount = 0;
	tjs_real parentTickSample = 0.0;
	tjs_real childEvalTickSample = 0.0;
	tjs_real timeOffsetSample = 0.0;
	tjs_real requestedTickSample = 0.0;
	tjs_real effectiveTickSample = 0.0;
	tjs_real twoFrameHoldTickSample = 0.0;
	std::string firstNestedMotionPath;
	bool timeOffsetConsumed = false;
	bool selfSyncTimeClamped = false;
	bool twoFrameSpecialApplied = false;
	tjs_int tickProbeCount = 0;
	tjs_int metadataNestedMotionRefCount = 0;
	tjs_int metadataTimeOffsetNonzeroCount = 0;
	std::string skipReason;
	tjs_int probed = 0;
};

struct MotionLimitSemanticsDiag {
	tjs_int probed = 0;
	tjs_int nanCoordCount = 0;
	tjs_int infCoordCount = 0;
	tjs_int type12LimInheritCount = 0;
	tjs_int nondrawableLimInheritCount = 0;
	tjs_int childLimPropCount = 0;
	tjs_real sampleParentLimOriginX = 0.0;
	tjs_real sampleParentLimOriginY = 0.0;
	tjs_real sampleParentLimWidth = 0.0;
	tjs_real sampleParentLimHeight = 0.0;
	tjs_real sampleParentLimZMax = kMotionDefaultLimitZMax;
	tjs_real sampleChildLimOriginX = 0.0;
	tjs_real sampleChildLimOriginY = 0.0;
	tjs_real sampleChildLimWidth = 0.0;
	tjs_real sampleChildLimHeight = 0.0;
	tjs_real sampleChildLimZMax = kMotionDefaultLimitZMax;
	tjs_real sampleFrameOriginX = 0.0;
	tjs_real sampleFrameOriginY = 0.0;
	tjs_real sampleFrameWidth = 0.0;
	tjs_real sampleFrameHeight = 0.0;
	tjs_real sampleCoordBeforeX = 0.0;
	tjs_real sampleCoordBeforeY = 0.0;
	tjs_real sampleCoordAfterX = 0.0;
	tjs_real sampleCoordAfterY = 0.0;
	bool nanInfPathVerified = false;
	tjs_int metadataSurveyFrameCount = 0;
	tjs_int metadataSurveyNanCandidateCount = 0;
	tjs_int metadataSurveyInfCandidateCount = 0;
	tjs_int singleFrameNormalizeCount = 0;
	tjs_int interpolationFramePairCount = 0;
	tjs_int coordInterpolationCount = 0;
	tjs_int interpolationNormalizeCount = 0;
	std::string skipReason;
};

static const tjs_real kMotionDefaultControlPoints[32] = {
	0.0, 0.0, 0.333333, 0.0, 0.666667, 0.0, 1.0, 0.0,
	0.0, 0.333333, 0.333333, 0.333333, 0.666667, 0.333333, 1.0, 0.333333,
	0.0, 0.666667, 0.333333, 0.666667, 0.666667, 0.666667, 1.0, 0.666667,
	0.0, 1.0, 0.333333, 1.0, 0.666667, 1.0, 1.0, 1.0
};

struct MotionRenderSurface {
	MotionMat4 matTrans;
	tjs_real originX = 0.0;
	tjs_real originY = 0.0;
	tjs_real originW = 0.0;
	tjs_real originH = 0.0;
	tjs_real opa = 1.0;
	tjs_int type = 0;
	tjs_real controlPts[32] = {
		0.0, 0.0, 0.333333, 0.0, 0.666667, 0.0, 1.0, 0.0,
		0.0, 0.333333, 0.333333, 0.333333, 0.666667, 0.333333, 1.0, 0.333333,
		0.0, 0.666667, 0.333333, 0.666667, 0.666667, 0.666667, 1.0, 0.666667,
		0.0, 1.0, 0.333333, 1.0, 0.666667, 1.0, 1.0, 1.0
	};
	bool hasStencil = false;
	tjs_real currCoordz = 0.0;
	ttstr label;
};

struct MotionRenderMethodItem {
	tjs_int type = 0;
	ttstr label;
	std::string srcKey;
	tjs_real originX = 0.0;
	tjs_real originY = 0.0;
	tjs_real width = 0.0;
	tjs_real height = 0.0;
	tjs_real opacity = 1.0;
	bool affineValid = false;
	MotionAffine2D affine;
	bool hasControlPoints = false;
	tjs_real controlPts[32] = {
		0.0, 0.0, 0.333333, 0.0, 0.666667, 0.0, 1.0, 0.0,
		0.0, 0.333333, 0.333333, 0.333333, 0.666667, 0.333333, 1.0, 0.333333,
		0.0, 0.666667, 0.333333, 0.666667, 0.666667, 0.666667, 1.0, 0.666667,
		0.0, 1.0, 0.333333, 1.0, 0.666667, 1.0, 1.0, 1.0
	};
	bool isFromLayout = false;
	bool isFromNestedMotion = false;
	bool linkedToLayout = false;
	tjs_int depth = 0;
	tjs_int layoutIndex = -1;
	tjs_int sourceNodeIndex = -1;
	tjs_int drawPriority = -1;
	std::vector<tjs_int> drawPriorityPath;
	bool m2ProgressSurface = false;
	bool m2UnsupportedDemux = false;
	std::vector<MotionRenderSurface> surfaces;
};

struct MotionRenderMethodSemanticsDiag {
	tjs_int probed = 0;
	bool stackBuilt = false;
	tjs_int layoutMergeCount = 0;
	tjs_int drawItemType2Count = 0;
	tjs_int drawItemType1Count = 0;
	tjs_int nestedMotionRenderPassCount = 0;
	tjs_int totalItems = 0;
	tjs_int linkedCommandCount = 0;
	tjs_int linkedLayoutCount = 0;
	ttstr firstSampleType;
	ttstr firstSampleLabel;
	tjs_real firstSampleOriginX = 0.0;
	tjs_real firstSampleOriginY = 0.0;
	tjs_real firstSampleWidth = 0.0;
	tjs_real firstSampleHeight = 0.0;
	tjs_real firstSampleOpacity = 1.0;
	bool firstSampleHasControlPoints = false;
	tjs_int surfaceCount = 0;
	tjs_int itemWithSurfaceCount = 0;
	tjs_int firstSampleSurfaceCount = 0;
	tjs_real firstSampleSurfaceOriginX = 0.0;
	tjs_real firstSampleSurfaceOriginY = 0.0;
	tjs_real firstSampleSurfaceOriginW = 0.0;
	tjs_real firstSampleSurfaceOriginH = 0.0;
	tjs_real firstSampleSurfaceOpa = 1.0;
	tjs_int firstSampleSurfaceType = 0;
	tjs_real firstSampleSurfaceMat00 = 1.0;
	tjs_real firstSampleSurfaceMat11 = 1.0;
	bool firstSampleSurfaceHasStencil = false;
	tjs_real firstSampleSurfaceCurrCoordz = 0.0;
	tjs_int metadataSurveyFrameCount = 0;
	tjs_int metadataSurveyBpCandidateCount = 0;
	ttstr rendererCommandSource;
	bool rendererCommandConsumedIR = false;
	tjs_int consumedType1CommandCount = 0;
	tjs_int consumedType2CommandCount = 0;
	tjs_int consumedType3CommandCount = 0;
	tjs_int m2SelfTested = 0;
	tjs_int m2SelfTestPassed = 0;
	tjs_int m2SingleSurfaceItemCount = 0;
	tjs_int m2UnsupportedDemuxCount = 0;
	tjs_real m2SampleMaxAbsError = 0.0;
	tjs_real m2SampleMat00 = 0.0;
	tjs_real m2SampleMat11 = 0.0;
	tjs_real m2SampleMat03 = 0.0;
	tjs_real m2SampleMat13 = 0.0;
	tjs_int m3SelfTested = 0;
	tjs_int m3SelfTestPassed = 0;
	tjs_int m3MultiSurfaceItemCount = 0;
	tjs_int m3DemuxResolvedCount = 0;
	tjs_int m3SampleSurfaceCount = 0;
	tjs_real m3SampleMaxAbsError = 0.0;
	tjs_real m3SampleMat00 = 0.0;
	tjs_real m3SampleMat11 = 0.0;
	tjs_real m3SampleMat03 = 0.0;
	tjs_real m3SampleMat13 = 0.0;
	tjs_int m4SelfTested = 0;
	tjs_int m4SelfTestPassed = 0;
	tjs_int m4SurfaceChainCommandCount = 0;
	tjs_int m4MaxSurfaceChainLength = 0;
	tjs_int m4RemappedVertexCount = 0;
	tjs_int m4ViewportTransformedVertexCount = 0;
	tjs_real m4SampleNdcX = 0.0;
	tjs_real m4SampleNdcY = 0.0;
	tjs_real m4SamplePixelX = 0.0;
	tjs_real m4SamplePixelY = 0.0;
	tjs_real m4SampleMaxAbsError = 0.0;
	tjs_int m5SelfTested = 0;
	tjs_int m5SelfTestPassed = 0;
	tjs_int m5Type1SurfaceCount = 0;
	tjs_int m5ControlPointSurfaceConsumed = 0;
	tjs_real m5SampleAffinePixelX = 0.0;
	tjs_real m5SampleAffinePixelY = 0.0;
	tjs_real m5SampleDeformedPixelX = 0.0;
	tjs_real m5SampleDeformedPixelY = 0.0;
	tjs_real m5SampleDeviationDist = 0.0;
	tjs_real m5SampleControlPt1X = 0.0;
	tjs_real m5SampleControlPt1Y = 0.0;
	std::string skipReason;
};

struct MotionAffineFrontMeshDiag {
	tjs_int probed = 0;
	tjs_int itemCount = 0;
	tjs_int affinePathItemCount = 0;
	tjs_int nonAxisAlignedItemCount = 0;
	tjs_int affineSelfTested = 0;
	tjs_int affineSelfTestPassed = 0;
	tjs_int affineSelfTestNonAxisAligned = 0;
	tjs_int affineSelfTestShearOk = 0;
	tjs_int affineSelfTestNestedOk = 0;
	tjs_int affineConsumerSelfTested = 0;
	tjs_int affineConsumerSelfTestPassed = 0;
	tjs_int affineConsumerSelfTestNonAxisAligned = 0;
	tjs_int affineConsumerUploadPrepared = 0;
	tjs_int affineConsumerStagingPopulated = 0;
	tjs_int affineConsumerShadowConsumed = 0;
	tjs_int affineConsumerCommandCount = 0;
	tjs_int affineConsumerVertexCount = 0;
	tjs_int affineConsumerIndexCount = 0;
	tjs_int affineConsumerCellCount = 0;
	tjs_int affineConsumerTriangleCount = 0;
	bool rotationConsumed = false;
	bool shearConsumed = false;
	bool affinePositionGenerated = false;
	tjs_real sampleAngle = 0.0;
	tjs_real sampleSx = 0.0;
	tjs_real sampleSy = 0.0;
	tjs_real sampleOx = 0.0;
	tjs_real sampleOy = 0.0;
	tjs_real sampleIconOriginX = 0.0;
	tjs_real sampleIconOriginY = 0.0;
	tjs_real sampleIconWidth = 0.0;
	tjs_real sampleIconHeight = 0.0;
	tjs_real sampleQ0x = 0.0;
	tjs_real sampleQ0y = 0.0;
	tjs_real sampleQ1x = 0.0;
	tjs_real sampleQ1y = 0.0;
	tjs_real sampleQ2x = 0.0;
	tjs_real sampleQ2y = 0.0;
	tjs_real sampleQ3x = 0.0;
	tjs_real sampleQ3y = 0.0;
	tjs_real sampleAabbLeft = 0.0;
	tjs_real sampleAabbTop = 0.0;
	tjs_real sampleAabbWidth = 0.0;
	tjs_real sampleAabbHeight = 0.0;
	std::string consumerSelfTestMismatchKind;
	std::string consumerSelfTestDetails;
	std::string skipReason;
};

struct MotionMeshDeformationDiag {
	tjs_int probed = 0;
	tjs_int type1ItemCount = 0;
	tjs_int type1CommandCount = 0;
	tjs_int bpFrameCount = 0;
	tjs_int meshDeformSelfTested = 0;
	tjs_int meshDeformSelfTestPassed = 0;
	tjs_int meshDeformConsumerSelfTested = 0;
	tjs_int meshDeformConsumerSelfTestPassed = 0;
	tjs_int meshDeformConsumerCommandCount = 0;
	tjs_int meshDeformConsumerVertexCount = 0;
	tjs_int meshDeformConsumerIndexCount = 0;
	tjs_int meshDeformConsumerCellCount = 0;
	tjs_int meshDeformConsumerTriangleCount = 0;
	tjs_int meshDeformConsumerUploadPrepared = 0;
	tjs_int meshDeformConsumerStagingPopulated = 0;
	tjs_int meshDeformConsumerShadowConsumed = 0;
	tjs_int meshDeformInteriorDeviated = 0;
	tjs_real sampleInteriorAffineX = 0.0;
	tjs_real sampleInteriorAffineY = 0.0;
	tjs_real sampleInteriorDeformedX = 0.0;
	tjs_real sampleInteriorDeformedY = 0.0;
	tjs_real sampleInteriorDeviationDist = 0.0;
	tjs_real sampleBp0 = 0.0;
	tjs_real sampleBp1 = 0.0;
	tjs_real sampleBp30 = 0.0;
	tjs_real sampleBp31 = 0.0;
	std::string consumerSelfTestMismatchKind;
	std::string consumerSelfTestDetails;
	std::string skipReason;
};

struct MotionSourceIconInfo {
	std::string key;
	std::string canonicalKey;
	std::string sourceName;
	std::string iconName;
	std::string name;
	tjs_real originX = 0.0;
	tjs_real originY = 0.0;
	tjs_real width = 0.0;
	tjs_real height = 0.0;
	bool hasPixel = false;
	bool hasPalette = false;
	tjs_uint32 pixelLength = 0;
	tjs_uint32 paletteLength = 0;
	bool decoderSupported = false;
};

struct MotionObjectGraphDiag {
	tjs_int objectCount = 0;
	tjs_int motionCount = 0;
	tjs_int nodeCount = 0;
	tjs_int motionRefCount = 0;
	tjs_int resolvedMotionRefCount = 0;
	tjs_int unresolvedMotionRefCount = 0;
	tjs_int externalMotionRefCount = 0;
	tjs_int sourceIconCount = 0;
	tjs_int sourceAliasCount = 0;
	tjs_int sourceKeysUsedCount = 0;
	tjs_int sourceKeysResolvedCount = 0;
	tjs_int sourceKeysUnresolvedCount = 0;
	tjs_int layoutCount = 0;
	tjs_int layoutWithObjectCount = 0;
	tjs_int layoutWithMotionCount = 0;
	tjs_int layoutWithSourceKeyCount = 0;
	tjs_int layoutWithRenderMethodLinkCount = 0;
	tjs_int renderMethodCount = 0;
	tjs_int renderMethodWithSourceKeyCount = 0;
	tjs_int renderMethodLinkedLayoutCount = 0;
	bool graphBuilt = false;
	std::string skipReason;
};

struct MotionResource {
	tjs_int id = 0;
	ttstr storage;
	tjs_uint64 byteSize = 0;
	std::vector<unsigned char> bytes;
	bool opened = false;
	bool parsed = false;
	bool isMotion = false;
	bool hasMetadata = false;
	ttstr spec;
	ttstr label;
	ttstr chara;
	ttstr motion;
	tjs_int width = 0;
	tjs_int height = 0;
	std::vector<MotionObjectInfo> objects;
	std::vector<MotionInfo> motions;
	std::vector<MotionVariableInfo> variables;
	std::vector<MotionSelectorInfo> selectors;
	std::vector<MotionTimelineInfo> timelines;
	std::vector<MotionLayoutInfo> layouts;
	std::vector<MotionSourceIconInfo> sourceIcons;
	tjs_int sourceAliasCount = 0;
	std::vector<MotionAttachmentRefInfo> attachmentRefs;
	MotionObjectGraphDiag objectGraphDiag;
	MotionTimeSemanticsDiag timeSemanticsDiag;
	MotionLimitSemanticsDiag limitSemanticsDiag;
	MotionRenderMethodSemanticsDiag motionRenderMethodSemantics;
	MotionAffineFrontMeshDiag affineFrontMeshDiag;
	MotionMeshDeformationDiag meshDeformationDiag;
	std::vector<MotionRenderMethodItem> renderMethodIR;
};

struct MotionRendererPrepInfo {
	bool meshPrepared = false;
	bool boundsValid = false;
	tjs_real meshDivisionRatio = 1.0;
	tjs_int meshDivision = 1;
	tjs_int layoutCount = 0;
	tjs_int drawItemCount = 0;
	tjs_int frontDrawItemCount = 0;
	tjs_int frontMeshCellCount = 0;
	tjs_int frontHitShapeCount = 0;
	tjs_int frontObjectStateCount = 0;
	tjs_int frontHitObjectStateCount = 0;
	tjs_int alphaAwareDrawItemCount = 0;
	tjs_int alphaAwareHitShapeCount = 0;
	tjs_int estimatedQuadCount = 0;
	tjs_int estimatedVertexCount = 0;
	tjs_int estimatedIndexCount = 0;
	tjs_real left = 0.0;
	tjs_real top = 0.0;
	tjs_real width = 0.0;
	tjs_real height = 0.0;
};

struct MotionFrameLayoutState {
	bool hasContent = false;
	bool hasCoord = false;
	tjs_real frameTime = 0.0;
	tjs_int frameType = 0;
	std::string src;
	tjs_real coordX = 0.0;
	tjs_real coordY = 0.0;
	tjs_real coordZ = 0.0;
	tjs_real ox = 0.0;
	tjs_real oy = 0.0;
	tjs_real scaleX = 1.0;
	tjs_real scaleY = 1.0;
	tjs_real opacity = 1.0;
	tjs_real timeOffset = 0.0;
	tjs_real angle = 0.0;
	tjs_real sx = 0.0;
	tjs_real sy = 0.0;
	bool hasMeshBP = false;
	tjs_real bp[32] = {
		0.0, 0.0, 0.333333, 0.0, 0.666667, 0.0, 1.0, 0.0,
		0.0, 0.333333, 0.333333, 0.333333, 0.666667, 0.333333, 1.0, 0.333333,
		0.0, 0.666667, 0.333333, 0.666667, 0.666667, 0.666667, 1.0, 0.666667,
		0.0, 1.0, 0.333333, 1.0, 0.666667, 1.0, 1.0, 1.0
	};
};

} // namespace emoteplayer
