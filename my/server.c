/* timeserv.c - a socket-based time of day server
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#define PORTNUM 22445	/* our time service phone number */
#define HOSTLEN 256
#define oops(msg)	{ perror(msg); exit(1); }

#define bufsize 1024

static int thr_exit = 1;

void *thread_recv(void *arg){
	char recv_data[100];
	while(!thr_exit){
		printf("%s\n", (char*)arg);
		recv(arg, recv_data, sizeof(recv_data), 0);
		printf("%s\n", recv_data);
	}
	exit(0);
}



void socket_init(int*, int*);

int main(int ac, char *av[])
{
	int sock_id, sock_fd;		/* line id, file descriptor	*/
	FILE *sock_fpW, *sock_fpR;	/* use socket as steam		*/
	char buf[bufsize];
	socket_init(&sock_id, &sock_fd);

	while(1){
		printf("waiting ...\n");
		sock_fd = accept (sock_id, NULL, NULL); /* wait for call */
		printf("Wow! got a call!\n");
		if( sock_fd == -1 )
			oops( "accept" );	/* error getting calls */

		pthread_t thr;
          	while(1){
			char* recv_data;
			char chat_data[100];
			thr_exit = 0;
			pthread_create(&thr, NULL, thread_recv, (void*)sock_fd);
			//recv(sock_fd, recv_data, sizeof(recv_data),0);
			//printf("%s\n", recv_data);
			fgets(chat_data, sizeof(chat_data), stdin);
			send(sock_fd, chat_data,sizeof(chat_data),0);
		}
		thr_exit = 1;
		pthread_join(thr, NULL);
	}
}


void socket_init(int* sock_id, int* sock_fd){
	struct sockaddr_in saddr;	/* build our address here	*/
	struct hostent *hp;		/* this part of our		*/
	char hostname[HOSTLEN];		/* address			*/
	/*
	 * Step 1: ask kernel for a socket
	 */
	*sock_id = socket( PF_INET, SOCK_STREAM, 0);
	if( *sock_id == -1)
		oops( "socket" );

	/* 
	 * Step 2: bind address to socket. Address is host, port  
	 */
	bzero ( (void *) &saddr, sizeof( saddr) ); /* clear out struct	*/

	gethostname( hostname, HOSTLEN );	   /* where am I ?	*/
	hp = gethostbyname( hostname );		   /* get info about host */
						   /* fill in host part	*/
	bcopy( (void *) hp->h_addr, (void *) &saddr.sin_addr, hp->h_length);
	saddr.sin_port = htons(PORTNUM);	   /* fill in socket port */
	saddr.sin_family = AF_INET;		   /* fill in addr family */


	if( bind(*sock_id, (struct sockaddr *) &saddr, sizeof(saddr)) != 0 )
		oops( "bind" );

	/* 
	 * Step 3: allow incoming calls with Qsize=1 on socket
	 */

	if ( listen (*sock_id, 1) != 0 )
		oops( "listen" );

}
