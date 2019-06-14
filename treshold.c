#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char **argv){
int t=atoi(argv[1]);

uint8_t *buffer=malloc(100000);
int q;
int size;

while(1){
size=fread(buffer,1,100000,stdin);
if(size<=0){break;}
for(q=0;q<size;q++){
buffer[q]=buffer[q]>t?255:0;
}
fwrite(buffer,1,size,stdout);
}
fflush(stdout);

return(0);
}



