# EmotePlayer Split Map

`emoteplayer_stub.cpp` remains the ncbind/plugin registration translation unit
for the WASM Emote runtime.  Most runtime code is still split into ordered
`.inc` slices for navigation and review, while the lowest-risk leaf utilities
have started moving into normal `.h/.cpp` files.

## Include Order

Keep the order in `emoteplayer_stub.cpp` dependency-driven:

1. `motion_types.h` - shared POD structs, constants, diagnostics structs,
   render surface types, `MotionResource`, and `MotionFrameLayoutState`.
2. `motion_math.h/.cpp` - pure matrix, affine, Bezier, surface-chain, and
   layout-geometry helpers.  This is intentionally free of TJS/ncbind calls.
3. `motion_trace_bootstrap.inc` - plugin bootstrap, renderer backend switch,
   Emscripten log/export helpers, click burst tracing, M4 trace setup, and logo
   fallback helpers.
4. `motion_psb_parse.inc` - storage byte reads, PSB primitive helpers,
   metadata/object/motion/timeline/selector parsing, and shared string/path
   helpers.
5. `motion_layout_runtime.inc` - frame/layout selection, nested variable
   resolution, layout bounds, renderMethodIR construction, runtime draw resource
   building, and `ParseMotionResource`.
6. `motion_tjs_core.inc` - TJS call/property helpers, layer drawable guard,
   runtime draw target sizing, variant object/array helpers, and resource
   registry primitives.
7. `motion_front_collect.inc` - front draw/hit candidate collection, object
   state grouping, runtime draw layout selection, and renderer prep summary
   calculation.
8. `motion_renderer.inc` - renderer command planning/consumption, upload
   staging, shadow/offscreen diagnostics, M2-M5/mesh self-tests, and visible
   CPU raster bridge.
9. `motion_tjs_dump.inc` - TJS diagnostic dump builders, texture snapshot
   warming, direct layer image loading, enum shims, and resource dump helpers.
10. `emoteplayer_resource_runtime.inc` - `Motion`, `ResourceManager`, and
   `SeparateLayerAdaptor`.
11. `motion_front_hit.inc` - `MotionHitShape`, layout/name hit helpers,
    front resolve dumps, layer getter construction, and adaptor target
    resolution.
12. `emoteplayer_runtime.inc` - `EmotePlayer` and `Player` runtime behavior.
13. `emoteplayer_web_bridge.inc` - exported web bridge functions, included
    after leaving the namespace so the C exports can call namespace helpers.
14. `emoteplayer_registration.inc` - ncbind registration and plugin
    init/done hooks.

## Rules

- Do not include an `.inc` file from any source other than
  `emoteplayer_stub.cpp`.
- Only move leaf-like code into `.h/.cpp` when the declaration boundary is
  explicit and the source does not depend on TJS/ncbind runtime state.
- Preserve static linkage and include order unless a build proves the new
  declaration boundary is complete.
- Keep behavior changes out of structural split commits; split first, change
  runtime semantics in later focused cuts.

## Next Extraction Candidates

The first leaf extraction is done: `motion_types.h` and `motion_math.h/.cpp`.
Next low-risk candidates are small pure helpers inside PSB parsing.  Leave
`motion_renderer.inc` and `emoteplayer_runtime.inc` for later because they still
carry the widest runtime, diagnostic, and ncbind coupling.
