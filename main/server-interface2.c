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
#include <pthread.h>
#include "mysqldb.h"
#include "functions.h"

#define SELECTED_MENU 1
#define UNSELECTED_MENU 2
#define MENU 4
#define BUF_SIZE 100 // 최대 글자 수
#define MAX_CLNT 256 // 최대 동시 접속 가능 수

void *handle_clnt(void *arg);
void *recv_msg(void *arg);
void error_handling(char *msg);


//socket add

// 접속한 클라이언트 수
int clnt_cnt = 0;
// 여러 명의 클라이언트가 접속하므로, 클라이언트 소켓은 배열이다.
// 멀티쓰레드 시, 이 두 전역변수, clnt_cnt, clnt_socks 에 여러 쓰레드가 동시 접근할 수 있기에 
// 두 변수의 사용이 들어간다면 무조건 임계영역이다.
int clnt_socks[MAX_CLNT]; // 클라이언트 최대 256명
pthread_mutex_t mutx; // mutex 선언 - 다중 스레드끼리 전역변수 사용시 데이터의 혼선 방지

//socket add

int port_num;
int cur_r, cur_c;
int cloud_file_num;
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

	int serv_sock;
  	struct sockaddr_in serv_adr;
  	pthread_t t_id; // thread 선언
  	pthread_t rcv_thread;
  	void *thread_return;


  	// 소켓 옵션 설정을 위한 두 변수
  	int option;
  	socklen_t optlen;


  	// 뮤텍스 만들기
  	pthread_mutex_init(&mutx, NULL);
  	serv_sock = socket(PF_INET, SOCK_STREAM, 0);

  	// Time-wait 해제
 	 // SO_REUSEADDR 를 0에서 1로 변경
  	optlen = sizeof(option);
  	option = 1;
  	setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void *)&option, optlen);

  	//socket add

	signal(SIGWINCH, SIG_IGN);
	if (access("./download", 0) == -1)
		mkdir("download", 0755);
	if (access("./client_download", 0) == -1)
		mkdir("client_download", 0755);
	login();
	// IPv4 , IP, Port 할당
	memset(&serv_adr, 0, sizeof(serv_adr));
  	serv_adr.sin_family = AF_INET;
  	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	serv_adr.sin_port = htons(port_num);

	// 주소 할당
  	if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
    	error_handling("bind() error");

  	// 총 256명까지 접속 가능한 것.
  	// 웹서버같이 수천명의 클라이언트로 바쁠 경우, 15로 잡는 경우가 보통임
  	if (listen(serv_sock, 5) == -1)
    	error_handling("listen() error");
  	// recv_thread
 	pthread_create(&rcv_thread, NULL, handle_clnt, (void *)&serv_sock);

  	//socket add

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
				printw("%d", cloud_file_num);
				if (strcmp(filename, "b") == 0) {
					move(w.ws_row -2, 2);
					for(int i = 0; i < w.ws_col; i++)
						printw(" ");
					break;
				}
				else if (cloud_file_num >= 14) {
					move(w.ws_row - 1, 2);
					printw("cloud is full!");
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
				
					printw("%d", op);
					if (op == 0) {
						move(w.ws_row-1, 2);
						addstr("upload success!");
					}
					else if (op == 1) {
						sleep(1);
						addstr("Fail - There is not %s in local!");
					}
					
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
	// 쓰레드 종료 대기 및 소멸 유도
  	pthread_join(rcv_thread, &thread_return);
  	close(serv_sock);

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
	cloud_file_num = cloud_list(id, pw);

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
	for (int i = 5; i < w.ws_row - 5; i++) {
		for (int j = 0; j < w.ws_col; j++) 
		{
			move(i, j);
			printw(" ");
		}
	}
}

