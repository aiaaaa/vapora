/* Original procedural ocean, 66 vertices / 165 unique edges.
 * GPL-3.0-or-later with OP-1 firmware permission. GUI thread only. */
#ifdef VAPORA_HOST
#include "tests/native_mock.h"
#else
#include "op1_synth.h"
#endif
#include "seq/seq_text.h"
#include "vapora_mailbox.h"
#include "vapora_visual.h"

static op1_pt vertices[PS_ROWS*PS_COLS];
static uint8_t edges[PS_EDGES][2],diagonal[PS_EDGES];
static int16_t bend[PS_EDGES];
static ps_snapshot seen[PS_VOICES];
static uint32_t sequence[PS_VOICES], clock_last, elapsed, ocean_phase, idle_phase;
static int view_slot=-1,view_selector=-1,ready;
static int32_t smooth_pitch[PS_VOICES], excitation[PS_VOICES];
static uint32_t note_phase[PS_VOICES], particle_phase;
static int active_knob, active_sun, control_ready;
static int sun_design; /* Original 05 Eclipse starts the cycle. */
static int16_t control_words[8];
static uint8_t control_pages[4];
static uint8_t text_objects[4][OP1_TEXT_SIZE] __attribute__((aligned(4)));
static uint8_t text_made[4];
static char text_cache[4][48];
static int16_t shape[PS_VOICES][PS_TRACE];
static int32_t visual_gain[PS_VOICES];

static uint32_t cycles(void)
{
#ifdef VAPORA_HOST
    return ps_host_cycles;
#else
    uint32_t c; __asm__ volatile("%0 = CYCLES;" : "=d"(c)); return c;
#endif
}
#include "vapora_q.h"
static int32_t rounded_wave(uint32_t phase)
{
    int32_t t=(int32_t)((phase>>16)&65535);
    t=t<32768?t*2-32768:98303-t*2;
    return (t*(65536-(t<0?-t:t)))>>15;
}
static int gradient(unsigned x,unsigned y,int dx,int dy)
{
    x&=7u;y&=7u; /* periodic tile: phase wrap cannot jump the noise field */
    uint32_t h=x*0x9e3779b9u+y*0x85ebca6bu+0x4f434541u;
    h^=h>>16;h*=0x7feb352du;h^=h>>15;
    switch(h&7){
        case 0:return dx+dy; case 1:return dx-dy;
        case 2:return -dx+dy; case 3:return -dx-dy;
        case 4:return dx; case 5:return -dx;
        case 6:return dy; default:return -dy;
    }
}
static int fade(int t)
{
    int t2=(t*t)>>8,t3=(t2*t)>>8;
    return (t3*(6*t2-15*t+2560))>>8;
}
static int coherent(unsigned x,unsigned y)
{
    int dx=x&255,dy=y&255; x>>=8;y>>=8;
    int fx=fade(dx),fy=fade(dy);
    int a=gradient(x,y,dx,dy),b=gradient(x+1,y,dx-256,dy);
    int c=gradient(x,y+1,dx,dy-256),d=gradient(x+1,y+1,dx-256,dy-256);
    a+=((b-a)*fx)>>8;c+=((d-c)*fx)>>8;
    return (a+(((c-a)*fy)>>8))*4;
}
static void line(void *r,int x,int y,int xx,int yy,int w,const uint8_t *c)
{
    op1_pt a={x,y},b={xx,yy};
    if(x==xx&&y==yy)return;
    op1_raster_line(r,&a,&b,w,c);
}
/* Clip at 1/64 pixel precision with bounded int32 products. Avoid the R1
 * software 64-bit division helper for every clipped edge. Raster uses Q10. */
