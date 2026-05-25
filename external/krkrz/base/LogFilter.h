// Runtime log verbosity filter for krkrsdl2 WASM build.
// L1 (default): essential events only (plugin load, errors, scene key beats, storage)
// L2: L1 + KAG tag stream + motion progress + TJS high-level calls
// L3: L1+L2 + per-frame pixel/font/bitmap/hit-test diagnostic firehose
//
// Activated via command-line arg: -krkr-loglevel=N (N in {1,2,3}). Default is 1.
// index.html forwards URL param ?loglevel=N to the WASM main() as this arg.
// Perf counters are separately activated with -krkr-perf=1.
//
// Usage in .cpp files:
//   if (TVPLogL2()) fprintf(stderr, "[KENV] ...\n");
//   if (TVPLogL3()) fprintf(stderr, "[NBC] ...\n");
// Keep L1 logs unguarded (they should stay always-on).
#ifndef KRKR_LOG_FILTER_H
#define KRKR_LOG_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

extern int g_krkr_log_level;
extern int g_krkr_perf_enabled;

static inline int TVPLogL2(void) { return g_krkr_log_level >= 2; }
static inline int TVPLogL3(void) { return g_krkr_log_level >= 3; }
static inline int TVPPerfEnabled(void) { return g_krkr_perf_enabled != 0; }

// Convenience macros: emit fprintf(stderr, ...) only when the required level is active.
// Use these for high-volume diagnostic prints that are safe to drop at L1 (default).
//   KRKR_LOG_L2(fmt, ...)  -> L2+L3 (KAG/motion streams)
//   KRKR_LOG_L3(fmt, ...)  -> L3 only (per-frame firehose)
#define KRKR_LOG_L2(...) do { if (TVPLogL2()) { fprintf(stderr, __VA_ARGS__); } } while (0)
#define KRKR_LOG_L3(...) do { if (TVPLogL3()) { fprintf(stderr, __VA_ARGS__); } } while (0)
#define KRKR_PERF_LOG(...) do { if (TVPPerfEnabled()) { fprintf(stderr, __VA_ARGS__); } } while (0)

// Initialized by SDLApplication once argv is available. If you need to call it
// earlier (before main() is reached, e.g. from static init), it's safe to read
// g_krkr_log_level (default 1) without initialization.
void TVPLogFilterInitFromArgs(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif
