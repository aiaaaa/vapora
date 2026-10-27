import {loadEngine,instantiate,validatePatch} from './engine.mjs';
import {drawScreen} from './screen.mjs';
const $=id=>document.getElementById(id);
let manifest,module,visual,context,node,gain,values=[],env=[.3,1,.85,2],octave=0,audioOn=false,selected=-1,active=0,frameCount=0,drawCount=0;
let sourceNode,audioBuffer,chordOn=false;
const held=new Map(),mode=new Map();
const keyCodes=['KeyA','KeyW','KeyS','KeyE','KeyD','KeyF','KeyT','KeyG','KeyY','KeyH','KeyU','KeyJ','KeyK'];
const names=['C','C♯','D','D♯','E','F','F♯','G','G♯','A','A♯','B','C'];
function error(e){$('error').hidden=false;$('error').textContent=e.message||String(e);}
function clearError(){$('error').hidden=true;}
function send(m){node?.port.postMessage(m);}
function applyValues(){values.forEach((v,i)=>visual.pv_parameter(i,v));send({type:'parameters',values});}
function applyEnvelope(){send({type:'envelope',value:env});renderEnvelope();}
function updateStatus(){
  $('status').textContent=audioOn?(active?`${active} / 6 voices`:'Ready to play'):'Audio is off · explore the controls';
  if(manifest.kind==='effect'&&audioOn)$('status').textContent=sourceNode?'Playing audio input':'Choose an audio file';
  $('indicator').classList.toggle('on',audioOn);$('audio').textContent=audioOn?'Audio enabled':'Enable audio';$('audio').setAttribute('aria-pressed',String(audioOn));
  $('chord').disabled=$('stop').disabled=!audioOn;
}
function markEdited(){selected=-1;document.querySelectorAll('#presets button').forEach(b=>b.setAttribute('aria-pressed','false'));$('preset-note').textContent='Custom sound · dry browser preview.';}
function format(p,v){if(p.choices)return p.choices[Math.round(v*(p.choices.length-1))];if(p.format==='speed'){const q=Math.round(1+v*32766);return ((20+Math.floor(Math.floor(q*q/32768)*1980/32768))/1000).toFixed(2)+' Hz';}return Math.round(v*100)+'%';}
function renderControls(){
  $('controls').replaceChildren();
  for(const [index,k] of manifest.knobs.entries()){
    const secondary=mode.get(index)&&k.secondary!=null,p=manifest.parameters[secondary?k.secondary:k.primary];
    const c=document.createElement('div');c.className='control '+(k.color||'blue');
    const label=document.createElement('label');label.htmlFor='param-'+p.id;label.textContent=p.label;
    const dial=document.createElement('div');dial.className='dial';dial.style.setProperty('--angle',`${values[p.id]*270-135}deg`);
    const output=document.createElement('output');output.textContent=format(p,values[p.id]);output.htmlFor=label.htmlFor;
    const input=document.createElement('input');input.type='range';input.id=label.htmlFor;input.min=0;input.max=1;input.step=p.choices?1/(p.choices.length-1):.001;input.value=values[p.id];input.setAttribute('aria-valuetext',output.textContent);
    input.addEventListener('input',()=>{values[p.id]=Number(input.value);output.textContent=format(p,values[p.id]);input.setAttribute('aria-valuetext',output.textContent);dial.style.setProperty('--angle',`${values[p.id]*270-135}deg`);applyValues();markEdited();});
    c.append(label,dial,output,input);
    if(k.secondary!=null){const b=document.createElement('button');b.className='alternate';b.textContent=manifest.parameters[secondary?k.primary:k.secondary].label+' ↔';b.setAttribute('aria-label',`Switch ${manifest.parameters[k.primary].label} / ${manifest.parameters[k.secondary].label}`);b.setAttribute('aria-pressed',String(!!secondary));b.onclick=()=>{mode.set(index,!secondary);if(k.action!=null)visual.pv_action(k.action,!secondary?1:0);renderControls();};c.append(b);}
    else{const spacer=document.createElement('div');spacer.className='alternate-space';c.append(spacer);}
    $('controls').append(c);
  }
}
function renderEnvelope(){
  $('envelope').replaceChildren();
  ['Attack','Decay','Sustain','Release'].forEach((name,i)=>{
    const row=document.createElement('div');row.className='env-row';const label=document.createElement('label');label.htmlFor='env-'+i;label.textContent=name;
    const value=document.createElement('output');const show=()=>value.textContent=i===2?Math.round(env[i]*100)+'%':env[i].toFixed(2)+' s';show();label.append(value);
    const input=document.createElement('input');input.id=label.htmlFor;input.type='range';input.min=i===2?0:.005;input.max=i===2?1:10;input.step=.005;input.value=env[i];input.oninput=()=>{env[i]=Number(input.value);show();send({type:'envelope',value:env});};row.append(label,input);$('envelope').append(row);
  });
}
function renderOctave(){const start=3+octave;$('octave').textContent=`C${start}–C${start+1}`;$('octave-down').disabled=octave<=-3;$('octave-up').disabled=octave>=3;document.querySelectorAll('.key').forEach((b,i)=>b.setAttribute('aria-label',names[i]+(i===12?start+1:start)));}
function noteOn(token,semitone){
  if(!audioOn||held.has(token))return;const note=48+octave*12+semitone;
  held.set(token,{note,semitone});send({type:'on',note,velocity:.8});document.querySelector(`[data-note="${semitone}"]`)?.classList.add('active');
}
function noteOff(token){const entry=held.get(token);if(!entry)return;held.delete(token);if(![...held.values()].some(v=>v.note===entry.note))send({type:'off',note:entry.note});if(![...held.values()].some(v=>v.semitone===entry.semitone))document.querySelector(`[data-note="${entry.semitone}"]`)?.classList.remove('active');}
function panic(){held.clear();send({type:'panic'});active=0;chordOn=false;$('chord').textContent='Play a chord';document.querySelectorAll('.key').forEach(b=>b.classList.remove('active'));if(sourceNode){sourceNode.stop();sourceNode=null;$('input-play').textContent='Play input';}updateStatus();}
function preset(index){
  panic();const p=manifest.presets[index];selected=index;values=[...p.values];octave=p.octave;env=[...p.envelope];applyValues();applyEnvelope();renderControls();renderOctave();$('preset-note').textContent=p.note;
  document.querySelectorAll('#presets button').forEach((b,i)=>b.setAttribute('aria-pressed',String(i===index)));
}
async function enableAudio(){
  clearError();$('audio').disabled=true;
  try{
    if(!context){
      context=new AudioContext({sampleRate:44100,latencyHint:'interactive'});
      // Resume immediately inside the user's gesture, before fetching the worklet.
      await context.resume();
      await context.audioWorklet.addModule(new URL('./processor.mjs',import.meta.url));
      node=new AudioWorkletNode(context,'hundekraft-preview',{numberOfInputs:manifest.kind==='effect'?1:0,numberOfOutputs:1,outputChannelCount:[2],processorOptions:{module,kind:manifest.kind}});
      gain=context.createGain();gain.gain.value=Number($('volume').value);node.connect(gain).connect(context.destination);
      node.port.onmessage=({data:m})=>{
        if(m.type==='telemetry'){
          if(m.snapshot?.byteLength<=1024){new Uint8Array(visual.memory.buffer,visual.pv_snapshot_buffer(),m.snapshot.byteLength).set(m.snapshot);visual.pv_receive_snapshot();}
          active=m.active;$('meter').style.width=Math.min(100,m.peak*Number($('volume').value)*100)+'%';updateStatus();
        }else if(m.type==='error'){error(new Error(m.message));panic();context.suspend();audioOn=false;updateStatus();}
      };
      node.onprocessorerror=()=>{error(new Error('Audio engine stopped. Reload the preview to restart.'));panic();audioOn=false;updateStatus();};
      applyValues();applyEnvelope();audioOn=true;
    }else if(audioOn){panic();await context.suspend();audioOn=false;}
    else{await context.resume();audioOn=true;}
    updateStatus();
  }catch(e){error(e);if(context){await context.close().catch(()=>{});context=null;node=null;}audioOn=false;updateStatus();}
  finally{$('audio').disabled=false;}
}
function patch(){return {schema:1,kind:'hundekraft-browser-preview',plugin:manifest.id,values,octave,envelope:env};}
async function start(){
  const response=await fetch(new URL('./plugin.json',import.meta.url));if(!response.ok)throw new Error('Could not load preview');manifest=await response.json();
  module=await loadEngine(new URL('./engine.wasm',import.meta.url),manifest.wasmSHA256);visual=instantiate(module,manifest.kind);
  values=manifest.parameters.map(p=>p.default);
  document.title=manifest.name+' · Play';$('title').textContent=manifest.name;$('description').textContent=manifest.description;$('instrument-label').textContent=`${manifest.name.toUpperCase()} / ${manifest.version}`;$('limitations').textContent=manifest.limitations;
  if(manifest.repository)$('source').href=manifest.repository;else $('source').hidden=true;
  const canvas=$('screen');canvas.width=manifest.screen.width*3;canvas.height=manifest.screen.height*3;canvas.setAttribute('aria-label',`Live ${manifest.name} screen`);
  manifest.buttons.forEach(b=>{const button=document.createElement('button');button.textContent=b.label;if(b.toggle)button.setAttribute('aria-pressed','false');button.onclick=()=>{visual.pv_action(b.action,1);drawScreen(canvas,visual,1000,manifest.screen.width,manifest.screen.height);if(b.pulse)visual.pv_action(b.action,0);if(b.toggle)button.setAttribute('aria-pressed',String(button.getAttribute('aria-pressed')!=='true'));};$('screen-actions').append(button);});
  manifest.presets.forEach((p,i)=>{const b=document.createElement('button');b.textContent=p.name;b.setAttribute('aria-pressed','false');b.onclick=()=>preset(i);$('presets').append(b);});
  if(manifest.kind==='synth'){
    let white=0;
    keyCodes.forEach((code,i)=>{
      const black=[1,3,6,8,10].includes(i),b=document.createElement('button');b.className='key'+(black?' black':'');b.dataset.note=i;b.textContent=code.slice(3);if(black)b.style.left=`${white*12.5-4}%`;else white++;
      b.onpointerdown=e=>{e.preventDefault();b.setPointerCapture(e.pointerId);noteOn('pointer-'+e.pointerId,i);};b.onpointerup=e=>noteOff('pointer-'+e.pointerId);b.onpointercancel=e=>noteOff('pointer-'+e.pointerId);b.onlostpointercapture=e=>noteOff('pointer-'+e.pointerId);
      b.onkeydown=e=>{if(e.key===' '||e.key==='Enter'){e.preventDefault();if(!e.repeat)noteOn('button-'+i,i);}};b.onkeyup=e=>{if(e.key===' '||e.key==='Enter')noteOff('button-'+i);};b.onblur=()=>noteOff('button-'+i);$('keyboard').append(b);
    });
  }else{$('performance').hidden=true;$('effect-input').hidden=false;document.querySelector('.preset-section').hidden=true;document.querySelector('details').hidden=true;}
  if(manifest.presets.length)preset(manifest.defaultPreset||0);else{applyValues();renderControls();renderEnvelope();renderOctave();}
  $('audio').disabled=false;updateStatus();
  let last=performance.now();function frame(now){const us=Math.min(100000,Math.round((now-last)*1000));last=now;try{drawCount=drawScreen(canvas,visual,us,manifest.screen.width,manifest.screen.height);frameCount++;}catch(e){error(e);return;}requestAnimationFrame(frame);}requestAnimationFrame(frame);
}
$('audio').onclick=enableAudio;$('stop').onclick=panic;
$('volume').oninput=()=>{$('volume-value').textContent=Math.round(Number($('volume').value)*100)+'%';if(gain)gain.gain.setTargetAtTime(Number($('volume').value),context.currentTime,.015);};
$('octave-down').onclick=()=>{panic();octave=Math.max(-3,octave-1);renderOctave();};$('octave-up').onclick=()=>{panic();octave=Math.min(3,octave+1);renderOctave();};
$('chord').onclick=()=>{if(chordOn){[0,1,2].forEach(i=>noteOff('chord-'+i));chordOn=false;$('chord').textContent='Play a chord';return;}[0,7,12].forEach((n,i)=>noteOn('chord-'+i,n));chordOn=true;$('chord').textContent='Release chord';};
window.addEventListener('keydown',e=>{if(e.code==='Escape'){panic();return;}if(e.repeat||e.ctrlKey||e.metaKey||e.altKey||['INPUT','SELECT','TEXTAREA','SUMMARY'].includes(e.target.tagName))return;const i=keyCodes.indexOf(e.code);if(i>=0){e.preventDefault();noteOn(e.code,i);}});
window.addEventListener('keyup',e=>noteOff(e.code));window.addEventListener('blur',()=>{if(manifest)panic();});document.addEventListener('visibilitychange',()=>{if(document.hidden&&manifest)panic();});
$('export').onclick=()=>{const blob=new Blob([JSON.stringify(patch(),null,2)+'\n'],{type:'application/json'}),url=URL.createObjectURL(blob),a=document.createElement('a');a.href=url;a.download=manifest.id+'-browser-patch.json';a.click();setTimeout(()=>URL.revokeObjectURL(url),1000);};
$('import').onclick=()=>$('patch-file').click();$('patch-file').onchange=async e=>{try{const f=e.target.files[0];if(!f)return;if(f.size>16384)throw new Error('Patch is too large (maximum 16 KB).');const p=validatePatch(JSON.parse(await f.text()),manifest);panic();values=p.values;octave=p.octave;env=p.envelope;applyValues();applyEnvelope();renderControls();renderOctave();markEdited();clearError();}catch(e){error(e);}finally{$('patch-file').value='';}};
$('audio-file').onchange=async e=>{try{const f=e.target.files[0];if(!f)return;if(f.size>50*1024*1024)throw new Error('Choose an audio file smaller than 50 MB.');if(!audioOn)await enableAudio();if(!context)throw new Error('Enable audio first.');audioBuffer=await context.decodeAudioData(await f.arrayBuffer());$('input-play').disabled=false;}catch(e){error(e);}};
$('input-play').onclick=()=>{if(sourceNode){panic();return;}if(!audioOn||!audioBuffer)return;sourceNode=context.createBufferSource();sourceNode.buffer=audioBuffer;sourceNode.loop=true;sourceNode.connect(node);sourceNode.start();$('input-play').textContent='Stop input';updateStatus();};
// Read-only diagnostic surface for host QA; no user data, no device operations.
window.previewStatus=()=>({ready:!!visual,audio:audioOn,active,frames:frameCount,commands:drawCount,kind:manifest?.kind,parameters:[...values]});
start().catch(e=>{error(e);$('status').textContent='Preview could not start';$('audio').textContent='Reload to retry';});
