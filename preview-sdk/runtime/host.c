#include "preview.h"
#include <string.h>
#ifndef PREVIEW_KIND
#define PREVIEW_KIND 0
#endif
static float inputs[2][PV_BLOCK],outputs[2][PV_BLOCK];
static pv_command commands[PV_MAX_COMMANDS];
static int command_count,overflow;
static uint8_t snapshot[PV_SNAPSHOT_BYTES];
static uint32_t serial;
static float attack=.3f,decay=1.f,sustain=.85f,release=2.f;
/* Fade an occupied voice for 256 samples (5.8 ms) before resetting its DSP. */
#define REPLACE_FADE 256
typedef struct {
    int note,stage,velocity;uint32_t pitch,age;float level,release_step;
    int pending,next_note,next_velocity,fade_left;uint32_t next_pitch;
} voice;
static voice voices[PV_MAX_VOICES];
static int16_t envelope[PV_BLOCK],rendered[PV_BLOCK];
static float clamp(float x,float lo,float hi){return x!=x?lo:x<lo?lo:x>hi?hi:x;}
int pv_abi(void){return 1;}
int pv_kind(void){return PREVIEW_KIND;}
int pv_rate(void){return PV_RATE;}
int pv_block(void){return PV_BLOCK;}
void pv_init(void){memset(voices,0,sizeof voices);serial=0;plugin_init();}
void *pv_input(void){return inputs;}
void *pv_output(void){return outputs;}
void *pv_commands(void){return commands;}
void *pv_snapshot_buffer(void){return snapshot;}
int pv_snapshot_size(void){int n=plugin_snapshot(snapshot);return n>=0&&n<=PV_SNAPSHOT_BYTES?n:0;}
void pv_receive_snapshot(void){plugin_receive(snapshot);}
int pv_overflow(void){return overflow;}
int pv_active(void){int n=0;for(int i=0;i<PV_MAX_VOICES;i++)n+=voices[i].stage!=0||voices[i].pending;return n;}
void pv_parameter(int id,float value){if(id>=0&&id<64)plugin_parameter(id,clamp(value,0,1));}
void pv_action(int id,int value){if(id>=0&&id<64)plugin_action(id,value);}
void pv_envelope(float a,float d,float s,float r){
    attack=clamp(a,.001f,20);decay=clamp(d,.001f,20);sustain=clamp(s,0,1);release=clamp(r,.005f,20);
}
void pv_panic(void){
    for(int i=0;i<PV_MAX_VOICES;i++)plugin_release(i);
    memset(voices,0,sizeof voices);
    memset(outputs,0,sizeof outputs);
}
void pv_note_off(int note){for(int i=0;i<PV_MAX_VOICES;i++){
    voice *v=&voices[i];
    if(v->pending&&v->next_note==note)v->pending=0;
    if(v->stage&&v->note==note&&v->stage!=4&&v->stage!=5){
        v->stage=4;v->release_step=v->level/(release*PV_RATE);plugin_release(i);
    }
}}
static void start_voice(int i,int note,int velocity,uint32_t pitch,uint32_t age){
    voices[i]=(voice){.note=note,.stage=1,.velocity=velocity,.pitch=pitch,.age=age};
    plugin_start(i,velocity);
}
void pv_note_on(int note,float velocity,uint32_t pitch){
    if(PREVIEW_KIND||note<0||note>127||velocity<=0)return;
    int pick=-1;
    for(int i=0;i<PV_MAX_VOICES;i++)if((voices[i].pending&&voices[i].next_note==note)||
        (voices[i].stage&&voices[i].stage!=5&&voices[i].note==note)){pick=i;break;}
    if(pick<0)for(int i=0;i<PV_MAX_VOICES;i++)if(!voices[i].stage&&!voices[i].pending){pick=i;break;}
    if(pick<0){pick=0;for(int i=1;i<PV_MAX_VOICES;i++)if(voices[i].age<voices[pick].age)pick=i;}
    voice *v=&voices[pick];int vel=(int)(clamp(velocity,0,1)*32767);
    if(!v->stage||v->level<=0){
        if(v->stage)plugin_release(pick);
        start_voice(pick,note,vel,pitch,++serial);return;
    }
    v->pending=1;v->next_note=note;v->next_velocity=vel;v->next_pitch=pitch;v->age=++serial;
    if(v->stage!=5){
        v->stage=5;v->fade_left=REPLACE_FADE;v->release_step=v->level/REPLACE_FADE;
        plugin_release(pick);
    }
}
void pv_process(void){
    memset(outputs,0,sizeof outputs);
    if(PREVIEW_KIND){plugin_effect(inputs[0],inputs[1],outputs[0],outputs[1]);return;}
    for(int i=0;i<PV_MAX_VOICES;i++){
        voice *v=&voices[i];
        if(!v->stage&&v->pending)start_voice(i,v->next_note,v->next_velocity,v->next_pitch,v->age);
        if(!v->stage)continue;
        for(int n=0;n<PV_BLOCK;n++){
            if(v->stage==1){v->level+=1/(attack*PV_RATE);if(v->level>=1){v->level=1;v->stage=2;}}
            else if(v->stage==2){v->level-=(1-sustain)/(decay*PV_RATE);if(v->level<=sustain){v->level=sustain;v->stage=3;}}
            else if(v->stage==3)v->level=sustain;
            else if(v->stage==4){v->level-=v->release_step;if(v->level<=0){v->level=0;v->stage=0;}}
            else if(v->stage==5){v->level-=v->release_step;if(--v->fade_left<=0){v->level=0;v->stage=0;}else if(v->level<0)v->level=0;}
            envelope[n]=(int16_t)(v->level*32767);
        }
        plugin_voice(i,v->pitch,v->velocity,envelope,rendered);
        for(int n=0;n<PV_BLOCK;n++)outputs[0][n]+=rendered[n]*(envelope[n]/32767.f)/32768.f;
    }
    memcpy(outputs[1],outputs[0],sizeof outputs[0]);
}
static pv_command *command(int type){
    if(command_count==PV_MAX_COMMANDS){overflow=1;return 0;}
    pv_command *c=&commands[command_count++];memset(c,0,sizeof *c);c->type=type;return c;
}
void pv_line(int x,int y,int x2,int y2,int width,uint32_t color){
    pv_command *c=command(1);if(!c)return;c->x=x;c->y=y;c->x2=x2;c->y2=y2;c->width=width;c->color=color;
}
void pv_circle(int x,int y,int radius,uint32_t color){pv_command *c=command(2);if(!c)return;c->x=x;c->y=y;c->width=radius;c->color=color;}
void pv_text(int x,int y,int size,uint32_t color,const char *text){pv_command *c=command(3);if(!c)return;c->x=x;c->y=y;c->width=size;c->color=color;for(int i=0;i<47&&text[i];i++)c->text[i]=text[i];}
int pv_frame(int elapsed_us){command_count=overflow=0;plugin_frame(elapsed_us<0?0:elapsed_us>250000?250000:elapsed_us);return command_count;}
