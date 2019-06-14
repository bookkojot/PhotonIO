#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



uint32_t flip32(uint32_t in, int size){
int q;
uint32_t res=0;
for(q=0;q<size;q++){
res|=((in>>q)&1)<<(size-q-1);
}
return(res);
}


int main(int argc, char **argv){

uint8_t *buffer=malloc(100000);
int q;
int size;
uint32_t step;
while(1){
size=fread(buffer,1,100000,stdin);
if(size<=0){break;}
for(q=0;q<size;q++){
step=0;
memcpy(&step,buffer+q,4);
printf("d:%.10d, x:%.8x, f:%f\t",step,step,step);
step=flip32(step,32);

printf("d:%.10d, x:%.8x, f:%f\n",step,step,step);


}
}
fflush(stdout);

return(0);
}



