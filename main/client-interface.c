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
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include "mysqldb.h"
#include "functions.h"

#define SELECTED_MENU 1
#define UNSELECTED_MENU 2
#define MENU 4

int port_num;
char ip_num[20];
char username[100];
int cur_r, cur_c;

struct winsize w;

int clnt_sock;

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
void flush_socket_buffer(int sock);

int main(int argc, char **argv) {
	
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	int row = w.ws_row, col = w.ws_col;

	struct sockaddr_in serv_addr;
	char message[1024] = {0x00, };

	signal(SIGWINCH, SIG_IGN);
	if (access("./download", 0) == -1)
		mkdir("download", 0755);

	clnt_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(clnt_sock == -1)
		error_handling("socket error");

	login();



	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(ip_num);
	serv_addr.sin_port = htons(port_num);

	if(connect(clnt_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect error");
	
	read(clnt_sock, id, sizeof(id));

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
					char msg[] = "3";
					int op = 0;
					//data_download(id, pw, filename);
					write(clnt_sock, msg, strlen(msg)+1);
					usleep(100000);
					write(clnt_sock, filename, strlen(filename)+1);

					DIR *dir_ptr;
					struct dirent *direntp;
					char path[256];

					if (chdir("./download") != 0) {
						printf("error: open download");
					}
					getcwd(path, sizeof(path));
					if ((dir_ptr = opendir(path)) == NULL)
						fprintf(stderr, "cannot open %s\n", path);
					else {
						while ((direntp = readdir(dir_ptr)) != NULL) {
							if (strcmp(direntp->d_name, ".") == 0 ||
								strcmp(direntp->d_name, "..") == 0)
								continue;
							if (strcmp(filename, direntp->d_name) == 0) {
								move(w.ws_row-1, 2);
								printw("Fail: same name exists!");
								op = -1;
								move(w.ws_row-1,0);
								for (int i = 0; i < w.ws_col; i++)
									printw(" ");
								break;
							}
						}
					}
					if (op == -1) {
						break;
					}

					chdir("./download");
					char buf[256];

					int nbyte = 256;
					size_t filesize = 0, bufsize = 0;
					FILE* file = NULL;
					file = fopen(filename, "wb");
					bufsize = 256;

					while(1) {
				        nbyte = read(clnt_sock, buf, sizeof(buf));
				        if (strcmp(buf, "sendend")==0){
				        	break;
				        }
				        fwrite(buf, sizeof(char), nbyte, file);
				    }
				    fclose(file);
				    chdir("..");


					move(w.ws_row -2, 2);
					for(int i = 0; i < w.ws_col; i++)
						printw(" ");
					break;
				}
			case 4:
				chdir("./download");
				load_file_name(upload_string, w);
				scanw("%s", filename);
				if (strcmp(filename, "b") == 0) {
					move(w.ws_row -2, 2);
					for(int i = 0; i < w.ws_col; i++)
						printw(" ");
					break;
				}
				else {
					//int op = data_upload(id, pw, filename);
					int op = 1;

					char buf[256];
					char msg[] = "4";
					write(clnt_sock, msg, strlen(msg)+1);
					usleep(100000);

					write(clnt_sock, filename, strlen(filename)+1);

					size_t nsize = 0, fsize;
					size_t fsize2;

			
		    		FILE* file = NULL;

		    		/* 전송할 파일 이름을 작성합니다 */
					if((file = fopen(filename, "rb")) == NULL){
						printf("file not exists\n");
					}

		    		/* 파일 크기 계산 */
				    // move file pointer to end
					fseek(file, 0, SEEK_END);
					// calculate file size
					fsize=ftell(file);	// move file pointer to first
					fseek(file, 0, SEEK_SET);

					// send file contents
					while (nsize != fsize) {
						//sleep(1);
						// read from file to buf
						// 1byte * 256 count = 256byte => buf[256];
						flush_socket_buffer(clnt_sock);
						int fpsize = fread(buf, 1, 256, file);
						nsize += fpsize;
						write(clnt_sock, buf, fpsize);
						printf("!\n");
						usleep(1000000);
					}
					char msgdone[] = "sendend";
					buf[0] = '\0';
					write(clnt_sock, msgdone, strlen(msgdone)+1);
					fclose(file);
					chdir("..");

					move(w.ws_row -2, 2);
					/*for(int i = 0; i < w.ws_col; i++)
						printw(" ");*/
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
				else{
					write(clnt_sock, filename, strlen(filename)+1);
				}
				//int op = data_delete(id, pw, filename);
				move(w.ws_row -2, 2);
					for(int i = 0; i < w.ws_col; i++)
						printw(" ");
				break;
		}
		//flush_socket_buffer(clnt_sock);
	}
	getch();
	endwin();

  	close(clnt_sock);


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
		printw("1. show cloud 2. show local 3. download 4. upload ctrl^C: quit");
	else {
		printw("1. show cloud 2. show local ");
		printw("3. download");
		printw(" 4. upload ctrl^C: quit");
	}
}

