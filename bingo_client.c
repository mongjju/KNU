/*
* Computer Science and Engineering 2020113766 박윤진
* Network Programing Hw4 (Bingo) client
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define RAND_START  1   // start random num
#define RAND_END    30  // end random num

// result field value
#define FAIL    0       // incorrect
#define SUCCESS 1       // correct
#define CHECKED 2       // already fit

// cmd field value
#define BINGO_REQ 0
#define BINGO_RES 1
#define BINGO_END 2

// bingo size
#define ROW 4
#define COL 4

// packet : client -> server
typedef struct {
    int cmd;    // BINGO_REQ
    int number; // random number(1~30)
}REQ_PACKET;

// packet : server -> client
typedef struct{
    int cmd;                // BINGO_RES, BINGO_END
    int number;             // client's number
    int board[ROW][COL];    // current bingo(0 or bingo number)
    int result;             // result from the server(FAIL, SUCCESS, CHECKED)
}RES_PACKET;

int player_choice_array[ROW][COL] = {0, };  // current bingo
int check_array[31] = {0, };                // check used number
//int check = BINGO_

void print_bingo(int arr[ROW][COL]);
void error_handling(char *message);

int main(int argc, char *argv[]) {
    if(argc != 3) {             // check argument
		printf("Usage: %s <IP> <port>\n", argv[0]);
		exit(1);
	}

    int sock;
    int r_number;               // random number
    int str_len;
    socklen_t adr_sz;
    struct sockaddr_in serv_adr, from_adr;
    REQ_PACKET req_packet;
    RES_PACKET res_packet;

    // create UDP socket
    sock = socket(PF_INET, SOCK_DGRAM, 0);

    if (sock == -1)
        error_handling("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    srand(time(NULL));


    while(1) {
        // create random number
        r_number = (rand() % RAND_END) + RAND_START;
        printf("Random number(1~30): %d\n", r_number);

        // send REQ PACKET
        req_packet.cmd = BINGO_REQ;
        req_packet.number = r_number;
        sendto(sock, &req_packet, sizeof(REQ_PACKET), 0, (struct sockaddr*)&serv_adr, sizeof(serv_adr));
        printf("[Tx] BINGO_REQ(number: %d)\n\n", req_packet.number);

        // receive RES PACKET
        adr_sz = sizeof(from_adr);
        str_len = recvfrom(sock, &res_packet, sizeof(RES_PACKET), 0, (struct sockaddr*)&from_adr, &adr_sz);
        memcpy(player_choice_array, res_packet.board, sizeof(player_choice_array));
        
        printf("[Rx] BINGO_RES(number: %d, result: %d)\n", res_packet.number, res_packet.result);
        print_bingo(player_choice_array);

        // check bingo end or not
        if (res_packet.cmd == BINGO_END) {
            printf("Exit Client\n");
            break;
        }
    }
    
    close(sock);

    return 0;
}

void print_bingo(int arr[ROW][COL]) {
    printf("+----+----+----+----+\n");
    for (int i = 0; i < ROW; i++)
    {
        for (int j = 0; j < COL; j++) {
            if (arr[i][j] == 0)
                printf("|    ");
            else 
                printf("| %2d ", arr[i][j]);
        }
        printf("|\n");
        printf("+----+----+----+----+\n");
    }
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}