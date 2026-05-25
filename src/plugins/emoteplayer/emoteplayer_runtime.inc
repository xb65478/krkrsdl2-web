class EmotePlayer {
public:
	enum EmotePlayerFlag {
		TimelinePlayFlagParallel = 0,
		TimelinePlayFlagDifference
	};

	EmotePlayer() = default;
	EmotePlayer(ResourceManager *resourceManager) : resourceManager(resourceManager)
	{
		// AnimKAGLayer creates the player before loading the requested storage
		// into the shared ResourceManager.  Keep the initial resource lazy so
		// the first chara/motion selection can bind to that just-loaded resource.
	}

	tTJSVariant serialize()
	{
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		SetTjsProperty(dict, TJS_W("currCoordx"), tTJSVariant(coordX));
		SetTjsProperty(dict, TJS_W("currCoordy"), tTJSVariant(coordY));
		SetTjsProperty(dict, TJS_W("currAngle"), tTJSVariant(rotation));
		SetTjsProperty(dict, TJS_W("currZx"), tTJSVariant(scaleX));
		SetTjsProperty(dict, TJS_W("currZy"), tTJSVariant(scaleY));
		SetTjsProperty(dict, TJS_W("tickCount"), tTJSVariant(tickCount));
		SetTjsProperty(dict, TJS_W("currentFrame"), tTJSVariant(currentFrame));
		SetTjsProperty(dict, TJS_W("speed"), tTJSVariant(speed));
		SetTjsProperty(dict, TJS_W("timeScale"), tTJSVariant(timeScale));
			SetTjsProperty(dict, TJS_W("motionKey"), tTJSVariant(motionKey));
			SetTjsProperty(dict, TJS_W("motion"), tTJSVariant(motion));
			SetTjsProperty(dict, TJS_W("chara"), tTJSVariant(chara));
			SetTjsProperty(dict, TJS_W("charaExplicit"), tTJSVariant((tjs_int)(charaExplicit ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("playing"), tTJSVariant((tjs_int)(playing ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("allplaying"), tTJSVariant((tjs_int)(allplaying ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("animating"), tTJSVariant((tjs_int)(animating ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("paused"), tTJSVariant((tjs_int)(paused ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("visible"), tTJSVariant((tjs_int)(visible ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("opacity"), tTJSVariant(opacity));
		SetTjsProperty(dict, TJS_W("left"), tTJSVariant(left));
		SetTjsProperty(dict, TJS_W("top"), tTJSVariant(top));
		SetTjsProperty(dict, TJS_W("width"), tTJSVariant(width));
		SetTjsProperty(dict, TJS_W("height"), tTJSVariant(height));
		SetTjsProperty(dict, TJS_W("bustScale"), tTJSVariant(bustScale));
		SetTjsProperty(dict, TJS_W("hairScale"), tTJSVariant(hairScale));
		SetTjsProperty(dict, TJS_W("partsScale"), tTJSVariant(partsScale));
		SetTjsProperty(dict, TJS_W("meshDivisionRatio"), tTJSVariant(meshDivisionRatio));
		SetTjsProperty(dict, TJS_W("completionType"), completionType);
		SetTjsProperty(dict, TJS_W("wind"), tTJSVariant(wind));
		SetTjsProperty(dict, TJS_W("color"), tTJSVariant((tjs_int64)drawColor));
		SetTjsProperty(dict, TJS_W("smoothing"), tTJSVariant((tjs_int)(smoothing ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("transform"), transform);
		SetTjsProperty(dict, TJS_W("physicsMetadata"), physicsMetadata);
		SetTjsProperty(dict, TJS_W("physicsGravityX"), tTJSVariant(physicsGravityX));
		SetTjsProperty(dict, TJS_W("physicsGravityY"), tTJSVariant(physicsGravityY));
		SetTjsProperty(dict, TJS_W("physicsWindX"), tTJSVariant(physicsWindX));
		SetTjsProperty(dict, TJS_W("physicsWindY"), tTJSVariant(physicsWindY));
		SetTjsProperty(dict, TJS_W("cameraWidth"), tTJSVariant(cameraWidth));
		SetTjsProperty(dict, TJS_W("cameraHeight"), tTJSVariant(cameraHeight));
		SetTjsProperty(dict, TJS_W("affineMatrix"), makeAffineMatrixState());
		SetTjsProperty(dict, TJS_W("windState"), makeWindState());
		SetTjsProperty(dict, TJS_W("outerForces"), makeOuterForceStateArray());
		SetTjsProperty(dict, TJS_W("variables"), makeVariableStateArray());
		SetTjsProperty(dict, TJS_W("variableRanges"), makeVariableRangeStateArray());
		SetTjsProperty(dict, TJS_W("timelines"), makeTimelineStateArray());
		SetTjsProperty(dict, TJS_W("rendererFrontState"), getRendererFrontState());
		tTJSVariant result(dict, dict);
		dict->Release();
		return result;
	}
	void unserialize(tTJSVariant data)
	{
		if (data.Type() != tvtObject) return;
		MotionReadVariantReal(data, TJS_W("currCoordx"), coordX);
		MotionReadVariantReal(data, TJS_W("currCoordy"), coordY);
		MotionReadVariantReal(data, TJS_W("currAngle"), rotation);
		MotionReadVariantReal(data, TJS_W("currZx"), scaleX);
		MotionReadVariantReal(data, TJS_W("currZy"), scaleY);
		scaleY = scaleY == 0.0 ? scaleX : scaleY;
		MotionReadVariantReal(data, TJS_W("tickCount"), tickCount);
		if (tickCount < 0.0) tickCount = 0.0;
		MotionReadVariantInt(data, TJS_W("currentFrame"), currentFrame);
		if (currentFrame < 0) currentFrame = 0;
		MotionReadVariantReal(data, TJS_W("speed"), speed);
		if (speed <= 0.0001) speed = 20.0;
		MotionReadVariantReal(data, TJS_W("timeScale"), timeScale);
		if (timeScale < 0.0) timeScale = 0.0;
			MotionReadVariantString(data, TJS_W("motionKey"), motionKey);
			MotionReadVariantString(data, TJS_W("motion"), motion);
			MotionReadVariantString(data, TJS_W("chara"), chara);
			if (!MotionReadVariantBool(data, TJS_W("charaExplicit"), charaExplicit)) {
				charaExplicit = chara.GetLen() > 0;
			}
			MotionReadVariantBool(data, TJS_W("playing"), playing);
		MotionReadVariantBool(data, TJS_W("allplaying"), allplaying);
		MotionReadVariantBool(data, TJS_W("animating"), animating);
		MotionReadVariantBool(data, TJS_W("paused"), paused);
		MotionReadVariantBool(data, TJS_W("visible"), visible);
		MotionReadVariantInt(data, TJS_W("opacity"), opacity);
		MotionReadVariantReal(data, TJS_W("left"), left);
		MotionReadVariantReal(data, TJS_W("top"), top);
		MotionReadVariantReal(data, TJS_W("width"), width);
		MotionReadVariantReal(data, TJS_W("height"), height);
		MotionReadVariantReal(data, TJS_W("bustScale"), bustScale);
		MotionReadVariantReal(data, TJS_W("hairScale"), hairScale);
		MotionReadVariantReal(data, TJS_W("partsScale"), partsScale);
		MotionReadVariantReal(data, TJS_W("meshDivisionRatio"), meshDivisionRatio);
		MotionReadVariantReal(data, TJS_W("wind"), wind);
		tjs_real colorValue = 0.0;
		if (MotionReadVariantReal(data, TJS_W("color"), colorValue)) {
			drawColor = (tjs_uint32)std::max<tjs_real>(0.0, colorValue);
		}
		MotionReadVariantBool(data, TJS_W("smoothing"), smoothing);
		tTJSVariant prop;
		if (GetTjsVariantObjectProperty(data, TJS_W("transform"), prop)) transform = prop;
		if (GetTjsVariantObjectProperty(data, TJS_W("physicsMetadata"), prop)) physicsMetadata = prop;
		if (GetTjsVariantObjectProperty(data, TJS_W("completionType"), prop)) completionType = prop;
		MotionReadVariantReal(data, TJS_W("physicsGravityX"), physicsGravityX);
		MotionReadVariantReal(data, TJS_W("physicsGravityY"), physicsGravityY);
		MotionReadVariantReal(data, TJS_W("physicsWindX"), physicsWindX);
		MotionReadVariantReal(data, TJS_W("physicsWindY"), physicsWindY);
		MotionReadVariantInt(data, TJS_W("cameraWidth"), cameraWidth);
		MotionReadVariantInt(data, TJS_W("cameraHeight"), cameraHeight);
		restoreAffineMatrixState(data);
		restoreWindState(data);
		restoreOuterForceState(data);
		restoreVariableState(data);
		restoreTimelineState(data);
		if (resourceManager && motionKey.GetLen() > 0) {
			currentResource = resourceManager->findResource(motionKey);
			if (!currentResource) {
				try {
					resourceManager->load(motionKey);
					currentResource = resourceManager->findResource(motionKey);
				} catch (...) {
				}
			}
		}
		selectCurrentMotion();
		updateTimelineVariables();
		updateCurrentFrame();
		invalidateDrawCache();
		invalidateRuntimeLayoutCache();
		stopCommandSent = playing ? false : true;
		playWasCalled = playing || playWasCalled;
	}
	void play(tTJSString name, tjs_int flag = 0)
	{
		motion = name;
		selectCurrentMotion();
		tickCount = 0.0;
		progressWallTimeMs = 0.0;
		updateCurrentFrame();
		invalidateDrawCache();
		invalidateRuntimeLayoutCache();
		playing = true;
		animating = true;
		allplaying = true;
		paused = false;
		playWasCalled = true;
		stopCommandSent = false;
	}
	static tjs_error TJS_INTF_METHOD playRaw(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, EmotePlayer *nativeInstance)
	{
		if (!nativeInstance) return TJS_E_NATIVECLASSCRASH;
		if (numparams < 1 || numparams > 2) return TJS_E_BADPARAMCOUNT;
		tTJSString name(*param[0]);
		tjs_int flag = numparams >= 2 ? (tjs_int)*param[1] : 0;
		nativeInstance->play(name, flag);
		if (result) result->Clear();
		return TJS_S_OK;
	}
	void initPhysics(tTJSVariant metadata) { physicsMetadata = metadata; }
	void clear(iTJSDispatch2 *layer, tjs_uint32 neutralColor = 0) {}
	void progress(tjs_real msTime)
	{
		selectCurrentMotion();
		if (msTime < 0.0) msTime = 0.0;
		updateEffectState(msTime);
		if (!playing || paused) {
			updateTimelineVariables();
			updateCurrentFrame();
			return;
		}

		tjs_real divisor = MotionEffectiveSpeedDivisor(speed);
		tickCount += (msTime / divisor) * timeScale;
		if (tickCount < 0.0) tickCount = 0.0;

		tjs_real duration = getPlaybackEndTime();
#ifdef __EMSCRIPTEN__
		if (MotionClickBurstTraceActive()) {
			std::shared_ptr<MotionResource> res = resolveResource();
			char buf[512];
			std::snprintf(buf, sizeof(buf),
				"progress storage=%s chara=%s motion=%s ms=%.1f tick=%.1f duration=%.1f speed=%.3f divisor=%.3f playing=%d all=%d anim=%d loop=%d var=%d keep=%d hasPlayable=%d currentLoop=%.1f vars=%zu motionVars=%zu",
				res ? res->storage.AsNarrowStdString().c_str() : "(null)",
				chara.GetLen() > 0 ? chara.AsNarrowStdString().c_str() : "(none)",
				motion.GetLen() > 0 ? motion.AsNarrowStdString().c_str() : "(none)",
				msTime, tickCount, duration, speed, divisor,
				(int)playing, (int)allplaying, (int)animating,
				(int)currentMotionHasLoop(),
				(int)isVariableDrivenResource(),
				(int)shouldKeepPlayingAtDuration(),
				(int)hasPlayableMotion(),
				currentMotionInfo ? currentMotionInfo->loopTime : -9999.0,
				res ? res->variables.size() : (size_t)0,
				currentMotionInfo ? currentMotionInfo->variables.size() : (size_t)0);
			MotionClickBurstTraceLog("player", buf);
		}
		// Diagnostic: log playback state periodically (every ~2s at 60fps)
		static int progressLogCounter = 0;
		if (MotionVerboseLogWanted() && ++progressLogCounter % 120 == 1) {
			std::shared_ptr<MotionResource> res = resolveResource();
			std::string storageName = res ? res->storage.AsNarrowStdString() : "(null)";
			std::string motionName = motion.GetLen() > 0 ? motion.AsNarrowStdString() : "(none)";
			tjs_real syncT = getSelectedSyncTime();
			tjs_real selDur = getSelectedDuration();
			fprintf(stderr,
				"[EMOTE-PROGRESS] playing=%d tick=%.1f duration=%.1f syncTime=%.1f selDur=%.1f "
				"speed=%.1f divisor=%.1f timeScale=%.1f hasMotion=%d playWas=%d stopSent=%d "
				"wallMs=%.0f storage=%s motion=%s\n",
				(int)playing, tickCount, duration, syncT, selDur,
				speed, divisor, timeScale, (int)hasPlayableMotion(),
				(int)playWasCalled, (int)stopCommandSent,
				progressWallTimeMs,
				storageName.c_str(), motionName.c_str());
			fflush(stderr);
		}
#endif
		// Track wall-clock time spent in active playback
		progressWallTimeMs += msTime;

		if (duration <= 0.0 && !hasPlayableMotion()) {
			finishPlayback();
			updateCurrentFrame();
			return;
		}
		if (duration > 0.0 && tickCount < duration) {
			tjs_real inputReadyTime = getInteractiveMotionInputReadyTime(duration);
			if (inputReadyTime > 0.0 && tickCount >= inputReadyTime) {
				tickCount = inputReadyTime;
#ifdef __EMSCRIPTEN__
				{
					std::shared_ptr<MotionResource> res = resolveResource();
					fprintf(stderr,
						"[EMOTE-PROGRESS] input-ready-release storage=%s chara=%s motion=%s tick=%.1f ready=%.1f duration=%.1f last=%.1f selfSync=%.1f all=%d anim=%d\n",
						res ? res->storage.AsNarrowStdString().c_str() : "(null)",
						chara.GetLen() > 0 ? chara.AsNarrowStdString().c_str() : "(none)",
						motion.GetLen() > 0 ? motion.AsNarrowStdString().c_str() : "(none)",
						tickCount,
						inputReadyTime,
						duration,
						currentMotionInfo ? currentMotionInfo->lastTime : 0.0,
						currentMotionInfo ? currentMotionInfo->selfSyncTime : 0.0,
						(int)allplaying,
						(int)animating);
					fflush(stderr);
				}
#endif
				finishInputPlaybackKeepAnimating();
				updateTimelineVariables();
				updateCurrentFrame();
				return;
			}
		}
		if (duration > 0.0 && tickCount >= duration) {
			if (shouldKeepPlayingAtDuration()) {
				if (MotionClickBurstTraceActive()) {
					std::shared_ptr<MotionResource> res = resolveResource();
					char buf[384];
					std::snprintf(buf, sizeof(buf),
						"duration-keep storage=%s chara=%s motion=%s tick=%.1f duration=%.1f loop=%d var=%d currentLoop=%.1f",
						res ? res->storage.AsNarrowStdString().c_str() : "(null)",
						chara.GetLen() > 0 ? chara.AsNarrowStdString().c_str() : "(none)",
						motion.GetLen() > 0 ? motion.AsNarrowStdString().c_str() : "(none)",
						tickCount, duration,
						(int)currentMotionHasLoop(),
						(int)isVariableDrivenResource(),
						currentMotionInfo ? currentMotionInfo->loopTime : -9999.0);
					MotionClickBurstTraceLog("player", buf);
				}
					if (!wrapLoopTick(duration) && !isVariableDrivenResource()) {
						tickCount = duration;
					}
					if (isVariableDrivenResource() && !currentMotionHasLoop()) {
						tickCount = duration;
						finishInputPlaybackKeepAnimating();
					} else {
						playing = true;
						animating = true;
						allplaying = true;
					}
				} else {
				if (MotionClickBurstTraceActive()) {
					std::shared_ptr<MotionResource> res = resolveResource();
					char buf[384];
					std::snprintf(buf, sizeof(buf),
						"duration-finish storage=%s chara=%s motion=%s tick=%.1f duration=%.1f loop=%d var=%d",
						res ? res->storage.AsNarrowStdString().c_str() : "(null)",
						chara.GetLen() > 0 ? chara.AsNarrowStdString().c_str() : "(none)",
						motion.GetLen() > 0 ? motion.AsNarrowStdString().c_str() : "(none)",
						tickCount, duration,
						(int)currentMotionHasLoop(),
						(int)isVariableDrivenResource());
					MotionClickBurstTraceLog("player", buf);
				}
				tickCount = duration;
				finishPlayback();
#ifdef __EMSCRIPTEN__
				if (MotionVerboseLogWanted()) {
					fprintf(stderr,
						"[EMOTE-PROGRESS] finishPlayback: tick=%.1f reached duration=%.1f\n",
						tickCount, duration);
					fflush(stderr);
				}
#endif
			}
		}
		// Logo playback is currently the only path that drives KAG [waitmovie]
		// through this WASM-side bridge. If the parsed timing does not emit a
		// clean stop, cap only that intro path so later character motion is not
		// force-ended by an intro-specific recovery guard.
		std::shared_ptr<MotionResource> safetyResource = resolveResource();
		ttstr safetyPath = safetyResource ? safetyResource->storage :
			(resourceManager ? resourceManager->getLoadedPath() : ttstr());
		bool logoPlayback = IsLogoPath(safetyPath);
		if (playing && logoPlayback && progressWallTimeMs > 15000.0) {
#ifdef __EMSCRIPTEN__
			if (MotionVerboseLogWanted()) {
				fprintf(stderr,
					"[EMOTE-PROGRESS] safety-net finishPlayback: wallMs=%.0f tick=%.1f duration=%.1f\n",
					progressWallTimeMs, tickCount, duration);
				fflush(stderr);
			}
#endif
			if (duration > 0.0) tickCount = duration;
			finishPlayback();
		}
		updateTimelineVariables();
		updateCurrentFrame();
	}
		void draw(iTJSDispatch2 *layer)
		{
			drawInvocationCount++;
			lastDrawAttempted = true;
			lastDrawHadTarget = false;
			lastDrawCacheHit = false;
			lastDrawLoadedLayerImage = false;
			lastDrawUsedFallback = false;
			lastDrawResult = TJS_W("no-resource");
			selectCurrentMotion();
			std::shared_ptr<MotionResource> resource = resolveResource();
			if (!resource && (!resourceManager || !resourceManager->hasLoadedPath())) return;
			iTJSDispatch2 *target = ResolveLayerTarget(layer);
			lastDrawHadTarget = target != nullptr;
			if (!target) {
				lastDrawResult = TJS_W("no-target");
				return;
			}

			const ttstr &path = resource ? resource->storage : resourceManager->getLoadedPath();
			if (lastDrawTarget == target && lastDrawPath == path &&
				lastDrawFrame == currentFrame && lastDrawTick == tickCount) {
				lastDrawCacheHit = true;
				lastDrawResult = TJS_W("cache-hit");
				return;
			}

			lastDrawTarget = target;
			lastDrawPath = path;
			lastDrawFrame = currentFrame;
			lastDrawTick = tickCount;
			drawActualCount++;

			MotionResource runtimeDrawResource;
			const MotionResource *drawResource = resource.get();
			drawRuntimeResourceBuilt = false;
			drawRuntimeLayoutCount = 0;
			drawRuntimeFrontDrawItemCount = 0;
			drawRuntimeFrontHitShapeCount = 0;
			drawRuntimeRenderMethodItemCount = 0;
			drawRuntimeSource = TJS_W("parse-time-resource");
			drawRuntimeObject = chara;
			drawRuntimeMotion = motion;
			drawRuntimeTick = tickCount;
			drawRuntimeDiagnosticResourceValid = false;
			drawRuntimeLimitOriginX = 0.0;
			drawRuntimeLimitOriginY = 0.0;
			drawRuntimeLimitWidth = resource ? resource->width : 0.0;
			drawRuntimeLimitHeight = resource ? resource->height : 0.0;
			drawRuntimeLimitZMax = kMotionDefaultLimitZMax;
				MotionResolveRuntimeDrawLimit(target,
					resource ? (tjs_real)resource->width : 0.0,
					resource ? (tjs_real)resource->height : 0.0,
					drawRuntimeLimitOriginX, drawRuntimeLimitOriginY,
					drawRuntimeLimitWidth, drawRuntimeLimitHeight,
					drawRuntimeLimitZMax);
				std::map<std::string, tjs_real> drawVariables =
					makeEffectiveVariableValues(resource.get());
				if (resource &&
					MotionBuildRuntimeDrawResource(
						*resource, chara, motion, tickCount,
						drawRuntimeLimitOriginX, drawRuntimeLimitOriginY,
						drawRuntimeLimitWidth, drawRuntimeLimitHeight,
						runtimeDrawResource, &drawVariables)) {
				drawResource = &runtimeDrawResource;
				drawRuntimeResourceBuilt = true;
				drawRuntimeDiagnosticResource = runtimeDrawResource;
				drawRuntimeDiagnosticResourceValid = true;
				drawRuntimeSource = TJS_W("selected-motion-runtime-layout");
				drawRuntimeObject = runtimeDrawResource.chara;
				drawRuntimeMotion = runtimeDrawResource.motion;
			}
#ifdef __EMSCRIPTEN__
			if (MotionClickBurstTraceActive()) {
				char buf[512];
				std::snprintf(buf, sizeof(buf),
					"draw-build storage=%s chara=%s motion=%s tick=%.3f built=%d vars=%zu layouts=%zu ir=%zu source=%s",
					resource ? resource->storage.AsNarrowStdString().c_str() : "(null)",
					chara.GetLen() > 0 ? chara.AsNarrowStdString().c_str() : "(none)",
					motion.GetLen() > 0 ? motion.AsNarrowStdString().c_str() : "(none)",
					(double)tickCount, (int)drawRuntimeResourceBuilt,
					drawVariables.size(),
					drawResource ? drawResource->layouts.size() : (size_t)0,
					drawResource ? drawResource->renderMethodIR.size() : (size_t)0,
					drawRuntimeSource.AsNarrowStdString().c_str());
				MotionClickBurstTraceLog("draw", buf);
			}
#endif
			drawRuntimeLayoutCount = drawResource ?
				(tjs_int)drawResource->layouts.size() : 0;
			drawRuntimeRenderMethodItemCount = drawResource ?
				(tjs_int)drawResource->renderMethodIR.size() : 0;
			drawRuntimeFrontDrawItemCount =
				(tjs_int)MotionCollectRuntimeDrawLayouts(drawResource).size();
			drawRuntimeFrontHitShapeCount =
				(tjs_int)MotionCollectFrontLayouts(drawResource, true).size();

			ttstr hintChara = chara;
			ttstr hintMotion = motion;
			if (currentMotionInfo) {
				if (hintChara.GetLen() <= 0) hintChara = currentMotionInfo->objectName;
				if (hintMotion.GetLen() <= 0) hintMotion = currentMotionInfo->name;
			}
			const ttstr &vbPath = resource ? resource->storage :
				(resourceManager ? resourceManager->getLoadedPath() : ttstr());
			if (g_motionRendererBackend == MotionRendererBackendCpuIR) {
				WarmMotionTextureSnapshot(vbPath, hintChara, hintMotion, tickCount);
			}
			const TVPStaticMotionBridgeSnapshot *commandTextureSnapshot =
				MotionResolveStaticBridgeSnapshot(vbPath, drawResource);

			rendererCommandEntries =
				MotionBuildRendererCommandEntriesFromDrawItems(
					drawResource, meshDivisionRatio);
			MotionFilterRendererCommandsBySnapshotTextures(
				rendererCommandEntries, drawResource, commandTextureSnapshot);
			drawTimeConsumedDrawItemCount = (tjs_int)rendererCommandEntries.size();
			drawTimePlanSourceCount = drawTimeConsumedDrawItemCount;
			drawTimeConsumedVertexCount =
				MotionRendererCommandVertexTotal(rendererCommandEntries);
			drawTimeConsumedIndexCount =
				MotionRendererCommandIndexTotal(rendererCommandEntries);
			drawTimeConsumedCellCount =
				MotionRendererCommandQuadTotal(rendererCommandEntries);
			drawTimePlanHash = MotionPlanHashText(
				MotionHashRendererCommandSharedFields(rendererCommandEntries));
			drawTimeOffsetContiguousOk = true;
			drawTimePerItemCountOk = true;
			drawTimePerItemOrderOk = true;
			drawTimePerItemSliceOk = true;
			for (size_t i = 0; i < rendererCommandEntries.size(); i++) {
				const MotionRendererCommandEntryInfo &entry = rendererCommandEntries[i];
				if (entry.drawItemIndex != (tjs_int)i) drawTimePerItemOrderOk = false;
				if (entry.vertexCount <= 0 || entry.indexCount <= 0 ||
					entry.cellCount <= 0 || entry.quadCount != entry.cellCount) {
					drawTimePerItemCountOk = false;
				}
				if (i == 0) {
					if (entry.vertexOffset != 0 || entry.indexOffset != 0 ||
						entry.firstCellIndex != 0) drawTimeOffsetContiguousOk = false;
				} else {
					const MotionRendererCommandEntryInfo &prev =
						rendererCommandEntries[i - 1];
					if (entry.vertexOffset != prev.vertexOffset + prev.vertexCount ||
						entry.indexOffset != prev.indexOffset + prev.indexCount ||
						entry.firstCellIndex != prev.firstCellIndex + prev.cellCount) {
						drawTimeOffsetContiguousOk = false;
					}
				}
				ttstr expectedVertexSlice = ttstr(entry.vertexOffset) + TJS_W("-") +
					ttstr(entry.vertexOffset + entry.vertexCount - 1);
				ttstr expectedIndexSlice = ttstr(entry.indexOffset) + TJS_W("-") +
					ttstr(entry.indexOffset + entry.indexCount - 1);
				if (entry.vertexSlice != expectedVertexSlice ||
					entry.indexSlice != expectedIndexSlice) {
					drawTimePerItemSliceOk = false;
				}
				if (i == 0) {
					drawTimeFirstItemOrder = entry.drawItemIndex;
					drawTimeFirstItemVertexOffset = entry.vertexOffset;
					drawTimeFirstItemIndexOffset = entry.indexOffset;
					drawTimeFirstItemVertexCount = entry.vertexCount;
					drawTimeFirstItemIndexCount = entry.indexCount;
					drawTimeFirstItemCellCount = entry.cellCount;
					drawTimeFirstItemFirstCellIndex = entry.firstCellIndex;
					drawTimeFirstItemHash = entry.sampleHash;
				}
			}
			if (!rendererCommandEntries.empty()) {
				const MotionRendererCommandEntryInfo &lastEntry =
					rendererCommandEntries.back();
				drawTimeLastItemOrder = lastEntry.drawItemIndex;
				drawTimeLastItemVertexOffset = lastEntry.vertexOffset;
				drawTimeLastItemIndexOffset = lastEntry.indexOffset;
				drawTimeLastItemVertexCount = lastEntry.vertexCount;
				drawTimeLastItemIndexCount = lastEntry.indexCount;
				drawTimeLastItemCellCount = lastEntry.cellCount;
				drawTimeLastItemFirstCellIndex = lastEntry.firstCellIndex;
				drawTimeLastItemHash = lastEntry.sampleHash;
			}
			drawTimeConsumptionValid = true;
			rendererCommandHash =
				MotionHashRendererCommandEntries(rendererCommandEntries);
			rendererCommandBuilt = true;
			rendererCommandOffsetsContiguous = drawTimeOffsetContiguousOk;
			rendererCommandPerEntryOk = drawTimePerItemCountOk &&
				drawTimePerItemOrderOk && drawTimePerItemSliceOk;
			rendererConsumerSummary =
				MotionConsumeRendererCommands(rendererCommandEntries);

			{
				bool hasIR = drawResource && !drawResource->renderMethodIR.empty();
				tjs_int irLinkedCount = 0;
				tjs_int irType1LinkedCount = 0;
				tjs_int irType2LinkedCount = 0;
				tjs_int irType3LinkedCount = 0;
				for (size_t ci = 0; ci < rendererCommandEntries.size(); ci++) {
					tjs_int ri = rendererCommandEntries[ci].renderMethodItemIndex;
					if (ri >= 0) {
						irLinkedCount++;
						if (drawResource && ri < (tjs_int)drawResource->renderMethodIR.size()) {
							switch (drawResource->renderMethodIR[(size_t)ri].type) {
								case 1: irType1LinkedCount++; break;
								case 2: irType2LinkedCount++; break;
								case 3: irType3LinkedCount++; break;
							}
						}
					}
				}
				if (hasIR && irLinkedCount > 0) {
					rendererConsumerSummary.rendererCommandSource =
						ttstr(TJS_W("renderMethodIR-in-")) +
						ttstr((tjs_int)drawResource->renderMethodIR.size()) +
						TJS_W("-items-cmd-linked=") + ttstr(irLinkedCount);
					rendererConsumerSummary.consumedIR = true;
				} else if (hasIR) {
					rendererConsumerSummary.rendererCommandSource =
						TJS_W("renderMethodIR-no-link");
				} else {
					rendererConsumerSummary.rendererCommandSource =
						TJS_W("layouts-only-no-renderMethodIR");
				}
				rendererConsumerSummary.consumedIR =
					hasIR && irLinkedCount > 0;
				if (resource.get()) {
					resource->motionRenderMethodSemantics.rendererCommandSource =
						rendererConsumerSummary.rendererCommandSource;
					resource->motionRenderMethodSemantics.rendererCommandConsumedIR =
						rendererConsumerSummary.consumedIR;
					resource->motionRenderMethodSemantics.linkedCommandCount =
						irLinkedCount;
					resource->motionRenderMethodSemantics.consumedType1CommandCount =
						irType1LinkedCount;
					resource->motionRenderMethodSemantics.consumedType2CommandCount =
						irType2LinkedCount;
					resource->motionRenderMethodSemantics.consumedType3CommandCount =
						irType3LinkedCount;
				}
			}

			// M4 trace setup: capture motion / tick / lim so per-vertex emit
			// inside upload prep can write a comparable JSONL line.
			g_motionTraceM4Setup.motion = motion.AsNarrowStdString();
			g_motionTraceM4Setup.tick = tickCount;
			g_motionTraceM4Setup.limOriginX = 0.0;
			g_motionTraceM4Setup.limOriginY = 0.0;
			g_motionTraceM4Setup.limWidth = 1280.0;
			g_motionTraceM4Setup.limHeight = 720.0;
			g_motionTraceM4Setup.limZMax = 30.0;
			g_motionTraceM4Setup.active = true;

			uploadPrepSummary =
				MotionPrepareRendererUploadFromCommands(
					rendererCommandEntries, drawResource,
					meshDivisionRatio, &stagingArtifact);

			g_motionTraceM4Setup.active = false;

				bool rendererDiagnostics = MotionRendererDiagnosticsWanted();
				if (rendererDiagnostics) {
					shadowDrawDiagnostic =
						MotionConsumeRendererStagingForShadowDraw(
							rendererCommandEntries, stagingArtifact,
							uploadPrepSummary);
					offscreenDrawDiagnostic =
						MotionRasterizeRendererOffscreenDrawDiagnostic(
							rendererCommandEntries, stagingArtifact,
							shadowDrawDiagnostic);
				} else {
					shadowDrawDiagnostic = MotionShadowDrawDiagnostic();
					offscreenDrawDiagnostic = MotionOffscreenDrawDiagnostic();
				}

			tTJSNI_BaseLayer *targetNativeLayer = MotionGetNativeLayer(target);
			tjs_int targetLayerLeft = 0;
			tjs_int targetLayerTop = 0;
			tjs_int targetLayerWidth = 0;
			tjs_int targetLayerHeight = 0;
			if (targetNativeLayer) {
				targetLayerLeft = targetNativeLayer->GetLeft();
				targetLayerTop = targetNativeLayer->GetTop();
				targetLayerWidth = (tjs_int)targetNativeLayer->GetWidth();
				targetLayerHeight = (tjs_int)targetNativeLayer->GetHeight();
			}
			MotionScopedDrawableLayerType targetLayerType(targetNativeLayer);
			if (targetLayerType.changedType()) {
				KRKR_LOG_L2("[emoteplayer] Player.draw prepared drawable target layer: %s\n",
					path.AsNarrowStdString().c_str());
			}
			bool runtimeRendererReady =
				g_motionRendererBackend == MotionRendererBackendCpuIR &&
				stagingArtifact.position.size() > 0 &&
				stagingArtifact.index.size() > 0 &&
				!rendererCommandEntries.empty() &&
				MotionHasStaticBridgeTextureSnapshot(vbPath, drawResource);
			// Parsed motion must not paint a visible static PSB frame while CPU-IR catches up.
			bool suppressStaticMotionFallback =
				g_motionRendererBackend == MotionRendererBackendCpuIR &&
				drawResource && drawResource->parsed &&
				(drawRuntimeResourceBuilt ||
				 !drawResource->motions.empty() ||
				 !drawResource->renderMethodIR.empty());
			bool loadedLayerImage = false;
			if (g_motionRendererBackend == MotionRendererBackendStaticPsb) {
				loadedLayerImage =
					LoadImageIntoLayer(target, path, hintChara, hintMotion, tickCount);
			} else if (!runtimeRendererReady && !suppressStaticMotionFallback) {
				loadedLayerImage =
					LoadImageIntoLayer(target, path, hintChara, hintMotion, tickCount);
			}
			if (loadedLayerImage) {
				lastDrawLoadedLayerImage = true;
				lastDrawResult =
					(g_motionRendererBackend == MotionRendererBackendStaticPsb) ?
					TJS_W("loaded-layer-image-tick") :
					TJS_W("loaded-layer-image");
				KRKR_LOG_L2("[emoteplayer] Player.draw loaded layer image: %s\n",
					path.AsNarrowStdString().c_str());
			} else if (runtimeRendererReady) {
				lastDrawResult = TJS_W("runtime-visible-bridge");
			} else {
				lastDrawResult =
					(g_motionRendererBackend == MotionRendererBackendCpuIR) ?
					TJS_W("runtime-visible-bridge-not-ready") :
					TJS_W("fallback-frame");
				if (g_motionRendererBackend != MotionRendererBackendCpuIR) {
					lastDrawUsedFallback = true;
					KRKR_LOG_L2("[emoteplayer] Player.draw completed fallback frame: %s\n",
						path.AsNarrowStdString().c_str());
				}
			}

				if (rendererDiagnostics) {
					diagnosticRenderBridge =
						MotionBuildDiagnosticRenderBridge(
							rendererCommandEntries, stagingArtifact,
							offscreenDrawDiagnostic, shadowDrawDiagnostic,
							lastDrawHadTarget, lastDrawUsedFallback);
				} else {
					diagnosticRenderBridge = MotionDiagnosticRenderBridge();
				}
			if (runtimeRendererReady &&
				stagingArtifact.position.size() > 0 &&
				stagingArtifact.index.size() > 0 && !rendererCommandEntries.empty()) {
				visibleRenderBridge =
					MotionBuildVisibleRenderBridge(
						target, rendererCommandEntries,
						stagingArtifact, drawResource,
						offscreenDrawDiagnostic,
						vbPath, !runtimeRendererReady && lastDrawUsedFallback);
				if (visibleRenderBridge.rendered &&
					visibleRenderBridge.mismatchKind == TJS_W("none")) {
					lastDrawResult = TJS_W("runtime-visible-bridge");
				} else if (!visibleRenderBridge.mismatchKind.IsEmpty()) {
					lastDrawResult =
						TJS_W("runtime-visible-bridge-") +
						visibleRenderBridge.mismatchKind;
				}
				} else {
					visibleRenderBridge = MotionVisibleRenderBridge();
					visibleRenderBridge.built = true;
					visibleRenderBridge.visibleBridgePath = vbPath;
					if (loadedLayerImage) {
					visibleRenderBridge.source = TJS_W("static-psb-tick-fallback");
					visibleRenderBridge.mismatchKind = TJS_W("none");
					visibleRenderBridge.commandCount =
						(tjs_int)rendererCommandEntries.size();
					visibleRenderBridge.targetWidth = targetLayerWidth;
					visibleRenderBridge.targetHeight = targetLayerHeight;
					visibleRenderBridge.rendered = loadedLayerImage;
					visibleRenderBridge.rendererPathUsed = loadedLayerImage;
				} else {
					visibleRenderBridge.mismatchKind = TJS_W("staging-not-ready");
				}
			}
		}
	void update() {}
	void render() {}
	// Minimal renderer contract (derived from krkrsdl3 emoteplayer semantics):
	// getRendererFrontState() exposes the following geometry-level fields
	// that a real WebGL2/CPU renderer should consume:
	//
	// ── per-draw-item (frontDrawItems[i]) ──
	//   name/objectPath/statePath  : semantic identity for hit-test routing
	//   left/top/width/height      : bounding rect (pre-transform)
	//   alphaMax                   : uniform opacity per draw call
	//   meshDivision               : number of grid subdivisions per axis
	//   quadCount                  : = meshDivision^2
	//   vertexCount/indexCount     : estimated buffer sizes
	//   v0..v3 (x/y)              : 4 corner vertices in screen space (CW or CCW)
	//
	// ── per-mesh-cell (frontMeshCells[j]) ──
	//   drawItemIndex              : parent draw item
	//   gridX/gridY                : cell position within draw item grid
	//   v0..v3 (x/y)              : 4 corner vertices in screen space
	//   uv0..uv3 (x/y)            : texture coordinates: (gridX/D, gridY/D) → ((gridX+1)/D, (gridY+1)/D)
	//   alpha0..alpha3             : uniform per-cell alpha (= draw item alphaMax for all 4 vertices)
	//   Contract invariants:
	//     alpha0 == alpha1 == alpha2 == alpha3
	//     uv_{i} ∈ [0, 1]
	//     adjacent cells: cell(x,y).uv1x == cell(x+1,y).uv0x, etc.
	//
	// ── per-hit-shape (frontHitShapes[k]) ──
	//   type="rect" / vertexCount=4
	//   v0..v3 (x/y)              : polygon vertices in screen space
	//   hitObjectPath              : object identity for hit-test dispatch
	//   alphaThreshold             : minimum alpha for hit-test pass
	//
	// ── renderer-prep (rendererPrep / top-level) ──
	//   meshDivision/meshDivisionRatio  : global subdivision factor
	//   total vertexCount/indexCount     : aggregate buffer allocation size
		//   drawOrder                        : z-sorted draw item indices
	//   bounds (left/top/width/height)   : composite FBO dimensions
	//
	// Reference: krkrsdl3 emoteplayer::emotenode::draw() uses
	//   surfaceCount × (matTrans + 16 controlPts) → glDrawArrays(GL_PATCHES, 0, 16)
	// with uniform totalOpa. This contract flattens the tessellation model
	// into pre-subdivided quads, suitable for WebGL2 (no tessellation shader).
	tTJSVariant getRendererFrontState() const
	{
		std::shared_ptr<MotionResource> resource = resolveResource();
		tTJSVariant prep = MakeMotionRendererPrepDump(resource.get(), meshDivisionRatio, true);
		MotionRendererPrepInfo info =
			MotionBuildRendererPrepInfo(resource.get(), meshDivisionRatio);
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		SetTjsProperty(dict, TJS_W("backend"),
			tTJSVariant(MotionRendererBackendName(g_motionRendererBackend)));
		SetTjsProperty(dict, TJS_W("rendererStage"), tTJSVariant(TJS_W("front-state")));
		SetTjsProperty(dict, TJS_W("resourceValid"), tTJSVariant((tjs_int)(resource && resource->opened ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("resourceParsed"), tTJSVariant((tjs_int)(resource && resource->parsed ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("resourceStorage"),
			tTJSVariant(resource ? resource->storage :
				(resourceManager ? resourceManager->getLoadedPath() : ttstr())));
		SetTjsProperty(dict, TJS_W("resourceWidth"), tTJSVariant(resource ? resource->width : 0));
		SetTjsProperty(dict, TJS_W("resourceHeight"), tTJSVariant(resource ? resource->height : 0));
		SetTjsProperty(dict, TJS_W("chara"), tTJSVariant(chara));
		SetTjsProperty(dict, TJS_W("motion"), tTJSVariant(motion));
		SetTjsProperty(dict, TJS_W("currentFrame"), tTJSVariant(currentFrame));
		SetTjsProperty(dict, TJS_W("tickCount"), tTJSVariant(tickCount));
		SetTjsProperty(dict, TJS_W("visible"), tTJSVariant((tjs_int)(visible ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("opacity"), tTJSVariant(opacity));
		SetTjsProperty(dict, TJS_W("coordX"), tTJSVariant(coordX));
		SetTjsProperty(dict, TJS_W("coordY"), tTJSVariant(coordY));
		SetTjsProperty(dict, TJS_W("scaleX"), tTJSVariant(scaleX));
		SetTjsProperty(dict, TJS_W("scaleY"), tTJSVariant(scaleY));
		SetTjsProperty(dict, TJS_W("rotation"), tTJSVariant(rotation));
		SetTjsProperty(dict, TJS_W("meshDivisionRatio"), tTJSVariant(meshDivisionRatio));
		SetTjsProperty(dict, TJS_W("meshDivision"), tTJSVariant(info.meshDivision));
		SetTjsProperty(dict, TJS_W("meshPrepared"), tTJSVariant((tjs_int)(info.meshPrepared ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("boundsValid"), tTJSVariant((tjs_int)(info.boundsValid ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("layoutCount"), tTJSVariant(info.layoutCount));
		SetTjsProperty(dict, TJS_W("drawItemCount"), tTJSVariant(info.drawItemCount));
		SetTjsProperty(dict, TJS_W("frontDrawItemCount"), tTJSVariant(info.frontDrawItemCount));
		SetTjsProperty(dict, TJS_W("frontMeshCellCount"), tTJSVariant(info.frontMeshCellCount));
		SetTjsProperty(dict, TJS_W("frontHitShapeCount"), tTJSVariant(info.frontHitShapeCount));
		SetTjsProperty(dict, TJS_W("frontObjectStateCount"), tTJSVariant(info.frontObjectStateCount));
		SetTjsProperty(dict, TJS_W("frontHitObjectStateCount"), tTJSVariant(info.frontHitObjectStateCount));
		SetTjsProperty(dict, TJS_W("alphaAwareDrawItemCount"), tTJSVariant(info.alphaAwareDrawItemCount));
		SetTjsProperty(dict, TJS_W("alphaAwareHitShapeCount"), tTJSVariant(info.alphaAwareHitShapeCount));
		SetTjsProperty(dict, TJS_W("estimatedQuadCount"), tTJSVariant(info.estimatedQuadCount));
		SetTjsProperty(dict, TJS_W("estimatedVertexCount"), tTJSVariant(info.estimatedVertexCount));
		SetTjsProperty(dict, TJS_W("estimatedIndexCount"), tTJSVariant(info.estimatedIndexCount));
		SetTjsProperty(dict, TJS_W("prepBoundsLeft"), tTJSVariant(info.left));
		SetTjsProperty(dict, TJS_W("prepBoundsTop"), tTJSVariant(info.top));
		SetTjsProperty(dict, TJS_W("prepBoundsWidth"), tTJSVariant(info.width));
		SetTjsProperty(dict, TJS_W("prepBoundsHeight"), tTJSVariant(info.height));
		SetTjsProperty(dict, TJS_W("bustScale"), tTJSVariant(bustScale));
		SetTjsProperty(dict, TJS_W("hairScale"), tTJSVariant(hairScale));
		SetTjsProperty(dict, TJS_W("partsScale"), tTJSVariant(partsScale));
		SetTjsProperty(dict, TJS_W("completionType"), completionType);
		{
			const MotionResource *resPtr = resource.get();
			const MotionTimeSemanticsDiag &diag = resPtr ? resPtr->timeSemanticsDiag : MotionTimeSemanticsDiag();
			iTJSDispatch2 *timeSem = TJSCreateDictionaryObject();
			SetTjsProperty(timeSem, TJS_W("probed"), tTJSVariant(diag.probed));
			SetTjsProperty(timeSem, TJS_W("nestedMotionCount"), tTJSVariant(diag.nestedMotionCount));
			SetTjsProperty(timeSem, TJS_W("timeOffsetNonzeroCount"), tTJSVariant(diag.timeOffsetNonzeroCount));
			SetTjsProperty(timeSem, TJS_W("parentTickSample"), tTJSVariant(diag.parentTickSample));
			SetTjsProperty(timeSem, TJS_W("childEvalTickSample"), tTJSVariant(diag.childEvalTickSample));
			SetTjsProperty(timeSem, TJS_W("timeOffsetSample"), tTJSVariant(diag.timeOffsetSample));
			SetTjsProperty(timeSem, TJS_W("requestedTickSample"), tTJSVariant(diag.requestedTickSample));
			SetTjsProperty(timeSem, TJS_W("effectiveTickSample"), tTJSVariant(diag.effectiveTickSample));
			SetTjsProperty(timeSem, TJS_W("twoFrameHoldTickSample"), tTJSVariant(diag.twoFrameHoldTickSample));
			SetTjsProperty(timeSem, TJS_W("firstNestedMotionPath"),
				tTJSVariant(diag.firstNestedMotionPath.empty() ? ttstr(TJS_W("none")) :
					ttstr(diag.firstNestedMotionPath.c_str())));
			SetTjsProperty(timeSem, TJS_W("timeOffsetConsumed"), tTJSVariant((tjs_int)(diag.timeOffsetConsumed ? 1 : 0)));
			SetTjsProperty(timeSem, TJS_W("selfSyncTimeClamped"), tTJSVariant((tjs_int)(diag.selfSyncTimeClamped ? 1 : 0)));
			SetTjsProperty(timeSem, TJS_W("twoFrameSpecialApplied"), tTJSVariant((tjs_int)(diag.twoFrameSpecialApplied ? 1 : 0)));
			SetTjsProperty(timeSem, TJS_W("tickProbeCount"), tTJSVariant(diag.tickProbeCount));
			SetTjsProperty(timeSem, TJS_W("metadataNestedMotionRefCount"), tTJSVariant(diag.metadataNestedMotionRefCount));
			SetTjsProperty(timeSem, TJS_W("metadataTimeOffsetNonzeroCount"), tTJSVariant(diag.metadataTimeOffsetNonzeroCount));
			if (!diag.skipReason.empty()) {
				SetTjsProperty(timeSem, TJS_W("skipReason"),
					tTJSVariant(ttstr(diag.skipReason.c_str())));
			}
			SetTjsProperty(timeSem, TJS_W("childEvalTickMatch"), tTJSVariant((tjs_int)(
				diag.nestedMotionCount > 0 &&
				std::abs(diag.childEvalTickSample - (diag.parentTickSample + diag.timeOffsetSample)) < 0.0001 ? 1 : 0)));
			tTJSVariant timeSemVal(timeSem, timeSem);
			timeSem->Release();
			SetTjsProperty(dict, TJS_W("motionTimeSemantics"), timeSemVal);
		}
		{
			const MotionResource *resPtr = resource.get();
			const MotionLimitSemanticsDiag &limDiag = resPtr ? resPtr->limitSemanticsDiag : MotionLimitSemanticsDiag();
			iTJSDispatch2 *limSem = TJSCreateDictionaryObject();
			SetTjsProperty(limSem, TJS_W("probed"), tTJSVariant(limDiag.probed));
			SetTjsProperty(limSem, TJS_W("nanCoordCount"), tTJSVariant(limDiag.nanCoordCount));
			SetTjsProperty(limSem, TJS_W("infCoordCount"), tTJSVariant(limDiag.infCoordCount));
			SetTjsProperty(limSem, TJS_W("type12LimInheritCount"), tTJSVariant(limDiag.type12LimInheritCount));
			SetTjsProperty(limSem, TJS_W("nondrawableLimInheritCount"), tTJSVariant(limDiag.nondrawableLimInheritCount));
			SetTjsProperty(limSem, TJS_W("childLimPropCount"), tTJSVariant(limDiag.childLimPropCount));
			SetTjsProperty(limSem, TJS_W("sampleParentLimOriginX"), tTJSVariant(limDiag.sampleParentLimOriginX));
			SetTjsProperty(limSem, TJS_W("sampleParentLimOriginY"), tTJSVariant(limDiag.sampleParentLimOriginY));
			SetTjsProperty(limSem, TJS_W("sampleParentLimWidth"), tTJSVariant(limDiag.sampleParentLimWidth));
			SetTjsProperty(limSem, TJS_W("sampleParentLimHeight"), tTJSVariant(limDiag.sampleParentLimHeight));
			SetTjsProperty(limSem, TJS_W("sampleParentLimZMax"), tTJSVariant(limDiag.sampleParentLimZMax));
			SetTjsProperty(limSem, TJS_W("sampleChildLimOriginX"), tTJSVariant(limDiag.sampleChildLimOriginX));
			SetTjsProperty(limSem, TJS_W("sampleChildLimOriginY"), tTJSVariant(limDiag.sampleChildLimOriginY));
			SetTjsProperty(limSem, TJS_W("sampleChildLimWidth"), tTJSVariant(limDiag.sampleChildLimWidth));
			SetTjsProperty(limSem, TJS_W("sampleChildLimHeight"), tTJSVariant(limDiag.sampleChildLimHeight));
			SetTjsProperty(limSem, TJS_W("sampleChildLimZMax"), tTJSVariant(limDiag.sampleChildLimZMax));
			SetTjsProperty(limSem, TJS_W("sampleFrameOriginX"), tTJSVariant(limDiag.sampleFrameOriginX));
			SetTjsProperty(limSem, TJS_W("sampleFrameOriginY"), tTJSVariant(limDiag.sampleFrameOriginY));
			SetTjsProperty(limSem, TJS_W("sampleFrameWidth"), tTJSVariant(limDiag.sampleFrameWidth));
			SetTjsProperty(limSem, TJS_W("sampleFrameHeight"), tTJSVariant(limDiag.sampleFrameHeight));
			SetTjsProperty(limSem, TJS_W("sampleCoordBeforeX"), tTJSVariant(limDiag.sampleCoordBeforeX));
			SetTjsProperty(limSem, TJS_W("sampleCoordBeforeY"), tTJSVariant(limDiag.sampleCoordBeforeY));
			SetTjsProperty(limSem, TJS_W("sampleCoordAfterX"), tTJSVariant(limDiag.sampleCoordAfterX));
			SetTjsProperty(limSem, TJS_W("sampleCoordAfterY"), tTJSVariant(limDiag.sampleCoordAfterY));
			SetTjsProperty(limSem, TJS_W("nanInfPathVerified"), tTJSVariant((tjs_int)(limDiag.nanInfPathVerified ? 1 : 0)));
			SetTjsProperty(limSem, TJS_W("metadataSurveyFrameCount"), tTJSVariant(limDiag.metadataSurveyFrameCount));
			SetTjsProperty(limSem, TJS_W("metadataSurveyNanCandidateCount"), tTJSVariant(limDiag.metadataSurveyNanCandidateCount));
			SetTjsProperty(limSem, TJS_W("metadataSurveyInfCandidateCount"), tTJSVariant(limDiag.metadataSurveyInfCandidateCount));
			SetTjsProperty(limSem, TJS_W("singleFrameNormalizeCount"), tTJSVariant(limDiag.singleFrameNormalizeCount));
			SetTjsProperty(limSem, TJS_W("interpolationFramePairCount"), tTJSVariant(limDiag.interpolationFramePairCount));
			SetTjsProperty(limSem, TJS_W("coordInterpolationCount"), tTJSVariant(limDiag.coordInterpolationCount));
			SetTjsProperty(limSem, TJS_W("interpolationNormalizeCount"), tTJSVariant(limDiag.interpolationNormalizeCount));
			if (!limDiag.skipReason.empty()) {
				SetTjsProperty(limSem, TJS_W("skipReason"),
					tTJSVariant(ttstr(limDiag.skipReason.c_str())));
			}
			tTJSVariant limSemVal(limSem, limSem);
			limSem->Release();
			SetTjsProperty(dict, TJS_W("motionLimitSemantics"), limSemVal);
		}
		{
			MotionResource *resPtr = resource.get();
			if (resPtr) {
				MotionAffineFrontMeshDiag &affDiag = resPtr->affineFrontMeshDiag;
				affDiag = MotionAffineFrontMeshDiag();
				affDiag.probed = 1;
				MotionRunAffineFrontMeshSelfTest(affDiag);
				std::vector<std::pair<const MotionLayoutInfo*, tjs_int> > frontItems =
					MotionCollectFrontLayouts(resPtr, false);
				affDiag.itemCount = (tjs_int)frontItems.size();
				for (size_t fi = 0; fi < frontItems.size(); fi++) {
					const MotionLayoutInfo *layout = frontItems[fi].first;
					if (!layout) continue;
					if (MotionLayoutNeedsAffinePath(layout)) {
						affDiag.affinePathItemCount++;
						if (MotionAffineHasNonAxisAlignedShape(layout->affine)) {
							affDiag.nonAxisAlignedItemCount++;
						}
					}
					if (std::abs(layout->angle) > 0.0001) affDiag.rotationConsumed = true;
					if (std::abs(layout->sx) > 0.0001 || std::abs(layout->sy) > 0.0001)
						affDiag.shearConsumed = true;
				}
				affDiag.affinePositionGenerated = true;
				if (affDiag.affinePathItemCount > 0 && frontItems.size() > 0) {
					const MotionLayoutInfo *firstAff = nullptr;
					for (size_t fi = 0; fi < frontItems.size(); fi++) {
						const MotionLayoutInfo *layout = frontItems[fi].first;
						if (!layout) continue;
						if (MotionLayoutNeedsAffinePath(layout) &&
							(affDiag.nonAxisAlignedItemCount == 0 ||
							MotionAffineHasNonAxisAlignedShape(layout->affine))) {
							firstAff = layout;
							break;
						}
					}
					if (firstAff) {
						affDiag.sampleAngle = firstAff->angle;
						affDiag.sampleSx = firstAff->sx;
						affDiag.sampleSy = firstAff->sy;
						affDiag.sampleOx = firstAff->ox;
						affDiag.sampleOy = firstAff->oy;
						affDiag.sampleIconOriginX = firstAff->iconOriginX;
						affDiag.sampleIconOriginY = firstAff->iconOriginY;
						affDiag.sampleIconWidth = firstAff->iconWidth;
						affDiag.sampleIconHeight = firstAff->iconHeight;
						affDiag.sampleAabbLeft = firstAff->left;
						affDiag.sampleAabbTop = firstAff->top;
						affDiag.sampleAabbWidth = firstAff->width;
						affDiag.sampleAabbHeight = firstAff->height;
						tjs_real q0x, q0y, q1x, q1y, q2x, q2y, q3x, q3y;
						MotionApplyIconDrawAffineToPoint(firstAff->affine, 0.0, 0.0, q0x, q0y);
						MotionApplyIconDrawAffineToPoint(firstAff->affine, 1.0, 0.0, q1x, q1y);
						MotionApplyIconDrawAffineToPoint(firstAff->affine, 1.0, 1.0, q2x, q2y);
						MotionApplyIconDrawAffineToPoint(firstAff->affine, 0.0, 1.0, q3x, q3y);
						affDiag.sampleQ0x = q0x; affDiag.sampleQ0y = q0y;
						affDiag.sampleQ1x = q1x; affDiag.sampleQ1y = q1y;
						affDiag.sampleQ2x = q2x; affDiag.sampleQ2y = q2y;
						affDiag.sampleQ3x = q3x; affDiag.sampleQ3y = q3y;
					}
				}
				if (affDiag.nonAxisAlignedItemCount == 0 && affDiag.itemCount > 0) {
					affDiag.skipReason = "no-non-axis-aligned-items-in-front-layouts";
				}
			}
		}
		{
			const MotionResource *resPtr = resource.get();
			const MotionRenderMethodSemanticsDiag &rmDiag =
				resPtr ? resPtr->motionRenderMethodSemantics :
					MotionRenderMethodSemanticsDiag();
			iTJSDispatch2 *rmSem = TJSCreateDictionaryObject();
			SetTjsProperty(rmSem, TJS_W("probed"), tTJSVariant(rmDiag.probed));
			SetTjsProperty(rmSem, TJS_W("renderMethodStackBuilt"),
				tTJSVariant((tjs_int)(rmDiag.stackBuilt ? 1 : 0)));
			SetTjsProperty(rmSem, TJS_W("layoutMergeCount"),
				tTJSVariant(rmDiag.layoutMergeCount));
			SetTjsProperty(rmSem, TJS_W("drawItemType2Count"),
				tTJSVariant(rmDiag.drawItemType2Count));
			SetTjsProperty(rmSem, TJS_W("drawItemType1Count"),
				tTJSVariant(rmDiag.drawItemType1Count));
			SetTjsProperty(rmSem, TJS_W("nestedMotionRenderPassCount"),
				tTJSVariant(rmDiag.nestedMotionRenderPassCount));
			SetTjsProperty(rmSem, TJS_W("totalItems"),
				tTJSVariant(rmDiag.totalItems));
			SetTjsProperty(rmSem, TJS_W("linkedCommandCount"),
				tTJSVariant(rmDiag.linkedCommandCount));
			SetTjsProperty(rmSem, TJS_W("linkedLayoutCount"),
				tTJSVariant(rmDiag.linkedLayoutCount));
			SetTjsProperty(rmSem, TJS_W("firstSampleType"),
				tTJSVariant(rmDiag.firstSampleType));
			SetTjsProperty(rmSem, TJS_W("firstSampleLabel"),
				tTJSVariant(rmDiag.firstSampleLabel));
			SetTjsProperty(rmSem, TJS_W("firstSampleOriginX"),
				tTJSVariant(rmDiag.firstSampleOriginX));
			SetTjsProperty(rmSem, TJS_W("firstSampleOriginY"),
				tTJSVariant(rmDiag.firstSampleOriginY));
			SetTjsProperty(rmSem, TJS_W("firstSampleWidth"),
				tTJSVariant(rmDiag.firstSampleWidth));
			SetTjsProperty(rmSem, TJS_W("firstSampleHeight"),
				tTJSVariant(rmDiag.firstSampleHeight));
			SetTjsProperty(rmSem, TJS_W("firstSampleOpacity"),
				tTJSVariant(rmDiag.firstSampleOpacity));
			SetTjsProperty(rmSem, TJS_W("firstSampleHasControlPoints"),
				tTJSVariant((tjs_int)(rmDiag.firstSampleHasControlPoints ? 1 : 0)));
			SetTjsProperty(rmSem, TJS_W("surfaceCount"),
				tTJSVariant(rmDiag.surfaceCount));
			SetTjsProperty(rmSem, TJS_W("itemWithSurfaceCount"),
				tTJSVariant(rmDiag.itemWithSurfaceCount));
			SetTjsProperty(rmSem, TJS_W("firstSampleSurfaceCount"),
				tTJSVariant(rmDiag.firstSampleSurfaceCount));
			SetTjsProperty(rmSem, TJS_W("firstSampleSurfaceOriginX"),
				tTJSVariant(rmDiag.firstSampleSurfaceOriginX));
			SetTjsProperty(rmSem, TJS_W("firstSampleSurfaceOriginY"),
				tTJSVariant(rmDiag.firstSampleSurfaceOriginY));
			SetTjsProperty(rmSem, TJS_W("firstSampleSurfaceOriginW"),
				tTJSVariant(rmDiag.firstSampleSurfaceOriginW));
			SetTjsProperty(rmSem, TJS_W("firstSampleSurfaceOriginH"),
				tTJSVariant(rmDiag.firstSampleSurfaceOriginH));
			SetTjsProperty(rmSem, TJS_W("firstSampleSurfaceOpa"),
				tTJSVariant(rmDiag.firstSampleSurfaceOpa));
			SetTjsProperty(rmSem, TJS_W("firstSampleSurfaceType"),
				tTJSVariant(rmDiag.firstSampleSurfaceType));
			SetTjsProperty(rmSem, TJS_W("firstSampleSurfaceMat00"),
				tTJSVariant(rmDiag.firstSampleSurfaceMat00));
			SetTjsProperty(rmSem, TJS_W("firstSampleSurfaceMat11"),
				tTJSVariant(rmDiag.firstSampleSurfaceMat11));
			SetTjsProperty(rmSem, TJS_W("firstSampleSurfaceHasStencil"),
				tTJSVariant((tjs_int)(rmDiag.firstSampleSurfaceHasStencil ? 1 : 0)));
			SetTjsProperty(rmSem, TJS_W("metadataSurveyFrameCount"),
				tTJSVariant(rmDiag.metadataSurveyFrameCount));
			SetTjsProperty(rmSem, TJS_W("metadataSurveyBpCandidateCount"),
				tTJSVariant(rmDiag.metadataSurveyBpCandidateCount));
			SetTjsProperty(rmSem, TJS_W("rendererCommandSource"),
				tTJSVariant(rmDiag.rendererCommandSource));
			SetTjsProperty(rmSem, TJS_W("rendererCommandConsumedIR"),
				tTJSVariant((tjs_int)(rmDiag.rendererCommandConsumedIR ? 1 : 0)));
			SetTjsProperty(rmSem, TJS_W("consumedType1CommandCount"),
				tTJSVariant(rmDiag.consumedType1CommandCount));
			SetTjsProperty(rmSem, TJS_W("consumedType2CommandCount"),
				tTJSVariant(rmDiag.consumedType2CommandCount));
			SetTjsProperty(rmSem, TJS_W("consumedType3CommandCount"),
				tTJSVariant(rmDiag.consumedType3CommandCount));
			SetTjsProperty(rmSem, TJS_W("m2SelfTested"),
				tTJSVariant(rmDiag.m2SelfTested));
			SetTjsProperty(rmSem, TJS_W("m2SelfTestPassed"),
				tTJSVariant(rmDiag.m2SelfTestPassed));
			SetTjsProperty(rmSem, TJS_W("m2SingleSurfaceItemCount"),
				tTJSVariant(rmDiag.m2SingleSurfaceItemCount));
			SetTjsProperty(rmSem, TJS_W("m2UnsupportedDemuxCount"),
				tTJSVariant(rmDiag.m2UnsupportedDemuxCount));
			SetTjsProperty(rmSem, TJS_W("m2SampleMaxAbsError"),
				tTJSVariant(rmDiag.m2SampleMaxAbsError));
			SetTjsProperty(rmSem, TJS_W("m2SampleMat00"),
				tTJSVariant(rmDiag.m2SampleMat00));
			SetTjsProperty(rmSem, TJS_W("m2SampleMat11"),
				tTJSVariant(rmDiag.m2SampleMat11));
			SetTjsProperty(rmSem, TJS_W("m2SampleMat03"),
				tTJSVariant(rmDiag.m2SampleMat03));
			SetTjsProperty(rmSem, TJS_W("m2SampleMat13"),
				tTJSVariant(rmDiag.m2SampleMat13));
			SetTjsProperty(rmSem, TJS_W("m3SelfTested"),
				tTJSVariant(rmDiag.m3SelfTested));
			SetTjsProperty(rmSem, TJS_W("m3SelfTestPassed"),
				tTJSVariant(rmDiag.m3SelfTestPassed));
			SetTjsProperty(rmSem, TJS_W("m3MultiSurfaceItemCount"),
				tTJSVariant(rmDiag.m3MultiSurfaceItemCount));
			SetTjsProperty(rmSem, TJS_W("m3DemuxResolvedCount"),
				tTJSVariant(rmDiag.m3DemuxResolvedCount));
			SetTjsProperty(rmSem, TJS_W("m3SampleSurfaceCount"),
				tTJSVariant(rmDiag.m3SampleSurfaceCount));
			SetTjsProperty(rmSem, TJS_W("m3SampleMaxAbsError"),
				tTJSVariant(rmDiag.m3SampleMaxAbsError));
			SetTjsProperty(rmSem, TJS_W("m3SampleMat00"),
				tTJSVariant(rmDiag.m3SampleMat00));
			SetTjsProperty(rmSem, TJS_W("m3SampleMat11"),
				tTJSVariant(rmDiag.m3SampleMat11));
			SetTjsProperty(rmSem, TJS_W("m3SampleMat03"),
				tTJSVariant(rmDiag.m3SampleMat03));
			SetTjsProperty(rmSem, TJS_W("m3SampleMat13"),
				tTJSVariant(rmDiag.m3SampleMat13));
			SetTjsProperty(rmSem, TJS_W("m4SelfTested"),
				tTJSVariant(rmDiag.m4SelfTested));
			SetTjsProperty(rmSem, TJS_W("m4SelfTestPassed"),
				tTJSVariant(rmDiag.m4SelfTestPassed));
			SetTjsProperty(rmSem, TJS_W("m4SurfaceChainCommandCount"),
				tTJSVariant(rmDiag.m4SurfaceChainCommandCount));
			SetTjsProperty(rmSem, TJS_W("m4MaxSurfaceChainLength"),
				tTJSVariant(rmDiag.m4MaxSurfaceChainLength));
			SetTjsProperty(rmSem, TJS_W("m4RemappedVertexCount"),
				tTJSVariant(rmDiag.m4RemappedVertexCount));
			SetTjsProperty(rmSem, TJS_W("m4ViewportTransformedVertexCount"),
				tTJSVariant(rmDiag.m4ViewportTransformedVertexCount));
			SetTjsProperty(rmSem, TJS_W("m4SampleNdcX"),
				tTJSVariant(rmDiag.m4SampleNdcX));
			SetTjsProperty(rmSem, TJS_W("m4SampleNdcY"),
				tTJSVariant(rmDiag.m4SampleNdcY));
			SetTjsProperty(rmSem, TJS_W("m4SamplePixelX"),
				tTJSVariant(rmDiag.m4SamplePixelX));
			SetTjsProperty(rmSem, TJS_W("m4SamplePixelY"),
				tTJSVariant(rmDiag.m4SamplePixelY));
			SetTjsProperty(rmSem, TJS_W("m4SampleMaxAbsError"),
				tTJSVariant(rmDiag.m4SampleMaxAbsError));
			SetTjsProperty(rmSem, TJS_W("m5SelfTested"),
				tTJSVariant(rmDiag.m5SelfTested));
			SetTjsProperty(rmSem, TJS_W("m5SelfTestPassed"),
				tTJSVariant(rmDiag.m5SelfTestPassed));
			SetTjsProperty(rmSem, TJS_W("m5Type1SurfaceCount"),
				tTJSVariant(rmDiag.m5Type1SurfaceCount));
			SetTjsProperty(rmSem, TJS_W("m5ControlPointSurfaceConsumed"),
				tTJSVariant(rmDiag.m5ControlPointSurfaceConsumed));
			SetTjsProperty(rmSem, TJS_W("m5SampleAffinePixelX"),
				tTJSVariant(rmDiag.m5SampleAffinePixelX));
			SetTjsProperty(rmSem, TJS_W("m5SampleAffinePixelY"),
				tTJSVariant(rmDiag.m5SampleAffinePixelY));
			SetTjsProperty(rmSem, TJS_W("m5SampleDeformedPixelX"),
				tTJSVariant(rmDiag.m5SampleDeformedPixelX));
			SetTjsProperty(rmSem, TJS_W("m5SampleDeformedPixelY"),
				tTJSVariant(rmDiag.m5SampleDeformedPixelY));
			SetTjsProperty(rmSem, TJS_W("m6CurrCoordzSample"),
				tTJSVariant(rmDiag.firstSampleSurfaceCurrCoordz));
			SetTjsProperty(rmSem, TJS_W("m6HasStencilSample"),
				tTJSVariant((tjs_int)(rmDiag.firstSampleSurfaceHasStencil ? 1 : 0)));
			SetTjsProperty(rmSem, TJS_W("m5SampleDeviationDist"),
				tTJSVariant(rmDiag.m5SampleDeviationDist));
			SetTjsProperty(rmSem, TJS_W("m5SampleControlPt1X"),
				tTJSVariant(rmDiag.m5SampleControlPt1X));
			SetTjsProperty(rmSem, TJS_W("m5SampleControlPt1Y"),
				tTJSVariant(rmDiag.m5SampleControlPt1Y));
			if (!rmDiag.skipReason.empty()) {
				SetTjsProperty(rmSem, TJS_W("skipReason"),
					tTJSVariant(ttstr(rmDiag.skipReason.c_str())));
			}
			tTJSVariant rmSemVal(rmSem, rmSem);
			rmSem->Release();
			SetTjsProperty(dict, TJS_W("motionRenderMethodSemantics"), rmSemVal);
		}
		{
			const MotionResource *resPtr = resource.get();
			const MotionAffineFrontMeshDiag &affDiag =
				resPtr ? resPtr->affineFrontMeshDiag : MotionAffineFrontMeshDiag();
			iTJSDispatch2 *affSem = TJSCreateDictionaryObject();
			SetTjsProperty(affSem, TJS_W("probed"), tTJSVariant(affDiag.probed));
			SetTjsProperty(affSem, TJS_W("itemCount"), tTJSVariant(affDiag.itemCount));
			SetTjsProperty(affSem, TJS_W("affinePathItemCount"),
				tTJSVariant(affDiag.affinePathItemCount));
			SetTjsProperty(affSem, TJS_W("nonAxisAlignedItemCount"),
				tTJSVariant(affDiag.nonAxisAlignedItemCount));
			SetTjsProperty(affSem, TJS_W("affineSelfTested"),
				tTJSVariant(affDiag.affineSelfTested));
			SetTjsProperty(affSem, TJS_W("affineSelfTestPassed"),
				tTJSVariant(affDiag.affineSelfTestPassed));
			SetTjsProperty(affSem, TJS_W("affineSelfTestNonAxisAligned"),
				tTJSVariant(affDiag.affineSelfTestNonAxisAligned));
			SetTjsProperty(affSem, TJS_W("affineSelfTestShearOk"),
				tTJSVariant(affDiag.affineSelfTestShearOk));
			SetTjsProperty(affSem, TJS_W("affineSelfTestNestedOk"),
				tTJSVariant(affDiag.affineSelfTestNestedOk));
			SetTjsProperty(affSem, TJS_W("affineConsumerSelfTested"),
				tTJSVariant(affDiag.affineConsumerSelfTested));
			SetTjsProperty(affSem, TJS_W("affineConsumerSelfTestPassed"),
				tTJSVariant(affDiag.affineConsumerSelfTestPassed));
			SetTjsProperty(affSem, TJS_W("affineConsumerSelfTestNonAxisAligned"),
				tTJSVariant(affDiag.affineConsumerSelfTestNonAxisAligned));
			SetTjsProperty(affSem, TJS_W("affineConsumerUploadPrepared"),
				tTJSVariant(affDiag.affineConsumerUploadPrepared));
			SetTjsProperty(affSem, TJS_W("affineConsumerStagingPopulated"),
				tTJSVariant(affDiag.affineConsumerStagingPopulated));
			SetTjsProperty(affSem, TJS_W("affineConsumerShadowConsumed"),
				tTJSVariant(affDiag.affineConsumerShadowConsumed));
			SetTjsProperty(affSem, TJS_W("affineConsumerCommandCount"),
				tTJSVariant(affDiag.affineConsumerCommandCount));
			SetTjsProperty(affSem, TJS_W("affineConsumerVertexCount"),
				tTJSVariant(affDiag.affineConsumerVertexCount));
			SetTjsProperty(affSem, TJS_W("affineConsumerIndexCount"),
				tTJSVariant(affDiag.affineConsumerIndexCount));
			SetTjsProperty(affSem, TJS_W("affineConsumerCellCount"),
				tTJSVariant(affDiag.affineConsumerCellCount));
			SetTjsProperty(affSem, TJS_W("affineConsumerTriangleCount"),
				tTJSVariant(affDiag.affineConsumerTriangleCount));
			SetTjsProperty(affSem, TJS_W("rotationConsumed"),
				tTJSVariant((tjs_int)(affDiag.rotationConsumed ? 1 : 0)));
			SetTjsProperty(affSem, TJS_W("shearConsumed"),
				tTJSVariant((tjs_int)(affDiag.shearConsumed ? 1 : 0)));
			SetTjsProperty(affSem, TJS_W("affinePositionGenerated"),
				tTJSVariant((tjs_int)(affDiag.affinePositionGenerated ? 1 : 0)));
			SetTjsProperty(affSem, TJS_W("sampleAngle"), tTJSVariant(affDiag.sampleAngle));
			SetTjsProperty(affSem, TJS_W("sampleSx"), tTJSVariant(affDiag.sampleSx));
			SetTjsProperty(affSem, TJS_W("sampleSy"), tTJSVariant(affDiag.sampleSy));
			SetTjsProperty(affSem, TJS_W("sampleOx"), tTJSVariant(affDiag.sampleOx));
			SetTjsProperty(affSem, TJS_W("sampleOy"), tTJSVariant(affDiag.sampleOy));
			SetTjsProperty(affSem, TJS_W("sampleIconOriginX"), tTJSVariant(affDiag.sampleIconOriginX));
			SetTjsProperty(affSem, TJS_W("sampleIconOriginY"), tTJSVariant(affDiag.sampleIconOriginY));
			SetTjsProperty(affSem, TJS_W("sampleIconWidth"), tTJSVariant(affDiag.sampleIconWidth));
			SetTjsProperty(affSem, TJS_W("sampleIconHeight"), tTJSVariant(affDiag.sampleIconHeight));
			SetTjsProperty(affSem, TJS_W("sampleQ0x"), tTJSVariant(affDiag.sampleQ0x));
			SetTjsProperty(affSem, TJS_W("sampleQ0y"), tTJSVariant(affDiag.sampleQ0y));
			SetTjsProperty(affSem, TJS_W("sampleQ1x"), tTJSVariant(affDiag.sampleQ1x));
			SetTjsProperty(affSem, TJS_W("sampleQ1y"), tTJSVariant(affDiag.sampleQ1y));
			SetTjsProperty(affSem, TJS_W("sampleQ2x"), tTJSVariant(affDiag.sampleQ2x));
			SetTjsProperty(affSem, TJS_W("sampleQ2y"), tTJSVariant(affDiag.sampleQ2y));
			SetTjsProperty(affSem, TJS_W("sampleQ3x"), tTJSVariant(affDiag.sampleQ3x));
			SetTjsProperty(affSem, TJS_W("sampleQ3y"), tTJSVariant(affDiag.sampleQ3y));
			SetTjsProperty(affSem, TJS_W("sampleAabbLeft"), tTJSVariant(affDiag.sampleAabbLeft));
			SetTjsProperty(affSem, TJS_W("sampleAabbTop"), tTJSVariant(affDiag.sampleAabbTop));
			SetTjsProperty(affSem, TJS_W("sampleAabbWidth"), tTJSVariant(affDiag.sampleAabbWidth));
			SetTjsProperty(affSem, TJS_W("sampleAabbHeight"), tTJSVariant(affDiag.sampleAabbHeight));
			if (!affDiag.consumerSelfTestMismatchKind.empty()) {
				SetTjsProperty(affSem, TJS_W("consumerSelfTestMismatchKind"),
					tTJSVariant(ttstr(affDiag.consumerSelfTestMismatchKind.c_str())));
			}
			if (!affDiag.consumerSelfTestDetails.empty()) {
				SetTjsProperty(affSem, TJS_W("consumerSelfTestDetails"),
					tTJSVariant(ttstr(affDiag.consumerSelfTestDetails.c_str())));
			}
			if (!affDiag.skipReason.empty()) {
				SetTjsProperty(affSem, TJS_W("skipReason"),
					tTJSVariant(ttstr(affDiag.skipReason.c_str())));
			}
			tTJSVariant affSemVal(affSem, affSem);
			affSem->Release();
			SetTjsProperty(dict, TJS_W("motionAffineFrontMesh"), affSemVal);
		}
		{
			MotionResource *resPtr = resource.get();
			if (resPtr) {
				MotionMeshDeformationDiag &defDiag = resPtr->meshDeformationDiag;
				defDiag = MotionMeshDeformationDiag();
				defDiag.probed = 1;
				MotionRunMeshDeformationSelfTest(defDiag);
				std::vector<std::pair<const MotionLayoutInfo*, tjs_int> > frontItems =
					MotionCollectFrontLayouts(resPtr, false);
				for (size_t fi = 0; fi < frontItems.size(); fi++) {
					const MotionLayoutInfo *layout = frontItems[fi].first;
					if (!layout) continue;
					if (layout->renderMethodItemIndex >= 0 &&
						layout->renderMethodItemIndex < (tjs_int)resPtr->renderMethodIR.size() &&
						resPtr->renderMethodIR[(size_t)layout->renderMethodItemIndex].type == 1) {
						defDiag.type1ItemCount++;
					}
					if (layout->hasControlPoints) defDiag.bpFrameCount++;
				}
				defDiag.type1CommandCount = resPtr->motionRenderMethodSemantics.consumedType1CommandCount;
				if (defDiag.type1ItemCount == 0 && defDiag.bpFrameCount == 0) {
					defDiag.skipReason = "no-type1-or-bp-in-front-layouts";
				}
			}
			const MotionResource *resConstPtr = resource.get();
			const MotionMeshDeformationDiag &defDiag =
				resConstPtr ? resConstPtr->meshDeformationDiag : MotionMeshDeformationDiag();
			iTJSDispatch2 *defSem = TJSCreateDictionaryObject();
			SetTjsProperty(defSem, TJS_W("probed"), tTJSVariant(defDiag.probed));
			SetTjsProperty(defSem, TJS_W("type1ItemCount"), tTJSVariant(defDiag.type1ItemCount));
			SetTjsProperty(defSem, TJS_W("type1CommandCount"), tTJSVariant(defDiag.type1CommandCount));
			SetTjsProperty(defSem, TJS_W("bpFrameCount"), tTJSVariant(defDiag.bpFrameCount));
			SetTjsProperty(defSem, TJS_W("meshDeformSelfTested"), tTJSVariant(defDiag.meshDeformSelfTested));
			SetTjsProperty(defSem, TJS_W("meshDeformSelfTestPassed"), tTJSVariant(defDiag.meshDeformSelfTestPassed));
			SetTjsProperty(defSem, TJS_W("meshDeformConsumerSelfTested"), tTJSVariant(defDiag.meshDeformConsumerSelfTested));
			SetTjsProperty(defSem, TJS_W("meshDeformConsumerSelfTestPassed"), tTJSVariant(defDiag.meshDeformConsumerSelfTestPassed));
			SetTjsProperty(defSem, TJS_W("meshDeformConsumerCommandCount"), tTJSVariant(defDiag.meshDeformConsumerCommandCount));
			SetTjsProperty(defSem, TJS_W("meshDeformConsumerVertexCount"), tTJSVariant(defDiag.meshDeformConsumerVertexCount));
			SetTjsProperty(defSem, TJS_W("meshDeformConsumerIndexCount"), tTJSVariant(defDiag.meshDeformConsumerIndexCount));
			SetTjsProperty(defSem, TJS_W("meshDeformConsumerCellCount"), tTJSVariant(defDiag.meshDeformConsumerCellCount));
			SetTjsProperty(defSem, TJS_W("meshDeformConsumerTriangleCount"), tTJSVariant(defDiag.meshDeformConsumerTriangleCount));
			SetTjsProperty(defSem, TJS_W("meshDeformConsumerUploadPrepared"), tTJSVariant(defDiag.meshDeformConsumerUploadPrepared));
			SetTjsProperty(defSem, TJS_W("meshDeformConsumerStagingPopulated"), tTJSVariant(defDiag.meshDeformConsumerStagingPopulated));
			SetTjsProperty(defSem, TJS_W("meshDeformConsumerShadowConsumed"), tTJSVariant(defDiag.meshDeformConsumerShadowConsumed));
			SetTjsProperty(defSem, TJS_W("meshDeformInteriorDeviated"), tTJSVariant(defDiag.meshDeformInteriorDeviated));
			SetTjsProperty(defSem, TJS_W("sampleInteriorAffineX"), tTJSVariant(defDiag.sampleInteriorAffineX));
			SetTjsProperty(defSem, TJS_W("sampleInteriorAffineY"), tTJSVariant(defDiag.sampleInteriorAffineY));
			SetTjsProperty(defSem, TJS_W("sampleInteriorDeformedX"), tTJSVariant(defDiag.sampleInteriorDeformedX));
			SetTjsProperty(defSem, TJS_W("sampleInteriorDeformedY"), tTJSVariant(defDiag.sampleInteriorDeformedY));
			SetTjsProperty(defSem, TJS_W("sampleInteriorDeviationDist"), tTJSVariant(defDiag.sampleInteriorDeviationDist));
			SetTjsProperty(defSem, TJS_W("sampleBp0"), tTJSVariant(defDiag.sampleBp0));
			SetTjsProperty(defSem, TJS_W("sampleBp1"), tTJSVariant(defDiag.sampleBp1));
			SetTjsProperty(defSem, TJS_W("sampleBp30"), tTJSVariant(defDiag.sampleBp30));
			SetTjsProperty(defSem, TJS_W("sampleBp31"), tTJSVariant(defDiag.sampleBp31));
			if (!defDiag.consumerSelfTestMismatchKind.empty()) {
				SetTjsProperty(defSem, TJS_W("consumerSelfTestMismatchKind"),
					tTJSVariant(ttstr(defDiag.consumerSelfTestMismatchKind.c_str())));
			}
			if (!defDiag.consumerSelfTestDetails.empty()) {
				SetTjsProperty(defSem, TJS_W("consumerSelfTestDetails"),
					tTJSVariant(ttstr(defDiag.consumerSelfTestDetails.c_str())));
			}
			if (!defDiag.skipReason.empty()) {
				SetTjsProperty(defSem, TJS_W("skipReason"),
					tTJSVariant(ttstr(defDiag.skipReason.c_str())));
			}
			tTJSVariant defSemVal(defSem, defSem);
			defSem->Release();
			SetTjsProperty(dict, TJS_W("motionMeshDeformation"), defSemVal);
		}
		SetTjsProperty(dict, TJS_W("rendererPrep"), prep);
		SetTjsProperty(dict, TJS_W("frontDrawItems"),
			MakeMotionFrontDrawItemDumpArray(resource.get(), meshDivisionRatio));
		SetTjsProperty(dict, TJS_W("frontObjectStates"),
			MakeMotionFrontObjectStateDumpArray(resource.get(), false));
		SetTjsProperty(dict, TJS_W("frontMeshCells"),
			MakeMotionFrontMeshCellDumpArray(resource.get(), meshDivisionRatio));
		SetTjsProperty(dict, TJS_W("frontHitShapes"),
			MakeMotionFrontHitShapeDumpArray(resource.get()));
		SetTjsProperty(dict, TJS_W("frontHitObjectStates"),
			MakeMotionFrontObjectStateDumpArray(resource.get(), true));
		{
			std::vector<std::pair<const MotionLayoutInfo*, tjs_int> > drawItemsForOrder =
				MotionCollectFrontLayouts(resource.get(), false);
			iTJSDispatch2 *drawOrderArray = TJSCreateArrayObject();
			for (size_t di = 0; di < drawItemsForOrder.size(); di++) {
				tTJSVariant val((tjs_int)di);
				drawOrderArray->PropSetByNum(TJS_MEMBERENSURE, (tjs_int)di, &val, drawOrderArray);
			}
			tTJSVariant drawOrderVal(drawOrderArray, drawOrderArray);
			drawOrderArray->Release();
			SetTjsProperty(dict, TJS_W("drawOrder"), drawOrderVal);
		}
		{
			std::vector<MotionRendererPlanEntryInfo> planEntries =
				MotionBuildRendererPlanEntries(resource.get(), meshDivisionRatio);
			tjs_int planVertexTotal = MotionRendererPlanVertexTotal(planEntries);
				tjs_int planIndexTotal = MotionRendererPlanIndexTotal(planEntries);
				tjs_int planQuadTotal = MotionRendererPlanQuadTotal(planEntries);
				ttstr rendererPlanHash =
					MotionPlanHashText(MotionHashRendererPlanEntries(planEntries));
				tjs_uint64 cmdHash = rendererCommandHash;
				tjs_uint64 cmdSharedHash =
					MotionHashRendererCommandSharedFields(rendererCommandEntries);
				tjs_int cmdVertexTotal =
					MotionRendererCommandVertexTotal(rendererCommandEntries);
				tjs_int cmdIndexTotal =
					MotionRendererCommandIndexTotal(rendererCommandEntries);
				tjs_int cmdCellTotal =
					MotionRendererCommandQuadTotal(rendererCommandEntries);
				tjs_int cmdCount = (tjs_int)rendererCommandEntries.size();
				MotionRendererCommandPlanMatchSummary cmdPlanMatch =
					MotionCompareRendererCommandsToPlan(
						rendererCommandEntries, planEntries,
						rendererCommandBuilt);
				bool runtimeCommandContractOk =
					rendererCommandBuilt &&
					drawTimeConsumptionValid &&
					rendererCommandOffsetsContiguous &&
					rendererCommandPerEntryOk &&
					drawTimeConsumedDrawItemCount == cmdCount &&
					drawTimeConsumedVertexCount == cmdVertexTotal &&
					drawTimeConsumedIndexCount == cmdIndexTotal &&
					drawTimeConsumedCellCount == cmdCellTotal &&
					cmdCount > 0 &&
					cmdSharedHash != 0;
				iTJSDispatch2 *rendererPlan = TJSCreateArrayObject();
				for (size_t pi = 0; pi < planEntries.size(); pi++) {
				const MotionRendererPlanEntryInfo &entry = planEntries[pi];
				iTJSDispatch2 *planEntry = TJSCreateDictionaryObject();
				SetTjsProperty(planEntry, TJS_W("drawItemIndex"), tTJSVariant(entry.drawItemIndex));
				SetTjsProperty(planEntry, TJS_W("drawOrder"), tTJSVariant(entry.drawOrder));
				SetTjsProperty(planEntry, TJS_W("sourceIndex"), tTJSVariant(entry.sourceIndex));
				SetTjsProperty(planEntry, TJS_W("renderMethodItemIndex"), tTJSVariant(entry.renderMethodItemIndex));
				SetTjsProperty(planEntry, TJS_W("vertexOffset"), tTJSVariant(entry.vertexOffset));
				SetTjsProperty(planEntry, TJS_W("vertexCount"), tTJSVariant(entry.vertexCount));
				SetTjsProperty(planEntry, TJS_W("indexOffset"), tTJSVariant(entry.indexOffset));
				SetTjsProperty(planEntry, TJS_W("indexCount"), tTJSVariant(entry.indexCount));
				SetTjsProperty(planEntry, TJS_W("firstCellIndex"), tTJSVariant(entry.firstCellIndex));
				SetTjsProperty(planEntry, TJS_W("cellCount"), tTJSVariant(entry.cellCount));
				SetTjsProperty(planEntry, TJS_W("quadCount"), tTJSVariant(entry.quadCount));
				const MotionLayoutInfo *planLayout = entry.layout;
				if (planLayout) {
					SetTjsProperty(planEntry, TJS_W("name"), tTJSVariant(planLayout->name));
					SetTjsProperty(planEntry, TJS_W("hitObjectName"), tTJSVariant(planLayout->hitObjectName));
					SetTjsProperty(planEntry, TJS_W("hitObjectPath"), tTJSVariant(planLayout->hitObjectPath));
					SetTjsProperty(planEntry, TJS_W("hitStateClass"), tTJSVariant(planLayout->hitStateClass));
					SetTjsProperty(planEntry, TJS_W("hitRole"), tTJSVariant(planLayout->hitRole));
					SetTjsProperty(planEntry, TJS_W("alphaMax"), tTJSVariant(MotionAlphaFromOpacity(planLayout->opacityMax)));
				}
				SetTjsProperty(planEntry, TJS_W("consumedVertexSlice"), tTJSVariant(entry.vertexSlice));
				SetTjsProperty(planEntry, TJS_W("consumedIndexSlice"), tTJSVariant(entry.indexSlice));
				SetTjsProperty(planEntry, TJS_W("consumedFirstCellLocalIndices"),
					tTJSVariant(entry.firstCellLocalIndices));
				SetTjsProperty(planEntry, TJS_W("planSampleHash"), tTJSVariant(entry.sampleHash));
				tTJSVariant entryVal(planEntry, planEntry);
				planEntry->Release();
				rendererPlan->PropSetByNum(TJS_MEMBERENSURE, (tjs_int)pi, &entryVal, rendererPlan);
			}
			tTJSVariant rendererPlanVal(rendererPlan, rendererPlan);
			rendererPlan->Release();
			SetTjsProperty(dict, TJS_W("rendererPlan"), rendererPlanVal);
			SetTjsProperty(dict, TJS_W("rendererPlanVertexCount"), tTJSVariant(planVertexTotal));
			SetTjsProperty(dict, TJS_W("rendererPlanIndexCount"), tTJSVariant(planIndexTotal));
			SetTjsProperty(dict, TJS_W("rendererPlanDrawPassCount"), tTJSVariant((tjs_int)planEntries.size()));
			SetTjsProperty(dict, TJS_W("drawTimeConsumptionValid"), tTJSVariant((tjs_int)(drawTimeConsumptionValid ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("drawTimeConsumedDrawItemCount"), tTJSVariant(drawTimeConsumedDrawItemCount));
				SetTjsProperty(dict, TJS_W("drawTimeConsumedVertexCount"), tTJSVariant(drawTimeConsumedVertexCount));
				SetTjsProperty(dict, TJS_W("drawTimeConsumedIndexCount"), tTJSVariant(drawTimeConsumedIndexCount));
				SetTjsProperty(dict, TJS_W("drawTimeConsumedCellCount"), tTJSVariant(drawTimeConsumedCellCount));
				SetTjsProperty(dict, TJS_W("drawTimePlanSourceCount"), tTJSVariant((tjs_int)planEntries.size()));
				SetTjsProperty(dict, TJS_W("drawTimeRendererPlanCount"), tTJSVariant((tjs_int)planEntries.size()));
				SetTjsProperty(dict, TJS_W("drawTimePlanVertexCount"), tTJSVariant(planVertexTotal));
				SetTjsProperty(dict, TJS_W("drawTimePlanIndexCount"), tTJSVariant(planIndexTotal));
				SetTjsProperty(dict, TJS_W("drawTimePlanCellCount"), tTJSVariant(planQuadTotal));
				SetTjsProperty(dict, TJS_W("drawTimePlanDrawPassCount"), tTJSVariant((tjs_int)planEntries.size()));
				SetTjsProperty(dict, TJS_W("drawTimePlanHash"), tTJSVariant(rendererPlanHash));
				SetTjsProperty(dict, TJS_W("drawTimeRendererPlanHash"), tTJSVariant(rendererPlanHash));
				SetTjsProperty(dict, TJS_W("drawTimeRuntimeSourceCount"), tTJSVariant(cmdCount));
				SetTjsProperty(dict, TJS_W("drawTimeRuntimeVertexCount"), tTJSVariant(cmdVertexTotal));
				SetTjsProperty(dict, TJS_W("drawTimeRuntimeIndexCount"), tTJSVariant(cmdIndexTotal));
				SetTjsProperty(dict, TJS_W("drawTimeRuntimeCellCount"), tTJSVariant(cmdCellTotal));
				SetTjsProperty(dict, TJS_W("drawTimeRuntimeDrawPassCount"), tTJSVariant(cmdCount));
				SetTjsProperty(dict, TJS_W("drawTimeRuntimeHash"), tTJSVariant(MotionPlanHashText(cmdSharedHash)));
				SetTjsProperty(dict, TJS_W("drawTimeRuntimeContractOk"), tTJSVariant((tjs_int)(runtimeCommandContractOk ? 1 : 0)));
				SetTjsProperty(dict, TJS_W("drawRuntimeResourceBuilt"), tTJSVariant((tjs_int)(drawRuntimeResourceBuilt ? 1 : 0)));
				SetTjsProperty(dict, TJS_W("drawRuntimeSource"), tTJSVariant(drawRuntimeSource));
			SetTjsProperty(dict, TJS_W("drawRuntimeLayoutCount"), tTJSVariant(drawRuntimeLayoutCount));
			SetTjsProperty(dict, TJS_W("drawRuntimeFrontDrawItemCount"), tTJSVariant(drawRuntimeFrontDrawItemCount));
			SetTjsProperty(dict, TJS_W("drawRuntimeFrontHitShapeCount"), tTJSVariant(drawRuntimeFrontHitShapeCount));
				SetTjsProperty(dict, TJS_W("drawRuntimeRenderMethodItemCount"), tTJSVariant(drawRuntimeRenderMethodItemCount));
				SetTjsProperty(dict, TJS_W("drawRuntimeObject"), tTJSVariant(drawRuntimeObject));
				SetTjsProperty(dict, TJS_W("drawRuntimeMotion"), tTJSVariant(drawRuntimeMotion));
				SetTjsProperty(dict, TJS_W("drawRuntimeTick"), tTJSVariant(drawRuntimeTick));
				SetTjsProperty(dict, TJS_W("drawRuntimeLimitOriginX"), tTJSVariant(drawRuntimeLimitOriginX));
				SetTjsProperty(dict, TJS_W("drawRuntimeLimitOriginY"), tTJSVariant(drawRuntimeLimitOriginY));
				SetTjsProperty(dict, TJS_W("drawRuntimeLimitWidth"), tTJSVariant(drawRuntimeLimitWidth));
				SetTjsProperty(dict, TJS_W("drawRuntimeLimitHeight"), tTJSVariant(drawRuntimeLimitHeight));
				SetTjsProperty(dict, TJS_W("drawRuntimeLimitZMax"), tTJSVariant(drawRuntimeLimitZMax));
				if (drawRuntimeDiagnosticResourceValid) {
					SetTjsProperty(dict, TJS_W("drawRuntimeRendererPrep"),
						MakeMotionRendererPrepDump(&drawRuntimeDiagnosticResource,
							meshDivisionRatio, true));
					SetTjsProperty(dict, TJS_W("drawRuntimeFrontDrawItems"),
						MakeMotionFrontDrawItemDumpArray(
							&drawRuntimeDiagnosticResource, meshDivisionRatio));
					SetTjsProperty(dict, TJS_W("drawRuntimeFrontObjectStates"),
						MakeMotionFrontObjectStateDumpArray(
							&drawRuntimeDiagnosticResource, false));
				}
				SetTjsProperty(dict, TJS_W("drawTimeVertexCountMatchesPlan"), tTJSVariant((tjs_int)(drawTimeConsumedVertexCount == planVertexTotal ? 1 : 0)));
				SetTjsProperty(dict, TJS_W("drawTimeIndexCountMatchesPlan"), tTJSVariant((tjs_int)(drawTimeConsumedIndexCount == planIndexTotal ? 1 : 0)));
				SetTjsProperty(dict, TJS_W("drawTimeCellCountMatchesPlan"), tTJSVariant((tjs_int)(drawTimeConsumedCellCount == planQuadTotal ? 1 : 0)));
				SetTjsProperty(dict, TJS_W("drawTimeDrawPassMatchesPlan"), tTJSVariant((tjs_int)(drawTimeConsumedDrawItemCount == (tjs_int)planEntries.size() ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("drawTimeOffsetContiguousOk"), tTJSVariant((tjs_int)(drawTimeOffsetContiguousOk ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("drawTimePerItemCountOk"), tTJSVariant((tjs_int)(drawTimePerItemCountOk ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("drawTimePerItemOrderOk"), tTJSVariant((tjs_int)(drawTimePerItemOrderOk ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("drawTimePerItemSliceOk"), tTJSVariant((tjs_int)(drawTimePerItemSliceOk ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("drawTimeFirstItemOrder"), tTJSVariant(drawTimeFirstItemOrder));
			SetTjsProperty(dict, TJS_W("drawTimeFirstItemHash"), tTJSVariant(drawTimeFirstItemHash));
			SetTjsProperty(dict, TJS_W("drawTimeFirstItemVertexOffset"), tTJSVariant(drawTimeFirstItemVertexOffset));
			SetTjsProperty(dict, TJS_W("drawTimeFirstItemIndexOffset"), tTJSVariant(drawTimeFirstItemIndexOffset));
			SetTjsProperty(dict, TJS_W("drawTimeFirstItemVertexCount"), tTJSVariant(drawTimeFirstItemVertexCount));
			SetTjsProperty(dict, TJS_W("drawTimeFirstItemIndexCount"), tTJSVariant(drawTimeFirstItemIndexCount));
			SetTjsProperty(dict, TJS_W("drawTimeFirstItemCellCount"), tTJSVariant(drawTimeFirstItemCellCount));
			SetTjsProperty(dict, TJS_W("drawTimeFirstItemFirstCellIndex"), tTJSVariant(drawTimeFirstItemFirstCellIndex));
			SetTjsProperty(dict, TJS_W("drawTimeLastItemOrder"), tTJSVariant(drawTimeLastItemOrder));
			SetTjsProperty(dict, TJS_W("drawTimeLastItemHash"), tTJSVariant(drawTimeLastItemHash));
			SetTjsProperty(dict, TJS_W("drawTimeLastItemVertexOffset"), tTJSVariant(drawTimeLastItemVertexOffset));
			SetTjsProperty(dict, TJS_W("drawTimeLastItemIndexOffset"), tTJSVariant(drawTimeLastItemIndexOffset));
			SetTjsProperty(dict, TJS_W("drawTimeLastItemVertexCount"), tTJSVariant(drawTimeLastItemVertexCount));
			SetTjsProperty(dict, TJS_W("drawTimeLastItemIndexCount"), tTJSVariant(drawTimeLastItemIndexCount));
			SetTjsProperty(dict, TJS_W("drawTimeLastItemCellCount"), tTJSVariant(drawTimeLastItemCellCount));
			SetTjsProperty(dict, TJS_W("drawTimeLastItemFirstCellIndex"), tTJSVariant(drawTimeLastItemFirstCellIndex));
			{
				bool firstOk =
					drawTimeConsumedDrawItemCount > 0 &&
					drawTimeFirstItemVertexOffset == 0 &&
					drawTimeFirstItemIndexOffset == 0 &&
					drawTimeFirstItemFirstCellIndex == 0 &&
					drawTimeFirstItemVertexCount > 0 &&
					drawTimeFirstItemIndexCount > 0 &&
					drawTimeFirstItemCellCount > 0 &&
					drawTimeFirstItemOrder == 0;
					bool lastOk =
						drawTimeConsumedDrawItemCount > 0 &&
						drawTimeLastItemVertexOffset >= drawTimeFirstItemVertexOffset &&
					drawTimeLastItemIndexOffset >= drawTimeFirstItemIndexOffset &&
					drawTimeLastItemFirstCellIndex >= drawTimeFirstItemFirstCellIndex &&
					drawTimeLastItemVertexCount > 0 &&
					drawTimeLastItemIndexCount > 0 &&
						drawTimeLastItemCellCount > 0 &&
						drawTimeLastItemOrder == drawTimeConsumedDrawItemCount - 1;
					bool firstPlanOk = false;
					bool lastPlanOk = false;
					if (!rendererCommandEntries.empty() && !planEntries.empty()) {
						ttstr unusedField;
						ttstr unusedDetails;
						firstPlanOk = MotionCompareRendererCommandEntryToPlan(
							rendererCommandEntries.front(), planEntries.front(),
							unusedField, unusedDetails);
						lastPlanOk = MotionCompareRendererCommandEntryToPlan(
							rendererCommandEntries.back(), planEntries.back(),
							unusedField, unusedDetails);
					}
					SetTjsProperty(dict, TJS_W("drawTimeFirstRuntimeItemOk"), tTJSVariant((tjs_int)(firstOk ? 1 : 0)));
					SetTjsProperty(dict, TJS_W("drawTimeLastRuntimeItemOk"), tTJSVariant((tjs_int)(lastOk ? 1 : 0)));
					SetTjsProperty(dict, TJS_W("drawTimeFirstItemPlanOk"), tTJSVariant((tjs_int)(firstPlanOk ? 1 : 0)));
					SetTjsProperty(dict, TJS_W("drawTimeLastItemPlanOk"), tTJSVariant((tjs_int)(lastPlanOk ? 1 : 0)));
					bool contractOk = firstPlanOk && lastPlanOk &&
						cmdPlanMatch.matches &&
						runtimeCommandContractOk &&
						drawTimeOffsetContiguousOk &&
						drawTimePerItemCountOk && drawTimePerItemOrderOk &&
						drawTimePerItemSliceOk;
					SetTjsProperty(dict, TJS_W("drawTimeContractMatchesPlan"), tTJSVariant((tjs_int)(contractOk ? 1 : 0)));
					// ── Renderer Command Buffer (draw-time consumption path) ──
					// This is the actual draw command buffer built by draw(), not a
					// getRendererFrontState() plan rebuild.  A future WebGL/CPU renderer
					// will iterate these entries as its draw command stream.
					bool cmdMatchPlan = cmdPlanMatch.matches;
					ttstr mismatchKind = cmdPlanMatch.mismatchKind;
					tjs_int mismatchIdx = cmdPlanMatch.mismatchIndex;
					ttstr mismatchDetails = cmdPlanMatch.mismatchDetails;
					// Legacy commandBuffer* fields for backward compat with existing web diag
					SetTjsProperty(dict, TJS_W("commandBufferBuilt"), tTJSVariant((tjs_int)(rendererCommandBuilt ? 1 : 0)));
					SetTjsProperty(dict, TJS_W("commandBufferCount"), tTJSVariant(cmdCount));
					SetTjsProperty(dict, TJS_W("commandBufferHash"), tTJSVariant(MotionPlanHashText(cmdSharedHash)));
					SetTjsProperty(dict, TJS_W("commandBufferOffsetsContiguous"), tTJSVariant((tjs_int)(rendererCommandOffsetsContiguous ? 1 : 0)));
					SetTjsProperty(dict, TJS_W("commandBufferPerEntryOk"), tTJSVariant((tjs_int)(rendererCommandPerEntryOk ? 1 : 0)));
					SetTjsProperty(dict, TJS_W("commandBufferRuntimeContractOk"), tTJSVariant((tjs_int)(runtimeCommandContractOk ? 1 : 0)));
					SetTjsProperty(dict, TJS_W("commandBufferMatchesRendererPlan"), tTJSVariant((tjs_int)(cmdMatchPlan ? 1 : 0)));
				// Renderer command buffer aggregate + per-entry comparison
				SetTjsProperty(dict, TJS_W("rendererCommandBuilt"), tTJSVariant((tjs_int)(rendererCommandBuilt ? 1 : 0)));
				SetTjsProperty(dict, TJS_W("rendererCommandCount"), tTJSVariant(cmdCount));
				SetTjsProperty(dict, TJS_W("rendererCommandHash"), tTJSVariant(MotionPlanHashText(cmdHash)));
				SetTjsProperty(dict, TJS_W("rendererCommandSharedHash"), tTJSVariant(MotionPlanHashText(cmdSharedHash)));
					SetTjsProperty(dict, TJS_W("rendererCommandVertexTotal"), tTJSVariant(cmdVertexTotal));
					SetTjsProperty(dict, TJS_W("rendererCommandIndexTotal"), tTJSVariant(cmdIndexTotal));
					SetTjsProperty(dict, TJS_W("rendererCommandCellTotal"), tTJSVariant(cmdCellTotal));
						SetTjsProperty(dict, TJS_W("rendererCommandRuntimeContractOk"), tTJSVariant((tjs_int)(runtimeCommandContractOk ? 1 : 0)));
						SetTjsProperty(dict, TJS_W("rendererCommandMatchesRendererPlan"), tTJSVariant((tjs_int)(cmdMatchPlan ? 1 : 0)));
					SetTjsProperty(dict, TJS_W("rendererCommandMismatchKind"), tTJSVariant(mismatchKind.IsEmpty() ? TJS_W("none") : mismatchKind));
					SetTjsProperty(dict, TJS_W("rendererCommandMismatchIndex"), tTJSVariant(mismatchIdx));
					SetTjsProperty(dict, TJS_W("rendererCommandMismatchDetails"), tTJSVariant(mismatchDetails));
					SetTjsProperty(dict, TJS_W("rendererCommandPlanCompared"), tTJSVariant((tjs_int)(cmdPlanMatch.compared ? 1 : 0)));
					SetTjsProperty(dict, TJS_W("rendererCommandPlanCount"), tTJSVariant(cmdPlanMatch.planCount));
					SetTjsProperty(dict, TJS_W("rendererCommandEntryComparedCount"), tTJSVariant(cmdPlanMatch.comparedCount));
					SetTjsProperty(dict, TJS_W("rendererCommandEntryMatchCount"), tTJSVariant(cmdPlanMatch.entryMatchCount));
					SetTjsProperty(dict, TJS_W("rendererCommandFirstMismatchField"), tTJSVariant(cmdPlanMatch.mismatchField.IsEmpty() ? TJS_W("none") : cmdPlanMatch.mismatchField));
					SetTjsProperty(dict, TJS_W("rendererCommandPlanHash"), tTJSVariant(MotionPlanHashText(cmdPlanMatch.planHash)));
					// Export full renderer command entries array (iterable by web)
					{
					iTJSDispatch2 *cmdArray = TJSCreateArrayObject();
					const MotionResource *cmdResource =
						drawRuntimeDiagnosticResourceValid ?
						&drawRuntimeDiagnosticResource : resource.get();
					for (size_t ci = 0; ci < rendererCommandEntries.size(); ci++) {
						const MotionRendererCommandEntryInfo &cmdE = rendererCommandEntries[ci];
						const MotionRenderMethodItem *cmdItem = nullptr;
						const MotionLayoutInfo *cmdLayout = nullptr;
						if (cmdResource && cmdE.renderMethodItemIndex >= 0 &&
							cmdE.renderMethodItemIndex < (tjs_int)cmdResource->renderMethodIR.size()) {
							cmdItem = &cmdResource->renderMethodIR[(size_t)cmdE.renderMethodItemIndex];
						}
						if (cmdResource && cmdE.sourceIndex >= 0 &&
							cmdE.sourceIndex < (tjs_int)cmdResource->layouts.size()) {
							cmdLayout = &cmdResource->layouts[(size_t)cmdE.sourceIndex];
						}
						iTJSDispatch2 *cmdEntry = TJSCreateDictionaryObject();
						SetTjsProperty(cmdEntry, TJS_W("drawItemIndex"), tTJSVariant(cmdE.drawItemIndex));
						SetTjsProperty(cmdEntry, TJS_W("drawOrder"), tTJSVariant(cmdE.drawOrder));
						SetTjsProperty(cmdEntry, TJS_W("sourceIndex"), tTJSVariant(cmdE.sourceIndex));
						SetTjsProperty(cmdEntry, TJS_W("renderMethodItemIndex"), tTJSVariant(cmdE.renderMethodItemIndex));
						SetTjsProperty(cmdEntry, TJS_W("vertexOffset"), tTJSVariant(cmdE.vertexOffset));
						SetTjsProperty(cmdEntry, TJS_W("vertexCount"), tTJSVariant(cmdE.vertexCount));
						SetTjsProperty(cmdEntry, TJS_W("indexOffset"), tTJSVariant(cmdE.indexOffset));
						SetTjsProperty(cmdEntry, TJS_W("indexCount"), tTJSVariant(cmdE.indexCount));
						SetTjsProperty(cmdEntry, TJS_W("firstCellIndex"), tTJSVariant(cmdE.firstCellIndex));
						SetTjsProperty(cmdEntry, TJS_W("cellCount"), tTJSVariant(cmdE.cellCount));
						SetTjsProperty(cmdEntry, TJS_W("quadCount"), tTJSVariant(cmdE.quadCount));
						SetTjsProperty(cmdEntry, TJS_W("consumedVertexSlice"), tTJSVariant(cmdE.vertexSlice));
						SetTjsProperty(cmdEntry, TJS_W("consumedIndexSlice"), tTJSVariant(cmdE.indexSlice));
						SetTjsProperty(cmdEntry, TJS_W("consumedFirstCellLocalIndices"), tTJSVariant(cmdE.firstCellLocalIndices));
						SetTjsProperty(cmdEntry, TJS_W("sampleHash"), tTJSVariant(cmdE.sampleHash));
						SetTjsProperty(cmdEntry, TJS_W("consumedAtIndex"), tTJSVariant(cmdE.consumedAtIndex));
						if (cmdItem) {
							SetTjsProperty(cmdEntry, TJS_W("label"), tTJSVariant(cmdItem->label));
							SetTjsProperty(cmdEntry, TJS_W("sourceKey"), tTJSVariant(MotionString(cmdItem->srcKey)));
							SetTjsProperty(cmdEntry, TJS_W("type"), tTJSVariant(cmdItem->type));
							SetTjsProperty(cmdEntry, TJS_W("surfaceCount"), tTJSVariant((tjs_int)cmdItem->surfaces.size()));
							SetTjsProperty(cmdEntry, TJS_W("isFromLayout"), tTJSVariant((tjs_int)(cmdItem->isFromLayout ? 1 : 0)));
							SetTjsProperty(cmdEntry, TJS_W("isFromNestedMotion"), tTJSVariant((tjs_int)(cmdItem->isFromNestedMotion ? 1 : 0)));
							SetTjsProperty(cmdEntry, TJS_W("originX"), tTJSVariant(cmdItem->originX));
							SetTjsProperty(cmdEntry, TJS_W("originY"), tTJSVariant(cmdItem->originY));
							SetTjsProperty(cmdEntry, TJS_W("width"), tTJSVariant(cmdItem->width));
							SetTjsProperty(cmdEntry, TJS_W("height"), tTJSVariant(cmdItem->height));
							SetTjsProperty(cmdEntry, TJS_W("opacity"), tTJSVariant(cmdItem->opacity));
							SetTjsProperty(cmdEntry, TJS_W("surfaces"),
								MakeMotionRenderSurfaceDumpArray(cmdItem->surfaces));
							if (!cmdItem->surfaces.empty()) {
								const MotionRenderSurface &surface = cmdItem->surfaces.front();
								SetTjsProperty(cmdEntry, TJS_W("surfaceFirstType"), tTJSVariant(surface.type));
								SetTjsProperty(cmdEntry, TJS_W("surfaceFirstOpacity"), tTJSVariant(surface.opa));
								SetTjsProperty(cmdEntry, TJS_W("surfaceFirstOriginX"), tTJSVariant(surface.originX));
								SetTjsProperty(cmdEntry, TJS_W("surfaceFirstOriginY"), tTJSVariant(surface.originY));
								SetTjsProperty(cmdEntry, TJS_W("surfaceFirstWidth"), tTJSVariant(surface.originW));
								SetTjsProperty(cmdEntry, TJS_W("surfaceFirstHeight"), tTJSVariant(surface.originH));
								SetTjsProperty(cmdEntry, TJS_W("surfaceFirstM00"), tTJSVariant(surface.matTrans.m[0]));
								SetTjsProperty(cmdEntry, TJS_W("surfaceFirstM01"), tTJSVariant(surface.matTrans.m[1]));
								SetTjsProperty(cmdEntry, TJS_W("surfaceFirstM03"), tTJSVariant(surface.matTrans.m[3]));
								SetTjsProperty(cmdEntry, TJS_W("surfaceFirstM10"), tTJSVariant(surface.matTrans.m[4]));
								SetTjsProperty(cmdEntry, TJS_W("surfaceFirstM11"), tTJSVariant(surface.matTrans.m[5]));
								SetTjsProperty(cmdEntry, TJS_W("surfaceFirstM13"), tTJSVariant(surface.matTrans.m[7]));
								SetTjsProperty(cmdEntry, TJS_W("surfaceFirstM22"), tTJSVariant(surface.matTrans.m[10]));
							}
						} else if (cmdLayout) {
							SetTjsProperty(cmdEntry, TJS_W("label"), tTJSVariant(cmdLayout->name));
							SetTjsProperty(cmdEntry, TJS_W("sourceKey"), tTJSVariant(cmdLayout->sourceKey));
							SetTjsProperty(cmdEntry, TJS_W("type"), tTJSVariant((tjs_int)2));
							SetTjsProperty(cmdEntry, TJS_W("surfaceCount"), tTJSVariant((tjs_int)0));
							SetTjsProperty(cmdEntry, TJS_W("isFromLayout"), tTJSVariant((tjs_int)1));
							SetTjsProperty(cmdEntry, TJS_W("isFromNestedMotion"), tTJSVariant((tjs_int)0));
							SetTjsProperty(cmdEntry, TJS_W("originX"), tTJSVariant(cmdLayout->iconOriginX));
							SetTjsProperty(cmdEntry, TJS_W("originY"), tTJSVariant(cmdLayout->iconOriginY));
							SetTjsProperty(cmdEntry, TJS_W("width"), tTJSVariant(cmdLayout->width));
							SetTjsProperty(cmdEntry, TJS_W("height"), tTJSVariant(cmdLayout->height));
							SetTjsProperty(cmdEntry, TJS_W("opacity"), tTJSVariant(cmdLayout->opacityMax));
							SetTjsProperty(cmdEntry, TJS_W("hitObjectName"), tTJSVariant(cmdLayout->hitObjectName));
							SetTjsProperty(cmdEntry, TJS_W("hitStateClass"), tTJSVariant(cmdLayout->hitStateClass));
							SetTjsProperty(cmdEntry, TJS_W("hitObjectPath"), tTJSVariant(cmdLayout->hitObjectPath));
							SetTjsProperty(cmdEntry, TJS_W("hitRole"), tTJSVariant(cmdLayout->hitRole));
						}
						tTJSVariant entryVal(cmdEntry, cmdEntry);
						cmdEntry->Release();
						cmdArray->PropSetByNum(TJS_MEMBERENSURE, (tjs_int)ci, &entryVal, cmdArray);
					}
					tTJSVariant cmdArrayVal(cmdArray, cmdArray);
					cmdArray->Release();
					SetTjsProperty(dict, TJS_W("rendererCommandEntries"), cmdArrayVal);
				}
				// Legacy first/last backward compat exports
				if (!rendererCommandEntries.empty()) {
					const MotionRendererCommandEntryInfo &cmdFirst = rendererCommandEntries.front();
					SetTjsProperty(dict, TJS_W("commandBufferFirstDrawItemIndex"), tTJSVariant(cmdFirst.drawItemIndex));
					SetTjsProperty(dict, TJS_W("commandBufferFirstSourceIndex"), tTJSVariant(cmdFirst.sourceIndex));
					SetTjsProperty(dict, TJS_W("commandBufferFirstVertexOffset"), tTJSVariant(cmdFirst.vertexOffset));
					SetTjsProperty(dict, TJS_W("commandBufferFirstIndexOffset"), tTJSVariant(cmdFirst.indexOffset));
					SetTjsProperty(dict, TJS_W("commandBufferFirstFirstCellIndex"), tTJSVariant(cmdFirst.firstCellIndex));
					SetTjsProperty(dict, TJS_W("commandBufferFirstVertexCount"), tTJSVariant(cmdFirst.vertexCount));
					SetTjsProperty(dict, TJS_W("commandBufferFirstIndexCount"), tTJSVariant(cmdFirst.indexCount));
					SetTjsProperty(dict, TJS_W("commandBufferFirstCellCount"), tTJSVariant(cmdFirst.cellCount));
					const MotionRendererCommandEntryInfo &cmdLast = rendererCommandEntries.back();
					SetTjsProperty(dict, TJS_W("commandBufferLastDrawItemIndex"), tTJSVariant(cmdLast.drawItemIndex));
					SetTjsProperty(dict, TJS_W("commandBufferLastSourceIndex"), tTJSVariant(cmdLast.sourceIndex));
					SetTjsProperty(dict, TJS_W("commandBufferLastVertexOffset"), tTJSVariant(cmdLast.vertexOffset));
					SetTjsProperty(dict, TJS_W("commandBufferLastIndexOffset"), tTJSVariant(cmdLast.indexOffset));
					SetTjsProperty(dict, TJS_W("commandBufferLastFirstCellIndex"), tTJSVariant(cmdLast.firstCellIndex));
					SetTjsProperty(dict, TJS_W("commandBufferLastVertexCount"), tTJSVariant(cmdLast.vertexCount));
					SetTjsProperty(dict, TJS_W("commandBufferLastIndexCount"), tTJSVariant(cmdLast.indexCount));
					SetTjsProperty(dict, TJS_W("commandBufferLastCellCount"), tTJSVariant(cmdLast.cellCount));
				}
			}
		}
		{
			const MotionRendererCommandConsumerSummary &cs = rendererConsumerSummary;
			SetTjsProperty(dict, TJS_W("rendererConsumerBuilt"), tTJSVariant((tjs_int)(cs.built ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererConsumerRan"), tTJSVariant((tjs_int)(cs.ran ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererConsumerSource"), tTJSVariant(cs.source));
			SetTjsProperty(dict, TJS_W("rendererConsumerCommandCount"), tTJSVariant(cs.commandCount));
			SetTjsProperty(dict, TJS_W("rendererConsumerVertexTotal"), tTJSVariant(cs.vertexTotal));
			SetTjsProperty(dict, TJS_W("rendererConsumerIndexTotal"), tTJSVariant(cs.indexTotal));
			SetTjsProperty(dict, TJS_W("rendererConsumerCellTotal"), tTJSVariant(cs.cellTotal));
			SetTjsProperty(dict, TJS_W("rendererConsumerOffsetsContiguous"), tTJSVariant((tjs_int)(cs.offsetsContiguous ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererConsumerPerEntryOk"), tTJSVariant((tjs_int)(cs.perEntryOk ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererConsumerOrderOk"), tTJSVariant((tjs_int)(cs.orderOk ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererConsumerSliceOk"), tTJSVariant((tjs_int)(cs.sliceOk ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererConsumerFirstCommandHash"), tTJSVariant(cs.firstCommandHash));
			SetTjsProperty(dict, TJS_W("rendererConsumerLastCommandHash"), tTJSVariant(cs.lastCommandHash));
			SetTjsProperty(dict, TJS_W("rendererConsumerCommandHash"), tTJSVariant(MotionPlanHashText(cs.commandHash)));
			SetTjsProperty(dict, TJS_W("rendererConsumerMatchesProducer"), tTJSVariant((tjs_int)(cs.matchesProducer ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererConsumerMismatchKind"), tTJSVariant(cs.mismatchKind.IsEmpty() ? TJS_W("none") : cs.mismatchKind));
			SetTjsProperty(dict, TJS_W("rendererConsumerMismatchIndex"), tTJSVariant(cs.mismatchIndex));
			SetTjsProperty(dict, TJS_W("rendererConsumerMismatchDetails"), tTJSVariant(cs.mismatchDetails));
			bool consumerHashMatches = (cs.commandHash == rendererCommandHash);
			SetTjsProperty(dict, TJS_W("rendererConsumerHashMatchesCommand"), tTJSVariant((tjs_int)(consumerHashMatches ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererConsumerConsumedIR"), tTJSVariant((tjs_int)(cs.consumedIR ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererConsumerCommandSource"), tTJSVariant(cs.rendererCommandSource));
		}
		{
			const MotionRendererUploadPrepSummary &up = uploadPrepSummary;
			SetTjsProperty(dict, TJS_W("rendererUploadPrepared"), tTJSVariant((tjs_int)(up.prepared ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererUploadRan"), tTJSVariant((tjs_int)(up.ran ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererUploadSource"), tTJSVariant(up.source));
			SetTjsProperty(dict, TJS_W("rendererUploadCommandCount"), tTJSVariant(up.commandCount));
			SetTjsProperty(dict, TJS_W("rendererUploadVertexBufferCount"), tTJSVariant(up.vertexBufferCount));
			SetTjsProperty(dict, TJS_W("rendererUploadIndexBufferCount"), tTJSVariant(up.indexBufferCount));
			SetTjsProperty(dict, TJS_W("rendererUploadCellCount"), tTJSVariant(up.cellBufferCount));
			SetTjsProperty(dict, TJS_W("rendererUploadVertexWriteCount"), tTJSVariant(up.vertexWriteCount));
			SetTjsProperty(dict, TJS_W("rendererUploadIndexWriteCount"), tTJSVariant(up.indexWriteCount));
			SetTjsProperty(dict, TJS_W("rendererUploadAlphaWriteCount"), tTJSVariant(up.alphaWriteCount));
			SetTjsProperty(dict, TJS_W("rendererUploadUvWriteCount"), tTJSVariant(up.uvWriteCount));
			SetTjsProperty(dict, TJS_W("rendererUploadOffsetsOk"), tTJSVariant((tjs_int)(up.offsetsOk ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererUploadRangesOk"), tTJSVariant((tjs_int)(up.rangesOk ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererUploadPerCommandOk"), tTJSVariant((tjs_int)(up.perCommandOk ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererUploadVertexHash"), tTJSVariant(MotionPlanHashText(up.vertexHash)));
			SetTjsProperty(dict, TJS_W("rendererUploadIndexHash"), tTJSVariant(MotionPlanHashText(up.indexHash)));
			SetTjsProperty(dict, TJS_W("rendererUploadAlphaHash"), tTJSVariant(MotionPlanHashText(up.alphaHash)));
			SetTjsProperty(dict, TJS_W("rendererUploadUvHash"), tTJSVariant(MotionPlanHashText(up.uvHash)));
			SetTjsProperty(dict, TJS_W("rendererUploadFirstVertexSample"), tTJSVariant(up.firstVertexSample));
			SetTjsProperty(dict, TJS_W("rendererUploadLastVertexSample"), tTJSVariant(up.lastVertexSample));
			SetTjsProperty(dict, TJS_W("rendererUploadFirstIndexSample"), tTJSVariant(up.firstIndexSample));
			SetTjsProperty(dict, TJS_W("rendererUploadLastIndexSample"), tTJSVariant(up.lastIndexSample));
			SetTjsProperty(dict, TJS_W("rendererUploadMismatchKind"), tTJSVariant(up.mismatchKind.IsEmpty() ? TJS_W("none") : up.mismatchKind));
			SetTjsProperty(dict, TJS_W("rendererUploadMismatchIndex"), tTJSVariant(up.mismatchIndex));
			SetTjsProperty(dict, TJS_W("rendererUploadMismatchDetails"), tTJSVariant(up.mismatchDetails));
			SetTjsProperty(dict, TJS_W("rendererUploadStagingArtifactPopulated"), tTJSVariant((tjs_int)(up.stagingArtifactPopulated ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererUploadStagingVertexArrayHash"), tTJSVariant(MotionPlanHashText(up.stagingVertexArrayHash)));
			SetTjsProperty(dict, TJS_W("rendererUploadStagingIndexArrayHash"), tTJSVariant(MotionPlanHashText(up.stagingIndexArrayHash)));
			SetTjsProperty(dict, TJS_W("rendererUploadStagingAlphaArrayHash"), tTJSVariant(MotionPlanHashText(up.stagingAlphaArrayHash)));
			SetTjsProperty(dict, TJS_W("rendererUploadStagingUvArrayHash"), tTJSVariant(MotionPlanHashText(up.stagingUvArrayHash)));
			SetTjsProperty(dict, TJS_W("rendererUploadStagingFirstPositionSample"), tTJSVariant(up.stagingFirstPositionSample));
			SetTjsProperty(dict, TJS_W("rendererUploadStagingLastPositionSample"), tTJSVariant(up.stagingLastPositionSample));
			SetTjsProperty(dict, TJS_W("rendererUploadStagingFirstUvSample"), tTJSVariant(up.stagingFirstUvSample));
			SetTjsProperty(dict, TJS_W("rendererUploadStagingLastUvSample"), tTJSVariant(up.stagingLastUvSample));
			SetTjsProperty(dict, TJS_W("rendererUploadStagingFirstAlphaSample"), tTJSVariant(up.stagingFirstAlphaSample));
			SetTjsProperty(dict, TJS_W("rendererUploadStagingLastAlphaSample"), tTJSVariant(up.stagingLastAlphaSample));
			SetTjsProperty(dict, TJS_W("rendererUploadStagingIndexRangeMin"), tTJSVariant(up.stagingIndexRangeMin));
			SetTjsProperty(dict, TJS_W("rendererUploadStagingIndexRangeMax"), tTJSVariant(up.stagingIndexRangeMax));
			SetTjsProperty(dict, TJS_W("rendererUploadM4SurfaceChainCommandCount"),
				tTJSVariant(up.m4SurfaceChainCommandCount));
			SetTjsProperty(dict, TJS_W("rendererUploadM4MaxSurfaceChainLength"),
				tTJSVariant(up.m4MaxSurfaceChainLength));
			SetTjsProperty(dict, TJS_W("rendererUploadM4RemappedVertexCount"),
				tTJSVariant(up.m4RemappedVertexCount));
			SetTjsProperty(dict, TJS_W("rendererUploadM4ViewportTransformedVertexCount"),
				tTJSVariant(up.m4ViewportTransformedVertexCount));
			SetTjsProperty(dict, TJS_W("rendererUploadM4SampleNdcX"),
				tTJSVariant(up.m4SampleNdcX));
			SetTjsProperty(dict, TJS_W("rendererUploadM4SampleNdcY"),
				tTJSVariant(up.m4SampleNdcY));
			SetTjsProperty(dict, TJS_W("rendererUploadM4SamplePixelX"),
				tTJSVariant(up.m4SamplePixelX));
			SetTjsProperty(dict, TJS_W("rendererUploadM4SamplePixelY"),
				tTJSVariant(up.m4SamplePixelY));
			SetTjsProperty(dict, TJS_W("rendererUploadM4SampleMaxAbsError"),
				tTJSVariant(up.m4SampleMaxAbsError));
		}
		{
			const MotionShadowDrawDiagnostic &sd = shadowDrawDiagnostic;
			SetTjsProperty(dict, TJS_W("rendererShadowDrawRan"), tTJSVariant((tjs_int)(sd.ran ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawConsumed"), tTJSVariant((tjs_int)(sd.consumed ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawSource"), tTJSVariant(sd.source));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawCommandCount"), tTJSVariant(sd.consumedCommandCount));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawVertexCount"), tTJSVariant(sd.consumedVertexCount));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawIndexCount"), tTJSVariant(sd.consumedIndexCount));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawCellCount"), tTJSVariant(sd.consumedCellCount));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawIndexInRange"), tTJSVariant((tjs_int)(sd.indexInRange ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawIndexRangeMin"), tTJSVariant(sd.indexRangeMin));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawIndexRangeMax"), tTJSVariant(sd.indexRangeMax));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawCoverageMinX"), tTJSVariant(sd.coverageMinX));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawCoverageMinY"), tTJSVariant(sd.coverageMinY));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawCoverageMaxX"), tTJSVariant(sd.coverageMaxX));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawCoverageMaxY"), tTJSVariant(sd.coverageMaxY));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawCoveredTriangleCount"), tTJSVariant(sd.coveredTriangleCount));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawCoveredCellCount"), tTJSVariant(sd.coveredCellCount));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawNonZeroAlphaVertices"), tTJSVariant(sd.nonZeroAlphaVertexCount));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawConsumedPosHash"), tTJSVariant(MotionPlanHashText(sd.consumedPosHash)));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawConsumedUvHash"), tTJSVariant(MotionPlanHashText(sd.consumedUvHash)));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawConsumedAlphaHash"), tTJSVariant(MotionPlanHashText(sd.consumedAlphaHash)));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawConsumedIndexHash"), tTJSVariant(MotionPlanHashText(sd.consumedIndexHash)));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawConsumedCompositeHash"), tTJSVariant(MotionPlanHashText(sd.consumedCompositeHash)));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawFirstConsumedTriangle"), tTJSVariant(sd.firstConsumedTriangle.IsEmpty() ? TJS_W("void") : sd.firstConsumedTriangle));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawFirstConsumedPixelSample"), tTJSVariant(sd.firstConsumedPixelSample.IsEmpty() ? TJS_W("void") : sd.firstConsumedPixelSample));
			bool hasLastTriangle = false;
			if (!stagingArtifact.index.empty() && stagingArtifact.index.size() >= 6) {
				size_t lastBase = stagingArtifact.index.size() - 6;
				tjs_int v0 = stagingArtifact.index[lastBase];
				tjs_int v1 = stagingArtifact.index[lastBase + 1];
				tjs_int v2 = stagingArtifact.index[lastBase + 2];
				tjs_int v4 = stagingArtifact.index[lastBase + 4];
				tjs_int v5 = stagingArtifact.index[lastBase + 5];
				hasLastTriangle = true;
				SetTjsProperty(dict, TJS_W("rendererShadowDrawLastConsumedTriangle"),
					tTJSVariant(ttstr(TJS_W("idx[")) + ttstr((tjs_int)lastBase) +
					TJS_W("]=(") + ttstr(v0) + TJS_W(",") + ttstr(v1) +
					TJS_W(",") + ttstr(v2) + TJS_W(",") + ttstr(v4) +
					TJS_W(",") + ttstr(v5) + TJS_W(")")));
			}
			if (!hasLastTriangle) {
				SetTjsProperty(dict, TJS_W("rendererShadowDrawLastConsumedTriangle"), tTJSVariant(TJS_W("void")));
			}
			SetTjsProperty(dict, TJS_W("rendererShadowDrawMismatchKind"), tTJSVariant(sd.mismatchKind.IsEmpty() ? TJS_W("none") : sd.mismatchKind));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawMismatchIndex"), tTJSVariant(sd.mismatchIndex));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawMismatchDetails"), tTJSVariant(sd.mismatchDetails.IsEmpty() ? TJS_W("void") : sd.mismatchDetails));
			SetTjsProperty(dict, TJS_W("rendererShadowDrawHashMatchesStaging"),
				tTJSVariant((tjs_int)((sd.consumedPosHash == uploadPrepSummary.stagingVertexArrayHash &&
					sd.consumedAlphaHash == uploadPrepSummary.stagingAlphaArrayHash &&
					sd.consumedUvHash == uploadPrepSummary.stagingUvArrayHash &&
					sd.consumedIndexHash == uploadPrepSummary.stagingIndexArrayHash) ? 1 : 0)));
		}
		{
			const MotionOffscreenDrawDiagnostic &od = offscreenDrawDiagnostic;
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawRan"), tTJSVariant((tjs_int)(od.ran ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawConsumed"), tTJSVariant((tjs_int)(od.consumed ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawSource"), tTJSVariant(od.source));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawWidth"), tTJSVariant(od.width));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawHeight"), tTJSVariant(od.height));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawTileGridW"), tTJSVariant(od.tileGridW));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawTileGridH"), tTJSVariant(od.tileGridH));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawCommandCount"), tTJSVariant(od.consumedCommandCount));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawTriangleCount"), tTJSVariant(od.consumedTriangleCount));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawCellCount"), tTJSVariant(od.consumedCellCount));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawCoveredTileCount"), tTJSVariant(od.coveredTileCount));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawAlphaNonZeroTileCount"), tTJSVariant(od.alphaNonZeroTileCount));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawAlphaAccum"), tTJSVariant(od.alphaAccum));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawBoundsMinX"), tTJSVariant(od.boundsMinX));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawBoundsMinY"), tTJSVariant(od.boundsMinY));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawBoundsMaxX"), tTJSVariant(od.boundsMaxX));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawBoundsMaxY"), tTJSVariant(od.boundsMaxY));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawCoverageHash"), tTJSVariant(MotionPlanHashText(od.coverageHash)));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawAlphaHash"), tTJSVariant(MotionPlanHashText(od.alphaHash)));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawCompositeHash"), tTJSVariant(MotionPlanHashText(od.compositeHash)));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawFirstTileSample"), tTJSVariant(od.firstOccupiedTileSample.IsEmpty() ? TJS_W("void") : od.firstOccupiedTileSample));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawLastTileSample"), tTJSVariant(od.lastOccupiedTileSample.IsEmpty() ? TJS_W("void") : od.lastOccupiedTileSample));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawFirstAlphaTileSample"), tTJSVariant(od.firstAlphaTileSample.IsEmpty() ? TJS_W("void") : od.firstAlphaTileSample));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawMismatchKind"), tTJSVariant(od.mismatchKind.IsEmpty() ? TJS_W("none") : od.mismatchKind));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawMismatchIndex"), tTJSVariant(od.mismatchIndex));
			SetTjsProperty(dict, TJS_W("rendererOffscreenDrawMismatchDetails"), tTJSVariant(od.mismatchDetails.IsEmpty() ? TJS_W("void") : od.mismatchDetails));
		}
		{
			const MotionDiagnosticRenderBridge &br = diagnosticRenderBridge;
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeBuilt"), tTJSVariant((tjs_int)(br.built ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeWouldDraw"), tTJSVariant((tjs_int)(br.bridgeWouldDraw ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeFallbackStillUsed"), tTJSVariant((tjs_int)(br.fallbackStillUsed ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeSource"), tTJSVariant(br.source));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeTargetWidth"), tTJSVariant(br.targetWidth));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeTargetHeight"), tTJSVariant(br.targetHeight));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeCompatibleWithLayerTarget"), tTJSVariant((tjs_int)(br.compatibleWithLayerTarget ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeCompatibleWithLayerTargetDetail"), tTJSVariant(br.compatibleWithLayerTargetDetail));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeCommandCount"), tTJSVariant(br.commandCount));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeCellCount"), tTJSVariant(br.cellCount));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeTriangleCount"), tTJSVariant(br.triangleCount));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeCoveredTileCount"), tTJSVariant(br.coveredTileCount));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeAlphaNonZeroTileCount"), tTJSVariant(br.alphaNonZeroTileCount));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeAlphaAccum"), tTJSVariant(br.alphaAccum));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeCoverageHash"), tTJSVariant(MotionPlanHashText(br.coverageHash)));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeAlphaHash"), tTJSVariant(MotionPlanHashText(br.alphaHash)));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeCompositeHash"), tTJSVariant(MotionPlanHashText(br.compositeHash)));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeBoundsMinX"), tTJSVariant(br.boundsMinX));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeBoundsMinY"), tTJSVariant(br.boundsMinY));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeBoundsMaxX"), tTJSVariant(br.boundsMaxX));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeBoundsMaxY"), tTJSVariant(br.boundsMaxY));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeBlendModeSummary"), tTJSVariant(br.blendModeSummary));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeAlphaModeSummary"), tTJSVariant(br.alphaModeSummary));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgePixelCoverageSummary"), tTJSVariant(br.pixelCoverageSummary));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeDrawTargetCompatNote"), tTJSVariant(br.drawTargetCompatNote));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeFirstTileSample"), tTJSVariant(br.firstTileSample.IsEmpty() ? TJS_W("void") : br.firstTileSample));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeLastTileSample"), tTJSVariant(br.lastTileSample.IsEmpty() ? TJS_W("void") : br.lastTileSample));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeFirstAlphaTileSample"), tTJSVariant(br.firstAlphaTileSample.IsEmpty() ? TJS_W("void") : br.firstAlphaTileSample));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeTileGridW"), tTJSVariant(br.tileGridW));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeTileGridH"), tTJSVariant(br.tileGridH));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeMismatchKind"), tTJSVariant(br.mismatchKind.IsEmpty() ? TJS_W("none") : br.mismatchKind));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeMismatchIndex"), tTJSVariant(br.mismatchIndex));
			SetTjsProperty(dict, TJS_W("rendererDiagnosticBridgeMismatchDetails"), tTJSVariant(br.mismatchDetails.IsEmpty() ? TJS_W("void") : br.mismatchDetails));
		}
		{
			const MotionVisibleRenderBridge &vb = visibleRenderBridge;
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeBuilt"), tTJSVariant((tjs_int)(vb.built ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeRendered"), tTJSVariant((tjs_int)(vb.rendered ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeSource"), tTJSVariant(vb.source));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeTargetWidth"), tTJSVariant(vb.targetWidth));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeTargetHeight"), tTJSVariant(vb.targetHeight));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeFallbackStillUsed"), tTJSVariant((tjs_int)(vb.fallbackStillUsed ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgePathUsed"), tTJSVariant((tjs_int)(vb.rendererPathUsed ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgePixelHash"), tTJSVariant(MotionPlanHashText(vb.pixelHash)));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgePixelCount"), tTJSVariant(vb.pixelCount));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeAlphaNonZeroPixelCount"), tTJSVariant(vb.alphaNonZeroPixelCount));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeAlphaAccum"), tTJSVariant(vb.alphaAccum));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticSnapshotCanvasWidth"), tTJSVariant(vb.staticSnapshotCanvasWidth));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticSnapshotCanvasHeight"), tTJSVariant(vb.staticSnapshotCanvasHeight));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticSnapshotOffsetX"), tTJSVariant(vb.staticSnapshotOffsetX));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticSnapshotOffsetY"), tTJSVariant(vb.staticSnapshotOffsetY));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticSnapshotUsedMotionLayout"), tTJSVariant((tjs_int)(vb.staticSnapshotUsedMotionLayout ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticSnapshotItemCount"), tTJSVariant(vb.staticSnapshotItemCount));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticReplayScaleX"), tTJSVariant(vb.staticReplayScaleX));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticReplayScaleY"), tTJSVariant(vb.staticReplayScaleY));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticReplayTargetMatchesCanvas"), tTJSVariant((tjs_int)(vb.staticReplayTargetMatchesCanvas ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticReplayBoundsValid"), tTJSVariant((tjs_int)(vb.staticReplayBoundsValid ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticReplayBoundsMinX"), tTJSVariant(vb.staticReplayBoundsMinX));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticReplayBoundsMinY"), tTJSVariant(vb.staticReplayBoundsMinY));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticReplayBoundsMaxX"), tTJSVariant(vb.staticReplayBoundsMaxX));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticReplayBoundsMaxY"), tTJSVariant(vb.staticReplayBoundsMaxY));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticReplayCoverageValid"), tTJSVariant((tjs_int)(vb.staticReplayCoverageValid ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticReplayCoverageMinX"), tTJSVariant(vb.staticReplayCoverageMinX));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticReplayCoverageMinY"), tTJSVariant(vb.staticReplayCoverageMinY));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticReplayCoverageMaxX"), tTJSVariant(vb.staticReplayCoverageMaxX));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticReplayCoverageMaxY"), tTJSVariant(vb.staticReplayCoverageMaxY));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticReplayInBounds"), tTJSVariant((tjs_int)(vb.staticReplayInBounds ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticReplayCoverageMatchesBounds"), tTJSVariant((tjs_int)(vb.staticReplayCoverageMatchesBounds ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticReplayCoverageInsideTarget"), tTJSVariant((tjs_int)(vb.staticReplayCoverageInsideTarget ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticReplayViewportClippingUsed"), tTJSVariant((tjs_int)(vb.staticReplayViewportClippingUsed ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticReplayViewportClipOk"), tTJSVariant((tjs_int)(vb.staticReplayViewportClipOk ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticReplaySkippedItemCount"), tTJSVariant(vb.staticReplaySkippedItemCount));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticReplayClippedItemCount"), tTJSVariant(vb.staticReplayClippedItemCount));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticReplayFullyOutsideItemCount"), tTJSVariant(vb.staticReplayFullyOutsideItemCount));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticFirstItemSample"), tTJSVariant(vb.staticFirstItemSample.IsEmpty() ? TJS_W("void") : vb.staticFirstItemSample));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeStaticLastItemSample"), tTJSVariant(vb.staticLastItemSample.IsEmpty() ? TJS_W("void") : vb.staticLastItemSample));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeTextureUniqueRefCount"), tTJSVariant(vb.textureUniqueRefCount));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeTextureTotalPixelCount"), tTJSVariant(vb.textureTotalPixelCount));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeTextureAlphaNonZeroPixelCount"), tTJSVariant(vb.textureAlphaNonZeroPixelCount));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeTextureRedPixelCount"), tTJSVariant(vb.textureRedPixelCount));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeTextureWhitePixelCount"), tTJSVariant(vb.textureWhitePixelCount));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeTextureBlackPixelCount"), tTJSVariant(vb.textureBlackPixelCount));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeTextureColorHash"), tTJSVariant(MotionPlanHashText(vb.textureColorHash)));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeTextureColorSummary"), tTJSVariant(vb.textureColorSummary.IsEmpty() ? TJS_W("void") : vb.textureColorSummary));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeSampledSourcePixelCount"), tTJSVariant(vb.sampledSourcePixelCount));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeSampledSourceAlphaNonZeroPixelCount"), tTJSVariant(vb.sampledSourceAlphaNonZeroPixelCount));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeSampledSourceRedPixelCount"), tTJSVariant(vb.sampledSourceRedPixelCount));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeSampledSourceWhitePixelCount"), tTJSVariant(vb.sampledSourceWhitePixelCount));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeSampledSourceBlackPixelCount"), tTJSVariant(vb.sampledSourceBlackPixelCount));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeSampledSourceColorHash"), tTJSVariant(MotionPlanHashText(vb.sampledSourceColorHash)));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeSampledUvBoundsValid"), tTJSVariant((tjs_int)(vb.sampledUvBoundsValid ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeSampledUvMinU"), tTJSVariant(vb.sampledUvMinU));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeSampledUvMaxU"), tTJSVariant(vb.sampledUvMaxU));
				SetTjsProperty(dict, TJS_W("rendererVisibleBridgeSampledUvMinV"), tTJSVariant(vb.sampledUvMinV));
				SetTjsProperty(dict, TJS_W("rendererVisibleBridgeSampledUvMaxV"), tTJSVariant(vb.sampledUvMaxV));
				SetTjsProperty(dict, TJS_W("rendererVisibleBridgeSampledSourceColorSummary"), tTJSVariant(vb.sampledSourceColorSummary.IsEmpty() ? TJS_W("void") : vb.sampledSourceColorSummary));
				SetTjsProperty(dict, TJS_W("rendererVisibleBridgeFinalLayerPixelCount"), tTJSVariant(vb.finalLayerPixelCount));
				SetTjsProperty(dict, TJS_W("rendererVisibleBridgeFinalLayerAlphaNonZeroPixelCount"), tTJSVariant(vb.finalLayerAlphaNonZeroPixelCount));
				SetTjsProperty(dict, TJS_W("rendererVisibleBridgeFinalLayerRedPixelCount"), tTJSVariant(vb.finalLayerRedPixelCount));
				SetTjsProperty(dict, TJS_W("rendererVisibleBridgeFinalLayerWhitePixelCount"), tTJSVariant(vb.finalLayerWhitePixelCount));
				SetTjsProperty(dict, TJS_W("rendererVisibleBridgeFinalLayerBlackPixelCount"), tTJSVariant(vb.finalLayerBlackPixelCount));
				SetTjsProperty(dict, TJS_W("rendererVisibleBridgeFinalLayerColorBucketCount"), tTJSVariant(vb.finalLayerColorBucketCount));
				SetTjsProperty(dict, TJS_W("rendererVisibleBridgeFinalLayerColorHash"), tTJSVariant(MotionPlanHashText(vb.finalLayerColorHash)));
				SetTjsProperty(dict, TJS_W("rendererVisibleBridgeFinalLayerColorSummary"), tTJSVariant(vb.finalLayerColorSummary.IsEmpty() ? TJS_W("void") : vb.finalLayerColorSummary));
				SetTjsProperty(dict, TJS_W("rendererVisibleBridgeTriangleCount"), tTJSVariant(vb.triangleCount));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeCommandCount"), tTJSVariant(vb.commandCount));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeMaxCommandCoverageSummary"),
				tTJSVariant(vb.maxCommandCoverageSummary.IsEmpty() ? TJS_W("void") : vb.maxCommandCoverageSummary));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeMaxCommandIRSummary"),
				tTJSVariant(vb.maxCommandIRSummary.IsEmpty() ? TJS_W("void") : vb.maxCommandIRSummary));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeMaxNonBackgroundCommandCoverageSummary"),
				tTJSVariant(vb.maxNonBackgroundCommandCoverageSummary.IsEmpty() ? TJS_W("void") : vb.maxNonBackgroundCommandCoverageSummary));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeMaxNonBackgroundCommandIRSummary"),
				tTJSVariant(vb.maxNonBackgroundCommandIRSummary.IsEmpty() ? TJS_W("void") : vb.maxNonBackgroundCommandIRSummary));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeMaxBlackCommandCoverageSummary"),
				tTJSVariant(vb.maxBlackCommandCoverageSummary.IsEmpty() ? TJS_W("void") : vb.maxBlackCommandCoverageSummary));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeMaxBlackCommandIRSummary"),
				tTJSVariant(vb.maxBlackCommandIRSummary.IsEmpty() ? TJS_W("void") : vb.maxBlackCommandIRSummary));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeMismatchKind"), tTJSVariant(vb.mismatchKind.IsEmpty() ? TJS_W("none") : vb.mismatchKind));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeMismatchIndex"), tTJSVariant(vb.mismatchIndex));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgeMismatchDetails"), tTJSVariant(vb.mismatchDetails.IsEmpty() ? TJS_W("void") : vb.mismatchDetails));
			SetTjsProperty(dict, TJS_W("rendererVisibleBridgePath"), tTJSVariant(vb.visibleBridgePath.IsEmpty() ? TJS_W("void") : vb.visibleBridgePath));
		}
		SetTjsProperty(dict, TJS_W("drawInvocationCount"), tTJSVariant(drawInvocationCount));
		SetTjsProperty(dict, TJS_W("drawActualCount"), tTJSVariant(drawActualCount));
		SetTjsProperty(dict, TJS_W("lastDrawAttempted"), tTJSVariant((tjs_int)(lastDrawAttempted ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("lastDrawHadTarget"), tTJSVariant((tjs_int)(lastDrawHadTarget ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("lastDrawCacheHit"), tTJSVariant((tjs_int)(lastDrawCacheHit ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("lastDrawLoadedLayerImage"), tTJSVariant((tjs_int)(lastDrawLoadedLayerImage ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("lastDrawUsedFallback"), tTJSVariant((tjs_int)(lastDrawUsedFallback ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("lastDrawResult"), tTJSVariant(lastDrawResult));
		SetTjsProperty(dict, TJS_W("lastDrawPath"), tTJSVariant(lastDrawPath));
		SetTjsProperty(dict, TJS_W("lastDrawFrame"), tTJSVariant(lastDrawFrame));
		SetTjsProperty(dict, TJS_W("lastDrawTick"), tTJSVariant(lastDrawTick));
		tTJSVariant result(dict, dict);
		dict->Release();
		return result;
	}
	tTJSVariant resolveFrontHit(tjs_real x, tjs_real y)
	{
		selectCurrentMotion();
		const MotionResource *resource = resolveRuntimeLayoutResource();
		std::shared_ptr<MotionResource> baseResource = resolveResource();
		tjs_real hitWidth = width > 0 ? width :
			(resource && resource->width > 0 ? resource->width :
				(baseResource && baseResource->width > 0 ? baseResource->width : 1280.0));
		tjs_real hitHeight = height > 0 ? height :
			(resource && resource->height > 0 ? resource->height :
				(baseResource && baseResource->height > 0 ? baseResource->height : 720.0));
		return MakeMotionFrontResolveDump(resource, chara, motion,
			x, y, visible, opacity, hitWidth, hitHeight);
	}
	void assign(iTJSDispatch2 *anotherAdaptor)
	{
		if (!anotherAdaptor) return;
		EmotePlayer *other =
			ncbInstanceAdaptor<EmotePlayer>::GetNativeInstance(anotherAdaptor, false);
		tTJSVariant state;
		if (other) {
			if (other == this) return;
			state = other->serialize();
			motionVariant = other->motionVariant;
			tags = other->tags;
			pendingLayerGetterName = other->pendingLayerGetterName;
			resourceManager = other->resourceManager;
			currentResource = other->currentResource;
		} else {
			if (TJS_FAILED(anotherAdaptor->FuncCall(0, TJS_W("serialize"), nullptr, &state, 0, nullptr, anotherAdaptor))) {
				return;
			}
		}
		unserialize(state);
		selectCurrentMotion();
		invalidateDrawCache();
		invalidateRuntimeLayoutCache();
	}
	void setCoord(tjs_real x, tjs_real y) { coordX = x; coordY = y; }
	tTJSVariant getCoord()
	{
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		tTJSVariant x(coordX), y(coordY);
		dict->PropSet(TJS_MEMBERENSURE, TJS_W("x"), nullptr, &x, dict);
		dict->PropSet(TJS_MEMBERENSURE, TJS_W("y"), nullptr, &y, dict);
		tTJSVariant result(dict, dict);
		dict->Release();
		return result;
	}
	void setScale(tjs_real scale) { scaleX = scale; scaleY = scale; }
	tjs_real getScale() const { return scaleX; }
	void setTimeScale(tjs_real value) { timeScale = value; }
	tjs_real getTimeScale() const { return timeScale; }
	void setAttachRect(tjs_real l, tjs_real t, tjs_real w, tjs_real h)
	{
		left = l;
		top = t;
		width = w;
		height = h;
	}
	tTJSVariant getAttachRect()
	{
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		tTJSVariant l(left), t(top), w(width), h(height);
		dict->PropSet(TJS_MEMBERENSURE, TJS_W("left"), nullptr, &l, dict);
		dict->PropSet(TJS_MEMBERENSURE, TJS_W("top"), nullptr, &t, dict);
		dict->PropSet(TJS_MEMBERENSURE, TJS_W("width"), nullptr, &w, dict);
		dict->PropSet(TJS_MEMBERENSURE, TJS_W("height"), nullptr, &h, dict);
		tTJSVariant result(dict, dict);
		dict->Release();
		return result;
	}
	void setTransform(tTJSVariant value)
	{
		transform = value;
		applyTransformVariant(value);
		invalidateDrawCache();
	}
	tTJSVariant getTransform() const { return transform; }
	void setRotate(tjs_real rotate) { rotation = rotate; }
	void setColor(tjs_uint32 color, tjs_real time = 0, tjs_real easing = 0)
	{
		(void)time;
		(void)easing;
		drawColor = color;
	}
	tjs_uint32 getColor() const { return drawColor; }
	void setVariable(tTJSString name, tjs_real value)
	{
		std::string key = variableKey(ttstr(name));
		std::shared_ptr<MotionResource> resource = resolveResource();
		const MotionSelectorInfo *selector =
			findSelectorControlForKey(resource.get(), key);
		auto previous = variableValues.find(key);
		bool changed = previous == variableValues.end() ||
			std::abs((double)(previous->second - value)) > 0.000001;
#ifdef __EMSCRIPTEN__
		if (MotionClickBurstTraceActive() &&
			MotionTraceInterestingSettingObject(key)) {
			char buf[512];
			std::snprintf(buf, sizeof(buf),
				"set-var key=%s value=%.3f changed=%d revision=%d selector=%d",
				key.c_str(), (double)value, (int)changed,
				(int)variableRevision, selector ? 1 : 0);
			MotionClickBurstTraceLog("player", buf);
		}
#endif
		if (selector) {
			variableValues[key] = value;
			applySelectorControlValues(*selector, value, variableValues);
			if (changed) variableRevision++;
			invalidateRuntimeLayoutCache();
			invalidateDrawCache();
			return;
		}
		tjs_real minValue = 0.0;
		tjs_real maxValue = 0.0;
		bool hasRange = getVariableRangeForKey(key, minValue, maxValue);
		if (hasRange && maxValue >= minValue) {
			value = std::max<tjs_real>(minValue, std::min<tjs_real>(maxValue, value));
		}
		variableValues[key] = value;
		if (changed) variableRevision++;
		invalidateRuntimeLayoutCache();
		invalidateDrawCache();
	}
	tjs_real getVariable(tTJSString name)
	{
		std::string key = variableKey(ttstr(name));
		auto it = variableValues.find(key);
		if (it != variableValues.end()) return it->second;
		tjs_real minValue = 0.0;
		tjs_real maxValue = 0.0;
		if (getVariableRangeForKey(key, minValue, maxValue)) {
			if (maxValue > minValue) return minValue + (maxValue - minValue) / 2.0;
			return minValue;
		}
		return 0.0;
	}
	void setMotionVariable(tTJSString name, tjs_real value)
	{
		setVariable(name, value);
	}
	tTJSVariant getMotionVariable(tTJSString name)
	{
		return tTJSVariant(getVariable(name));
	}
	void setVariableRange(tTJSString name, tjs_real minValue, tjs_real maxValue)
	{
		if (maxValue < minValue) std::swap(minValue, maxValue);
		std::string key = variableKey(ttstr(name));
		manualVariableRanges[key] = std::make_pair(minValue, maxValue);
		auto it = variableValues.find(key);
		if (it != variableValues.end()) {
			it->second = std::max<tjs_real>(minValue, std::min<tjs_real>(maxValue, it->second));
			invalidateRuntimeLayoutCache();
			invalidateDrawCache();
		}
	}
	void setOuterForce(tTJSString name, tjs_real ofx, tjs_real ofy)
	{
		std::string key = timelineKey(ttstr(name));
		if (std::abs((double)ofx) <= 0.000001 && std::abs((double)ofy) <= 0.000001) {
			outerForces.erase(key);
			return;
		}
		MotionOuterForceState &state = outerForces[key];
		state.name = ttstr(name);
		state.x = ofx;
		state.y = ofy;
	}
	void setDrawAffineTranslateMatrix(tjs_real a, tjs_real b, tjs_real c, tjs_real d, tjs_int tx, tjs_int ty)
	{
		affineA = a;
		affineB = b;
		affineC = c;
		affineD = d;
		affineTx = tx;
		affineTy = ty;
		invalidateDrawCache();
	}
	void setCameraOffset(tjs_int width, tjs_int height)
	{
		cameraWidth = width;
		cameraHeight = height;
	}
	void startWind(tjs_real start, tjs_real goal, tjs_real speed, tjs_real powMin, tjs_real powMax)
	{
		windState.active = true;
		windState.start = start;
		windState.goal = goal;
		windState.speed = speed;
		windState.powerMin = powMin;
		windState.powerMax = powMax;
		windState.progress = 0.0;
		wind = powMin;
	}
	void stopWind()
	{
		windState.active = false;
		windState.progress = 0.0;
		wind = 0.0;
	}
	bool contains(tjs_real x, tjs_real y)
	{
		const MotionResource *resource = resolveRuntimeLayoutResource();
		if (resource) {
			bool hadLayout = false;
			if (MotionResourceAnyLayoutContains(*resource, x, y, &hadLayout)) {
				return true;
			}
			if (hadLayout) return false;
		}
		return containsPoint(x, y);
	}
	bool contains(tTJSString valName, tjs_real x, tjs_real y)
	{
		ttstr name(valName);
		if (name.StartsWith(TJS_W("hit_"))) return false;
		const MotionResource *resource = resolveRuntimeLayoutResource();
		if (resource) {
			const MotionLayoutInfo *layout = FindMotionLayout(*resource, name, chara, motion);
			if (layout) return MotionLayoutAnyContains(*resource, name, *layout, x, y);
			MotionLayoutInfo group;
			std::vector<const MotionLayoutInfo*> groupLayouts;
			if (BuildMotionGroupLayout(*resource, name, chara, motion, group, &groupLayouts)) {
				return MotionLayoutAnyContains(groupLayouts, group, x, y);
			}
		}
		return containsPoint(x, y);
	}
	static tjs_error TJS_INTF_METHOD containsRaw(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, EmotePlayer *nativeInstance)
	{
		if (!nativeInstance) {
			if (result) *result = tTJSVariant((tjs_int)0);
			return TJS_S_OK;
		}
		bool hit = false;
		if (numparams >= 3) {
			ttstr name(*param[0]);
			hit = nativeInstance->contains(
				name,
				(tjs_real)*param[1], (tjs_real)*param[2]);
		} else if (numparams >= 2) {
			hit = nativeInstance->contains((tjs_real)*param[0], (tjs_real)*param[1]);
		} else {
			return TJS_E_BADPARAMCOUNT;
		}
		if (result) *result = tTJSVariant((tjs_int)(hit ? 1 : 0));
		return TJS_S_OK;
	}
	void skip()
	{
		selectCurrentMotion();
		tjs_real endTime = getPlaybackEndTime();
		if (endTime > 0.0) tickCount = endTime;
		finishPlayback();
		updateTimelineVariables();
		updateCurrentFrame();
		invalidateDrawCache();
	}
	void skipToSync()
	{
		selectCurrentMotion();
		tjs_real syncTime = getSelectedSyncTime();
		if (syncTime <= 0.0) syncTime = getPlaybackEndTime();
		if (syncTime > 0.0) tickCount = syncTime;
		updateTimelineVariables();
		updateCurrentFrame();
		invalidateDrawCache();
	}
	bool checkStopMotion() const
	{
		return !playing && !allplaying;
	}
	void pass() { skip(); }
	void stop() { finishPlayback(); }
	void addMotion(tTJSString name) {}
	void removeMotion(tTJSString name) {}
	tTJSVariant getMotionList()
	{
		std::shared_ptr<MotionResource> resource = resolveResource();
		if (!resource) return MakeEmptyArray();
		std::vector<ttstr> names;
		for (size_t i = 0; i < resource->motions.size(); i++) {
			if (chara.GetLen() <= 0 || resource->motions[i].objectName == chara) {
				names.push_back(resource->motions[i].name);
			}
		}
		return MakeStringArray(names);
	}
	tTJSVariant getCommandList()
	{
		iTJSDispatch2 *array = TJSCreateArrayObject();
		bool variableCommandPending = variableRevision != lastCommandVariableRevision;
		if (playing || allplaying || animating || variableCommandPending) {
			// Return a tick-stamped entry so Scripts.equalStruct detects change
			// each frame and triggers update(0,0,1,1) -> onPaint -> draw.
			iTJSDispatch2 *dict = TJSCreateDictionaryObject();
			SetTjsProperty(dict, TJS_W("type"),
				tTJSVariant(variableCommandPending &&
					!playing && !allplaying && !animating ?
					TJS_W("variable") : TJS_W("progress")));
			SetTjsProperty(dict, TJS_W("tick"), tTJSVariant(tickCount));
			SetTjsProperty(dict, TJS_W("variableRevision"), tTJSVariant((tjs_int64)variableRevision));
			SetTjsProperty(dict, TJS_W("motion"), tTJSVariant(motion));
			SetTjsProperty(dict, TJS_W("chara"), tTJSVariant(chara));
			tTJSVariant value(dict, dict);
			dict->Release();
			array->PropSetByNum(TJS_MEMBERENSURE, 0, &value, array);
			lastCommandVariableRevision = variableRevision;
		} else if (playWasCalled && !stopCommandSent) {
			stopCommandSent = true;
			iTJSDispatch2 *dict = TJSCreateDictionaryObject();
			SetTjsProperty(dict, TJS_W("type"), tTJSVariant(TJS_W("stop")));
			SetTjsProperty(dict, TJS_W("name"), tTJSVariant(motion));
			SetTjsProperty(dict, TJS_W("motion"), tTJSVariant(motion));
			SetTjsProperty(dict, TJS_W("chara"), tTJSVariant(chara));
			tTJSVariant value(dict, dict);
			dict->Release();
			array->PropSetByNum(TJS_MEMBERENSURE, 0, &value, array);
#ifdef __EMSCRIPTEN__
			if (MotionVerboseLogWanted()) {
				std::shared_ptr<MotionResource> res = resolveResource();
				fprintf(stderr,
					"[EMOTE-CMDLIST] emitting STOP command: motion=%s chara=%s tick=%.1f storage=%s\n",
					motion.GetLen() > 0 ? motion.AsNarrowStdString().c_str() : "(none)",
					chara.GetLen() > 0 ? chara.AsNarrowStdString().c_str() : "(none)",
					tickCount,
					res ? res->storage.AsNarrowStdString().c_str() : "(null)");
				fflush(stderr);
			}
#endif
		}
		tTJSVariant result(array, array);
		array->Release();
		return result;
	}
	tTJSVariant getLayerGetter(tTJSString name)
	{
		return makeLayerGetter(ttstr(name), nullptr);
	}
	tTJSVariant getLayerMotion(tTJSString name)
	{
		tTJSVariant getter = makeLayerGetter(ttstr(name), nullptr);
		tTJSVariant motion;
		ExtractLayerGetterMotion(getter, &motion);
		return motion;
	}
	static tjs_error TJS_INTF_METHOD getLayerGetterRaw(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
	{
		if (!objthis) {
			if (result) result->Clear();
			return TJS_S_OK;
		}
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;
		EmotePlayer *nativeInstance =
			ncbInstanceAdaptor<EmotePlayer>::GetNativeInstance(objthis, false);
		return getLayerGetterRawImpl(result, numparams, param, objthis, nativeInstance);
	}
	static tjs_error TJS_INTF_METHOD getLayerMotionRaw(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
	{
		if (!objthis) {
			if (result) result->Clear();
			return TJS_S_OK;
		}
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;
		tTJSVariant getter;
		tjs_error hr = getLayerGetterRaw(&getter, numparams, param, objthis);
		if (TJS_FAILED(hr)) {
			if (result) result->Clear();
			return TJS_S_OK;
		}
		ExtractLayerGetterMotion(getter, result);
		return TJS_S_OK;
	}
	static tjs_error getLayerGetterRawImpl(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis,
		EmotePlayer *nativeInstance)
	{
		if (!nativeInstance) {
			if (result) result->Clear();
			return TJS_S_OK;
		}
		if (result) {
			ttstr layerName(*param[0]);
			*result = nativeInstance->makeLayerGetter(layerName, objthis);
		}
		return TJS_S_OK;
	}
	tTJSVariant makeLayerGetter(const ttstr &layerName, iTJSDispatch2 *objthis)
	{
		const MotionResource *resource = resolveRuntimeLayoutResource();
		if (!resource) {
			return tTJSVariant();
		}

		const MotionLayoutInfo *layout = FindMotionLayout(*resource, layerName, chara, motion);
		if (layout) {
			pendingLayerGetterName = layerName;
			std::vector<const MotionLayoutInfo*> hitLayouts =
				CollectMotionHitLayouts(*resource, layerName);
			return MakeMotionLayoutLayerGetter(*layout, hitLayouts, objthis);
		}
		ttstr scopedName;
		ttstr leafName;
		if (MotionSplitScopedLeafName(layerName, scopedName, leafName)) {
			std::vector<const MotionLayoutInfo*> scopedLayouts;
			CollectMotionScopedDescendantLeafLayouts(
				*resource, scopedName, leafName, chara, motion, scopedLayouts);
			if (!scopedLayouts.empty()) {
				SortMotionScopedDescendantLeafLayouts(scopedLayouts);
				pendingLayerGetterName = scopedName;
				return MakeMotionLayoutLayerGetter(
					*scopedLayouts[0], scopedLayouts, objthis);
			}
		}
		if (pendingLayerGetterName.GetLen() > 0 && layerName.GetLen() > 0) {
			std::vector<const MotionLayoutInfo*> scopedLayouts;
			CollectMotionScopedDescendantLeafLayouts(
				*resource, pendingLayerGetterName, layerName, chara, motion,
				scopedLayouts);
			if (!scopedLayouts.empty()) {
				SortMotionScopedDescendantLeafLayouts(scopedLayouts);
				return MakeMotionLayoutLayerGetter(
					*scopedLayouts[0], scopedLayouts, objthis);
			}
		}
		MotionLayoutInfo group;
		std::vector<const MotionLayoutInfo*> groupLayouts;
		if (BuildMotionGroupLayout(*resource, layerName, chara, motion, group, &groupLayouts)) {
			pendingLayerGetterName = layerName;
			return MakeMotionLayoutLayerGetter(group, groupLayouts, objthis);
		}
		return tTJSVariant();
	}
	tTJSVariant getVariableList() { return get_variableKeys(); }
	tjs_int getVariableCount() const { return (tjs_int)getVariableInfos().size(); }
	tTJSString getVariableName(tjs_int index) const
	{
		std::vector<MotionVariableInfo> vars = getVariableInfos();
		if (index < 0 || index >= (tjs_int)vars.size()) return TJS_W("");
		return vars[(size_t)index].name;
	}
	tjs_real getVariableMin(tTJSString name) const
	{
		std::string key = variableKey(ttstr(name));
		auto manual = manualVariableRanges.find(key);
		if (manual != manualVariableRanges.end()) return manual->second.first;
		std::vector<MotionVariableInfo> vars = getVariableInfos();
		for (size_t i = 0; i < vars.size(); i++) {
			if (variableKey(vars[i].name) == key) return vars[i].minValue;
		}
		return 0.0;
	}
	tjs_real getVariableMax(tTJSString name) const
	{
		std::string key = variableKey(ttstr(name));
		auto manual = manualVariableRanges.find(key);
		if (manual != manualVariableRanges.end()) return manual->second.second;
		std::vector<MotionVariableInfo> vars = getVariableInfos();
		for (size_t i = 0; i < vars.size(); i++) {
			if (variableKey(vars[i].name) == key) return vars[i].maxValue;
		}
		return 0.0;
	}
	tjs_int getModelWidth() const
	{
		std::shared_ptr<MotionResource> resource = resolveResource();
		return resource ? resource->width : 0;
	}
	tjs_int getModelHeight() const
	{
		std::shared_ptr<MotionResource> resource = resolveResource();
		return resource ? resource->height : 0;
	}
	tjs_int getFrameCount() const
	{
		if (currentMotionInfo) {
			return std::max(currentMotionInfo->frameCount, (tjs_int)std::ceil(currentMotionInfo->syncTime));
		}
		std::shared_ptr<MotionResource> resource = resolveResource();
		tjs_int count = 0;
		if (resource) {
			for (size_t i = 0; i < resource->motions.size(); i++) {
				count = std::max(count, resource->motions[i].frameCount);
				count = std::max(count, (tjs_int)std::ceil(resource->motions[i].syncTime));
			}
		}
		return count;
	}
	tjs_int getCurrentFrame() const { return currentFrame; }
	void setCurrentFrame(tjs_int frame)
	{
		if (frame < 0) frame = 0;
		tjs_real duration = getPlaybackEndTime();
		if (duration > 0.0 && frame > (tjs_int)std::ceil(duration)) {
			frame = (tjs_int)std::ceil(duration);
		}
		currentFrame = frame;
		tickCount = frame;
		updateTimelineVariables();
		invalidateDrawCache();
	}
	tjs_real getDuration() const { return getSelectedDuration(); }
	void load(tTJSString filename)
	{
		motionKey = filename;
			if (resourceManager) {
				resourceManager->load(filename);
				currentResource = resourceManager->findResource(filename);
				if (!currentResource) currentResource = resourceManager->getCurrentResource();
				if (currentResource) {
					if (chara.GetLen() > 0 && !resourceHasObject(*currentResource, chara)) {
						chara.Clear();
						charaExplicit = false;
					}
					if (chara.GetLen() <= 0 && motion.GetLen() <= 0) {
						chara = currentResource->chara;
						charaExplicit = false;
					}
					if (motion.GetLen() <= 0 || !resourceHasMotion(*currentResource, motion, chara)) {
						motion = chooseDefaultMotion(*currentResource, chara);
					}
			}
		}
		invalidateDrawCache();
		invalidateRuntimeLayoutCache();
		selectCurrentMotion();
	}
	void loadPSB(tTJSString filename) { load(filename); }
	void loadFromMemory(tTJSVariant buffer) {}
	bool isLoading() const { return loading; }
	bool isLoaded() const
	{
		std::shared_ptr<MotionResource> resource = resolveResource();
		return resource ? resource->opened : false;
	}
	void setResourceManager(ResourceManager *value)
	{
		resourceManager = value;
		currentResource = resourceManager ? resourceManager->getCurrentResource() : nullptr;
		currentMotionInfo = nullptr;
		selectCurrentMotion();
		invalidateRuntimeLayoutCache();
	}
	ResourceManager *getResourceManager() const { return resourceManager; }
	void setMotion(tTJSVariant value)
	{
		motionVariant = value;
		if (value.Type() == tvtString) {
			motion = ttstr(value);
			selectCurrentMotion();
			invalidateDrawCache();
			invalidateRuntimeLayoutCache();
			return;
		}
		if (value.Type() != tvtObject) return;

			tTJSVariant prop;
			if (GetTjsVariantObjectProperty(value, TJS_W("motionKey"), prop) && prop.Type() != tvtVoid) {
				set_motionKey(ttstr(prop));
			}
			if (GetTjsVariantObjectProperty(value, TJS_W("chara"), prop) && prop.Type() != tvtVoid) {
				chara = ttstr(prop);
				charaExplicit = chara.GetLen() > 0;
			}
		if (GetTjsVariantObjectProperty(value, TJS_W("motion"), prop) && prop.Type() != tvtVoid) {
			motion = ttstr(prop);
		} else if (GetTjsVariantObjectProperty(value, TJS_W("name"), prop) && prop.Type() != tvtVoid) {
			motion = ttstr(prop);
		}
		selectCurrentMotion();
		invalidateDrawCache();
		invalidateRuntimeLayoutCache();
	}
	void setPhysicsTimestep(tjs_real value) { physicsTimestep = value; }
	tjs_real getPhysicsTimestep() const { return physicsTimestep; }
	void setPhysicsGravity(tjs_real x, tjs_real y) { physicsGravityX = x; physicsGravityY = y; }
	void setPhysicsWind(tjs_real x, tjs_real y) { physicsWindX = x; physicsWindY = y; }
	void playTimeline(tTJSString name, tjs_int flags = 0)
	{
		ttstr label(name);
		if (label.GetLen() <= 0) return;
		std::string key = timelineKey(label);
		MotionTimelineRuntimeState &state = activeTimelines[key];
		state.label = label;
		state.flags = flags;
		state.blendRatio = 1.0;
		state.blendStartRatio = 1.0;
		state.blendTargetRatio = 1.0;
		state.blendTime = 0.0;
		state.easing = 0.0;
		state.startedTick = tickCount;
		state.blendStartedTick = tickCount;
		state.stopWhenBlendDone = false;
		state.active = true;
		animating = true;
		updateTimelineVariables();
	}
	static tjs_error TJS_INTF_METHOD playTimelineRaw(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, EmotePlayer *nativeInstance)
	{
		if (!nativeInstance) return TJS_E_NATIVECLASSCRASH;
		if (numparams < 1 || numparams > 2) return TJS_E_BADPARAMCOUNT;
		tTJSString name(*param[0]);
		tjs_int flags = numparams >= 2 ? (tjs_int)*param[1] : 0;
		nativeInstance->playTimeline(name, flags);
		if (result) result->Clear();
		return TJS_S_OK;
	}
	void stopTimeline(tTJSString name = TJS_W(""))
	{
		ttstr label(name);
		if (label.GetLen() <= 0) activeTimelines.clear();
		else activeTimelines.erase(timelineKey(label));
		if (!playing && activeTimelines.empty()) animating = false;
	}
	static tjs_error TJS_INTF_METHOD stopTimelineRaw(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, EmotePlayer *nativeInstance)
	{
		if (!nativeInstance) return TJS_E_NATIVECLASSCRASH;
		if (numparams > 1) return TJS_E_BADPARAMCOUNT;
		tTJSString name = numparams >= 1 ? tTJSString(*param[0]) : TJS_W("");
		nativeInstance->stopTimeline(name);
		if (result) result->Clear();
		return TJS_S_OK;
	}
	bool getTimelinePlaying(tTJSString name = TJS_W(""))
	{
		pruneInactiveTimelines();
		ttstr label(name);
		if (label.GetLen() <= 0) return !activeTimelines.empty();
		return activeTimelines.find(timelineKey(label)) != activeTimelines.end();
	}
	static tjs_error TJS_INTF_METHOD getTimelinePlayingRaw(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, EmotePlayer *nativeInstance)
	{
		if (!nativeInstance) return TJS_E_NATIVECLASSCRASH;
		if (numparams > 1) return TJS_E_BADPARAMCOUNT;
		tTJSString name = numparams >= 1 ? tTJSString(*param[0]) : TJS_W("");
		bool playing = nativeInstance->getTimelinePlaying(name);
		if (result) *result = tTJSVariant((tjs_int)(playing ? 1 : 0));
		return TJS_S_OK;
	}
	bool getLoopTimeline(tTJSString name)
	{
		const MotionTimelineInfo *timeline = findTimeline(ttstr(name));
		return timeline ? timeline->lastTime < 0 : false;
	}
	tjs_real getTimelineTotalFrameCount(tTJSString name)
	{
		const MotionTimelineInfo *timeline = findTimeline(ttstr(name));
		if (!timeline) return 0.0;
		if (timeline->loopBegin >= 0 && timeline->loopEnd >= timeline->loopBegin) {
			return (tjs_real)(timeline->loopEnd - timeline->loopBegin + 1);
		}
		if (timeline->lastTime >= 0) return (tjs_real)timeline->lastTime;
		tjs_real maxTime = 0.0;
		for (size_t i = 0; i < timeline->variables.size(); i++) {
			for (size_t f = 0; f < timeline->variables[i].frames.size(); f++) {
				maxTime = std::max(maxTime, timeline->variables[i].frames[f].time);
			}
		}
		return maxTime;
	}
	tTJSVariant getMainTimelineLabelList() { return getTimelineLabelList(0); }
	tTJSVariant getDiffTimelineLabelList() { return getTimelineLabelList(1); }
	void setTimelineBlendRatio(tTJSString name, tjs_real ratio, tjs_real time = 0, tjs_real easing = 0)
	{
		ttstr label(name);
		if (label.GetLen() <= 0) return;
		std::string key = timelineKey(label);
		MotionTimelineRuntimeState &state = activeTimelines[key];
		state.label = label;
		state.active = true;
		state.blendStartRatio = state.blendRatio;
		state.blendTargetRatio = std::max<tjs_real>(0.0, std::min<tjs_real>(1.0, ratio));
		state.blendRatio = state.blendTargetRatio;
		state.blendTime = std::max<tjs_real>(0.0, time);
		state.easing = easing;
		state.blendStartedTick = tickCount;
		state.stopWhenBlendDone = false;
		if (state.blendTime <= 0.0) state.blendStartRatio = state.blendTargetRatio;
		if (state.startedTick == 0.0 && tickCount > 0.0) state.startedTick = tickCount;
		updateTimelineVariables();
	}
	static tjs_error TJS_INTF_METHOD setTimelineBlendRatioRaw(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, EmotePlayer *nativeInstance)
	{
		if (!nativeInstance) return TJS_E_NATIVECLASSCRASH;
		if (numparams < 2 || numparams > 4) return TJS_E_BADPARAMCOUNT;
		tTJSString name(*param[0]);
		tjs_real ratio = (tjs_real)*param[1];
		tjs_real time = numparams >= 3 ? (tjs_real)*param[2] : 0.0;
		tjs_real easing = numparams >= 4 ? (tjs_real)*param[3] : 0.0;
		nativeInstance->setTimelineBlendRatio(name, ratio, time, easing);
		if (result) result->Clear();
		return TJS_S_OK;
	}
	void fadeInTimeline(tTJSString name, tjs_real time = 0, tjs_real easing = 0)
	{
		playTimeline(name);
		ttstr label(name);
		std::string key = timelineKey(label);
		MotionTimelineRuntimeState &state = activeTimelines[key];
		state.blendRatio = (time > 0.0) ? 0.0 : 1.0;
		state.blendStartRatio = state.blendRatio;
		state.blendTargetRatio = 1.0;
		state.blendTime = std::max<tjs_real>(0.0, time);
		state.easing = easing;
		state.blendStartedTick = tickCount;
		state.stopWhenBlendDone = false;
		updateTimelineVariables();
	}
	void fadeOutTimeline(tTJSString name, tjs_real time = 0, tjs_real easing = 0)
	{
		ttstr label(name);
		if (label.GetLen() <= 0) return;
		std::string key = timelineKey(label);
		MotionTimelineRuntimeState &state = activeTimelines[key];
		state.label = label;
		state.active = true;
		state.blendStartRatio = state.blendRatio;
		state.blendTargetRatio = 0.0;
		state.blendTime = std::max<tjs_real>(0.0, time);
		state.easing = easing;
		state.blendStartedTick = tickCount;
		state.stopWhenBlendDone = true;
		if (time <= 0.0) {
			state.blendRatio = 0.0;
			activeTimelines.erase(key);
		}
		if (!playing && activeTimelines.empty()) animating = false;
	}
	static tjs_error TJS_INTF_METHOD fadeInTimelineRaw(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, EmotePlayer *nativeInstance)
	{
		if (!nativeInstance) return TJS_E_NATIVECLASSCRASH;
		if (numparams < 1 || numparams > 3) return TJS_E_BADPARAMCOUNT;
		tTJSString name(*param[0]);
		tjs_real time = numparams >= 2 ? (tjs_real)*param[1] : 0.0;
		tjs_real easing = numparams >= 3 ? (tjs_real)*param[2] : 0.0;
		nativeInstance->fadeInTimeline(name, time, easing);
		if (result) result->Clear();
		return TJS_S_OK;
	}
	static tjs_error TJS_INTF_METHOD fadeOutTimelineRaw(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, EmotePlayer *nativeInstance)
	{
		if (!nativeInstance) return TJS_E_NATIVECLASSCRASH;
		if (numparams < 1 || numparams > 3) return TJS_E_BADPARAMCOUNT;
		tTJSString name(*param[0]);
		tjs_real time = numparams >= 2 ? (tjs_real)*param[1] : 0.0;
		tjs_real easing = numparams >= 3 ? (tjs_real)*param[2] : 0.0;
		nativeInstance->fadeOutTimeline(name, time, easing);
		if (result) result->Clear();
		return TJS_S_OK;
	}
	tTJSString getRendererBackend() const
	{
		return tTJSString(MotionRendererBackendName(g_motionRendererBackend));
	}
	void setRendererBackend(tTJSString value)
	{
		ttstr backend(value);
		if (backend == TJS_W("cpu-ir") || backend == TJS_W("cpu_ir") ||
			backend == TJS_W("cpu")) {
			g_motionRendererBackend = MotionRendererBackendCpuIR;
		} else {
			g_motionRendererBackend = MotionRendererBackendStaticPsb;
		}
		invalidateDrawCache();
	}
	tTJSVariant getPlayingTimelineInfoList()
	{
		pruneInactiveTimelines();
		iTJSDispatch2 *array = TJSCreateArrayObject();
		tjs_int index = 0;
		for (auto it = activeTimelines.begin(); it != activeTimelines.end(); ++it) {
			const MotionTimelineRuntimeState &state = it->second;
			iTJSDispatch2 *dict = TJSCreateDictionaryObject();
			SetTjsProperty(dict, TJS_W("label"), tTJSVariant(state.label.GetLen() > 0 ? state.label : MotionString(it->first)));
			SetTjsProperty(dict, TJS_W("flags"), tTJSVariant(state.flags));
			SetTjsProperty(dict, TJS_W("blendRatio"), tTJSVariant(state.blendRatio));
			SetTjsProperty(dict, TJS_W("blendStartRatio"), tTJSVariant(state.blendStartRatio));
			SetTjsProperty(dict, TJS_W("blendTargetRatio"), tTJSVariant(state.blendTargetRatio));
			SetTjsProperty(dict, TJS_W("blendTime"), tTJSVariant(state.blendTime));
			SetTjsProperty(dict, TJS_W("easing"), tTJSVariant(state.easing));
			SetTjsProperty(dict, TJS_W("startedTick"), tTJSVariant(state.startedTick));
			SetTjsProperty(dict, TJS_W("blendStartedTick"), tTJSVariant(state.blendStartedTick));
			SetTjsProperty(dict, TJS_W("stopWhenBlendDone"), tTJSVariant((tjs_int)(state.stopWhenBlendDone ? 1 : 0)));
			tTJSVariant value(dict, dict);
			dict->Release();
			array->PropSetByNum(TJS_MEMBERENSURE, index++, &value, array);
		}
		tTJSVariant result(array, array);
		array->Release();
		return result;
	}
	tTJSVariant getVariableFrameList(tTJSString name)
	{
		std::shared_ptr<MotionResource> resource = resolveResource();
		if (!resource) return MakeEmptyArray();
		ttstr label(name);
		for (size_t i = 0; i < resource->timelines.size(); i++) {
			const MotionTimelineInfo &timeline = resource->timelines[i];
			for (size_t v = 0; v < timeline.variables.size(); v++) {
				if (nameMatches(timeline.variables[v].label, label)) {
					return MakeMotionTimelineFrameDumpArray(timeline.variables[v].frames);
				}
			}
		}
		return MakeEmptyArray();
	}

	bool get_animating() const { return animating; }
	void set_animating(bool value) { animating = value; }
	bool get_playing() const { return playing; }
	void set_playing(bool value)
	{
		playing = value;
		if (value) {
			animating = true;
			playWasCalled = true;
			stopCommandSent = false;
		}
	}
	bool get_allplaying() const { return allplaying; }
	void set_allplaying(bool value)
	{
		allplaying = value;
		if (value) {
			animating = true;
			playWasCalled = true;
			stopCommandSent = false;
		}
	}
	tjs_int get_loopTime() const { return loopTime; }
	void set_loopTime(tjs_int value) { loopTime = value; }
	tjs_real get_tickCount() const { return tickCount; }
	void set_tickCount(tjs_real value)
	{
		tickCount = std::max<tjs_real>(0.0, value);
		updateTimelineVariables();
		updateCurrentFrame();
		invalidateDrawCache();
		invalidateRuntimeLayoutCache();
	}
	tjs_real get_speed() const { return speed; }
	void set_speed(tjs_real value) { speed = value; }
	tTJSVariant get_outline() { return tTJSVariant(); }
	void set_outline(tTJSVariant value) {}
	tTJSString get_motionKey() const { return motionKey; }
	void set_motionKey(tTJSString value)
	{
		motionKey = value;
		if (resourceManager) currentResource = resourceManager->findResource(value);
		selectCurrentMotion();
		invalidateDrawCache();
		invalidateRuntimeLayoutCache();
	}
	tTJSString get_motion() const { return motion; }
	void set_motion(tTJSString value)
	{
		motion = value;
		selectCurrentMotion();
		invalidateDrawCache();
		invalidateRuntimeLayoutCache();
	}
	tTJSString get_chara() const { return chara; }
		void set_chara(tTJSString value)
		{
			chara = value;
			charaExplicit = chara.GetLen() > 0;
			selectCurrentMotion();
			invalidateDrawCache();
			invalidateRuntimeLayoutCache();
		}
	tTJSVariant get_variableKeys()
	{
		std::vector<MotionVariableInfo> vars = getVariableInfos();
		std::vector<ttstr> keys;
		for (size_t i = 0; i < vars.size(); i++) keys.push_back(vars[i].name);
		for (auto it = variableValues.begin(); it != variableValues.end(); ++it) {
			ttstr key = MotionString(it->first);
			bool exists = false;
			for (size_t i = 0; i < keys.size(); i++) {
				if (keys[i] == key) {
					exists = true;
					break;
				}
			}
			if (!exists) keys.push_back(key);
		}
		return MakeStringArray(keys);
	}
	void set_variableKeys(tTJSVariant value) {}
	tTJSVariant get_tags()
	{
		if (tags.Type() == tvtVoid) tags = MakeEmptyArray();
		return tags;
	}
	void set_tags(tTJSVariant value) { tags = value; }
	bool get_motionPlaying() const { return playing; }
	void set_motionPlaying(bool value)
	{
		playing = value;
		if (value) {
			animating = true;
			allplaying = true;
			paused = false;
			playWasCalled = true;
			stopCommandSent = false;
		} else {
			allplaying = false;
			animating = !activeTimelines.empty();
			paused = false;
		}
	}
	bool get_visible() const { return visible; }
	void set_visible(bool value) { visible = value; }
	tjs_int get_opacity() const { return opacity; }
	void set_opacity(tjs_int value) { opacity = value; }
	tjs_real get_left() const { return left; }
	void set_left(tjs_real value) { left = value; }
	tjs_real get_top() const { return top; }
	void set_top(tjs_real value) { top = value; }
	tjs_real get_width() const { return width; }
	void set_width(tjs_real value) { width = value; }
	tjs_real get_height() const { return height; }
	void set_height(tjs_real value) { height = value; }
	tjs_real get_scale() const { return scaleX; }
	void set_scale(tjs_real value) { setScale(value); }
	tjs_real get_timeScale() const { return timeScale; }
	void set_timeScale(tjs_real value) { timeScale = value; }
		tjs_real get_bustScale() const { return bustScale; }
		void set_bustScale(tjs_real value) { bustScale = value; }
		tjs_real get_hairScale() const { return hairScale; }
		void set_hairScale(tjs_real value) { hairScale = value; }
		tjs_real get_partsScale() const { return partsScale; }
		void set_partsScale(tjs_real value) { partsScale = value; }
		tjs_real get_meshDivisionRatio() const { return meshDivisionRatio; }
		void set_meshDivisionRatio(tjs_real value) { setMeshDivisionRatio(value); }
		tTJSVariant get_completionType() const { return completionType; }
		void set_completionType(tTJSVariant value) { completionType = value; }
		tjs_real get_wind() const { return wind; }
		void set_wind(tjs_real value)
		{
		wind = value;
		if (std::abs((double)value) <= 0.000001) windState.active = false;
	}
	tjs_uint32 get_color() const { return drawColor; }
	void set_color(tjs_uint32 value) { drawColor = value; }
	bool get_smoothing() const { return smoothing; }
	void set_smoothing(bool value) { smoothing = value; }
	bool get_paused() const { return paused; }
	void set_paused(bool value) { paused = value; }
	void setLeft(tjs_real value) { left = value; }
	tjs_real getLeft() const { return left; }
	void setTop(tjs_real value) { top = value; }
	tjs_real getTop() const { return top; }
	void setWidth(tjs_real value) { width = value; }
	tjs_real getWidth() const { return width; }
	void setHeight(tjs_real value) { height = value; }
	tjs_real getHeight() const { return height; }
	void setVisible(bool value) { visible = value; }
	bool getVisible() const { return visible; }
	void setOpacity(tjs_int value) { opacity = value; }
	tjs_int getOpacity() const { return opacity; }
	void setSmoothing(bool value) { smoothing = value; }
	bool getSmoothing() const { return smoothing; }
	void setBustScale(tjs_real value) { bustScale = value; }
	tjs_real getBustScale() const { return bustScale; }
		void setWind(tjs_real value) { set_wind(value); }
		tjs_real getWind() const { return wind; }
		void setMeshDivisionRatio(tjs_real value) { meshDivisionRatio = value; }
		tjs_real getMeshDivisionRatio() const { return meshDivisionRatio; }

private:
	static bool nameMatches(const ttstr &a, const ttstr &b)
	{
		if (a == b) return true;
		return a.AsLowerCase() == b.AsLowerCase();
	}

	static std::string variableKey(const ttstr &name)
	{
		std::string key;
		if (name.GetLen() > 0 && TVPUtf16ToUtf8(key, name.AsStdString())) return key;
		return name.AsNarrowStdString();
	}

	static std::string timelineKey(const ttstr &name)
	{
		return name.AsLowerCase().AsNarrowStdString();
	}

	static bool resourceHasObject(const MotionResource &resource, const ttstr &name)
	{
		if (name.GetLen() <= 0) return false;
		for (size_t i = 0; i < resource.objects.size(); i++) {
			if (nameMatches(resource.objects[i].name, name)) return true;
		}
		return false;
	}

	static bool resourceHasMotion(const MotionResource &resource,
		const ttstr &motionName, const ttstr &charaName = ttstr())
	{
		if (motionName.GetLen() <= 0) return false;
		for (size_t i = 0; i < resource.motions.size(); i++) {
			const MotionInfo &candidate = resource.motions[i];
			if (!nameMatches(candidate.name, motionName)) continue;
			if (charaName.GetLen() > 0 &&
				!nameMatches(candidate.objectName, charaName)) {
				continue;
			}
			return true;
		}
		return false;
	}

		static ttstr chooseDefaultMotion(const MotionResource &resource,
			const ttstr &charaName = ttstr())
		{
		if (resource.motion.GetLen() > 0 &&
			(charaName.GetLen() <= 0 || resourceHasMotion(resource, resource.motion, charaName))) {
			return resource.motion;
		}
		for (size_t i = 0; i < resource.motions.size(); i++) {
			if (charaName.GetLen() <= 0 ||
				nameMatches(resource.motions[i].objectName, charaName)) {
				return resource.motions[i].name;
			}
		}
			return resource.motions.empty() ? ttstr() : resource.motions[0].name;
		}

		static bool motionObjectHasRequestedMotion(
			const MotionObjectInfo &object, const ttstr &motionName)
		{
			if (motionName.GetLen() <= 0) return false;
			for (size_t i = 0; i < object.motions.size(); i++) {
				if (nameMatches(object.motions[i].name, motionName)) return true;
			}
			return false;
		}

		static int scoreMotionObjectForRequest(const MotionResource &resource,
			const MotionObjectInfo &object, const ttstr &motionName)
		{
			if (motionName.GetLen() <= 0 ||
				!motionObjectHasRequestedMotion(object, motionName)) {
				return std::numeric_limits<int>::min();
			}
			int score = 1000;
			int frontDraw = 0;
			int frontHit = 0;
			int renderLeaves = 0;
			int interactiveLeaves = 0;
			int contextLayouts = 0;
			for (size_t i = 0; i < resource.layouts.size(); i++) {
				const MotionLayoutInfo &layout = resource.layouts[i];
				if (!nameMatches(layout.contextObjectName, object.name) ||
					!nameMatches(layout.contextMotionName, motionName)) {
					continue;
				}
				contextLayouts++;
				if (layout.frontDrawCandidate) frontDraw++;
				if (layout.frontHitCandidate) frontHit++;
				if (layout.sourceKey.GetLen() > 0 &&
					MotionStartsWith(layout.sourceKey.AsNarrowStdString(), "src/")) {
					renderLeaves++;
				}
				if (layout.hitInteractive || layout.hitRole == TJS_W("control") ||
					layout.hitRole == TJS_W("hit-area") ||
					MotionStringHasInteractivePrefix(layout.name) ||
					MotionStringHasInteractivePrefix(layout.objectName) ||
					MotionStringHasInteractivePrefix(layout.hitObjectName)) {
					interactiveLeaves++;
				}
			}
			score += contextLayouts * 3;
			score += renderLeaves * 6;
			score += frontDraw * 4;
			score += frontHit * 16;
			score += interactiveLeaves * 12;
			for (size_t i = 0; i < resource.renderMethodIR.size(); i++) {
				const MotionRenderMethodItem &item = resource.renderMethodIR[i];
				if (!MotionRenderMethodItemRenderableForRuntimeDraw(item)) continue;
				bool linkedToContext = false;
				if (item.layoutIndex >= 0 &&
					item.layoutIndex < (tjs_int)resource.layouts.size()) {
					const MotionLayoutInfo &layout =
						resource.layouts[(size_t)item.layoutIndex];
					linkedToContext =
						nameMatches(layout.contextObjectName, object.name) &&
						nameMatches(layout.contextMotionName, motionName);
				}
				if (!linkedToContext && item.label.GetLen() > 0) {
					for (size_t li = 0; li < resource.layouts.size(); li++) {
						const MotionLayoutInfo &layout = resource.layouts[li];
						if (layout.renderMethodItemIndex != (tjs_int)i) continue;
						if (nameMatches(layout.contextObjectName, object.name) &&
							nameMatches(layout.contextMotionName, motionName)) {
							linkedToContext = true;
							break;
						}
					}
				}
				if (linkedToContext) score += item.type == 1 ? 10 : 8;
			}
			return score;
		}

		static ttstr chooseDefaultObjectForMotion(
			const MotionResource &resource, const ttstr &motionName)
		{
			if (motionName.GetLen() <= 0) {
				return resource.chara.GetLen() > 0 ? resource.chara :
					(resource.objects.empty() ? ttstr() : resource.objects[0].name);
			}
			ttstr best;
			int bestScore = std::numeric_limits<int>::min();
			for (size_t i = 0; i < resource.objects.size(); i++) {
				const MotionObjectInfo &object = resource.objects[i];
				int score = scoreMotionObjectForRequest(resource, object, motionName);
				if (score == std::numeric_limits<int>::min()) continue;
				if (best.GetLen() <= 0 || score > bestScore) {
					best = object.name;
					bestScore = score;
				}
			}
			if (best.GetLen() > 0) return best;
			if (resource.chara.GetLen() > 0 &&
				resourceHasMotion(resource, motionName, resource.chara)) {
				return resource.chara;
			}
			for (size_t i = 0; i < resource.motions.size(); i++) {
				if (nameMatches(resource.motions[i].name, motionName)) {
					return resource.motions[i].objectName;
				}
			}
			return resource.chara.GetLen() > 0 ? resource.chara :
				(resource.objects.empty() ? ttstr() : resource.objects[0].name);
		}

		static int scoreResourceForRequest(const MotionResource &resource,
			const ttstr &motionName, const ttstr &charaName)
	{
		int score = 0;
		if (charaName.GetLen() > 0) {
			score += resourceHasObject(resource, charaName) ? 8 : -8;
		}
		if (motionName.GetLen() > 0) {
			if (resourceHasMotion(resource, motionName, charaName)) score += 4;
			else if (resourceHasMotion(resource, motionName)) score += 1;
			else score -= 2;
		}
		return score;
	}

	std::shared_ptr<MotionResource> resolveResource(
		const ttstr &requestedMotion = ttstr(),
		const ttstr &requestedChara = ttstr()) const
	{
		if (!resourceManager) return nullptr;
		if (motionKey.GetLen() > 0) {
			std::shared_ptr<MotionResource> byKey = resourceManager->findResource(motionKey);
			if (byKey) return byKey;
			if (currentResource) return currentResource;
			return resourceManager->getCurrentResource();
		}
		std::shared_ptr<MotionResource> managerCurrent = resourceManager->getCurrentResource();
		if (!currentResource) return managerCurrent;
		if (!managerCurrent || managerCurrent == currentResource) return currentResource;
		if (requestedMotion.GetLen() <= 0 && requestedChara.GetLen() <= 0) return currentResource;
		int currentScore = scoreResourceForRequest(*currentResource, requestedMotion, requestedChara);
		int managerScore = scoreResourceForRequest(*managerCurrent, requestedMotion, requestedChara);
		if (managerScore > currentScore) {
			return managerCurrent;
		}
		return currentResource;
	}
		const MotionInfo *findMotion(const MotionResource &resource,
			const ttstr &motionName, const ttstr &charaName) const
	{
		for (size_t i = 0; i < resource.motions.size(); i++) {
			const MotionInfo &candidate = resource.motions[i];
			if (motionName.GetLen() > 0 && !nameMatches(candidate.name, motionName)) continue;
			if (charaName.GetLen() > 0 && !nameMatches(candidate.objectName, charaName)) continue;
			return &candidate;
		}
		if (motionName.GetLen() > 0) {
			for (size_t i = 0; i < resource.motions.size(); i++) {
				if (nameMatches(resource.motions[i].name, motionName)) return &resource.motions[i];
			}
		}
		return resource.motions.empty() ? nullptr : &resource.motions[0];
	}

	void selectCurrentMotion()
	{
		const ttstr requestedMotion = motion;
		const ttstr requestedChara = chara;
		std::shared_ptr<MotionResource> resource =
			resolveResource(requestedMotion, requestedChara);
		currentResource = resource;
		currentMotionInfo = nullptr;
			if (!resource) return;
			if (chara.GetLen() > 0 && !resourceHasObject(*resource, chara)) {
				chara.Clear();
				charaExplicit = false;
			}
			if ((chara.GetLen() <= 0 || !charaExplicit) &&
				motion.GetLen() > 0 && resourceHasMotion(*resource, motion)) {
				chara = chooseDefaultObjectForMotion(*resource, motion);
				charaExplicit = false;
			}
			if (chara.GetLen() <= 0) {
				chara = resource->chara;
				charaExplicit = false;
			}
			if (motion.GetLen() <= 0 || !resourceHasMotion(*resource, motion, chara)) {
				motion = chooseDefaultMotion(*resource, chara);
		}
		currentMotionInfo = findMotion(*resource, motion, chara);
			if (currentMotionInfo) {
				loopTime = (tjs_int)std::lround(currentMotionInfo->loopTime);
				motion = currentMotionInfo->name;
				chara = currentMotionInfo->objectName;
		}
	}

	tjs_real getSelectedDuration() const
	{
		if (currentMotionInfo) return getMotionEndTime(*currentMotionInfo);
		std::shared_ptr<MotionResource> resource = resolveResource();
		if (!resource) return 0.0;
		tjs_real duration = 0.0;
		for (size_t i = 0; i < resource->motions.size(); i++) {
			duration = std::max(duration, getMotionEndTime(resource->motions[i]));
		}
		return duration;
	}

	tjs_real getSelectedSyncTime() const
	{
		if (currentMotionInfo) return getMotionSyncTime(*currentMotionInfo);
		std::shared_ptr<MotionResource> resource = resolveResource();
		if (!resource) return 0.0;
		tjs_real syncTime = 0.0;
		for (size_t i = 0; i < resource->motions.size(); i++) {
			syncTime = std::max(syncTime, getMotionSyncTime(resource->motions[i]));
		}
		return syncTime;
	}

	tjs_real getPlaybackEndTime() const
	{
		std::shared_ptr<MotionResource> resource = resolveResource();
		if (resource && resource->isMotion) {
			tjs_real syncTime = getSelectedSyncTime();
			if (syncTime > 0.0) return syncTime;
		}
		return getSelectedDuration();
	}

	tjs_real getMotionEndTime(const MotionInfo &info) const
	{
		std::shared_ptr<MotionResource> resource = resolveResource();
		if (resource && !resource->isMotion && info.lastTime > 0.0) return info.lastTime;
		if (info.syncTime > 0.0) return info.syncTime;
		if (info.lastTime > 0.0) return info.lastTime;
		return info.selfSyncTime;
	}

	tjs_real getMotionSyncTime(const MotionInfo &info) const
	{
		if (info.syncTime > 0.0) return info.syncTime;
		if (info.selfSyncTime > 0.0) return info.selfSyncTime;
		if (info.lastTime > 0.0) return info.lastTime;
		return 0.0;
	}

	bool hasPlayableMotion() const
	{
		if (currentMotionInfo) return true;
		std::shared_ptr<MotionResource> resource = resolveResource();
		return resource && !resource->motions.empty();
	}

	bool currentMotionHasLoop() const
	{
		return currentMotionInfo && currentMotionInfo->loopTime >= 0.0;
	}

	bool isVariableDrivenResource() const
	{
		std::shared_ptr<MotionResource> resource = resolveResource();
		if (!resource) return false;
		if (!resource->variables.empty()) return true;
		return currentMotionInfo && !currentMotionInfo->variables.empty();
	}

	bool currentMotionHasInteractiveLayouts() const
	{
		std::shared_ptr<MotionResource> resource = resolveResource();
		if (!resource || chara.GetLen() <= 0 || motion.GetLen() <= 0) return false;
		for (size_t i = 0; i < resource->layouts.size(); i++) {
			const MotionLayoutInfo &layout = resource->layouts[i];
			if (!(layout.hitInteractive || layout.hitRole == TJS_W("control"))) continue;
			bool contextMatches =
				nameMatches(layout.contextObjectName, chara) &&
				nameMatches(layout.contextMotionName, motion);
			bool directMatches =
				nameMatches(layout.objectName, chara) &&
				nameMatches(layout.motionName, motion);
			if (contextMatches || directMatches) return true;
		}
		return false;
	}

	tjs_real getInteractiveMotionInputReadyTime(tjs_real playbackEndTime) const
	{
		std::shared_ptr<MotionResource> resource = resolveResource();
		if (!resource || !resource->isMotion || !currentMotionInfo) return 0.0;
		if (currentMotionHasLoop()) return 0.0;
		if (!currentMotionHasInteractiveLayouts()) return 0.0;
		tjs_real readyTime = currentMotionInfo->lastTime;
		if (readyTime <= 0.0) readyTime = currentMotionInfo->selfSyncTime;
		if (readyTime <= 0.0 || readyTime >= playbackEndTime) return 0.0;
		return readyTime;
	}

	bool shouldKeepPlayingAtDuration() const
	{
		return currentMotionHasLoop() || isVariableDrivenResource();
	}

	bool wrapLoopTick(tjs_real duration)
	{
		if (!currentMotionHasLoop() || duration <= 0.0) return false;
		tjs_real begin = std::max<tjs_real>(0.0, currentMotionInfo->loopTime);
		if (begin >= duration) {
			tickCount = begin;
			return true;
		}
		tjs_real span = duration - begin;
		if (span <= 0.0) return false;
		tickCount = begin + std::fmod(tickCount - begin, span);
		if (tickCount < begin) tickCount += span;
		return true;
	}

	const MotionTimelineInfo *findTimeline(const ttstr &name) const
	{
		std::shared_ptr<MotionResource> resource = resolveResource();
		if (!resource || name.GetLen() <= 0) return nullptr;
		for (size_t i = 0; i < resource->timelines.size(); i++) {
			if (nameMatches(resource->timelines[i].label, name)) return &resource->timelines[i];
		}
		return nullptr;
	}

	tTJSVariant getTimelineLabelList(tjs_int diffType) const
	{
		std::shared_ptr<MotionResource> resource = resolveResource();
		if (!resource) return MakeEmptyArray();
		std::vector<ttstr> labels;
		for (size_t i = 0; i < resource->timelines.size(); i++) {
			if (resource->timelines[i].diff == diffType) labels.push_back(resource->timelines[i].label);
		}
		return MakeStringArray(labels);
	}

	tjs_real timelineLocalTime(const MotionTimelineInfo &timeline) const
	{
		tjs_real time = std::max<tjs_real>(0.0, tickCount - timelineStartTick(timeline));
		if (timeline.loopBegin >= 0) time += (tjs_real)timeline.loopBegin;
		if (timeline.loopEnd > timeline.loopBegin &&
			time > (tjs_real)timeline.loopEnd) {
			tjs_real span = (tjs_real)(timeline.loopEnd - timeline.loopBegin);
			if (span > 0.0) {
				time = (tjs_real)timeline.loopBegin +
					std::fmod(time - (tjs_real)timeline.loopBegin, span);
			}
		} else if (timeline.lastTime >= 0) {
			time = std::min<tjs_real>(time, (tjs_real)timeline.lastTime);
		}
		return time;
	}

	bool evaluateTimelineVariable(const MotionTimelineVariableInfo &variable,
		tjs_real time, tjs_real &value) const
	{
		if (variable.frames.empty()) return false;
		const MotionTimelineFrameInfo *current = nullptr;
		const MotionTimelineFrameInfo *next = nullptr;
		for (size_t i = 0; i < variable.frames.size(); i++) {
			const MotionTimelineFrameInfo &frame = variable.frames[i];
			if (frame.time <= time) current = &frame;
			else {
				next = &frame;
				break;
			}
		}
		if (!current) current = &variable.frames[0];
		if (!current->hasContent) return false;
		value = current->value;
		if (next && next->hasContent && next->time > current->time && time > current->time) {
			tjs_real ratio = (time - current->time) / (next->time - current->time);
			ratio = std::max<tjs_real>(0.0, std::min<tjs_real>(1.0, ratio));
			value = current->value + (next->value - current->value) * ratio;
		}
		return true;
	}

	tjs_real applyEasingRatio(tjs_real ratio, tjs_real easing) const
	{
		ratio = std::max<tjs_real>(0.0, std::min<tjs_real>(1.0, ratio));
		if (easing < 0.0) {
			return 1.0 - std::cos((ratio * (tjs_real)M_PI) / 2.0);
		}
		if (easing > 0.0) {
			return std::sin((ratio * (tjs_real)M_PI) / 2.0);
		}
		return ratio;
	}

	tjs_real timelineStartTick(const MotionTimelineInfo &timeline) const
	{
		for (auto it = activeTimelines.begin(); it != activeTimelines.end(); ++it) {
			const MotionTimelineRuntimeState &state = it->second;
			if (!state.active) continue;
			ttstr label = state.label.GetLen() > 0 ? state.label : MotionString(it->first);
			if (nameMatches(label, timeline.label)) return state.startedTick;
		}
		return 0.0;
	}

	tjs_real timelineBlendRatioForKey(const std::string &key) const
	{
		auto it = activeTimelines.find(key);
		if (it == activeTimelines.end()) return 1.0;
		return std::max<tjs_real>(0.0, std::min<tjs_real>(1.0, it->second.blendRatio));
	}

	void updateTimelineVariables()
	{
		std::shared_ptr<MotionResource> resource = resolveResource();
		if (!resource || activeTimelines.empty()) return;
		pruneInactiveTimelines();
		for (auto it = activeTimelines.begin(); it != activeTimelines.end(); ++it) {
			const MotionTimelineInfo *timeline =
				findTimeline(it->second.label.GetLen() > 0 ? it->second.label : MotionString(it->first));
			if (!timeline) continue;
			tjs_real time = timelineLocalTime(*timeline);
			tjs_real blend = timelineBlendRatioForKey(it->first);
			for (size_t i = 0; i < timeline->variables.size(); i++) {
				tjs_real value = 0.0;
				if (evaluateTimelineVariable(timeline->variables[i], time, value)) {
					std::string key = variableKey(timeline->variables[i].label);
					if (blend < 0.999) {
						tjs_real before = getVariable(timeline->variables[i].label);
						value = before + (value - before) * blend;
					}
					variableValues[key] = value;
				}
			}
		}
	}

	void finishPlayback()
	{
		playing = false;
		animating = !activeTimelines.empty();
		allplaying = false;
	}

	void finishInputPlaybackKeepAnimating()
	{
		playing = false;
		animating = true;
		allplaying = true;
	}

	void updateCurrentFrame()
	{
		tjs_int frame = tickCount < 0.0 ? 0 : (tjs_int)std::lround(tickCount);
		tjs_real duration = getPlaybackEndTime();
		if (duration > 0.0) frame = std::min<tjs_int>(frame, (tjs_int)std::ceil(duration));
		currentFrame = frame;
	}

	void invalidateDrawCache()
	{
		lastDrawPath.Clear();
		lastDrawTarget = nullptr;
		lastDrawCacheHit = false;
	}

	void invalidateRuntimeLayoutCache()
	{
		runtimeLayoutResourceValid = false;
	}

	static bool applySelectorControlValues(const MotionSelectorInfo &selector,
		tjs_real value, std::map<std::string, tjs_real> &values)
	{
		if (selector.options.empty() || !std::isfinite((double)value)) return false;
		int selected = static_cast<int>(value);
		if (selected < 0 || selected >= (int)selector.options.size()) return false;
		for (size_t i = 0; i < selector.options.size(); i++) {
			const MotionSelectorOptionInfo &option = selector.options[i];
			values[variableKey(option.label)] =
				((int)i == selected) ? option.onValue : option.offValue;
		}
		return true;
	}

	const MotionSelectorInfo *findSelectorControlForKey(
		const MotionResource *resource, const std::string &key) const
	{
		if (!resource) return nullptr;
		for (size_t i = 0; i < resource->selectors.size(); i++) {
			if (variableKey(resource->selectors[i].label) == key) {
				return &resource->selectors[i];
			}
		}
		return nullptr;
	}

	std::map<std::string, tjs_real> makeEffectiveVariableValues(
		const MotionResource *resource) const
	{
		std::map<std::string, tjs_real> values = variableValues;
		if (!resource) return values;
		for (size_t i = 0; i < resource->selectors.size(); i++) {
			const MotionSelectorInfo &selector = resource->selectors[i];
			std::string key = variableKey(selector.label);
			tjs_real value = 0.0;
			std::map<std::string, tjs_real>::iterator it = values.find(key);
			if (it != values.end()) {
				value = it->second;
			}
			applySelectorControlValues(selector, value, values);
		}
		return values;
	}

	bool variableValuesContainLayoutSelectors(const MotionResource *resource,
		const std::map<std::string, tjs_real> &values) const
	{
		if (!resource) return false;
		if (!resource->selectors.empty()) return true;
		for (size_t i = 0; i < resource->variables.size(); i++) {
			const MotionVariableInfo &var = resource->variables[i];
			if (var.division <= 0.0 || var.name.GetLen() <= 0) continue;
			if (values.find(variableKey(var.name)) != values.end()) {
				return true;
			}
		}
		for (size_t mi = 0; mi < resource->motions.size(); mi++) {
			const MotionInfo &motionInfo = resource->motions[mi];
			for (size_t vi = 0; vi < motionInfo.variables.size(); vi++) {
				const MotionVariableInfo &var = motionInfo.variables[vi];
				if (var.division <= 0.0 || var.name.GetLen() <= 0) continue;
				std::string localKey = variableKey(var.name);
				std::vector<std::string> candidates;
				MotionAppendUniqueVariableCandidate(candidates, localKey);
				MotionAppendUniqueVariableCandidate(candidates,
					variableKey(motionInfo.objectName) + "/" + localKey);
				MotionAppendUniqueVariableCandidate(candidates,
					variableKey(motionInfo.objectName) + "/" + variableKey(motionInfo.name));
				for (size_t ci = 0; ci < candidates.size(); ci++) {
					if (values.find(candidates[ci]) != values.end()) return true;
				}
			}
		}
		return false;
	}

	const MotionResource *resolveRuntimeLayoutResource()
	{
		selectCurrentMotion();
		std::shared_ptr<MotionResource> resource = resolveResource();
		if (!resource) return nullptr;
		if (!resource->parsed) return resource.get();

		std::map<std::string, tjs_real> effectiveVariables =
			makeEffectiveVariableValues(resource.get());
		bool hasLayoutSelectors =
			variableValuesContainLayoutSelectors(resource.get(), effectiveVariables);
		if (!hasLayoutSelectors) return resource.get();
		bool cacheMatches =
			runtimeLayoutResourceValid &&
			runtimeLayoutSource.get() == resource.get() &&
			runtimeLayoutObject == chara &&
			runtimeLayoutMotion == motion &&
			runtimeLayoutTick == tickCount &&
			runtimeLayoutVariables == effectiveVariables;
		if (cacheMatches) return &runtimeLayoutResource;

		MotionResource candidate;
		if (MotionBuildRuntimeDrawResource(*resource, chara, motion, tickCount,
			0.0, 0.0,
			resource->width > 0 ? (tjs_real)resource->width : 0.0,
			resource->height > 0 ? (tjs_real)resource->height : 0.0,
			candidate, &effectiveVariables)) {
			runtimeLayoutResource = candidate;
			runtimeLayoutSource = resource;
			runtimeLayoutObject = chara;
			runtimeLayoutMotion = motion;
			runtimeLayoutTick = tickCount;
			runtimeLayoutVariables = effectiveVariables;
			runtimeLayoutResourceValid = true;
			return &runtimeLayoutResource;
		}
		runtimeLayoutResourceValid = false;
		return resource.get();
	}

	std::vector<MotionVariableInfo> getVariableInfos() const
	{
		std::vector<MotionVariableInfo> vars;
		if (currentMotionInfo) {
			for (size_t i = 0; i < currentMotionInfo->variables.size(); i++) {
				MotionAddVariable(vars, currentMotionInfo->variables[i]);
			}
		}
		std::shared_ptr<MotionResource> resource = resolveResource();
		if (resource) {
			for (size_t i = 0; i < resource->variables.size(); i++) {
				MotionAddVariable(vars, resource->variables[i]);
			}
		}
		for (auto it = manualVariableRanges.begin(); it != manualVariableRanges.end(); ++it) {
			MotionVariableInfo var;
			var.name = MotionString(it->first);
			var.minValue = it->second.first;
			var.maxValue = it->second.second;
			MotionAddVariable(vars, var);
		}
		return vars;
	}

	bool getVariableRangeForKey(const std::string &key,
		tjs_real &minValue, tjs_real &maxValue) const
	{
		auto manual = manualVariableRanges.find(key);
		if (manual != manualVariableRanges.end()) {
			minValue = manual->second.first;
			maxValue = manual->second.second;
			return true;
		}
		std::vector<MotionVariableInfo> vars = getVariableInfos();
		for (size_t i = 0; i < vars.size(); i++) {
			if (variableKey(vars[i].name) == key) {
				minValue = vars[i].minValue;
				maxValue = vars[i].maxValue;
				return true;
			}
		}
		return false;
	}

	tTJSVariant makeVariableStateArray() const
	{
		iTJSDispatch2 *array = TJSCreateArrayObject();
		tjs_int index = 0;
		for (auto it = variableValues.begin(); it != variableValues.end(); ++it) {
			iTJSDispatch2 *dict = TJSCreateDictionaryObject();
			SetTjsProperty(dict, TJS_W("name"), tTJSVariant(MotionString(it->first)));
			SetTjsProperty(dict, TJS_W("value"), tTJSVariant(it->second));
			tTJSVariant value(dict, dict);
			dict->Release();
			array->PropSetByNum(TJS_MEMBERENSURE, index++, &value, array);
		}
		tTJSVariant result(array, array);
		array->Release();
		return result;
	}

	tTJSVariant makeVariableRangeStateArray() const
	{
		iTJSDispatch2 *array = TJSCreateArrayObject();
		tjs_int index = 0;
		for (auto it = manualVariableRanges.begin(); it != manualVariableRanges.end(); ++it) {
			iTJSDispatch2 *dict = TJSCreateDictionaryObject();
			SetTjsProperty(dict, TJS_W("name"), tTJSVariant(MotionString(it->first)));
			SetTjsProperty(dict, TJS_W("min"), tTJSVariant(it->second.first));
			SetTjsProperty(dict, TJS_W("max"), tTJSVariant(it->second.second));
			tTJSVariant value(dict, dict);
			dict->Release();
			array->PropSetByNum(TJS_MEMBERENSURE, index++, &value, array);
		}
		tTJSVariant result(array, array);
		array->Release();
		return result;
	}

	tTJSVariant makeTimelineStateArray() const
	{
		iTJSDispatch2 *array = TJSCreateArrayObject();
		tjs_int index = 0;
		for (auto it = activeTimelines.begin(); it != activeTimelines.end(); ++it) {
			const MotionTimelineRuntimeState &state = it->second;
			iTJSDispatch2 *dict = TJSCreateDictionaryObject();
			SetTjsProperty(dict, TJS_W("label"), tTJSVariant(state.label.GetLen() > 0 ? state.label : MotionString(it->first)));
			SetTjsProperty(dict, TJS_W("flags"), tTJSVariant(state.flags));
			SetTjsProperty(dict, TJS_W("blendRatio"), tTJSVariant(state.blendRatio));
			SetTjsProperty(dict, TJS_W("blendStartRatio"), tTJSVariant(state.blendStartRatio));
			SetTjsProperty(dict, TJS_W("blendTargetRatio"), tTJSVariant(state.blendTargetRatio));
			SetTjsProperty(dict, TJS_W("blendTime"), tTJSVariant(state.blendTime));
			SetTjsProperty(dict, TJS_W("easing"), tTJSVariant(state.easing));
			SetTjsProperty(dict, TJS_W("startedTick"), tTJSVariant(state.startedTick));
			SetTjsProperty(dict, TJS_W("blendStartedTick"), tTJSVariant(state.blendStartedTick));
			SetTjsProperty(dict, TJS_W("stopWhenBlendDone"), tTJSVariant((tjs_int)(state.stopWhenBlendDone ? 1 : 0)));
			SetTjsProperty(dict, TJS_W("active"), tTJSVariant((tjs_int)(state.active ? 1 : 0)));
			tTJSVariant value(dict, dict);
			dict->Release();
			array->PropSetByNum(TJS_MEMBERENSURE, index++, &value, array);
		}
		tTJSVariant result(array, array);
		array->Release();
		return result;
	}

	void restoreVariableState(tTJSVariant data)
	{
		tTJSVariant vars;
		if (GetTjsVariantObjectProperty(data, TJS_W("variables"), vars) && vars.Type() == tvtObject) {
			tjs_int count = GetTjsVariantArrayCount(vars);
			for (tjs_int i = 0; i < count; i++) {
				tTJSVariant item;
				if (!GetTjsVariantArrayElement(vars, i, item) || item.Type() != tvtObject) continue;
				ttstr name;
				tjs_real value = 0.0;
				if (MotionReadVariantString(item, TJS_W("name"), name) &&
					MotionReadVariantReal(item, TJS_W("value"), value)) {
					variableValues[variableKey(name)] = value;
				}
			}
		}
		tTJSVariant ranges;
		if (GetTjsVariantObjectProperty(data, TJS_W("variableRanges"), ranges) && ranges.Type() == tvtObject) {
			tjs_int count = GetTjsVariantArrayCount(ranges);
			for (tjs_int i = 0; i < count; i++) {
				tTJSVariant item;
				if (!GetTjsVariantArrayElement(ranges, i, item) || item.Type() != tvtObject) continue;
				ttstr name;
				tjs_real minValue = 0.0;
				tjs_real maxValue = 0.0;
				if (MotionReadVariantString(item, TJS_W("name"), name) &&
					MotionReadVariantReal(item, TJS_W("min"), minValue) &&
					MotionReadVariantReal(item, TJS_W("max"), maxValue)) {
					if (maxValue < minValue) std::swap(minValue, maxValue);
					manualVariableRanges[variableKey(name)] = std::make_pair(minValue, maxValue);
				}
			}
		}
	}

	void restoreTimelineState(tTJSVariant data)
	{
		tTJSVariant timelines;
		if (!GetTjsVariantObjectProperty(data, TJS_W("timelines"), timelines) ||
			timelines.Type() != tvtObject) {
			return;
		}
		activeTimelines.clear();
		tjs_int count = GetTjsVariantArrayCount(timelines);
		for (tjs_int i = 0; i < count; i++) {
			tTJSVariant item;
			if (!GetTjsVariantArrayElement(timelines, i, item) || item.Type() != tvtObject) continue;
			ttstr label;
			if (!MotionReadVariantString(item, TJS_W("label"), label) || label.GetLen() <= 0) continue;
			MotionTimelineRuntimeState state;
			state.label = label;
			MotionReadVariantInt(item, TJS_W("flags"), state.flags);
			MotionReadVariantReal(item, TJS_W("blendRatio"), state.blendRatio);
			state.blendStartRatio = state.blendRatio;
			state.blendTargetRatio = state.blendRatio;
			MotionReadVariantReal(item, TJS_W("blendStartRatio"), state.blendStartRatio);
			MotionReadVariantReal(item, TJS_W("blendTargetRatio"), state.blendTargetRatio);
			MotionReadVariantReal(item, TJS_W("blendTime"), state.blendTime);
			MotionReadVariantReal(item, TJS_W("easing"), state.easing);
			MotionReadVariantReal(item, TJS_W("startedTick"), state.startedTick);
			MotionReadVariantReal(item, TJS_W("blendStartedTick"), state.blendStartedTick);
			MotionReadVariantBool(item, TJS_W("stopWhenBlendDone"), state.stopWhenBlendDone);
			MotionReadVariantBool(item, TJS_W("active"), state.active);
			state.blendRatio = std::max<tjs_real>(0.0, std::min<tjs_real>(1.0, state.blendRatio));
			state.blendStartRatio = std::max<tjs_real>(0.0, std::min<tjs_real>(1.0, state.blendStartRatio));
			state.blendTargetRatio = std::max<tjs_real>(0.0, std::min<tjs_real>(1.0, state.blendTargetRatio));
			activeTimelines[timelineKey(label)] = state;
		}
	}

	tTJSVariant makeAffineMatrixState() const
	{
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		SetTjsProperty(dict, TJS_W("a"), tTJSVariant(affineA));
		SetTjsProperty(dict, TJS_W("b"), tTJSVariant(affineB));
		SetTjsProperty(dict, TJS_W("c"), tTJSVariant(affineC));
		SetTjsProperty(dict, TJS_W("d"), tTJSVariant(affineD));
		SetTjsProperty(dict, TJS_W("tx"), tTJSVariant(affineTx));
		SetTjsProperty(dict, TJS_W("ty"), tTJSVariant(affineTy));
		tTJSVariant result(dict, dict);
		dict->Release();
		return result;
	}

	tTJSVariant makeWindState() const
	{
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		SetTjsProperty(dict, TJS_W("active"), tTJSVariant((tjs_int)(windState.active ? 1 : 0)));
		SetTjsProperty(dict, TJS_W("start"), tTJSVariant(windState.start));
		SetTjsProperty(dict, TJS_W("goal"), tTJSVariant(windState.goal));
		SetTjsProperty(dict, TJS_W("speed"), tTJSVariant(windState.speed));
		SetTjsProperty(dict, TJS_W("powerMin"), tTJSVariant(windState.powerMin));
		SetTjsProperty(dict, TJS_W("powerMax"), tTJSVariant(windState.powerMax));
		SetTjsProperty(dict, TJS_W("progress"), tTJSVariant(windState.progress));
		tTJSVariant result(dict, dict);
		dict->Release();
		return result;
	}

	tTJSVariant makeOuterForceStateArray() const
	{
		iTJSDispatch2 *array = TJSCreateArrayObject();
		tjs_int index = 0;
		for (auto it = outerForces.begin(); it != outerForces.end(); ++it) {
			const MotionOuterForceState &state = it->second;
			iTJSDispatch2 *dict = TJSCreateDictionaryObject();
			SetTjsProperty(dict, TJS_W("name"), tTJSVariant(state.name));
			SetTjsProperty(dict, TJS_W("x"), tTJSVariant(state.x));
			SetTjsProperty(dict, TJS_W("y"), tTJSVariant(state.y));
			tTJSVariant value(dict, dict);
			dict->Release();
			array->PropSetByNum(TJS_MEMBERENSURE, index++, &value, array);
		}
		tTJSVariant result(array, array);
		array->Release();
		return result;
	}

	void restoreAffineMatrixState(tTJSVariant data)
	{
		tTJSVariant item;
		if (!GetTjsVariantObjectProperty(data, TJS_W("affineMatrix"), item) || item.Type() != tvtObject) return;
		MotionReadVariantReal(item, TJS_W("a"), affineA);
		MotionReadVariantReal(item, TJS_W("b"), affineB);
		MotionReadVariantReal(item, TJS_W("c"), affineC);
		MotionReadVariantReal(item, TJS_W("d"), affineD);
		MotionReadVariantInt(item, TJS_W("tx"), affineTx);
		MotionReadVariantInt(item, TJS_W("ty"), affineTy);
	}

	void restoreWindState(tTJSVariant data)
	{
		tTJSVariant item;
		if (!GetTjsVariantObjectProperty(data, TJS_W("windState"), item) || item.Type() != tvtObject) return;
		MotionReadVariantBool(item, TJS_W("active"), windState.active);
		MotionReadVariantReal(item, TJS_W("start"), windState.start);
		MotionReadVariantReal(item, TJS_W("goal"), windState.goal);
		MotionReadVariantReal(item, TJS_W("speed"), windState.speed);
		MotionReadVariantReal(item, TJS_W("powerMin"), windState.powerMin);
		MotionReadVariantReal(item, TJS_W("powerMax"), windState.powerMax);
		MotionReadVariantReal(item, TJS_W("progress"), windState.progress);
	}

	void restoreOuterForceState(tTJSVariant data)
	{
		tTJSVariant items;
		if (!GetTjsVariantObjectProperty(data, TJS_W("outerForces"), items) || items.Type() != tvtObject) return;
		outerForces.clear();
		tjs_int count = GetTjsVariantArrayCount(items);
		for (tjs_int i = 0; i < count; i++) {
			tTJSVariant item;
			if (!GetTjsVariantArrayElement(items, i, item) || item.Type() != tvtObject) continue;
			MotionOuterForceState state;
			if (!MotionReadVariantString(item, TJS_W("name"), state.name) || state.name.GetLen() <= 0) continue;
			MotionReadVariantReal(item, TJS_W("x"), state.x);
			MotionReadVariantReal(item, TJS_W("y"), state.y);
			outerForces[timelineKey(state.name)] = state;
		}
	}

	void applyTransformVariant(tTJSVariant value)
	{
		if (value.Type() != tvtObject) return;
		tjs_real realValue = 0.0;
		if (MotionReadVariantReal(value, TJS_W("x"), realValue)) coordX = realValue;
		if (MotionReadVariantReal(value, TJS_W("y"), realValue)) coordY = realValue;
		if (MotionReadVariantReal(value, TJS_W("left"), realValue)) left = realValue;
		if (MotionReadVariantReal(value, TJS_W("top"), realValue)) top = realValue;
		if (MotionReadVariantReal(value, TJS_W("width"), realValue)) width = realValue;
		if (MotionReadVariantReal(value, TJS_W("height"), realValue)) height = realValue;
		if (MotionReadVariantReal(value, TJS_W("scale"), realValue)) setScale(realValue);
		if (MotionReadVariantReal(value, TJS_W("zoomX"), realValue)) scaleX = realValue;
		if (MotionReadVariantReal(value, TJS_W("zoomY"), realValue)) scaleY = realValue;
		if (MotionReadVariantReal(value, TJS_W("angle"), realValue)) rotation = realValue;
		if (MotionReadVariantReal(value, TJS_W("angleDeg"), realValue)) rotation = realValue;
		tjs_int intValue = 0;
		if (MotionReadVariantInt(value, TJS_W("opacity"), intValue)) opacity = intValue;
		bool boolValue = false;
		if (MotionReadVariantBool(value, TJS_W("visible"), boolValue)) visible = boolValue;
	}

	void updateEffectState(tjs_real msTime)
	{
		if (windState.active) {
			windState.progress += std::max<tjs_real>(0.0, windState.speed) * (msTime / 1000.0);
			if (windState.progress >= 1.0) {
				windState.progress = 1.0;
				windState.active = false;
			}
			tjs_real wave = 0.5 + 0.5 * std::sin(windState.progress * (tjs_real)M_PI);
			wind = windState.powerMin + (windState.powerMax - windState.powerMin) * wave;
		}

		for (auto it = activeTimelines.begin(); it != activeTimelines.end(); ++it) {
			MotionTimelineRuntimeState &state = it->second;
			if (!state.active) continue;
			if (state.blendTime > 0.0) {
				tjs_real elapsed = tickCount - state.blendStartedTick;
				tjs_real ratio = applyEasingRatio(elapsed / state.blendTime, state.easing);
				state.blendRatio = state.blendStartRatio +
					(state.blendTargetRatio - state.blendStartRatio) * ratio;
				if (elapsed >= state.blendTime) {
					state.blendRatio = state.blendTargetRatio;
					state.blendTime = 0.0;
				}
			} else {
				state.blendRatio = state.blendTargetRatio;
			}
			state.blendRatio = std::max<tjs_real>(0.0, std::min<tjs_real>(1.0, state.blendRatio));
			if (state.stopWhenBlendDone && state.blendTime <= 0.0 && state.blendRatio <= 0.0001) {
				state.active = false;
			}
		}
		pruneInactiveTimelines();
	}

	void pruneInactiveTimelines()
	{
		for (auto it = activeTimelines.begin(); it != activeTimelines.end();) {
			if (!it->second.active) it = activeTimelines.erase(it);
			else ++it;
		}
		if (!playing && activeTimelines.empty()) animating = false;
	}

	bool containsPoint(tjs_real x, tjs_real y) const
	{
		if (!visible || opacity <= 0) return false;
		std::shared_ptr<MotionResource> resource = resolveResource();
		tjs_real hitWidth = width > 0 ? width : (resource && resource->width > 0 ? resource->width : 1280.0);
		tjs_real hitHeight = height > 0 ? height : (resource && resource->height > 0 ? resource->height : 720.0);
		return x >= 0 && y >= 0 && x < hitWidth && y < hitHeight;
	}

	ResourceManager *resourceManager = nullptr;
	std::shared_ptr<MotionResource> currentResource;
	const MotionInfo *currentMotionInfo = nullptr;
	bool animating = false;
	bool playing = false;
	bool allplaying = false;
	tjs_int loopTime = 0;
	tjs_real tickCount = 0.0;
		tjs_real speed = 20.0;
		tTJSString motionKey;
		tTJSString motion;
		tTJSString chara;
		bool charaExplicit = false;
		tjs_real coordX = 0.0;
	tjs_real coordY = 0.0;
	tjs_real scaleX = 1.0;
	tjs_real scaleY = 1.0;
	tjs_real timeScale = 1.0;
	tjs_real rotation = 0.0;
	tjs_uint32 drawColor = 0xffffffff;
	tjs_real left = 0.0;
	tjs_real top = 0.0;
	tjs_real width = 0.0;
	tjs_real height = 0.0;
	bool visible = true;
	bool paused = false;
	bool loading = false;
	bool smoothing = true;
		tjs_int opacity = 255;
		tjs_real bustScale = 1.0;
		tjs_real hairScale = 1.0;
		tjs_real partsScale = 1.0;
		tjs_real wind = 0.0;
		tjs_real meshDivisionRatio = 1.0;
	tjs_real physicsTimestep = 0.0;
	tjs_real physicsGravityX = 0.0;
	tjs_real physicsGravityY = 0.0;
	tjs_real physicsWindX = 0.0;
	tjs_real physicsWindY = 0.0;
		tjs_int currentFrame = 0;
		tjs_uint64 variableRevision = 0;
		tjs_uint64 lastCommandVariableRevision = 0;
		tTJSVariant transform;
		tTJSVariant motionVariant;
		tTJSVariant tags;
		tTJSVariant physicsMetadata;
		tTJSVariant completionType;
		ttstr pendingLayerGetterName;
	std::map<std::string, tjs_real> variableValues;
	std::map<std::string, std::pair<tjs_real, tjs_real>> manualVariableRanges;
	std::map<std::string, MotionTimelineRuntimeState> activeTimelines;
	std::map<std::string, MotionOuterForceState> outerForces;
	MotionWindState windState;
	MotionResource runtimeLayoutResource;
	std::shared_ptr<MotionResource> runtimeLayoutSource;
	bool runtimeLayoutResourceValid = false;
	ttstr runtimeLayoutObject;
	ttstr runtimeLayoutMotion;
	tjs_real runtimeLayoutTick = -1.0;
	std::map<std::string, tjs_real> runtimeLayoutVariables;
		bool playWasCalled = false;
			bool stopCommandSent = true;
			tjs_real progressWallTimeMs = 0.0;
			bool drawRuntimeResourceBuilt = false;
			tjs_int drawRuntimeLayoutCount = 0;
			tjs_int drawRuntimeFrontDrawItemCount = 0;
			tjs_int drawRuntimeFrontHitShapeCount = 0;
			tjs_int drawRuntimeRenderMethodItemCount = 0;
			MotionResource drawRuntimeDiagnosticResource;
			bool drawRuntimeDiagnosticResourceValid = false;
			ttstr drawRuntimeSource;
			ttstr drawRuntimeObject;
			ttstr drawRuntimeMotion;
			tjs_real drawRuntimeTick = 0.0;
			tjs_real drawRuntimeLimitOriginX = 0.0;
			tjs_real drawRuntimeLimitOriginY = 0.0;
			tjs_real drawRuntimeLimitWidth = 0.0;
			tjs_real drawRuntimeLimitHeight = 0.0;
			tjs_real drawRuntimeLimitZMax = kMotionDefaultLimitZMax;
			ttstr lastDrawPath;
			iTJSDispatch2 *lastDrawTarget = nullptr;
		ttstr lastDrawResult;
		bool lastDrawAttempted = false;
		bool lastDrawHadTarget = false;
		bool lastDrawCacheHit = false;
		bool lastDrawLoadedLayerImage = false;
		bool lastDrawUsedFallback = false;
		tjs_int drawInvocationCount = 0;
		tjs_int drawActualCount = 0;
		bool drawTimeConsumptionValid = false;
		tjs_int drawTimeConsumedDrawItemCount = 0;
		tjs_int drawTimeConsumedVertexCount = 0;
		tjs_int drawTimeConsumedIndexCount = 0;
		tjs_int drawTimeConsumedCellCount = 0;
		tjs_int drawTimePlanSourceCount = 0;
		bool drawTimeOffsetContiguousOk = true;
		bool drawTimePerItemCountOk = true;
		bool drawTimePerItemOrderOk = true;
		bool drawTimePerItemSliceOk = true;
		bool drawTimeContractMatchesPlan = true;
		ttstr drawTimePlanHash;
		ttstr drawTimeFirstItemHash;
		ttstr drawTimeLastItemHash;
		tjs_int drawTimeFirstItemOrder = 0;
		tjs_int drawTimeLastItemOrder = 0;
		tjs_int drawTimeFirstItemVertexOffset = 0;
		tjs_int drawTimeLastItemVertexOffset = 0;
		tjs_int drawTimeFirstItemIndexOffset = 0;
		tjs_int drawTimeLastItemIndexOffset = 0;
		tjs_int drawTimeFirstItemVertexCount = 0;
		tjs_int drawTimeLastItemVertexCount = 0;
		tjs_int drawTimeFirstItemIndexCount = 0;
		tjs_int drawTimeLastItemIndexCount = 0;
		tjs_int drawTimeFirstItemCellCount = 0;
		tjs_int drawTimeLastItemCellCount = 0;
		tjs_int drawTimeFirstItemFirstCellIndex = 0;
		tjs_int drawTimeLastItemFirstCellIndex = 0;
		std::vector<MotionRendererCommandEntryInfo> rendererCommandEntries;
		bool rendererCommandBuilt = false;
		tjs_uint64 rendererCommandHash = 0;
		bool rendererCommandOffsetsContiguous = true;
		bool rendererCommandPerEntryOk = true;
		MotionRendererCommandConsumerSummary rendererConsumerSummary;
		MotionRendererUploadPrepSummary uploadPrepSummary;
		MotionStagingArtifact stagingArtifact;
		MotionShadowDrawDiagnostic shadowDrawDiagnostic;
		MotionOffscreenDrawDiagnostic offscreenDrawDiagnostic;
		MotionDiagnosticRenderBridge diagnosticRenderBridge;
		MotionVisibleRenderBridge visibleRenderBridge;
		tjs_int lastDrawFrame = 0;
		tjs_real lastDrawTick = 0.0;
		tjs_real affineA = 1.0;
	tjs_real affineB = 0.0;
	tjs_real affineC = 0.0;
	tjs_real affineD = 1.0;
	tjs_int affineTx = 0;
	tjs_int affineTy = 0;
	tjs_int cameraWidth = 0;
	tjs_int cameraHeight = 0;
	};

class Player : public EmotePlayer {
public:
	Player() = default;
	Player(ResourceManager *resourceManager) : EmotePlayer(resourceManager) {}
	static tjs_error TJS_INTF_METHOD playRaw(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, Player *nativeInstance)
	{
		return EmotePlayer::playRaw(result, numparams, param, nativeInstance);
	}
	static tjs_error TJS_INTF_METHOD containsRaw(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, Player *nativeInstance)
	{
		return EmotePlayer::containsRaw(result, numparams, param, nativeInstance);
	}
	static tjs_error TJS_INTF_METHOD getLayerGetterRaw(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
	{
		if (!objthis) {
			if (result) result->Clear();
			return TJS_S_OK;
		}
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;
		Player *nativeInstance =
			ncbInstanceAdaptor<Player>::GetNativeInstance(objthis, false);
		return EmotePlayer::getLayerGetterRawImpl(
			result, numparams, param, objthis, nativeInstance);
	}
	static tjs_error TJS_INTF_METHOD getLayerMotionRaw(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
	{
		if (!objthis) {
			if (result) result->Clear();
			return TJS_S_OK;
		}
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;
		tTJSVariant getter;
		tjs_error hr = getLayerGetterRaw(&getter, numparams, param, objthis);
		if (TJS_FAILED(hr)) {
			if (result) result->Clear();
			return TJS_S_OK;
		}
		ExtractLayerGetterMotion(getter, result);
		return TJS_S_OK;
	}
	static tjs_error TJS_INTF_METHOD playTimelineRaw(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, Player *nativeInstance)
	{
		return EmotePlayer::playTimelineRaw(result, numparams, param, nativeInstance);
	}
	static tjs_error TJS_INTF_METHOD stopTimelineRaw(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, Player *nativeInstance)
	{
		return EmotePlayer::stopTimelineRaw(result, numparams, param, nativeInstance);
	}
	static tjs_error TJS_INTF_METHOD getTimelinePlayingRaw(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, Player *nativeInstance)
	{
		return EmotePlayer::getTimelinePlayingRaw(result, numparams, param, nativeInstance);
	}
	static tjs_error TJS_INTF_METHOD setTimelineBlendRatioRaw(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, Player *nativeInstance)
	{
		return EmotePlayer::setTimelineBlendRatioRaw(result, numparams, param, nativeInstance);
	}
	static tjs_error TJS_INTF_METHOD fadeInTimelineRaw(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, Player *nativeInstance)
	{
		return EmotePlayer::fadeInTimelineRaw(result, numparams, param, nativeInstance);
	}
	static tjs_error TJS_INTF_METHOD fadeOutTimelineRaw(tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, Player *nativeInstance)
	{
		return EmotePlayer::fadeOutTimelineRaw(result, numparams, param, nativeInstance);
	}
};
