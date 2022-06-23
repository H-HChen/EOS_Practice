#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <thread>
#include "sockop.h"
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <boost/asio.hpp>

using namespace std;
sem_t binSem, countSem;
int total_customer = 0, total_cash = 0;
int sockfd;

void signal_handler(int signum)
{
    if ((sem_destroy(&binSem) < 0) || (sem_destroy(&countSem) < 0)) {
        fprintf(stderr, "unable to remove semaphore");
        exit(1);
    }

    printf("Semaphore has been remove \n");

    ofstream logfile;
    logfile.open ("result.txt");
    logfile << "customer: " << total_customer << "\nincome: " << total_cash;
    logfile.close();

    close(sockfd);
    std::terminate();
}

struct food {
    int shop_index;
    int price;
};

std::map < int, int > store_dist = {
    {1, 3},
    {2, 5},
    {3, 8}
};

std::map < string, food > food_list = {
    {"cookie",        {1, 60 }},
    {"cake",          {1, 80 }},
    {"tea",           {2, 40 }},
    {"boba",          {2, 70 }},
    {"fried-rice",    {3, 120}},
    {"Egg-drop-soup", {3, 50 }}
};

class TcpThread {
public:
    TcpThread(vector < int > &time_list) : arrive_time(time_list)
    {
        if_confirm = false;
    }
    ~TcpThread()
    {}

    void Main()
    {
        while (1) {
            char *temp;
            std::vector < char* > action_vec;

            if (read(connfd, buffer, sizeof buffer) < 0)
                errexit("Error : read()\n");

            std::cout << "buffer: " << buffer << " from " << connfd << std::endl;

            if (!strlen(buffer)) continue;

            temp = strtok(buffer, " ");

            while (temp) {
                action_vec.push_back(temp);
                temp = strtok(NULL, " ");
            }

            if (if_confirm) {
                if (strcmp(action_vec[0], "Yes") == 0) {
                    int res = sem_wait(&binSem);
                    int min_index = _get_min_index(arrive_time);
                    arrive_time[min_index] += store_dist[shop_index];
                    res = sem_post(&binSem);
                    sem_wait(&countSem);
                    send_order(min_index);
                    sem_post(&countSem);
                    break;
                } else {
                    break;
                }
            }

            char* action;
            asprintf(&action, "%s", action_vec[0]);

            if (strcmp(action, "order") == 0) {
                int number;
                char* _food_name;

                sscanf(action_vec[2], "%d", &number);
                asprintf(&_food_name, "%s", action_vec[1]);

                string food_name = _food_name;
                int shop_index_temp = food_list[food_name].shop_index;

                if (!shop_index) {
                    shop_index = shop_index_temp;
                } else if ((shop_index) && (shop_index != shop_index_temp)) {
                    std::cout << "User can only order in one shop!!" << std::endl;
                    callback_order_list();

                    callback_transmit();
                    continue;
                }

                if (order_list.find(food_name) != order_list.end()) {
                    order_list[food_name] += number;
                } else {
                    order_list[food_name] = number;
                }
                callback_order_list();
                callback_transmit();
            } else if (strcmp(action, "confirm") == 0) {
                if (order_list.size()) {
                    int min_index = _get_min_index(arrive_time);
                    int temp_time = arrive_time[min_index] + store_dist[shop_index];
                    std::cout << " Wait time: " << temp_time << std::endl;

                    if (temp_time > 30) {
                        if_confirm = true;
                        sprintf(callback, "Your delivery will take a long time, do you want to wait?");
                        callback_transmit();
                        continue;
                    } else {
                        int res = sem_wait(&binSem);
                        int min_index = _get_min_index(arrive_time);
                        arrive_time[min_index] += store_dist[shop_index];
                        res = sem_post(&binSem);
                        sem_wait(&countSem);
                        send_order(min_index);
                        sem_post(&countSem);
                        break;
                    }
                } else {
                    sprintf(callback, "Please order some meals");
                    callback_transmit();
                }
            } else if (strcmp(action, "cancel") == 0) {
                break;
            } else {
                sprintf(callback, "Dessert Shop:3km\n- cookie:60$|cake:80$\nBeverage Shop:5km\n- tea:40$|boba:70$\nDiner:8km\n- fried-rice:120$|Egg-drop-soup:50$");
                callback_transmit();
            }
        }

        close(connfd);
        delete this;
    }

