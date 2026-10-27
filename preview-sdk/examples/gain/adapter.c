/* Minimal stereo effect adapter, used to test the host's audio-input path. */
#include "preview.h"
static float gain=1;
void plugin_init(void){gain=1;}
void plugin_parameter(int id,float value){if(id==0)gain=value;}
void plugin_action(int id,int value){(void)id;(void)value;}
void plugin_start(int v,int velocity){(void)v;(void)velocity;}
void plugin_release(int v){(void)v;}
void plugin_voice(int v,uint32_t p,int vel,const int16_t *e,int16_t *o){(void)v;(void)p;(void)vel;(void)e;(void)o;}
void plugin_effect(const float *l,const float *r,float *ol,float *or){for(int i=0;i<PV_BLOCK;i++){ol[i]=l[i]*gain;or[i]=r[i]*gain;}}
void plugin_frame(uint32_t us){(void)us;pv_line(20000,80000,300000,80000,2048,0x29c895ff);}
int plugin_snapshot(void *o){(void)o;return 0;}
void plugin_receive(const void *i){(void)i;}