static int region(int x,int y)
{
    return (x<PS_LEFT*64?1:x>PS_RIGHT*64?2:0)|
           (y<PS_TOP*64?4:y>PS_BOTTOM*64?8:0);
}
static void waterline(void *r,op1_pt a,op1_pt b,const uint8_t *c)
{
    a.x>>=4;a.y>>=4;b.x>>=4;b.y>>=4;
    for(int tries=0;tries<5;tries++){
        int ca=region(a.x,a.y),cb=region(b.x,b.y),v=ca?ca:cb;
        if(!(ca|cb)){line(r,a.x*16,a.y*16,b.x*16,b.y*16,PS_MESH_STROKE,c);return;}
        if(ca&cb)return;
        int x,y;
        if(v&12){
            y=(v&4?PS_TOP:PS_BOTTOM)*64;
            if(a.y==b.y)return;
            x=a.x+(b.x-a.x)*(y-a.y)/(b.y-a.y);
        }else{
            x=(v&1?PS_LEFT:PS_RIGHT)*64;
            if(a.x==b.x)return;
            y=a.y+(b.y-a.y)*(x-a.x)/(b.x-a.x);
        }
        if(ca){a.x=x;a.y=y;}else{b.x=x;b.y=y;}
    }
}
static void init(void *scr)
{
    (void)scr;
    /* Artwork has no mutable nodes, so no parsed-node pointer is needed. */
    int e=0;
    for(int k=0;k<PS_EDGES;k++){diagonal[k]=0;bend[k]=0;}
    for(int row=0;row<PS_ROWS;row++)for(int col=0;col<PS_COLS;col++){
        int i=row*PS_COLS+col;
        if(col<PS_COLS-1){edges[e][0]=i;edges[e++][1]=i+1;}
        if(row<PS_ROWS-1){edges[e][0]=i;edges[e++][1]=i+PS_COLS;}
        if(row<PS_ROWS-1&&col<PS_COLS-1){
            diagonal[e]=1;
            int flip=col<2?0:col>=PS_COLS-3?1:((row+col)&1);
            edges[e][0]=i+flip;
            edges[e++][1]=i+PS_COLS+1-flip;
        }
    }
    view_slot=-1; control_ready=0; q(4,0); ready=1; clock_last=cycles(); elapsed=25000000u;
}
static void reset_view(int slot,int selector)
{
    view_slot=slot; view_selector=selector;
    for(int i=0;i<PS_VOICES;i++){
        sequence[i]=ps_mail[i].sequence; /* discard all pre-entry telemetry */
        seen[i].serial=0; seen[i].pitch=0;seen[i].level=0;smooth_pitch[i]=0;
        excitation[i]=0;note_phase[i]=(uint32_t)i*0x23000000u;
        for(int j=0;j<PS_TRACE;j++)seen[i].trace[j]=0;
    }
    for(int e=0;e<PS_EDGES;e++)bend[e]=0;
}
static int pitch_spatial(uint32_t inc)
{
    if(!inc)return 2*256;
    uint32_t x=inc;unsigned n=0;
    while(x>1&&n<31){x>>=1;n++;}
    int fraction=n>=8?(int)((inc>>(n-8))&255):0;
    return ps_clamp(((int)n-18)*256+fraction,2*256,12*256);
}

