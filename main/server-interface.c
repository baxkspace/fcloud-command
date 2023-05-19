#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <curses.h>
#include <string.h>
#include <signal.h>
#include "database.h"

#define SELECTED_MENU 1
#define UNSELECTED_MENU 2
#define MENU 4

int cur_r, cur_c;
void print_welcome(char* name);
void menu_number(struct winsize w);
char id[30], pw[100];

// color set
init_pair(SELECTED_MENU, COLOR_WHITE, COLOR_MAGENTA);
init_pair(UNSELECTED_MENU, COLOR_WHITE, COLOR_CYAN);
init_pair(MENU, COLOR_BLACK, COLOR_MAGENTA);

int main(int argc, char **argv) {
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	int row = w.ws_row, col = w.ws_col;

	signal(SIGWINCH, SIG_IGN);

	login();

	initscr();
	start_color();

	print_welcome(id);

	// make menu - selected small
	move(cur_r + 2, 2);
	attron(COLOR_PAIR(SELECTED_MENU));
	printw(" cloud ");
	attroff(SELECTED_MENU);

	// unselected small
	move (cur_r + 2, 4+strlen(" cloud "));
	attron(COLOR_PAIR(UNSELECTED_MENU));
	printw(" local ");
	attroff(COLOR_PAIR(UNSELECTED_MENU));

	// ls info
	move(cur_r + 3, 0);
	attron(COLOR_PAIR(SELECTED_MENU));
	for (int i = 0; i < w.ws_col; i++)
		printw(" ");
	attroff(COLOR_PAIR(SELECTED_MENU));
	move(cur_r + 3, 2);
	attron(COLOR_PAIR(MENU));
	printw("%-18s %-17s %-10s %-12s %-17s", "file name", "uploader", "size", "mode", "time");

	move(w.ws_row - 5, 0);
	for (int i =0; i < w.ws_col; i++) {
		printw(" ");
	}
	attroff(COLOR_PAIR(MENU));
	menu_number(w);

	getch();
	endwin();

	return 0;
}

void print_welcome(char* name) {
	attrset(A_BOLD);
	cur_r = 0;
	cur_c = 2;
	move(++cur_r, cur_c);
	printw(" >> Welecome to FCLOUD of ");
	init_pair(3, COLOR_BLUE, COLOR_BLACK);
	attron(COLOR_PAIR(3));
	printw("%s", name);
	attroff(COLOR_PAIR(3));
	attroff(A_BOLD);
}

void menu_number(struct winsize w) {
	move(w.ws_row - 4, 2);
	init_pair(5, COLOR_BLACK, COLOR_WHITE);
	attron(COLOR_PAIR(5));
	printw("< list of menu >");
	attroff(COLOR_PAIR(5));
	move(w.ws_row - 3, 2);
	printw("1. show cloud 2. show local 3. download 4. upload 5. delete");
}

void login() {
	printf("Enter your mysql ID: ");
	scanf("%s", id);
	printf("Enter password: ");
	scanf("%s", pw);
	mysqlConnect(id, pw);
	mysqlMake();
}