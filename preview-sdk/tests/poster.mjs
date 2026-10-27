/* README link image rendered from the compiled production screen. */
import fs from 'node:fs';
import {instantiate} from '../web/engine.mjs';
const module=await WebAssembly.compile(fs.readFileSync('site/engine.wasm'));
const e=instantiate(module,'synth');
const p=JSON.parse(fs.readFileSync('site/plugin.json')).presets[7];
p.values.forEach((v,i)=>e.pv_parameter(i,v));
let count=0;for(let i=0;i<64;i++)count=e.pv_frame(62500);
const data=new DataView(e.memory.buffer,e.pv_commands(),count*80);
let shapes='';
for(let i=0;i<count;i++){
  const at=i*80,type=data.getInt32(at,true),q=o=>(data.getInt32(at+o,true)/1024).toFixed(3);
  const c=data.getUint32(at+24,true),color='#'+(c>>>8).toString(16).padStart(6,'0');
  if(type===1)shapes+=`<line x1="${q(4)}" y1="${q(8)}" x2="${q(12)}" y2="${q(16)}" stroke="${color}" stroke-width="${q(20)}"/>`;
  if(type===2)shapes+=`<circle cx="${q(4)}" cy="${q(8)}" r="${q(20)}" fill="${color}"/>`;
}
const svg=`<svg xmlns="http://www.w3.org/2000/svg" width="960" height="480" viewBox="0 0 640 320" role="img" aria-labelledby="title desc"><title id="title">Play Vapora in your browser</title><desc id="desc">Vapora's original animated ocean screen. Open the demo to play the keyboard, turn controls, and explore 15 presets.</desc><rect width="640" height="320" rx="10" fill="#101713"/><g transform="translate(0 0) scale(2)" stroke-linecap="round">${shapes}</g><rect x="14" y="238" width="612" height="68" rx="6" fill="#edf1e8"/><circle cx="47" cy="272" r="19" fill="#286957"/><path d="M43 262L56 272L43 282Z" fill="#fff"/><text x="80" y="268" fill="#26332f" font-family="Arial,sans-serif" font-size="20" font-weight="bold" letter-spacing="2">PLAY VAPORA</text><text x="81" y="289" fill="#67766d" font-family="Arial,sans-serif" font-size="12">15 presets · Live controls · Play with your keyboard</text><text x="589" y="279" fill="#286957" font-family="Arial,sans-serif" font-size="25">↗</text></svg>`;
fs.writeFileSync('media/browser-preview.svg',svg+'\n');
console.log('Rendered README preview image from production screen commands.');
