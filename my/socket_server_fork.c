#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT 5600
#define BUF_LEN 1024
int main(int argc, char* argv[])
{
    char buf[BUF_LEN];
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int ret=0, count;
    socklen_t len;
    int serv_fd;
    int cli_fd;

    /************ additional item **********/
    pid_t childpid;

    /***************** socket ***********************/
    if((serv_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("server socket error\n");
        return -1;
    }

    /******************** bind ************************/
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if(bind(serv_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("server bind error\n");
        return -1;
    }

    /******************** listen **********************/
    if(listen(serv_fd,10) < 0)
    {
        printf("sercer listen error\n");
        return -1;
    }

    while(1)
    {
        /*************** accept ******************/
        len = sizeof(client_addr);
        cli_fd = accept(serv_fd, (struct sockaddr*)&client_addr, &len);
        if(cli_fd < 0)
        {
            printf("accept fail\n");
            return -1;
        }

        /*********** additional item ************/
        if((childpid = fork()) == 0) /* child process */
        {
            if((close(serv_fd))<0)
            {
                printf("close socket error\n");
                return -1;
            }
            while(1)
            {
                if((ret = read(cli_fd, buf, 1024)) < 0)
                {
                    printf("read error\n");
                    return -1;
                }
                else
                {
                    if((write(cli_fd, buf, ret)) < 0)
                    {
                        printf("write error\n");
                        return -1;
                    }
                }
            }
        }
        else if(childpid == -1)
        {
            printf("fork error\n");
            return -1;
        }

/*
        ret = read(cli_fd, buf, 1024);
        if(ret <0)
        {
            printf("read error\n");
            return -1;
        }
        else
        {
            if((write(cli_fd, buf, ret)) < 0)
            {
                printf("write erroe\n");
                return -1;
            }
        }
*/
        close(cli_fd);
    }
}
