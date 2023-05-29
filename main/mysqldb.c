#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <curses.h>
#include "mysqldb.h"

void finish_error(MYSQL *conn) {
	fprintf(stderr, "%s\n", mysql_error(conn));
	mysql_close(conn);
	exit(1);
}

void mysqlConnect(char* id, char* pw) {
	MYSQL *conn = mysql_init(NULL);
	if(conn == NULL) {
		fprintf(stderr, "%s", mysql_error(conn));
		exit(1);
	}
	if (mysql_real_connect(conn, NULL, id, pw, NULL, 0, NULL, 0) == NULL) {
		finish_error(conn);
	}
	if ((mysql_query(conn, "CREATE DATABASE IF NOT EXISTS fcloud"))) {
		finish_error(conn);
	}
	if ((mysql_query(conn, "USE fcloud"))) {
		finish_error(conn);
	}
	if ((mysql_query(conn, "CREATE TABLE IF NOT EXISTS filetable (Name VARCHAR(30), Uploader VARCHAR(30) default 'Anonymous', Size INT, Mode CHAR(10), Time DATETIME default current_timestamp, Contents LONGBLOB)"))) {
		finish_error(conn);
	}
}

int cloud_list(char* id, char* pw) {
	MYSQL data;
	MYSQL_RES* res;
	MYSQL_ROW row;
	int fields;
	int row_num = 5;
	int file_num = 0;

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

	while ((row = mysql_fetch_row(res))) {
		file_num++;
		move(row_num++, 2);
		printw("%-18s%-16s%-10s%-14s%.17s", row[4], row[3], row[2], row[1], row[0]);
		printw("\n");
	}
	mysql_free_result(res);
	mysql_close(&data);
	return file_num;
}