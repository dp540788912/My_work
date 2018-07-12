/* 
This is a very newbie program 
Created by frank lee
*/


#include "util.h"



void Handle_UDP (int udpfd) {
    // set up
    struct sockaddr_in cliaddr;
    struct sockaddr_in* cliaddr2 = malloc( sizeof(struct sockaddr_in))  ;
    socklen_t len = sizeof(cliaddr);
    char mesg[MAX_MSGLEN];
    // check
    
    
    // recieve from UDP
    recvfrom(udpfd,mesg,MAX_MSGLEN,0,(SA*)cliaddr2,&len);
    cliaddr = *cliaddr2;
    printf("MAIN: Rcvd incoming UDP datagram from: %s\n", inet_ntoa((struct in_addr)cliaddr.sin_addr));
    fflush(stdout);

    //niu bi de cheng xu
    int arg_num = 0;
    char ag_ary[MAX_CLIENT][MAX_MSGLEN_REAL]; // argument array
    rm_symbol(ag_ary, mesg, &arg_num, 4);

    if (strcmp(ag_ary[0], "LOGIN") == 0){
        if (arg_num < 2){
            char* tmp1 = "ERROR Inlvalid format\n";
            sendto(udpfd,tmp1,strlen(tmp1),0,(SA*)&cliaddr,len);
            return;
        } else if (strlen(ag_ary[1]) < 3 || strlen(ag_ary[1]) > 20){
        	char* tmp1 = "ERROR Not valid name\n";
            sendto(udpfd,tmp1,strlen(tmp1),0,(SA*)&cliaddr,len);
            return;
        }

        printf("MAIN: Rcvd LOGIN request for userid %s\n", ag_ary[1]);
        fflush(stdout);

        int ind = UDP_LOGIN(ag_ary[1], (struct in_addr)cliaddr.sin_addr);
        if ( ind == 0){
            char* tmp1 = "ERROR Already connected\n";
            printf("MAIN: Sent ERROR (Already connected)\n");
            fflush(stdout);
            sendto(udpfd,tmp1,strlen(tmp1),0,(SA*)&cliaddr,len);
        } else if ( ind == 1){
        	printf("MAIN: Sent ERROR (Already connected)\n");
            fflush(stdout);
            char* tmp1 = "ERROR Already connected\n";
            sendto(udpfd,tmp1,strlen(tmp1),0,(SA*)&cliaddr,len);
        } else if ( ind == 2 || ind == 3 ){
        	char* tmp10 = "OK\n";
            sendto(udpfd,tmp10,3,0,(SA*)&cliaddr,len);
        }  else {
            // new user login 
            char* tmp10 = "OK\n";
            sendto(udpfd,tmp10,3,0,(SA*)&cliaddr,len);
            p(&mutex);
                strcpy(UDP_c[UDPc_num].name, ag_ary[1]);
                UDP_c[UDPc_num].client = (struct in_addr)cliaddr.sin_addr;
                UDP_c[UDPc_num].socket = -1;
                UDP_c[UDPc_num].cliaddr = cliaddr2;
                UDP_c[UDPc_num].fd_udp = udpfd;
                UDPc_num++;
            v(&mutex);
        }
    } else{
        // if not loin 
        if (!chkUDPIfLoin((struct in_addr)cliaddr.sin_addr)){
        	printf("MAIN: Sent ERROR (Not logged in)\n");
        	fflush(stdout);
            char* tmp1 = "ERROR Not logged in\n";
            sendto(udpfd,tmp1,strlen(tmp1),0,(SA*)&cliaddr,len);
        } else {
            if(strcmp(ag_ary[0], "WHO") == 0){
            	printf("MAIN: Rcvd WHO request\n");
            	fflush(stdout);
                char tmp3[CHUNK*2];
                handle_WHO(tmp3);
                sendto(udpfd,tmp3,strlen(tmp3),0,(SA*)&cliaddr,len);
            } else if (strcmp(ag_ary[0], "LOGOUT") == 0){
            	printf("MAIN: Rcvd LOGOUT request\n");
            	fflush(stdout);
            	char* tmp10 = "OK\n";
            	sendto(udpfd,tmp10,3,0,(SA*)&cliaddr,len);
                UDP_logout((struct in_addr)cliaddr.sin_addr);
            } else if (strcmp(ag_ary[0], "SEND") == 0){
                // check if valid
                handle_send(ag_ary,arg_num, udpfd , cliaddr,len, 1);
            } else if (strcmp(ag_ary[0], "BROADCAST" ) == 0) {
            	char* tmp10 = "OK\n";
            	sendto(udpfd,tmp10,3,0,(SA*)&cliaddr,len);

            	int arg_num2 = 0;
	    		char ag_ary2[4][MAX_MSGLEN_REAL];
            	rm_symbol(ag_ary2, mesg, &arg_num2, 3);

            	handle_broadcast(ag_ary2, arg_num2, udpfd, cliaddr, len, 1);
            } else if (strcmp(ag_ary[0], "SHARE")== 0){
            	char* tmp1 = "SHARE not supported over UDP\n";
            	sendto(udpfd,tmp1,strlen(tmp1),0,(SA*)&cliaddr,len);
            }
        } 
    }
}


