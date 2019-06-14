#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// +unpack previews
// +unpack layers
// +print info about all layers

#pragma pack(1)

typedef struct{
uint32_t signature;
uint32_t version;
float bed_x;
float bed_y;
float bed_z; // 84 bytes later goes data

uint32_t reserved[3];
float layer_height; // default?
float exp_time;
float exp_bottom_time;
float off_time;
uint32_t bottom_layers;
uint32_t resolution_x;
uint32_t resolution_y;
uint32_t preview0; // high resolution preview
uint32_t layers_offset;
uint32_t layers;
uint32_t preview1; // low resolution preview
uint32_t print_time;
uint32_t type;

uint32_t parameters_offset;
uint32_t parameters_length;
uint32_t antialiasing;
uint16_t light_pwm;
uint16_t light_pwm_bottom;

uint32_t unknown[3];
} HEADER;

typedef struct{
uint32_t version_signature_mult;
uint16_t header_size;
double pixel_size;
double layer_height;
double exp_time;
double off_time;
double exp_bottom_time;
uint32_t bottom_layers;
double z_lift_distance;
double z_lift_speed;
double z_retract_speed;
double volume;
uint32_t preview_width;
uint32_t preview_top;
uint32_t preview_height;
uint32_t preview_left;
} HEADER_S;


typedef struct{
float bottom_distance;
float bottom_distance_speed;
float distance;
float distance_speed;
float retract_speed;

float volume;
float weight;
float cost;

float bottom_off_delay;
float off_delay;
uint32_t bottom_layers;// double??? we have this field in main header!
uint32_t unknown[4];
} HEADER_EXT;

typedef struct{
uint32_t resolution_x;
uint32_t resolution_y;
uint32_t offset;
uint32_t size;
uint32_t padding[4];
uint8_t data[];
} PREVIEW;

typedef struct{
float height;
float exp_time;
float off_time;
uint32_t offset;
uint32_t size;
uint32_t padding[4];
} LAYER;

uint32_t covered_bytes=0;
FILE *in;
uint8_t *buffer;
HEADER *head;
HEADER_S *head_s;
uint8_t *head_buffer;


int width=1440;
int height=2560;

#define clamp_byte(x) (x<0?0:(x>255?255:x))

void plasma(uint16_t *put, int width,int height){
srand(time(0));
int x,y,sec;
double dx,dy,dv;
double ra[100];
int q;
double dex=(random()%30)+10;
double dey=(random()%20)+10;
int rr,rg,rb;
for(x=0;x<100;x++){
ra[x]=random()%(150)+100;
}

for(x=0;x<width;x++){
for(y=0;y<height;y++){
dx=sin(x/dex);
dy=cos(y/dey);
dv=(double)dy+dx+sqrt(x*y)/5.0+ra[3];
rr=127+ra[0]*(sin(dv));
rg=127+ra[1]*(sin(dv*ra[7]/100.0+ra[5]));
rb=127+ra[2]*(sin(dv*ra[8]/100.0+ra[6]));
rr=clamp_byte(rr);
rg=clamp_byte(rg);
rb=clamp_byte(rb);
rr=
((rb<<8)&0xF800)|
((rg<<3)&0x7E0)|
((rr>>3)&0x1F);
put[x+y*width]=rr;

        }
}
fprintf(stderr,"code: %x\n",rr);
}


void plasma_test(){

int pic_width=224;
int pic_height=128;
int pic_in_bytes=pic_width*pic_height*2;
int pic_out_bytes=640*480*3;
int q,w;

FILE *vid=fopen("/dev/shm/3dview/video","wb");

uint8_t *plasma_buf=malloc(pic_in_bytes);
uint8_t *screen=malloc(pic_out_bytes);
int in_pos,out_pos;
int code;
while(1){
plasma((uint16_t*)plasma_buf,pic_width,pic_height);

in_pos=0;
for(w=0;w<pic_height;w++){
for(q=0;q<pic_width;q++){
code=plasma_buf[in_pos]|(plasma_buf[in_pos+1]<<8);
out_pos=(q+w*640)*3;
screen[out_pos++]=(code&0x1F)<<3;
screen[out_pos++]=((code>>5)&0x3F)<<2;
screen[out_pos++]=((code>>11)&0x1f)<<3;
in_pos+=2;
}
}
fwrite(screen,1,pic_out_bytes,vid);
}
}