static void geometry(const ps_params *p)
{
    for(int v=0;v<PS_VOICES;v++){
        ps_snapshot s;
        if(ps_read_mail(v,view_slot,view_selector,&s,&sequence[v])){
            if(s.serial!=seen[v].serial)excitation[v]=24576;
            seen[v]=s;
        }
        else {
            seen[v].level=(seen[v].level*3)>>2;
            for(int j=0;j<PS_TRACE;j++)seen[v].trace[j]=(seen[v].trace[j]*3)>>2;
        }
        int target=pitch_spatial(seen[v].pitch);
        int gain=ps_clamp(seen[v].level*64+excitation[v],0,32768);
        excitation[v]=(excitation[v]*3)>>2;
        /* Normalize only for the display; the stock audio envelope is untouched.
         * A bounded onset response remains visible during slow pad attacks. */
        visual_gain[v]=gain;
        int norm=32768/(seen[v].level+32);
        for(int j=0;j<PS_TRACE;j++)
            shape[v][j]=(int16_t)((ps_clamp(seen[v].trace[j]*norm,-32768,32767)*gain)>>15);
        note_phase[v]+=0x01800000u+(uint32_t)target*4096u;
        smooth_pitch[v]+=(target-smooth_pitch[v])/4;
    }
    uint32_t phase=ocean_phase,quiet=idle_phase>>21;
    int32_t morph=p->q[PS_MORPH], motion=16384+(p->q[PS_MOTION]>>1), thick=p->q[PS_THICK];
    uint32_t space=0x10000000u+(uint32_t)thick*4096u;
    uint32_t column_step=(space*4u)/5u; /* 8/10 original columns; bounded uint32 */
    uint32_t visual_ratio=(uint32_t)ps_interval_ratio(p->interval);
    for(int row=0;row<PS_ROWS;row++)for(int col=0;col<PS_COLS;col++){
        int i=row*PS_COLS+col;
        unsigned cx=(unsigned)col*112u;
        uint32_t pos=(uint32_t)col*column_step+(uint32_t)row*0x15000000u;
        uint32_t drift=phase;
        int base=rounded_wave(pos)>>5;
        int broad=base+((((rounded_wave(pos+drift)>>5)-base)*motion)>>15);
        base=rounded_wave(pos*2)>>5;
        int ridge=base+((((rounded_wave(pos*2+drift)>>5)-base)*motion)>>15);
        int swell=broad+(((ridge-broad)*morph)>>15);
        uint32_t paired=(uint32_t)(((uint64_t)pos*visual_ratio)>>15);
        int pairbase=rounded_wave(paired)>>6;
        int pair=pairbase+((((rounded_wave(paired-drift)>>6)-pairbase)*motion)>>15);
        swell+=(pair*thick)>>15;
        int noise=coherent(cx+(phase>>21),row*170+(phase>>21));
        if(p->character){
            noise=(noise+coherent(col*256+(phase>>20),row*110))/2;
        }
        swell+=(noise*motion)>>15;
        /* Geometry is independent of note telemetry. Orange zero now has
         * the former 50% visual motion; pitch/audio bend diagonals below. */
        int idle=coherent(cx+quiet,row*170+quiet);
        int height=(idle*(row+3)*3)>>2;
        int activity=(motion*motion)>>15;
        int waveamp=(activity*(row+3))>>13;
        height+=swell*waveamp;
        height=ps_clamp(height,-24*1024,24*1024);
        height=(height*ps_response[col])>>15;
        int side=col<PS_COLS/2?col:PS_COLS-1-col;
        if(row==0&&side==2)height=height*4/3;
        int x=160*1024+((2*col-(PS_COLS-1))*ps_width[row]*512)/(PS_COLS-1);
        int y=ps_depth[row]*1024-height;
        if(side<2){
            int edge_x=side?ps_shoulder_x[row]:ps_side_x[row];
            x=(col<PS_COLS/2?edge_x:320-edge_x)*1024;
            /* Top shoulders flow with the water; the corner tip stays quiet. */
            int response=row==0?(side?5243:655):row==1&&side?2621:1311;
            y=(side?ps_shoulder_y[row]:ps_side_y[row])*1024-
                (height*response)/(ps_response[col]);
        }else{
            x+=((((rounded_wave(pos)>>7)*(thick>>7))>>6)*ps_response[col])>>15;
        }
        if(row==0&&side==2)y-=3*1024;
        vertices[i].x=x;vertices[i].y=y;
        if(side==2&&row){
            int min_y=vertices[i-PS_COLS].y+6*1024;
            if(vertices[i].y<min_y)vertices[i].y=min_y;
        }
    }
    /* Only joining diagonals react to sound. A quadratic bow preserves the
     * endpoints; neither note level nor pitch can move the wave vertices. */
    for(int e=0;e<PS_EDGES;e++)if(diagonal[e]){
        int col=edges[e][0]%PS_COLS,row=edges[e][0]/PS_COLS,trace=0;
        for(int v=0;v<PS_VOICES;v++){
            uint32_t spatial=note_phase[v]+(uint32_t)(col+row)*
                (uint32_t)smooth_pitch[v]*0x00020000u;
            unsigned idx=spatial>>28;
            trace+=(shape[v][idx]>>2)+
                (((rounded_wave(spatial)>>2)*visual_gain[v])>>15);
        }
        int response=(ps_response[col]+ps_response[edges[e][1]%PS_COLS])/2;
        int target=(ps_clamp(trace,-14*1024,14*1024)*response)>>15;
        bend[e]+=(int16_t)((target-bend[e])/3);
    }

}
#ifdef VAPORA_HOST
/* Inspect production geometry at a fixed phase, without advancing the clock. */
void ps_test_geometry(const ps_params *p,op1_pt *points,int16_t *curves)
{
    geometry(p);
    for(int k=0;k<PS_ROWS*PS_COLS;k++)points[k]=vertices[k];
    for(int k=0;k<PS_EDGES;k++)curves[k]=bend[k];
}
#endif
/* Raw words detect physical edits; live modulation does not steal the readout.
 * Synth API 3.0.2 has page toggles but no press callback. Consume white's
 * page flag at the next GUI update, before reading any parameter values. */
