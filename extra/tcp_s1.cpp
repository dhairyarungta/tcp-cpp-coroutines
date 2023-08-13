//this is just a overview of the implementation
// the file does not compile 
class ClientConnection {
public:
    ClientConnection(asio::ip::tcp::socket socket)
        : socket_(std::move(socket)) {
    }

    void Start() {
        ReadAsync();
    }

private:
    void ReadAsync() {
        asio::async_read_until(socket_, buffer_, '\n',
            [this](std::error_code ec, std::size_t bytes_transferred) {
                if (!ec) {
                    std::istream is(&buffer_);
                    std::string message;
                    std::getline(is, message);
                    ProcessMessage(message);

                    // Continue reading
                    ReadAsync();
                }
            });
    }

    void ProcessMessage(const std::string& message) {
        // Process the received message
        std::cout << "Received: " << message << std::endl;
    }

    asio::ip::tcp::socket socket_;
    asio::streambuf buffer_;
};

class ThreadPool {
public:
    ThreadPool(std::size_t num_threads)
        : work_(io_context_), stop_(false) {
        for (std::size_t i = 0; i < num_threads; ++i) {
            threads_.emplace_back([this] {
                io_context_.run();
            });
        }
    }

    ~ThreadPool() {
        Stop();
    }

    void Enqueue(std::function<void()> task) {
        io_context_.post(task);
    }

    void Stop() {
        if (!stop_.exchange(true)) {
            io_context_.stop();
            for (std::thread& thread : threads_) {
                thread.join();
            }
        }
    }

private:
    asio::io_context io_context_;
    asio::io_context::work work_;
    std::vector<std::thread> threads_;
    std::atomic_bool stop_;
};

class TCPServer {
public:
    TCPServer(unsigned short port, std::size_t num_threads, asio::io_context &io_context)
        : acceptor_(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
          thread_pool_(num_threads) {
        StartAccept();
    }

private:
    void StartAccept() {
        acceptor_.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
            if (!ec) {
                auto client_connection = std::make_shared<ClientConnection>(std::move(socket));
                client_connection->Start();
            }

            StartAccept(); // Continue accepting connections
        });
    }

    asio::ip::tcp::acceptor acceptor_;
    ThreadPool thread_pool_;
};

int main() {
    try {
        asio::io_context io_context;
        TCPServer server(12345, 4); // Port 12345, 4 worker threads
        io_context.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
