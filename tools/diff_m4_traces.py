#!/usr/bin/env python3
"""Diff krkrsdl2 M4 trace JSONL against krkrsdl3 expected JSON.

Match key: (motion, tick, src, uv). src is language-agnostic icon path so
this avoids label encoding differences between the two traces.

Usage:
    python3 tools/diff_m4_traces.py \\
        /tmp/krkrsdl3_expected.json \\
        /tmp/krkrsdl2_trace.jsonl

Output:
    Stats summary, then per-tick diff list with first divergence per (motion,
    tick, src, uv). Tolerance for matrix elements / NDC / pixel: configurable.

Exit code 0 if all matched samples agree within tolerance; non-zero otherwise.
"""
import json
import sys
from collections import defaultdict


MAT_TOL = 1e-3
NDC_TOL = 1e-3


def load_expected(path):
    with open(path) as f:
        data = json.load(f)
    out = {}
    for s in data['samples']:
        key = (
            s['motion'],
            round(float(s['tick']), 6),
            s['src'],
            round(float(s['uv'][0]), 4),
            round(float(s['uv'][1]), 4),
        )
        out[key] = s
    return out


def load_actual(path):
    out = {}
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            try:
                s = json.loads(line)
            except Exception as e:
                continue
            key = (
                s['motion'],
                round(float(s['tick']), 6),
                s['src'],
                round(float(s['uv'][0]), 4),
                round(float(s['uv'][1]), 4),
            )
            out[key] = s
    return out


def vec_diff(a, b, tol):
    if a is None or b is None:
        return None if (a is None and b is None) else 'one-side-missing'
    if len(a) != len(b):
        return f'len {len(a)} vs {len(b)}'
    bad = []
    for i in range(len(a)):
        if abs(float(a[i]) - float(b[i])) > tol:
            bad.append((i, float(a[i]), float(b[i])))
    if not bad:
        return None
    return bad


def fmt_floats(v, n=8):
    return '[' + ', '.join(f'{x:+.4f}' for x in v[:n]) + (', ...' if len(v) > n else '') + ']'


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)
    expected_path = sys.argv[1]
    actual_path = sys.argv[2]
    expected = load_expected(expected_path)
    actual = load_actual(actual_path)

    common = set(expected.keys()) & set(actual.keys())
    only_exp = set(expected.keys()) - set(actual.keys())
    only_act = set(actual.keys()) - set(expected.keys())

    print(f'expected samples: {len(expected)}')
    print(f'actual   samples: {len(actual)}')
    print(f'common   samples: {len(common)}')
    print(f'only in expected: {len(only_exp)}')
    print(f'only in actual:   {len(only_act)}')

    # Group by (motion, tick) for readable output
    by_mt = defaultdict(list)
    for k in common:
        by_mt[(k[0], k[1])].append(k)

    pass_count = 0
    fail_count = 0
    failures_by_class = defaultdict(int)
    sample_failures = []

    for (motion, tick), keys in sorted(by_mt.items()):
        for k in keys:
            exp = expected[k]
            act = actual[k]
            local_fail = []

            # 1. surfaceCount
            if exp['surfaceCount'] != act['surfaceCount']:
                local_fail.append(
                    ('surfaceCount',
                     exp['surfaceCount'], act['surfaceCount']))

            # 2. matTrans element-by-element per surface
            exp_surfaces = exp.get('drawSurfaces', [])
            act_surfaces = act.get('surfaces', [])
            n = min(len(exp_surfaces), len(act_surfaces))
            for i in range(n):
                e = exp_surfaces[i]
                a = act_surfaces[i]
                d = vec_diff(e['matTransRowMajor'], a['matTransRowMajor'],
                             MAT_TOL)
                if d is not None:
                    local_fail.append(
                        (f'surface[{i}].matTrans',
                         e['matTransRowMajor'], a['matTransRowMajor'], d))

            # 3. final glPosition
            if exp.get('final') and act.get('final'):
                d = vec_diff(exp['final']['glPosition'],
                             act['final']['glPosition'], NDC_TOL)
                if d is not None:
                    local_fail.append(
                        ('glPosition',
                         exp['final']['glPosition'],
                         act['final']['glPosition'], d))

            if local_fail:
                fail_count += 1
                for f in local_fail:
                    failures_by_class[f[0]] += 1
                if len(sample_failures) < 5:
                    sample_failures.append((k, local_fail))
            else:
                pass_count += 1

    print(f'\nMatched: {pass_count} pass, {fail_count} fail')
    if failures_by_class:
        print('\nFailure breakdown:')
        for cls, n in sorted(failures_by_class.items(), key=lambda x: -x[1]):
            print(f'  {cls}: {n}')

    if sample_failures:
        print('\nFirst 5 failure samples:')
        for k, fails in sample_failures:
            (motion, tick, src, u, v) = k
            print(f'\n  motion={motion} tick={tick} src={src} uv=({u},{v})')
            for f in fails:
                if len(f) == 3:
                    name, e, a = f
                    print(f'    {name}: expected={e!r} actual={a!r}')
                else:
                    name, e, a, d = f
                    if isinstance(d, list):
                        print(f'    {name}: {len(d)} elements differ; first 3:')
                        for idx, ev, av in d[:3]:
                            print(f'      [{idx}] expected={ev:+.6f} actual={av:+.6f} delta={av-ev:+.6f}')
                        print(f'    expected: {fmt_floats(e, 16)}')
                        print(f'    actual:   {fmt_floats(a, 16)}')
                    else:
                        print(f'    {name}: {d}')

    if only_exp:
        print(f'\nMissing in actual ({len(only_exp)}):')
        for k in sorted(only_exp)[:10]:
            print(f'  {k}')

    sys.exit(0 if fail_count == 0 and pass_count > 0 else 1)


if __name__ == '__main__':
    main()