//socket add
void *handle_clnt(void *arg)
{
  	int sock = *((int *)arg);
  	int clnt_sock;
  	struct sockaddr_in clnt_adr;
  	int clnt_adr_sz;
  	pthread_t t_id; // thread 선언

  	while (1)
 	{
    	clnt_adr_sz = sizeof(clnt_adr);
    	clnt_sock = accept(sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);


    	// clnt_socks[], clnt_cnt 전역변수를 사용하기 위해 뮤텍스 잠금
    	pthread_mutex_lock(&mutx);
    	// 클라이언트 카운터 올리고, 소켓 배정 . 첫 번째 클라이언트라면, clnt_socks[0] 에 들어갈 것
    	clnt_socks[clnt_cnt++] = clnt_sock;
    	// 뮤텍스 잠금해제
    	pthread_mutex_unlock(&mutx);
    	// 쓰레드 생성. 쓰레드의 main 은 handle_clnt
    	// 네 번째 파라미터로 accept 이후 생겨난 소켓의 파일 디스크립터 주소값을 넣어주어
    	// handle_clnt 에서 파라미터로 받을 수 있도록 함
    	pthread_create(&t_id, NULL, recv_msg, (void *)&clnt_sock);
    	// 이걸 호출했다고 해서 끝나지도 않은 쓰레드가 종료되진 않음
    	// 즉, t_id 로 접근했을 때, 해당 쓰레드가 NULL 값을 리턴한 경우가 아니라면 무시하고 진행됨
    	// 만약 해당 쓰레드가 NULL 값을 리턴했다면, 쓰레드 종료
    	pthread_detach(t_id);
  	}
  	close(sock);

}

