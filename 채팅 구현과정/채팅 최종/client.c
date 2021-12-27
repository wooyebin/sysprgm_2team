#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/timeb.h>
#include <time.h>
#include <stdarg.h>
#include <curses.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUF_SIZE 100
#define NAME_SIZE 20
struct stat route;
char command[250], send_str[250];
/* GNU style struct initialization */
//   v one for "All"//point client array (sorted)
int online = 0;//the num people on line
char my_id[11], cur_id[20];
char recvr_n[11] = { "All" };
int svr_fd, clt_fd;


typedef struct {
	char roomName[20]; //chatroom name
	int cnt; //people count
	int roomnum; //chatroom portnum
	int option; //to check enter or make room
	int clnt_socks[10];
	char clnt_names[10][20];
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

void escape(int);
void redraw(int);

char IP[20];
int port;
char output[BUF_SIZE];
char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];
int room = 0;
roominfo info;
static sem_t sem_one;
static sem_t sem_two;

int emojiCount = 1;
char emoji[1][100];


//ÀÚ¸®ºñ¿ò   
char disturb[300][300] = { "\0" };
int i = 0;
int didx = 0;
int afk_mode = 0;

//         
char noticebuffer[300];

WINDOW *win[7],
*win_c, /* current win */
*rbox, /* root win */
*mbox, /* member list box */
*bbox, /* banner box */
*tbox, /* talk box */
*ibox, /* msg input box */
*wbox, /* msg to whom displayer */
*obox, /* option box */
*pbox; /* beside input box to print ps */

	   /*   bbox                    | mbox   */
	   /*  -------------------------|        */
	   /*   tbox   +----------+     |        */
	   /*          |   rbox   |     |        */
	   /*          +----------+     |        */
	   /*                           |        */
	   /*  -------------------------|------  */
	   /*  pbox| ibox       ________| obox   */
	   /*      |           |  wbox  |        */


int mbox_t = 0;//the user current mbox top line point
char history[1000][300] = { "\0" };
int tbox_t = 0;//for talk box
int curline = 0;//for history
int tbox_c = 0;//for talk bodx
int x, y;

char ps = '$';//$(normal) >(just good) #(root)

			  //function for curses
void initial(void);
void mvwWipen(WINDOW *awin, int y, int x, int n);
void mvwAttrw(WINDOW *awin, int y, int x, int attrs, char *format, ...);
int terminal(WINDOW *twin, char *str, int n);//manipulate insert box

											 //normal function
ssize_t combsend(int fd, char *msg, unsigned int msg_t, char *format, ...);

int main(int argc, char *argv[])
{
	int sock;
	strcpy(emoji[0], "^^\n");
	init(argc, argv);
	socket_init(&sock);
	logInPage(sock);
	chat_start(&sock);
	close(sock);
	endwin();
	return 0;
}



////////////////////////////////////////////////////////////////////

void init(int argc, char** argv) {
	if (argc != 4) {
		printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		exit(1);
	}
	sprintf(name, "[%s]", argv[3]);
	strcpy(IP, argv[1]);
	port = atoi(argv[2]);
}