void processLayers(int offset, int count){
LAYER *p=malloc(sizeof(LAYER));
int pix_bytes=width*height;
uint8_t *pixels=malloc(pix_bytes);
int layer;
int q,w,px,pix_pos=0,aa;
//count=0xFFFFFF;


for(layer=0;layer<count;layer++){

for(aa=head->antialiasing-1;aa>=0;aa--){

fseek(in,offset+sizeof(LAYER)*layer+count*sizeof(LAYER)*aa,SEEK_SET);
fread(p,1,sizeof(LAYER),in);

fprintf(stderr,"===[Layer %d:%d]======================\n"
"* Header: offset: %d (%x), end of header: %d (%x)\n"
"Height: %f\n"
"Exp time: %f\n"
"Off time: %f\n"
"Offset: %d (0x%x)\n"
"Size in bytes: %d (0x%x) (only for 1 layer, without AA)\n"
"Padding: %d %d %d %d\n"
"Sizeof: %d\n",
layer,aa,
offset+sizeof(LAYER)*layer+count*sizeof(LAYER)*aa,
offset+sizeof(LAYER)*layer+count*sizeof(LAYER)*aa,
offset+sizeof(LAYER)*layer+count*sizeof(LAYER)*aa+sizeof(LAYER),
offset+sizeof(LAYER)*layer+count*sizeof(LAYER)*aa+sizeof(LAYER),
p->height,
p->exp_time,
p->off_time,
p->offset,p->offset,
p->size,p->size,
p->padding[0],
p->padding[1],
p->padding[2],
p->padding[3],
sizeof(LAYER)
);

//if(count==0xFFFFFF){count=(p->offset-offset)/sizeof(LAYER);fprintf(stderr,"REAL number of layers: %d\n\n",count);}

covered_bytes+=sizeof(LAYER);

fseek(in,p->offset,SEEK_SET);
fread(buffer,1,p->size,in);

covered_bytes+=p->size;


q=0;
pix_pos=0;
while(1){
px=buffer[q]&0x80?0xFF:0;
for(w=(buffer[q]&0x7F);w>0;w--){
pixels[pix_pos++]=px;
}
q++;
if(pix_pos>1440*2560){
fprintf(stderr,"Error???\n\n");
abort();
}
if(pix_pos==1440*2560){
break;
}

}



//if(layer>150){exit(0);}
char filename[256];
sprintf(filename,"layer-%.5d.gray",layer);
fprintf(stderr,"out: %s at %d (0x%x)\n",filename,p->offset+q,p->offset+q);
//FILE *out=fopen(filename,"wb");
fwrite(pixels,1,pix_pos,stdout);
//fclose(out);

//if(layer>=50){exit(0);}
}


}

free(p);
free(pixels);

}

