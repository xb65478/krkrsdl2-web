#pragma once

#include <string>
#include <vector>

struct TVPStaticMotionBridgeItem {
	std::string ref;
	int dstX = 0;
	int dstY = 0;
	int width = 0;
	int height = 0;
	double scaleX = 1.0;
	double scaleY = 1.0;
	double opacity = 1.0;
	bool hasQuad = false;
	double quadX[4] = {0.0, 0.0, 0.0, 0.0};
	double quadY[4] = {0.0, 0.0, 0.0, 0.0};
	std::vector<unsigned char> pixelsBGRA;
};

struct TVPStaticMotionBridgeSnapshot {
	std::string storage;
	std::string objectName;
	std::string motionName;
	double frameTime = 0.0;
	int canvasWidth = 0;
	int canvasHeight = 0;
	int offsetX = 0;
	int offsetY = 0;
	bool usedMotionLayout = false;
	std::vector<TVPStaticMotionBridgeItem> items;
};

const TVPStaticMotionBridgeSnapshot *TVPGetLastPSBStaticMotionBridgeSnapshot();
const TVPStaticMotionBridgeSnapshot *TVPFindPSBStaticMotionBridgeSnapshot(
	const char *storage, const char *objectName, const char *motionName);
void TVPClearLastPSBStaticMotionBridgeSnapshot();
