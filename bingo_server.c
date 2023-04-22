/*
 * Computer Science and Engineering 2020113766 박윤진
 * Network Programing Hw4 (Bingo) server
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define RAND_START 1 // start random num
#define RAND_END 30  // end random num

// result field value
#define FAIL 0    // incorrect
#define SUCCESS 1 // correct
#define CHECKED 2 // already fit

// cmd field value
#define BINGO_REQ 0
#define BINGO_RES 1
#define BINGO_END 2

// bingo size
#define ROW 4
#define COL 4

// packet : client -> server
typedef struct
{
    int cmd;    // BINGO_REQ
    int number; // random number(1~30)
} REQ_PACKET;

// packet : server -> client
typedef struct
{
    int cmd;             // BINGO_RES, BINGO_END
    int number;          // client's number
    int board[ROW][COL]; // current bingo(0 or bingo number)
    int result;          // result from the server(FAIL, SUCCESS, CHECKED)
} RES_PACKET;

int bingo_array[ROW][COL] = {0, };          // random bingo number arrary (1~30)
int player_choice_array[ROW][COL] = {0, };  // current bingo
int check_array[31] = {0, };                // check used number

void create_bingo(int arr[ROW][COL]);
void print_bingo(int arr[ROW][COL]);
int check_end(int arr[ROW][COL]);
void error_handling(char *message);

int main(int argc, char *argv[]) {
    if (argc != 2)
    { // check argument
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int serv_sock;
    int str_len;
    int check_bingo;
    int i, j;
    int check = 0;
    socklen_t clnt_adr_sz;
    struct sockaddr_in serv_adr, clnt_adr;
    REQ_PACKET req_packet;
    RES_PACKET res_packet;

    // create UDP socket
    serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (serv_sock == -1)
        error_handling("UDP socket createion error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");

    create_bingo(bingo_array);
    print_bingo(bingo_array);

    printf("\nReady!\n");

    while (1)
    {
        // receive REQ PACKET from client
        clnt_adr_sz = sizeof(clnt_adr);
        str_len = recvfrom(serv_sock, &req_packet, sizeof(req_packet), 0, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
        printf("[Rx] BINGO_REQ(cmd: %d, number: %d)\n", req_packet.cmd, req_packet.number);

        // compare to bingo
        for (i = 0; i < ROW; i++) {
            for (j = 0; j < COL; j++) {
                if ((bingo_array[i][j] == req_packet.number) && (player_choice_array[i][j] == req_packet.number)) {
                    check_bingo = CHECKED;
                    break;
                }

                else if ((bingo_array[i][j] == req_packet.number) && (player_choice_array[i][j] == 0)) {
                    check_bingo = SUCCESS;
                    player_choice_array[i][j] = req_packet.number;
                    break;
                }
                else
                    check_bingo = FAIL;
            }
            if ((check_bingo == SUCCESS) || (check_bingo == CHECKED))
                break;
        }

        // send RES PACKET to client
        if (check_bingo == FAIL){ 
            // Result = fail
            res_packet.cmd = BINGO_RES;
            res_packet.number = req_packet.number;
            res_packet.result = FAIL;
            memcpy(res_packet.board, player_choice_array, sizeof(player_choice_array));
            sendto(serv_sock, &res_packet, sizeof(res_packet), 0, (struct sockaddr *)&clnt_adr, clnt_adr_sz);
            
            printf("Not found: num: %d\n", res_packet.number);
        }
        else if (check_bingo == SUCCESS) {
            // Result = success
            if (check_end(player_choice_array) == BINGO_RES) {
                res_packet.cmd = BINGO_RES;
                res_packet.number = req_packet.number;
                res_packet.result = SUCCESS;
                memcpy(res_packet.board, player_choice_array, sizeof(player_choice_array));
                sendto(serv_sock, &res_packet, sizeof(res_packet), 0, (struct sockaddr*)&clnt_adr, clnt_adr_sz);

                printf("Bingo: [%d][%d]: %d\n", i, j, req_packet.number);
            }
            else {  // bingo end
                res_packet.cmd = check_end(player_choice_array);
                res_packet.number = req_packet.number;
                res_packet.result = SUCCESS;
                memcpy(res_packet.board, player_choice_array, sizeof(player_choice_array));
                sendto(serv_sock, &res_packet, sizeof(res_packet), 0, (struct sockaddr*)&clnt_adr, clnt_adr_sz);
                
                printf("Bingo: [%d][%d]: %d\n", i, j, req_packet.number);
                printf("No available space.\n");

                break;
            }
        }
        else if (check_bingo == CHECKED) {
            // Result = checked
            res_packet.cmd = BINGO_RES;
            res_packet.number = req_packet.number;
            res_packet.result = CHECKED;
            memcpy(res_packet.board, player_choice_array, sizeof(player_choice_array));
            sendto(serv_sock, &res_packet, sizeof(res_packet), 0, (struct sockaddr*)&clnt_adr, clnt_adr_sz);

            printf("Client already chose(num: %d)\n", res_packet.number);    
        }

        printf("[Tx] BINGO_RES(cmd: %d, result: %d)\n", res_packet.cmd, res_packet.result);
        print_bingo(player_choice_array);
        
    }

    printf("[Tx] BINGO_END\n");
    printf("Exit Server\n");

    return 0;
}

void create_bingo(int arr[ROW][COL]) {
    int rand_num, count = 0;

    srand(time(NULL));

    while (count < ROW * COL)
    {
        rand_num = (rand() % RAND_END) + RAND_START;
        if (check_array[rand_num] == 0)
        {
            bingo_array[count / ROW][count % COL] = rand_num;
            check_array[rand_num] = 1;
            count++;
        }
    }
}

int check_end(int arr[ROW][COL]) {
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            if (arr[i][j] == 0)
                return BINGO_RES; 
        }
    }
    return BINGO_END;
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