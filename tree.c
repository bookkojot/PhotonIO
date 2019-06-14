#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define WIDTH 2560
#define HEIGHT 1440

uint8_t *pixels=NULL;
uint8_t *screen_pixels=NULL;

uint8_t *sprites[10];

int raw=0;

void screen_flush(){
// to rgb24 for play.c

if(raw){
fwrite(pixels,1,WIDTH*HEIGHT,stdout);
return;
}

int screen_pixels_bytes=640*480*3;
if(!screen_pixels){
screen_pixels=malloc(screen_pixels_bytes);
}
int q,w,p=0,i=0;
for(w=0;w<HEIGHT;w+=4){
for(q=0;q<WIDTH;q+=4){
i=q+w*WIDTH;
screen_pixels[p++]=pixels[i];
screen_pixels[p++]=pixels[i];
screen_pixels[p++]=pixels[i];
}
}
p=640*360*3;
for(w=0;w<120;w++){
for(q=0;q<640;q++){
i=q+w*WIDTH;
screen_pixels[p++]=pixels[i];
screen_pixels[p++]=pixels[i];
screen_pixels[p++]=pixels[i];
}
}


fwrite(screen_pixels,1,screen_pixels_bytes,stdout);
fflush(stdout);
}

void setPixel(int x, int y, int c){
if(x<0 || y<0 || x>=WIDTH || y>=HEIGHT){return;}
pixels[x+y*WIDTH]=c;
}

void fillCircle(uint8_t *out, int wi, int x, int y, int r, int c){

if(r==0){
setPixel(x,y,c);
return;
}

int q,w,len,r2=r*r;
int ox,oy;
double lend,rd2=(double)r*r;
for(w=-r;w<=r;w++){
for(q=-r;q<=r;q++){
len=q*q+w*w;
lend=sqrt((double)q*q+(double)w*w)-.5;
//fprintf(stderr,"len %d, r2 %d\n",len,r2);
ox=q<0?0:-1;
oy=w<0?0:-1;
if(lend<=r){
if(out){
out[(q+x+ox)+(w+y+oy)*wi]=c;
} else {
setPixel(q+x+ox,w+y+oy,c);
}
}
}
}

for(q=0;q<50;q++){
//setPixel(x+sin((double)q/25*3)*r,y+cos((double)q/25*3)*r,c+20);
}

}

void generate_sprites(){
// here may be anything other

int e,d,d2;
for(e=0;e<10;e++){
d=pow(2,e);
d2=d*d;
sprites[e]=malloc(d2);
memset(sprites[e],0,d2);
fillCircle(sprites[e],d,d/2,d/2,d/2,50);
}

}

void draw_sprite(int id, int x, int y){
if(id>=10 || id<0){
fprintf(stderr,"Wrong sprite %d at %dx%d\n",id,x,y);
return;
}
int d=1<<id;
x-=d>>1;
y-=d>>1;
int q,w,rx,ry;
for(w=0;w<d;w++){
for(q=0;q<d;q++){
rx=x+q;
ry=y+w;
if(rx<0 || ry<0 || rx>=WIDTH || ry>=HEIGHT){continue;}
pixels[rx+ry*WIDTH]+=sprites[id][q+w*d];
}
}

}



int count=0;
void born(int x, int y, int r, int lev, int rot, int max, double final_phase){

if(lev>max){
return;
}

count++;
double phase=(lev==max)?final_phase:1;
double ip=1.0-phase,cp=cos(phase*M_PI+M_PI)*.5+.5;
double icp=1.0-cp;
double t;
int q;
int num=pow(2,lev+1);
int nx,ny;
num=lev>1?lev:2;
int current_radius_pixels=pow(2,r);
double orbital=(double)current_radius_pixels*1*(1+pow(3,lev)/50.0);
double arot=rot*lev*M_PI*phase;
arot+=M_PI/2*lev;
int next_radius=r-1;
double next_len;
do{
next_len=pow(2,next_radius)*num;
if(next_len>orbital){
next_radius--;
}
break;
}while(1);

if(next_radius<0){next_radius=0;}
int next_radius_pixels=pow(2,next_radius);
double radius_diff=next_radius_pixels-current_radius_pixels-.8;
double phase_raduis_pixels=((double)current_radius_pixels+radius_diff*phase);
//fprintf(stderr,"cur radius: %d, next: %d, diff:%f, phase: %f, p:%f\n",current_radius_pixels,next_radius_pixels,radius_diff,phase_raduis_pixels,phase);

//fprintf(stderr,"num: %d\n",num);

for(q=0;q<num;q++){
//fprintf(stderr,"lev:%d/%d\n",lev,num);
t=(double)q/num*2.0*M_PI+arot;
nx=x+cos(t)*orbital*cp;
ny=y+sin(t)*orbital*cp;
//fprintf(stderr,"cp: %f/%f, pos: %dx%d\n",cp,icp,nx,ny);
if(nx<0 || ny<0 || nx>=WIDTH || ny>=HEIGHT){continue;}
if(lev==max){
fillCircle(0,0,nx,ny,phase_raduis_pixels-1,255);
}
born(nx,ny,next_radius,lev+1,lev==0?q:rot,max,final_phase);
}

}




int main(int argc, char **argv){

int pixels_bytes=WIDTH*HEIGHT;
pixels=malloc(pixels_bytes);
int q,w,e,r,t,y;
int ox=WIDTH/2,oy=HEIGHT/2;
int cx,cy;
memset(pixels,0,pixels_bytes);

raw=argc==2?1:0;

// draw sprites first
generate_sprites();
screen_flush();

//exit(0);

// fill base
for(q=0;q<200;q++){
memset(pixels,0,pixels_bytes);
fillCircle(0,0,ox,oy,712+sin((double)q/200*M_PI/2.0)*-200,255);
screen_flush();
}

///exit(1);

int ii;
int sub;
for(e=0;e<8;e++){
for(ii=0;ii<160;ii++){
memset(pixels,0,pixels_bytes);
for(sub=-50;sub<80;sub++){
//for(sub=0;sub<1;sub++)
double p=(double)ii/160.0+sub/160.0/30.0;

fprintf(stderr,"stage:%d, phase:%f\n",e,p);
born(ox,oy,9,0,0,e,p);
}
screen_flush();

}
//screen_flush();
//sleep(1);
fprintf(stderr,"stage:%d done\n",e);
}
fprintf(stderr,"count:%d\n",count);

return 0;
}



