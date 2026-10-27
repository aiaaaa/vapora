import {instantiate} from './engine.mjs';
class PreviewProcessor extends AudioWorkletProcessor {
  constructor(options) {
    super();
    if (sampleRate !== 44100) throw new Error('Preview requires a 44.1 kHz AudioContext.');
    const p=options.processorOptions;
    this.e=instantiate(p.module,p.kind);
    this.input=new Float32Array(this.e.memory.buffer,this.e.pv_input(),256);
    this.output=new Float32Array(this.e.memory.buffer,this.e.pv_output(),256);
    this.offset=128; this.frames=0; this.failed=false;
    this.port.onmessage=({data:m})=>{
      const e=this.e;
      if(m.type==='parameters' && Array.isArray(m.values) && m.values.length<=64 && m.values.every(v=>Number.isFinite(v)&&v>=0&&v<=1))m.values.forEach((v,i)=>e.pv_parameter(i,v));
      else if(m.type==='on' && Number.isInteger(m.note) && m.note>=0 && m.note<=127 && Number.isFinite(m.velocity)){
        const pitch=Math.round(440*2**((m.note-69)/12)*4294967296/44100);
        e.pv_note_on(m.note,m.velocity,pitch);
      }
      else if(m.type==='off')e.pv_note_off(m.note);
      else if(m.type==='panic'){e.pv_panic();this.offset=128;}
      else if(m.type==='envelope' && m.value?.length===4 && m.value.every(Number.isFinite))e.pv_envelope(...m.value);
    };
    this.port.postMessage({type:'ready'});
  }
  process(inputs,outputs) {
    const out=outputs[0];if(!out?.length)return true;
    if(this.failed){for(const c of out)c.fill(0);return true;}
    try {
      // ABI block is 128. Reject a changed browser quantum for effects rather than misalign input.
      if(this.e.pv_kind() && out[0].length!==128)throw new Error('Unsupported effect audio quantum');
      for(let n=0;n<out[0].length;n++){
        if(this.offset===128){
          for(let c=0;c<2;c++){
            const source=inputs[0]?.[c]??inputs[0]?.[0];
            if(source)this.input.set(source.subarray(0,128),c*128);else this.input.fill(0,c*128,c*128+128);
          }
          this.e.pv_process();this.offset=0;
        }
        for(let c=0;c<out.length;c++){
          const value=this.output[Math.min(c,1)*128+this.offset];
          if(!Number.isFinite(value))throw new Error('Non-finite engine output');
          out[c][n]=value;
        }
        this.offset++;
      }
      this.frames+=out[0].length;
      if(this.frames>=1470){
        this.frames=0;
        const size=this.e.pv_snapshot_size();
        const snapshot=new Uint8Array(this.e.memory.buffer,this.e.pv_snapshot_buffer(),size).slice();
        let peak=0;for(const x of this.output)peak=Math.max(peak,Math.abs(x));
        this.port.postMessage({type:'telemetry',snapshot,active:this.e.pv_active(),peak},[snapshot.buffer]);
      }
    } catch(error){this.failed=true;for(const c of out)c.fill(0);this.port.postMessage({type:'error',message:error.message});}
    return true;
  }
}
registerProcessor('hundekraft-preview',PreviewProcessor);
