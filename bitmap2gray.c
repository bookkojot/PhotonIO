#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char **argv){
//int t=atoi(argv[1]);

uint8_t *buffer=malloc(100000);
uint8_t *buffer2=malloc(1000000);
int q,w;
int size;
int outpos=0;
while(1){
size=fread(buffer,1,100000,stdin);
if(size<=0){break;}
outpos=0;
for(q=0;q<size;q++){
//fprintf(stderr,"alive\n");
for(w=0;w<8;w++){
buffer2[outpos++]=(buffer[q]>>w)&1?255:0;
}
}
fwrite(buffer2,1,size*8,stdout);
}
fflush(stdout);

return(0);
}



