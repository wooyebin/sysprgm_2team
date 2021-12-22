#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
	
#define BUF_SIZE 400
#define NAME_SIZE 20

typedef struct {
    char roomName[20]; //chatroom name
    int cnt; //people count
    int roomnum; //chatroom portnum
}roominfo;

void logInPage();
void showmenu(char name[]);
void help();
void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * output);
void createRoom(int sock, int chatclnt_sock,roominfo rinfo,char name[]);    
void enterRoom(int sock, char name[]);   

char name[NAME_SIZE]="[DEFAULT]";
char output[BUF_SIZE];
roominfo info;
	
int main()
{
	int sock;
    int chatclnt_sock;

	int clnt_adr_sz;
	struct sockaddr_in serv_addr, chatclnt_addr;
	
	
	sock = socket(PF_INET, SOCK_STREAM, 0);
	chatclnt_sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));    
	
	
	memset(&chatclnt_adr, 0, sizeof(chatclnt_adr));
	chatclnt_adr.sin_family = AF_INET; 
	chatclnt_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	chatclnt_adr.sin_port = htons(atoi(argv[1]));
	
	if(bind(chatclnt_sock, (struct sockaddr*) &chatserv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");

	if(listen(chatclnt_sock, 5)==-1)
		error_handling("listen() error");	
	  
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");
	
    logInPage();
    

    close(chatclnt_sock);
	close(sock);  
	return 0;
}
/*	
void * send_msg(void * arg)   // send thread main
{
	int sock=*((int*)arg);
	char name_output[NAME_SIZE+BUF_SIZE];
	while(1) 
	{
        logInPage();
           
		fgets(output, BUF_SIZE, stdin);
		if(!strcmp(output,"q\n")||!strcmp(output,"Q\n")) 
		{
			close(sock);
			exit(0);
		}
		sprintf(name_output,"%s %s", name, output);
		write(sock, name_output, strlen(name_output));
	}
	return NULL;
}
	
void * recv_msg(void * arg)   // read thread main
{
	int sock=*((int*)arg);
	char name_output[NAME_SIZE+BUF_SIZE];
	int str_len;
	while(1)
	{
		str_len=read(sock, name_output, NAME_SIZE+BUF_SIZE-1);
		if(str_len==-1) 
			return (void*)-1;
		name_output[str_len]=0;
		fputs(name_output, stdout);
	}
	return NULL;
}
*/	
void error_handling(char *output)
{
	fputs(output, stderr);
	fputc('\n', stderr);
	exit(1);
}

void logInPage(){
    //enter NickName
    char name[NAME_SIZE];
    printf("Show your NickName : ");
    fgets(name, BUF_SIZE, stdin);

    showmenu(name);
}

void showmenu(char name[]){
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
        createRoom(name);
    }
    else if(menu == 2){
        enterRoom(name);
    }
    else if(menu == 3){
        printf("terminate chatting room program\n");
        exit(1);
    }
    else (menu == 4){
        Help();
    }

}

void help(){
    FILE *fp;
    char buf[BUF_SIZE];
    fp = fopen("README.txt","r");
    
    while(fgets(buf, sizeof(buf), fp) != NULL){
        printf("%s", buf);
        memset(buf,0,sizeof(buf));
    }
    fclose(fp);

}

void createRoom(int sock, int chatclnt_sock,roominfo rinfo,char name[]){    
    int str_len;
    int pid;   
    char roomname[NAME_SIZE];
    pid = fork();

    if((pid = fork())<0){
        printf("error fork\n");
        exit(0);
    }
    //enter roomname
    //child process, receive portnum from Rmanage_serv.c
    else if(pid == 0){          
        //input roomName
        printf("input your roonName : ");
        fgets(roomname, NAME_SIZE, stdin);        
        strcpy(rinfo.roomName, roomname);
        //send to rmanage server
        write(sock, (void*) &rinfo, sizeof(info));

        //for receive rmanage server           
        str_len = read(sock, (void*)&rinfo, sizeof(info));
        if(str_len==-1){
            error_handling("roominfo error");
        }
        // for testing
        printf("recv roomnum : %d", rinfo.roomnum);        
    }
    //parent process, send roominfo to chatserv.c  
    else{              
        int status;

        wait(&status);        
        if(WIFEXITED(status)){                     
            
            write(chatclnt_sock, (void*) &rinfo, sizeof(info));             
            // for testing
            printf("write to chatclnt roomnum : %d ", rinfo.roomnum); 

            //enter to room which we create 

        }
    }
}
void enterRoom(int sock, char name[]){    
    char enterRoomName[NAME_SIZE];   

    //input roomName
    printf("input your roonName : ");
    fgets(enterRoomName, NAME_SIZE, stdin);        
    
    strcpy(enterRoomName, name);

    //enter to room by using strcmp()
    //so we modify Rmanage serv.c

}