#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
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
}roominfo;
void * handle_clnt(void * arg);
void send_msg(int clnt_sock, roominfo rinfo,int len);
int check_msg(int clnt_sock, roominfo rinfo, int len);
void error_handling(char * msg);

int roomnum = 0;
roominfo info[ROOMCOUNT];
pthread_mutex_t mutx;

int main()
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;	
  
	pthread_mutex_init(&mutx, NULL);
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(PORTNUMBER));
	
	printf("\n Rmanage_server start \n");

	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");

	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	clnt_adr_sz = sizeof(clnt_adr);

	while(1)
	{		
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);			
	
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
		printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
	}

	close(serv_sock);
	return 0;
}
	
void * handle_clnt(void * arg)
{
	roominfo presentinfo;
	int clnt_sock = *((int*)arg);
	int str_len = 0, i;
	int fd_num; //file descripter
	struct timeval timeout;
	char msg[BUF_SIZE];
	fd_set fd2;
	
	timeout.tv_sec = 3;
    timeout.tv_usec = 3000;

	FD_ZERO(&fd2);
    FD_SET(clnt_sock, &fd2);                                             	


	if((fd_num = select(clnt_sock+1, 0, &fd2, 0, &timeout)) > 0){
		str_len = read(clnt_sock, (void*)&presentinfo, BUF_SIZE);

		if(str_len == 0){			
			printf("close client : %d", clnt_sock);
		}
		
		if((str_len >0) && presentinfo.option == 1){
			//for debugging
			printf("for test sendinfo success\n");		             		
			send_msg(clnt_sock, presentinfo, str_len);	
		}
		else if((str_len >0) &&(presentinfo.option == 2)){
			//for debugging
			printf("for test checkinfo success\n");		             		
			if(check_msg(clnt_sock, presentinfo, str_len) > 0){
				printf("room name search success\n");
			}
			else{
				printf("room name search failure\n");
			}
			
		}  
		printf("\n");
	}	
		
	
	//close(clnt_sock);
	return NULL;
}
void send_msg(int clnt_sock,roominfo rinfo, int len)   // send to all
{
	int i;
	int send_roomnum;
	pthread_mutex_lock(&mutx);       

	//count up roomnum
	send_roomnum = roomnum;
	rinfo.roomnum = send_roomnum;
	rinfo.cnt = 1;
	info[roomnum] = rinfo;
	write(clnt_sock, (void*)&rinfo, sizeof(rinfo));
    
	roomnum++;
	pthread_mutex_unlock(&mutx);


}
int check_msg(int clnt_sock ,roominfo rinfo, int len){
	int i;
	for(i = 0; i< ROOMCOUNT; i++){
		//have to check this can be error
		if(strcmp(info[i].roomName, rinfo.roomName) == 0){			
			rinfo.roomnum = i;
			info[i].cnt++;
			rinfo.cnt = info[i].cnt;
			write(clnt_sock, (void*)&rinfo, sizeof(rinfo));
			return 1;
		}
	}
	rinfo.roomnum = -1;
	write(clnt_sock, (void*)&rinfo, sizeof(rinfo));
	return -1;
}
void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
