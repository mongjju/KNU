/*
* Computer Science and Engineering 2020113766 Park Yoonjin
* Network Programming assignment #2
* hw2_server.c
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
    int serv_sock;
    int clnt_sock;

    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    PACKET send_packet, recv_packet;
    int rx_len;
    socklen_t clnt_addr_size;
    struct in_addr iaddr;

    if (argc != 2){
        printf("Usage : ./hw2_server <port>\n");
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
    
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    clnt_addr_size = sizeof(clnt_addr);

    clnt_sock = accept(serv_sock, (struct sockaddr *) &clnt_addr, &clnt_addr_size);

    if (clnt_sock == -1)
        error_handling("accept error");

    printf("-----------------------------\n");
    printf(" Address Conversion Server\n");
    printf("-----------------------------\n");

    while(1) {
        rx_len = read(clnt_sock, &recv_packet, sizeof(PACKET));

        if (rx_len == 0)
            break;
        
        if (recv_packet.cmd == REQUEST) {
            printf("[Rx] Received Dotted-Decimal Address: %s\n", recv_packet.addr);

            if (inet_aton(recv_packet.addr, &iaddr) == 0) {
                // address conversion fail
                send_packet.cmd = RESPONSE;
                send_packet.result = ERROR;
                write(clnt_sock, &send_packet, sizeof(PACKET));
                printf("[Tx] Address conversion fail:(%s)\n\n", recv_packet.addr);
            }
            else {
                // address conversion success
                send_packet.cmd = RESPONSE;
                send_packet.result = SUCCESS;
                send_packet.iaddr = iaddr;
                write(clnt_sock, &send_packet, sizeof(PACKET));
                printf("inet_aton(%s) -> %#x\n", recv_packet.addr, send_packet.iaddr.s_addr);
                printf("[Tx] cmd: %d, iaddr: %#x, result: %d\n\n", send_packet.cmd, iaddr.s_addr, send_packet.result);
            }
        }
        else if(recv_packet.cmd == QUIT) {
            printf("[Rx] QUIT message received\n");
            break;
        }
        else {
            printf("[Rx] Invalid command: %d\n", recv_packet.cmd);
            break;
        }
    }

    printf("Server socket close and exit.\n");

    close(clnt_sock);
    close(serv_sock);

    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}