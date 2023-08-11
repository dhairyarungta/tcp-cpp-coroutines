#pragma once

#include "socket_op.h"
#include <atomic>

class ByteBuffer;

class SocketStream
{
private :
    int socket_fd;
    std::atomic_bool is_closed;
public:
    SocketStream(int socket_fd);

    SocketOperation recv_from_socket(ByteBuffer& buffer);
    SocketOperation send_to_socket(ByteBuffer& buffer);
    void close();
};