#!/usr/bin/env python3
"""Data checks and previews of actual C-generated line commands.
Pillow rasterization is a HOST APPROXIMATION, not the OP-1 renderer.
"""
from pathlib import Path
import hashlib,json,re,wave,xml.etree.ElementTree as ET
import numpy as np
from PIL import Image,ImageDraw,ImageFont
ROOT=Path(__file__).resolve().parents[1]

def check(out):
    text=(ROOT/"vapora_tables.h").read_text()
    arrays={}
    for name,n,body in re.findall(r"int16_t (ps_t\d+_\d+)\[(\d+)\] = \{(.*?)\};",text,re.S):
        a=np.fromstring(body.replace("\n",""),sep=",",dtype=np.int16)
        assert len(a)==int(n)
        arrays[name]=a
    cfg=json.loads((ROOT/"data/voices.json").read_text())
    assert len(arrays)==(2+len(cfg["textures"]))*8
    preset_count=len(json.loads((ROOT/"data/presets.json").read_text()))
    fundamental=cfg["reference_cycles"]
    result=[]
    for name,a in arrays.items():
        x=a.astype(float);spec=np.fft.rfft(x);energy=abs(spec)**2
        assert abs(x.mean())<=.5 and abs(x).max()<=23000
        leak=float(energy[int(len(x)*.4)+1:].sum()/max(1,energy.sum()))
        quantization_rms=float(np.sqrt(2*energy[int(len(x)*.4)+1:].sum())/len(x))
        assert leak<2e-7 or quantization_rms<=.5,(name,leak,quantization_rms)
        if name.endswith("_7"):assert not a.any() # top safety level is silent
        result.append(dict(table=name,dc=float(x.mean()),alias_band_energy_ratio=leak,alias_band_rms_lsb=quantization_rms))
    spectra=[]
    for t in range(2):
        spectrum=np.abs(np.fft.rfft(arrays[f"ps_t{t}_0"].astype(float)))
        bins=np.arange(fundamental-24,fundamental+25);power=spectrum[bins]**2
        center=float(np.sum(bins*power)/power.sum())
        width=float(np.sqrt(np.sum((bins-center)**2*power)/power.sum()))
        support=int(np.sum(spectrum[bins]>.1*spectrum[bins].max()))
        assert abs(center-fundamental)<1 and width>1.2 and support>=9
        spectra.append(dict(endpoint=t,fundamental_center_bin=center,
                            fundamental_power_std_bins=width,bins_above_10pct_amplitude=support))
    for t,e in enumerate(cfg["textures"],2):
        if e.get("gain_group")!="cinema":continue
        spectrum=abs(np.fft.rfft(arrays[f"ps_t{t}_0"].astype(float)))
        f0=e["master_length"]//64
        bins=np.arange(f0-16,f0+17);power=spectrum[bins]**2
        center=float(np.sum(bins*power)/power.sum())
        width=float(np.sqrt(np.sum((bins-center)**2*power)/power.sum()))
        support=int(np.sum(spectrum[bins]>.1*spectrum[bins].max()))
        assert width>1.1 and support>=7,(e["name"],width,support)
        spectra.append(dict(endpoint=t,name=e["name"],fundamental_center_bin=center,
                            fundamental_power_std_bins=width,bins_above_10pct_amplitude=support))
    morph=[]
    for f in range(1):
        a=arrays[f"ps_t{2*f}_0"].astype(float);b=arrays[f"ps_t{2*f+1}_0"].astype(float)
        ratio=float(np.sqrt(np.mean(((a+b)/2)**2))/np.sqrt(np.mean(a*a)))
        assert .8<ratio<=1.01
        morph.append(ratio)
    audio=[]
    for pcm in sorted(out.glob("*.pcm")):
        data=pcm.read_bytes()
        with wave.open(str(pcm.with_suffix(".wav")),"wb") as w:
            w.setnchannels(1);w.setsampwidth(2);w.setframerate(44100);w.writeframes(data)
        x=np.frombuffer(data,dtype="<i2").astype(float)
        peak=int(np.max(np.abs(x)));rms=float(np.sqrt(np.mean(x*x)))
        assert peak>100 and rms>10,(pcm.name,"silent or near-silent fixture")
        # Approximate correlation at the nominal long-loop period at C4.
        lag=round(fundamental*44100/261.625565)
        held=x[4410:4*44100]
        corr=float(np.corrcoef(held[:-lag],held[lag:])[0,1]) if np.std(held[lag:])>1e-9 and np.std(held[:-lag])>1e-9 else None
        spec=np.abs(np.fft.rfft(held*np.hanning(len(held))))**2
        freq=np.fft.rfftfreq(len(held),1/44100)
        image_ratio=float(spec[freq>11025].sum()/max(spec.sum(),1))
        assert image_ratio<.002,(pcm.name,image_ratio)
        audio.append(dict(file=pcm.with_suffix(".wav").name,peak=peak,rms=rms,
                          above_internal_nyquist_energy_ratio=image_ratio,
                          nominal_c4_loop_correlation=corr,
                          sha256=hashlib.sha256(pcm.with_suffix(".wav").read_bytes()).hexdigest()))
    assert len(audio)==preset_count+4
    report=dict(tables=result,measured_spectral_profiles=spectra,morph_midpoint_rms_relative_to_endpoint=morph,audio=audio,
                note="WAVs use production DSP and simulated per-preset attack/decay/sustain/release. No stock T2 renderer or FX: Deep Pressure's saved Reverb is not rendered in these dry fixtures. No per-file normalization. Correlation is diagnostic, not an audition.")
    (out/"assets.json").write_text(json.dumps(report,indent=2)+"\n")
    diversity(out)
    visual_checks(out)
    render(out)
    print(f"{len(arrays)} tables: DC/peaks/spectral limits checked; coherent morph midpoint RMS",morph)
    print(f"{len(audio)} WAV fixtures wrapped without normalization; scene/sun/animation previews at 320x160")
    return report