void login() {
	printf("Enter port number to open: ");
	scanf("%d", &port_num);
	printf("Enter ip number of server: ");
	scanf("%s", ip_num);
	printf("loading...");
}

void show_cloud(struct winsize w) {
	MYSQL data;
	MYSQL_RES* res;
	MYSQL_ROW row;
	int fields;
	int row_num = 5;
	int file_num = 0;

  	char msg[] = "1";
  	char buf[100][255][255];
  	char messages[255];
  	int str_len;
  	int i = 0;
  	int m = 0;
  	write(clnt_sock, msg, strlen(msg)+1);
  	str_len = read(clnt_sock, messages, sizeof(messages));
    // 서버에서 들어온 메세지 수신
    // str_len 이 -1 이라는 건, 서버 소켓과 연결이 끊어졌다는 뜻임
    // 왜 끊어졌는가? send_msg 에서 close(sock) 이 실행되고,
    // 서버로 EOF 가 갔으면, 서버는 그걸 받아서 "자기가 가진" 클라이언트 소켓을 close 할 것
    // 그러면 read 했을 때 결과가 -1 일 것.
    if (str_len == -1)
      // 종료를 위한 리턴값. thread_return 으로 갈 것
      return (void *)-1; // pthread_join를 실행시키기 위해
     
    if (strcmp(messages, "cloudlist") == 0){
   		//printf("loading...\n");
    	while(1){
    		str_len = read(clnt_sock, messages, sizeof(messages));
    		if (strcmp(messages, "listdone") == 0){
    			break;
    		}
    		sprintf(buf[i][m], "%s", messages);
    		m++;
    		if (m == 5){
    			m = 0;
    			i++;
    		}
    	}
    }
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
	//cloud_list(id, pw);

    // 버퍼 맨 마지막 값 NULL
    // 받은 메세지 출력
    for (int k = 0; k < i; k++){
    		file_num++;
    		move(row_num++, 2);
  			printw("%-18s%-16s%-10s%-14s%.17s", buf[k][4], buf[k][3], buf[k][2], buf[k][1], buf[k][0]);
			printw("\n");	
  	}
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

void *recv_msg(void *arg){
	MYSQL data;
	MYSQL_RES* res;
	MYSQL_ROW row;
	int fields;
	int row_num = 5;
	int file_num = 0;

	int sock = *((int *)arg);
  	char msg[100];
  	char buf[100][255][255];
  	char messages[255];
  	int str_len;
  	int i = 0;
  	int m = 0;
  	while (str_len = read(sock, messages, sizeof(messages)))
  	{
    // 서버에서 들어온 메세지 수신
    // str_len 이 -1 이라는 건, 서버 소켓과 연결이 끊어졌다는 뜻임
    // 왜 끊어졌는가? send_msg 에서 close(sock) 이 실행되고,
    // 서버로 EOF 가 갔으면, 서버는 그걸 받아서 "자기가 가진" 클라이언트 소켓을 close 할 것
    // 그러면 read 했을 때 결과가 -1 일 것.
    if (str_len == -1)
      // 종료를 위한 리턴값. thread_return 으로 갈 것
      return (void *)-1; // pthread_join를 실행시키기 위해
     
    if (strcmp(messages, "cloudlist") == 0){
   		//printf("loading...\n");
    	while(1){
    		str_len = read(sock, messages, sizeof(messages));
    		if (strcmp(messages, "listdone") == 0){
    			break;
    		}
    		sprintf(buf[i][m], "%s", messages);
    		m++;
    		if (m == 5){
    			m = 0;
    			i++;
    		}
    	}

    	for (int k = 0; k < i; k++){
    		file_num++;
    		move(row_num++, 2);
  			printw("%-18s%-16s%-10s%-14s%.17s", buf[k][4], buf[k][3], buf[k][2], buf[k][1], buf[k][0]);
			printw("\n");	
  		}
    }
    // 버퍼 맨 마지막 값 NULL
    // 받은 메세지 출력
	}
	return NULL;
}

void *send_msg(void *arg){
	// void형 int형으로 전환
  int sock = *((int *)arg);
  // 사용자 아이디와 메세지를 "붙여서" 한 번에 보낼 것이다
  char message[100];
 

	while (1)
  	{
    // 입력받음
    	scanf("%s",message);
    // Q 입력 시 종료
    	if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
    	{
      // 서버에 EOF 를 보냄
      	close(sock);
      	exit(0);
    	}
    // 서버로 메세지 보냄
    	write(sock, message, strlen(message)+1);
  	}
}

void error_handling(char* message){
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}


void flush_socket_buffer(int socket_fd) {
    int bytes_available;

    // 소켓의 읽기 버퍼에 있는 데이터의 바이트 수를 확인합니다.
    if (ioctl(socket_fd, FIONREAD, &bytes_available) < 0) {
        perror("ioctl");
        exit(1);
    }

    // 읽기 버퍼에 데이터가 있는 경우, 해당 데이터를 읽어서 버립니다.
    if (bytes_available > 0) {
        char buffer[bytes_available];
        if (read(socket_fd, buffer, bytes_available) < 0) {
            perror("read");
            exit(1);
        }
    }
}