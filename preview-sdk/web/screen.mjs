export function drawScreen(canvas,e,us,width,height) {
  const n=e.pv_frame(us);
  if(e.pv_overflow())throw new Error('Screen command capacity exceeded');
  const ctx=canvas.getContext('2d'), scale=canvas.width/width;
  ctx.setTransform(scale,0,0,scale,0,0);ctx.fillStyle='#000';ctx.fillRect(0,0,width,height);
  const view=new DataView(e.memory.buffer,e.pv_commands(),n*80);
  ctx.lineCap='round';ctx.lineJoin='round';
  for(let i=0;i<n;i++){
    const at=i*80,get=o=>view.getInt32(at+o,true)/1024;
    const type=view.getInt32(at,true), color=view.getUint32(at+24,true);
    const css=`rgba(${color>>>24},${color>>>16&255},${color>>>8&255},${(color&255)/255})`;
    ctx.strokeStyle=ctx.fillStyle=css;
    if(type===1){ctx.lineWidth=get(20);ctx.beginPath();ctx.moveTo(get(4),get(8));ctx.lineTo(get(12),get(16));ctx.stroke();}
    else if(type===2){ctx.beginPath();ctx.arc(get(4),get(8),get(20),0,Math.PI*2);ctx.fill();}
    else if(type===3){
      const chars=new Uint8Array(e.memory.buffer,e.pv_commands()+at+28,48);let text='';
      for(const c of chars){if(!c)break;text+=String.fromCharCode(c);}
      ctx.font=`${get(20)}px monospace`;ctx.textBaseline='top';ctx.fillText(text.toUpperCase(),get(4),get(8));
    }
  }
  return n;
}
