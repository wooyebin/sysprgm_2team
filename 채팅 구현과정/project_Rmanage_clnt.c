#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/wait.h>

	
#define BUF_SIZE 400
#define NAME_SIZE 20
#define PORTNUMBER "9000"
#define CHATCLNTPORT "9005"

typedef struct {
    char roomName[20]; //chatroom name
    int cnt; //people count
    int roomnum; //chatroom portnum
    int option; //to check enter or make room
}roominfo;

void logInPage(int sock, int chatclnt_sock);
void showmenu(int sock, int chatclnt_sock);
void help();
void error_handling(char * output);
void createRoom(int sock, int chatclnt_sock, roominfo rinfo);    
void enterRoom(int sock, int chatclnt_sock);   
void chatting(int chatclnt_sock,roominfo rinfo);

char output[BUF_SIZE];
char msg[BUF_SIZE];
char name[NAME_SIZE];
roominfo info;
static sem_t sem_one;
static sem_t sem_two;
	
int main()
{
	int sock;
    int chatclnt_sock;

	int chatclnt_adr_sz;
	struct sockaddr_in serv_addr, chatclnt_adr;
	// serv _addr -> to connect Rmanage_serv
    // chatclnt_adr -> to connect chat_serv
	
	sock = socket(PF_INET, SOCK_STREAM, 0);
	chatclnt_sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	serv_addr.sin_port=htons(atoi(PORTNUMBER));    
	
	
    memset(&chatclnt_adr, 0, sizeof(chatclnt_adr));
	chatclnt_adr.sin_family = AF_INET; 
	serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	chatclnt_adr.sin_port = htons(atoi(CHATCLNTPORT));
	
    printf("\nRmanage_client start\n");   
    

	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");
	
    logInPage(sock, chatclnt_sock);
    

    //close(chatclnt_sock);
	close(sock);  
	return 0;
}

void error_handling(char *output)
{
	fputs(output, stderr);
	fputc('\n', stderr);
	exit(1);
}

void logInPage(int sock, int chatclnt_sock){

    //enter NickName    
    printf("Show your NickName : ");
    fgets(name, BUF_SIZE, stdin);

    showmenu(sock, chatclnt_sock);
}

void showmenu(int sock, int chatclnt_sock){
    int menu = 0;
    printf("1. Create Room\n");
    printf("2. Enter Room\n");
    printf("3. Exit\n");
    printf("4. Help\n\n");

    printf("Input Menu : ");
    scanf("%d", &menu);
    if((menu < 1) || (menu > 4)){
        printf("Not correct Number please try again\n");
        printf("Input Menu : ");
        scanf("%d", &menu);
    }
    
    else if(menu == 1){
        createRoom(sock, chatclnt_sock, info);
    }
    else if(menu == 2){
        enterRoom(sock, chatclnt_sock);
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

void createRoom(int sock, int chatclnt_sock,roominfo rinfo){    
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
    printf("recv roomnum : %d", rinfo.roomnum);        
    // because it is tcp, we have two option 1,2
    close(sock);    
    

    //start chat
    //chatting(chatclnt_sock, rinfo);
}

void enterRoom(int sock, int chatclnt_sock){    
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
    }
    else{
        printf("write to chatclnt roomnum : %d ", sendinfo.roomnum);     
    }
    
    close(sock);
    //start chat

    //chatting(chatclnt_sock, sendinfo);
}

/*
void chatting(int chatclnt_sock,roominfo rinfo){

    if(connect(chatclnt_sock, (struct sockaddr*)&chatclnt_adr, sizeof(chatclnt_adr))==-1)
	    error_handling("connect() error");
    
    int str_len;
    pthread_t snd_thread, rcv_thread;
	void * thread_return;
    sem_init(&sem_one, 0, 0);
	sem_init(&sem_two, 0, 1);    

    printf("\nstart chatting welcome %s\n\n", name);    
    printf("say hello to new participator %s!!\n", name);        

    pthread_create(&snd_thread, NULL, send_msg, (void*)&chatclnt_sock);
    pthread_create(&rcv_thread, NULL, recv_msg, (void*)&chatclnt_sock);

    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);

    close(chatclnt_sock);  
    sem_destroy(&sem_one);
    sem_destroy(&sem_two);
    

    
}

void * send_msg(void * arg){
	
	int sock=*((int*)arg);
	char name_msg[NAME_SIZE+BUF_SIZE];

	while(1) 
	{
		sem_wait(&sem_two);
		fgets(msg, BUF_SIZE, stdin);
		if(!strcmp(msg,"q\n")||!strcmp(msg,"Q\n")) 
		{
			close(sock);
			exit(0);
		}
		sprintf(name_msg,"%s %s", name, msg);
		write(sock, name_msg, strlen(name_msg));
		sem_post(&sem_one);
	}
	return NULL;
}
	
void * recv_msg(void * arg){
	int sock=*((int*)arg);
	char name_msg[NAME_SIZE+BUF_SIZE];
	int str_len;
	while(1)
	{
		sem_wait(&sem_one);
		str_len=read(sock, name_msg, NAME_SIZE+BUF_SIZE-1);
		if(str_len==-1) 
			return (void*)-1;
		name_msg[str_len]=0;
		fputs(name_msg, stdout);
		sem_post(&sem_two);
		
	}
	return NULL;
}

*/
