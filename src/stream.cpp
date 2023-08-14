#include "../include/stream.h"
#include "../include/socket_op.h"
#include "../include/buffer.h"
#include "../include/error.h"
#include <iostream>
#include <sys/socket.h>

SocketStream::SocketStream(int socket_fd)
{
    this->socket_fd = socket_fd;
    this->is_closed = false;

}

SocketOperation SocketStream::recv_from_socket(ByteBuffer& buffer)
{
    if(is_closed)
    {
        return SOCKET_OP_CLOSE;
    }
    while(buffer.get_total_capacity()>0)
    {
        IOChunk chunk =buffer.get_write_chunk();
        int size_read = recv(socket_fd, chunk.data, chunk.size,0);
        if(size_read==0)
        {
            this->close();
            return SOCKET_OP_CLOSE;
        }
        if(size_read<0)
        {
            if(errno==EAGAIN||errno==EWOULDBLOCK)
            {
                return SOCKET_OP_BLOCK;
            }
            throw_errno(size_read,"SocketStream::recv error");
        }
        buffer.increment_write_pointer(size_read);

    }
    return SOCKET_OP_SUCCESS;
}
SocketOperation SocketStream::send_to_socket(ByteBuffer& buffer)
{
    if(is_closed)
    {
        return SOCKET_OP_CLOSE;
    }
    while(buffer.get_total_remaining()>0)
    {
        IOChunk chunk = buffer.get_read_chunk();
        int size_written = send(socket_fd, chunk.data, chunk.size,0);
        if(size_written<0)
        {
            if(errno==EAGAIN|| errno==EWOULDBLOCK)
            {
                return SOCKET_OP_BLOCK;
            }
            throw_errno(size_written, "SocketStream::send() error");
        }
        buffer.increment_read_pointer(size_written);
    }
    return SOCKET_OP_SUCCESS;
}


void SocketStream::close()
{
    is_closed = true;
}