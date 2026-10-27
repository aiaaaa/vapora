/* Browser preview ABI 1. GPL-3.0-or-later; see repository LICENSE. */
#ifndef HK_PREVIEW_H
#define HK_PREVIEW_H
#include <stdint.h>
#include <stddef.h>
#define PV_BLOCK 128
#define PV_RATE 44100
#define PV_MAX_VOICES 6
#define PV_MAX_COMMANDS 4096
#define PV_SNAPSHOT_BYTES 1024
/* Q10 drawing coordinates; packed RGBA in most-significant-byte order. */
typedef struct {
    int32_t type, x, y, x2, y2, width;
    uint32_t color;
    char text[48];
    uint32_t reserved;
} pv_command; /* 80 bytes; types 1 line, 2 filled circle, 3 text */
void pv_line(int x,int y,int x2,int y2,int width,uint32_t color);
void pv_circle(int x,int y,int radius,uint32_t color);
void pv_text(int x,int y,int size,uint32_t color,const char *text);
/* Implemented by each source adapter, never by the browser host. */
void plugin_init(void);
void plugin_parameter(int id,float value);
void plugin_action(int id,int value);
void plugin_start(int voice,int velocity);
void plugin_release(int voice);
void plugin_voice(int voice,uint32_t pitch,int velocity,const int16_t *envelope,int16_t *out);
void plugin_effect(const float *left,const float *right,float *out_left,float *out_right);
void plugin_frame(uint32_t elapsed_us);
int plugin_snapshot(void *out);
void plugin_receive(const void *data);
#endif
