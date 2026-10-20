#!/usr/bin/env python3
"""Regenerate, sanitize, render fixtures, pack and validate the actual candidate."""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[1]
KENNEL=ROOT.parents[2]
TASK=KENNEL.parents[2]
sys.path.insert(0,str(Path(__file__).parent))
import gen_tables,gen_art,gen_presets,check_assets,check_package

def run(args,log,env=None):
    p=subprocess.run(list(map(str,args)),cwd=KENNEL,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,env=env)
    log.write(p.stdout);log.flush()
    print(p.stdout,end="")
    if p.returncode:raise SystemExit(p.returncode)
def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--toolchain",type=Path,default=Path(os.environ.get("KENNEL_TOOLCHAIN",TASK/"toolchain")))
    ap.add_argument("--skip-regenerate",action="store_true",help="use checked-in tables")
    a=ap.parse_args()
    out=KENNEL/"build/vapora-1.1-validation";out.mkdir(parents=True,exist_ok=True)
    if not a.skip_regenerate:
        old=hashlib.sha256((ROOT/"vapora_tables.h").read_bytes()).hexdigest() if (ROOT/"vapora_tables.h").exists() else None
        gen_tables.generate()
        first=hashlib.sha256((ROOT/"vapora_tables.h").read_bytes()).hexdigest()
        gen_tables.generate()
        assert first==hashlib.sha256((ROOT/"vapora_tables.h").read_bytes()).hexdigest(),"nondeterministic generator"
        print("Generator repeatability: PASS", "(unchanged)" if old==first else "(new assets)")
    gen_art.generate();gen_presets.generate()
    # Preset renames must not leave stale generated fixtures in the current report.
    for pattern in ("preset-*.pcm","preset-*.wav"):
        for fixture in out.glob(pattern):fixture.unlink()
    with (out/"checks.log").open("w") as log:
        run(["clang","-std=c11","-O1","-g","-Wall","-Wextra","-Wno-unknown-pragmas",
             "-fsanitize=address,undefined","-fno-sanitize-recover=all","-DVAPORA_HOST",
             "-I"+str(KENNEL/"synth_sdk"),"-I"+str(ROOT),
             ROOT/"tests/host.c",
             ROOT/"vapora.c",ROOT/"vapora_dsp.c",ROOT/"vapora_screen.c",
             "-lm","-o",out/"check"],log)
        run([out/"check",out],log)
        assets=check_assets.check(out)
        run([sys.executable,KENNEL/"synth_sdk/harness/svg_check.py",ROOT],log)
        # Keep prior candidate before replacement, even during development.
        target=KENNEL/"build/vapora.knl"
        if target.exists():
            history=KENNEL/"build/vapora-history";history.mkdir(exist_ok=True)
            h=hashlib.sha256(target.read_bytes()).hexdigest()[:12]
            shutil.copyfile(target,history/f"vapora-{h}.knl")
        run([sys.executable,KENNEL/"tools/kennel/pack.py",ROOT,"-o",target,
             "--keep",KENNEL/"build/vapora-objects","--toolchain",a.toolchain],log)
        package=check_package.check(a.toolchain,out)
    provenance=json.loads((KENNEL.parents[1]/"PROVENANCE.json").read_text())
    sources={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest()
             for p in sorted(ROOT.rglob("*")) if p.is_file() and ".git" not in p.parts
             and "__pycache__" not in p.parts}
    stacks={}
    for p in (out/"stack").glob("*.su"):
        stacks[p.name]=p.read_text()
    record=dict(sdk="Hundekraft 3.0.2",kennel_commit=provenance["kennel_commit"],
                sdk_archive_sha256=(hashlib.sha256((TASK/"downloads/hundekraft-3.0.2-source.tar.gz").read_bytes()).hexdigest()
                                    if (TASK/"downloads/hundekraft-3.0.2-source.tar.gz").exists() else "see pinned source checksum"),
                compiler_sha256=hashlib.sha256((a.toolchain/"bfin-gcc/bin/bfin-elf-gcc").read_bytes()).hexdigest(),
                package=package,tables=json.loads((ROOT/"data/tables.json").read_text()),
                source_hashes=sources,stack_usage=stacks,
                status=dict(implemented=True,native_compiled=True,package_checked=True,host_tested=True,
                            native_rendered=False,hardware_tested=False),
                missing="No native emulator/renderer in archive; no connected OP-1 inventory, CPU/GUI timing or listening test.",
                tests_log="checks.log",assets_report="assets.json")
    if (ROOT/".git").exists():
        record["source_revision"]=subprocess.check_output(["git","rev-parse","HEAD"],cwd=ROOT,text=True).strip()
        record["source_dirty"]=bool(subprocess.check_output(["git","status","--porcelain"],cwd=ROOT,text=True).strip())
    (out/"build-record.json").write_text(json.dumps(record,indent=2)+"\n")
    print("Hardware-test candidate:",target)
if __name__=="__main__":main()
