#pragma once 
#include "../webserver.hpp"
#include <poll.h>
#include <fcntl.h> // for fcntl

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
    Server(int fd);
    ~Server();
    // geters and setters
    void set_fd_server(int fd);
    int get_fd_server();
    void set_addr_server(struct sockaddr_in addr);
    struct sockaddr_in get_addr_server();
    // void set_Clients(std::vector<Client> & C);
    // std::vector<Client> & get_Clients();

    // functions
    
    void startServer();
    int  createServer();
    void bindServer();
    void listenServer();
    int acceptClient();
    void closeServer();



    //client functions
    void handleNewConnection();
    void handleClient(size_t index);
    bool handleClientRead(Client& client, pollfd& pfd);
    bool handleClientWrite(Client& client, pollfd& pfd);
    void generateResponse(Client& client);
    Client& findClient(int fd);

};

int bindSocket(int server_fd);
int createSocket();



