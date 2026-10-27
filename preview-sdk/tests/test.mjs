import fs from 'node:fs';
import assert from 'node:assert/strict';
import path from 'node:path';
import {fileURLToPath} from 'node:url';
import {instantiate,validatePatch} from '../web/engine.mjs';
const root=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'../..');
const bytes=fs.readFileSync(path.join(root,'site/engine.wasm'));
const module=await WebAssembly.compile(bytes);
assert.deepEqual(WebAssembly.Module.imports(module),[],'no firmware or WASI imports');
const manifest=JSON.parse(fs.readFileSync(path.join(root,'site/plugin.json')));
const pitch=note=>Math.round(440*2**((note-69)/12)*4294967296/44100);
function output(e){return new Float32Array(e.memory.buffer,e.pv_output(),256);}
let e=instantiate(module,'synth');
e.pv_envelope(.01,.1,.8,.1);for(let p=0;p<7;p++)e.pv_parameter(p,p/8);e.pv_note_on(60,.8,25480119);
const actual=new Float32Array(256*256);
for(let i=0;i<256;i++){
  if(i===100)e.pv_note_on(67,.65,38176255);if(i===160)e.pv_note_off(60);if(i===200)e.pv_note_off(67);
  e.pv_process();actual.set(output(e),i*256);
}
const native=fs.readFileSync(path.join(root,'.preview-tests/native.f32'));
const expected=new Float32Array(native.buffer,native.byteOffset,native.byteLength/4);
assert.equal(actual.length,expected.length);
let maxError=0;for(let i=0;i<actual.length;i++)maxError=Math.max(maxError,Math.abs(actual[i]-expected[i]));
assert(maxError<1e-7,`native/WASM mismatch: ${maxError}`);
console.log('PASS: native/WASM audio parity, max error',maxError);
const peaks=[];
for(const preset of manifest.presets){
  e=instantiate(module,'synth');preset.values.forEach((v,i)=>e.pv_parameter(i,v));e.pv_envelope(.005,.05,.85,.05);
  const notes=[48,52,55,60,64,67].map(n=>n+preset.octave*12);
  for(const n of notes)e.pv_note_on(n,.8,pitch(n));
  let peak=0,energy=0;
  for(let block=0;block<512;block++){
    e.pv_process();for(const sample of output(e)){assert(Number.isFinite(sample));peak=Math.max(peak,Math.abs(sample));energy+=sample*sample;}
  }
  assert(energy>.00001,`${preset.name} is silent`);assert(peak<1,`${preset.name} clipped: ${peak}`);assert.equal(e.pv_active(),6);
  for(const n of notes)e.pv_note_off(n);for(let b=0;b<32;b++)e.pv_process();assert.equal(e.pv_active(),0);
  e.pv_panic();e.pv_process();assert(output(e).every(v=>v===0));peaks.push({name:preset.name,peak});
}
console.log('PASS: all 15 presets, six voices, finite audio, headroom, release and panic');
e=instantiate(module,'synth');for(let n=40;n<60;n++)e.pv_note_on(n,.8,pitch(n));assert.equal(e.pv_active(),6);e.pv_panic();assert.equal(e.pv_active(),0);
// Separate graphics instance receives only a fixed-size snapshot, never shared mutable DSP state.
const visual=instantiate(module,'synth');e.pv_note_on(60,.8,pitch(60));for(let b=0;b<10;b++)e.pv_process();
const size=e.pv_snapshot_size();assert(size>0&&size<=1024);
new Uint8Array(visual.memory.buffer,visual.pv_snapshot_buffer(),size).set(new Uint8Array(e.memory.buffer,e.pv_snapshot_buffer(),size));visual.pv_receive_snapshot();
let maxCommands=0;
for(let character=0;character<14;character++)for(let sun=0;sun<7;sun++){
  visual.pv_parameter(4,character/13);visual.pv_action(2,1);
  for(let frame=0;frame<4;frame++){const n=visual.pv_frame(62500);assert(n>100);assert.equal(visual.pv_overflow(),0);maxCommands=Math.max(maxCommands,n);}
}
visual.pv_action(4,1);visual.pv_frame(1000);visual.pv_action(4,0);const count=visual.pv_frame(62500);
const commands=new Int32Array(visual.memory.buffer,visual.pv_commands(),count*20);assert([...commands].every(Number.isFinite));
console.log('PASS: all 14 characters × 7 suns; screen snapshots; readout;',maxCommands,'max commands');
const patch={schema:1,kind:'hundekraft-browser-preview',plugin:manifest.id,values:manifest.presets[0].values,octave:0,envelope:[.1,.1,.8,1]};
assert.deepEqual(validatePatch(patch,manifest).values,patch.values);
for(const bad of [{...patch,plugin:'other'},{...patch,values:[NaN]},{...patch,octave:9},{...patch,envelope:[.1,0,8,1]}])assert.throws(()=>validatePatch(bad,manifest));
console.log('PASS: patch round-trip and malformed/foreign patch rejection');
const fxModule=await WebAssembly.compile(fs.readFileSync(path.join(root,'.preview-tests/gain/engine.wasm')));
const fx=instantiate(fxModule,'effect'),input=new Float32Array(fx.memory.buffer,fx.pv_input(),256);
fx.pv_parameter(0,.25);for(let n=0;n<128;n++){input[n]=Math.sin(n/20);input[n+128]=Math.cos(n/11)*.4;}fx.pv_process();
for(let n=0;n<256;n++)assert(Math.abs(output(fx)[n]-input[n]*.25)<1e-7);
fx.pv_note_on(60,1,pitch(60));assert.equal(fx.pv_active(),0);
console.log('PASS: reusable effect ABI, independent stereo input/output, gain, notes ignored');
// Exercise the real worklet with an offline stand-in for its browser base class.
globalThis.sampleRate=44100;let Processor;
globalThis.AudioWorkletProcessor=class{constructor(){this.messages=[];this.port={postMessage:m=>this.messages.push(m)};}};
globalThis.registerProcessor=(name,C)=>{Processor=C;};
await import('../web/processor.mjs');
const w=new Processor({processorOptions:{module,kind:'synth'}});w.port.onmessage({data:{type:'envelope',value:[.005,.05,.8,.05]}});w.port.onmessage({data:{type:'on',note:60,velocity:.8}});
const audio=[new Float32Array(128),new Float32Array(128)];let workletPeak=0;
for(let b=0;b<200;b++){w.process([], [audio]);for(const x of audio[0])workletPeak=Math.max(workletPeak,Math.abs(x));}
assert(workletPeak>0);assert(w.messages.some(m=>m.type==='telemetry'&&m.active===1));
w.port.onmessage({data:{type:'panic'}});w.process([], [audio]);assert(audio.every(c=>c.every(v=>v===0)));
const fw=new Processor({processorOptions:{module:fxModule,kind:'effect'}});fw.port.onmessage({data:{type:'parameters',values:[.5]}});
fw.process([[Float32Array.from({length:128},()=>.4),Float32Array.from({length:128},()=>-.2)]],[audio]);assert(Math.abs(audio[0][0]-.2)<1e-7);assert(Math.abs(audio[1][0]+.1)<1e-7);
console.log('PASS: production AudioWorklet transport, telemetry, panic and stereo effect input');
fs.writeFileSync(path.join(root,'.preview-tests/results.json'),JSON.stringify({nativeWasmMaxError:maxError,peaks,maxCommands,workletPeak,passed:true},null,2)+'\n');
