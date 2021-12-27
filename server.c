/* System Programming Team 2
 * OS : Ubuntu 18.04
 * complier : gcc 7.5.0
 * file : server.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100
#define MAX_CLNT 256
#define ROOMCOUNT 10
#define PORTNUMBER "9000"

typedef struct {
    char roomName[20]; //chatroom name
    int cnt; //people count
    int roomnum; //chatroom portnum
    int option; //to check enter or make room
    int clnt_socks[ROOMCOUNT];
    char clnt_names[ROOMCOUNT][20];
}roominfo;

void init(int, char**);
void socket_init(int*);
void chat_start(int*);
void * handle_clnt(void * arg);
void send_msg(char * msg, int len, int);
void error_handling(char * msg);
int command_detection(char*);
int quit_detection(char*);
void option1(int, roominfo, int);
int option2(int, roominfo, int);
void delete(int);


int port;
int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
char room_name[10][10];
char room_member[10][10][10];
int room_mem[10] = {0, };
int room_num=0;
pthread_mutex_t mutx;

int roomnum = 0;
roominfo info[ROOMCOUNT];


int main(int argc, char *argv[])
{
	int serv_sock;
	init(argc, argv);
	socket_init(&serv_sock);	
	chat_start(&serv_sock);	
	close(serv_sock);
	return 0;
}



///////////////////////////////////////////////////////////

void init(int argc, char** argv){
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	port = atoi(argv[1]);
}
void socket_init(int* serv_sock){
	struct sockaddr_in serv_adr;
	*serv_sock=socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(port);
	if(bind(*serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(*serv_sock, 5)==-1)
		error_handling("listen() error");
}
void chat_start(int* serv_sock){
	int clnt_sock;
	struct sockaddr_in clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;
	pthread_mutex_init(&mutx, NULL);
	while(1)
	{
		clnt_adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(*serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);
		
		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++]=clnt_sock;
		pthread_mutex_unlock(&mutx);
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
		printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
	}
}

void * handle_clnt(void * arg)
{
	roominfo presentinfo;
	int clnt_sock=*((int*)arg);
	int str_len=0, i, j;
	int fd_num;
	struct timeval timeout;
	char msg[BUF_SIZE];
	fd_set fd2;

	timeout.tv_sec = 3;
	timeout.tv_usec = 3000;

	FD_ZERO(&fd2);
	FD_SET(clnt_sock, &fd2);

	if((fd_num = select(clnt_sock+1, 0, &fd2, 0, &timeout)) > 0){
		pthread_mutex_lock(&mutx);
		for(int i=0; i<10; i++){
			write(clnt_sock, (void*)&info[i], sizeof(info[i]));
		}
		pthread_mutex_unlock(&mutx);
		str_len = read(clnt_sock, (void*)&presentinfo, BUF_SIZE);

		if(str_len == 0){			
			printf("close client : %d", clnt_sock);
		}
		
		if((str_len >0) && presentinfo.option == 1){
			//for debugging
			printf("for test sendinfo success\n");		             		
			option1(clnt_sock, presentinfo, str_len);	
		}
		else if((str_len >0) &&(presentinfo.option == 2)){
			//for debugging
			printf("for test checkinfo success\n");		             		
			if(option2(clnt_sock, presentinfo, str_len) > 0){
				printf("room name search success\n");
			}
			else{
				printf("room name search failure\n");
			}
			
		}  
		printf("\n");
	}

	str_len=read(clnt_sock, msg, sizeof(msg));
	while((str_len=read(clnt_sock, msg, sizeof(msg)))!=0){
		if ( quit_detection(msg) == 1){
			printf("scuas\n");
			send_msg(msg, str_len, clnt_sock);
			delete(clnt_sock);
			close(clnt_sock);
			return NULL;
		}
		else{
			send_msg(msg, str_len, clnt_sock);
		}
	}
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)   // remove disconnected client
	{
		if(clnt_sock==clnt_socks[i])
		{
			while(i < clnt_cnt){
				clnt_socks[i]=clnt_socks[i+1];
				i++;
			}	
			break;
			
		}
	}
	printf("disconnect\n");
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
	return NULL;
}

void delete(int clnt_sock){
	pthread_mutex_lock(&mutx);
	for(int i=0; i<clnt_cnt; i++)   // remove disconnected client
	{
		if(clnt_sock==clnt_socks[i])
		{
			while(i < clnt_cnt){
				clnt_socks[i]=clnt_socks[i+1];
				i++;
			}
			break;
		}
	}
	clnt_cnt--;
	for(int j=0; j<roomnum; j++){
		for(int i=0; i<info[j].cnt; i++)   // remove disconnected client
		{
			if(clnt_sock==info[j].clnt_socks[i])
			{
				while(i < info[j].cnt){
					info[j].clnt_socks[i]=info[j].clnt_socks[i+1];
					i++;
				}
				info[j].cnt--;
				break;
			}
		}
	}

	pthread_mutex_unlock(&mutx);
}


int quit_detection(char* msg){
	char* quitMsg = "is quit";
	int k=0, j=0;
	if (strstr(msg, "]")){
		while(msg[k] != ']') k++;
		k+=2;
		while(msg[k] == quitMsg[j]){
			k++; j++;
		}
		if (j==6){
			return 1;
		}
		else{
			return 0;
		}
	}
	else return 0;
}


int command_detection(char* msg){
//	char* command = "notice ";
	int i=0, j=0;
	while(msg[i] != ']') i++;
	i += 2;
/*	while(msg[i] == command[j]){
		i++; j++;
	}
	if ( j==7){
		return 1;
	}
	else{
		return 0;
	}
*/
	return msg[i] - 48;
}

void option1(int clnt_sock, roominfo rinfo, int len){
	int i;
	int send_roomnum;
	pthread_mutex_lock(&mutx);       

	//count up roomnum
	send_roomnum = roomnum;
	rinfo.roomnum = send_roomnum;
	rinfo.cnt = 1;
	rinfo.clnt_socks[rinfo.cnt-1] = clnt_sock;
	info[roomnum] = rinfo;
	write(clnt_sock, (void*)&rinfo, sizeof(rinfo));
    
	roomnum++;
	pthread_mutex_unlock(&mutx);
}

int option2(int clnt_sock ,roominfo rinfo, int len){

	int i;
	for(i = 0; i< ROOMCOUNT; i++){
		//have to check this can be error
		if(strcmp(info[i].roomName, rinfo.roomName) == 0){				     rinfo.roomnum = i;
			info[i].cnt++;
			rinfo.cnt = info[i].cnt;
			info[i].clnt_socks[info[i].cnt-1] = clnt_sock;
			strcpy(info[i].clnt_names[info[i].cnt-1], rinfo.clnt_names[0]);
			rinfo.clnt_socks[rinfo.cnt-1] = clnt_sock;
			write(clnt_sock, (void*)&rinfo, sizeof(rinfo));
			return 1;
		}
	}
	rinfo.roomnum = -1;
	write(clnt_sock, (void*)&rinfo, sizeof(rinfo));
	return -1;
}

void send_msg(char * msg, int len, int clnt_sock)   // send to all
{
	int i, j;
	int myRoomNum = -1;
	pthread_mutex_lock(&mutx);
	for(i=0; i<roomnum; i++){
		for(j=0; j<info[i].cnt; j++){
			if(clnt_sock == info[i].clnt_socks[j]){
				myRoomNum = info[i].roomnum;
			}
		}
	}

	for(i=0; i<info[myRoomNum].cnt; i++)
		write(info[myRoomNum].clnt_socks[i], msg, len);
	pthread_mutex_unlock(&mutx);
}
void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