def diversity(out):
    features=[];names=[];raw=[]
    for f in sorted(out.glob("preset-*.pcm")):
        x=np.frombuffer(f.read_bytes(),dtype="<i2").astype(float)
        # Equal-power log-frequency fingerprints expose near-duplicate timbres.
        power=np.zeros(8193)
        for start in range(0,min(len(x)-16384,6*44100),8192):
            power+=abs(np.fft.rfft(x[start:start+16384]*np.hanning(16384)))**2
        freq=np.fft.rfftfreq(16384,1/44100);edges=np.geomspace(45,10000,97)
        bands=np.array([power[(freq>=lo)&(freq<hi)].sum() for lo,hi in zip(edges[:-1],edges[1:])])
        bands=np.sqrt(bands/max(bands.sum(),1));bands/=max(np.linalg.norm(bands),1e-12)
        features.append(bands);names.append(f.stem);raw.append(hashlib.sha256(x.tobytes()).hexdigest())
    count=len(json.loads((ROOT/"data/presets.json").read_text()))
    assert len(set(raw))==count
    similarities=np.array(features)@np.array(features).T
    pairs=sorted([(float(similarities[i,j]),names[i],names[j]) for i in range(count) for j in range(i)],reverse=True)
    report={"method":"Cosine similarity of 96 equal-power log-frequency fingerprints from the first six seconds of dry fixtures. 1 means identical distribution; this is not a listening verdict.",
            "unique_pcm_fixtures":count,"closest_pairs":[{"similarity":a,"a":b,"b":c} for a,b,c in pairs[:20]]}
    (out/"preset-diversity.json").write_text(json.dumps(report,indent=2)+"\n")
    print("Nearest spectral pairs:",pairs[:5])

