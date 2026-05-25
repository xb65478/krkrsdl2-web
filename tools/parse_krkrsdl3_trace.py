#!/usr/bin/env python3
"""Parse krkrsdl3 motion trace log into structured ground-truth records.

Usage:
    python3 tools/parse_krkrsdl3_trace.py \\
        /Users/xiabin/my_work1/web-krkr/logo_motion_trace.m4-values-20260509135316.log \\
        /tmp/krkrsdl3_expected.json

The log is line-oriented; each line is `key="value" key="value" ...`. We extract
the sub-set of actions that matter for M4 verification:

  - emoteplayer::ResourceManager::load   -> motion currently being prepared
  - emoteplayer::EmotePlayer::play       -> player + chara/motion binding
  - emoteplayer::emotenode::emotenode    -> tree topology (node ptr -> parent ptr,
                                            label, type, depth)
  - emoteplayer::emotenode::progress     -> per-tick interpolated-state,
                                            push-render-method, child-progress,
                                            ref-motion-progress
  - emoteplayer::emotenode::checkDrawStatus -> frame-selected
  - emoteplayer::emotenode::draw         -> draw-surface, m4-sample-*

Output JSON has the following shape:

    {
      "motions": [...],                # ordered list of motions seen in load order
      "nodes": {                       # tree topology, indexed by node ptr (hex)
         "<ptr>": {
            "ptr": "<ptr>",
            "label": "...",
            "type": 0,
            "parent": "<ptr or 'root'>",
            "labelChain": "label0/label1/leafLabel",
            "depthInTree": int,
            "frames": [{"index": 0, "time": 0.0, ...}],
         }, ...
      },
      "samples": [                     # one record per (motion, tick, nodePtr, uv)
         {
           "motion": "yuzulogo",
           "tick": 0.0,
           "nodePtr": "...",
           "nodeLabel": "white",
           "labelChain": "slide/yuzu_mi/.../white",
           "uv": [0.0, 0.0],
           "surfaceCount": 1,
           "lim": [0.0, 0.0, 1280.0, 720.0, 30.0],
           "interpolated": {coord, angle, scale, skew, originOffset, opacity, ...},
           "drawSurfaces": [          # one per surfaceIndex
              {
                 "surfaceIndex": 0,
                 "renderType": 2,
                 "size": [479.0, 277.0],
                 "origin": [239.0, 138.0],
                 "matTransRowMajor": [16 floats],   # row-major: m00,m01,m02,m03;m10,...
                 "matTransGlmValuePtr": [16 floats],# column-major (glm)
                 "controlPts": [16 vec2 = 32 floats],
                 "controlSource": "default_control_points",
              }, ...
           ],
           "steps": [                 # one per surfaceIndex per uv
              {
                 "surfaceIndex": 0,
                 "renderIndex": 0,
                 "renderType": 2,
                 "lastPtBeforeRemap": [4 floats],
                 "surfaceUv": [2 floats],
                 "bezierPosition": [2 floats],
                 "lastPtAfter": [4 floats],
                 "controlSource": "default_control_points",
              }, ...
           ],
           "final": {
              "finalLastPt": [4 floats],
              "glPosition": [4 floats],
              "tessCoord": [2 floats],
              "totalOpa": float,
           },
         }, ...
      ],
    }
"""
import json
import re
import sys
from pathlib import Path
from typing import Dict, List, Optional, Tuple


_KV_PATTERN = re.compile(r'(\w+)="([^"]*)"')


def parse_kv(line: str) -> Dict[str, str]:
    return dict(_KV_PATTERN.findall(line))


def parse_floats(s: str) -> List[float]:
    if not s:
        return []
    return [float(x) for x in s.replace(';', ',').split(',') if x.strip()]


def normalize_label(raw: str) -> str:
    # krkrsdl3 logs labels with explicit \0; strip trailing \0\0...
    return raw.rstrip('\\0').rstrip('\x00')


def make_label_chain(node_ptr: str, nodes: Dict[str, dict]) -> str:
    chain = []
    cur = node_ptr
    seen = set()
    while cur and cur != "0000000000000000" and cur in nodes and cur not in seen:
        seen.add(cur)
        chain.append(nodes[cur].get("label", "?"))
        cur = nodes[cur].get("parent", "")
        if cur == "root":
            break
    return "/".join(reversed(chain))


