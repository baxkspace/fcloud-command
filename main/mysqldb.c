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
	if (mysql_real_connect(conn, "localhost", id, pw, NULL, 0, NULL, 0) == NULL) {
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

void cloud_list(char* id, char* pw) {
	MYSQL data;
	MYSQL_RES* res;
	MYSQL_ROW row;
	int fields;

	mysql_init(&data);
	mysql_real_connect(&data, "localhost", id, pw, NULL, 0, NULL, 0);
	printf("1");
	if (mysql_query(&data, "USE fcloud")) {
		printf("%s\n", mysql_error(&data));
	}
	if (mysql_query(&data, "SELECT * FROM filetable")) {
		printf("%s\n", mysql_error(&data));
	}
	printf("2");

	res = mysql_store_result(&data);
	fields = mysql_num_fields(res);

	while ((row = mysql_fetch_row(res))) {
		for (int cnt = 0; cnt < fields; ++cnt) {
			printw("%s\n", row[cnt]);
		}
	}
	mysql_free_result(res);
	mysql_close(&data);
}