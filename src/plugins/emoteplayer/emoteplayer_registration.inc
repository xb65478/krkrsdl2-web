using namespace emoteplayer;

#define ENUM(n) Variant(TJS_W(#n), static_cast<tjs_int>(n))
#define PROPERTY(name) NCB_PROPERTY(name, get_##name, set_##name)

NCB_REGISTER_SUBCLASS(ResourceManager)
{
	NCB_CONSTRUCTOR(());
	NCB_CONSTRUCTOR((iTJSDispatch2 *, tjs_int));
	NCB_METHOD(load);
	NCB_METHOD(loadPSB);
	NCB_METHOD(loadFromMemory);
	NCB_METHOD(loadPSBFromMemory);
	NCB_METHOD(unload);
	NCB_METHOD(unloadAll);
	NCB_METHOD(clearCache);
	NCB_METHOD(setEmotePSBDecryptSeed);
	NCB_METHOD(setEmotePSBDecryptFunc);
	NCB_METHOD(getModelCount);
	NCB_METHOD(getModelWidth);
	NCB_METHOD(getModelHeight);
	NCB_METHOD(getModelFrameCount);
	NCB_METHOD(getModelMotionCount);
	NCB_METHOD(getModelMotionName);
	NCB_METHOD(getModelVariableCount);
	NCB_METHOD(getModelVariableName);
	NCB_METHOD(getModelVariableMin);
	NCB_METHOD(getModelVariableMax);
	NCB_METHOD(getResourceDump);
	NCB_METHOD(isLoaded);
}

NCB_REGISTER_SUBCLASS_DELAY(SeparateLayerAdaptor)
{
	NCB_CONSTRUCTOR(());
	NCB_CONSTRUCTOR((iTJSDispatch2 *));
	NCB_METHOD(assign);
	NCB_METHOD(clear);
	PROPERTY(absolute);
}

NCB_REGISTER_SUBCLASS(MotionHitShape)
{
	NCB_CONSTRUCTOR(());
	NCB_METHOD_RAW_CALLBACK(contains, &Class::containsRaw, 0);
	NCB_METHOD_RAW_CALLBACK(getLayerMotion, &Class::getLayerMotionRaw, 0);
	NCB_METHOD_RAW_CALLBACK(getLayerGetter, &Class::getLayerGetterRaw, 0);
	NCB_PROPERTY_RO(type, get_type);
	NCB_PROPERTY_RO(x, get_x);
	NCB_PROPERTY_RO(y, get_y);
	NCB_PROPERTY_RO(l, get_l);
	NCB_PROPERTY_RO(t, get_t);
	NCB_PROPERTY_RO(w, get_w);
	NCB_PROPERTY_RO(h, get_h);
	NCB_PROPERTY_RO(objectName, get_objectName);
	NCB_PROPERTY_RO(objectPath, get_objectPath);
	NCB_PROPERTY_RO(stateName, get_stateName);
	NCB_PROPERTY_RO(stateClass, get_stateClass);
	NCB_PROPERTY_RO(stateGroup, get_stateGroup);
	NCB_PROPERTY_RO(statePath, get_statePath);
	NCB_PROPERTY_RO(hitPriority, get_hitPriority);
	NCB_PROPERTY_RO(interactive, get_interactive);
}

