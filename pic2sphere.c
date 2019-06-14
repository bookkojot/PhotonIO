#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Bresenham3D.h"







int map_width=8192;
int map_height=4096;
int radius=1280/2;

uint8_t *map;
uint8_t *bitmap;

FILE *in;
FILE *out;

int main(int argc, char **argv){
setRadius(radius);

int map_bytes=map_width*map_height;
map=malloc(map_bytes);

int bitmap_bytes=radius*radius*radius;
bitmap=malloc(bitmap_bytes);

in=fopen("elevation.bin","rb");
fread(map,1,map_bytes,in);
fclose(in);

memset(bitmap,0,bitmap_bytes);

int step=1;

int q,w,d,ox,oy,oz;
double sinX,cosX,sinY,cosY,thetaX,thetaY;
for(w=0;w<map_height;w+=step){
fprintf(stderr,"Processing line %d/%d\n",w,map_height);
thetaY=(double)w/map_height*M_PI;
sinY=sin(thetaY);
cosY=(cos(thetaY));
for(q=0;q<map_width;q+=step){
d=(int)map[q+w*map_width]+128+256;
thetaX=(double)q/map_width*M_PI*2.0;
sinX=sin(thetaX);
cosX=cos(thetaX);
ox=sinX*sinY*(double)d+radius;
oy=cosX*sinY*(double)d+radius;
oz=cosY*(double)d+radius;
Bresenham3D(ox,oy,oz,radius,radius,radius,bitmap,radius,radius,radius);
}
}


out=fopen("sphere.bitmap","wb");
fwrite(bitmap,1,bitmap_bytes,out);
fclose(out);


return 0;
}