void* Deal_with_TCP (void* arg){
	// detach
	pthread_detach(pthread_self());
	// accept the connection
	int TCPfd = *(int*)arg;
	struct sockaddr_in cliaddr;
	socklen_t len = sizeof(cliaddr);
	int connfd = accept(TCPfd,(SA*)&cliaddr,&len);


	// error check 
    printf("MAIN: Rcvd incoming TCP connection from: %s\n", inet_ntoa((struct in_addr)cliaddr.sin_addr));
    fflush(stdout);
    // recv
    int location = -1;
    ssize_t n;
    char buf[MAX_MSGLEN];
    int login_stat = 0;
    char cur_id[20];
    again:
    while((n=recv(connfd,buf,MAX_MSGLEN, 0))>0){
    	int arg_num = 0;
	    char ag_ary[4][MAX_MSGLEN_REAL]; // argument array
	    rm_symbol(ag_ary, buf, &arg_num, 4);
        // send(connfd,buf,n, 0);
        if (login_stat == 0){
        	
	        if( strncmp(ag_ary[0], "LOGIN", 5) == 0){
	        	printf("CHILD %u: Rcvd LOGIN request for userid %s\n", (unsigned int)pthread_self(), ag_ary[1] );
	        	fflush(stdout);

	        	//set up struct and store client information 
	        	if (arg_num < 2){
	        		char* tmp0 = "Not invalid\n";
	        		send (connfd, tmp0, strlen(tmp0) ,0);
	        		continue;
	        	} else if( strlen(ag_ary[1]) < 3 || strlen(ag_ary[1]) > 20){
	        		printf("CHILD %u: Sent error (Invalid userid)\n", (unsigned int)pthread_self());
	        		fflush(stdout);
	        		char* tmp1 = "ERROR <Invalid userid>\n";
	        		send (connfd, tmp1, strlen(tmp1) ,0);
	        		continue;
	        	} else {
	        		if (TCPNameCheck(ag_ary[1], connfd) == 0) continue;
	        	}
	        	p(&mutex);
		        	strcpy(TCP_c[TCPc_num].name, ag_ary[1]);
		        	TCP_c[TCPc_num].socket = connfd;
		        	TCP_c[TCPc_num].client = (struct in_addr)cliaddr.sin_addr;
		        	// if ( strcmp (inet_ntoa(TCP_c[TCPc_num].client) , inet_ntoa((struct in_addr)cliaddr.sin_addr)) == 0)
		        	// printf( "Received message from %s\n",
           			//        inet_ntoa( TCP_c[TCPc_num].client) );
		        	location = TCPc_num;
		        	TCPc_num++;
		        	login_stat = 1;
	        	v(&mutex);
	        	strcpy(cur_id, ag_ary[1]);
	        	char* tmp10 = "OK\n";
	        	send (connfd, tmp10, 3 ,0);

	        } else {
	        	printf("CHILD %u: Rcvd %s request\n", (unsigned int)pthread_self(), ag_ary[0]);
        		fflush(stdout);
        		printf("CHILD %u: Sent ERROR (Not logged in)\n", (unsigned int)pthread_self());
        		fflush(stdout);
	        	char* my_send = "ERROR Not logged in\n";
	        	send (connfd, my_send, strlen(my_send) ,0);
	        }
	    } else {
	        if( strcmp(ag_ary[0], "LOGIN") == 0){
	        	printf("CHILD %u: Rcvd LOGIN request for userid %s\n", (unsigned int)pthread_self(), ag_ary[1] );
	        	fflush(stdout);
	        	if (strcmp(cur_id, ag_ary[1]) == 0){
	        		printf("CHILD %u: Sent ERROR (Already connected)\n", (unsigned int)pthread_self());
            		fflush(stdout);
	        		char* tmp1 = "ERROR Already connected\n";
	        		send (connfd, tmp1, strlen(tmp1) ,0);
	        	} else {
	        		char* tmp10 = "OK\n";
	        		send (connfd, tmp10, 3 ,0);
	        		strcpy(TCP_c[location].name, ag_ary[1]);
	        	}
	   
	        } else if(strcmp(ag_ary[0], "WHO") == 0){
	        	printf("CHILD %u: Rcvd WHO request\n", (unsigned int)pthread_self());
	        	fflush(stdout);

	        	handle_WHO_TCP(connfd);
	        } else if (strcmp(ag_ary[0], "LOGOUT") == 0){
	        	printf("CHILD %u: Rcvd LOGOUT request\n", (unsigned int)pthread_self());
	        	fflush(stdout);
	        	char* tmp10 = "OK\n";
	        	send (connfd, tmp10, 3 ,0);
	        	logout(location);
	        	login_stat = 0;
	        	
	        } else if (strcmp(ag_ary[0], "SEND") == 0){
	        	printf("CHILD %u: Rcvd SEND request to userid %s\n", (unsigned int)pthread_self(), ag_ary[1]);
	        	fflush(stdout);
	        	handle_send(ag_ary, arg_num, connfd, cliaddr, len, 0);
	        } else if (strcmp(ag_ary[0], "BROADCAST") == 0 ){
	        	printf("CHILD %u: Rcvd BROADCAST request\n", (unsigned int)pthread_self());
	        	fflush(stdout);
	        	char* tmp10 = "OK\n";
	        	send (connfd, tmp10, 3 ,0);

	        	int arg_num2 = 0;
	    		char ag_ary2[MAX_CLIENT][MAX_MSGLEN_REAL];
	        	rm_symbol(ag_ary2, buf, &arg_num2, 3);

	        	handle_broadcast(ag_ary2, arg_num2, connfd, cliaddr, len, 0);
	        } else if (strcmp(ag_ary[0], "SHARE") == 0){
	        	printf("CHILD %u: Rcvd SHARE request\n", (unsigned int)pthread_self());
	        	fflush(stdout);
	        	handle_share(ag_ary, connfd, cliaddr, len);
	        }
	    }

    }
    if(n<0 && errno==EINTR){
        goto again;
    } else {
        printf("CHILD %u: Client disconnected\n", (unsigned int)pthread_self());
        fflush(stdout);
      	logout(location);
      	close(connfd);
    }
    return NULL;
}


