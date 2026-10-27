#!/usr/bin/env python3
"""Build a standalone static preview from a source adapter manifest."""
import argparse, hashlib, json, os, shutil, subprocess
from pathlib import Path

SDK = Path(__file__).resolve().parent
EXPORTS = 'abi kind rate block init input output commands snapshot_buffer snapshot_size receive_snapshot overflow active parameter action envelope panic note_off note_on process frame'.split()

def validate(m):
    if m.get('schema') != 1 or m.get('kind') not in ('synth', 'effect'):
        raise ValueError('Schema 1 supports synth and effect adapters only')
    params = m['parameters']
    if len(params) > 64 or [p['id'] for p in params] != list(range(len(params))):
        raise ValueError('Parameter IDs must be contiguous from zero, maximum 64')
    for p in params:
        if not 0 <= p['default'] <= 1: raise ValueError('Invalid default')
    for preset in m.get('presets', []):
        if len(preset['values']) != len(params) or any(not 0 <= v <= 1 for v in preset['values']):
            raise ValueError('Invalid preset')
    for k in m['knobs']:
        for name in ('primary','secondary'):
            if k.get(name) is not None and not 0 <= k[name] < len(params): raise ValueError('Invalid knob')

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--plugin', type=Path, default=SDK.parent/'preview/plugin.json')
    ap.add_argument('--out', type=Path, default=SDK.parent/'site')
    ap.add_argument('--cc', default=os.environ.get('WASM_CC', 'clang'))
    a = ap.parse_args()
    manifest = a.plugin.resolve(); m = json.loads(manifest.read_text()); validate(m)
    base = manifest.parent; out = a.out.resolve()
    if out == base or out in base.parents: raise ValueError('Output must not overwrite source')
    out.mkdir(parents=True, exist_ok=True)
    sources = [SDK/'runtime/host.c', SDK/'runtime/memory.c'] + [(base/p).resolve() for p in m['build']['sources']]
    includes = [SDK/'include'] + [(base/p).resolve() for p in m['build']['includes']]
    cmd = [a.cc, '--target=wasm32', '-std=c11', '-O2', '-ffreestanding', '-fno-builtin', '-nostdlib', '-Wno-unknown-pragmas',
           '-Wl,--no-entry', '-Wl,--export-memory', '-Wl,--initial-memory=4194304', '-Wl,--max-memory=4194304', '-Wl,-z,stack-size=262144',
           '-DPREVIEW_KIND='+str(int(m['kind']=='effect'))]
    cmd += ['-D'+d for d in m['build']['defines']]
    cmd += ['-I'+str(p) for p in includes]
    cmd += ['-Wl,--export=pv_'+name for name in EXPORTS]
    cmd += [str(p) for p in sources] + ['-o', str(out/'engine.wasm')]
    subprocess.run(cmd, check=True)
    for p in (SDK/'web').iterdir():
        if p.is_file(): shutil.copy2(p,out/p.name)
    public = {k:v for k,v in m.items() if k != 'build'}
    public['abi'] = 1; public['sampleRate'] = 44100; public['blockSize'] = 128
    public['wasmSHA256'] = hashlib.sha256((out/'engine.wasm').read_bytes()).hexdigest()
    (out/'plugin.json').write_text(json.dumps(public, indent=2)+'\n')
    (out/'.nojekyll').touch()
    shutil.copy2(SDK.parent/'LICENSE', out/'LICENSE.txt')
    # Include every adapter/DSP header in provenance, not absolute workstation paths.
    files=set(sources+[manifest])
    for d in includes: files.update(d.rglob('*.h'))
    record = {'compiler': subprocess.check_output([a.cc,'--version'],text=True).splitlines()[0],
              'wasmSHA256': public['wasmSHA256'],
              'inputs': {os.path.relpath(p,SDK.parent):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(files)}}
    (out/'build.json').write_text(json.dumps(record,indent=2)+'\n')
    print(f'Built {m["name"]}: {out} ({(out/"engine.wasm").stat().st_size:,} bytes)')

if __name__ == '__main__': main()
