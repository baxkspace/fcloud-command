#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>

void finish_error(MYSQL *conn) {
	fprintf(stderr, "%s\n", mysql_error(conn));
	mysql_close(conn);
	exit(1);
}

int main(int argc, char *argv[]) {
	MYSQL *conn = mysql_init(NULL);

	if(conn == NULL) {
		fprintf(stderr, "%s", mysql_error(conn));
		exit(1);
	}
	if (mysql_real_connect(conn, "localhost", "root", "rootroot", NULL, 0, NULL, 0) == NULL) {
		finish_error(conn);
	}
	if ((mysql_query(conn, "CREATE DATABASE IF NOT EXISTS fcloud"))) {
		finish_error(conn);
	}
	if ((mysql_query(conn, "USE fcloud"))) {
		finish_error(conn);
	}
	if ((mysql_query(conn, "CREATE TABLE IF NOT EXISTS filetable (Name VARCHAR(30), Uploader VARCHAR(30) default 'Anonymous', Size INT, Mode CHAR(10), Time DATETIME default current_timestamp)"))) {
		finish_error(conn);
	}
	mysql_close(conn);

	exit(0);
}