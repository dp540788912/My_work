#ifndef _util_h_
#define _util_h_


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
#include <pthread.h>
#include <ctype.h>

#define MAX_CLIENT 128
#define LISTENQ 5
#define ID_MIN 3
#define ID_MAX 20
#define MAX_MSGLEN 994
#define MAX_MSGLEN_REAL 994
#define CHUNK 1024

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y)) // self defined max function

// for convenient 

//======================================================
//============Struct and variable define================
//======================================================
typedef struct sockaddr SA; 

// store client 
typedef struct thread_args {
    int socket; // if socket == -1, it represents the UDP client 
    int fd_udp;
    struct in_addr client;
    struct sockaddr_in* cliaddr;
    char name[20];
} CA;

//port  
int udpport = 0;
int tcpport = 0;

//Clients information 
int TCPc_num = 0;
int UDPc_num = 0;
CA TCP_c[MAX_CLIENT]; // TCP clients
CA UDP_c[MAX_CLIENT]; // UDP clients
//======================================================
//======================================================
//======================================================

//======================================================
//===================Mutex function=====================
//======================================================
int mutex = 1;
void p(int* mutex){
	while(*mutex == 0);
	(*mutex)--;
}
// release opreation
void v(int* mutex){
	(*mutex)++;
}
//======================================================
//======================================================
//======================================================


static int myCompare (const void* a, const void* b)
{
    return strcmp ( (char*)a, (char*)b);
}
 
void sort(char arr[MAX_CLIENT*2][20], int n)
{
    qsort (arr, n, sizeof (arr[0]), myCompare);
}

static void del_client( CA c[MAX_CLIENT], int i, int* trc ){
    p(&mutex);
        if(c[i].socket < 0)
           free(c[i].cliaddr);
        for (int k = i; k < *trc; k++ ){
            c[k] = c[k+1];
        }
        (*trc)--;
    v(&mutex);
    return;
}


// name check 
int TCPNameCheck(char* pch, int connfd){
    int ind = 1;
    p(&mutex);
        for (int i = 0; i < TCPc_num; i++){
            if(strcmp(pch, TCP_c[i].name) == 0){
                printf("CHILD %u: Sent ERROR Already connected\n", (unsigned int)pthread_self());
                fflush(stdout);
                char* tmp1 = "ERROR Already connected\n";
                send (connfd, tmp1, strlen(tmp1) ,0);
                ind = 0;
            }
        }
        for (int i = 0; i < UDPc_num; i++){
            if(strcmp(pch, UDP_c[i].name) == 0){
               v(&mutex);
               del_client(UDP_c, i, &UDPc_num);
               p(&mutex);
            }
        }
    v(&mutex);
    return ind;
}

//========== WHO ===============
void handle_WHO (char tmp3[CHUNK*2]){
    char tmp_name[MAX_CLIENT][20];
    int i;
    strcpy(tmp3, "OK\n");
    //========extrme operation=========
    p(&mutex);
    for(i = 0; i < TCPc_num; i++){
        strcpy(tmp_name[i], TCP_c[i].name);
    }
    for(int k = 0; k < UDPc_num; k++){
        strcpy(tmp_name[i+k], UDP_c[k].name);
    }

    sort(tmp_name, TCPc_num+UDPc_num);
    for (int l = 0; l < TCPc_num+UDPc_num; l++){
        strcat(tmp3, tmp_name[l]);
        strcat(tmp3, "\n");
    }
    v(&mutex);
    //========extrme operation (极限操作)========
}

