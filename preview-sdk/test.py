#!/usr/bin/env python3
"""Build native and WebAssembly adapters, compare sound, exercise synth and effect paths."""
import argparse, subprocess, sys
from pathlib import Path
SDK=Path(__file__).resolve().parent
ROOT=SDK.parent
ap=argparse.ArgumentParser(description=__doc__)
ap.add_argument('--cc',default='clang');ap.add_argument('--native-cc',default='clang');ap.add_argument('--node',default='node')
a=ap.parse_args();out=ROOT/'.preview-tests';out.mkdir(exist_ok=True)
subprocess.run([sys.executable,str(SDK/'build.py'),'--cc',a.cc],check=True)
subprocess.run([sys.executable,str(SDK/'build.py'),'--cc',a.cc,'--plugin',str(SDK/'examples/gain/plugin.json'),'--out',str(out/'gain')],check=True)
sources=[SDK/'tests/native.c',SDK/'runtime/host.c',ROOT/'preview/adapters/vapora.c',ROOT/'plugin/vapora/vapora.c',ROOT/'plugin/vapora/vapora_dsp.c',ROOT/'plugin/vapora/vapora_screen.c']
cmd=[a.native_cc,'-std=c11','-O2','-DVAPORA_HOST','-Wno-unknown-pragmas','-I'+str(SDK/'include'),'-I'+str(ROOT/'plugin/vapora'),'-I'+str(ROOT/'preview/adapters/include')]+list(map(str,sources))+['-o',str(out/'native')]
subprocess.run(cmd,check=True)
with (out/'native.f32').open('wb') as f:subprocess.run([str(out/'native')],stdout=f,check=True)
subprocess.run([a.node,str(SDK/'tests/test.mjs')],check=True)
