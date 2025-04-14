#include "../webserver.hpp"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sstream>
#include <signal.h>

Server::Server() {
    // Add a default server configuration
    signal(SIGPIPE, SIG_IGN);
    // addServerConfig("localhost", "0.0.0.0", DEFAULT_PORT);
}

Server::~Server() {
    this->closeServer();
}

void Server::addServerConfig(const std::string& host, const std::string& ip, int port) {
    ServerConfig config(host, ip, port);
    server_configs.push_back(config);
}

void Server::setServerConfig(size_t index, const std::string& host, const std::string& ip, int port) {
    if (index >= server_configs.size()) {
        std::cerr << "Invalid server index in setServerConfig" << std::endl;
        return;
    }
    
    server_configs[index].host = host;
    server_configs[index].ip = ip;
    server_configs[index].port = port;
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
    static int server_idx = 0;
    for (size_t i = 0; i < server_configs.size(); i++) {
        ServerConfig& config = server_configs[i];
        
        // Create, bind, and listen on each server
        config.fd = createServer(config);
        config.server_index = server_idx++;
        
        if (config.fd >= 0) {
            struct pollfd server_poll;
            server_poll.fd = config.fd;
            server_poll.events = POLLIN;
            server_poll.revents = 0;
            
            pollfds.push_back(server_poll);
            pollfds_servers.push_back(server_poll);
            
            std::cout << "\033[33mServer initialized on " << config.host << ":" << config.port 
                      << " (FD: " << config.fd << ")\033[0m" << std::endl;
        }
    }
}

int Server::createServer(ServerConfig& config) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Socket creation failed for " << config.host << ":" << config.port 
                  << " - " << strerror(errno) << std::endl;
        return -1;
    }

    // Set socket to non-blocking mode
    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        std::cerr << "Setsockopt failed for " << config.host << ":" << config.port 
                  << " - " << strerror(errno) << std::endl;
        close(server_fd);
        return -1;
    }

    // Set up address structure
    memset(&config.addr, 0, sizeof(config.addr));
    config.addr.sin_family = AF_INET;
    config.addr.sin_port = htons(config.port);
    
    // Convert IP address string to network format
    if (inet_pton(AF_INET, config.ip.c_str(), &config.addr.sin_addr) <= 0) {
        std::cerr << "Invalid IP address: " << config.ip << " - " << strerror(errno) << std::endl;
        close(server_fd);
        return -1;
    }

    // Bind the socket
    if (bind(server_fd, (struct sockaddr*)&config.addr, sizeof(config.addr)) < 0) {
        std::cerr << "Bind failed for " << config.host << ":" << config.port 
                  << " - " << strerror(errno) << std::endl;
        close(server_fd);
        return -1;
    }

    // Listen for connections
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        std::cerr << "Listen failed for " << config.host << ":" << config.port 
                  << " - " << strerror(errno) << std::endl;
        close(server_fd);
        return -1;
    }

    return server_fd;
}

// Remove unused parameters to avoid warnings
void Server::bindServer(ServerConfig& /* config */) {
    // Already handled in createServer
}

void Server::listenServer(ServerConfig& /* config */) {
    // Already handled in createServer
}

int Server::acceptClient(int server_fd, struct sockaddr_in& , ServerBlock & server_block_obj) {
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    
    std::cout << server_block_obj.get_client_body_size() << std::endl;
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

    // Create client object first
    // std::cout << server_block_obj[0].get_port() << std::endl;
    
    clients.push_back(Client(client_fd, client_addr, server_block_obj));
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
        ss << server_configs[server_index].host << ":" << server_configs[server_index].port;
        server_info = ss.str();
    } else {
        server_info = "unknown server";
    }
    
    std::cout << "\033[38;5;214m" << "New client connected on " << server_info
              << " - Client FD: " << client_fd << "\033[0m" << std::endl;
    
    return client_fd;
}

void Server::closeClientConnection(size_t index) {
    if (index >= pollfds.size()) {
        std::cerr << "\033[31mInvalid pollfd index in closeClientConnection\033[0m" << std::endl;
        return;
    }
    
    int client_fd = pollfds[index].fd;
    
    // Find the client by fd
    size_t client_index;
    getClientIndexByFd(client_fd, client_index);
    
    // Clean up resources if client found
    if (client_index < clients.size()) {
        // Close any open file streams
        std::ifstream& fileStream = clients[client_index].get_response().get_fileStream();
        if (fileStream.is_open()) {
            fileStream.close();
        }
        
        // Find and remove from pollfds_clients
        for (size_t i = 0; i < pollfds_clients.size(); i++) {
            if (pollfds_clients[i].fd == client_fd) {
                pollfds_clients.erase(pollfds_clients.begin() + i);
                break;
            }
        }
        
        // Remove from clients vector
        clients.erase(clients.begin() + client_index);
    }
    
    // Close socket and remove from main pollfds
    std::cout << "\033[31m from clodse function Closing client connection. Socket FD: " << client_fd << "\033[0m" << std::endl;
    close(client_fd);
    pollfds.erase(pollfds.begin() + index);
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
                        // For server sockets, we might want to try to recreate them
                        break;
                    }
                }
                
                if (!is_server) {
                    // Client socket error
                    std::cerr << "Client socket error on fd " << pollfds[idx].fd << std::endl;
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
                        acceptClient(server_configs[j].fd, server_configs[j].addr, server_block_obj[j]);

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
                        std::cerr << "No matching client found for fd: " << client_fd << std::endl;
                        closeClientConnection(idx);
                        continue;
                    }
                    
                    // Process client request
                    char buffer[16384] = {0};
                    ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
                    // std::cout << "Received data from client: " << buffer << std::endl;
                    // exit(1);
                    if (bytes_read <= 0) {
                        if (bytes_read == 0) {
                            std::cout << "\033[31mFrom rcv Client disconnected. Socket FD: " << client_fd << "\033[0m" << std::endl;
                        } else {
                            std::cerr << "Recv error on fd " << client_fd << ": " << strerror(errno) << std::endl;
                        }
                        std::cout << bytes_read << " hereeeeeeeeeeeeeeeeeeeee" << std::endl;
                        closeClientConnection(idx);
                        continue;
                    }
                    
                    // Process the request data
                    std::string req(buffer, bytes_read);
                    clients[client_index].get_request().set_s_request(req);
                    check_request(clients[client_index]);


                    
                    // clients[client_index].set_Alive();
                    
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
                // std::cout << idx << " Sending response to client: " << client_fd << std::endl;
                
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
                        // closeClientConnection(idx);
                    }
                }
            }
        }
    }
}


// void Server::handleClientRead(size_t /* index */) {
//     // This functionality is now integrated in startServer method
// }

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
        std::cout << " >>" << response << std::endl;
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
        char buffer[8192];
        fileStream.read(buffer, sizeof(buffer));
        int bytes_read = fileStream.gcount();
        
        if (bytes_read > 0) {
            // Check if socket is still writable
            struct pollfd pfd;
            pfd.fd = client_fd;
            pfd.events = POLLOUT;
            pfd.revents = 0;
            
            if (poll(&pfd, 1, 0) <= 0 || (pfd.revents & (POLLERR | POLLHUP | POLLNVAL))) {
                std::cerr << "Socket error detected before send" << std::endl;
                closeClientConnection(index);
                return;
            }
            
            // Socket is writable, proceed with send
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


// ServerBlock Server::get_ServerConfByIndex(int index)
// {
//     for (size_t i = 0; i < ; i++)
//     {
//         /* code */
//     }
    
// }



