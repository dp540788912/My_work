#include <unistd.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <sys/socket.h>  
#include <arpa/inet.h>  
#include <string.h>  
#include <sys/types.h>
  
#define SERV_PORT 8123 
#define SA struct sockaddr  
  
int main(int argc, char** argv)  
{  
    int sockfd;  
    struct sockaddr_in servaddr;  
      
    bzero(&servaddr, sizeof(servaddr));  
  
    servaddr.sin_family = AF_INET;  
    servaddr.sin_port = htons(SERV_PORT);  
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);  
  
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);  

    int MAXLINE = 4096  ;
    int n;  
    char sendline[MAXLINE], recvline[MAXLINE+1];  

    while(fgets(sendline, MAXLINE, stdin) != NULL)  
    {  
        sendto(sockfd, sendline, strlen(sendline), 0, servaddr, sizeof(servaddr));  
    }
  
    return 0;  
}  