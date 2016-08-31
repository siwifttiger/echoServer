//能处理多个客户端请求的回射服务器


#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<error.h>
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
struct package{
	int len;
	char buf[1024];
};

void doService(int conn){
	struct package recvbuf;
	int n;
	while(1){
		memset(&recvbuf,0,sizeof(recvbuf));
		int ret = readn(conn,&recvbuf.len,4);
		if(ret == -1){
			ERR_EXIT("read");
		}
		else if(ret < 4){
			std::cout<< "client close" << std::endl;
			break;
		}
		n = ntohl(recvbuf.len);
		ret = readn(conn,&recvbuf.buf,n);
		if(ret == -1){
			ERR_EXIT("read");
		}
		else if(ret < n){
			std::cout<< "client close" << std::endl;
			break;
		}
		fputs(recvbuf.buf,stdout);
		write(conn,&recvbuf,4+n);
		
	}
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
			close(socketfd);
			doService(conn);
			exit(EXIT_SUCCESS);
		}
		else{
			close(conn);
		}	

	}
}