void handle_WHO_TCP (int connfd){
    char tmp3[CHUNK*2];
    char tmp_name[MAX_CLIENT][20];
    strcpy(tmp3, "OK\n");
    int i;
    // ========extrme operation=========
    p(&mutex);
    for(i = 0; i < TCPc_num; i++){
        strcpy(tmp_name[i], TCP_c[i].name);
    }
    for(int k = 0; k < UDPc_num; k++){
        strcpy(tmp_name[i+k], UDP_c[k].name);
    }

    sort(tmp_name, TCPc_num+UDPc_num);
    for (int l = 0; l < TCPc_num+UDPc_num; l++){
        strcat(tmp3, tmp_name[l]);
        strcat(tmp3, "\n");
    }
    v(&mutex);
    send (connfd, tmp3, strlen(tmp3) ,0);
    
    return;
    //========extrme operation (极限操作)========
}
//==============================


//=========== handle UDP login =====================
int cmpAddr(struct in_addr a, struct in_addr b){
    if ( strcmp (inet_ntoa(a)
         ,inet_ntoa(b) ) == 0 ){
        return 1;
    } 
    return 0;
}
int chkUDPIfLoin( struct in_addr  a ){
    int ind = 0;
    p(&mutex);
    for (int i = 0; i < UDPc_num; i++){
        if (cmpAddr(a, (struct in_addr)UDP_c[i].client)){
            ind = 1;
        }
    }
    v(&mutex);
    return ind;
}

void logout(int loc){
    del_client(TCP_c, loc, &TCPc_num);
}

void UDP_logout(struct in_addr a){
    p(&mutex);
    int loc = -1;
    for (int i = 0; i < UDPc_num; i++ ){
        if (cmpAddr(a, (struct in_addr)UDP_c[i].client)){
            loc = i;
            break;
        } 
    }
    v(&mutex);
        del_client(UDP_c, loc, &UDPc_num);
}

void rm_symbol(char x[MAX_CLIENT][MAX_MSGLEN_REAL],char ori[], int* num, int seg){
    int i = 0; int j = 0; int number = 0;
    while(ori[i] != '\0' && ori[i] != '\n'){
        while(!isalnum(ori[i]) && ori[i] != '\0')i++;
        if(ori[i] == '\0' || ori[i] == '\n')break;
        if(number == seg - 1){
            while( ori[i] != '\0' && ori[i] != '\n' ){
                x[number][j++] = ori[i];
                i++;
            }
            x[number][j] = '\0';
            number++;
            break;
        } else {
            while(isalnum(ori[i])){
                // memset(x[number], MAX_MSGLEN_REAL, CHUNK);
                x[number][j++] = ori[i];
                i++;
            }
            x[number][j] = '\0';
            number++;
            j = 0;
            if(ori[i] == '\0')break;
            i++;
        }

    }
    *num = number;
}

static int UDP_LOGIN(char* userid, struct in_addr addr) {
    // first check tcp 
    int ind1 = 1;
    int ind2 = 4;
    p(&mutex);
    for (int i = 0; i < TCPc_num ; i++){
       if (strcmp(userid, TCP_c[i].name) == 0){
            ind1 = 0;    
       }
    }

    for ( int i = 0; i < UDPc_num; i++){
        if (strcmp(userid, UDP_c[i].name) == 0){
            if ( cmpAddr(UDP_c[i].client, addr) ){
                ind2 = 1; // already login 
            } else {// kick out 
                UDP_c[i].client = addr;
                ind2 = 2;
            }
        } else {
            // replace 
            if (cmpAddr(UDP_c[i].client, addr)){
                strcpy(UDP_c[i].name, userid);
                ind2 = 3;
            }

        }
    }

    v(&mutex);
    if(ind1 == 0) return 0;
    else return ind2;
}
//======================================================

//============ handle send and broadcast ===============

void addend(int flag, char a[], char ag_ary[4][MAX_MSGLEN_REAL], int num, int ind ){
    int k = 0;
    if (flag == 0)
        k = 3;
    else k = 2;
    char max[MAX_MSGLEN];
    while (k < ind && (strlen(max)) < num){
        if (strlen(max) == 0){
            strcpy(max, " ");
        } else {
            strcat(max, " ");
        }
        strcat(max, ag_ary[k]);
        k++;
    }
    strcat(a, max);
    strcat(a, "\n");
}

