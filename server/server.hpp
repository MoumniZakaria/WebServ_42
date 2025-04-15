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
    std::vector<int> ports;  // Changed from int port to vector of ports
    int fd;
    struct sockaddr_in addr;
    
    ServerConfig() : host("localhost"), ip("0.0.0.0"), fd(-1) {
        ports.push_back(DEFAULT_PORT);  // Default port
    }
    
    ServerConfig(const std::string& h, const std::string& i, const std::vector<int>& p) : 
        host(h), ip(i), ports(p), fd(-1) {}
};

class Server {
    private:
        // Server configuration from config file
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
        void addServerConfig(const std::string& host, const std::string& ip, std::vector<int> ports);
        void setServerConfig(size_t index, const std::string& host, const std::string& ip, std::vector<int> ports);
        ServerConfig& getServerConfig(size_t index);
        size_t getServerCount() const;
        
        // Server management
        void initializeServers();
        int createServer(ServerConfig& config);
        void bindServer(ServerConfig& config);
        void listenServer(ServerConfig& config);
        int acceptClient(int server_fd, struct sockaddr_in& addr, ServerBlock& server_block_obj);
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
};