#pragma once

class IoEventMonitor;
class IoEventListener;
class Socket;


class ServerSocket
{
private:
    int socket_fd;
    IoEventMonitor& io_monitor;
    IoEventListener* io_listener;
    std::atomic_bool is_closed;

public :
    ServerSocket(short port,IoEventMonitor& io_monitor);
    AwaitableValue<std::shared_ptr<Socket>> accept_conn();
    void close_socket();
}