char* fd_tcpname(int fd){
    p(&mutex);
    for (int i = 0; i < TCPc_num; i++){
        if (TCP_c[i].socket == fd){
            v(&mutex);
            return TCP_c[i].name;
        }
    }
    v(&mutex);
    return NULL;
}

char* fd_udpname( struct sockaddr_in cliaddr ){
    p(&mutex);
    for (int i = 0; i < UDPc_num; i++){
        if( cmpAddr( (UDP_c[i].client), (struct in_addr)cliaddr.sin_addr ) ){
            v(&mutex);
            return UDP_c[i].name;
        }
    }
    v(&mutex);
    return NULL;
}

static int handle_send(char ag_ary[4][MAX_MSGLEN_REAL]
                            , int arg_num, int udpfd, struct sockaddr_in cliaddr 
                            ,socklen_t len, int flag){// flag == 0: TCP
        int msglen = atoi(ag_ary[2]);
        if( msglen < 1 || msglen > MAX_MSGLEN_REAL){
            printf("CHILD %u: Sent ERROR (Invalid msglen)\n", (unsigned int)pthread_self());
            fflush(stdout);
            char* tmp4 = "ERROR Invalid msglen\n";
            if (flag == 1)
                sendto(udpfd,tmp4,strlen(tmp4),0,(SA*)&cliaddr,len);
            else send (udpfd, tmp4, strlen(tmp4), 0);
            return 0; 
        } else if (arg_num < 4) {
            printf("CHILD %u: Sent ERROR (Invalid SEND format)\n", (unsigned int)pthread_self());
            fflush(stdout);
            char* tmp4 = "ERROR Invalid SEND format\n";
            if (flag == 1)
                sendto(udpfd,tmp4,strlen(tmp4),0,(SA*)&cliaddr,len);
                else send (udpfd, tmp4, strlen(tmp4), 0);
            return 0;
        } else {
            // check TCP
            char tmp4[MAX_MSGLEN] = "FROM ";
            if (flag == 0) strcat(tmp4, fd_tcpname(udpfd));
            else strcat(tmp4, fd_udpname(cliaddr));
            strcat(tmp4, " ");
            strcat(tmp4, ag_ary[2]);
            strcat(tmp4, " ");
            // printf("this is ::::::::::::: %c, %lu \n", ag_ary[3][15], sizeof(ag_ary[3]));
            // char test = '\n';
            // printf("%c====\n", test);
            strncat(tmp4, ag_ary[3], atoi(ag_ary[2]));
            p(&mutex);
                for (int i = 0; i < TCPc_num; i++){
                    if (strcmp(TCP_c[i].name, ag_ary[1]) == 0){
                        char tmp10[] = "OK\n";
                        send (udpfd, tmp10, 3 ,0);
                        
                        send (TCP_c[i].socket, tmp4, strlen(tmp4), 0);
                        v(&mutex);
                        return 1;
                    }
                }

                for (int i = 0; i < UDPc_num; i++){
                    if (strcmp(UDP_c[i].name, ag_ary[1]) == 0){
                        char* tmp10 = "OK\n";
                        sendto(udpfd,tmp10,3,0,(SA*)&cliaddr,len);
                        sendto(UDP_c[i].fd_udp,tmp4,strlen(tmp4),0,(SA*)UDP_c[i].cliaddr,len);
                        
                        v(&mutex);
                        return 1;
                    }
                }
            v(&mutex);
        }
    printf("CHILD %u: Sent ERROR (Unknown userid)\n", (unsigned int)pthread_self());
    fflush(stdout);
    char* tmp4 = "ERROR Unknown userid\n";
    if (flag == 1)
        sendto(udpfd,tmp4,strlen(tmp4),0,(SA*)&cliaddr,len);
        else send (udpfd, tmp4, strlen(tmp4), 0);
    return 0;
}