NCB_REGISTER_SUBCLASS_DELAY(EmotePlayer)
{
	NCB_CONSTRUCTOR(());
	NCB_CONSTRUCTOR((ResourceManager *));
	NCB_METHOD(serialize);
	NCB_METHOD(unserialize);
	NCB_METHOD_RAW_CALLBACK(play, &Class::playRaw, 0);
	NCB_METHOD(initPhysics);
	NCB_METHOD(clear);
	NCB_METHOD(progress);
	NCB_METHOD(draw);
	NCB_METHOD(update);
	NCB_METHOD(render);
	NCB_METHOD(assign);
	NCB_METHOD(setResourceManager);
	NCB_METHOD(getResourceManager);
	NCB_METHOD(setMotion);
	NCB_METHOD(load);
	NCB_METHOD(loadPSB);
	NCB_METHOD(loadFromMemory);
	NCB_METHOD(isLoading);
	NCB_METHOD(isLoaded);
	NCB_METHOD(setCoord);
	NCB_METHOD(getCoord);
	NCB_METHOD(setScale);
	NCB_METHOD(getScale);
	NCB_METHOD(setTimeScale);
	NCB_METHOD(getTimeScale);
	NCB_METHOD(setAttachRect);
	NCB_METHOD(getAttachRect);
	NCB_METHOD(setTransform);
	NCB_METHOD(getTransform);
	NCB_METHOD(setRotate);
	NCB_METHOD(setColor);
	NCB_METHOD(getColor);
	NCB_METHOD(setVariable);
	NCB_METHOD(getVariable);
	NCB_METHOD(setMotionVariable);
	NCB_METHOD(getMotionVariable);
	NCB_METHOD(setVariableRange);
	NCB_METHOD(setOuterForce);
	NCB_METHOD(setDrawAffineTranslateMatrix);
	NCB_METHOD(setCameraOffset);
	NCB_METHOD(startWind);
	NCB_METHOD(stopWind);
	NCB_METHOD_RAW_CALLBACK(contains, &Class::containsRaw, 0);
	NCB_METHOD(skip);
	NCB_METHOD(skipToSync);
	NCB_METHOD(checkStopMotion);
	NCB_METHOD(pass);
	NCB_METHOD(stop);
	NCB_METHOD(addMotion);
	NCB_METHOD(removeMotion);
	NCB_METHOD(getMotionList);
	NCB_METHOD(getCommandList);
	NCB_METHOD_RAW_CALLBACK(getLayerMotion, &Class::getLayerMotionRaw, 0);
	NCB_METHOD_RAW_CALLBACK(getLayerGetter, &Class::getLayerGetterRaw, 0);
	NCB_METHOD(getVariableList);
	NCB_METHOD(getVariableCount);
	NCB_METHOD(getVariableName);
	NCB_METHOD(getVariableMin);
	NCB_METHOD(getVariableMax);
	NCB_METHOD(getModelWidth);
	NCB_METHOD(getModelHeight);
	NCB_METHOD(getFrameCount);
	NCB_METHOD(getCurrentFrame);
	NCB_METHOD(setCurrentFrame);
	NCB_METHOD(getDuration);
	NCB_METHOD(setPhysicsTimestep);
	NCB_METHOD(getPhysicsTimestep);
	NCB_METHOD(setPhysicsGravity);
	NCB_METHOD(setPhysicsWind);
	NCB_METHOD_RAW_CALLBACK(playTimeline, &Class::playTimelineRaw, 0);
	NCB_METHOD_RAW_CALLBACK(stopTimeline, &Class::stopTimelineRaw, 0);
	NCB_METHOD_RAW_CALLBACK(getTimelinePlaying, &Class::getTimelinePlayingRaw, 0);
	NCB_METHOD(getLoopTimeline);
	NCB_METHOD(getTimelineTotalFrameCount);
	NCB_METHOD(getMainTimelineLabelList);
	NCB_METHOD(getDiffTimelineLabelList);
	NCB_METHOD_RAW_CALLBACK(setTimelineBlendRatio, &Class::setTimelineBlendRatioRaw, 0);
	NCB_METHOD_RAW_CALLBACK(fadeInTimeline, &Class::fadeInTimelineRaw, 0);
	NCB_METHOD_RAW_CALLBACK(fadeOutTimeline, &Class::fadeOutTimelineRaw, 0);
	NCB_METHOD(getPlayingTimelineInfoList);
		NCB_METHOD(getVariableFrameList);
		NCB_METHOD(getRendererFrontState);
		NCB_METHOD(getRendererBackend);
		NCB_METHOD(setRendererBackend);
		NCB_METHOD(resolveFrontHit);
	PROPERTY(animating);
	PROPERTY(playing);
	PROPERTY(allplaying);
	PROPERTY(loopTime);
	PROPERTY(tickCount);
	PROPERTY(speed);
	PROPERTY(outline);
	PROPERTY(motionKey);
	PROPERTY(motion);
	PROPERTY(motionPlaying);
	PROPERTY(visible);
	PROPERTY(opacity);
	PROPERTY(left);
	PROPERTY(top);
	PROPERTY(width);
	PROPERTY(height);
	PROPERTY(scale);
	PROPERTY(timeScale);
	PROPERTY(bustScale);
	PROPERTY(hairScale);
	PROPERTY(partsScale);
	PROPERTY(meshDivisionRatio);
	PROPERTY(completionType);
	PROPERTY(wind);
	PROPERTY(color);
	PROPERTY(smoothing);
	PROPERTY(paused);
	PROPERTY(chara);
	PROPERTY(variableKeys);
	PROPERTY(tags);
	Variant(TJS_W("TimelinePlayFlagParallel"), static_cast<tjs_int>(EmotePlayer::TimelinePlayFlagParallel));
	Variant(TJS_W("TimelinePlayFlagDifference"), static_cast<tjs_int>(EmotePlayer::TimelinePlayFlagDifference));
}

