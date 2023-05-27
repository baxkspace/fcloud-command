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
#include <time.h>
#include <mysql.h>
#include <unistd.h>
#include "mysqldb.h"
#include "functions.h"

#define SELECTED_MENU 1
#define UNSELECTED_MENU 2
#define MENU 4

int port_num;
int cur_r, cur_c;
void print_welcome(char* name);
void menu_number(struct winsize w, int num);
char id[30], pw[100];
char download_string[] = "file name to download(enter 'b' to back) >> ";
char upload_string[] = "file name to upload(enter 'b' to back) >> ";
char delete_string[] = "file name to delete(enter 'b' to back) >> ";
void login();
void make_blank(struct winsize w);

void show_cloud(struct winsize w);
void show_local(struct winsize w);
void load_file_name(char* input, struct winsize w);
int enter_menu(struct winsize w);

void local_list(char dirname[]);
void dostat(char* filename);
void show_file_info(char* filename, struct stat* info_p);
void mode_to_letters(int mode, char str[]);

int main(int argc, char **argv) {
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	int row = w.ws_row, col = w.ws_col;

	signal(SIGWINCH, SIG_IGN);
	if (access("./download", 0) == -1)
		mkdir("download", 0755);

	login();

	initscr();
	crmode();
	start_color();

	// color set
	init_pair(SELECTED_MENU, COLOR_WHITE, COLOR_MAGENTA);
	init_pair(UNSELECTED_MENU, COLOR_WHITE, COLOR_CYAN);
	init_pair(MENU, COLOR_BLACK, COLOR_MAGENTA);

	print_welcome(id);

	show_cloud(w);
	int num = 0;
	char character;
	char filename[100];

	while (1) {
		num = enter_menu(w);
		menu_number(w, num);

		switch(num) {
			case 1:
				show_cloud(w);
				break;
			case 2:
				show_local(w);
				break;
			case 3:
				load_file_name(download_string, w);
				scanw("%s", filename);
				if (strcmp(filename, "b") == 0) {
					move(w.ws_row -2, 2);
					for(int i = 0; i < w.ws_col; i++)
						printw(" ");
					break;
				}
				else {
					data_download(id, pw, filename);
					move(w.ws_row -2, 2);
					for(int i = 0; i < w.ws_col; i++)
						printw(" ");
					break;
				}
			case 4: 
				load_file_name(upload_string, w);
				scanw("%s", filename);
				if (strcmp(filename, "b") == 0) {
					move(w.ws_row -2, 2);
					for(int i = 0; i < w.ws_col; i++)
						printw(" ");
					break;
				}
				else {
					int op = data_upload(id, pw, filename);
					move(w.ws_row -2, 2);
					for(int i = 0; i < w.ws_col; i++)
						printw(" ");
					init_pair(223, COLOR_CYAN, COLOR_BLACK);
					attron(COLOR_PAIR(223));
					if (op == 0) {
						move(w.ws_row-2, 2);
						printw("upload success!");
					}
					else if (op == 1) {
						printw("Fail - There is not %s in local!", filename);
					}
					sleep(1);
					attroff(COLOR_PAIR(223));
					move(w.ws_row -2, 2);
					for(int i = 0; i < w.ws_col; i++)
						printw(" ");
					break;
				}
			case 5:
				load_file_name(delete_string, w);
				scanw("%s", filename);
				if (strcmp(filename, "b") == 0) {
					move(w.ws_row -2, 2);
					for(int i = 0; i < w.ws_col; i++)
						printw(" ");
					break;
				}
				int op = data_delete(id, pw, filename);
				move(w.ws_row -2, 2);
					for(int i = 0; i < w.ws_col; i++)
						printw(" ");
				break;
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
	printw(" << (port number: %d)", port_num);
	attroff(A_BOLD);
}

void menu_number(struct winsize w, int num) {
	move(w.ws_row - 4, 2);
	init_pair(5, COLOR_BLACK, COLOR_WHITE);
	attron(COLOR_PAIR(5));
	printw("< list of menu >");
	attroff(COLOR_PAIR(5));
	move(w.ws_row - 3, 2);
	if (num == 0)
		printw("1. show cloud 2. show local 3. download 4. upload 5. delete ctrl^C: quit");
	else {
		printw("1. show cloud 2. show local ");
		printw("3. download");
		printw(" 4. upload 5. delete ctrl^C: quit");
	}
}

void login() {
	printf("Enter your mysql ID: ");
	scanf("%s", id);
	printf("Enter password: ");
	scanf("%s", pw);
	mysqlConnect(id, pw);
	printf("Enter port number to open: ");
	scanf("%d", &port_num);
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
	printw("%-18s %-10s     %-10s %-13s %-17s", "file name", "uploader", "size", "mode", "time");

	move(w.ws_row - 5, 0);
	for (int i =0; i < w.ws_col; i++) {
		printw(" ");
	}
	attroff(COLOR_PAIR(MENU));

	move(5, 2);
	make_blank(w);
	cloud_list(id, pw);

}

void show_local(struct winsize w) {
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
	printw("%-18s %-10s     %-10s %-13s %-17s", "file name", "uploader", "size", "mode", "time");

	move(w.ws_row - 5, 0);
	for (int i =0; i < w.ws_col; i++) {
		printw(" ");
	}
	attroff(COLOR_PAIR(MENU));

	move(5, 0);
	make_blank(w);
	local_list("download");
}

void local_list(char dirname[]) {

	DIR *dir_ptr;
	struct dirent *direntp;
	char path[256];

	chdir(dirname);

	getcwd(path, sizeof(path));
	int cur_line = 5;

	if ((dir_ptr = opendir(path)) == NULL)
		fprintf(stderr, "ls1: cannot open %s\n", dirname);
	else {
		while ((direntp = readdir(dir_ptr)) != NULL){
			if(strcmp(direntp->d_name, ".") == 0 ||
				strcmp(direntp->d_name, "..") == 0)
					continue;
			move(cur_line++, 2);
			dostat(direntp->d_name);
		} 
		closedir(dir_ptr);
	}
}

void dostat(char* filename) {
	struct stat info;
	if (stat(filename, &info) == -1)
		printw("error");
	else {
		show_file_info(filename, &info);
	}
}

void show_file_info(char* filename, struct stat* info_p) {
	char modestr[] = "----------";
	void mode_to_letters();
	mode_to_letters(info_p->st_mode, modestr);
	printw("%-18s ", filename);

	printw("%-16s", "local");
	printw("%-10ld", (long)info_p->st_size);
	printw("%-14s", modestr);
	printw("%.17s", 4+ctime(&info_p->st_mtime));
}

void mode_to_letters(int mode, char str[]) {
	if (S_ISDIR(mode)) str[0] = 'd';
	if (S_ISCHR(mode)) str[0] = 'c';
	if (S_ISBLK(mode)) str[0] = 'b';

	if(mode & S_IRUSR) str[1] = 'r';
	if(mode & S_IWUSR) str[2] = 'w';
	if(mode & S_IXUSR) str[3] = 'x';

	if(mode & S_IRGRP) str[4] = 'r';
	if(mode & S_IWGRP) str[5] = 'w';
	if(mode & S_IXGRP) str[6] = 'x';

	if(mode & S_IROTH) str[7] = 'r';
	if(mode & S_IWOTH) str[8] = 'w';
	if(mode & S_IXOTH) str[9] = 'x';
}

void load_file_name(char* input, struct winsize w) {
	move(w.ws_row -2, 2);
	init_pair(123, COLOR_CYAN, COLOR_BLACK);
	attron(COLOR_PAIR(123));
	printw(input);
	attroff(COLOR_PAIR(123));	

	char filename[100];
	move(w.ws_row-2, strlen(input) + 2);
}

int enter_menu(struct winsize w) {
	int num;
	move(w.ws_row -2, 2);
	char menu_select_string[100] = "enter menu num >> ";
	init_pair(123, COLOR_CYAN, COLOR_BLACK);
	attron(COLOR_PAIR(123));
	printw(menu_select_string);
	attroff(COLOR_PAIR(123));
	menu_number(w, 0);
	move(w.ws_row-2, strlen(menu_select_string) + 2);
	printw("            ");
	move(w.ws_row-2, strlen(menu_select_string) + 2);
	scanw("%d", &num);
	return num;
}

void make_blank(struct winsize w) {
	for (int i = 5; i < w.ws_row - 6; i++) {
		for (int j = 0; j < w.ws_col; j++) 
		{
			move(i, j);
			printw(" ");
		}
	}
}