void socket_init(int* sock) {
	struct sockaddr_in serv_addr;
	*sock = socket(PF_INET, SOCK_STREAM, 0);
	if (*sock == -1)
		error_handling("socket");
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(IP);
	serv_addr.sin_port = htons(port);

	if (connect(*sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error");
}
void logInPage(int sock) {

	//enter NickName    
	//printf("Show your NickName : ");
	//fgets(name, BUF_SIZE, stdin);

	showmenu(sock);
}


void showmenu(int sock) {
	int menu = 0;
	printf("1. Create Room\n");
	printf("2. Enter Room\n");
	printf("3. Exit\n");
	printf("4. Help\n\n");

	printf("Input Menu : ");
	scanf("%d", &menu);
	while ((menu < 1) || (menu > 4)) {
		printf("Not correct Number please try again\n");
		printf("Input Menu : ");
		scanf("%d", &menu);
	}

	if (menu == 1) {
		createRoom(sock, info);
	}
	else if (menu == 2) {
		enterRoom(sock);
	}
	else if (menu == 3) {
		printf("terminate chatting room program\n");
		exit(1);
	}
	else if (menu == 4) {
		help();
	}
	else {
		printf("error in menu \n");
	}

}

void help() {
	FILE *fp;
	char buf[BUF_SIZE];
	fp = fopen("README.txt", "r");

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		printf("%s", buf);
		memset(buf, 0, sizeof(buf));
	}
	fclose(fp);

}

void createRoom(int sock, roominfo rinfo) {
	int status;
	int str_len = 0;
	int pid;
	char roomname[NAME_SIZE];

	//enter roomname
	//child process, receive portnum from Rmanage_serv.c
	roominfo sendinfo;
	for (int i = 0; i<10; i++) {
		str_len = read(sock, (void*)&sendinfo, sizeof(sendinfo));
		if (sendinfo.cnt > 0) {
			printf("%s %d %d ", sendinfo.roomName, sendinfo.cnt, sendinfo.roomnum);
			for (int j = 0; j<sendinfo.cnt; j++) {
				printf("%s ", sendinfo.clnt_names[j]);
			}
			printf("\n");
		}
	}
	//input roomName
	printf("input your roomName to create: ");
	scanf("%s", roomname);
	char temp;
	scanf("%c", &temp);
	//fgets(roomname, NAME_SIZE, stdin);        

	strcpy(rinfo.roomName, roomname);

	//input option
	rinfo.option = 1;
	strcpy(rinfo.clnt_names[0], name);
	//send to rmanage server
	write(sock, (void*)&rinfo, sizeof(info));

	//for receive rmanage server           
	str_len = read(sock, (void*)&rinfo, sizeof(info));
	if (str_len == -1) {
		error_handling("roominfo error");
	}

	// for testing
	printf("recv roomnum : %d\n", rinfo.roomnum);
	// because it is tcp, we have two option 1,2


	//start chat
	//chatting(chatclnt_sock, rinfo);
}
void enterRoom(int sock) {
	char enterRoomName[NAME_SIZE];
	int str_len;
	roominfo sendinfo;
	for (int i = 0; i<10; i++) {
		str_len = read(sock, (void*)&sendinfo, sizeof(sendinfo));
		if (sendinfo.cnt > 0) {
			printf("%s %d %d ", sendinfo.roomName, sendinfo.cnt, sendinfo.roomnum);
			for (int j = 0; j<sendinfo.cnt; j++) {
				printf("%s ", sendinfo.clnt_names[j]);
			}
			printf("\n");
		}
	}
	//input roomName
	printf("input your roomName to enter : ");

	//fgets(enterRoomName, NAME_SIZE, stdin);          
	scanf("%s", enterRoomName);
	char temp;
	scanf("%c", &temp);
	strcpy(sendinfo.roomName, enterRoomName);

	sendinfo.option = 2;
	strcpy(sendinfo.clnt_names[0], name);
	write(sock, (void*)&sendinfo, sizeof(sendinfo));

	printf("\n loading......\n");
	sleep(1);

	str_len = read(sock, (void*)&sendinfo, sizeof(sendinfo));
	if (str_len == -1) {
		error_handling("roominfo error");
	}

	if (sendinfo.roomnum == -1) {
		printf("this chatting room name is not in room information.\n");
		error_handling("not chatting room name");
	}
	else {
		printf("write to chatclnt roomnum : %d \n", sendinfo.roomnum);
	}

	//close(sock);
	//start chat

	//chatting(chatclnt_sock, sendinfo);
}



void chat_start(int* sock) {
	pthread_t snd_thread, rcv_thread;
	void * thread_return;

	pthread_create(&snd_thread, NULL, send_msg, (void*)sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)sock);

	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
}