    int _get_min_index(vector < int > time)
    {
        if (time[0] > time[1]) {
            return 1;
        } else {
            return 0;
        }
    }

    void callback_transmit()
    {
        std::cout << "callback: " << callback << std::endl;

        try {
            signal(SIGPIPE, SIG_IGN);
            if ((write(connfd, callback, sizeof(callback)) == -1))
                fprintf(stderr, "write fd err . fd == %d/n", connfd);
        } catch (std::exception& e)
        {
            printf("errrno is: %d", errno);
            std::cout << "Exception: " << e.what();
        }

        memset(buffer, 0, sizeof buffer);
        memset(callback, 0, sizeof callback);
    }

    void send_order(int time_index)
    {
        int total = 0;

        for (const auto key: order_list) {
            total += food_list[key.first].price * key.second;
        }

        if (!if_confirm) {
            sprintf(callback, "Please wait a few minutes...");
            callback_transmit();
        }


        for (int i = 0; i < store_dist[shop_index]; i++) {
            timer = boost::asio::deadline_timer(io_service, boost::posix_time::seconds(1));
            timer.wait();
            int res = sem_wait(&binSem);
            arrive_time[time_index] -= 1;
            res = sem_post(&binSem);
        }

        sprintf(callback, "Delivery has arrived and you need to pay %d$", total);
        callback_transmit();
        total_customer += 1;
        total_cash += total;
        std::cout << "income: " << total_cash << std::endl;
        std::cout << "customer: " << total_customer << std::endl;
    }

    void callback_order_list()
    {
        int count = 0;

        for (const auto key: order_list) {
            //special case for reverse order
            if ((key.first == "Egg-drop-soup") && (order_list.size() > 1)) {
                sprintf(callback, "%s %d|%s %d", "fried-rice", order_list["fried-rice"], "Egg-drop-soup", order_list["Egg-drop-soup"]);
                return;
            } else if ((key.first == "boba") && (order_list.size() > 1)) {
                sprintf(callback, "%s %d|%s %d", "tea", order_list["tea"], "boba", order_list["boba"]);
                return;
            } else {
                char temp_key[50];
                // cout << key.first << " : " << key.second << endl;
                if (count) strcat(callback, "|");

                sprintf(temp_key, "%s %d", key.first.c_str(), key.second);
                strcat(callback, temp_key);
                count++;
            }
        }
    }

    char buffer[256];
    char callback[256];
    int connfd;
    int shop_index = 0;
    int time_intervel = 0;
    vector < int > &arrive_time;
    bool if_confirm = false;
    std::map < string, int > order_list;
    boost::asio::io_service io_service;
    boost::asio::deadline_timer timer = boost::asio::deadline_timer(io_service, boost::posix_time::seconds(1));
};

int main(int argc, char *argv[])
{
    int connfd; /*socket descriptor*/
    struct sockaddr_in addr_cln;
    socklen_t sLen = sizeof(addr_cln);

    vector < int > arrive_time = {0, 0};
    int res;

    if (argc != 2)
        errexit("Usage:%s port\n", argv[0]);

    signal(SIGINT, signal_handler);

    res = sem_init(&binSem, 0, 1);
    res = sem_init(&countSem, 0, 2);

    if (res < 0) {
        fprintf(stderr, "%s:creation of semaphore failed:%s\n", argv[0], strerror(errno));
        exit(1);
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

        TcpThread *th = new TcpThread(arrive_time);
        th->connfd = connfd;
        thread sth(&TcpThread::Main, th);
        sth.detach();
    }

    close(sockfd);

    return 0;
}
