#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#define SPEAKER_MAJOR_NUMBER		503
#define SPEAKER_MINOR_NUMBER		100
#define SPEAKER_DEV_PATH_NAME		"/dev/speaker_dev"

#define IOCTL_MAGIC_NUMBER		'j'
#define IOCTL_CMD_SET_DIRECTION			_IOWR(IOCTL_MAGIC_NUMBER,0,int)
#define IOCTL_CMD_TOGGLE				_IOWR(IOCTL_MAGIC_NUMBER,1,int)
#define IOCTL_CMD_LEVEL					_IOWR(IOCTL_MAGIC_NUMBER,2,int)

#define CS_MCP 11
#define SPI_CHANNEL 0
#define SPI_SPEED 1000000

#define CS_MCP3008              8  


int read_mcp(unsigned char adcChannel){
	
unsigned char buff[3];
int adcValue =0;
//adcChannel = 5;
buff[0] = 0x06 | ((adcChannel & 0x07)>>2);
buff[1] = ((adcChannel& 0x07) <<6);
buff[2] = 0x00;

printf("%d %d %d\n",buff[0],buff[1],buff[2]);

wiringPiSPIDataRW(SPI_CHANNEL,buff,3);
buff[1] = 0x0f & buff[1];

printf("%d %d %d\n",buff[0],buff[1],buff[2]);
adcValue = (buff[1]<<8) | buff[2];

return adcValue;
}

int main(void){
	
	dev_t speaker_dev;
	int dev;
	int pin_direction = -1,off=1, in=0;
	char str[10];
	
	
	speaker_dev = makedev(SPEAKER_MAJOR_NUMBER, SPEAKER_MINOR_NUMBER);
	mknod(SPEAKER_DEV_PATH_NAME, S_IFCHR|0666, speaker_dev);
	
	
		dev =open(SPEAKER_DEV_PATH_NAME,O_RDWR);
	if(dev==-1){
		perror("failed to open; because ");
		return 1;
		}

	pin_direction =1;
	ioctl(dev,IOCTL_CMD_SET_DIRECTION, &pin_direction); //set pin out

	int adcChannel = 0;
	int adcValue =0;
	
	if(wiringPiSetup() == -1)return 1;
	if(wiringPiSPISetup(SPI_CHANNEL,SPI_SPEED)==-1)return 1;
	pinMode(CS_MCP3008, OUTPUT);

	printf("while loop start!\n");
	while(1){
		adcValue = read_mcp(adcChannel);
		printf("%d\n",adcValue);
		if(adcValue<1000)
		ioctl(dev, IOCTL_CMD_TOGGLE,&in);
		else
		ioctl(dev, IOCTL_CMD_TOGGLE,&off);
		
		delay(5);
}
	close(dev);
	return 0;
}


