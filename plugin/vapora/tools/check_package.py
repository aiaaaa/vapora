#!/usr/bin/env python3
"""Check actual .knl through reference loader, C loader, validator and GNU ld."""
from pathlib import Path
import hashlib,json,os,re,struct,subprocess,sys
ROOT=Path(__file__).resolve().parents[1]
KENNEL=ROOT.parents[2]
sys.path.insert(0,str(KENNEL/"tools/kennel"))
import pack,reloc
from test_plugin_format import BASE_LD

def run(args):
    return subprocess.check_output(list(map(str,args)),text=True,stderr=subprocess.STDOUT)
def check(toolchain,out):
    gcc,ld=pack.find_toolchain(str(toolchain))
    bindir=Path(ld).parent
    knl=KENNEL/"build/vapora.knl";data=knl.read_bytes()
    p=reloc.parse(data,1)
    exports=set(re.findall(r'\{\s*"(_\w+)"',(KENNEL/"synth_sdk/kennel/kn_exports.c").read_text()))
    assert not set(p["imports"])-exports
    assert p["type_name"]=="vapora" and p["legacy_order"]==0 and p["kind"]==1
    assert p["flags"]==2 and p["page2_defaults"]==[8192]*3
    assert len(data)<512*1024 and p["svg_size"]<8192
    cexe=out/"loader"
    run(["clang","-O1","-g","-fsanitize=address,undefined","-fno-sanitize-recover=all",
         "-I"+str(KENNEL/"synth_sdk"),ROOT/"tests/loader.c",KENNEL/"synth_sdk/kennel/kn_load.c",
         KENNEL/"synth_sdk/kennel/kn_validate.c","-o",cexe])
    ex={name:0x01e00100+i*0x40 for i,name in enumerate(p["imports"])}
    ro=KENNEL/"build/vapora-objects/vapora.ro"
    for base in (0x0299f000,0x02a40000):
        img=reloc.relocate(p,base,ex)
        cimage=out/f"c-{base:x}.bin"
        run([cexe,knl,hex(base),cimage,*p["imports"]])
        assert cimage.read_bytes()==img
        script=out/f"base-{base:x}.ld";script.write_text(BASE_LD.format(base=base))
        elf=out/f"base-{base:x}.elf";binary=elf.with_suffix(".bin")
        run([ld,"-T",script,ro,*[f"--defsym={n}={a:#x}" for n,a in ex.items()],"-o",elf])
        run([bindir/"bfin-elf-objcopy","-O","binary","-j",".text","-j",".rodata","-j",".data",elf,binary])
        ref=binary.read_bytes()
        assert img[:len(ref)]==ref and not any(img[p["image_size"]:])
        entry=struct.unpack_from("<3I4B",img,p["entry_offset"])
        assert entry[2]==0 and entry[3:]==(1,1,1,1)
    # Missing import rejection is a package load check, not a storage-fault test.
    missing=subprocess.run([str(cexe),str(knl),"0x0299f000",str(out/"missing.bin")],capture_output=True)
    assert missing.returncode!=0
    dsp=KENNEL/"build/vapora-objects/vapora_dsp.o"
    undefined=run([bindir/"bfin-elf-nm","-u",dsp]).strip()
    assert not undefined,undefined # all DSP calls resolved locally: no soft-float/division helpers.
    dis=run([bindir/"bfin-elf-objdump","-dr",dsp])
    (out/"dsp-disassembly.txt").write_text(dis)
    # Packer intentionally restricts manifest flags; measure stack separately
    # using identical compile flags plus the non-code-changing report option.
    stackdir=out/"stack";stackdir.mkdir(exist_ok=True)
    for source in ("vapora.c","vapora_dsp.c","vapora_screen.c"):
        run([gcc,*pack.CFLAGS,"-fstack-usage","-I"+str(KENNEL/"synth_sdk"),
             "-c",ROOT/source,"-o",stackdir/(Path(source).stem+".o")])
    report={k:p[k] for k in ("file_size","text_size","rodata_size","data_size","bss_size","load_size","svg_size","reloc_count","imports")}
    report.update(sha256=hashlib.sha256(data).hexdigest(),compiler=run([gcc,"--version"]).splitlines()[0],
                  binutils=run([ld,"--version"]).splitlines()[0],
                  package_checks="Python/C relocation and GNU ld equal at 2 bases; C descriptors valid; imports exported; no DSP helpers",
                  native_emulator=False,physical_op1=False)
    (out/"package.json").write_text(json.dumps(report,indent=2)+"\n")
    print(json.dumps(report,indent=2))
    return report
if __name__=="__main__":
    check(Path(sys.argv[1]),Path(sys.argv[2]))
