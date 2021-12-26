#include <ncurses.h>
#include <curses.h>
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
void chat_start(int*);
void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * msg);
void color(char * msg);

char IP[20];
int port;
char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];

//�ڸ� ��� ���õ� ����`
char disturb[300][300]={"\0"};
int i=0;
int afk_mode =0;

//���� ����
char noticebuffer[300];


int main(int argc, char *argv[])
{
    
    int sock;
    init(argc, argv);
    socket_init(&sock);
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
//�ʱ�ȭ


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
    else{return 0;}
}

void * send_msg(void * arg)   // send thread main
{
    int sock=*((int*)arg);
    char name_msg[NAME_SIZE+BUF_SIZE];
    while(1)
    {
        fgets(msg, BUF_SIZE, stdin);
        
        //notice ġ�� �������� ����
        if(!strcmp(msg,"notice\n"))
        {
//            fputs(noticebuffer,stdout);

            color(noticebuffer);
            continue;

        }

        //�ڸ� ����� ���� �Լ��̴�.
        if (msgcheck(msg) == 1)
        {
            continue;
        }

        if(!strcmp(msg,"q\n")||!strcmp(msg,"Q\n"))
        {
            close(sock);
            exit(0);
        }

        //notice�� ���� �̸� ���� ������ �Է½�Ű�� ��
        if(strstr(msg,"notice"))
        {
            sprintf(name_msg,"%s",msg);
        }
        //�� �ܴ� �� �Է½�Ŵ
        else{
            sprintf(name_msg,"%s %s", name, msg);
        }
        write(sock, name_msg, strlen(name_msg));
    }
    return NULL;
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
        
        if(strstr(name_msg,"notice"))
        {
            strcpy(noticebuffer,name_msg);
        }
        //afk_mode�� 1�̶� �ڸ���� �迭�� �޽��� �°� ������
        else
        {
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