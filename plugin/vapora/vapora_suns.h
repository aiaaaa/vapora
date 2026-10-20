/* Original primitive silhouettes, selected by stable original-sheet ID.
 * Fractional circle geometry translates on whole pixels; overlapping joins avoid seams.
 * GPL-3.0-or-later with OP-1 firmware permission. */
#ifndef VAPORA_SUNS_H
#define VAPORA_SUNS_H
static const char *const ps_sun_names[PS_SUN_DESIGNS]={
    "ECLIPSE","HALO","RETRO","BANDS","SATURN","TRIPLE"};
static const uint8_t ps_sun_color[4]={255,190,107,255};
static void sun_line(void *r,int cy,int x,int y,int xx,int yy)
{line(r,160*1024+x,cy*1024+y,160*1024+xx,cy*1024+yy,PS_SUN_STROKE,ps_sun_color);}
static void sun_arc(void *r,int cy,int cx,int yy,int rx,int ry,int start,int count,int stride)
{
    for(int j=start;j<start+count;j+=stride){
        int a=j&31,b=(j+stride)&31;
        int x=ps_circle[a][0]*rx,y=ps_circle[a][1]*ry;
        int xx=ps_circle[b][0]*rx,yyy=ps_circle[b][1]*ry;
        int dx=(xx-x)/8,dy=(yyy-y)/8;
        sun_line(r,cy,cx*1024+x-dx,yy*1024+y-dy,
                     cx*1024+xx+dx,yy*1024+yyy+dy);
    }
}
/* Fixed quarter-circle spans: no runtime square root or radius changes. */
static const int16_t eclipse_span[20]={
20480,20454,20377,20248,20066,19830,19537,19185,18770,18289,17736,17104,16384,15563,14626,13546,12288,10789,8927,6395};
static int sun_half_width(int y,int radius)
{
    int x=radius;while(x>0&&x*x+y*y>radius*radius)x--;
    return x;
}
static void draw_sun(void *r,int cy,int design,int opening)
{
    switch(ps_sun_ids[design]){
    case 1: sun_arc(r,cy,0,0,15,15,0,32,1);break;
    case 2:
        for(int y=-20;y<=-1;y++){int x=sun_half_width(y,21);sun_line(r,cy,-x*1024,y*1024,x*1024,y*1024);}
        for(int y=4;y<=14;y+=5){int x=sun_half_width(y,21);sun_line(r,cy,-x*1024,y*1024,x*1024,y*1024);}break;
    case 3:
        sun_arc(r,cy,0,0,15,15,0,32,1);
        for(int y=-10;y<=10;y+=5){int x=sun_half_width(y,15);sun_line(r,cy,-x*1024,y*1024,x*1024,y*1024);}break;
    case 5: {
        int q=ps_clamp(opening,0,32767);
        int offset=(int)(((int64_t)q*q*(40*1024))/(32767LL*32767));
        for(int y=-19;y<=19;y+=2){
            int x=eclipse_span[y<0?-y:y],right=ps_clamp(offset-x,-x,x);
            if(right>-x)line(r,160*1024-x,(cy+y)*1024,
                              160*1024+right,(cy+y)*1024,
                              PS_SUN_FILL_STROKE,ps_sun_color);
        }break;}
    case 10: sun_arc(r,cy,0,0,12,12,0,32,1);sun_arc(r,cy,0,2,21,5,0,32,1);break;
    case 18:
        sun_arc(r,cy,-17,0,10,10,0,32,1);sun_arc(r,cy,0,0,14,14,0,32,1);sun_arc(r,cy,17,0,10,10,0,32,1);break;
    }
}
#endif
