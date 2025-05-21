#include "../webserver.hpp"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sstream>
#include <signal.h>

Server::Server() {
    // Ignore SIGPIPE to prevent program termination on broken pipes
    signal(SIGPIPE, SIG_IGN);
}

Server::~Server() {
    this->closeServer();
}

void Server::addServerConfig(int server_idx, const std::string& host, std::vector<int> ports, const std::string& server_name) {
    // Create a ServerConfig for each port in the server block
    for (size_t i = 0; i < ports.size(); i++) {
        ServerConfig config(server_idx, host, ports[i], server_name);
        server_configs.push_back(config);
    }
}

ServerConfig& Server::getServerConfig(size_t index) {
    if (index >= server_configs.size()) {
        throw std::runtime_error("Invalid server index in getServerConfig");
    }
    return server_configs[index];
}

size_t Server::getServerCount() const {
    return server_configs.size();
}

void Server::initializeServers() {
    for (size_t i = 0; i < server_configs.size(); i++) {
        ServerConfig& config = server_configs[i];
        
        // print config details
        
        // Create, bind, and listen on each server
        config.fd = createServer(config);
        
        if (config.fd >= 0) {
            struct pollfd server_poll;
            server_poll.fd = config.fd;
            server_poll.events = POLLIN;
            server_poll.revents = 0;
            
            pollfds.push_back(server_poll);
            pollfds_servers.push_back(server_poll);
            
            std::cout << "\033[33mServer initialized on " << config.host << ":" << config.port 
                      << " (FD: " << config.fd << ", Server Block: " << config.server_block_index 
                      << ", Name: " << config.server_name << ")\033[0m" << std::endl;
        }
    }
}

int Server::createServer(ServerConfig& config) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Socket creation failed for " << config.host << ":" << config.port 
                  << " (Name: " << config.server_name << ") - " << strerror(errno) << std::endl;
        return -1;
    }

    // Set socket to non-blocking mode
    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        std::cerr << "Setsockopt failed for " << config.host << ":" << config.port 
                  << " (Name: " << config.server_name << ") - " << strerror(errno) << std::endl;
        close(server_fd);
        return -1;
    }

    // Set up address structure
    memset(&config.addr, 0, sizeof(config.addr));
    config.addr.sin_family = AF_INET;
    config.addr.sin_port = htons(config.port);
    
    // Always use INADDR_ANY (0.0.0.0) for binding
    config.addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    if (bind(server_fd, (struct sockaddr*)&config.addr, sizeof(config.addr)) < 0) {
        std::cerr << "Bind failed for " << config.host << ":" << config.port 
                  << " (Name: " << config.server_name << ") - " << strerror(errno) << std::endl;
        close(server_fd);
        return -1;
    }

    // Listen for connections
    if (listen(server_fd, SOMAXCONN) < 0) {
        std::cerr << "Listen failed for " << config.host << ":" << config.port 
                  << " (Name: " << config.server_name << ") - " << strerror(errno) << std::endl;
        close(server_fd);
        return -1;
    }

    return server_fd;
}

int Server::acceptClient(int server_fd, ServerBlock& server_block) {
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
    
    if (client_fd < 0) {
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            std::cerr << "Accept failed: " << strerror(errno) << std::endl;
        }
        return -1;
    }

    // Set new socket to non-blocking
    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

    // Disable Nagle's algorithm for better performance
    int nodelay = 1;
    setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));

    // Create client object with the correct ServerBlock
    clients.push_back(Client(client_fd, client_addr, server_block));
    
    // Add to polling structures
    struct pollfd new_pollfd;
    new_pollfd.fd = client_fd;
    new_pollfd.events = POLLIN;
    new_pollfd.revents = 0;
    
    pollfds.push_back(new_pollfd);
    pollfds_clients.push_back(new_pollfd);
    
    // Get the server index that accepted this client
    int server_index = getServerIndexByFd(server_fd);
    std::string server_info;
    
    if (server_index >= 0) {
        std::ostringstream ss;
        ss << server_configs[server_index].host << ":" << server_configs[server_index].port 
           << " (Server Block: " << server_configs[server_index].server_block_index
           << ", Name: " << server_configs[server_index].server_name << ")";
        server_info = ss.str();
    } else {
        server_info = "unknown server";
    }
    
    std::cout << "\033[38;5;214mNew client connected on " << server_info << " - Client FD: " << client_fd << "\033[0m" << std::endl;
    
    return client_fd;
}

