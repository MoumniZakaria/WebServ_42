#include "../webserver.hpp"

Server::Server() {
    
    signal(SIGPIPE, SIG_IGN);
}

Server::~Server() {
    this->closeServer();
}

void Server::addServerConfig(int server_idx, const std::string& host, std::vector<int> ports, const std::string& server_name) {

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
        
        config.fd = createServer(config);
        
        if (config.fd >= 0) {
            
            struct pollfd server_poll;
            server_poll.fd = config.fd;
            server_poll.events = POLLIN;  
            server_poll.revents = 0;      
            
            pollfds.push_back(server_poll);
            pollfds_servers.push_back(server_poll);
            
            std::cout << "\033[38;5;208mServer initialized on " << config.host << ":" << config.port 
                      << " (FD: " << config.fd << ")\033[0m" << std::endl;
        }
    }
}

int Server::createServer(ServerConfig& config) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Socket creation failed for " << config.host << ":" << config.port << std::endl;
        return -1;
    }

    int flags = fcntl(server_fd, F_GETFL, 0);
    if (flags < 0) {
        std::cerr << "Failed to get socket flags" << std::endl;
        close(server_fd);
        return -1;
    }
    
    if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        std::cerr << "Failed to set non-blocking mode" << std::endl;
        close(server_fd);
        return -1;
    }
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Failed to set SO_REUSEADDR" << std::endl;
        close(server_fd);
        return -1;
    }
    memset(&config.addr, 0, sizeof(config.addr));
    config.addr.sin_family = AF_INET;
    config.addr.sin_port = htons(config.port);
    config.addr.sin_addr.s_addr = inet_addr(config.host.c_str());
    
    if (bind(server_fd, (struct sockaddr*)&config.addr, sizeof(config.addr)) < 0) {
        std::cerr << "Bind failed for " << config.host << ":" << config.port << std::endl;
        close(server_fd);
        // check if we need to close all server if one of them does not bind
        return -1;
    }

    if (listen(server_fd, SOMAXCONN) < 0) {
        std::cerr << "Listen failed for " << config.host << ":" << config.port << std::endl;
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
        return -1;
    }

    int flags = fcntl(client_fd, F_GETFL, 0);
    if (flags < 0 || fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        std::cerr << "Failed to set client socket to non-blocking mode" << std::endl;
        close(client_fd);
        return -1;
    }

    int nodelay = 1;
    setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
    clients.push_back(Client(client_fd, client_addr, server_block));
    
    struct pollfd new_pollfd;
    new_pollfd.fd = client_fd;
    new_pollfd.events = POLLIN;  // Start by reading the HTTP request
    new_pollfd.revents = 0;      // Reset event flags

    pollfds.push_back(new_pollfd);
    pollfds_clients.push_back(new_pollfd);
    
    std::cout << "\033[33mNew client connected - FD: " << client_fd << "\033[0m" << std::endl;
    
    return client_fd;
}

void Server::closeClientConnection(size_t index) {
    
    if (index >= pollfds.size()) {
        // std::cerr << "Invalid pollfd index in closeClientConnection" << std::endl;
        return;
    }
    
    int client_fd = pollfds[index].fd;
    
    size_t client_index;
    getClientIndexByFd(client_fd, client_index);
    
    if (client_index >= clients.size()) {
        // std::cout << "Client not found in vector, closing socket FD: " << client_fd << std::endl;
        close(client_fd);
        pollfds.erase(pollfds.begin() + index);
        return;
    }
    bool keepAlive = (clients[client_index].get_Alive() == 1);
    std::ifstream& fileStream = clients[client_index].get_response().get_fileStream();
    if (fileStream.is_open()) {
        fileStream.close();
    }
    if (keepAlive == 0) {
        for (size_t i = 0; i < pollfds_clients.size(); i++) {
            if (pollfds_clients[i].fd == client_fd) {
                pollfds_clients.erase(pollfds_clients.begin() + i);
                break;
            }
        }
        Request *request = &clients[client_index].get_request();
        delete request;
        request = NULL;
        Response *response = &clients[client_index].get_response();
        delete response;
        response = NULL;
        clients.erase(clients.begin() + client_index);
        std::cout << "\033[31mClosing client connection. Socket FD: " << client_fd << "\033[0m" << std::endl;
        close(client_fd);
        pollfds.erase(pollfds.begin() + index);
    } else {
        clients[client_index].reset();
        pollfds[index].events = POLLIN;
    }
}

void Server::closeServer() {
    for (size_t i = 0; i < clients.size(); i++) {
        int client_fd = clients[i].get_client_id();
        
        std::ifstream& fileStream = clients[i].get_response().get_fileStream();
        if (fileStream.is_open()) {
            fileStream.close();
        }
        close(client_fd);
    }
    for (size_t i = 0; i < server_configs.size(); i++) {
        if (server_configs[i].fd >= 0) {
            std::cout << "Closing server socket: " << server_configs[i].fd << std::endl;
            close(server_configs[i].fd);
            server_configs[i].fd = -1;
        }
    }
    pollfds.clear();
    pollfds_clients.clear();
    pollfds_servers.clear();
    clients.clear();
}

