/*
* 컴퓨터학부 2020113766 박윤진
* Network Programming hw3_client.c
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

int main(int argc, char *argv[])
{
	int sock;
	FILE *fp;
	
	char buf[BUF_SIZE];
	char file_name[BUF_SIZE];
	int read_cnt;
	struct sockaddr_in serv_addr;
    int total_byte = 0;

    char error_msg[30] = "File Not Found\n";

    Packet send_packet, recv_packet;

	if (argc != 3) {
		printf("Usage: %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	printf("Input file name: ");
	scanf("%s", file_name);

    // create socket
	sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error!");
    
    printf("[Client] request %s\n\n", file_name);

    // send file name
    send_packet.buf_len = strlen(file_name);
    strcpy(send_packet.buf, file_name);
	write(sock, &send_packet, sizeof(Packet));

    // check file's existence
    int exist = read(sock, &recv_packet, sizeof(Packet));
    
    if (exist == -1)
        error_handling("read() error");
    
    if (recv_packet.buf_len == 0) {
        // no file
        printf("%s", error_msg);
        close(sock);
        return 0;
    }

    // file open
    fp = fopen(file_name, "w");
    if (fp == NULL)
        error_handling("fopen() error");
    
    printf("\n\n");

    // receive file
	while (exist != 0) {
        fwrite(recv_packet.buf, 1, recv_packet.buf_len, fp);
        printf("[Client] Rx SEQ: %d, len: %d bytes\n", recv_packet.seq, recv_packet.buf_len);
        total_byte += recv_packet.buf_len;
        
        // send ack
        send_packet.ack = recv_packet.seq + recv_packet.buf_len;
        write(sock, &send_packet, sizeof(Packet));


        if (recv_packet.buf_len < BUF_SIZE) {
            // last packet
            break;
        }
        printf("[Client] Tx ACK: %d\n\n", send_packet.ack);

        exist = read(sock, &recv_packet, sizeof(Packet));
    }
	
    printf("%s received (%d Bytes)\n", file_name, total_byte);

	fclose(fp);
	close(sock);

	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
