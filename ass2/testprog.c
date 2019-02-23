#include <stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define buffSize 1024
void getUserInput();

void readChar(){
	printf("\nEnter number of bytes you wish to read:\n");
  int numBytes;
  scanf("%d", &numBytes);
  getchar();
  //printf("Youve chosen: %d\n", numBytes);
  char * inputBuff = malloc(numBytes);
	int ret;

  int fd = open("/dev/simple_char_device", O_RDWR);
  ret = read(fd,inputBuff,numBytes);
  printf("Data read from device:\n");
  printf("%s\n",inputBuff);
  free(inputBuff);
  close(fd);
	getUserInput();
}

void writeChar(){
	printf("\nEnter data you want to write to the device:\n");
	char inputBuff[buffSize];
	if (fgets(inputBuff, buffSize, stdin) != NULL){
    //printf("Writing to file : %s\n", input);
    int length = strlen(inputBuff);
    //printf("Attempting to write %d bytes.\n",length);
    //printf("Value of buffOff before write : %ld\n", *buffOff);
    int fd = open("/dev/simple_char_device", O_RDWR);
    int ret  = write(fd,inputBuff,length);
    //printf("Value of buffOff after write : %ld\n", *buffOff);
    //printf("Returned from write value is: %d\n", ret);
    close(fd);
	}
	getUserInput();
}
void seekChar(){
  int fd = open("/dev/simple_char_device", O_RDWR);
  int ret;
  loff_t offset;
  int whence;
  printf("Enter desired offset:\n");
  scanf("%ld", &offset);
  getchar();
  printf("Enter whence value:\n");
  scanf("%d",&whence);
  getchar();
  lseek(fd,offset,whence);
  //printf("After SEEK, Offset is now %ld", *buffOff);
  getUserInput();
}

void getUserInput(){

	char inputBuff[buffSize];
  //printf("Offset value has been initialized to : %ld\n", *buffOff);
  printf("\nPress r to read");
  printf("\nPress w to write");
  printf("\nPress s to seek to a position in the file");
  printf("\nPress e to exit");
  printf("\nPress anything else to read these commands again.\n");
  printf("\nEnter Input:\n");
	if (fgets(inputBuff, buffSize, stdin) != NULL){
		if (inputBuff[0] == 'r'){
			readChar();
		}
		else if (inputBuff[0] == 'w'){
			writeChar();
		}
    else if(inputBuff[0] == 's'){
      seekChar();
    }
		else if (inputBuff[0] == 'e'){
		}
		else{
			printf("Please try again! Here are your options:\n");
			getUserInput();
		}
	}
}

int main(){
	getUserInput();
}
