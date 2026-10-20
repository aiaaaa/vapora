#!/usr/bin/env python3
"""Minimal native SVG backing; waves, sun and sky are drawn by screen C."""
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT.parents[1]/"display"))
from svgart import shape

def generate():
    objects=[]
    text='<svg xmlns="http://www.w3.org/2000/svg" width="320" height="160" viewBox="0 0 320 160">\n'
    text+='<g id="background">\n'+'\n'.join(objects)+'\n</g>\n</svg>\n'
    (ROOT/"vapora.svg").write_text(text)
    print("SVG",len(text.encode()),"bytes")
if __name__=="__main__":generate()
