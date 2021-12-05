#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <softPwm.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>

#define PORTNUM		0x5005
#define BUFFSIZE	256
#define P_SVM		26

#define MOTOR_MAJOR_NUBMER  	 501
#define MOTER_MINOR_NUMBER		100
#define MOTER_DEV_PATH_NAME		"/dev/dcmoter"


#define I2C_ADDR   0x27
#define LCD_CHR  1
#define LCD_CMD  0

#define LINE1  0x80
#define LINE2  0xC0
#define LCD_BACKLIGHT   0x08
#define ENABLE  0b00000100


void lcd_init(void);
void lcd_byte(int bits, int mode);
void lcd_toggle_enable(int bits);
void typeInt(int i);
void typeFloat(float myFloat);
void lcdLoc(int line);
void ClrLcd(void);
void typeln(const char *s);
void typeChar(char val);
int fd;

void typeFloat(float myFloat)   {
  char buffer[20];
  sprintf(buffer, "%4.2f",  myFloat);
  typeln(buffer);
}



void ClrLcd(void)   {
  lcd_byte(0x01, LCD_CMD);
  lcd_byte(0x02, LCD_CMD);
}

void lcdLoc(int line)   {
  lcd_byte(line, LCD_CMD);
}


void typeln(const char *s)   {

  while ( *s ) lcd_byte(*(s++), LCD_CHR);

}

void lcd_byte(int bits, int mode)   {


  int bits_high;
  int bits_low;
  bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT ;
  bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT ;

  wiringPiI2CReadReg8(fd, bits_high);
  lcd_toggle_enable(bits_high);

  wiringPiI2CReadReg8(fd, bits_low);
  lcd_toggle_enable(bits_low);
}

void lcd_toggle_enable(int bits)   {
  delayMicroseconds(500);
  wiringPiI2CReadReg8(fd, (bits | ENABLE));
  delayMicroseconds(500);
  wiringPiI2CReadReg8(fd, (bits & ~ENABLE));
  delayMicroseconds(500);
}


void lcd_init()   {
  lcd_byte(0x33, LCD_CMD);
  lcd_byte(0x32, LCD_CMD);
  lcd_byte(0x06, LCD_CMD);
  lcd_byte(0x0C, LCD_CMD);
  lcd_byte(0x28, LCD_CMD);
  lcd_byte(0x01, LCD_CMD);
  delayMicroseconds(500);
}

void wheel_turn(int direction){
	int i;
	if(wiringPiSetup() == -1){ exit(1); }

	pinMode(P_SVM, SOFT_PWM_OUTPUT);
	softPwmCreate(P_SVM, 0, 200);
	printf("direction = %d\n", direction);
	softPwmWrite(P_SVM, direction);
}

int move_kart_front_back(int state){
	int fd = open(MOTER_DEV_PATH_NAME, O_RDWR);
	if(fd == -1){
		perror("fail to open\n");
		return 1;
	}

	ClrLcd();
	if(state ==2 || state ==5){
		lcdLoc(LINE1);
		typeln("fire!!!!");
		lcdLoc(LINE2);
		typeln("room 401");
	}
	else{
		lcdLoc(LINE1);
		typeln(" ");
		lcdLoc(LINE2);
		typeln(" ");
	}

	if( state == 1 ){
		wheel_turn(5);
		write(fd, &state, 4);
	}
	else if( state == 2){
		int s =1;
		wheel_turn(15);
		write(fd, &s, 4);
	}
	else if( state == 3){
		int s = 1;
		wheel_turn(30);
		write(fd, &s, 4);
	}
	else if( state == 4){
		int s = 2;
		wheel_turn(5);
		write(fd, &s, 4);
	}
	else if( state == 5){
		int s = 2;
		wheel_turn(15);
		write(fd, &s, 4);
	}
	else if( state == 6){
		int s = 2;
		wheel_turn(30);
		write(fd, &s, 4);
	}
	else if( state == 0){
		int s = 3;
		wheel_turn(15);
		write(fd, &s, 4);
	}

	close(fd);
	return 0;
}

int socket_receive(){
	struct sockaddr_in serv_addr, cli_addr;
	int serv_fd, cli_fd, clilen;
	char buffer[BUFFSIZE];

	if (wiringPiSetup () == -1) exit (1);
	fd = wiringPiI2CSetup(I2C_ADDR);
	lcd_init(); // setup LCD

	printf("Socket programming activate\n");

	serv_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(serv_fd == -1){
		perror("ERROR opening socket\n");
		exit(1);
	}

	memset((char*) &serv_addr, 0x00, sizeof(serv_addr));
	serv_addr.sin_family			= AF_INET;
	serv_addr.sin_addr.s_addr 		= htonl(INADDR_ANY);
	serv_addr.sin_port				= htons(PORTNUM);

	if(bind(serv_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1){
		perror("ERROR on binding\n");
		exit(1);
	}

	listen(serv_fd, 5);

loop:

	clilen = sizeof(cli_addr);
	if((cli_fd = accept(serv_fd, (struct sockaddr *)&cli_addr,&clilen)) == -1) {
		perror("ERROR on accept");
		exit(1);
	}
	write(cli_fd, "Send Data Please\n", BUFFSIZE);
	while(1) {
		memset(buffer, 0x00, sizeof(buffer));

		if((read(cli_fd, buffer, BUFFSIZE)) == -1) {
			perror("ERROR reading from socket");
			exit(1);
		}
		printf("Data : %s\n", buffer);
		if(move_kart_front_back(atoi(buffer)) == 1){
			perror("ERROR reading from socket");
			exit(1);
		}

		if(buffer[0] == 'q') {
			close(cli_fd);
			goto loop;
		}

		buffer[0] = '1';
		write(cli_fd, buffer, BUFFSIZE);
		if(buffer[0] == 'q'){break;}
	}

	close(serv_fd);
}


int main(void){
	socket_receive();
	return 0;
}
