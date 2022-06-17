#include <shm_game.hpp>
#include <boost/asio.hpp>


int main(int argc, char *argv[])
{
    shared_memory_object shm(open_only, "game_shm", read_write);
    mapped_region region(shm, read_write);

    Data* data = static_cast < Data * > (region.get_address());
    boost::asio::io_service io_service;
    boost::asio::deadline_timer timer = boost::asio::deadline_timer(io_service, boost::posix_time::seconds(1));

    int upper_bound;
    int lower_bound = 1;
    sscanf(argv[1], "%d", &upper_bound);

    std::cout << data->pid << std::endl;
    while (strcmp(data->result, "bingo") != 0) {
        int guess;
        if ((lower_bound + upper_bound) % 2) {
            guess = (lower_bound + upper_bound - 1) / 2;
        } else {
            guess = (lower_bound + upper_bound) / 2;
        }
        data->guess = guess;
        kill(data->pid, SIGUSR1);
        timer = boost::asio::deadline_timer(io_service, boost::posix_time::seconds(1));
        timer.wait();
        std::cout << data->guess << std::endl;
        std::cout << data->result << std::endl;

        if (strcmp(data->result, "bigger") == 0) {
            lower_bound = guess;
        } else if (strcmp(data->result, "smaller") == 0) {
            upper_bound = guess;
        }
    }

    return 0;
}
