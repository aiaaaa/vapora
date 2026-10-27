#include <stdio.h>
#include <stdint.h>
void pv_init(void),pv_parameter(int,float),pv_envelope(float,float,float,float),pv_note_on(int,float,uint32_t),pv_note_off(int),pv_process(void);
void *pv_output(void);
int main(void){
    pv_init();pv_envelope(.01f,.1f,.8f,.1f);
    for(int p=0;p<7;p++)pv_parameter(p,(float)p/8);
    pv_note_on(60,.8f,25480119u);
    for(int i=0;i<256;i++){
        if(i==100)pv_note_on(67,.65f,38176255u);
        if(i==160)pv_note_off(60);
        if(i==200)pv_note_off(67);
        pv_process();if(fwrite(pv_output(),sizeof(float),256,stdout)!=256)return 1;
    }
    return 0;
}
