/* Vapora 1.2.4 adapter. Production DSP/screen compile unchanged with VAPORA_HOST. */
#include "preview.h"
#include "tests/native_mock.h"
#include "vapora_mailbox.h"
#include <string.h>
uint32_t ps_host_voices[6][9],ps_host_cycles;
int32_t ps_host_pitch[128];
int16_t ps_host_env[128],ps_host_words[8][8],ps_host_velocity[6];
int ps_host_slot,ps_host_shift,ps_host_particle_layer=-1;
volatile uint8_t op1_eng_page[4];
typedef struct {int visible;int32_t m[9];} node;
static node nodes[2];
static uint8_t sun_color[4]={255,255,255,255};
static uint32_t rgba(const uint8_t *c){return (uint32_t)c[0]<<24|(uint32_t)c[1]<<16|(uint32_t)c[2]<<8|c[3];}
void *op1_find_node(void *s,const char *id){(void)s;return !strcmp(id,"eclipse_curve")?&nodes[0]:!strcmp(id,"cover_curve")?&nodes[1]:0;}
uint8_t *op1_node_fill_rgba(void *s,const char *id){(void)s;(void)id;return sun_color;}
void op1_node_set_visible(void *n,int visible){if(n)((node*)n)->visible=visible;}
void op1_node_set_transform(void *n,const int32_t *m){if(n)memcpy(((node*)n)->m,m,36);}
void op1_screen_draw(void *s,int dt){
    (void)s;(void)dt;if(!nodes[0].visible)return;
    int x=nodes[0].m[6]+20*1024,y=nodes[0].m[7]+20*1024;
    pv_circle(x,y,20*1024,rgba(sun_color));
    if(nodes[1].visible)pv_circle(x+nodes[1].m[6],y,20*1024,0x000000ff);
}
void *op1_raster(void){return (void*)1;}
void op1_raster_line(void *r,const op1_pt *a,const op1_pt *b,int32_t w,const uint8_t *c){(void)r;pv_line(a->x,a->y,b->x,b->y,w,rgba(c));}
typedef struct {int x,y,size;uint32_t color;char text[48];} text_node;
void op1_text_ctor(void *obj,int32_t x,int y,int size,const uint8_t *c){text_node *t=obj;memset(t,0,sizeof *t);t->x=x;t->y=y;t->size=size;t->color=rgba(c);}
void op1_text_set(void *obj,const char *s){text_node *t=obj;int i=0;for(;i<47&&s[i];i++)t->text[i]=s[i];t->text[i]=0;}
void op1_text_draw(void *obj,void *r,const int32_t *m){(void)r;text_node *t=obj;pv_text(t->x+m[6],t->y*1024+m[7],t->size*1024,t->color,t->text);}
void plugin_init(void){
    const int16_t words[8]={9000,26000,18500,12500,0x5320,8192,8192,8192};
    memcpy(ps_host_words[0],words,sizeof words);SCREEN_vapora.init(0);
}
void plugin_parameter(int id,float v){
    if(id<4)ps_host_words[0][id]=(int16_t)(v*32767+.5f);
    else if(id==4)ps_host_words[0][5]=(int16_t)(8192+(int)(v*13+.5f)*256);
    else if(id==5)ps_host_words[0][6]=(int16_t)(((int)(v*9+.5f)*32768+16384)/10);
    else if(id==6)ps_host_words[0][7]=(int16_t)(1+v*32766);
}
void plugin_action(int id,int value){
    if(id<4)op1_eng_page[id]=value!=0;
    else if(id==4)ps_host_shift=value!=0;
}
void plugin_start(int i,int velocity){ps_host_velocity[i]=velocity;ENGINE_vapora.note_on(ps_host_voices[i],ps_host_words[0]);}
void plugin_release(int i){ENGINE_vapora.note_off(ps_host_voices[i]);}
void plugin_voice(int i,uint32_t pitch,int velocity,const int16_t *env,int16_t *out){
    for(int n=0;n<128;n++){ps_host_pitch[n]=(int32_t)pitch;ps_host_env[n]=env[n];}
    ps_host_velocity[i]=velocity;ENGINE_vapora.render(ps_host_voices[i],out,ps_host_words[0]);
}
void plugin_effect(const float *a,const float *b,float *c,float *d){(void)a;(void)b;(void)c;(void)d;}
void plugin_frame(uint32_t us){ps_host_cycles+=us*400;SCREEN_vapora.update(0,0);}
typedef struct {ps_mailbox mail[6];uint32_t held[6];} telemetry;
_Static_assert(sizeof(telemetry)<=PV_SNAPSHOT_BYTES,"snapshot limit");
int plugin_snapshot(void *out){telemetry *t=out;memcpy(t->mail,ps_mail,sizeof ps_mail);for(int i=0;i<6;i++)t->held[i]=ps_held_serial[i];return sizeof *t;}
void plugin_receive(const void *data){const telemetry *t=data;memcpy(ps_mail,t->mail,sizeof ps_mail);for(int i=0;i<6;i++)ps_held_serial[i]=t->held[i];}
