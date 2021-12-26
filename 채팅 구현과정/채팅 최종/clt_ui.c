#include<curses.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<string.h>
#include<pthread.h>
/* for ctrl-C */
#include<signal.h>
/* for inet_aton */
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
/* for usleep */
#include<sys/timeb.h>
#include<time.h>

#include<stdarg.h>
struct stat route;
char command[250];

/* GNU style struct initialization */
//   v one for "All"//point client array (sorted)
int online = 0;//the num people on line
char my_id[11], cur_id[20];
char recvr_n[11]= {"All"};
int svr_fd;

///function for socket
void escape(int a);
void redraw(int mod);
//var for curses
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
char history[1000][300] = {"\0"};
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

//start main program
int main() {
	signal(SIGINT, escape);
	int i;
	char output[400];
	strcpy(my_id, "nobody");
	// input name
	printf("Input your name(less than 10 letters):");
	//fgets(my_id , sizeof(my_id) , stdin);
	scanf("%s", my_id);
	for (i = 0;i<sizeof(my_id);i++) {
		if (my_id[i] == '\n') {
			my_id[i] = '\0';
		}
	}
	strcpy(cur_id, my_id);
	setbuf(stdin, NULL);
	// start curses terminal
	initial();

	/*   bbox                    | mbox                  */
	/*  -------------------------|                       */
	/*   tbox   +----------+     |                       */
	/*          |   rbox   |     |                       */
	/*          +----------+     |                       */
	/*                  ---+     |                       */


	mbox = newwin( 2, 12, 0, COLS - 13);
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
		
		wrefresh(ibox);
		wmove(ibox, 1, 0);
		if (terminal(ibox, string, 280) == ERR) {
			printf("terminal ERR\n");
			escape(0);
		}
		if (string[0] != '\0') {
			/*if (!strcmp(recvr_n, "All"))
				combsend(svr_fd, send_str,
					sizeof(send_str),
					"%s %s", PUBC_MSG,
					string);
			else
				combsend(svr_fd, send_str,
					sizeof(send_str),
					"%s %s %s", PRVT_MSG,
					recvr_n, string);
			*/
			sprintf(output, "%s:%s" , my_id,string);
			mvwaddstr(tbox, tbox_c,0,output);
			strcpy(history[curline], output);
			for(int i=0;i<(strlen(output)/(tbox->_maxx+1));i++){
				tbox_c++;
				curline++;
			}
				if((strlen(output)%(tbox->_maxx+1))!=0){
					tbox_c++;
					curline++;
					}
			while(tbox_c>LINES-(ibox->_maxy+1)-(bbox->_maxy+1)){				wscrl(tbox , 1);
			    tbox_c--;
			   tbox_t++;
			}


		}
		memset(string, 0, sizeof(string));
		wclear(ibox);
		mvwprintw(pbox, 1, 0, "%c", ps);
		wrefresh(pbox);
		redraw(1);
	}
	return 0;
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
		mvwAttrw(wbox, 0, 10- strlen(recvr_n), A_BOLD, " TO %s", recvr_n);
		}
	mvwprintw(bbox, 0, 4, "Hi!");
	mvwWipen(bbox, 0, 7, 10);
	mvwAttrw(bbox, 0, 7, A_BOLD, "%s", cur_id);

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
	if(mod == 1){
		wmove(win_c, win_c ->_cury, win_c->_curx);
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
