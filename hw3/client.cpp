#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sockop.h"
#include <signal.h>
#include <sys/wait.h>
#include <iostream>

int connfd; /* socket descriptor */

void handler(int signum)
{
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        ;
    }
    close(connfd);
}

int main(int argc, char *argv[])
{
    char buffer[256];
    char callback[256];
    int number = 0;
    char action[10], food[15];

    if (argc != 3)
        errexit("Usage : %s host_address host_port \n", argv[0]);

    signal(SIGCHLD, handler);

    /* create socket and connect to server */
    connfd = connectsock(argv[1], argv[2], "tcp");
    while (true) {
        scanf("%s", action);

        if ((strcmp(action, "confirm") == 0) || (strcmp(action, "cancel") == 0)) {
            sprintf(buffer, "%s", action);
        } else if (strcmp(action, "order") == 0) {
            printf("input order:\n");
            scanf("%s %d", food, &number);
            sprintf(buffer, "%s %s %d", action, food, number);
        } else if (strcmp(action, "shop") == 0) {
            sprintf(buffer, "%s list", action);
        } else {
            continue;
        }

        std::cout << buffer << std::endl;
        if ((write(connfd, buffer, sizeof buffer) == -1))
            errexit("Error : write()\n");

        int read_count = 1;

        if ((strcmp(action, "confirm") == 0)) {
            read_count = 2;
        }

        for (int i = 0; i < read_count; i++) {
            std::cout << "read" << std::endl;
            read(connfd, callback, sizeof callback);
            std::cout << callback << std::endl;
        }

        number = 0;
        action[0] = '\0';
        buffer[0] = '\0';
        callback[0] = '\0';
    }
    /* close client socket */
    close(connfd);
    return 0;
}