int msgcheck(char* msg)
{
	strcat(msg,"\n");
	if (!strcmp(msg, "afk\n"))
	{
		strcpy(msg, "is away from keyboard.");
		afk_mode = 1;
		wclear(ibox);
		redraw(1);
		return 1;
	}
	else if (!strcmp(msg, "nafk\n"))
	{
		strcpy(msg, "is Not away from keyboard.");
		
		for (int j = 0;j<didx;j++)
		{
			
			mvwprintw(tbox, tbox_c, 0, disturb[j]);
			tbox_c++;
		}
		didx = 0;
		wclear(ibox);
		redraw(1);
		afk_mode = 0;
		return 1;
	}
	else if (!strcmp(msg, "notice\n")) {
		// color(noticebuffer);
		mvwprintw(tbox, ++tbox_c, 0, noticebuffer);
		mvwprintw(tbox, tbox_c, 0, noticebuffer);
		tbox_c++;
		return 2;
	}
	else if (!strcmp(msg, "emoji\n")) {
		for (int i = 0; i<emojiCount; i++) {
			printf("%d : %s\n", i, emoji[i]);
		}
		return 3;
	}
	else if (!strcmp(msg, "q") || !strcmp(msg, "Q")) {
		return -1;
	}

	else { return 0; }
}

void * send_msg(void * arg)   // send thread main
{
	signal(SIGINT, escape);
	int i;
	char output[400];
	strcpy(my_id, name);
	sleep(1);
	
	getyx(ibox, y, x);

	int sock = *((int*)arg);
	char name_msg[NAME_SIZE + BUF_SIZE];
	char string[300];
	while (1) {
		/*wrefresh(ibox);*/
		wmove(ibox, 1, 0);
		if (terminal(ibox, string, 280) == ERR) {
			printf("terminal ERR\n");
			escape(0);
		}
		if (string[0] != '\0') {
			strcpy(msg, string);
			int msgcheckNum = msgcheck(msg);
			if (msgcheckNum == 1 || msgcheckNum == 2) { // afk
				
				//continue;
			}
			if (msgcheckNum == -1) {
				close(sock);
				strcpy(name_msg, "");
				sprintf(name_msg, "%s is quit\n", name);
				write(sock, name_msg, strlen(name_msg));
				endwin();
				close(svr_fd);
				exit(0);
			}
			if (msgcheckNum == 3) {
				int emojiNum = 0;
				char temp;
				scanf("%d", &emojiNum);
				scanf("%c", &temp);
				strcpy(msg, emoji[emojiNum]);
			}
			make_msg(msg, name_msg);
			write(sock, name_msg, strlen(name_msg));
			
		}
		memset(string, 0, sizeof(string));
		wclear(ibox);
		mvwprintw(pbox, 1, 0, "%c", ps);
		wrefresh(pbox);
		redraw(1);
	}

	
}

void make_msg(char* msg, char* name_msg) {
	 
	if (strstr(msg, "notice:"))
	{
		sprintf(name_msg, "%s", msg);
	}
	
	else {
		sprintf(name_msg, "%s %s", name, msg);
	}
}