static void track_controls(const int16_t *raw,int new_view,int white_press)
{
    if(new_view||!control_ready){
        q(4,0);
        active_knob=0;active_sun=0;
        for(int k=0;k<4;k++)if(k!=2&&op1_eng_page[k])active_knob=k;
        for(int k=0;k<8;k++)control_words[k]=raw[k];
        for(int k=0;k<4;k++)control_pages[k]=op1_eng_page[k];
        control_ready=1;
    }else{
        for(int k=0;k<4;k++){
            int word=k==0?5:k==1?6:7;
            if(raw[k]!=control_words[k]||(k!=2&&raw[word]!=control_words[word])){
                q(4,0);active_knob=k;active_sun=0;
            }
            if(k!=2&&op1_eng_page[k]!=control_pages[k]){
                q(k,op1_eng_page[k]);active_knob=k;active_sun=0;
            }
        }
        for(int k=0;k<8;k++)control_words[k]=raw[k];
        for(int k=0;k<4;k++)control_pages[k]=op1_eng_page[k];
    }
    if(white_press){q(2,1);sun_design=(sun_design+1)%PS_SUN_DESIGNS;active_knob=2;active_sun=1;}
}
#include "vapora_suns.h"
#ifdef VAPORA_HOST
void ps_test_draw_sun(int cy,int design){draw_sun(op1_raster(),cy,design,(56-cy)*32767/26);}
void ps_test_draw_sun_at(int cy,int design,int opening){draw_sun(op1_raster(),cy,design,opening);}
#endif
static void controls(void *r,const ps_params *p)
{
    static const char *const normal[4]={"MORPH","THICKNESS","BRIGHTNESS","MOTION"};
    int k=active_knob,q=p->q[k];
    char value[32],caption[48],*o=value;const char *label=normal[k],*s=0;
    if(active_sun){
        label="SUN";o=sq_num(o,(unsigned)ps_sun_ids[sun_design],2);o=sq_cat(o," ");sq_cat(o,ps_sun_names[sun_design]);
        q=sun_design*32767/(PS_SUN_DESIGNS-1);
    }else if(k==0&&op1_eng_page[k]){label="CHARACTER";s=ps_character_names[p->character];q=p->character*32767/(PS_CHARACTERS-1);}
    else if(k==1&&op1_eng_page[k]){label="INTERVAL";s=ps_interval_names[p->interval];q=p->interval*32767/(PS_INTERVALS-1);}
    else if(k==3&&op1_eng_page[k]){
        label="SPEED";q=p->q[PS_SPEED];int hz=ps_speed_millihz(q);
        o=sq_num(o,(unsigned)(hz/1000),1);*o++='.';
        o=sq_num(o,(unsigned)(hz%1000/10),2);sq_cat(o," HZ");
    }else {o=sq_num(o,(unsigned)((q*100+16383)/32767),1);sq_cat(o,"%");}
    if(s)sq_cat(value,s);
    o=sq_cat(caption,label);o=sq_cat(o,"  ");sq_cat(o,value);
    for(int n=0;caption[n];n++)if(caption[n]>='A'&&caption[n]<='Z')caption[n]+='a'-'A';
    /* The stock font uses lowercase codes for letters; uppercase codes are icons. */
    static const int32_t identity[9]={1024,0,0,0,1024,0,0,0,1024};
    if(!text_made[k]){op1_text_ctor(text_objects[k],34*1024,PS_LABEL_Y,PS_TEXT_SIZE,ps_colors[k]);text_made[k]=1;}
    int different=0,i=0;
    do {if(text_cache[k][i]!=caption[i])different=1;i++;}while(caption[i-1]&&i<48);
    if(different){for(i=0;i<48;i++){text_cache[k][i]=caption[i];if(!caption[i])break;}op1_text_set(text_objects[k],text_cache[k]);}
    op1_text_draw(text_objects[k],r,identity);
    /* FMbell-style 270-degree dial, coloured ring and moving indicator. */
    for(int j=4;j<28;j++){
        int a=(j+8)&31,b=(j+9)&31;
        line(r,16*1024+ps_circle[a][0]*10,PS_DIAL_Y*1024+ps_circle[a][1]*10,
             16*1024+ps_circle[b][0]*10,PS_DIAL_Y*1024+ps_circle[b][1]*10,PS_MESH_STROKE,ps_colors[k]);
    }
    unsigned phase=12u*1024u+(unsigned)q*24u*1024u/32767u;
    int a=(phase>>10)&31,b=(a+1)&31,f=phase&1023;
    int dx=ps_circle[a][0]+((ps_circle[b][0]-ps_circle[a][0])*f>>10);
    int dy=ps_circle[a][1]+((ps_circle[b][1]-ps_circle[a][1])*f>>10);
    line(r,16*1024+dx*3,PS_DIAL_Y*1024+dy*3,16*1024+dx*8,PS_DIAL_Y*1024+dy*8,PS_MESH_STROKE,ps_colors[k]);
}
#ifdef VAPORA_HOST
int ps_test_secret(void){return z==2;}
uint32_t ps_test_particle_phase(void){return particle_phase;}
int ps_test_sun_design(void){return sun_design;}
int ps_test_active_control(void){return active_sun?4:active_knob;}
#endif
static void update(void *scr,int dt)
{
    if(!ready)init(scr);
    int white_press=op1_eng_page[2]!=0;
    op1_eng_page[2]=0; /* acknowledge one-shot; white turns return to Brightness */
    int slot=OP1_SCREEN_SLOT&7;
    const int16_t *raw=op1_synth_knobs(slot);
    track_controls(raw,view_slot!=slot||view_selector!=raw[4],white_press);
    int16_t words[8];for(int i=0;i<8;i++)words[i]=raw[i];
    op1_synth_knobs_live(slot,words);
    ps_params p=ps_decode(words);
    if(view_slot!=slot||view_selector!=words[4])reset_view(slot,words[4]);
    uint32_t now=cycles(),delta=now-clock_last;clock_last=now;
    int secret=j(now,delta),tick=0;
    if(delta>100000000u)delta=100000000u;
    elapsed+=delta;
    if(elapsed>=25000000u){ /* 16 geometry updates/sec at the core's 400 MHz */
        uint32_t step=elapsed;elapsed=0;tick=1;
        idle_phase+=(uint32_t)(((uint64_t)step*429u)/1000u); /* 0.040 Hz */
        ocean_phase+=(uint32_t)(((uint64_t)step*ps_visual_speed_millihz(p.q[PS_SPEED])*10737u)/1000000u);
        /* High sun: half the former daytime drift; nighttime speed unchanged. */
        int night=32767-p.q[PS_BRIGHT];
        particle_phase+=(uint32_t)(((uint64_t)step*(160u+((unsigned)night*320u>>15)))/1000u);
        if(!secret)geometry(&p);
    }
    op1_screen_draw(scr,dt);
    void *r=op1_raster();if(!r)return;
    if(secret){v(r,tick);return;}
    int cy=ps_sun_y(p.q[PS_BRIGHT]);
    int night=32767-p.q[PS_BRIGHT];
    for(int k=0;k<PS_PARTICLES;k++){
        unsigned seed=(unsigned)k*0x9e3779b9u+0x53544152u;
        seed^=seed>>16;seed*=0x7feb352du;seed^=seed>>15;
        int x=5+(int)(((uint64_t)(uint32_t)(seed+particle_phase)*307u)>>32);
        int wander=coherent((particle_phase>>20)+(seed&1023),(seed>>20)&1023);
        int y=7+(int)((seed>>16)%43);
        y+=(rounded_wave(particle_phase+(unsigned)k*0x23000000u)>>14);
        y+=(wander*(2+(night>>13)))>>10;
        y=ps_clamp(y,5,55);
        int dx=x-160,dy=y-cy;
        int radius=ps_sun_ids[sun_design]==18?30:ps_sun_ids[sun_design]==2?24:ps_sun_ids[sun_design]==5?22:18;
        int light=ps_clamp((dx*dx+dy*dy-radius*radius)*6,0,255);
        light=(light*(192+(night>>9)))>>8;
        int size=k%6,large=size==0,medium=size==1||size==2;
        uint8_t dot[4]={(uint8_t)(((large?170:medium?140:115)*light)>>8),
            (uint8_t)(((large?220:medium?195:175)*light)>>8),(uint8_t)light,255};
        line(r,x*1024,y*1024,x*1024+(large?1200:medium?800:650),y*1024,
             large?PS_BIG_DOT_STROKE:medium?PS_MED_DOT_STROKE:PS_DOT_STROKE,dot);
    }
    draw_sun(r,cy,sun_design,p.q[PS_BRIGHT]);
    for(int e=0;e<PS_EDGES;e++){
        int row=edges[e][0]/PS_COLS,crest=vertices[edges[e][0]].y<ps_depth[row]*1024;
        int b=p.q[PS_BRIGHT]>>8;
        uint8_t color[4]={(uint8_t)(8+crest*(15+(b>>2))),
            (uint8_t)(70+row*10+(b>>1)+crest*35),
            (uint8_t)(165+row*7+(b>>2)),255};
        op1_pt a=vertices[edges[e][0]],z=vertices[edges[e][1]];
        if(diagonal[e]&&(bend[e]>128||bend[e]<-128)){
            op1_pt m={a.x+(z.x-a.x)/3,a.y+(z.y-a.y)/3+bend[e]*4/9};
            op1_pt n={a.x+(z.x-a.x)*2/3,a.y+(z.y-a.y)*2/3+bend[e]*4/9};
            waterline(r,a,m,color);waterline(r,m,n,color);waterline(r,n,z,color);
        }else waterline(r,a,z,color);
    }
    controls(r,&p);
}
const op1_screen_t SCREEN_vapora={init,update};