def visual_checks(out):
    def lines(name,width):
        root=ET.parse(out/name).getroot()
        return [e.attrib for e in root.iter() if e.tag.split("}")[-1]=="line"
                and abs(float(e.get("stroke-width","0"))-width/1024)<.0001]
    for name in ("minimum.svg","default.svg"):
        assert len(lines(name,3200))==8
        assert len(lines(name,1900))==24
        assert len(lines(name,2500))==16
        dots=lines(name,1900)+lines(name,2500)+lines(name,3200)
        xs=[float(d["x1"]) for d in dots];ys=[float(d["y1"]) for d in dots]
        assert max(xs)-min(xs)>270 and max(ys)-min(ys)>30
        mesh=lines(name,1950)
        xs=[float(d[k]) for d in mesh for k in ("x1","x2")]
        assert min(xs)<=2.1 and max(xs)>=316.9
    assert lines("idle-00.svg",1950)!=lines("idle-23.svg",1950),"idle surface did not move"
    ids=(5,1,2,3,10,18)
    for style in ids:
        normal=[]
        for cy in range(30,57):
            width=2048 if style==5 else 2200
            aa=lines(f"sunfixed-{style:02d}-{cy:02d}.svg",width)
            geometry=[tuple(round(float(a[k])-(cy if k.startswith("y") else 0),6)
                            for k in ("x1","y1","x2","y2")) for a in aa]
            normal.append(geometry)
        assert all(g==normal[0] for g in normal),"sun shape changed during translation"
    fill=[]
    for cy in range(30,57):
        aa=lines(f"sunmove-05-{cy:02d}.svg",2048)
        fill.append(sum(abs(float(a["x2"])-float(a["x1"])) for a in aa))
        assert all(a["y1"]==a["y2"] for a in aa)
        assert all((float(a["y1"])-cy)%2==1 for a in aa),"Eclipse row phase changed"
    assert fill[0]>600 and fill[-1]==0 and all(a>=b for a,b in zip(fill,fill[1:]))
    full=lines("sunfixed-05-30.svg",2048)
    assert len(full)==20
    xs=[float(a[k]) for a in full for k in ("x1","x2")]
    ys=[float(a["y1"])-30 for a in full]
    assert abs((max(xs)-min(xs))-(max(ys)-min(ys)+2))<.1,"Full Eclipse is not circular"
    assert all(abs((float(a["x1"])+float(a["x2"]))/2-160)<.001 for a in full)
    # Circle segment ends overlap instead of meeting at independently rounded pixels.
    for style,circles in [(1,[(0,0,15)]),(3,[(0,0,15)]),(10,[(0,0,12)]),
                          (18,[(-17,0,10),(0,0,14),(17,0,10)])]:
        aa=lines(f"sunfixed-{style:02d}-30.svg",2200)
        for n,(cx,yy,radius) in enumerate(circles):
            for a in aa[n*32:(n+1)*32]:
                for x,y in [(float(a["x1"])-160-cx,float(a["y1"])-30-yy),
                            (float(a["x2"])-160-cx,float(a["y2"])-30-yy)]:
                    assert abs((x*x+y*y)**.5-radius)<.1,"Notched circle geometry"
    retro=lines("sunmove-02-30.svg",2200)
    assert max(float(a["y1"])-30 for a in retro)==14,"Retro bottom stripe returned"
    for style,name in [(1,"HALO"),(2,"RETRO"),(3,"BANDS"),(5,"ECLIPSE"),(10,"SATURN"),(18,"TRIPLE")]:
        root=ET.parse(out/f"sun-{style:02d}.svg").getroot()
        text=" ".join(e.text or "" for e in root.iter() if e.tag.split("}")[-1]=="text")
        assert f"SUN {style:02d} {name}" in " ".join(text.split()),(style,text)
    (out/"sun-motion.json").write_text(json.dumps(dict(styles=6,original_ids=[5,1,2,3,10,18],heights=27,
        translation_invariant=list(ids),eclipse_radius=20,eclipse_full_aspect_error_pixels=abs((max(xs)-min(xs))-(max(ys)-min(ys)+2)),eclipse_scanline_width_sum_high_to_low=fill,
        limitation="Production line geometry and host pixels, not an OP-1 raster capture."),indent=2)+"\n")
    print("Visual contract: 48 sky dots / 24 small / 16 medium / 8 large; full-width water; Motion=0 idle mesh moves")

