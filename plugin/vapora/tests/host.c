#include "native_mock.h"
#include "../vapora_dsp.h"
#include "../vapora_mailbox.h"
#include "../vapora_visual.h"
#include "preset_words.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <limits.h>

uint32_t ps_host_voices[6][9],ps_host_cycles;
int32_t ps_host_pitch[128];
int16_t ps_host_env[128],ps_host_words[8][8],ps_host_velocity[6];
int ps_host_slot;
volatile uint8_t op1_eng_page[4];
static FILE *drawing;
static unsigned drawn_lines,particle_light;
static const int16_t defaults[8]={9000,26000,18500,12500,0x5320,0,0,0};
void op1_screen_draw(void *s,int dt){(void)s;(void)dt;}
void *op1_raster(void){return (void*)1;}
void op1_raster_line(void *r,const op1_pt *a,const op1_pt *b,int32_t w,const uint8_t *c)
{
    (void)r;
    assert(a->x>=0&&a->x<320*1024&&a->y>=0&&a->y<153*1024);
    assert(b->x>=0&&b->x<320*1024&&b->y>=0&&b->y<153*1024);
    assert(w>0&&w<=PS_BIG_DOT_STROKE);drawn_lines++;
    if(w==PS_DOT_STROKE||w==PS_MED_DOT_STROKE||w==PS_BIG_DOT_STROKE)particle_light+=c[2];
    if(drawing)fprintf(drawing,"<line x1=\"%.4f\" y1=\"%.4f\" x2=\"%.4f\" y2=\"%.4f\" stroke=\"#%02x%02x%02x\" stroke-width=\"%.4f\"/>\n",
        a->x/1024.,a->y/1024.,b->x/1024.,b->y/1024.,c[0],c[1],c[2],w/1024.);
}
/* No TE font is extracted. Record stock text API arguments for host preview. */
typedef struct {int x,y,size;uint8_t c[4];char text[48];} host_text;
void op1_text_ctor(void *obj,int32_t x,int y,int size,const uint8_t *c)
{
    host_text *t=obj; t->x=x;t->y=y;t->size=size;memcpy(t->c,c,4);t->text[0]=0;
    assert(sizeof(*t)<=OP1_TEXT_SIZE&&(x==34*1024||x==0)&&size==12);
}
void op1_text_set(void *obj,const char *s)
{host_text *t=obj;assert(strlen(s)<sizeof t->text);for(int i=0;s[i];i++)assert(s[i]<'A'||s[i]>'Z');strcpy(t->text,s);}
void op1_text_draw(void *obj,void *r,const int32_t *m)
{
    (void)r;host_text *t=obj;int x=t->x+m[6],y=t->y+(m[7]>>10);
    assert(x>=0&&x/1024+(int)strlen(t->text)*8<320&&y>=0&&y+t->size<153);
    if(drawing){
        fprintf(drawing,"<text x=\"%.4f\" y=\"%d\" font-size=\"%d\" fill=\"#%02x%02x%02x\">",x/1024.,y+t->size,t->size,t->c[0],t->c[1],t->c[2]);
        for(const char *c=t->text;*c;c++){
            if(*c=='<')fputs("&lt;",drawing);else if(*c=='&')fputs("&amp;",drawing);
            else fputc(*c>='a'&&*c<='z'?*c-'a'+'A':*c,drawing);
        }
        fputs("</text>\n",drawing);
    }
}
static uint32_t rng=12345;
static uint32_t random32(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static int32_t pitch_hz(double hz){return (int32_t)(hz*4294967296./44100.);}
static void set_pitch(double hz)
{for(int n=0;n<128;n++)ps_host_pitch[n]=pitch_hz(hz);}
static void reset(void)
{
    memcpy(ps_host_words[ps_host_slot],defaults,sizeof defaults);
    for(int i=0;i<6;i++){ps_host_voices[i][0]=0xdeadbeef;ps_host_velocity[i]=32767;}
    for(int i=0;i<128;i++)ps_host_env[i]=32767;
}
static void params_test(void)
{
    int16_t p[8];memcpy(p,defaults,sizeof p);
    ps_params a=ps_decode(p);
    assert(a.character==0&&a.interval==PS_UNISON&&ps_speed_millihz(a.q[PS_SPEED])==143);
    for(int w=5;w<8;w++)p[w]=8192;
    ps_params b=ps_decode(p);
    for(int k=0;k<PS_CONTROLS;k++)assert(a.q[k]==b.q[k]);
    assert(a.character==b.character&&a.interval==b.interval);
    for(int d=-1;d<=1;d+=2){
        for(int k=5;k<8;k++)p[k]=8192+d; /* actual handler delta +/-4 */
        b=ps_decode(p);assert(b.character==0&&b.interval==PS_UNISON);
        assert(abs(ps_speed_millihz(b.q[PS_SPEED])-143)<=1);
    }
    for(int k=5;k<8;k++)p[k]=1;
    b=ps_decode(p);assert(b.q[PS_SPEED]==1&&b.character==(PS_CHARACTERS-32%PS_CHARACTERS)%PS_CHARACTERS&&b.interval==0);
    for(int k=0;k<8;k++)p[k]=INT16_MIN;
    b=ps_decode(p);assert(b.q[0]==0&&b.q[PS_SPEED]==8192);
    assert(ps_interval_ratio(PS_UNISON)==32768&&ps_interval_ratio(PS_OCTAVE)==65536);
    assert(fabs(ps_interval_ratio(PS_FIFTH)/32768.-pow(2,7./12))<.00002);
    for(int i=0;i<PS_INTERVALS;i++){
        assert(fabs(ps_interval_ratio(i)/32768.-pow(2,ps_semitones[i]/12.))<.00002);
        p[6]=(int16_t)((i*32768+16384)/PS_INTERVALS);
        assert(ps_decode(p).interval==i);
    }
    p[6]=16384;assert(ps_decode(p).interval==PS_FIFTH);
    assert(ps_visual_speed_millihz(32767)<=540&&ps_visual_speed_millihz(32767)>=538);
    for(double hz=8.1758;hz<16000;hz*=1.05946){
        int32_t pi=pitch_hz(hz);
        double decoded=ps_loop_increment(pi)*256.*44100./4294967296.;
        assert(fabs(decoded-hz)<.0027);
    }
    assert(ps_loop_increment(INT32_MIN)==0&&ps_loop_increment(-1)==0);
    for(int l=0;l<7;l++){
        uint32_t edge=1u<<(18+l);int32_t m;
        assert(ps_level_for(edge-1,&m)==l&&m>=32760);
        assert(ps_level_for(edge,&m)==l+1&&m==0);
    }
}
static void stress(void)
{
    struct {uint32_t a;int16_t samples[128];uint32_t b;} guard={0x1234,{0},0x5678};
    reset();set_pitch(440);
    for(int v=0;v<6;v++)ENGINE_vapora.note_on(ps_host_voices[v],ps_host_words[0]);
    clock_t t=clock();
    for(int block=0;block<3000;block++){
        int16_t *w=ps_host_words[0];
        if(block%11==0)for(int k=0;k<8;k++)if(k!=4)w[k]=(int16_t)(random32()&32767);
        if(block%17==0)ENGINE_vapora.note_on(ps_host_voices[block%6],w);
        for(int n=0;n<128;n++){
            ps_host_pitch[n]=block%97==0?INT32_MAX:block%101==0?INT32_MIN:
                pitch_hz(20+((block*37+n)%11000));
            ps_host_env[n]=(int16_t)(random32()&32767);
        }
        for(int v=0;v<6;v++){
            ENGINE_vapora.render(ps_host_voices[v],guard.samples,w);
            assert(ps_host_voices[v][0]==0xdeadbeef);
            for(int n=0;n<128;n++)assert(abs(guard.samples[n])<5000);
        }
        assert(guard.a==0x1234&&guard.b==0x5678);
        if(block%19==0){
            ps_host_cycles+=25000000; SCREEN_vapora.update(0,16);
        }
    }
    double seconds=(clock()-t)/(double)CLOCKS_PER_SEC;
    printf("Stress: 3000 six-voice blocks, randomized controls/pitch/retriggers, %.3f host CPU seconds (not Blackfin timing)\n",seconds);
    ENGINE_vapora.render((void*)(OP1_VOICE_BASE+1),guard.samples,ps_host_words[0]);
    for(int n=0;n<128;n++)assert(!guard.samples[n]);
    ps_host_voices[0][1]=0;ENGINE_vapora.render(ps_host_voices[0],guard.samples,ps_host_words[0]);
    for(int n=0;n<128;n++)assert(!guard.samples[n]);
    /* Reader never consumes an incomplete or foreign publication. */
    ps_snapshot s;uint32_t seq=0;
    ps_mail[0].sequence=3;assert(!ps_read_mail(0,0,0x5320,&s,&seq));
    ps_mail[0].sequence=4;ps_mail[0].slot=1;assert(!ps_read_mail(0,0,0x5320,&s,&seq));
    ps_mail[0].slot=0;ps_mail[0].selector=0x5321;assert(!ps_read_mail(0,0,0x5320,&s,&seq));
    ps_mail[0].selector=0x5320;assert(ps_read_mail(0,0,0x5320,&s,&seq));
}
static void sound_contract(void)
{
    int16_t words[8];memcpy(words,defaults,sizeof words);
    words[3]=0;words[7]=1;
    ps_params p=ps_decode(words);ps_voice a,b;
    ps_start(&a,&p,123,1);ps_start(&b,&p,123,1);
    int16_t x[128],y[128];set_pitch(440);
    /* Speed must be inaudible when Motion is zero. */
    ps_params fast=p;fast.q[PS_SPEED]=32767;
    for(int i=0;i<200;i++){
        ps_render(&a,x,ps_host_pitch,32767,&p);ps_render(&b,y,ps_host_pitch,32767,&fast);
        assert(!memcmp(x,y,sizeof x));
        for(int n=0;n<128;n++)assert(abs(x[n])*6<32768);
    }
    /* Character crossfade and Interval gating remain bounded under reversals. */
    int prev=0,largest=0,flips=0;
    for(int block=0;block<1000;block++){
        p.character=(block/9)%PS_CHARACTERS;p.interval=(block/13)%PS_INTERVALS;
        p.q[PS_MORPH]=(block&1)?32767:0;
        p.q[PS_BRIGHT]=32767;p.q[PS_THICK]=32767;
        int before=a.interval, beforegain=a.aux_gain;
        ps_render(&a,x,ps_host_pitch,32767,&p);
        if(a.interval!=before){assert(beforegain==0);flips++;}
        for(int n=0;n<128;n++){
            int d=abs(x[n]-prev);if(d>largest)largest=d;prev=x[n];
            assert(abs(x[n])*6<32768);
        }
    }
    assert(flips>20&&largest<3000);
    printf("Sound contract: Motion=0 speed invariance; coincident six-voice headroom; %d gated interval changes; largest sample step %d\n",flips,largest);
    /* Sweep level boundaries in both directions, with an audible octave head.
     * Guarded old and new banks must remain safe throughout each normal fade. */
    p=ps_decode(defaults);p.interval=PS_OCTAVE;p.q[PS_BRIGHT]=32767;
    ps_start(&a,&p,100,1);largest=prev=0;int fades=0;
    for(int block=0;block<3000;block++){
        double semitones=60.*(block<1500?block:3000-block)/1500.;
        set_pitch(65.4064*pow(2,semitones/12.));
        ps_render(&a,x,ps_host_pitch,32767,&p);
        uint32_t inc=ps_loop_increment(ps_host_pitch[0]);
        uint32_t bound=inc*4+(inc>>4);
        assert(bound<=(1u<<(18+a.bank_lod)));
        if(a.bank_mix<32768){
            assert(bound<=(1u<<(18+a.old_lod)));fades++;
        }
        for(int n=0;n<128;n++){
            int step=abs(x[n]-prev);if(step>largest)largest=step;prev=x[n];
        }
    }
    assert(fades>100&&largest<3000);
    printf("Pitch sweep: %d safe crossfade blocks; largest sample step %d\n",fades,largest);
    /* Three-minute held chord on exactly the production DSP; no huge fixture. */
    ps_voice held[6];ps_params slow=ps_decode(defaults);
    int32_t chordpitch[6][128];
    for(int v=0;v<6;v++){
        ps_start(&held[v],&slow,100u+(unsigned)v,(unsigned)v+1);
        for(int n=0;n<128;n++)chordpitch[v][n]=pitch_hz(130.8128*pow(2,v*3./12));
    }
    for(int block=0;block<62016;block++)for(int v=0;v<6;v++){
        ps_render(&held[v],x,chordpitch[v],32767,&slow);
        assert(abs(held[v].lp1)<30000&&abs(held[v].lp2)<30000);
        for(int n=0;n<128;n++)assert(abs(x[n])*6<32768);
    }
    puts("Three-minute six-voice sustained-state check: PASS (host, no FX)");
    int16_t released[128]={0};ps_snapshot tail;
    ps_analyze(&held[0],x,released,(uint32_t)chordpitch[0][0],&tail);
    assert(tail.level==0);
    for(int n=0;n<PS_TRACE;n++)assert(tail.trace[n]==0);
}
static void fixture(const char *dir,const char *name,int which)
{
    char path[1024];snprintf(path,sizeof path,"%s/%s.pcm",dir,name);
    FILE *f=fopen(path,"wb");assert(f);
    reset();
    int16_t *w=ps_host_words[0];
    int count=which==1?6:1;
    double length=which==0?16:which==1?18:12;
    const int notes[6]={48,55,60,64,67,72};
    if(which==3){w[5]=24000;w[6]=30000;w[2]=32767;}
    if(which>=4){memcpy(w,preset_words[which-4],sizeof defaults);length=16;}
    for(int v=0;v<count;v++)ENGINE_vapora.note_on(ps_host_voices[v],w);
    int clips=0,peak=0;double sq=0;long frames=0;
    for(int b=0;b<(int)(length*44100/128);b++){
        double sec=b*128./44100.;
        if(which==2){
            double t=sec/length;
            w[0]=(int16_t)(32767*t);w[1]=(int16_t)(32767*(1-t));
            w[2]=(int16_t)(32767*t);w[3]=(int16_t)(32767*t);
            w[5]=sec<4?8192:sec<8?24576:8192;w[6]=sec<3?8192:sec<6?16384:30000;
            w[7]=(int16_t)(1+32766*t);
        }
        int32_t sum[128]={0};
        for(int v=0;v<count;v++){
            int note=which==1?notes[v]:which==3?96:60;
            if(which>=4)note+=12*preset_octave[which-4];
            set_pitch(440*pow(2,(note-69)/12.));
            for(int n=0;n<128;n++){
                double t=(b*128+n)/44100.;
                double env=fmin(1,t/1.5)*fmin(1,fmax(0,(length-t)/2.5));
                if(which>=4){
                    const int32_t *e=preset_preview[which-4];
                    double at=e[0]/1000.,de=e[1]/1000.,su=e[2]/32767.;
                    env=t<at?t/at:t<at+de?1-(1-su)*(t-at)/de:su;
                    env*=fmin(1,fmax(0,(length-t)/(e[3]/1000.)));
                }
                ps_host_env[n]=(int16_t)(env*32767);
            }
            int16_t out[128];ENGINE_vapora.render(ps_host_voices[v],out,w);
            for(int n=0;n<128;n++)sum[n]+=(out[n]*ps_host_env[n])>>15;
        }
        int16_t out[128];
        for(int n=0;n<128;n++){
            if(sum[n]>32767||sum[n]<-32768)clips++;
            out[n]=(int16_t)ps_clamp(sum[n],-32768,32767);
            if(abs(out[n])>peak)peak=abs(out[n]);
            sq+=(double)out[n]*out[n];frames++;
        }
        assert(fwrite(out,sizeof *out,128,f)==128);
    }
    fclose(f);assert(!clips);
    assert(peak>100&&sq/frames>100); /* Reject accidental silent/near-silent banks. */
    printf("Fixture %s: peak %d, RMS %.1f, clipping %d\n",name,peak,sqrt(sq/frames),clips);
}
static void previews(const char *dir)
{
    const char *names[]={"minimum","default","maximum","character","interval","speed","all-alternates","voice-onset","single-low","single-semitone","single-high"};
    for(int s=0;s<11;s++){
        reset();for(int k=0;k<4;k++)op1_eng_page[k]=0;
        if(s==0)for(int k=0;k<8;k++)if(k!=4)ps_host_words[0][k]=k<4?0:1;
        if(s==2||s==7)for(int k=0;k<8;k++)if(k!=4)ps_host_words[0][k]=32767;
        if(s==3||s==6)op1_eng_page[0]=1;
        if(s==4||s==6)op1_eng_page[1]=1;
        if(s==5||s==6)op1_eng_page[3]=1;
        if(s==6){ps_host_words[0][5]=25000;ps_host_words[0][6]=30000;ps_host_words[0][7]=32767;}
        SCREEN_vapora.init(0);
        ps_host_cycles+=25000000;SCREEN_vapora.update(0,16);
        for(int frame=0;frame<32;frame++){
            if(s==7||s>=8){
                int voices=s==7?6:1;
                set_pitch(s==8?130.8128:s==9?138.5913:1046.5);int16_t out[128];
                if(!frame)for(int v=0;v<voices;v++)ENGINE_vapora.note_on(ps_host_voices[v],ps_host_words[0]);
                for(int b=0;b<22;b++)for(int v=0;v<voices;v++)ENGINE_vapora.render(ps_host_voices[v],out,ps_host_words[0]);
            }
            if((s==7||s==0||s==1||s>=8)&&frame<24){
                const char *prefix=s==0?"idle":s==1?"default-idle":s>=8?names[s]:"animation";
                char path[1024];snprintf(path,sizeof path,"%s/%s-%02d.svg",dir,prefix,frame);
                drawing=fopen(path,"w");assert(drawing);
                fputs("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"320\" height=\"160\" viewBox=\"0 0 320 160\">\n",drawing);
            }
            ps_host_cycles+=25000000;SCREEN_vapora.update(0,16);
            if(drawing){fputs("</svg>\n",drawing);fclose(drawing);drawing=0;}
        }
        char path[1024];snprintf(path,sizeof path,"%s/%s.svg",dir,names[s]);
        drawing=fopen(path,"w");assert(drawing);
        fputs("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"320\" height=\"160\" viewBox=\"0 0 320 160\">\n",drawing);
        drawn_lines=0;
        ps_host_cycles+=25000000;SCREEN_vapora.update(0,16);
        fputs("</svg>\n",drawing);fclose(drawing);drawing=0;
        assert(drawn_lines<650);
        printf("Preview %s: %u overlay strokes, all inside 320x153\n",names[s],drawn_lines);
    }
    /* Animated extremes + every independent page combination, slot returns. */
    for(int n=0;n<500;n++){
        ps_host_slot=n%8;reset();
        for(int k=0;k<8;k++)if(k!=4)ps_host_words[ps_host_slot][k]=(int16_t)(random32()&32767);
        for(int k=0;k<4;k++)op1_eng_page[k]=(n>>k)&1;
        ps_host_cycles+=25000000;SCREEN_vapora.update(0,16);
    }
    ps_host_slot=0;
}
extern int ps_test_sun_design(void),ps_test_active_control(void);
extern uint32_t ps_test_particle_phase(void);
extern void ps_test_draw_sun(int cy,int design);
extern void ps_test_draw_sun_at(int cy,int design,int opening);
static void sun_motion_contract(const char *dir)
{
    assert(ps_sun_y(0)==56&&ps_sun_y(32767)==30);
    for(int design=0;design<PS_SUN_DESIGNS;design++)for(int cy=30;cy<=56;cy++){
        char path[1024];snprintf(path,sizeof path,"%s/sunmove-%02d-%02d.svg",dir,ps_sun_ids[design],cy);
        drawing=fopen(path,"w");assert(drawing);
        fputs("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"320\" height=\"160\">\n",drawing);
        drawn_lines=0;ps_test_draw_sun(cy,design);assert(drawn_lines<=100);
        fputs("</svg>\n",drawing);fclose(drawing);drawing=0;
        snprintf(path,sizeof path,"%s/sunfixed-%02d-%02d.svg",dir,ps_sun_ids[design],cy);
        drawing=fopen(path,"w");assert(drawing);
        fputs("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"320\" height=\"160\">\n",drawing);
        drawn_lines=0;ps_test_draw_sun_at(cy,design,32767);assert(drawn_lines<=100);
        fputs("</svg>\n",drawing);fclose(drawing);drawing=0;
    }
}
static void controls_contract(const char *dir)
{
    reset();for(int k=0;k<4;k++)op1_eng_page[k]=0;
    SCREEN_vapora.init(0);SCREEN_vapora.update(0,16);
    int initial=ps_test_sun_design();int16_t before[8];memcpy(before,ps_host_words[0],sizeof before);
    for(int i=0;i<PS_SUN_DESIGNS;i++){
        int previous=ps_test_sun_design();op1_eng_page[2]=1;SCREEN_vapora.update(0,16);
        assert(op1_eng_page[2]==0&&ps_test_sun_design()==(previous+1)%PS_SUN_DESIGNS&&ps_test_active_control()==4);
        assert(!memcmp(before,ps_host_words[0],sizeof before));
        int style=ps_test_sun_design();char path[1024];snprintf(path,sizeof path,"%s/sun-%02d.svg",dir,ps_sun_ids[style]);
        drawing=fopen(path,"w");assert(drawing);
        fputs("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"320\" height=\"160\">\n",drawing);
        drawn_lines=0;SCREEN_vapora.update(0,16);assert(drawn_lines<550);
        fputs("</svg>\n",drawing);fclose(drawing);drawing=0;
    }
    assert(ps_test_sun_design()==initial);
    for(int k=0;k<4;k++){
        ps_host_words[0][k]+=256;SCREEN_vapora.update(0,16);assert(ps_test_active_control()==k);
        if(k!=2){op1_eng_page[k]^=1;SCREEN_vapora.update(0,16);assert(ps_test_active_control()==k);}
    }
    for(int family=0;family<PS_CHARACTERS;family++)for(int direction=-1;direction<=1;direction+=2){
        int16_t w[8];memcpy(w,defaults,sizeof w);w[5]=(int16_t)(8192+family*256);
        assert(ps_decode(w).character==family);w[5]+=direction*256;
        assert(ps_decode(w).character==(family+direction+PS_CHARACTERS)%PS_CHARACTERS);
    }
    for(int n=0;n<PRESET_COUNT;n++)assert(ps_decode(preset_words[n]).character==(preset_words[n][5]-8192)/256);
    uint32_t phase=ps_test_particle_phase();ps_host_words[0][2]=0;ps_host_cycles+=25000000;
    particle_light=0;SCREEN_vapora.update(0,16);
    uint32_t night_step=ps_test_particle_phase()-phase;unsigned night_light=particle_light;
    phase=ps_test_particle_phase();ps_host_words[0][2]=32767;ps_host_cycles+=25000000;
    particle_light=0;SCREEN_vapora.update(0,16);
    uint32_t day_step=ps_test_particle_phase()-phase;
    assert(night_step>day_step*2&&night_light>particle_light);
    printf("Particles: low-sun step %u / high-sun step %u; sky brightness %u / %u\n",night_step,day_step,night_light,particle_light);
    puts("Controls: 6 sun steps/wrap, original IDs 05/01/02/03/10/18; saved words unchanged by presses; active turn/page; Character changes in one detent; core preset Character mappings");
}
extern int ps_test_secret(void);
static void secret_setup(void)
{
    ps_host_slot=0;reset();for(int k=0;k<4;k++)op1_eng_page[k]=0;
    SCREEN_vapora.init(0);SCREEN_vapora.update(0,16);
}
static void secret_press(int k)
{op1_eng_page[k]^=1;ps_host_cycles+=25000000;SCREEN_vapora.update(0,16);}
static void secret_arm(void)
{secret_press(0);secret_press(1);secret_press(3);secret_press(2);}
static void secret_wait(int frames)
{for(int i=0;i<frames;i++){ps_host_cycles+=25000000;SCREEN_vapora.update(0,16);}}
static void secret_contract(const char *dir)
{
    secret_setup();secret_arm();secret_wait(111);assert(!ps_test_secret());
    secret_wait(1);assert(ps_test_secret());
    for(int i=0;i<96;i++){
        char path[1024];snprintf(path,sizeof path,"%s/secret-%02d.svg",dir,i);
        drawing=fopen(path,"w");assert(drawing);
        fputs("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"320\" height=\"160\">\n",drawing);
        drawn_lines=0;secret_wait(1);assert(ps_test_secret()&&!drawn_lines);
        fputs("</svg>\n",drawing);fclose(drawing);drawing=0;
    }
    ps_host_words[0][1]++;secret_wait(1);assert(!ps_test_secret());
    secret_setup();secret_arm();secret_wait(110);ps_host_words[0][0]++;secret_wait(10);assert(!ps_test_secret());
    secret_setup();secret_press(1);secret_press(0);secret_press(3);secret_press(2);secret_wait(120);assert(!ps_test_secret());
    ps_host_cycles=UINT32_MAX-500000000u;secret_setup();secret_arm();secret_wait(112);assert(ps_test_secret());
    secret_press(0);assert(!ps_test_secret());
    secret_setup();secret_arm();ps_host_cycles+=2800000000u;SCREEN_vapora.update(0,16);assert(!ps_test_secret());
    puts("Secret: ordered four-click sequence, exact 7-second boundary, movement cancellation, exit, cycle wrap, and hidden-screen gap; 96 frames of three bounded native text objects");
}
extern void ps_test_geometry(const ps_params *,op1_pt *,int16_t *);
static void geometry_contract(void)
{
    reset();SCREEN_vapora.init(0);SCREEN_vapora.update(0,16);
    ps_params p=ps_decode(defaults);
    op1_pt quiet[PS_ROWS*PS_COLS],playing[PS_ROWS*PS_COLS];
    int16_t curves[PS_EDGES],low[PS_EDGES],high[PS_EDGES];
    ps_test_geometry(&p,quiet,curves);
    for(int octave=0;octave<2;octave++){
        for(int frame=0;frame<40;frame++){
            ps_mailbox *m=&ps_mail[0];
            m->sequence=(m->sequence+2)&~1u;
            m->slot=0;m->selector=defaults[4];
            m->snapshot.pitch=(uint32_t)pitch_hz(octave?1046.5:130.8128);
            m->snapshot.level=2000;
            for(int k=0;k<PS_TRACE;k++)m->snapshot.trace[k]=(int16_t)((k-8)*200);
            ps_test_geometry(&p,playing,curves);
            assert(!memcmp(quiet,playing,sizeof quiet));
        }
        memcpy(octave?high:low,curves,sizeof curves);
    }
    int changed=0,curved=0;
    for(int e=0;e<PS_EDGES;e++){
        changed+=low[e]!=high[e];curved+=abs(high[e])>128;
    }
    assert(changed>10&&curved>10);
    /* Real adapter/DSP publication: a single quiet-to-full note must visibly
     * bend several central diagonals, including a long pad-style attack. */
    reset();SCREEN_vapora.init(0);SCREEN_vapora.update(0,16);
    set_pitch(261.625565);ENGINE_vapora.note_on(ps_host_voices[0],ps_host_words[0]);
    int16_t out[128];int visible=0;
    for(int frame=0;frame<32;frame++){
        for(int n=0;n<128;n++)ps_host_env[n]=(int16_t)(32767*frame/31);
        for(int block=0;block<22;block++)ENGINE_vapora.render(ps_host_voices[0],out,ps_host_words[0]);
        ps_test_geometry(&p,playing,curves);
        for(int e=0;e<PS_EDGES;e++)if(abs(curves[e])>4608)visible++;
    }
    assert(visible>20); /* >2 px rendered bow (4/9 of bend). */
    /* Side cells must retain their orientation under knob extremes. */
    for(int frame=0;frame<500;frame++){
        int16_t w[8];memcpy(w,defaults,sizeof w);
        for(int k=0;k<8;k++)if(k!=4)w[k]=(int16_t)(random32()&32767);
        memcpy(ps_host_words[0],w,sizeof w);ps_host_cycles+=25000000;
        SCREEN_vapora.update(0,16);
        ps_params extreme=ps_decode(w);ps_test_geometry(&extreme,playing,curves);
        for(int row=0;row<PS_ROWS-1;row++)for(int col=0;col<PS_COLS-1;col++){
            if(col>1&&col<PS_COLS-3)continue;
            int i=row*PS_COLS+col;
            op1_pt a=playing[i],b=playing[i+1],c=playing[i+PS_COLS],d=playing[i+PS_COLS+1];
            int flip=col<2?0:1;
            int64_t area1=flip?(int64_t)(b.x-a.x)*(c.y-a.y)-(int64_t)(b.y-a.y)*(c.x-a.x):
                (int64_t)(b.x-a.x)*(d.y-a.y)-(int64_t)(b.y-a.y)*(d.x-a.x);
            int64_t area2=flip?(int64_t)(d.x-b.x)*(c.y-b.y)-(int64_t)(d.y-b.y)*(c.x-b.x):
                (int64_t)(d.x-a.x)*(c.y-a.y)-(int64_t)(d.y-a.y)*(c.x-a.x);
            assert(area1>0&&area2>0);
        }
    }
    puts("Visual note contract: fixed wave vertices; single-note bows exceed 2 px; pitch changes curvature; side triangles stay unfolded");
}
int main(int argc,char **argv)
{
    assert(argc==2);reset();params_test();sound_contract();SCREEN_vapora.init(0);stress();
    fixture(argv[1],"held-c4",0);fixture(argv[1],"six-note-pad",1);
    fixture(argv[1],"macro-sweep",2);fixture(argv[1],"high-octave",3);
    for(int i=0;i<PRESET_COUNT;i++){
        char name[100];snprintf(name,sizeof name,"preset-%02d-%s",i+1,preset_names[i]);
        fixture(argv[1],name,i+4);
    }
    geometry_contract();previews(argv[1]);controls_contract(argv[1]);sun_motion_contract(argv[1]);secret_contract(argv[1]);puts("PASS: host DSP/adapter/mailbox/screen checks");
    return 0;
}