static int handle_broadcast(char ag_ary[4][MAX_MSGLEN_REAL]
                            , int arg_num, int udpfd, struct sockaddr_in cliaddr 
                            ,socklen_t len, int flag){
    if (arg_num < 3) {
        printf("CHILD %u: Sent ERROR (Invalid BROADCAST format)\n", (unsigned int)pthread_self());
        fflush(stdout);
        char* tmp4 = "ERROR Invalid BROADCAST format\n";
        if (flag == 1)
            sendto(udpfd,tmp4,strlen(tmp4),0,(SA*)&cliaddr,len);
            else send (udpfd, tmp4, strlen(tmp4), 0);
        return 0;
    } else {
        int msglen = atoi(ag_ary[1]);
        if( msglen < 1 || msglen > MAX_MSGLEN_REAL){
            printf("CHILD %u: Sent ERROR (Invalid msglen)\n", (unsigned int)pthread_self());
            fflush(stdout);
            char* tmp4 = "ERROR Invalid msglen\n";
            if (flag == 1)
                sendto(udpfd,tmp4,strlen(tmp4),0,(SA*)&cliaddr,len);
            else send (udpfd, tmp4, strlen(tmp4), 0);
            return 0; 
        } else {
            // check TCP
            char tmp4[MAX_MSGLEN] = "FROM ";
            if (flag == 0) strcat(tmp4, fd_tcpname(udpfd));
            else strcat(tmp4, fd_udpname(cliaddr));
            strcat(tmp4, " ");
            strcat(tmp4, ag_ary[1]);
            strcat(tmp4, " ");
            strncat(tmp4, ag_ary[2], atoi(ag_ary[1]));

            p(&mutex);
                for (int i = 0; i < TCPc_num; i++){
                    send (TCP_c[i].socket, tmp4, strlen(tmp4), 0);
                    v(&mutex);
                }

                for (int i = 0; i < UDPc_num; i++){
                    sendto(UDP_c[i].fd_udp,tmp4,strlen(tmp4),0,(SA*)UDP_c[i].cliaddr,len);
                    v(&mutex);
                }
            v(&mutex);
        }
    }
    return 0;
}


void handle_share(char ag_ary[4][MAX_MSGLEN_REAL] 
                , int fd, struct sockaddr_in cliaddr,socklen_t len ) {
    //check userid
    p(&mutex);
        for (int i = 0; i < UDPc_num; i++){
            if ( strcmp(UDP_c[i].name, ag_ary[1] ) ==0  ){
                char* tmp2 = "SHARE not supported because recipient is using UDP\n";
                send (fd, tmp2, strlen(tmp2) ,0);
                v(&mutex);
                return;
            }
        }
        // check TCP
        int tcpfd = -1;
        for (int i = 0; i < TCPc_num; i++){
            if( strcmp(TCP_c[i].name, ag_ary[1]) ==0 ){
                tcpfd = TCP_c[i].socket;
            }
        }
        if (tcpfd < 0) {
            char* tmp2 = "ERROR Unknow userid\n";
            send (fd, tmp2, strlen(tmp2) ,0);
            v(&mutex);
            return;
        }
    v(&mutex);

    // ready to recieve 
    char* tmp1 = "OK\n";
    send (fd, tmp1, 3 ,0);

    // set up 
    

    char tmp2[MAX_MSGLEN];
    strcpy(tmp2, "SHARE ");
    strcat(tmp2, fd_tcpname(fd));
    strcat(tmp2, " ");
    strcat(tmp2, ag_ary[2]);
    send(tcpfd, tmp2, strlen(tmp2), 0);

    int size = 0;
    char datachunk[CHUNK];
    while(1){
        size = recv(fd, datachunk, CHUNK, 0);
        send (fd, tmp1, strlen(tmp1) ,0);
        if(size == CHUNK){
            
            send(tcpfd, datachunk, CHUNK, 0);

        } else { // size < CHUNK
            send(tcpfd, datachunk, size, 0);
            break;
        }
    }

}













#endif