void *recv_msg(void *arg){
  	struct sockaddr_in clnt_adr;
  	int clnt_adr_sz;
  	char msg[100];

  	int str_len;
  	// 소켓 파일 디스크립터가 void 포인터로 들어왔으므로, int 로 형변환
  	int clnt_sock = *((int *)arg);
  // 클라이언트에서 보낸 메세지 받음.
  // 클라이언트에서 EOF 보내서, str_len 이 0이 될때까지 반복
  // EOF 를 보내는 순간은 언제인가? 클라이언트에서, 소켓을 close 했을 때이다!
  // 즉, 클라이언트가 접속을 하고 있는 동안에, while 문을 벗어나진 않는다.
  	while ((str_len = read(clnt_sock, msg, sizeof(msg))) != 0){

    	if (strcmp(msg, "1") == 0){
    		//write(clnt_sock, mmm, strlen(mmm) + 1);
    		MYSQL data;
			MYSQL_RES* res;
			MYSQL_ROW row;

			int fields;
			int row_num = 5;
			int i = 0;

			mysql_init(&data);
			mysql_real_connect(&data, "localhost", id, pw, NULL, 0, NULL, 0);

			if (mysql_query(&data, "USE fcloud")) {
				printf("%s\n", mysql_error(&data));
			}
			if (mysql_query(&data, "SELECT Time,Mode,Size,Uploader,Name FROM filetable")) {
				printf("%s\n", mysql_error(&data));
			}

			res = mysql_store_result(&data);
			fields = mysql_num_fields(res);
			char message[100] = "cloudlist";
			char message2[100] = "listdone";
			write(clnt_sock, message, strlen(message)+1);
			
        	char buf[255];
        	int m = 0;

			while ((row = mysql_fetch_row(res))) {
				for (int j = 0; j < 5; j++){
					sprintf(buf, "%s", row[j]);

					usleep(100000);
					write(clnt_sock, buf, strlen(buf)+1);
					buf[0] = '\0';
					
				}
			}
			usleep(100000);
			write(clnt_sock, message2, strlen(message2)+1);
			mysql_free_result(res);
			mysql_close(&data);
    	}
    	else if (strcmp(msg, "3") == 0){
    		FILE* f = NULL;	
			MYSQL data;

			mysql_init(&data);
			mysql_real_connect(&data, "localhost", id, pw, NULL, 0, NULL, 0);

			if (mysql_query(&data, "USE fcloud")) {
				printf("%s\n", mysql_error(&data));
			}
			chdir("./client_download");

			char filename[256];
			read(clnt_sock, filename, sizeof(filename));
			printf("filename : %s\n",filename);

			MYSQL_RES* res;
			MYSQL_ROW row;
			char file_location[100];
			getcwd(file_location, 100);
			sprintf(file_location, "%s/%s",file_location, filename);	
			if (mysql_query(&data, "SELECT Time,Mode,Size,Uploader,Name,Contents FROM filetable")) {
				printf("%s\n", mysql_error(&data));
			}
			f = fopen(file_location, "wb");
	
			res = mysql_store_result(&data);
			while ((row = mysql_fetch_row(res))) {
				if(strcmp(row[4], filename) == 0){
					fwrite(row[5], sizeof(row[5]), 1, f);
					fclose(f);
					break;
				}
			}
			close(f);

			
			char buf[256];

			//write(clnt_sock, filename, strlen(filename)+1);

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
			
			// send file size first
			 //fsize2 = htonl(fsize);
			// send file size
			 //send(clnt_sock, &fsize2, sizeof(fsize), 0);

			// send file contents
			while (nsize != fsize) {
				// read from file to buf
				// 1byte * 256 count = 256byte => buf[256];
				int fpsize = fread(buf, 1, 256, file);
				nsize += fpsize;
				send(clnt_sock, buf, fpsize + 1, 0);
				printf("nsize : %d fsize : %d\n",nsize, fsize);
			}
			printf("1123123\n");
			fclose(file);
			//unlink(filename);
			chdir("..");
    	}
    	else if (strcmp(msg, "4") == 0){
    		chdir("./client_download");
    		char filename[256];
			char buf[256];

			int str_len = read(clnt_sock, filename, sizeof(filename));
			if (str_len != -1) {
				break;
			}
			int nbyte = 256;
			size_t filesize = 0, bufsize = 0;
			FILE* file = NULL;
			file = fopen(filename, "wb");
			bufsize = 256;

			while(nbyte != 0) {
		        nbyte = recv(clnt_sock, buf, bufsize, 0);
		        fwrite(buf, sizeof(char), nbyte, file);		
		    }
		    fclose(file);

		    MYSQL data;

			mysql_init(&data);
			mysql_real_connect(&data, "localhost", id, pw, NULL, 0, NULL, 0);

			if (mysql_query(&data, "USE fcloud")) {
				printf("%s\n", mysql_error(&data));
			}
			
			char query[255];
			char file_location[100];
			chdir("./client_download");
			getcwd(file_location, 100);
			sprintf(file_location, "%s/%s",file_location, filename);

			struct stat info;
			stat(filename, &info);
			int mode = info.st_mode;
			char str[] = "----------";
			long size = (long)info.st_size;
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

			sprintf(query, "INSERT INTO filetable(Name, Uploader, Size, Mode, Contents) VALUES ('%s', '%s', '%ld','%s', LOAD_FILE('%s'))"
				,filename,id,size,str,file_location);
			if (mysql_query(&data, query) != 0) {
				printf("uploadError\n");
			}
			mysql_close(&data);
			unlink(filename);
			chdir("..");
    	}
    	else if (strcmp(msg, "5") == 0){
    		char filename[100];
    		read(clnt_sock, filename, sizeof(filename));
    		data_delete(id, pw, filename);
    	}
  	}

  	// while 문 탈출했다는 건, 현재 담당하는 소켓의 연결이 끊어졌다는 뜻임.
  	// 그러면 당연히, clnt_socks[] 에서 삭제하고, 쓰레드도 소멸시켜야.

  	// 전역변수 clnt_cnt 와 clnt_socks[] 를 건드릴 것이기에, 뮤텍스 잠금
  	pthread_mutex_lock(&mutx);
  	// 연결 끊어진 클라이언트인 "현재 쓰레드에서 담당하는 소켓" 삭제
  	for (int i = 0; i < clnt_cnt; i++)
  	{
    	// 현재 담당하는 클라이언트 소켓의 파일 디스크립터 위치를 찾으면,
    	if (clnt_sock == clnt_socks[i])
    	{
      	// 현재 소켓이 원래 위치했던 곳을 기준으로
      	// 뒤의 클라이언트들을 땡겨옴
      	while (i++ < clnt_cnt - 1) // 쓰레드 1개 삭제할 것이기 때문에 -1 해줘야 함
        	clnt_socks[i] = clnt_socks[i + 1];
      	break;
    	}
  	}
  	// 클라이언트 수 하나 줄임
  	clnt_cnt--;
  	// 뮤텍스 잠금해제
  	pthread_mutex_unlock(&mutx);
  	// 서버의 쓰레드에서 담당하는 클라이언트 소켓 종료
  	close(clnt_sock);

  	return NULL;
}

void error_handling(char *msg)
{
  fputs(msg, stderr);
  fputc('\n', stderr);
  exit(1);
}

// socket add