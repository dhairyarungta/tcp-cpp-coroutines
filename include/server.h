#pragma once
#include "future.h"
#include "server_socket.h"
#include <atomic>
class ServerApplication;
class ThreadPool;
class IoEventMonitor;

class Server
{
private :
    ServerApplication& server_application;
    IoEventMonitor& io_monitor;
    ThreadPool& thread_pool;
    ServerSocket server_socket;
    std::atomic_bool market_for_close;
    Future run_server_loop();
public : 
    Server(short port, ServerApplication& server_app, IoEventMonitor& io_monitor, ThreadPool& thread_pool);
    void start ();
    void shutdown ();
};