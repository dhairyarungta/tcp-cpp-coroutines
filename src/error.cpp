#include "../include/error.h"

#include <cstring> 
#include <cerrno>


void throw_errno(int status, std::string prefix)
{
    if(status ==-1)
    {
        throw std::runtime_error(prefix.append(strerror(errno)));
    }
}