def render(out):
    static=ET.parse(ROOT/"vapora.svg").getroot()
    for svg in sorted(out.glob("*.svg")):
        root=ET.parse(svg).getroot()
        # Embed original static geometry into the host line-command recording.
        if not any(e.get("id")=="background" for e in root):
            root.insert(0,static[0])
            ET.register_namespace("","http://www.w3.org/2000/svg")
            ET.ElementTree(root).write(svg,encoding="unicode")
        image=Image.new("RGB",(1280,640),(0,0,0));draw=ImageDraw.Draw(image)
        for el in root.iter():
            tag=el.tag.split("}")[-1];a=el.attrib
            if tag=="line":pts=[(float(a["x1"]),float(a["y1"])),(float(a["x2"]),float(a["y2"]))]
            elif tag=="polyline":
                nums=[float(x) for x in re.findall(r"-?\d+(?:\.\d+)?",a["points"])]
                pts=list(zip(nums[::2],nums[1::2]))
            elif tag=="text":
                # Substitute only: native firmware supplies the actual stock glyphs.
                font_path=Path("/System/Library/Fonts/Menlo.ttc")
                font=(ImageFont.truetype(str(font_path),round(float(a["font-size"])*4)) if font_path.exists()
                      else ImageFont.load_default(size=round(float(a["font-size"])*4)))
                draw.text((round(float(a["x"])*4),round((float(a["y"])-float(a["font-size"]))*4)),el.text or "",font=font,fill=a["fill"])
                continue
            else:continue
            pts=[(round(x*4),round(y*4)) for x,y in pts]
            draw.line(pts,fill=a.get("stroke","#ffffff"),width=max(1,round(float(a.get("stroke-width",1))*4)))
        image.resize((320,160),Image.Resampling.LANCZOS).save(svg.with_suffix(".png"))
    suns=Image.new("RGB",(990,402),(20,23,30));sd=ImageDraw.Draw(suns)
    for i,style in enumerate((5,1,2,3,10,18)):
        name=f"sun-{style:02d}";x=10+(i%3)*330;y=10+(i//3)*198
        sd.text((x,y),name+" / HOST FONT SUBSTITUTE",fill=(180,185,200))
        suns.paste(Image.open(out/(name+".png")),(x,y+20))
    suns.save(out/"sun-contact-sheet.png")
    for style in (2,5,18):
        frames=[Image.open(out/f"sunmove-{style:02d}-{cy:02d}.png") for cy in range(56,29,-1)]
        frames=frames+frames[-2:0:-1]
        frames[0].save(out/f"sun-{style:02d}-travel-host.gif",save_all=True,append_images=frames[1:],duration=100,loop=0)
    names=["minimum","default","maximum","character","interval","speed","all-alternates","voice-onset","single-low","single-semitone","single-high"]
    sheet=Image.new("RGB",(660,1200),(20,23,30));d=ImageDraw.Draw(sheet)
    for i,n in enumerate(names):
        x=10+(i%2)*330;y=10+(i//2)*198
        d.text((x,y),n+" / HOST",fill=(180,185,200))
        sheet.paste(Image.open(out/(n+".png")),(x,y+20))
    sheet.save(out/"contact-sheet.png")
    for prefix,name in [("animation","ocean-host"),("idle","idle-host"),("default-idle","default-idle-host"),("single-low","single-low-host"),("single-semitone","single-semitone-host"),("single-high","single-high-host"),("secret","secret-host")]:
        frames=[Image.open(p).convert("RGB") for p in sorted(out.glob(prefix+"-*.png"))]
        if frames:frames[0].save(out/(name+".gif"),save_all=True,append_images=frames[1:],duration=63,loop=0)
if __name__=="__main__":
    import sys
    check(Path(sys.argv[1]))
