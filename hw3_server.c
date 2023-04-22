/*
* 컴퓨터학부 2020113766 박윤진
* Network Programming hw3_server.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 100
#define SEQ_START 1000

typedef	struct	{
    int seq;                // SEQ num
    int ack;                // ACK num
    int buf_len;            // file read/write bytes
    char buf[BUF_SIZE];     // transmit file name or content
} Packet;

void error_handling(char *message);

int main(int argc, char *argv[]) {
	int serv_sock, clnt_sock;
	FILE * fp;
	char buf[BUF_SIZE];
	char file_name[BUF_SIZE];
	int read_cnt;
    int total_byte = 0;

    char error_msg[30] = "File Not Found\n";
	
	struct sockaddr_in serv_addr, clnt_addr;
	socklen_t clnt_addr_size;
	
    Packet send_packet, recv_packet;

	if(argc != 2) {             // check argument
		printf("Usage: %s <port>\n", argv[0]);
		exit(1);
	}
	
    // create socket
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);   
	if (serv_sock == -1)
        error_handling("socket() error");
	
    memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));
	
    // bind
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
    
    // listen
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");
	
	clnt_addr_size = sizeof(clnt_addr);
    
    // accept
	clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
	if (clnt_sock == -1)
        error_handling("accept error");

    printf("-----------------------------\n");
    printf(" Address Conversion Server\n");
    printf("-----------------------------\n");

    // receive file name
    read(clnt_sock, &recv_packet, sizeof(Packet));
    strcpy(file_name, recv_packet.buf);

    // open file
	fp = fopen(recv_packet.buf, "r");
    send_packet.seq = SEQ_START;
    send_packet.ack = SEQ_START;

	if (fp == NULL) {
        // no file -> quit
        send_packet.buf_len = 0;
        strcpy(send_packet.buf, error_msg);
        write(clnt_sock, &send_packet, sizeof(Packet));
        printf("%s %s", recv_packet.buf, error_msg);

        return 0;
	}
    else {
        // send file
        printf("[Server] sending %s\n\n",recv_packet.buf);

		while(1) {
            send_packet.buf_len = fread(send_packet.buf, 1, BUF_SIZE, fp); // 실제로 읽는 크기 = 1 * BUF_SIZE

            printf("[Server] Tx SEQ: %d, %d byte data\n", send_packet.seq, send_packet.buf_len); // 디버깅 용도로 출력
            write(clnt_sock, &send_packet, sizeof(Packet));

            // receive ack
            int ack = read(clnt_sock, &recv_packet, sizeof(Packet));
            send_packet.seq = recv_packet.ack;
            
            total_byte += send_packet.buf_len; // 총 바이트 수 누적
            // memset(&send_packet, 0, sizeof(Packet))
            // 쓰레기값이 들어갈 수 있기 때문에 초기화 필요함

			if (send_packet.buf_len < BUF_SIZE) {
				// last packet
				break;
			}

            printf("[Server] Rx ACK: %d\n\n", recv_packet.ack);
		}
    }
	
    printf("%s sent (%d Bytes)\n", file_name, total_byte);

	fclose(fp);
	close(clnt_sock);
    close(serv_sock);

	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
