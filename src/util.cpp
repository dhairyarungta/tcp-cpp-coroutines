#include "../include/util.h"
#include "../include/error.h"
#include <string>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

void set_non_blocking_socket(int socket_fd)
{
    int flags = fcntl(socket_fd, F_GETFL);
    int status = fcntl (socket_fd, F_SETFL, flags | O_NONBLOCK);
    throw_errno(status, "set_non_blocking_socket error: ");
}