NCB_REGISTER_SUBCLASS_DELAY(Player)
{
	NCB_CONSTRUCTOR(());
	NCB_CONSTRUCTOR((ResourceManager *));
	NCB_METHOD(serialize);
	NCB_METHOD(unserialize);
	NCB_METHOD_RAW_CALLBACK(play, &Class::playRaw, 0);
	NCB_METHOD(initPhysics);
	NCB_METHOD(clear);
	NCB_METHOD(progress);
	NCB_METHOD(draw);
	NCB_METHOD(update);
	NCB_METHOD(render);
	NCB_METHOD(assign);
	NCB_METHOD(setResourceManager);
	NCB_METHOD(getResourceManager);
	NCB_METHOD(setMotion);
	NCB_METHOD(load);
	NCB_METHOD(loadPSB);
	NCB_METHOD(loadFromMemory);
	NCB_METHOD(isLoading);
	NCB_METHOD(isLoaded);
	NCB_METHOD(setCoord);
	NCB_METHOD(getCoord);
	NCB_METHOD(setScale);
	NCB_METHOD(getScale);
	NCB_METHOD(setTimeScale);
	NCB_METHOD(getTimeScale);
	NCB_METHOD(setAttachRect);
	NCB_METHOD(getAttachRect);
	NCB_METHOD(setTransform);
	NCB_METHOD(getTransform);
	NCB_METHOD(setRotate);
	NCB_METHOD(setColor);
	NCB_METHOD(getColor);
	NCB_METHOD(setVariable);
	NCB_METHOD(getVariable);
	NCB_METHOD(setMotionVariable);
	NCB_METHOD(getMotionVariable);
	NCB_METHOD(setVariableRange);
	NCB_METHOD(setOuterForce);
	NCB_METHOD(setDrawAffineTranslateMatrix);
	NCB_METHOD(setCameraOffset);
	NCB_METHOD(startWind);
	NCB_METHOD(stopWind);
	NCB_METHOD_RAW_CALLBACK(contains, &Class::containsRaw, 0);
	NCB_METHOD(skip);
	NCB_METHOD(skipToSync);
	NCB_METHOD(checkStopMotion);
	NCB_METHOD(stop);
	NCB_METHOD(pass);
	NCB_METHOD(addMotion);
	NCB_METHOD(removeMotion);
	NCB_METHOD(getMotionList);
	NCB_METHOD(getCommandList);
	NCB_METHOD_RAW_CALLBACK(getLayerMotion, &Class::getLayerMotionRaw, 0);
	NCB_METHOD_RAW_CALLBACK(getLayerGetter, &Class::getLayerGetterRaw, 0);
	NCB_METHOD(getVariableList);
	NCB_METHOD(getVariableCount);
	NCB_METHOD(getVariableName);
	NCB_METHOD(getVariableMin);
	NCB_METHOD(getVariableMax);
	NCB_METHOD(getModelWidth);
	NCB_METHOD(getModelHeight);
	NCB_METHOD(getFrameCount);
	NCB_METHOD(getCurrentFrame);
	NCB_METHOD(setCurrentFrame);
	NCB_METHOD(getDuration);
	NCB_METHOD(setPhysicsTimestep);
	NCB_METHOD(getPhysicsTimestep);
	NCB_METHOD(setPhysicsGravity);
	NCB_METHOD(setPhysicsWind);
	NCB_METHOD_RAW_CALLBACK(playTimeline, &Class::playTimelineRaw, 0);
	NCB_METHOD_RAW_CALLBACK(stopTimeline, &Class::stopTimelineRaw, 0);
	NCB_METHOD_RAW_CALLBACK(getTimelinePlaying, &Class::getTimelinePlayingRaw, 0);
	NCB_METHOD(getLoopTimeline);
	NCB_METHOD(getTimelineTotalFrameCount);
	NCB_METHOD(getMainTimelineLabelList);
	NCB_METHOD(getDiffTimelineLabelList);
	NCB_METHOD_RAW_CALLBACK(setTimelineBlendRatio, &Class::setTimelineBlendRatioRaw, 0);
	NCB_METHOD_RAW_CALLBACK(fadeInTimeline, &Class::fadeInTimelineRaw, 0);
	NCB_METHOD_RAW_CALLBACK(fadeOutTimeline, &Class::fadeOutTimelineRaw, 0);
	NCB_METHOD(getPlayingTimelineInfoList);
		NCB_METHOD(getVariableFrameList);
		NCB_METHOD(getRendererFrontState);
		NCB_METHOD(getRendererBackend);
		NCB_METHOD(setRendererBackend);
		NCB_METHOD(resolveFrontHit);
	PROPERTY(animating);
	PROPERTY(playing);
	PROPERTY(allplaying);
	PROPERTY(loopTime);
	PROPERTY(tickCount);
	PROPERTY(speed);
	PROPERTY(outline);
	PROPERTY(motionKey);
	PROPERTY(motion);
	PROPERTY(motionPlaying);
	PROPERTY(visible);
	PROPERTY(opacity);
	PROPERTY(left);
	PROPERTY(top);
	PROPERTY(width);
	PROPERTY(height);
	PROPERTY(scale);
	PROPERTY(timeScale);
	PROPERTY(bustScale);
	PROPERTY(hairScale);
	PROPERTY(partsScale);
	PROPERTY(meshDivisionRatio);
	PROPERTY(completionType);
	PROPERTY(wind);
	PROPERTY(color);
	PROPERTY(smoothing);
	PROPERTY(paused);
	PROPERTY(chara);
	PROPERTY(variableKeys);
	PROPERTY(tags);
}

NCB_REGISTER_CLASS(Motion)
{
	NCB_METHOD(getD3DAvailable);
	NCB_PROPERTY(enableD3D, getEnableD3D, setEnableD3D);
	ENUM(MaskModeAlpha);
	ENUM(PlayFlagForce);
	ENUM(ShapeTypePoint);
	ENUM(ShapeTypeCircle);
	ENUM(ShapeTypeRect);
	ENUM(ShapeTypeQuad);
	NCB_SUBCLASS(ResourceManager, ResourceManager);
	NCB_SUBCLASS(EmotePlayer, EmotePlayer);
	NCB_SUBCLASS(Player, Player);
	NCB_SUBCLASS(SeparateLayerAdaptor, SeparateLayerAdaptor);
	NCB_SUBCLASS(MotionHitShape, MotionHitShape);
}

static void emoteplayer_init()
{
	KRKR_LOG_L2("[emoteplayer] registered minimal WASM Motion runtime\n");
}

static void emoteplayer_done() {}

NCB_PRE_REGIST_CALLBACK(emoteplayer_init);
NCB_POST_UNREGIST_CALLBACK(emoteplayer_done);
