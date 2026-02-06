#ifndef DEBUG_HPP
#define DEBUG_HPP
#include <cstddef>
#include <iostream>
#include <time.h>

inline void debug(const char* msg)
{
    time_t cur_t = time(NULL);
    struct tm* t = localtime(&cur_t);
    std::cout << "[" << t->tm_hour << ":" << t->tm_min << ":" << t->tm_sec <<
               "]" << msg <<std::endl;
}
#endif