void processPreview(int offset){
PREVIEW *p=malloc(sizeof(PREVIEW));
fseek(in,offset,SEEK_SET);
fread(p,1,sizeof(PREVIEW),in);


fprintf(stderr,"===[Preview]====================\n"
"Resolution: %dx%d\n"
"Offset: %d (0x%x)\n"
"Size: %d (0x%x)\n"
"Padding: %d %d %d %d\n\n",

p->resolution_x,
p->resolution_y,
p->offset,p->offset,
p->size,p->size,
p->padding[0],
p->padding[1],
p->padding[2],
p->padding[3]
);

fseek(in,p->offset,SEEK_SET);
fread(buffer,1,p->size,in);

covered_bytes+=p->size+sizeof(PREVIEW);

char filename[256];
sprintf(filename,"preview-%d.raw",offset);
FILE *out=fopen(filename,"wb");
fwrite(buffer,1,p->size,out);
fclose(out);

// decode
int q,w,pos_src=0,pos_dst=0;
int pix_bytes=p->resolution_x*p->resolution_y*3;
uint8_t *pixels=malloc(pix_bytes);
uint16_t *src=(uint16_t*)buffer;
int r,g,b;
int count,repeat;
int total=0;
for(q=0;q<p->size;){
r=(buffer[q+1]>>3)<<3;
g=((((buffer[q+1]<<8)|(buffer[q]))>>6)&0x1F)<<3;
b=(buffer[q]&0x1F)<<3;
repeat=buffer[q]&0x20;
q++;
q++;
if(repeat!=0){
count=(buffer[q]|(buffer[q+1]<<8))&0xFFF;
count++;
q+=2;
} else {
count=1;
}
for(w=0;w<count && pos_dst<pix_bytes;w++){
pixels[pos_dst++]=r;
pixels[pos_dst++]=g;
pixels[pos_dst++]=b;
}


}


sprintf(filename,"preview-%d.rgb",offset);
out=fopen(filename,"wb");
fwrite(pixels,1,pix_bytes,out);
fclose(out);

char cmd[1024];
sprintf(cmd,"ffmpeg -v 0 -f rawvideo -pix_fmt rgb24 -s %dx%d -i preview-%d.rgb -y preview-%d.png",p->resolution_x,p->resolution_y,offset,offset);
system(cmd);



free(pixels);
free(p);

}


int unpack_layer(uint8_t *src, uint8_t *dst, int white, int width, int height, uint32_t *got){

int q=0,w,pix_pos=0,pix_max=width*height,px;
int step;
while(1){
//if(src[q]==0){break;}
//fprintf(stdout,"...byte %.2x\n",src[q]);
step=src[q];

step=
(((step>>0)&1)<<7)|
(((step>>1)&1)<<6)|
(((step>>2)&1)<<5)|
(((step>>3)&1)<<4)|
(((step>>4)&1)<<3)|
(((step>>5)&1)<<2)|
(((step>>6)&1)<<1)|
(((step>>7)&1)<<0);

px=step&0x80?0xFF:0;
w=(step&0x7f)+1;
if(px){
white-=w;
}

for(;w>0&&pix_pos<pix_max;w--){
dst[pix_pos++]=px;
}
q++;
if(pix_pos>=pix_max || white<=0){
break;
}

}
*got=pix_pos;
return(q);
}

#define big_endian(bb,ii) ((bb[ii]<<24) | (bb[ii+1]<<16) | (bb[ii+2]<<8) | bb[ii+3])
#define big_endian_int(bb) (((bb&0xFF)<<24) | ((bb&0xFF00)<<8) | ((bb&0xFF0000)>>8) | (bb>>24))


inline void swap_bytes(void *a, void *b, int c){
int q;
uint8_t t;
uint8_t *i=(uint8_t*)b;
int c2=c>>1;
for(q=0;q<c/2;q++){
t=i[q];
i[q]=i[c-q-1];
i[c-q-1]=t;
}
}

#define swap_bytes_field(st,fi) swap_bytes(st,&(st->fi),sizeof(st->fi));

void swap_bytes_in_header_s(){

swap_bytes_field(head_s,version_signature_mult);
swap_bytes_field(head_s,header_size);
swap_bytes_field(head_s,pixel_size);
swap_bytes_field(head_s,layer_height);
swap_bytes_field(head_s,exp_time);
swap_bytes_field(head_s,off_time);
swap_bytes_field(head_s,exp_bottom_time);
swap_bytes_field(head_s,bottom_layers);
swap_bytes_field(head_s,z_lift_distance);
swap_bytes_field(head_s,z_lift_speed);
swap_bytes_field(head_s,z_retract_speed);
swap_bytes_field(head_s,volume);
swap_bytes_field(head_s,preview_width);
swap_bytes_field(head_s,preview_top);
swap_bytes_field(head_s,preview_height);
swap_bytes_field(head_s,preview_left);

}

