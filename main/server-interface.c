#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include "mysqldb.h"

#define SELECTED_MENU 1
#define UNSELECTED_MENU 2
#define MENU 4

int cur_r, cur_c;
void print_welcome(char* name);
void menu_number(struct winsize w);
char id[30], pw[100];
void login();

void show_cloud(struct winsize w);
void show_local(struct winsize w);

int main(int argc, char **argv) {
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	int row = w.ws_row, col = w.ws_col;

	signal(SIGWINCH, SIG_IGN);

	login();

	initscr();
	start_color();

	// color set
	init_pair(SELECTED_MENU, COLOR_WHITE, COLOR_MAGENTA);
	init_pair(UNSELECTED_MENU, COLOR_WHITE, COLOR_CYAN);
	init_pair(MENU, COLOR_BLACK, COLOR_MAGENTA);

	print_welcome(id);

	show_cloud(w);
	char num;
	while (!(num == 'q' || num == 'Q')) {
		num = getch();
		printw("%d", num);
		if (num == '1') {
			show_cloud(w);
		}
		else if (num == '2') {
			show_local(w);
		}
	}
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
	printw("1. show cloud 2. show local 3. download 4. upload 5. delete Q: quit");
}

void login() {
	printf("Enter your mysql ID: ");
	scanf("%s", id);
	printf("Enter password: ");
	scanf("%s", pw);
	mysqlConnect(id, pw);
}

void show_cloud(struct winsize w) {
	move(3, 2);
	attron(COLOR_PAIR(SELECTED_MENU));
	printw(" cloud ");
	attroff(SELECTED_MENU);

	// unselected small
	move (3, 4+strlen(" cloud "));
	attron(COLOR_PAIR(UNSELECTED_MENU));
	printw(" local ");
	attroff(COLOR_PAIR(UNSELECTED_MENU));

	// ls info
	move(4, 0);
	attron(COLOR_PAIR(SELECTED_MENU));
	for (int i = 0; i < w.ws_col; i++)
		printw(" ");
	attroff(COLOR_PAIR(SELECTED_MENU));
	move(4, 2);
	attron(COLOR_PAIR(MENU));
	printw("%-18s %-17s %-10s %-12s %-17s", "file name", "uploader", "size", "mode", "time");

	move(w.ws_row - 5, 0);
	for (int i =0; i < w.ws_col; i++) {
		printw(" ");
	}
	attroff(COLOR_PAIR(MENU));
	menu_number(w);
}

void show_local(struct winsize w) {
	addstr("hi");
	move(3, 2);
	attron(COLOR_PAIR(UNSELECTED_MENU));
	printw(" cloud ");
	attroff(UNSELECTED_MENU);

	// unselected small
	move (3, 4+strlen(" cloud "));
	attron(COLOR_PAIR(SELECTED_MENU));
	printw(" local ");
	attroff(COLOR_PAIR(SELECTED_MENU));

	// ls info
	move(4, 0);
	attron(COLOR_PAIR(SELECTED_MENU));
	for (int i = 0; i < w.ws_col; i++)
		printw(" ");
	attroff(COLOR_PAIR(SELECTED_MENU));
	move(4, 2);
	attron(COLOR_PAIR(MENU));
	printw("%-18s %-17s %-10s %-12s %-17s", "file name", "uploader", "size", "mode", "time");

	move(w.ws_row - 5, 0);
	for (int i =0; i < w.ws_col; i++) {
		printw(" ");
	}
	attroff(COLOR_PAIR(MENU));

	move(5, 0);
	local_list("download");
	menu_number(w);
}

void local_list(char dirname[]) {
	DIR *dir_ptr;
	struct dirent *direntp;
	char path[256];

	chdir(dirname);

	getcwd(path, sizeof(path));

	if ((dir_ptr = opendir(path)) == NULL)
		fprintf(stderr, "ls1: cannot open %s\n", dirname);
	else {
		while ((direntp = readdir(dir_ptr)) != NULL){
			if(strcmp(direntp->d_name, ".") == 0 ||
				strcmp(direntp->d_name, "..") == 0)
					continue;
			dostat(direntp->d_name);
		} 
		closedir(dir_ptr);
	}
}

void dostat(char* filename) {
	struct stat info;
	if (stat(filename, &info) == -1)
		printw("error");
	else
		show_file_info(filename, &info);
}

void show_file_info(char* filename, struct stat* info_p) {
	
	void mode_to_letters(int mode, char str[]);
	char modestr[] = "----------";

	mode_to_letters(info_p->st_mode, modestr);
	printf("%-18s ", filename);
	printf("%-17s", "local");
	printf("%-10ld", (long)info_p->st_size);
	printf("%-12s", modestr);
	printf("%-17s", 4+ctime(&info_p->st_mtime));
}

void mode_to_letters(int mode, char str[]){
	if(S_ISDIR(mode)) str[0] = 'd';
	if(S_ISCHR(mode)) str[0] = 'c';
	if(S_ISBLK(mode)) str[0] = 'b';

	if(mode & S_IRUSR) str[1] = 'r';
	if(mode & S_IWUSR) str[2] = 'w';
	if(mode & S_IXUSR) str[3] = 'x';
	if(mode & S_ISUID) str[3] = 's';

	if(mode & S_IRGRP) str[4] = 'r';
	if(mode & S_IWGRP) str[5] = 'w';
	if(mode & S_IXGRP) str[6] = 'x';
	if(mode & S_ISGID) str[6] = 's';

	if(mode & S_IROTH) str[7] = 'r';
	if(mode & S_IWOTH) str[8] = 'w';
	if(mode & S_IXOTH) str[9] = 'x';
	if(mode & S_ISVTX) str[9] = 't';
}


