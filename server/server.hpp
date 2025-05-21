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

// Structure to hold server configuration
struct ServerConfig {
    int server_block_index;  // Index to map back to server_block_obj
    std::string host;
    int port;                // Single port
    int fd;
    std::string server_name;
    struct sockaddr_in addr;
    
    // Constructor with parameters
    ServerConfig(int block_idx, const std::string& h, int p, const std::string& sn = "default") : 
        server_block_index(block_idx), host(h), port(p), fd(-1), server_name(sn) {}
};

class Server {
private:
    // Server properties
    
    // Client management
    std::vector<ServerConfig> server_configs;  // One config per port per server
    std::vector<Client> clients;
    
    // Polling structures
    std::vector<pollfd> pollfds;           // All file descriptors (servers + clients)
    std::vector<pollfd> pollfds_clients;   // Only client file descriptors
    std::vector<pollfd> pollfds_servers;   // Only server file descriptors
    
public:
    // All server blocks from configuration file
    std::vector<ServerBlock> server_block_obj;
    size_t number_of_servers;
    
    // Constructors/Destructors
    Server();
    ~Server();

    // Configuration methods
    void addServerConfig(int server_idx, const std::string& host, std::vector<int> ports, const std::string& server_name = "default");
    ServerConfig& getServerConfig(size_t index);
    size_t getServerCount() const;
    
    // Server management
    void initializeServers();
    int createServer(ServerConfig& config);
    int acceptClient(int server_fd, ServerBlock& server_block);
    void closeServer();
    
    // Main server loop
    void startServer();
    
    // Client handling
    void handleClientWrite(size_t index);
    void closeClientConnection(size_t index);

    // Helper methods
    void getClientIndexByFd(int fd, size_t& client_index);
    int getServerIndexByFd(int fd);
    int getServerIndexByHostPortName(const std::string& host, int port, const std::string& server_name);
    ServerBlock get_ServerConfByIndex(int fd_server);
};