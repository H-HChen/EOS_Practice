#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sockop.h"
#include <signal.h>
#include <sys/wait.h>

int connfd; /* socket descriptor */

void handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
    close(connfd);
}

int main(int argc, char *argv[])
{
    
    char buffer[12];
    char callback[12];
    int action;

    if (argc != 6)
        errexit("Usage : %s host_address host_port <deposit | withdraw> amount number  \n", argv[0]);

    signal(SIGCHLD, handler);

    /* create socket and connect to server */
    connfd = connectsock(argv[1], argv[2], "tcp");

    if (strcmp(argv[3], "deposit") == 0) {
        action = 0;
    } else {
        action = 1;
    }
    
    int cash = atoi(argv[4]);
    int number = atoi(argv[5]);
    int temp_cash = cash;
    int digit;

    for (digit = 0; temp_cash != 0; digit++) {
        temp_cash /= 10;
    }
    
    printf("Send for %d times\n", number);

    for (int i=0; i < number; i++) {
        sprintf(buffer, "%d %d %d", action, digit, cash);

        if((write(connfd, buffer, strlen(buffer)) == -1))
            errexit("Error : write()\n");

        read(connfd, callback, sizeof callback);

        int time = callback[0]-48;
        int temp_num = 0;

        for(int i = 0; i < time; i++){
            temp_num = 10 * temp_num + callback[2 + i] - 48;
        }

        printf("current number: %d\n", temp_num);
    }

    buffer[0]='#';

    if((write(connfd, buffer, strlen(buffer)) == -1))
            errexit("Error : write()\n");

    /* close client socket */
    close(connfd) ;
    return 0 ;
}
