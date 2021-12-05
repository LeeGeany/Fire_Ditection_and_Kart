#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>

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
		
		//memset(buffer, 0x00, sizeof(buffer));
		//printf("[Server] ");
		//fgets(buffer, BUFFSIZE, stdin);
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