int main(int argc, char const *argv[])
{
	// set up arguments 
	if (argc < 3) {
		fprintf(stderr, "ERROR < Not enough argument >\n" );
		return EXIT_FAILURE;
	}

	int TCPport = atoi(argv[1]);
	int UDPport = atoi(argv[2]);

	// set uo these parameters 
	int listenfd,udpfd,nready,maxfdp1;
	fd_set rset; // for select 
    const int on = 1;
    struct sockaddr_in TCPservaddr, UDPservaddr;

    // set up tcp 
    listenfd = socket(AF_INET,SOCK_STREAM,0);
    bzero(&TCPservaddr,sizeof(TCPservaddr));
    TCPservaddr.sin_family=AF_INET;
    TCPservaddr.sin_addr.s_addr = htonl(0);
    TCPservaddr.sin_port=htons(TCPport);
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
    bind(listenfd,(SA*)&TCPservaddr,sizeof(TCPservaddr));
    listen(listenfd,LISTENQ);

    // set up UDP
	udpfd = socket(AF_INET,SOCK_DGRAM,0);
    bzero(&UDPservaddr,sizeof(UDPservaddr));
    UDPservaddr.sin_family = AF_INET;
    UDPservaddr.sin_addr.s_addr=htonl(0);
    UDPservaddr.sin_port = htons(UDPport);
    bind(udpfd,(SA*)&UDPservaddr,sizeof(UDPservaddr));

    // for select
    FD_ZERO(&rset);
    maxfdp1 = MAX(listenfd,udpfd) +1;

    FD_SET(listenfd,&rset);
    FD_SET(udpfd,&rset);
  	int length1 = sizeof(UDPservaddr);
    if ( getsockname( udpfd, (struct sockaddr *) &UDPservaddr, (socklen_t *) &length1 ) < 0 )
  	{
	    perror( "getsockname() failed" );
	    return EXIT_FAILURE;
  	}
    
    printf("MAIN: Started server\n");
    printf("MAIN: Listening for TCP connections on port: %d\n", TCPport);
    printf("MAIN: Listening for UDP datagrams on port: %d\n", UDPport);
    fflush(stdout);
    while(1){ 
    	
        // bang, select 
        FD_SET(listenfd,&rset);
        FD_SET(udpfd,&rset);
        if((nready = select(maxfdp1,&rset,NULL,NULL,NULL) < 0)){
            if(errno == EINTR){
                continue;
            } else {
                fprintf(stderr, "ERROR < Select Error >\n" );
                exit(-1);
            }
        }
        // check TCP
        if(FD_ISSET(listenfd,&rset)){
        	pthread_t tid;
            int rc = pthread_create(&tid, NULL, Deal_with_TCP, &listenfd);
            if ( rc != 0 )
		    {
		      fprintf( stderr, "MAIN: ERROR <Could not create thread>\n" );
		      return EXIT_FAILURE;
		    }
            // 
            // len = sizeof(cliaddr);
            // connfd = accept(listenfd,(SA*)&cliaddr,&len);
        }

        if(FD_ISSET(udpfd,&rset)){
        	Handle_UDP(udpfd);
        }

    }

	return 0;
}