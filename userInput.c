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

#include <ncurses.h>
// $gcc -o nosocket nosocket.c -lncurses	

#define BUF_SIZE 100
#define NAME_SIZE 20

#define IN_BUF 100

// this is for ncurses
#define COLOR_BLACK 0
#define COLOR_RED 1
#define COLOR_GREEN 2
#define COLOR_YELLOW 3
#define COLOR_BLUE 4
#define COLOR_MAGENTA 5
#define COLOR_CYAN 6
#define COLOR_WHITE 7

void init(int, char**);
void error_handling(char * msg);

char IP[20];
int port;
char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];

//자리 비움 관련된 것임
char disturb[300][300]={"\0"};
int i=0;
int afk_mode =0;

//공지 사항
char noticebuffer[300];

char inputbuf[IN_BUF];


void initial(WINDOW *top, WINDOW *talkbox);
void colored();
void make_top(WINDOW *top);
void make_tbox(WINDOW *talkbox);

    WINDOW 
//    *mainWin,   // at ncurses : stdscr
    *top,
//    *infoLine,  // left side
//    *chatWin,   // changed to main window
    *talkbox; 

void userInput(void);

int main(int argc, char argv[])
{
	init(argc, argv);

    initial(top, talkbox);
    move(2, 2);
    printw("Enter 'Exit' to exit the program.");
    refresh();

    while(1){
//      wgetstr(talkbox, inputbuf); 
//        stdscr = talkbox;
//        scanw("%s", inputbuf);  

//        int key; 
        userInput();

//        wscanw(talkbox,"%s",inputbuf);

//        printw("%s", inputbuf); 
//        userinput에서 \n 입력받으면 해당 기능 후 버퍼 초기화하도록 수정했음

    touchwin(stdscr);
//    touchwin(talkbox);
    refresh();

    }

    endwin();
	return 0;
}


void colored(){
//   init_pair(1, -1, -1); // Default
    start_color();
   init_pair(1, COLOR_WHITE, COLOR_BLUE); // main window. txt color, bg color
   init_pair(2, COLOR_BLUE, COLOR_WHITE); // top 
   init_pair(3, COLOR_RED, COLOR_WHITE); // notice?
//   init_pair(4, COLOR_MAGENTA, COLOR_BLUE);
//   init_pair(4, COLOR_MAGENTA, COLOR_WHITE);

   
}


//
void userInput(){

    echo();
    int key; move(22, 4);
    key = getch();
    int char_i = 0; // def = 0 (nothing in buffer)

// read&write to buffer until [Enter] input
    while(key != '\n') {
      // Backspace
      if (key == 8 || key == 127 || key == KEY_LEFT) 
      {
         if (char_i > 0) 
         {
            wmove(talkbox, char_i, 2);
            wprintw(talkbox, "\b \b\0");
            inputbuf[--char_i] = '\0';
            wrefresh(talkbox);
         }
         else 
         { // nothing in buffer
            wmove(talkbox, char_i, 2);
            wprintw(talkbox, "\b \0");
         }
      }
      // write something in input buffer(str)
      else if (key!= ERR)
      {
          if (char_i<(IN_BUF-1))
          {
              strcat(inputbuf, (char*)&key);
              char_i++;
              wmove(talkbox, char_i, 2);
              wprintw(talkbox, (char*)&key);
              wrefresh(talkbox);
          }
          else // buffer full
          {
            wmove(talkbox, char_i, 2);
            wprintw(talkbox, "\b%s", (char*)&key);
            inputbuf[char_i-1] = '\0';
            strcat(inputbuf, (char*)&key);
            wrefresh(talkbox);              
          }
      }

      key = getch();

    }

    // print out buffers
    move(keyidx+3, 3);
    printw("%s\n", inputbuf);   keyidx++; 

    wclear(talkbox);
    wrefresh(talkbox);
    inputbuf[0] = '\0';
}

void make_top(WINDOW *top)
{
    top = subwin(stdscr, 2, 80, 0, 0);
    wbkgd(top, COLOR_PAIR(2));
    wattron(top, COLOR_PAIR(2));
    wmove(top, 0, 30);
    waddstr(top, "====CHATROOM====");

}

void make_tbox(WINDOW *talkbox){
    talkbox = subwin(stdscr, 4 , 75 , 20 , 1);
    wbkgd(talkbox, COLOR_PAIR(2));
    wattron(talkbox, COLOR_PAIR(2));
    box(talkbox, '|','-');
    wcursyncup(talkbox);
    wrefresh(talkbox);
}

void initial(WINDOW *top, WINDOW *talkbox){		// start windows using curses.h
    initscr();
    cbreak();
//    nonl();
    echo();
    curs_set(2);
    keypad(stdscr, TRUE);
    scrollok(stdscr , TRUE);
    keypad(talkbox, TRUE);
    colored();

    bkgd(COLOR_PAIR(1));

//    mainWin = newwin(LINES , COLS , 0 , 0); 세로, 가로, 시작 lines 시작 cols 
//    top = subwin(stdscr, 2, 80, 0, 0); // window, lines-세로, cols-가로, y(세로), x(가로) 
//  moved to maketop
    make_top(top);

//    chatWin = subwin(stdscr, 21 , 60 , 3 , 20);
//    infoLine = subwin(stdscr, 21 , 20 , 3 , 0);
//    talkbox = newwin(20 , 80 , 23 , 0);
    make_tbox(talkbox);

/*
	box(top, '|','-');
	box(chatWin, '|','-');
	box(infoLine, '|','-');
	box(talkbox, '|','-');
*/
    touchwin(stdscr);
    refresh();  
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