void Server::closeClientConnection(size_t index) {
    // Check if the index is valid
    if (index >= pollfds.size()) {
        std::cerr << "\033[31mInvalid pollfd index in closeClientConnection\033[0m" << std::endl;
        return;
    }
    
    int client_fd = pollfds[index].fd;
    
    // Find the client by fd
    size_t client_index;
    getClientIndexByFd(client_fd, client_index);
    
    // Check if client was found
    if (client_index >= clients.size()) {
        // Client not found, just close the socket and remove from pollfds
        std::cout << "\033[31mClient not found in vector, closing socket FD: " << client_fd << "\033[0m" << std::endl;
        close(client_fd);
        pollfds.erase(pollfds.begin() + index);
        return;
    }
    
    // Store the keep-alive status before we remove the client
    bool keepAlive = (clients[client_index].get_Alive() == 1);
    
    // Close any open file streams
    std::ifstream& fileStream = clients[client_index].get_response().get_fileStream();
    if (fileStream.is_open()) {
        fileStream.close();
    }
    if (keepAlive == 0) {
        // Find client index in pollfds_clients
        size_t client_poll_index = SIZE_MAX;
        for (size_t i = 0; i < pollfds_clients.size(); i++) {
            if (pollfds_clients[i].fd == client_fd) {
                client_poll_index = i;
                break;
            }
        }
        
        // Remove from pollfds_clients if found
        if (client_poll_index != SIZE_MAX) {
            pollfds_clients.erase(pollfds_clients.begin() + client_poll_index);
        }
        
        // Remove from clients vector - careful with the order!
        clients.erase(clients.begin() + client_index);
        
        // Close socket and remove from pollfds
        std::cout << "\033[31mClosing client connection. Socket FD: " << client_fd << "\033[0m" << std::endl;
        close(client_fd);
        pollfds.erase(pollfds.begin() + index);
    } else {
        // If keep-alive is on, keep the connection open
        std::cout << "\033[35mClient connection kept alive. Socket FD: " << client_fd << "\033[0m" << std::endl;
        
        // Reset the client state for the next request but keep it in the vectors
        clients[client_index].reset();
        
        // Make sure we switch back to listening for new requests
        pollfds[index].events = POLLIN;
    }
}

void Server::closeServer() {
    // Close all client connections
    for (size_t i = 0; i < clients.size(); i++) {
        int client_fd = clients[i].get_client_id();
        
        // Close any open file streams
        std::ifstream& fileStream = clients[i].get_response().get_fileStream();
        if (fileStream.is_open()) {
            fileStream.close();
        }
        
        // Close socket
        std::cout << "Closing client connection: " << client_fd << std::endl;
        close(client_fd);
    }
    
    // Close all server sockets
    for (size_t i = 0; i < server_configs.size(); i++) {
        if (server_configs[i].fd >= 0) {
            std::cout << "Closing server socket: " << server_configs[i].fd << std::endl;
            close(server_configs[i].fd);
            server_configs[i].fd = -1;
        }
    }
    
    // Clear all containers
    pollfds.clear();
    pollfds_clients.clear();
    pollfds_servers.clear();
    clients.clear();
}

