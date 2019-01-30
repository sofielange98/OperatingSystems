#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv){
long res = syscall(333);
printf("The result was - %ld.\n",res);

int a = 1;
int b = 11;
int * c;
int sum = 1+5;
c = &sum;
res = syscall(334, a,b,c);
printf("Now, the value of c is %d\n", *c);
return res;
}
