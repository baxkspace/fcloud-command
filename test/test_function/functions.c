void data_upload(char* id, char* pw, char* filename){
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


	sprintf(query, "INSERT INTO filetable VALUES ('%s', NULL, NULL, NULL, NULL, LOAD_FILE('%s'))"
		,filename,file_location);
	if (mysql_query(&data, query) != 0) {
		fprintf(stderr, "Mysql query error : %s", mysql_error(&data));
	}
	mysql_close(&data);
} // done

void data_download(char* id, char* pw, char* filename){
	FILE* f = NULL;	
	MYSQL data;

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

}//done

void cloud_to_client_list(char* id, char* pw) {
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
	if (mysql_query(&data, "SELECT Time,Mode,Size,Uploader,Name FROM filetable")) {
		printf("%s\n", mysql_error(&data));
	}

	res = mysql_store_result(&data);
	fields = mysql_num_fields(res);

	while ((row = mysql_fetch_row(res))) {
		write(clnt_sock, row, sizeof(row));
	}
	mysql_free_result(res);
	mysql_close(&data);
}

void receive_row(){
	MYSQL_ROW rowlist[100];
	MYSQL_ROW row;
	int i = 0;
	while (1) {
		int str_len = read(clnt_sock, row, sizeof(row));
		rowlist[i++] = row;
		if (str_len != -1)
			break;
	}
}