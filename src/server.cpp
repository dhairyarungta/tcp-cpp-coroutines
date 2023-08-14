#include "../include/server.h"
#include "../include/socket.h"
#include "../include/app.h"
#include "../include/thread_pool.h"
#include "../include/monitor.h"
#include <iostream>
#include <string>
#include <stdexcept>
#include <memory>


Server::Server(short port , ServerApplication& server_app, IoEventMonitor&io_monitor, ThreadPool& thread_pool)
    :server_socket(port, io_monitor), server_app(server_app), io_monitor(io_monitor), thread_pool(thread_pool)
{
    this->market_for_close= false;
}    

Future Server::run_server_loop()
{
    try
    {
        while(!market_for_close)
        {
            std::shared_ptr<Socket> socket = co_await server_socket.accept_conn();
            thread_pool.run([&, socket_ptr = std::move(socket)] 
            {
                server_app.handle_socket(
                    *this,
                    std::move(socket_ptr)
                );
            });

        }
    }
    catch(const std::runtime_error error)
    {
        std::cout<<"Error: "<<error.what()<<std::endl;

    }
    server_socket.close_socket();
    
}

void Server::start()
{
    thread_pool.run([&]
    {
        run_server_loop();
    });
}

void Server::shutdown()
{
    if(market_for_close.exchange(true))
    {
        throw std::runtime_error(std::string("Server::shutdown() error: Server already shutdown"));

    }
    server_socket.close_socket();
    server_app.shutdown();
    io_monitor.shutdown();
    thread_pool.shutdown();
}