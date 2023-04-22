/*
* Computer Science and Engineering 2020113766 Park Yoonjin
* Network Programming assignment #2
* hw2_client.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define BUF_SIZE 20

// result
#define ERROR 0
#define SUCCESS 1

// cmd
#define REQUEST 0
#define RESPONSE 1
#define QUIT 2

typedef struct {
    int cmd;                // 0: request, 1: response, 2: quit
    char addr[BUF_SIZE];    // dotted-decimal address(20 bytes)
    struct in_addr iaddr;   // inet_aton() address
    int result;             // 0: error, 1: success
}PACKET;

void error_handling(char *message);

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    PACKET send_packet, recv_packet;
    char input[BUF_SIZE];
    int str_len;

    if (argc != 3){
        printf("Usage : ./hw2_client <IP> <port>\n");
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error!");

    while(1) {
        printf("Input dotted-decimal address: ");
        scanf("%s", input);

        if (strcmp(input, "quit") == 0){                // when input = quit
            send_packet.cmd = QUIT;
            write(sock, &send_packet, sizeof(PACKET));
            printf("[Tx] cmd: %d(QUIT)\n", send_packet.cmd);
            printf("Client socket close and exit\n");
            break;
        }
        send_packet.cmd = REQUEST;
        strcpy(send_packet.addr, input);
        write(sock, &send_packet, sizeof(PACKET));
        printf("[Tx] cmd: %d, addr: %s\n", send_packet.cmd, send_packet.addr);
        
        str_len = read(sock, &recv_packet, sizeof(PACKET));

        if (str_len == -1)
            error_handling("read() error!");
        
        if (recv_packet.result == 1)
            printf("[Rx] cmd: %d, Address conversion: %#x (result: %d)\n\n", recv_packet.cmd, recv_packet.iaddr.s_addr, recv_packet.result);
        
        else
            printf("[Rx] cmd: %d, Address conversion fail! (result: %d)\n\n", recv_packet.cmd, recv_packet.result);
    }

    close(sock);

    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}