void unpack_stream(){
fseek(in,0,SEEK_SET);
int size=fread(buffer,1,100000000,in);
int frame_size=1440*2560;
fprintf(stderr,"frames: %dx%d = %d\n",width,height,frame_size);

int q,offset=75366,used;
//offset=0x62+4;
int got;
int whitepixels,local_width,local_height;
int packed_size;
uint64_t total_white=0;
int layer=0;
while(offset<size){
//for(q=0;q<28;q++){printf("%.2x ",buffer[offset+q]);}
//printf("\n");
whitepixels=big_endian(buffer,offset);
local_width=big_endian(buffer,offset+12);
local_height=big_endian(buffer,offset+16);
packed_size=(big_endian(buffer,offset+20)>>3)-4;

fprintf(stderr,"%d: reading at offset: %d/%d, size:%dx%d, white:%d\n",layer,offset,size,local_width,local_height,whitepixels);
layer++;
//memset(pixels,1,frame_size);
//used=unpack_layer(buffer+offset+28,pixels,whitepixels,local_width,local_height,&got);
//fprintf(stderr,"...used %d, got: %d\n\n\n\n\n",used,got);
//exit(1);
//abort();
offset+=packed_size+28;
//fwrite(pixels,1,frame_size,stdout);
total_white+=whitepixels;
}

fprintf(stderr,"total white: %llu\n\n",total_white);

//exit(1);

}


uint8_t *framebuffer=NULL;
uint8_t *framebuffer_rle=NULL;

int pack_rle(int for_s, int *whitepower, uint8_t *src, uint8_t *dst){

// encode
int q,w;
int rle_max;
int current,code,pix,len;
int total_white=0;
int rle_pos=0;
if(for_s){
rle_max=128;
current=-1;
len=0;
for(w=0;w<height;w++){
for(q=0;q<width;q++){
pix=src[width-q-1+w*width]>100?1:0;
if(pix!=current || len>=rle_max){
if(len>0){
code=((current&1)<<7)|(len-1);
code=
(((code>>0)&1)<<7)|
(((code>>1)&1)<<6)|
(((code>>2)&1)<<5)|
(((code>>3)&1)<<4)|
(((code>>4)&1)<<3)|
(((code>>5)&1)<<2)|
(((code>>6)&1)<<1)|
(((code>>7)&1)<<0);
dst[rle_pos++]=code;
if(current){
total_white+=len;
}
}
current=pix;
len=1;
} else {
len++;
}

}
}

} else {
// for photon
abort();
}

*whitepower=total_white;

return(rle_pos);
}

void treshold_framebuffer(){
int framebuffer_size=width*height;
while(framebuffer_size--){
framebuffer[framebuffer_size]=framebuffer[framebuffer_size]>128?255:0;
}

}

int read_and_rle(int for_s, int *whitepower, int *out_size){
int framebuffer_size=width*height;
fprintf(stderr,"Reading buffer %dx%d=%d bytes from stdin\n",width,height,framebuffer_size);
if(!framebuffer){
}
int size=fread(framebuffer,1,framebuffer_size,stdin);
*out_size=size;
if(size<=0){
return(0);
}
treshold_framebuffer();
int rle_pos=pack_rle(for_s,whitepower,framebuffer,framebuffer_rle);

static int framenum=0;
fprintf(stderr,"%d: Read %d and packed %d bytes, white pixels %d\n",framenum++,size,rle_pos,*whitepower);
return(rle_pos);
}


uint8_t *make_skirt(int *white, int *size){
int q,w,xmin=0xFFFF,ymin=0xFFFF,xmax=0,ymax=0;
for(w=0;w<height;w++){
for(q=0;q<width;q++){
if(framebuffer[q+w*width]){
if(q<xmin){xmin=q;}
if(q>xmax){xmax=q;}
if(w<ymin){ymin=w;}
if(w>ymax){ymax=w;}
}
}
}

q=xmax-xmin+1;
for(w=ymin;w<=ymax;w++){
memset(framebuffer+xmin+w*width,0xff,q);
}

uint8_t *skirt_buffer=malloc(100000);
*size=pack_rle(1,white,framebuffer,skirt_buffer);
return(skirt_buffer);
}


