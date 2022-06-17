#include "shm_game.hpp"
#include <boost/asio.hpp>


namespace {
    std::function < void(int) > shutdown_handler;
    void signal_handler(int signal)
    {
        shutdown_handler(signal);
    }
}

int main(int argc, char *argv[])
{
    std::signal(SIGUSR1, signal_handler);

    shared_memory_object::remove("game_shm");
    shared_memory_object shm(create_only, "game_shm", read_write);
    shm.truncate(sizeof(Data));
    mapped_region region(shm, read_write);
    std::memset(region.get_address(), 0, region.get_size());
    Data* data = static_cast < Data * > (region.get_address());

    boost::asio::io_service io_service;
    boost::asio::deadline_timer timer = boost::asio::deadline_timer(io_service, boost::posix_time::seconds(1));

    auto pid = getpid();
    std::cout << pid << std::endl;

    data->pid = pid;
    data->guess = 0;

    int fin;
    sscanf(argv[1], "%d", &fin);
    std::cout << "Answer: " << fin << std::endl;

    shutdown_handler = [&](int signal) {
        if (signal == SIGUSR1) {
            if (data->guess < fin) {
                sprintf(data->result, "bigger");
            } else if (data->guess > fin) {
                sprintf(data->result, "smaller");
            } else {
                sprintf(data->result, "bingo");
                sleep(1);
            }
        } else {
            shared_memory_object::remove("game_shm");
            exit(0);
        }
    };
    while (strcmp(data->result, "bingo") != 0) {}

    return 0;
}
