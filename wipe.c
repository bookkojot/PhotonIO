#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int steps=360;
int wi=1440;
int he=2560;

int main(int argc, char **argv){
int pix_bytes=wi*he;
uint8_t *src=malloc(pix_bytes);
uint8_t *dst=malloc(pix_bytes);
memset(dst,0,pix_bytes);

fread(src,1,pix_bytes,stdin);


int q,w,e,l;
int size;

for(e=0;e<steps;e++){
l=(e+1)*wi/steps;
for(w=0;w<he;w++){
memcpy(dst+w*wi,src+w*wi,l);
dst[w*wi]=255;
dst[w*wi+wi-1]=255;
}
memset(dst,255,wi);
memset(dst+(he-1)*wi,255,wi);
fwrite(dst,1,pix_bytes,stdout);
fflush(stdout);
}

return(0);
}