void * recv_msg(void * arg)   // read thread main
{
	int sock = *((int*)arg);
	char name_msg[NAME_SIZE + BUF_SIZE];
	int str_len;
	signal(SIGINT, escape);
	int i;
	char output[400];

	//combsend(svr_fd, send_str, sizeof(send_str), "name %s", cur_id);



	// start curses terminal
	initial();

	/*   bbox                    | mbox                  */
	/*  -------------------------|                       */
	/*   tbox   +----------+     |                       */
	/*          |   rbox   |     |                       */
	/*          +----------+     |                       */
	/*                           |                       */
	/*  -------------------------|------                 */
	/*  pbox| ibox       ________| obox                  */
	/*      |           |  wbox  | ctrl-l to call rbox   */

	mbox = newwin(2, 12, 0, COLS - 13);
	bbox = newwin(3, COLS - 13, 0, 0);
	tbox = newwin(1000, COLS - 13, 3, 0);
	pbox = newwin(5, 2, LINES - 5, 0);
	ibox = newwin(5, COLS - 23, LINES - 5, 2);
	obox = newwin(6, 20, LINES - 6, COLS - 21);
	wbox = newwin(1, 15, LINES - 5 + (ibox->_maxy), (ibox->_maxx) - 12);

	win[0] = tbox, win[1] = ibox, win[2] = mbox;
	win[3] = bbox, win[4] = obox, win[5] = wbox, win[6] = pbox;

	for (i = 0; i < 7; i++)
		if (win[i] == NULL) puts("NULL"), exit(1);
	for (i = 0; i < 4; i++)
		keypad(win[i], TRUE);
	keypad(rbox, TRUE);

	wsetscrreg(tbox, 0, 299);
	scrollok(tbox, TRUE);// let box can be scrolled
	scrollok(mbox, TRUE);
	idlok(tbox, TRUE);
	leaveok(tbox, TRUE);

	rbox = newwin(10, COLS / 3, 5, COLS / 3);
	win_c = ibox;
	redraw(0);//need win_c
	wmove(ibox, 1, 0);
	mvwprintw(pbox, 1, 0, "%c", ps);
	wrefresh(pbox);
	wrefresh(ibox);
	mvwprintw(mbox, 0, 4, "All");
	wrefresh(mbox);
	getyx(ibox, y, x);

	char string[300];
	while (1) {

		/*wrefresh(ibox);
		wmove(ibox, 1, 0);*/
		str_len = read(sock, name_msg, NAME_SIZE + BUF_SIZE - 1);
		if (str_len == -1)
			return (void*)-1;
		name_msg[str_len] = 0;

		if (strstr(name_msg, "notice:"))
		{
			strcpy(noticebuffer, name_msg);
			mvwprintw(bbox, 1, 25, noticebuffer);
		}
		if (afk_mode == 1)
		{
			strcpy(disturb[didx], name_msg);
			didx++;
		}
		else {
			//	fputs(name_msg, stdout);
			strcpy(output, name_msg);
			mvwaddstr(tbox, tbox_c, 0, output);
			strcpy(history[curline], output);
			for (int i = 0;i<(strlen(output) / (tbox->_maxx + 1));i++) {
				tbox_c++;
				curline++;
			}
			if ((strlen(output) % (tbox->_maxx + 1)) != 0) {
				tbox_c++;
				curline++;
			}
			while (tbox_c>LINES - (ibox->_maxy + 1) - (bbox->_maxy + 1)) {
				wscrl(tbox, 1);
				tbox_c--;
				tbox_t++;
			}
			
			wmove(ibox, 1, 0);
			usleep(10000);
			redraw(1);
		}
	}


}

void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}




void escape(int idx) {
		char exit_msg[3][25] = {
	"Exit chatroom" ,
	"connection break" ,
	"server terminated" ,
	};
	endwin();
	close(svr_fd);
	puts(exit_msg[idx]);
	exit(0);
	printf("\nif you want to terminate Chatting program enter 'q' or 'Q'\n");
}

void initial(void) {
	initscr();
	cbreak();
	nonl();
	noecho();
	intrflush(stdscr, FALSE);
	intrflush(mbox, FALSE);
	keypad(stdscr, TRUE);
	refresh();
	return;
}

void mvwWipen(WINDOW *awin, int y, int x, int n) {
	for (int i = x; i < x + n; i++)
		mvwaddch(awin, y, i, ' ');
	return;
}

void mvwAttrw(WINDOW *awin, int y, int x, int attrs, char *format, ...) {
	char text[300];
	wattron(awin, attrs);
	va_list arg;
	va_start(arg, format);
	vsnprintf(text, sizeof(text), format, arg);
	va_end(arg);
	mvwprintw(awin, y, x, text);
	wattroff(awin, attrs);
}

