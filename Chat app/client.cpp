#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include <cmath>

#define MAXLINE     1024
#define SERV_PORT   29988   //server使用的本地端口
#define LISTENQ     100     //listen侦听队列最大数
typedef struct sockaddr SA;
using namespace std;
void sig_chld(int signo) {
    pid_t pid;
    int stat;
    //使用waitpid回收子进程
    while((pid=waitpid(-1,&stat,WNOHANG))>0){
        cout << "child " << pid << " terminated." << std::endl;
    }
    return ;
}
void str_echo(int sockfd) {
    ssize_t n;
    char buf[MAXLINE];
    again:
    while((n=recv(sockfd,buf,MAXLINE, 0))>0){
        send(sockfd,buf,n, 0);
    }
    //如果errno为EINTR，表示被信号中断的系统调用，可以重入，应该重新调用
    if(n<0 && errno==EINTR){
        goto again;
    } else {
        cout << "str_echo read error." << endl;
        exit(-1);
    }
}
int main(int argc , char** argv) {
    int listenfd,connfd,udpfd,nready,maxfdp1;
    char mesg[MAXLINE];
    pid_t childpid;
    fd_set rset;
    ssize_t n;
    socklen_t len;
    const int on = 1;
    struct sockaddr_in cliaddr,servaddr;
    void sig_chld(int);

    //创建TCP本地套接字，指定端口，地址任意选取本地接口
    listenfd = socket(AF_INET,SOCK_STREAM,0);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr = htonl(0);
    servaddr.sin_port=htons(SERV_PORT);
    //设置套接字端口可以重入
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
    //绑定一个本地套接字地址
    bind(listenfd,(SA*)&servaddr,sizeof(servaddr));
    //侦听本地套接字，最大排队数量为LISTENQ
    listen(listenfd,LISTENQ);

    //创建本地UDP套接字
    udpfd = socket(AF_INET,SOCK_DGRAM,0);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=htonl(0);
    servaddr.sin_port = htons(SERV_PORT);
    //绑定一个本地套接字地址
    bind(udpfd,(SA*)&servaddr,sizeof(servaddr));

    //注册信号SIGCHLD，回收fork子进程
    signal(SIGCHLD,sig_chld);

    FD_ZERO(&rset);
    maxfdp1 = max(listenfd,udpfd) +1;

    for(;;){
        //添加侦听套接字listenfd和本地udp套接字到select可读集合
        FD_SET(listenfd,&rset);
        FD_SET(udpfd,&rset);
        //使用select判断是否可以读
        if((nready = select(maxfdp1,&rset,NULL,NULL,NULL)<0)){
            if(errno == EINTR){
                continue;
            } else {
                cout << "select error ." << endl;
                exit(-1);
            }
        }
        //检测listenfd
        if(FD_ISSET(listenfd,&rset)){
            len = sizeof(cliaddr);
            connfd = accept(listenfd,(SA*)&cliaddr,&len);
            //创建子进程，实现并发服务器
            if((childpid=fork())==0){
                close(listenfd);
                str_echo(connfd);
                exit(0);
            }
            close(connfd);
        }
        //检测udpfd
        if(FD_ISSET(udpfd,&rset)){
            len=sizeof(cliaddr);
            n=recvfrom(udpfd,mesg,MAXLINE,0,(SA*)&cliaddr,&len);
            sendto(udpfd,mesg,n,0,(SA*)&cliaddr,len);
        }
    }
    return 0;
}       