void make_layer_header_s(uint32_t *header,int white, int size){
header[0]=big_endian_int(white);
header[1]=0;
header[2]=0;
int tmp=1440;
header[3]=big_endian_int(tmp);
tmp=2560;
header[4]=big_endian_int(tmp);
tmp=(size+4)<<3;
header[5]=big_endian_int(tmp);
header[6]=0x005005a0;
}

void repack_stream(){
fseek(in,0,SEEK_SET);
int header_size=75366;
FILE *out=fopen("out.photons","wb");
fread(buffer,1,header_size,in);
fwrite(buffer,1,header_size,out);
int p,whitepixels,packed_size,read_size,tmp;
uint32_t header[7];
int is_first=1;
int layers=0;
uint64_t total_white=0;
while(1){
packed_size=read_and_rle(1,&whitepixels,&read_size);
if(read_size<=0){break;}
if(whitepixels && packed_size){

if(is_first){
int skirt_white,skirt_size;
uint8_t *skirt=make_skirt(&skirt_white,&skirt_size);
make_layer_header_s(header,skirt_white,skirt_size);
int sk;
for(sk=0;sk<10;sk++){
fwrite(header,1,sizeof(header),out);
fwrite(skirt,1,skirt_size,out);
layers++;
total_white+=skirt_white;
}
is_first=0;
free(skirt);
}

make_layer_header_s(header,whitepixels,packed_size);
fwrite(header,1,sizeof(header),out);
int outsize=fwrite(framebuffer_rle,1,packed_size,out);
if(outsize!=packed_size){abort();}
total_white+=whitepixels;
layers++;
}
}

// patch layers count
tmp=big_endian_int(layers);
fseek(out,75362,SEEK_SET);
fwrite(&tmp,1,4,out);

double th=150.0/layers;
if(th>.050){
th=0.050;
}

head_s->pixel_size=0.04725;
head_s->bottom_layers=10;

double vol=10.0/head_s->pixel_size;
vol*=vol;
vol*=10.0/th;
vol=(double)total_white/vol;

head_s->layer_height=th;
head_s->volume=vol;

//make plasma
plasma((uint16_t*)framebuffer,head_s->preview_width,head_s->preview_height);
fseek(out,sizeof(HEADER_S),SEEK_SET);
fwrite(framebuffer,1,head_s->preview_width*head_s->preview_height*2,out);

// save header
swap_bytes_in_header_s();
fseek(out,0,SEEK_SET);
fwrite(head_buffer,1,sizeof(HEADER_S),out);

fclose(out);
fprintf(stderr,"DONE! Written %llu pixels in %d layers, %fml\n\n",total_white,layers,head_s->volume=vol);
exit(1);
}



uint16_t flip16(uint16_t in){
uint16_t out=0;
int q;
for(q=0;q<16;q++){
out|=((in>>q)&1)<<(15-q);
}
return(out);
}


void dump_preview(){
fseek(in,98,SEEK_SET);

int pic_width=224;
int pic_height=128;
int pic_in_bytes=pic_width*pic_height*2;
int pic_out_bytes=pic_width*pic_height*3;

uint8_t *buf_in=malloc(pic_in_bytes);
uint8_t *buf_out=malloc(pic_out_bytes);
fread(buf_in,1,pic_in_bytes,in);

int out_pos=0;
int in_pos=0;
int code;
while(in_pos<pic_in_bytes){
code=buf_in[in_pos]|(buf_in[in_pos+1]<<8);
buf_out[out_pos++]=(code&0x1F)<<3;
buf_out[out_pos++]=((code>>5)&0x3F)<<2;
buf_out[out_pos++]=((code>>11)&0x1f)<<3;
in_pos+=2;
}

fprintf(stderr,"out_pos:%d, fill:%d\n",out_pos,pic_out_bytes);

FILE *out=fopen("preview.bin","wb");
fwrite(buf_out,1,pic_out_bytes,out);
fclose(out);

//exit(1);

}


