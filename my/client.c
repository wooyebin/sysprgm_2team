/* timeclnt.c - a client for timeserv.c
 *		usage: timeclnt hostname portnumber
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <stdlib.h>
#include <strings.h>

#define oops(msg)	{ perror(msg); exit(1); }

void socket_init(int*, char**);

int main(int ac, char *av[])
{
	int	sock_id/*, sock_fd*/;	/* the socket and fd	*/
	char	message[BUFSIZ];	/* to receive message	*/
	int	messlen;		/* for message length	*/
	
	socket_init(&sock_id, av);	

	while(1){
		char* str, recv_data;
		//recv(sock_id, recv_data, sizeof(recv_data),0);
		//printf("%s\n", recv_data);
		scanf("%s", str);
		send(sock_id, str, sizeof(str), 0);
	}
	close( sock_id );
}

void socket_init(int* sock_id, char** av){
	struct	sockaddr_in servadd;	/* the number to call	*/
	struct	hostent *hp;		/* used to get number	*/

	
	/*
	 * Step 1: Get a socket
	 */
	*sock_id = socket( AF_INET, SOCK_STREAM, 0);	/* get a line */
	if( *sock_id == -1)
		oops( "socket" );			/* or fail */

	/* 
	 * Step 2: connect to server
	 *	   need to build address ( host, port ) of server first
	 */
	bzero ( &servadd, sizeof(servadd) ); 	/* zero the address */
	hp = gethostbyname(av[1]);		/* lookup host's ip # */
	if ( hp == NULL)
		oops(av[1]);
	bcopy( hp->h_addr, (struct sockaddr *)&servadd.sin_addr, hp->h_length);
	servadd.sin_port = htons(atoi(av[2]));	/* fill in port number */
	servadd.sin_family = AF_INET;		/* fill in socket type */

						/* now dial */
	if(connect(*sock_id, (struct sockaddr *)&servadd, sizeof(servadd)) != 0 )
		oops( "bind" );
}