void Server::startServer() {
    // Initialize all server sockets
    initializeServers();
    
    std::cout << "All servers started and listening..." << std::endl;
    
    while (true) {
        // Wait for activity on any socket
        int poll_count = poll(pollfds.data(), pollfds.size(), -1);
        if (poll_count < 0) {
            std::cerr << "Poll failed: " << strerror(errno) << std::endl;
            continue;
        }
        
        // Iterate backwards to safely remove elements if needed
        for (int i = static_cast<int>(pollfds.size() - 1); i >= 0; i--) {
            size_t idx = static_cast<size_t>(i);
            
            // Skip invalid indices
            if (idx >= pollfds.size()) continue;
            
            // Handle errors
            if (pollfds[idx].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                // Check if this is a server or client fd
                bool is_server = false;
                for (size_t j = 0; j < pollfds_servers.size(); j++) {
                    if (pollfds[idx].fd == pollfds_servers[j].fd) {
                        is_server = true;
                        std::cerr << "Server socket error on fd " << pollfds[idx].fd << std::endl;
                        break;
                    }
                }
                
                if (!is_server) {
                    // Client socket error
                    closeClientConnection(idx);
                }
                continue;
            }
            
            // Handle incoming data/connections
            if (pollfds[idx].revents & POLLIN) {
                // Check if this is a server socket (accept new connection)
                bool is_server = false;
                for (size_t j = 0; j < server_configs.size(); j++) {
                    if (pollfds[idx].fd == server_configs[j].fd) {
                        is_server = true;
                        // Find matching ServerBlock for this server config
                        int server_block_idx = server_configs[j].server_block_index;
                        // Make sure index is valid
                        if (server_block_idx >= 0 && server_block_idx < static_cast<int>(server_block_obj.size())) {
                            // Pass the correct ServerBlock to acceptClient
                            acceptClient(server_configs[j].fd, server_block_obj[server_block_idx]);
                        } else {
                            std::cerr << "Invalid server_block_idx: " << server_block_idx << std::endl;
                        }
                        break;
                    }
                }
                
                if (!is_server) {
                    // Client socket - read data
                    int client_fd = pollfds[idx].fd;
                    
                    // Find the client by fd
                    size_t client_index;
                    getClientIndexByFd(client_fd, client_index);
                    
                    if (client_index >= clients.size()) {
                        // Client not found - close connection
                        closeClientConnection(idx);
                        continue;
                    }
                    
                    // Process client request
                    char buffer[16384] = {0};
                    ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
                    std::cout << "Bytes read: " << bytes_read << std::endl;
                    std::cout << "Buffer: " << buffer << std::endl;
                    if (bytes_read <= 0) {
                        if (bytes_read == 0) {
                            if (clients[client_index].get_Alive() == 0) {
                                std::cout << "Client disconnected. Socket FD: " << client_fd << "\033[0m" << std::endl;
                            }
                        } else {
                            std::cerr << "Recv error on fd " << client_fd << ": " << strerror(errno) << std::endl;
                        }
                        closeClientConnection(idx);
                        continue;
                    }
                    
                    // Process the request data
                    std::string req(buffer, bytes_read);
                    clients[client_index].get_request().set_s_request(req);
                    check_request(clients[client_index]);
                    
                    // If we've received all data, switch to write mode
                    if (clients[client_index].get_all_recv()) {
                        pollfds[idx].events = POLLOUT;
                    }
                }
            }
            // Handle outgoing data
            else if (pollfds[idx].revents & POLLOUT) {
                int client_fd = pollfds[idx].fd;
                
                // Find the client by fd
                size_t client_index;
                getClientIndexByFd(client_fd, client_index);
                
                if (client_index >= clients.size()) {
                    // Client not found - close connection
                    std::cerr << "No matching client found for fd: " << client_fd << std::endl;
                    closeClientConnection(idx);
                    continue;
                }
                
                Client& client = clients[client_index];
                
                // Send the response
                handleClientWrite(idx);
                
                // Check if file stream has ended
                if (client.get_response().get_fileStream().eof()) {
                    // Switch back to POLLIN for next request
                    pollfds[idx].events = POLLIN;
                    
                    // Check if we need to close the connection
                    if (!client.get_Alive()) {
                        closeClientConnection(idx);
                    } else {
                        // Reset client for the next request
                        client.reset();
                    }
                }
            }
        }
    }
}

