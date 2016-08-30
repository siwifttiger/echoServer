#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<error.h>
#include<unistd.h>
#include<signal.h>
#include<string.h>
#define ERR_EXIT(m) \
	do\
	{\
		perror(m); \
		exit(EXIT_FAILURE); \
	}while(0)


void handler(int sig){
	std::cout<< "recieve sig " << sig << std::endl;
	exit(EXIT_SUCCESS);
}

int main(){
	int socketfd = -1;
	//创建一个套接字
	if((socketfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) < 0)
		ERR_EXIT("socket");
	struct sockaddr_in  servaddr; //ipv4 地址格式
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5188);  //转换为网络字节序
	//将点分十进制转化为一个32位长整数类型，网络字节序
servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//主动套接字，连接
	if((connect(socketfd,(struct sockaddr*)&servaddr,sizeof(servaddr))) < 0)
		ERR_EXIT("connect");
	
	
	pid_t pid;
	pid = fork();
	if(pid == -1)
		ERR_EXIT("fork");
	if(pid == 0){
		//子进程用来接收数据
		char recvBuf[1024];
		char s[1024]; 
		while(1){
			memset(recvBuf,0,sizeof(recvBuf));
			int ret = read(socketfd,recvBuf,sizeof(recvBuf));
			if(ret == -1){
				ERR_EXIT("read");

			}
			else if(ret == 0){
				std::cout << "peer close" << std::endl;
				break;
			}
			else{
				memset(s,1024,sizeof(s));
				s[0] = 'O';
				s[1] = 't';
				s[2] = 'h';
				s[3] = 'e';
				s[4] = 'r';
				s[5] = ' ';
				s[6] = 0;
				strcat(s,recvBuf);
				fputs(s,stdout);
			}
		}
		close(socketfd);
		kill(getppid(),SIGUSR1);
	}
	else{
		signal(SIGUSR1,handler);
		//父进程用来发送数据
		char sendBuf[1024] = {0};
		while(fgets(sendBuf,sizeof(sendBuf),stdin) != NULL){
			write(socketfd,sendBuf,strlen(sendBuf));
			memset(sendBuf,0,sizeof(sendBuf));
		}
		close(socketfd);
		
	}
	
	


}