def parse_log(path: Path) -> dict:
    nodes: Dict[str, dict] = {}        # ptr -> dict
    motions_seen: List[str] = []
    motion_ptr_to_name: Dict[str, str] = {}  # motion ptr -> motion name
    file_ptr_to_motion: Dict[str, str] = {}  # file ptr -> last loaded motion name

    # Stack-based context
    progress_stack: List[Tuple[str, float]] = []  # (nodePtr, tick) for emotenode::progress
    current_motion: Optional[str] = None
    current_lim: List[float] = []
    current_player_motion: Dict[str, str] = {}  # player ptr -> motion name

    # Tracks which motion a tick belongs to. We use per-progress stack:
    # when EmotePlayer::progress enter for player P, we know P's motion name; push.
    player_progress_stack: List[str] = []  # list of motion names

    # Pending sample being assembled
    sample: Optional[dict] = None
    samples: List[dict] = []
    interpolated_by_node_tick: Dict[Tuple[str, float], dict] = {}
    # draw-surface records buffered for later attach to samples;
    # key = (motion, tick, nodePtr, surfaceIndex)
    draw_surfaces_buf: Dict[Tuple[str, float, str, int], dict] = {}

    # Track "current draw" context: emotenode::draw enter sets node ptr; later
    # actions m4-sample-* belong to it. tick is in the action itself.
    draw_node_ctx: Optional[str] = None

    with path.open(encoding='utf-8', errors='replace') as f:
        for raw in f:
            line = raw.strip()
            if not line or line.startswith('#'):
                continue
            kv = parse_kv(line)
            event = kv.get('event', '')
            func = kv.get('function', '')
            action = kv.get('action', '')
            depth = int(kv.get('depth', '0') or 0)

            # ---- ResourceManager::load: bind path -> motion name (by file ptr) ----
            if func == "emoteplayer::ResourceManager::load" and event == "enter":
                req = kv.get('requestedPath', '')
                m = re.search(r'(\w+)\.psb', req)
                if m:
                    name = m.group(1)
                    motion_ptr_to_name[''] = name  # placeholder

            # ---- EmotePlayer::play: bind player ptr -> motion name ----
            if func == "emoteplayer::EmotePlayer::play" and action == "play-selected":
                player = kv.get('player', '')
                motion_name = kv.get('motion', '')
                if player:
                    current_player_motion[player] = motion_name

            # ---- EmotePlayer::progress: push motion name for tick context ----
            if func == "emoteplayer::EmotePlayer::progress":
                if event == "enter":
                    player = kv.get('player', '')
                    motion_name = (kv.get('motion', '')
                                   or current_player_motion.get(player, ''))
                    player_progress_stack.append(motion_name)
                    if motion_name and motion_name not in motions_seen:
                        motions_seen.append(motion_name)
                elif event == "return":
                    if player_progress_stack:
                        player_progress_stack.pop()

            current_motion = (player_progress_stack[-1]
                              if player_progress_stack else None)

            # ---- emotenode::emotenode: tree topology ----
            if func == "emoteplayer::emotenode::emotenode":
                if event == "enter":
                    parent = kv.get('parent', '')
                    # Stash for next node-header
                    pending_parent = parent
                    pending_depth = depth
                    # We rely on the immediately-following node-header for label
                    # so save into a stash keyed by depth; subsequent node-header
                    # at same line tier consumes it.
                    nodes['__pending__'] = {
                        'parent': parent if parent != "0000000000000000" else "root",
                        'depth': depth,
                    }
                elif action == "node-header":
                    np = kv.get('node', '')
                    if np:
                        pending = nodes.pop('__pending__', None)
                        node = {
                            'ptr': np,
                            'label': normalize_label(kv.get('label', '?')),
                            'type': int(kv.get('type', '0') or 0),
                            'parent': (pending['parent']
                                       if pending else "root"),
                            'depth': (pending['depth']
                                      if pending else 0),
                            'meshDivision': int(kv.get('meshDivision', '0') or 0),
                            'frames': [],
                        }
                        nodes[np] = node
                elif action == "frame":
                    np = kv.get('node', '')
                    if np in nodes:
                        nodes[np]['frames'].append({
                            'index': int(kv.get('frameIndex', '0') or 0),
                            'time': float(kv.get('time', '0') or 0),
                            'type': int(kv.get('type', '0') or 0),
                            'hasContent': int(kv.get('hasContent', '0') or 0),
                            'src': normalize_label(kv.get('src', '')),
                            'coord': parse_floats(kv.get('coord', '')),
                            'angle': float(kv.get('angle', '0') or 0),
                            'scale': parse_floats(kv.get('scale', '')),
                            'skew': parse_floats(kv.get('skew', '')),
                            'originOffset': parse_floats(
                                kv.get('originOffset', '')),
                            'opacity': float(kv.get('opacity', '0') or 0),
                            'bm': int(kv.get('bm', '0') or 0),
                            'mask': int(kv.get('mask', '0') or 0),
                            'timeOffset': float(kv.get('timeOffset', '0') or 0),
                            'hasbp': int(kv.get('hasbp', '0') or 0),
                        })

            # ---- progress lim tracking ----
            if (func == "emoteplayer::emotemotion::progress"
                    and event == "enter"):
                lim_str = kv.get('lim', '')
                if lim_str:
                    current_lim = parse_floats(lim_str)

            # ---- interpolated-state ----
            if (func == "emoteplayer::emotenode::progress"
                    and action == "interpolated-state"):
                np = kv.get('node', '')
                tick = float(kv.get('tick', '0') or 0)
                interpolated_by_node_tick[(np, tick)] = {
                    'src': normalize_label(kv.get('src', '')),
                    'coord': parse_floats(kv.get('coord', '')),
                    'angle': float(kv.get('angle', '0') or 0),
                    'scale': parse_floats(kv.get('scale', '')),
                    'skew': parse_floats(kv.get('skew', '')),
                    'originOffset': parse_floats(
                        kv.get('originOffset', '')),
                    'opacity': float(kv.get('opacity', '0') or 0),
                    'timeOffset': float(kv.get('timeOffset', '0') or 0),
                    'isNeedBp': int(kv.get('isNeedBp', '0') or 0),
                    'isLayout': int(kv.get('isLayout', '0') or 0),
                    'isIcon': int(kv.get('isIcon', '0') or 0),
                }

            # ---- m4 sample: assemble per-vertex records ----
            if (func == "emoteplayer::emotenode::draw"
                    and action == "m4-sample-begin"):
                np = kv.get('node', '')
                tick = float(kv.get('tick', '0') or 0)
                uv = parse_floats(kv.get('uv', ''))
                lim = parse_floats(kv.get('lim', '')) or current_lim
                sample = {
                    'motion': current_motion or '?',
                    'tick': tick,
                    'nodePtr': np,
                    'nodeLabel': normalize_label(kv.get('label', '?')),
                    'src': normalize_label(kv.get('src', '')),
                    'uv': uv,
                    'surfaceCount': int(kv.get('surfaceCount', '0') or 0),
                    'lim': lim,
                    'interpolated': interpolated_by_node_tick.get(
                        (np, tick), {}),
                    'drawSurfaces': [],
                    'steps': [],
                    'final': None,
                }
                # find ALL push-render-method records for this node at this tick
                # earlier in stream (we already saw them) — but since we don't
                # currently store them keyed, attach later in second pass.
                samples.append(sample)
                continue

            if (func == "emoteplayer::emotenode::draw"
                    and action == "m4-sample-step" and sample is not None):
                step = {
                    'surfaceIndex': int(kv.get('surfaceIndex', '0') or 0),
                    'renderIndex': int(kv.get('renderIndex', '0') or 0),
                    'renderType': int(kv.get('renderType', '0') or 0),
                    'surfaceLabel': normalize_label(
                        kv.get('surfaceLabel', '')),
                    'lastPtBeforeRemap': parse_floats(
                        kv.get('lastPtBeforeRemap', '')),
                    'surfaceUv': parse_floats(kv.get('surfaceUv', '')),
                    'bezierPosition': parse_floats(
                        kv.get('bezierPosition', '')),
                    'lastPtAfter': parse_floats(
                        kv.get('lastPtAfter', '')),
                    'controlSource': kv.get('controlSource', ''),
                }
                sample['steps'].append(step)
                continue

            if (func == "emoteplayer::emotenode::draw"
                    and action == "m4-sample-final" and sample is not None):
                sample['final'] = {
                    'finalLastPt': parse_floats(kv.get('finalLastPt', '')),
                    'glPosition': parse_floats(kv.get('glPosition', '')),
                    'tessCoord': parse_floats(kv.get('tessCoord', '')),
                    'totalOpa': float(kv.get('totalOpa', '0') or 0),
                }
                # Attach all draw-surface records for this (motion,tick,node)
                m = sample['motion']
                t = sample['tick']
                np_ = sample['nodePtr']
                attached = []
                for si in range(sample['surfaceCount']):
                    key = (m, t, np_, si)
                    ds = draw_surfaces_buf.get(key)
                    if ds is not None:
                        attached.append(ds)
                sample['drawSurfaces'] = attached
                sample = None  # close
                continue

            # ---- draw-surface: per-tick draw uniforms (multiple per node) ----
            if (func == "emoteplayer::emotenode::draw"
                    and action == "draw-surface"):
                np = kv.get('node', '')
                tick = float(kv.get('tick', '0') or 0)
                si = int(kv.get('surfaceIndex', '0') or 0)
                surface = {
                    'nodePtr': np,
                    'tick': tick,
                    'surfaceIndex': si,
                    'renderIndex': int(
                        kv.get('renderIndex', '0') or 0),
                    'renderType': int(
                        kv.get('renderType', '0') or 0),
                    'surfaceLabel': normalize_label(
                        kv.get('surfaceLabel', '')),
                    'opa': float(kv.get('opa', '0') or 0),
                    'totalOpaBeforeUpload': float(
                        kv.get('totalOpaBeforeUpload', '0') or 0),
                    'size': parse_floats(
                        kv.get('surfaceSize', '0x0').replace('x', ',')),
                    'origin': parse_floats(
                        kv.get('surfaceOrigin', '')),
                    'hasStencil': int(
                        kv.get('hasStencil', '0') or 0),
                    'matTransRowMajor': parse_floats(
                        kv.get('matTrans', '').replace(';', ',')),
                    'matTransGlmValuePtr': parse_floats(
                        kv.get('matTransValuePtr', '')),
                    'controlSource': kv.get('controlSource', ''),
                    'controlPts': parse_floats(
                        kv.get('controlPts', '').replace(';', ',')),
                }
                key = (current_motion or '?', tick, np, si)
                draw_surfaces_buf[key] = surface

            # ---- push-render-method: also track for diagnostic ----
            if (func == "emoteplayer::emotenode::progress"
                    and action == "push-render-method"):
                # Currently not consumed by samples but kept as a separate
                # array if needed.
                pass

    # Resolve label chains
    for np, n in list(nodes.items()):
        if np == '__pending__':
            continue
        n['labelChain'] = make_label_chain(np, nodes)

    # Inject labelChain into samples
    for s in samples:
        s['labelChain'] = nodes.get(s['nodePtr'], {}).get('labelChain', '')

    # Drop pending sentinel
    nodes.pop('__pending__', None)

    return {
        'motions': motions_seen,
        'nodes': nodes,
        'samples': samples,
    }


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)
    in_path = Path(sys.argv[1])
    out_path = Path(sys.argv[2])
    data = parse_log(in_path)

    with out_path.open('w', encoding='utf-8') as f:
        json.dump(data, f, ensure_ascii=False, indent=2)

    # Quick stats summary
    samples = data['samples']
    nodes = data['nodes']
    print(f"motions: {data['motions']}")
    print(f"nodes: {len(nodes)}")
    print(f"samples: {len(samples)}")
    multi_surface = sum(1 for s in samples if s['surfaceCount'] > 1)
    print(f"  surfaceCount>1: {multi_surface}")
    by_motion = {}
    for s in samples:
        by_motion.setdefault(s['motion'], 0)
        by_motion[s['motion']] += 1
    print(f"  by motion: {by_motion}")
    ticks = sorted({s['tick'] for s in samples})
    print(f"  ticks: {len(ticks)} unique, min={ticks[0] if ticks else 'NA'}, "
          f"max={ticks[-1] if ticks else 'NA'}")
    print(f"\nWrote {out_path}")


if __name__ == "__main__":
    main()