void print_header(){

fprintf(stderr,"===[Header]=====================\n"
"Signature: 0x%.8x, version: %d\n"
"Bed size: %.2fx%.2fx%.2f\n"
"Reserved: %.8x %.8x %.8x\n"
"Layer height: %f\n"
"exp_time: %f\n"
"exp_bottom_time %f\n"
"off_time %f\n"
"botton layers: %d\n"
"Resolution %dx%d\n"
"Offset Preview0 %d (0x%x)\n"
"layers offset %d (0x%x)\n"
"layers %d\n"
"Offset Preview1 %d (0x%x)\n"
"Print time: %d seconds (%.2d:%.2d)\n"
"Type: %d (%s)\n"

"Parameters offset: %d (%x)\n"
"Parameters length: %d (%x)\n"
"Antialiasing level: %d\n"
"Light PWM/Bottom PWM: %d/%d\n"
"Unknown values: %x %x %x\n\n"
"sizeof: %d (0x%x)\n\n",

head->signature,
head->version,
head->bed_x,
head->bed_y,
head->bed_z,
head->reserved[0],
head->reserved[1],
head->reserved[2],
head->layer_height,
head->exp_time,
head->exp_bottom_time,
head->off_time,
head->bottom_layers,
head->resolution_x,
head->resolution_y,
head->preview0,head->preview0,
head->layers_offset,head->layers_offset,
head->layers,
head->preview1,head->preview1,
head->print_time,head->print_time/60,head->print_time%60,
head->type,
head->type==1?"LCD_X_MIRROR":"CAST",
head->parameters_offset,head->parameters_offset,
head->parameters_length,head->parameters_length,
head->antialiasing,
head->light_pwm,
head->light_pwm_bottom,
head->unknown[0],
head->unknown[1],
head->unknown[2],
sizeof(HEADER),sizeof(HEADER)
);


}

void print_header_s(){
swap_bytes_in_header_s();


fprintf(stderr,"===[Header]=====================\n"
"Version/Signature: %d (must be 2)\n"
"Header size/2, offset to preview/2: %d (0x%x), real offset *2=%d (0x%x)\n"
"Pixel size: %fnm\n"
"Layer height: %fmm\n"
"Exposure time: %f seconds\n"
"Off time: %f seconds\n"
"Exposure bottom layers: %f seconds\n"
"Bottom layers count: %u\n"
"Lift distance: %f\n"
"Lift speed: %f\n"
"Retract speed: %f\n"
"Total volume: %fml\n"
"Preview: %dx%d+%d+%d\n"
"\n"
"sizeof(HEADER_S): %d (0x%x)\n\n",
head_s->version_signature_mult,
head_s->header_size,head_s->header_size,
head_s->header_size<<1,head_s->header_size<<1,
head_s->pixel_size,
head_s->layer_height,
head_s->exp_time,
head_s->off_time,
head_s->exp_bottom_time,
head_s->bottom_layers,
head_s->z_lift_distance,
head_s->z_lift_speed,
head_s->z_retract_speed,
head_s->volume,
head_s->preview_width,
head_s->preview_top,
head_s->preview_height,
head_s->preview_left,
sizeof(HEADER_S),sizeof(HEADER_S)
);
/*

0 int32_t version? multiplier for offset?
4 int16_t val=*2; preview offset? header size?
6 double pixel_size
14 double tickness
22 double normal_exposure_time
30 double off_time;
38 double bottom_exposure
46 uint32_t bottom_layers
50 double z_lift_distance
58 double z_lift_speed
66 double z_retract_speed;
74 double VOLUME
82 uint32_t preview_width
86 uint32_t preview_offset_top?
90 uint32_t preview_height
94 uint32_t preview_offset_left?
98 ...image data... uncompressed 16-bit bgr565 preview_width*preview_height*2bytes
...
...uint32_t slices count
... ...slices...

slice:
0 uint32_t white_pixels;
4 uint32_t left; (0)
8 uint32_t top; (0)
12 uint32_t width; (1440)
16 uint32_t height; (2560)
20 uint32_t (size>>3)-4
24 uint32_t width&height (reverse bits) = a0 05 50 00 -> (5a0 a00)
28 ... rle-packed image ...
Format same as original photon, but bits in reversed order

A00=101000000000 -> reverse
->  000000000101 = 0x5

0xFE -> 0x7F

*/

}

