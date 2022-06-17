#include <iostream>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

using namespace boost::interprocess;

struct Data
{
    int guess;
    char result[8];
    pid_t pid;
};