void Server::startServer() {
    initializeServers();
    while (true) {
        int poll_count = poll(pollfds.data(), pollfds.size(), -1);
        if (poll_count < 0) {
            // std::cerr << "Poll failed" << std::endl;
            continue;
        }
        for (int i = static_cast<int>(pollfds.size() - 1); i >= 0; i--) {
            size_t idx = static_cast<size_t>(i);
            
            if (idx >= pollfds.size()) continue;
    
            if (pollfds[idx].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                // Determine if this is a server or client socket
                bool is_server = false;
                for (size_t j = 0; j < pollfds_servers.size(); j++) {
                    if (pollfds[idx].fd == pollfds_servers[j].fd) {
                        is_server = true;
                        // std::cerr << "Server socket error on fd " << pollfds[idx].fd << std::endl;
                        break;
                    }
                }
                
                if (!is_server) {
                    closeClientConnection(idx);
                }
                continue;
            }
            if (pollfds[idx].revents & POLLIN) {
                bool is_server = false;
                for (size_t j = 0; j < server_configs.size(); j++) {
                    if (pollfds[idx].fd == server_configs[j].fd) {
                        is_server = true;
                        
                        int server_block_idx = server_configs[j].server_block_index;
                        if (server_block_idx >= 0 && server_block_idx < static_cast<int>(server_block_obj.size())) {
                            acceptClient(server_configs[j].fd, server_block_obj[server_block_idx]);
                        } else {
                            std::cerr << "Invalid server_block_idx: " << server_block_idx << std::endl;
                        }
                        break;
                    }
                }
                
                if (!is_server) {
                    int client_fd = pollfds[idx].fd;
                    
                    // Find the corresponding client object
                    size_t client_index;
                    getClientIndexByFd(client_fd, client_index);
                    
                    if (client_index >= clients.size()) {
                        // not found
                        closeClientConnection(idx);
                        continue;
                    }
                    char buffer[16384] = {0};
                    ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

                    if (bytes_read <= 0) {
                        if (bytes_read == 0) {
                            std::cout << "Client disconnected. Socket FD: " << client_fd << std::endl;
                        } else {
                            std::cerr << "Recv error on fd " << client_fd << std::endl;
                        }
                        closeClientConnection(idx);
                        continue;
                    }

                    std::string req(buffer, bytes_read);
                    clients[client_index].get_request().set_s_request(req);
                    
                    // Process the HTTP request
                    check_request(clients[client_index]);
                    if (clients[client_index].get_all_recv()) {
                        pollfds[idx].events = POLLOUT;
                    }
                }
            }
            else if (pollfds[idx].revents & POLLOUT) {
                int client_fd = pollfds[idx].fd;
                size_t client_index;
                getClientIndexByFd(client_fd, client_index);
                
                if (client_index >= clients.size()) {
                    // not found
                    // std::cerr << "No matching client found for fd: " << client_fd << std::endl;
                    closeClientConnection(idx);
                    continue;
                }
                
                Client& client = clients[client_index];
                handleClientWrite(idx);
                if (!client.get_response().get_fileStream().is_open() ||  client.get_response().get_fileStream().eof()) {
                    pollfds[idx].events = POLLIN;
                    
                    // Handle connection closure or keep-alive
                    if (!client.get_Alive()) {
                        closeClientConnection(idx);
                    } else {
                        // Reset client state for the next request
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
    
    size_t client_index;
    getClientIndexByFd(client_fd, client_index);
    
    if (client_index >= clients.size()) {
        // std::cerr << "No matching client found for fd: " << client_fd << std::endl;
        closeClientConnection(index);
        return;
    }
    
    Client& client = clients[client_index];
    if (!client.get_request().is_string_req_send) {
        const std::string& response = client.get_response().get_response();
        ssize_t bytes_sent = send(client_fd, response.c_str(), response.size(), 0);
        if (bytes_sent <= 0) {
            closeClientConnection(index);
            return;
        }
        // set that the string request has been sent
        client.get_request().is_string_req_send = true;
    }
    std::ifstream& fileStream = client.get_response().get_fileStream();
    if (fileStream.is_open() && !fileStream.eof()) {
        char buffer[4096];
        fileStream.read(buffer, sizeof(buffer));
        int bytes_read = fileStream.gcount();
        
        if (bytes_read > 0) {
            // ssize_t bytes_sent = send(client_fd, buffer, bytes_read, MSG_NOSIGNAL);
            ssize_t bytes_sent = send(client_fd, buffer, bytes_read, 0);
            if (bytes_sent <= 0) {
                closeClientConnection(index);
                return;
            }
        }
        if (fileStream.eof()) {
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