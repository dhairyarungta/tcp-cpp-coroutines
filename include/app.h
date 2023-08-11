#pragma once

#include "future.h"
#include <memory>
#include <mutex>
#include <unordered_map>

class Socket ;
class Server ;

class ServerApplication
{
private :
    std::mutex socket_mutex;
    std::unordered_map<int, std::weak_ptr<Socket>> socket_map;

public :
    void handle_socket(Server& server, std::shared_ptr<Socket> socket_ptr);
    virtual Future on_request(Server& server, std::shared_ptr<Socket> socket)=0;
    void shutdown();

};