enum{
PRINTER_UNKNOWN,
PRINTER_PHOTON,
PRINTER_PHOTON_S
};

int file_type=PRINTER_UNKNOWN;


int guess_printer(){
if(*(uint32_t*)head_buffer==0x12FD0019){return(PRINTER_PHOTON);}
if(*(uint32_t*)head_buffer==0x02000000 && head_buffer[5]==0x31){return(PRINTER_PHOTON_S);}
return(PRINTER_UNKNOWN);
}

int main(int argc, char **argv){

//plasma_test();

buffer=malloc(100000000);
framebuffer=malloc(width*height);
framebuffer_rle=malloc(width*height*2);

head_buffer=malloc(1024); // will be enough
in=fopen(argv[1],"rb");
if(!in){
fprintf(stderr,"Can't open file!\n\n");
exit(1);
}
fseek(in,0,SEEK_SET);
fread(head_buffer,1,1024,in);

file_type=guess_printer();
if(file_type==PRINTER_UNKNOWN){
fprintf(stderr,"Sorry, but I don't know what to do with this file! File may be corrupted or new version has arrived! Try to send this file to author!\n\n");
exit(1);
}


if(file_type==PRINTER_PHOTON_S){
head_s=(HEADER_S*)head_buffer;
print_header_s();
dump_preview();
unpack_stream();
}

if(file_type==PRINTER_PHOTON){
head=(HEADER*)head_buffer;
print_header();
}

repack_stream();

exit(0);
//dump_preview();

//exit(0);

exit(0);


covered_bytes=head->preview0; // Since head->preview0 is first data struct

if(head->version==1){
head->antialiasing=1;
}

if(head->version==2){
if(head->parameters_length!=sizeof(HEADER_EXT)){
fprintf(stderr,"* * * WARNING!!! Size of header %d not equals to %d!!!\n\n",head->parameters_length,sizeof(HEADER_EXT));
}

fseek(in,head->parameters_offset,SEEK_SET);
fread(buffer,1,head->parameters_length,in);

HEADER_EXT *ext=(HEADER_EXT *)buffer;

fprintf(stderr,"===[Header ext]=================\n"
"Bottom distance: %f\n"
"Bottom speed: %f\n"
"Distance: %f\n"
"Distance speed: %f\n"
"Retract speed: %f\n"
"-----\n"
"Volume: %fml\n"
"Weight: %fg\n"
"Cost: $%f\n"
"-----\n"
"Bottom off time: %f\n"
"Off time: %f\n"
"Bottom layers: %d\n"
"Unknown (paddding?): %x %x %x %x\n\n",

ext->bottom_distance,
ext->bottom_distance_speed,
ext->distance,
ext->distance_speed,
ext->retract_speed,

ext->volume,
ext->weight,
ext->cost,

ext->bottom_off_delay,
ext->off_delay,
ext->bottom_layers,
ext->unknown[0],
ext->unknown[1],
ext->unknown[2],
ext->unknown[3]
);

covered_bytes+=head->parameters_length;


}


processPreview(head->preview0);
processPreview(head->preview1);

processLayers(head->layers_offset,head->layers);

fprintf(stderr,"Total bytes covered: %d\n\n",covered_bytes);

return 0;
}



