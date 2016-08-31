#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<errno.h>
#include<unistd.h>
#include<string.h>
#define ERR_EXIT(m) \
	do\
	{\
		perror(m); \
		exit(EXIT_FAILURE); \
	}while(0)


//接收确定的count个字节数的包，解决粘包问题
ssize_t readn(int fd, void *buf, size_t count){
	size_t nleft = count;   //nleft 剩余的字节数
	ssize_t nread;         // nread 已接收的字节数
	char *bufp = (char*)buf;
	//循环读，直到读取完毕
	while(nleft){
		//小于0有两种情况
		if((nread = read(fd,bufp,nleft)) < 0){
			//如果是信号中断，可以继续
			if(errno == EINTR)
				continue;
			//如果是读取失败，就返回-1
			return -1;
		}	
		//对等方关闭连接，直接返回
		else if(nread == 0){
			return count-nleft;
		}
		//成功就继续读
		nleft -= nread;
		bufp += nread;
	}
	return count;
}

//向fd写count个字节数的数据
ssize_t writen(int fd, const void *buf, size_t count){
	size_t nleft = count;   //nleft 剩余的字节数
	ssize_t nwrite;         // nread 已接收的字节数
	char *bufp = (char*)buf;
	//循环读，直到读取完毕
	while(nleft){
		//小于0有两种情况
		if((nwrite = write(fd,bufp,nleft)) < 0){
			//如果是信号中断，可以继续
			if(errno == EINTR)
				continue;
			//如果是读取失败，就返回-1
			return -1;
		}	
		//对等方关闭连接，直接返回
		else if(nwrite == 0){
			continue;
		}
		//成功就继续读
		nleft -= nwrite;
		bufp += nwrite;
	}
	return count;
}
//自定义包
struct package{
	int len;    //包头，存放包的大小
	char buf[1024]; //包体长度
};
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
	
	//接下来可以进行数据通信了
	struct package recvbuf;
	struct package sendbuf;
	memset(&recvbuf,0,sizeof(recvbuf));
	memset(&sendbuf,0,sizeof(recvbuf));
	int n;
	while(fgets(sendbuf.buf,sizeof(sendbuf.buf),stdin) != NULL){
		n = strlen(sendbuf.buf);
		sendbuf.len = htonl(n);                 //转化成网络字节序传输
		writen(socketfd,&sendbuf,4+n);
		int ret = readn(socketfd,&recvbuf.len,4);
		if(ret == -1){
			ERR_EXIT("read");
		}
		else if(ret < 4){
			std::cout<< "client close" << std::endl;
			break;
		}
		n = ntohl(recvbuf.len);
		ret = readn(socketfd,&recvbuf.buf,n);
		if(ret == -1){
			ERR_EXIT("read");
		}
		else if(ret < n){
			std::cout<< "client close" << std::endl;
			break;
		}
		fputs(recvbuf.buf,stdout);
		memset(&sendbuf,0,sizeof(sendbuf));
		memset(&recvbuf,0,sizeof(recvbuf));
	}
	close(socketfd);


}