void Server::handleClientWrite(size_t index) {
    if (index >= pollfds.size()) {
        std::cerr << "Invalid pollfd index in handleClientWrite" << std::endl;
        return;
    }
    
    int client_fd = pollfds[index].fd;
    
    // Find the client by fd
    size_t client_index;
    getClientIndexByFd(client_fd, client_index);
    
    if (client_index >= clients.size()) {
        std::cerr << "No matching client found for fd: " << client_fd << std::endl;
        closeClientConnection(index);
        return;
    }
    
    Client& client = clients[client_index];
    
    // Send the string response if not yet sent
    if (!client.get_request().is_string_req_send) {
        const std::string& response = client.get_response().get_response();
        ssize_t bytes_sent = send(client_fd, response.c_str(), response.size(), 0);
        std::cout << "Bytes sent: " << bytes_sent << std::endl;
        std::cout << "Response: " << response << std::endl;
        if (bytes_sent < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                return; // Retry in next POLLOUT
            }
            std::cerr << "Send error on fd " << client_fd << ": " << strerror(errno) << std::endl;
            closeClientConnection(index);
            return;
        }
        client.get_request().is_string_req_send = true;
    }
    
    // Send file content if available
    std::ifstream& fileStream = client.get_response().get_fileStream();
    if (fileStream.is_open() && !fileStream.eof()) {
        char buffer[4096];
        fileStream.read(buffer, sizeof(buffer));
        int bytes_read = fileStream.gcount();
        
        if (bytes_read > 0) {
            ssize_t bytes_sent = send(client_fd, buffer, bytes_read, MSG_NOSIGNAL);
            if (bytes_sent < 0) {
                if (errno == EPIPE || errno == ECONNRESET) {
                    std::cerr << "Client closed connection" << std::endl;
                    closeClientConnection(index);
                    return;
                } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // Would block, try again later
                    // Rewind the file position since we didn't send
                    fileStream.seekg(-bytes_read, std::ios::cur);
                    return;
                } else {
                    std::cerr << "Send error: " << strerror(errno) << std::endl;
                    closeClientConnection(index);
                    return;
                }
            } else if (bytes_sent < bytes_read) {
                // Partial send - rewind file position for the unsent portion
                fileStream.seekg(-(bytes_read - bytes_sent), std::ios::cur);
                return; // Try again later
            }
        }

        if (fileStream.eof()) {
            // File stream has ended, close it
            fileStream.close();
        }
    }
}

void Server::getClientIndexByFd(int fd, size_t& client_index) {
    client_index = clients.size(); // Default to invalid index
    
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].get_client_id() == fd) {
            client_index = i;
            break;
        }
    }
}

int Server::getServerIndexByFd(int fd) {
    for (size_t i = 0; i < server_configs.size(); i++) {
        if (server_configs[i].fd == fd) {
            return static_cast<int>(i);
        }
    }
    return -1; // Not found
}

int Server::getServerIndexByHostPortName(const std::string& host, int port, const std::string& server_name) {
    // First, check for exact match of host, port, and server name
    for (size_t i = 0; i < server_configs.size(); i++) {
        if (server_configs[i].host == host && 
            server_configs[i].port == port && 
            server_configs[i].server_name == server_name) {
            return static_cast<int>(i);
        }
    }
    
    // If not found, check for match with just host and port
    for (size_t i = 0; i < server_configs.size(); i++) {
        if (server_configs[i].host == host && server_configs[i].port == port) {
            return static_cast<int>(i);
        }
    }
    
    return -1; // Not found
}

ServerBlock Server::get_ServerConfByIndex(int fd_server) {
    int index = getServerIndexByFd(fd_server);
    if (index >= 0) {
        int server_block_idx = server_configs[index].server_block_index;
        if (server_block_idx >= 0 && server_block_idx < static_cast<int>(server_block_obj.size())) {
            return server_block_obj[server_block_idx];
        }
    }
    // Return a default ServerBlock if not found
    return ServerBlock();
}