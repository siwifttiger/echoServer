//能处理多个客户端请求的回射服务器


#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<error.h>
#include<unistd.h>
#include<string.h>
#define ERR_EXIT(m) \
	do\
	{\
		perror(m); \
		exit(EXIT_FAILURE); \
	}while(0)
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
	
	//开启
	int on = 1;
	//REUSEADDR  为了能够立即重启
	if(setsockopt(socketfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) < 0)
		ERR_EXIT("setsockopt");
	
	//将套接字和地址绑定
        if((bind(socketfd,(struct sockaddr*)&servaddr,sizeof(servaddr))) < 0)
		ERR_EXIT("bind");
	//被动套接字，接受连接
	if(listen(socketfd,SOMAXCONN) < 0)
		ERR_EXIT("listen");
	struct sockaddr_in peeraddr; //对等方地址
	socklen_t peerlen = sizeof(peeraddr); 	//对等方地址长度
	//已连接套接字,经accept返回后是主动套接字
	int conn;
	pid_t pid;
	while(1){
		if((conn =accept(socketfd,(struct sockaddr*)&peeraddr,&peerlen)) < 0)
			ERR_EXIT("accept");
		//连接成功后打印客户端的ip地址和端口号
		std::cout << "ip = " << inet_ntoa(peeraddr.sin_addr) << " port= " << ntohs(peeraddr.sin_port) << std::endl;
		//让子进程处理通信
		pid = fork();
		if(pid == -1)
			ERR_EXIT("fork");
		else if(pid == 0){
			char sendbuf[1024] = {0};
			while(fgets(sendbuf,sizeof(sendbuf),stdin) != NULL){
				write(conn,sendbuf,strlen(sendbuf));
				memset(sendbuf,0,sizeof(sendbuf));
			}
			exit(EXIT_SUCCESS);
		}
		else{
				
			char recvbuf[1024];
			while(1){
				memset(recvbuf,0,sizeof(recvbuf));
				//获取数据
				int ret = read(conn,recvbuf,sizeof(recvbuf));
			//客户端关闭 输出一些信息
				if(ret == 0){
					std::cout<< "client close" << std::endl;
					break;
				}
				//失败
				else if(ret == -1){
					ERR_EXIT("read");
				}
		
				//打印数据
				fputs(recvbuf,stdout);
			}
			eixt(EXIT_SUCCESS);
		}	

	}
}
