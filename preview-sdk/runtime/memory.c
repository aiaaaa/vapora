/* Freestanding helpers: no WASI, firmware, or browser imports. */
#include <stddef.h>
void *memset(void *d,int value,size_t n){unsigned char *p=d;for(size_t i=0;i<n;i++)p[i]=(unsigned char)value;return d;}
void *memcpy(void *d,const void *s,size_t n){unsigned char *p=d;const unsigned char *q=s;for(size_t i=0;i<n;i++)p[i]=q[i];return d;}
int strcmp(const char *a,const char *b){while(*a&&*a==*b){a++;b++;}return (unsigned char)*a-(unsigned char)*b;}
