
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>

#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>

#define P_MCP_CLK	14
#define P_MCP_DIN	12
#define P_MCP_DOUT	13
#define P_MCP_CS0	10
#define SPI_CHAN_0	0

#define SPI_SPEED	10000000

#define LED_DEV_PATH_NAME		"/dev/led"

int readMCP3008(int fd, uint8_t analogCh){
	uint8_t buf[3];
	int val;
	
	buf[0] = 0x06 | ((analogCh & 0x07) >> 2);
	buf[1] = ((analogCh & 0x07) << 6);
	buf[2] = 0x00;
	
	wiringPiSPIDataRW(fd, buf,3);
	
	buf[1] = 0x0F & buf[1];
	val = (buf[1] << 8) | buf[2];
	
	return val;
}

int send_data(int state){
	int fd1 = open(LED_DEV_PATH_NAME, O_RDWR);
	if(fd1 == -1){
		perror("fail to open\n");
		return 1;
	}
	
	write(fd1, &state, 4);
	sleep(0.5);

	close(fd1);
	return 0;
}

int main(void){
	int fd;
	int val;
	
	if(wiringPiSetup() == -1){ return 1; }
	if((fd = wiringPiSPISetup(SPI_CHAN_0, SPI_SPEED)) == -1) { return 1; }
	
	while(1){
		val = readMCP3008(fd, 0);
		printf("Val = %d\n", val);
		
		if(val > 4000){ send_data(1); }
		else{ send_data(0); }
		
		delay(1000);
	}
	return 0;
}

