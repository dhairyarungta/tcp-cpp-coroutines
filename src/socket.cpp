#include "../include/socket.h"
#include "../include/monitor.h"
#include "../include/listener.h"
#include <atomic>
#include <algorithm>
#include <cstddef>
#include <sys/socket.h>
#include <unistd.h>

#define SOCKET_BUFFER_SIZE 8192

Socket::Socket(SocketDetails details , IoEventMonitor& io_monitor)
    :details(details), io_monitor(io_monitor),stream(details.socket_fd),input_buffer(SOCKET_BUFFER_SIZE),
    output_buffer(SOCKET_BUFFER_SIZE), is_closed(false)
{
    io_listener = io_monitor.register_fd(details.socket_fd);

}


void Socket::read_available(std::byte* &dest, long long &size, long long &total_read)
{
    long long bufer_remaining =input_buffer.get_total_remaining();
    if(buffer_remaining==0)
    {
        return;
    }

    long long size_to_read = std::min(size, buffer_remaining);
    input_buffer.read(dest, size_to_read);
    dest+=size_to_read;
    size-=size_to_read;
    total_read +=size_to_read;
}

AwaitableValue<long long> Socket::read(std::byte* dest, long long size, bool read_fully)
{
    long long total_read = 0;
    read_available(dest ,size, total_read);
    while(size>0)
    {
        SocketOperation status = stream.recv_from_socket(input_buffer);
        read_available(dest ,size, total_read);
        if(size==0||status==SOCKET_OP_SUCCESS)
        {
            continue;
        }
        if(status==SOCKET_OP_BLOCK)
        {
            co_await io_listener->await_read();
        }else if(status==SOCKET_OP_CLOSE)
        {
            if(read_fully)
            {
                throw std::runtime_error("Socket read(): Socket closed");

            }
            break;
        }

    }
    co_return total_read;
}
void Socket::skip_available(long long &size, long long &total_skipped)
{
    long long buffer_remaining =input_buffer.get_total_remaining();
    if(buffer_remaining==0)
    {
        return;
    }
    long long size_to_skip = std::min(size, buffer_remaining);
    input_buffer.increment_read_pointer(size_to_skip);
    size-=size_to_skip;
    total_skipped +=size_to_skip;
}

AwaitableValue<long long> Socket::skip(long long size, bool skip_fully)
{
    long long total_skipped =0;
    
}

void Socket::write_availble(const std::byte*& src, long long &size)
{
    long long buffer_capacity = output_buffer.get_total_capacity();
    if(buffer_capacity==0)
    {
        return;
    }
    long long size_to_write = std::min(buffer_capacity,size);
    output_buffer.write(src, size_to_write);
    src+=size_to_write;
    size-=size_to_write;
}


AwaitableFuture Socket::write(const std::byte*src, long long size)
{

}

AwaitableFuture Socket::flush()
{
    while(output_buffer.get_total_remaining()>0)
    {
        SocketOperation status = stream.send_to_socket(output_buffer);
        if(status==SOCKET_OP_BLOCK)
        {
            co_await io_listener->await_write();
        } else if(status==SOCKET_OP_CLOSE)
        {
            throw std::runtime_error("Socket::flush() error: socket closed")
        }       
        
    }
}



int Socket::get_fd()
{
    return details.socket_fd ;

}

void Socket::close_socket()
{
    if(is_closed.exchange(true))
    {
        return;
    }

    stream.close();
    io_listener->close();
    io_monitor.remove_fd(details.socket_fd);
    close(details.socket_fd);
}
