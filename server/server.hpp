#pragma once 
#include "../webserver.hpp"
#include "../parsing/ServerBlock.hpp"
#include "../parsing/Confile.hpp"
#include <poll.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <sstream>


#define DEFAULT_PORT 8080
#define MAX_CLIENTS 128

// Structure to hold server configuration
struct ServerConfig {
    
    int server_index;
    std::string host;
    std::string ip;
    int port;
    int fd;
    struct sockaddr_in addr;
    
    ServerConfig() : host("localhost"), ip("0.0.0.0"), port(DEFAULT_PORT), fd(-1) {}
    ServerConfig(const std::string& h, const std::string& i, int p) : 
        host(h), ip(i), port(p), fd(-1) {}
};

class Server
{
    private:
    // Server configuration from cofig file
    // Server properties
    std::vector<ServerConfig> server_configs;

    
    
    // Client management
    std::vector<Client> clients;
    
    // Polling structures
    std::vector<pollfd> pollfds;           // All file descriptors (servers + clients)
    std::vector<pollfd> pollfds_clients;   // Only client file descriptors
    std::vector<pollfd> pollfds_servers;   // Only server file descriptors
    
    // Helper function for string conversion 
    std::string intToString(int num) {
        std::ostringstream ss;
        ss << num;
        return ss.str();
    }
    
    public:
        std::vector<ServerBlock> server_block_obj;
        size_t number_of_servers;
        Server();
        ~Server();

    // Configuration setters and getters
    void addServerConfig(const std::string& host, const std::string& ip, int port);
    void setServerConfig(size_t index, const std::string& host, const std::string& ip, int port);
    ServerConfig& getServerConfig(size_t index);
    size_t getServerCount() const;
    
    // Server management
    void initializeServers();
    int createServer(ServerConfig& config);
    void bindServer(ServerConfig& config);
    void listenServer(ServerConfig& config);
    // int acceptClient(int server_fd, struct sockaddr_in& server_addr);
    int acceptClient(int server_fd, struct sockaddr_in& , ServerBlock & server_block_obj);
    void closeServer();
    
    // Main server loop
    void startServer();
    
    // Client handling
    void handleClientRead(size_t index);
    void handleClientWrite(size_t index);
    void closeClientConnection(size_t index);

    // Helper methods
    void getClientIndexByFd(int fd, size_t& client_index);
    int getServerIndexByFd(int fd);


    // int get_number_of_servers();
    // void set_number_of_servers(int number_of_servers);

    
    ServerBlock get_ServerConfByIndex(int fd_server);
};