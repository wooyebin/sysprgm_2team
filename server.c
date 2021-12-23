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

void init(int, char**);
void socket_init(int*);
void chat_start(int*);
void * handle_clnt(void * arg);
void send_msg(char * msg, int len);
void error_handling(char * msg);
int command_detection(char*);

int port;
int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
char room_name[10][10];
char room_member[10][10][10];
int room_mem[10] = {0, };
int room_num=0;
pthread_mutex_t mutx;

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
	int clnt_sock=*((int*)arg);
	int str_len=0, i, j;
	char msg[BUF_SIZE];
	str_len=read(clnt_sock, msg, sizeof(msg));
	int commandNum = command_detection(msg);
	if(commandNum == 1){
		str_len=read(clnt_sock, msg, sizeof(msg));
		str_len=read(clnt_sock, msg, sizeof(msg));
		char *makingRoomName =strtok(msg, " ");
		strcpy(room_member[room_num][room_mem[room_num]],makingRoomName);
		makingRoomName = strtok(NULL, " ");
		strcpy(room_name[room_num], makingRoomName);
		room_mem[room_num] ++;
		room_num++;
	}
	else if(commandNum == 2){
		char roomlist[4096];
		char temp[1024];
		strcpy(roomlist, "num | room name(mem num) | room members\n");
		for(i=0; i<room_num; i++){
			sprintf(temp, "%d) %s(%d) ", i, room_name[i], room_mem[i]);
			strcat(roomlist, temp);
			for(j=0; j<room_mem[i]; j++){
				sprintf(temp, "%s ", room_member[i][j]);
				strcat(roomlist, temp);
			}
			strcat(roomlist, "\n");
		}
		send_msg(roomlist, strlen(roomlist));
		str_len=read(clnt_sock, msg, sizeof(msg));
		int room = command_detection(msg);
		strcpy(room_member[room][room_mem[room]], strtok(msg," "));
		room_mem[room] ++;
		str_len=read(clnt_sock, msg, sizeof(msg));
	}
	//else
	while((str_len=read(clnt_sock, msg, sizeof(msg)))!=0){
		send_msg(msg, str_len);
	}
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)   // remove disconnected client
	{
		if(clnt_sock==clnt_socks[i])
		{
			while(i < clnt_cnt)
				clnt_socks[i]=clnt_socks[i+1];
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
	return NULL;
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


void send_msg(char * msg, int len)   // send to all
{
	int i;
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)
		write(clnt_socks[i], msg, len);
	pthread_mutex_unlock(&mutx);
}
void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
