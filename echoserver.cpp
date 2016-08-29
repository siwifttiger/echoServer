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
	if((conn =accept(socketfd,(struct sockaddr*)&peeraddr,&peerlen)) < 0)
		ERR_EXIT("accept");
	//接下来可以进行数据通信了
	char recvbuf[1024];
	while(1){
		memset(recvbuf,0,sizeof(recvbuf));
		//获取数据
		int ret = read(conn,recvbuf,sizeof(recvbuf));
		char* p = recvbuf+ret;
		for(int i = 65;i<=70;i++ ){
			*p=i;
			p++;
		}	
		//打印数据
		fputs(recvbuf,stdout);
		write(conn,recvbuf,sizeof(recvbuf));
		memset(recvbuf,0,sizeof(recvbuf));
	}
	close(conn);
	close(socketfd);


}