void redraw(int mod) {
	int w1cy, w1cx;
	int i;

	w1cy = win_c->_cury;
	w1cx = win_c->_curx;

	for (i = 0;i <= ibox->_maxx;i++) // insert box border
		mvwaddch(ibox, 0, i, '-');

	win_c->_cury = w1cy;
	win_c->_curx = w1cx;

	for (i = 0;i <= pbox->_maxx;i++)
		mvwaddch(pbox, 0, i, '-');
	for (i = 0;i <= obox->_maxx;i++) //opinion box
		mvwaddch(obox, 0, i + 1, '-');
	for (i = 0;i <= obox->_maxy;i++)
		mvwaddch(obox, i, 0, '|');


	for (i = 0;i <= bbox->_maxx;i++)//command line
		mvwaddch(bbox, 2, i, '-');

	mvwprintw(bbox, 1, 0, "===CHATROOM===");


	for (i = 0;i <= mbox->_maxy;i++)//member box
		mvwaddch(mbox, i, 0, '|');

	if (mbox_t<(online - (LINES - 6))) {
		mvwprintw(obox, 0, 12, "v");
	}
	else {
		mvwprintw(obox, 0, 12, "-");
	}

	for (i = 0;i<mbox->_maxx;i++)//it's not <=  maxx because  = maxx it will be scrl
		mvwaddch(mbox, mbox->_maxy, i, '-');

	if (mod == 0) {
		mvwWipen(wbox, 0, 0, 15);
		//mvwprintw(wbox , 0 , 0 , "               ");
		mvwAttrw(wbox, 0, 10 - strlen(recvr_n), A_BOLD, " To %s", recvr_n);
		
	}
	mvwprintw(bbox, 0, 4, "Hi!");
	mvwWipen(bbox, 0, 7, 10);
	
	mvwAttrw(bbox, 0, 7, A_BOLD, "%s", name);
	

	touchwin(tbox);
	wrefresh(tbox);
	wrefresh(mbox);
	wrefresh(bbox);
	touchwin(obox);
	wrefresh(obox);
	touchwin(ibox);
	wrefresh(ibox);
	touchwin(wbox);
	wrefresh(wbox);
	touchwin(pbox);
	wrefresh(pbox);
	if (mod == 1) {
		wmove(win_c, win_c->_cury, win_c->_curx);
		wrefresh(win_c);
	}


	return;
}
int terminal(WINDOW *twin, char *str, int n) {
	char *ostr, ec, kc;
	int c, oldx, remain;
	int prex, prey;
	int i;
	ostr = str;
	ec = erasechar();
	kc = killchar();

	oldx = twin->_curx;
	remain = n - 1;

	while ((c = wgetch(twin)) != ERR && c != '\n' && c != '\r') {

		*str = c;
		touchline(twin, twin->_cury, 1);
		if (c == ec || c == KEY_BACKSPACE || c == 263 || c == 127 || c == 8) {
			*str = '\0';
			if (str != ostr) {
				/* getch() displays the key sequence */
				if (mvwaddch(twin, twin->_cury, twin->_curx - 1, ' ') == ERR) {
					if (mvwaddch(twin, twin->_cury - 1, twin->_maxx, ' ') == ERR) {
						//DEBUG
					}
				}
				if (wmove(twin, twin->_cury, twin->_curx - 1) == ERR) {
					if (wmove(twin, twin->_cury - 1, twin->_maxx) == ERR) {
						//DEBUG
					}
				}
				str--;
				*str = '\0';
				if (n != -1) {
					/* We're counting chars */
					remain++;
				}
			}
		}
		else if (c == kc) {
			*str = '\0';
			if (str != ostr) {
				/* getch() displays the kill character */
				//				if(mvwaddch(twin , twin->_cury , twin->_curx - 1 , ' ') == ERR)
				//					mvwaddch(twin , twin->_cury-1 , twin->_maxx , ' ');
				/* Clear the characters from screen and str */
				while (str != ostr) {
					if (mvwaddch(twin, twin->_cury, twin->_curx - 1, ' ') == ERR) {
						if (mvwaddch(twin, twin->_cury - 1, twin->_maxx, ' ') == ERR) {
						}
					}
					if (wmove(twin, twin->_cury, twin->_curx - 1) == ERR) {
						if (wmove(twin, twin->_cury - 1, twin->_maxx) == ERR) {
						}
					}
					str--;
					if (n != -1)
						/* We're counting chars */
						remain++;
				}
			}
			else
				/* getch() displays the kill character */
				mvwaddch(twin, twin->_cury, oldx, ' ');
			wmove(twin, twin->_cury, oldx);

		}
		else if (c == '\t' || c == 27) {
			prey = twin->_cury;//record text position
			prex = twin->_curx;
			curs_set(0);
			mvwprintw(pbox, 1, 0, " ");
			wrefresh(pbox);
			win_c = ibox;
			mvwprintw(pbox, 1, 0, "%c", ps);
			wrefresh(pbox);
			curs_set(1);
			twin->_cury = prey;
			twin->_curx = prex;
			wmove(twin, twin->_cury, twin->_curx);
			wrefresh(ibox);

		}
		else if (c == 14) {
			char psn[] = "$>@!%?$";
			ps = psn[(strchr(psn, ps) - psn + 1) % strlen(psn)];
			mvwprintw(pbox, 1, 0, "%c", ps);
			wrefresh(pbox);
		}
		else if (c == KEY_UP || c == KEY_DOWN || c == KEY_LEFT || c == KEY_UP) {
			if (c == KEY_UP) {
				if (tbox_t != 0) {

					wscrl(tbox, -1);

					int i = strlen(str) - 1;
					mvwprintw(tbox, 0, 0, "%s", history[tbox_t - 1]);
					for (i = 0;i <= 6;i++) {
						if ((i != 2) && (i != 3)) {
							touchwin(win[i]);
							wrefresh(win[i]);
						}
					}
					tbox_t--;
					tbox_c++;
				}
			}
			else if (c == KEY_DOWN) {
				if (tbox_t<(curline - (LINES - (ibox->_maxy + 1) - (bbox->_maxy + 1)))) {
					wscrl(tbox, 1);
					for (i = 0;i <= 6;i++) {
						if ((i != 2) && (i != 3)) {
							touchwin(win[i]);
							wrefresh(win[i]);
						}
					}
					tbox_t++;
					tbox_c--;
				}
			}
		}
		else if (c >= KEY_MIN && c <= KEY_MAX) {}//disable other function key
		else if (c >= 32 && c <= 126) {
			mvwaddch(twin, twin->_cury, twin->_curx, c);//good job
			wrefresh(ibox);
			if (remain) {
				str++;
				remain--;
			}
			else {
				mvwaddch(twin, twin->_cury, twin->_curx - 1, ' ');//
				wmove(twin, twin->_cury, twin->_curx - 1);
			}
		}
		wrefresh(ibox);
		touchwin(wbox);
		wrefresh(wbox);
		if (wmove(twin, twin->_cury, twin->_curx) == ERR)
			wmove(twin, twin->_cury - 1, twin->_maxx);
	}

	if (c == ERR) {
		*str = '\0';
		return (ERR);
	}
	*str = '\0';
	wrefresh(tbox);
	return (OK);
}

ssize_t combsend(int fd, char *msg, unsigned int msg_t, char *format, ...) {
	va_list arg;
	va_start(arg, format);
	vsnprintf(msg, msg_t, format, arg);
	va_end(arg);
	return send(fd, msg, msg_t, 0);
}
