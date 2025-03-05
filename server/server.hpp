#pragma once 
#include "../webserver.hpp"
#include <poll.h>
#include <fcntl.h>

#define PORT 8080
#define MAX_CLIENTS 128

class Server
{
private:
    int fd_server;
    struct sockaddr_in addr_server;
    std::vector<Client> Clients;
    std::vector<pollfd> pollfds;

public:
    Server();
    ~Server();

    // Getters and setters
    void set_fd_server(int fd);
    int get_fd_server();
    void set_addr_server(struct sockaddr_in addr);
    struct sockaddr_in get_addr_server();

    // Main server functions
    void startServer();
    int createServer();
    void bindServer();
    void listenServer();
    int acceptClient();
    void closeServer();

    // Client handling functions
    void handleClientRead(size_t index);
    void handleClientWrite(size_t index);
    void closeClientConnection(size_t index);
};

// 8192