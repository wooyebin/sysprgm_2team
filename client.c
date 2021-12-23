/* System Programming Team 2
 * OS : Ubuntu 18.04
 * complier : gcc 7.5.0
 * file : client.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
	
#define BUF_SIZE 100
#define NAME_SIZE 20

void init(int, char**);
void socket_init(int*);
void show_menu(char*, int);
void chat_start(int*);
void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * msg);

int checkmsg(char*);
void make_msg(char*, char*);

char IP[20];
int port;
char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];
int room=0;

//자리 비움 관련된 것임
char disturb[300][300]={"\0"};
int i=0;
int afk_mode =0;

//공지 사항
char noticebuffer[300];


int main(int argc, char *argv[])
{
	int sock;
	init(argc, argv);
	socket_init(&sock);
	show_menu(name, sock);
	chat_start(&sock);
	close(sock);  
	return 0;
}



////////////////////////////////////////////////////////////////////

void init(int argc, char** argv){
	if(argc!=4) {
		printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		exit(1);
	 }
	sprintf(name, "[%s]", argv[3]);
	strcpy(IP, argv[1]);
	port = atoi(argv[2]);
}

void socket_init(int* sock){
	struct sockaddr_in serv_addr;
	*sock=socket(PF_INET, SOCK_STREAM, 0);
	if(*sock == -1)
		error_handling("socket");
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(IP);
	serv_addr.sin_port=htons(port);
	  
	if(connect(*sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");
}

void show_menu(char* name, int sock){
	int menu, str_len;
	char makingRoomName[10];
	char name_msg[NAME_SIZE+5];
	printf("1. GENERATE chatting room.\n");
	printf("2. ENTER the chatting room.\n");
	printf("3. Quit.\n");
	printf("4. help\n");
	scanf("%d", &menu);
	sprintf(name_msg, "%s %d", name, menu);
	write(sock, name_msg, strlen(name_msg));
	if ( menu == 1 ){
		fgets(makingRoomName, 10, stdin);
		printf("%s\n", makingRoomName);
		write(sock, name_msg, strlen(name_msg));
	}
	else if ( menu == 2){
		str_len=read(sock, name_msg, NAME_SIZE+BUF_SIZE-1);
		printf("%s", name_msg);
		printf("\nwhat\n");
		scanf("%d", &room);
		sprintf(name_msg, "%s %d", name, room);
		write(sock, name_msg, strlen(name_msg));
	}
}

//초기화


void chat_start(int* sock){
	pthread_t snd_thread, rcv_thread;
	void * thread_return;
	
	pthread_create(&snd_thread, NULL, send_msg, (void*)sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)sock);

	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
}

int msgcheck(char* msg)
{
	if (!strcmp(msg, "afk\n"))
	{
		afk_mode = 1;
		return 1;
	}
	else if (!strcmp(msg, "nafk\n"))
	{
		for (int j = 0;j<i;j++)
		{
			fputs(disturb[j], stdout);
		}
		i = 0;
		afk_mode = 0;
		return 1;
	}
	else if ( !strcmp(msg,"notice\n") ){
		fputs(noticebuffer,stdout);
		return 2;
	}
	else if ( !strcmp(msg,"q\n")||!strcmp(msg,"Q\n") ){
		return -1;
	}

	else{return 0;}
}

void * send_msg(void * arg)   // send thread main
{
	int sock=*((int*)arg);
	char name_msg[NAME_SIZE+BUF_SIZE];
	while(1) 
	{
		fgets(msg, BUF_SIZE, stdin);
		int msgcheckNum = msgcheck(msg);
		//notice 치면 공지사항 나옴
		//자리 비움을 위한 함수이다.
		if(msgcheckNum == 2 || msgcheckNum == 1){
			continue;
		}
		if(msgcheckNum == -1){
			close(sock);
			exit(0);
		}
		make_msg(msg, name_msg);
		write(sock, name_msg, strlen(name_msg));		
	}
	return NULL;
}

void make_msg(char* msg, char* name_msg){
	//notice가 들어가면 이름 빼고 서버로 입력시키는 것
	if(strstr(msg,"notice"))
	{
		sprintf(name_msg,"%s",msg);	
	}
	//그 외는 다 입력시킴
	else{
		sprintf(name_msg,"%d%s %s", room, name, msg);
	}
}


void * recv_msg(void * arg)   // read thread main
{
	int sock=*((int*)arg);
	char name_msg[NAME_SIZE+BUF_SIZE];
	int str_len;
	while(1)
	{
		str_len=read(sock, name_msg, NAME_SIZE+BUF_SIZE-1);
		if(str_len==-1) 
			return (void*)-1;
		name_msg[str_len]=0;
		if(name_msg[0]-48 == room){
			if(strstr(name_msg,"notice"))
			{
				strcpy(noticebuffer,name_msg);	
			}
			//afk_mode가 1이라서 자리비움 배열에 메시지 온것 저장함
			if(afk_mode ==1)
			{
			  strcpy(disturb[i],name_msg);
			  i++;
			}
			else{
				fputs(name_msg, stdout);
			}
		}
	}
	return NULL;
}
	
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
