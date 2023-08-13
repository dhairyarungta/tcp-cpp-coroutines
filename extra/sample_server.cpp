#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <experimental/coroutine>

template <typename T>
struct promise_type;

template<>
struct promise_type<int> 
{
    auto get_return_object()
    {
        return std::experimental::coroutine_handle<promise_type>::from_promise(*this);
    }

    auto initial_suspend()
    {
        return std::experimental::suspend_never{};
    }

    auto final_suspend()
    {
        return std::experimental::suspend_never{};
    }

    void return_value(int value){resutl= value;}

    void unhandled_exception(){std::terminate();}

    int result;
};


using coro_t = using coro_t = std::experimental::coroutine_handle<promise_type<int>>;

coro_t async_sleep(int seconds)
{
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec=0;
    select(0, nullptr, nullptr, nullptr,&tv);
    co_return 0;
}

class TCPServer
{
public:
    TCPServer(int port):port_(port){}

    int start()
    {
        int serverSocket = createServerSocket();
        int epollFd = epoll_create1(0);
        if(epollFd ==-1)
        {
            std::cerr << "Error creating epoll" << std::endl;
            return 1;

        }
        epoll_event event ;
        event.events = EPOLLIN;
        event.data.fd = serverSocket;
        if(epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket,&event) == -1)
        {
            std::cerr<<"Error adding server socket to epoll"<<std::endl;
            return 1;

        }

        while(true)
        {
            epoll_event events[10];
            int readyCount = epoll_wait(epollFd, events,10,-1);
            for(int i =0;i<readyCount;i++)
            {
                if(events[i].data.fd==serverSocket)
                {
                    acceptClient(serverSocket,epollFd);
                }
            }
        }
        close (serverSocket);
        return 0;
    }


private:
    //port number attribute
    int port_;

    int createServerSocket()
    {
        int serverSocket = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0);
        if(serverSocket==-1)
        {
            std::cerr << "Error creating server socket" << std::endl;
            return -1;

        }

        int opt = 1;
        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR|SO_REUSEPORT, &opt,sizeof(opt))==-1)
        {
            std::cerr << "Error setting socket options" << std::endl;
            return -1;

        }


        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr =INADDR_ANY;
        serverAddr.sin_port = httons(port_);

        if(bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr))==-1)
        {
            std::cerr << "Error binding to port " << port_ << std::endl;
            return -1;

        }
        if (listen(serverSocket, SOMAXCONN) == -1) {
            std::cerr << "Error listening on port " << port_ << std::endl;
            return -1;
        }

        return serverSocket;
    }

    void acceptClient(int serverSocket,int epollFd)
    {

    }

    void handleClient(int clientSocket)
    {

    }

    void asynProcess(int clientSocket, const char*data, int size)
    {

    }

};

int main()
{
    TCPServer server(8080);
    server.start();
    return 0;
}