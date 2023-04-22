/*
* 컴퓨터학부 2020113766 박윤진
* network programming 01
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFSIZE 10

void error_handling(char* message);

int main(int argc, char *argv[]) {
    int s_fd, d_fd;
    char buf[BUFSIZE];
    ssize_t read_len = 0, total_len = 0;

    if (argc != 3) {                    // check argument count
        printf("[Error] mymove Usage : ./mymove src_file dest_file\n");
        exit(1);
    }
    
    s_fd = open(argv[1], O_RDONLY);     // open source file
    if (s_fd == -1)
        error_handling("open() error!");

    d_fd = open(argv[2], O_CREAT|O_WRONLY|O_TRUNC, 0644);   // create destination file
    if (d_fd == 1)
        error_handling("open() error!");

    while (read_len = read(s_fd, buf, BUFSIZE)) {
        if (read_len == -1)
            error_handling("read() error!");

        write(d_fd, buf, BUFSIZE);
        total_len += read_len;
    }
    
    printf("move from %s to %s (bytes: %ld) finished.\n\n", argv[1], argv[2], total_len);

    close(s_fd);
    close(d_fd);

    unlink(argv[1]);

    return 0;
}

void error_handling(char* message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
