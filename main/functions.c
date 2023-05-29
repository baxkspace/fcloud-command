#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <curses.h>
#include "functions.h"

int data_upload(char* id, char* pw, char* filename){
	MYSQL data;

	mysql_init(&data);
	mysql_real_connect(&data, "localhost", id, pw, NULL, 0, NULL, 0);

	if (mysql_query(&data, "USE fcloud")) {
		printf("%s\n", mysql_error(&data));
	}
	
	char query[255];
	char file_location[100];
	chdir("./download");
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
		return 1;
	}
	mysql_close(&data);
	chdir("..");
	return 0;
} 

int data_download(char* id, char* pw, char* filename){
	FILE* f = NULL;	
	MYSQL data;

	DIR *dir_ptr;
	struct dirent *direntp;
	char path[256];

	chdir("./download");
	getcwd(path, sizeof(path));
	if ((dir_ptr = opendir(path)) == NULL)
		fprintf(stderr, "cannot open %s\n", path);
	else {
		while ((direntp = readdir(dir_ptr)) != NULL) {
			if (strcmp(direntp->d_name, ".") == 0 ||
				strcmp(direntp->d_name, "..") == 0)
				continue;
			if (strcmp(filename, direntp->d_name) == 0) {
				return -1;
			}
		}
	}
	closedir(dir_ptr);

	mysql_init(&data);
	mysql_real_connect(&data, "localhost", id, pw, NULL, 0, NULL, 0);

	if (mysql_query(&data, "USE fcloud")) {
		printf("%s\n", mysql_error(&data));
	}
	chdir("./download");

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
	chdir("..");
	return 0;

}//done

int data_delete(char* id, char* pw, char* filename) {
	MYSQL data;

	mysql_init(&data);
	mysql_real_connect(&data, "localhost", id, pw, NULL, 0, NULL, 0);

	if (mysql_query(&data, "USE fcloud")) {
		printf("%s\n", mysql_error(&data));
	}
	char query[200];
	sprintf(query, "DELETE FROM filetable WHERE Name='%s'", filename);
	if (mysql_query(&data, query) != 0) {
		return 1;
	}
	return 0;
}

/*void cloud_to_client_list(char* id, char* pw) {
	MYSQL data;
	MYSQL_RES* res;
	MYSQL_ROW row;
	int fields;
	int row_num = 5;

	mysql_init(&data);
	mysql_real_connect(&data, "localhost", id, pw, NULL, 0, NULL, 0);
	
	if (mysql_query(&data, "USE fcloud")) {
		printf("%s\n", mysql_error(&data));
	}
	if (mysql_query(&data, "SELECT Time,Mode,Size,Uploader,Name,Contents FROM filetable")) {
		printf("%s\n", mysql_error(&data));
	}

	res = mysql_store_result(&data);
	fields = mysql_num_fields(res);

	while ((row = mysql_fetch_row(res))) {
		write(clnt_sock, row, sizeof(row));
	}
	mysql_free_result(res);
	mysql_close(&data);
}*/

/*void receive_row(){
	MYSQL_ROW rowlist[100];
	MYSQL_ROW row;
	int i = 0;
	while (1) {
		int str_len = read(clnt_sock, row, sizeof(row));
		rowlist[i++] = row;
		if (str_len != -1)
			break;
	}
}*/