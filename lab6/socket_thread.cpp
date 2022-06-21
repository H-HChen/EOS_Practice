#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <thread>
#include "sockop.h"
#include <iostream>
#include <pthread.h>
#include <sys/sem.h>
#include <sys/types.h>

#define SEM_KEY 11223366
#define SEMMODE 0666 

using namespace std;

int sem;

int P(int s)
{
    struct sembuf sop;
    /*the operation parameters*/
    sop.sem_num = 0; /*access the 1st(and only) sem in the array*/
    sop.sem_op = -1; /*wait..*/
    sop.sem_flg = 0; /*no special option sneeded*/
    if (semop(s, &sop, 1) < 0) {
        fprintf(stderr, "P():semop failed:%s\n",strerror(errno));
        return -1;
    }else{
        return 0;
    }
}

/*V()- returns 0 if OK;-1 if there was a problem*/
int V(int s)
{
    struct sembuf sop;
    /*the operation parameters*/
    sop.sem_num = 0; /*the 1st(and only)semin the array*/
    sop.sem_op = 1; /*signal*/
    sop.sem_flg = 0; /*no special option sneeded*/
    if (semop(s, &sop, 1) < 0) {
        fprintf(stderr, "V():semop failed:%s\n", strerror(errno));
        return -1;
    } else {
        return 0;
    }
}

void signal_handler(int signum)
{
    if (semctl(sem, 0, IPC_RMID, 0) < 0) {
        fprintf(stderr, "unable to remove semaphore %d \n", SEM_KEY);
        exit(1);
    }
    printf("Semaphore %d has been remove \n" , SEM_KEY);
}

class TcpThread {
public:
    TcpThread(int &total): money(total) {}
    void Main()
    {
        while(1) {
            char buffer[12];
            char callback[12];
            if(read(connfd, buffer, sizeof buffer) < 0)
                errexit("Error : read()\n");

            if(strcmp(buffer, "end") == 0) break;
            if(buffer[0]=='#') break;

            int temp_num = 0;
            int action = buffer[0] - 48;
            int time = buffer[2] - 48;

            for(int i = 0; i < time; i++){
                temp_num = 10 * temp_num + buffer[4 + i] - 48;
            }

            // semaphore
            P(sem);
            if (action)
                money = money - temp_num;
            else
                money = money + temp_num;

            V(sem);
            int temp = money;
            int digit;

            for(digit = 0; temp != 0; digit++){
                temp /= 10;
            }

            printf("After %s, total number is: %d\n", action==0?"deposit":"withdraw", money);
            sprintf(callback,"%d %d", digit, money);

            if((write(connfd ,callback, strlen(callback)) == -1))
                errexit("Error : write()\n");
        }
        close(connfd);
        delete this;
    }

    int connfd;
    int &money;
};

int main(int argc, char *argv[])
{
    int sockfd, connfd; /*socket descriptor*/
    struct sockaddr_in addr_cln;
    socklen_t sLen = sizeof(addr_cln);
    int money = 0;

    signal(SIGINT, signal_handler);

    if (argc != 2)
        errexit("Usage:%s port\n", argv[0]);

    sem = semget(SEM_KEY, 1, IPC_CREAT | IPC_EXCL | SEMMODE);

    if(sem<0)
    {
        fprintf(stderr, "%s:creation of semaphore %d failed:%s\n", argv[0], SEM_KEY, strerror(errno));
        exit(1);
    }

    if(semctl(sem,0,SETVAL,1)<0)
    {
        fprintf(stderr, "%s:Unable to initialize semaphore: %s\n", argv[0], strerror(errno));
        exit(0);
    }

    /*create socket and bind socket to port*/
    sockfd = passivesock(argv[1], "tcp", 10);

    while (1) {
        connfd = accept(sockfd, (struct sockaddr*) &addr_cln, &sLen);
        if (connfd <= 0) break;
        printf("\raccept client %d\n", connfd);
        char *ip = inet_ntoa(addr_cln.sin_addr);
        unsigned short cport = ntohs(addr_cln.sin_port);
        printf("\rclient ip: %s, port is %d\n", ip, cport);

        TcpThread *th = new TcpThread(money);
        th->connfd = connfd;
        thread sth(&TcpThread::Main, th);
        sth.detach();
    }

    close(sockfd);

    return 0;
}
