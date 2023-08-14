#include "../include/server_socket.h"
#include "../include/util.h"
#include "../include/socket.h"
#include "../include/monitor.h"
#include "../include/listener.h"
#include "../include/error.h"

#include <memory>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

addrinfo* get_local_addr_info(short port)
{
    std::string service = std::to_string(port);
    int status;
    addrinfo* res;
    addrinfo hints = {};
    hints.ai_family= AF_INET;
    hints.ai_socktype= SOCK_STREAM;
    hints.ai_flags= AI_PASSIVE;
    status = getaddrinfo(NULL, service.c_str(), &hints, &res);
    throw_errno(status, "get_addr_info: ");
    return res;
}

ServerSocket::ServerSocket(short port, IoEventMonitor& io_monitor) 
    :io_monitor(io_monitor),is_closed(false)
{
    addrinfo* info = get_local_addrinfo(port);
    socket_fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

}

AwaitableValue<std::shared_ptr<Socket>> ServerSocket::accept_conn()
{
    SocketDetails details;
    whiel(true)
    {
        details.socket_fd = accept(socket_fd, reinterpret_cast<sockaddr*>(&details.client_addr),&details.addr_size);

        if(details.socket_fd!=-1)
        {
            break;
        }
        if(errno!=EAGAIN && errno!=EWOULDBLOCK)
        {
            throw_errno(details.socket_fd, "ServerSocket::accept() error: ");

        }
        co_await io_listener->await_read();
    }
    set_non_blocking_socket(details.socket_fd);
    co_return std::make_shared<Socket>(details, )
}



void ServerSocket::close_socket()
{
    if(is_closed.exchange(true))
    {
        return;

    }
    io_monitor.remove_fd(socket_fd);
    close(socket_fd);
}