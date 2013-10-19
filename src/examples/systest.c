#include<stdio.h>
#include<syscall.h>

int ATOI(char*src,int *dst);

int
ATOI(char *src,int *dst){
  int i=0,sign=1;

  if(src[i]<'0' || src[i]>'9') return -1;

  //is it minus?
  if(src[i]=='-'){
    i++;
    sign=-1;
  }

  *dst = 0;
  while(src[i] && '0'<=src[i] && src[i]<='9' ){
    *dst = (*dst)*10 + src[i++]-'0' ;
  }
   
  (*dst) *= sign;
  return 1;
}
    
int 
main(int argc,char**argv)
{
  int i,input[4];
  int result_f,result_s;
  int f1=0,f2=1,temp;

  if(argc != 5) return -1;

  //set input from argv 
  for(i=0;i<4;i++){
    if(ATOI(argv[i+1],input+i)<0)
      return -1;
  }
 
  //calc fibonacci
  for(i=0;i<input[0];i++){
    temp = f2;
    f2 += f1;
    f1 = temp;
  }
  result_f = f1;
  result_s = input[0]+input[1]+input[2]+input[3];
    
  printf("%d %d\n", result_f, result_s);

  return EXIT_SUCCESS;
}
