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
#include <semaphore.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

typedef struct {
    char roomName[20]; //chatroom name
    int cnt; //people count
    int roomnum; //chatroom portnum
    int option; //to check enter or make room
    int clnt_socks[10];
}roominfo;


void init(int, char**);
void socket_init(int*);
void logInPage(int sock);
void showmenu(int sock);
void chat_start(int*);
void * send_msg(void * arg);
void * recv_msg(void * arg);
void createRoom(int sock, roominfo rinfo);    
void enterRoom(int sock);   
void chatting(roominfo rinfo);
void help();
void error_handling(char * msg);

int checkmsg(char*);
void make_msg(char*, char*);
void color(char *msg);

char IP[20];
int port;
char output[BUF_SIZE];
char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];
int room=0;
roominfo info;
static sem_t sem_one;
static sem_t sem_two;

int emojiCount = 1;
char emoji[1][100];


//�ڸ� ��� ���õ� ����
char disturb[300][300]={"\0"};
int i=0;
int afk_mode =0;

//���� ����
char noticebuffer[300];

int main(int argc, char *argv[])
{
	int sock;
	strcpy(emoji[0], "^^\n");
	init(argc, argv);
	socket_init(&sock);
	logInPage(sock);
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
void logInPage(int sock){

    //enter NickName    
    //printf("Show your NickName : ");
    //fgets(name, BUF_SIZE, stdin);

    showmenu(sock);
}
void showmenu(int sock){
    int menu = 0;
    printf("1. Create Room\n");
    printf("2. Enter Room\n");
    printf("3. Exit\n");
    printf("4. Help\n\n");

    printf("Input Menu : ");
    scanf("%d", &menu);
    while((menu < 1) || (menu > 4)){
        printf("Not correct Number please try again\n");
        printf("Input Menu : ");
        scanf("%d", &menu);
    }
    
    if(menu == 1){
        createRoom(sock, info);
    }
    else if(menu == 2){
        enterRoom(sock);
    }
    else if(menu == 3){
        printf("terminate chatting room program\n");
        exit(1);
    }
    else if(menu == 4){
        help();
    }
    else {
        printf("error in menu \n");
    }

}
void help(){
    FILE *fp;
    char buf[BUF_SIZE];
    fp = fopen("README.txt","r");
    
    while(fgets(buf, sizeof(buf), fp) != NULL){
        printf("%s", buf);
        memset(buf, 0, sizeof(buf));
    }
    fclose(fp);

}

void createRoom(int sock, roominfo rinfo){    
    int status;
    int str_len = 0;
    int pid;   
    char roomname[NAME_SIZE];
    
    //enter roomname
    //child process, receive portnum from Rmanage_serv.c
    
    //input roomName
    printf("input your roomName to create: ");
    scanf("%s", roomname);
    //fgets(roomname, NAME_SIZE, stdin);        
    
    strcpy(rinfo.roomName, roomname);

    //input option
    rinfo.option = 1;

    //send to rmanage server
    write(sock, (void*)&rinfo, sizeof(info));

    //for receive rmanage server           
    str_len = read(sock, (void*)&rinfo, sizeof(info));
    if(str_len == -1){
        error_handling("roominfo error");
    }

    // for testing
    printf("recv roomnum : %d\n", rinfo.roomnum);        
    // because it is tcp, we have two option 1,2
    

    //start chat
    //chatting(chatclnt_sock, rinfo);
}
void enterRoom(int sock){    
    char enterRoomName[NAME_SIZE];   
    int str_len;
    roominfo sendinfo;

    //input roomName
    printf("input your roomName to enter : ");

    //fgets(enterRoomName, NAME_SIZE, stdin);          
    scanf("%s", enterRoomName);
    strcpy(sendinfo.roomName, enterRoomName);
    
    sendinfo.option = 2;    

    write(sock, (void*) &sendinfo, sizeof(sendinfo));

    printf("\n loading......\n");
    sleep(1);

    str_len = read(sock, (void*)&sendinfo, sizeof(sendinfo));
    if(str_len==-1){
        error_handling("roominfo error");
    }
    
    if(sendinfo.roomnum == -1){
        printf("this chatting room name is not in room information.\n"); 
	error_handling("not chatting room name");
    }
    else{
        printf("write to chatclnt roomnum : %d \n", sendinfo.roomnum);     
    }
    
    //close(sock);
    //start chat

    //chatting(chatclnt_sock, sendinfo);
}



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
            	// color(noticebuffer);
		fputs(noticebuffer,stdout);
		return 2;
	}
	else if ( !strcmp(msg, "emoji\n") ){
		for ( int i=0; i<emojiCount; i++){
			printf("%d : %s\n", i, emoji[i]);
		}
		return 3;
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
		//notice ġ�� �������� ����
		//�ڸ� ����� ���� �Լ��̴�.
		if(msgcheckNum == 1 || msgcheckNum == 2){ // afk
			continue;
		}
		if(msgcheckNum == -1){
			close(sock);
			exit(0);
		}
		if(msgcheckNum == 3){
			int emojiNum = 0;
			char temp;
			scanf("%d", &emojiNum);
			scanf("%c", &temp);
			strcpy(msg, emoji[emojiNum]);
		}
		make_msg(msg, name_msg);
		write(sock, name_msg, strlen(name_msg));		
	}
	return NULL;
}

void make_msg(char* msg, char* name_msg){
	//notice�� ���� �̸� ���� ������ �Է½�Ű�� ��
	if(strstr(msg,"notice"))
	{
		sprintf(name_msg,"%s",msg);	
	}
	//�� �ܴ� �� �Է½�Ŵ
	else{
		sprintf(name_msg,"%s %s", name, msg);
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
			
		//if(name_msg[0]-48 == room){
			if(strstr(name_msg,"notice"))
			{
				strcpy(noticebuffer,name_msg);	
			}
			//afk_mode�� 1�̶� �ڸ���� �迭�� �޽��� �°� ������
			if(afk_mode ==1)
			{
			  strcpy(disturb[i],name_msg);
			  i++;
			}
			else{
				fputs(name_msg, stdout);
			}
		//}
	}
	return NULL;
}
	
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

/*
void color(char *msg){
    initscr(); //curses ����
    start_color(); // ������ curses��� ����
    init_pair(1, COLOR_GREEN,COLOR_BLACK);//��������, ���� ����
    attron(COLOR_PAIR(1));//1�� ���� ����
    printw("%s", msg);
    attroff(COLOR_PAIR(1));
    getch(); //���� ���� Ȯ��
    refresh(); //�� �ڵ��� ���� ���
    endwin(); //curses ����
//    clear(); ������ ������ �����ִ� ����
}
*/
