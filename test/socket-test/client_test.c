#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
void error_handling(char* message);

int main(int argc, char* argv[]){
	int clnt_sock;
	struct sockaddr_in serv_addr;
	char message[1024] = {0x00, };

	clnt_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(clnt_sock == -1)
		error_handling("socket error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if(connect(clnt_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect error");

	if(read(clnt_sock, message, sizeof(message)-1) == -1)
		error_handling("read error");
	printf("Message from server :%s\n",message);

	char msge[100];

	scanf("%s",msge);


	//write(clnt_sock, msge, strlen(msge));

	size_t nsize = 0, fsize;
	size_t fsize2;

	char buf[256];
    FILE* file = NULL;
    /* 전송할 파일 이름을 작성합니다 */
	if((file = fopen(msge, "rb")) == NULL){
		printf("file not exists\n");
	}
    /* 파일 크기 계산 */
    // move file pointer to end
	fseek(file, 0, SEEK_END);
	// calculate file size
	fsize=ftell(file);
	printf("%ld", fsize);
	// move file pointer to first
	fseek(file, 0, SEEK_SET);
	
	// send file size first
	 fsize2 = htonl(fsize);
	// send file size
	 send(clnt_sock, &fsize2, sizeof(fsize), 0);

	// send file contents
	while (nsize != fsize) {
		// read from file to buf
		// 1byte * 256 count = 256byte => buf[256];
		int fpsize = fread(buf, 1, 256, file);
		nsize += fpsize;
		send(clnt_sock, buf, fpsize, 0);
		printf("success\n");
	}
	fclose(file);

	close(clnt_sock);
	return 0;

}

void error_handling(char* message){
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}