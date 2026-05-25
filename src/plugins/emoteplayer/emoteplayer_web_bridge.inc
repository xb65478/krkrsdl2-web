#ifdef __EMSCRIPTEN__
bool TVPEmotePlayerShowPsbLogoFallback(const ttstr &storage, const ttstr &motion)
{
	if (!emoteplayer::TVPIsDracuBringupEnabled()) {
		emoteplayer::LogDracuMotionHitHack("disabled", "logo-fallback", storage);
		return false;
	}
	emoteplayer::LogDracuMotionHitHack("enabled", "logo-fallback", storage);
	return emoteplayer::ShowPsbLogoFallback(storage, motion);
}

void TVPEmotePlayerHidePsbLogoFallback()
{
	if (!emoteplayer::TVPIsDracuBringupEnabled()) return;
	emoteplayer::HidePsbLogoFallback();
}

void TVPArmMotionClickBurstTrace(const char *reason, double x, double y)
{
	emoteplayer::MotionArmClickBurstTrace(reason, x, y);
}

void TVPArmMotionHostBurstTrace(const char *reason)
{
	emoteplayer::MotionArmClickBurstTrace(reason, -1.0, -1.0);
}

void TVPLogMotionHostBurstTrace(const char *tag, double a, double b, double c, double d)
{
	if (!emoteplayer::MotionClickBurstTraceActive()) return;
	std::fprintf(stderr,
		"[MOTION-HOST-BURST] gen=%d rem=%d tag=%s a=%.3f b=%.3f c=%.3f d=%.3f\n",
		(int)emoteplayer::g_motionClickBurstTrace.generation,
		(int)emoteplayer::g_motionClickBurstTrace.remaining,
		tag ? tag : "-",
		a, b, c, d);
	std::fflush(stderr);
}
#endif
