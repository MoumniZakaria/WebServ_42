# include "../webserver.hpp"
# include <fcntl.h>
# include <sys/ioctl.h>

Server::Server() {
    this->fd_server = createServer();
    struct pollfd server_poll;
    server_poll.fd = this->fd_server;
    server_poll.events = POLLIN;
    server_poll.revents = 0;
    this->pollfds.push_back(server_poll);
    this->bindServer();
    this->listenServer();
}

Server::~Server() {
    this->closeServer();
}

int Server::createServer()
{
    this->fd_server = socket(AF_INET, SOCK_STREAM, 0);
    if (this->fd_server < 0)
    {
        throw std::runtime_error("Socket creation failed: " + std::string(strerror(errno)));
    }

    // Set socket to non-blocking mode
    int flags = fcntl(this->fd_server, F_GETFL, 0);
    fcntl(this->fd_server, F_SETFL, flags | O_NONBLOCK);

    // Set socket options
    int opt = 1;
    if (setsockopt(this->fd_server, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
    {
        throw std::runtime_error("Setsockopt failed: " + std::string(strerror(errno)));
    }

    return this->fd_server;
}

void Server::bindServer()
{
    memset(&this->addr_server, 0, sizeof(this->addr_server));
    this->addr_server.sin_family = AF_INET;
    this->addr_server.sin_addr.s_addr = INADDR_ANY;
    this->addr_server.sin_port = htons(PORT);

    if (bind(this->fd_server, (struct sockaddr *)&this->addr_server, sizeof(this->addr_server)) < 0)
    {
        throw std::runtime_error("Bind failed: " + std::string(strerror(errno)));
    }
}

void Server::listenServer()
{
    if (listen(this->fd_server, MAX_CLIENTS) < 0)
    {
        throw std::runtime_error("Listen failed: " + std::string(strerror(errno)));
    }
}

int Server::acceptClient()
{
    struct sockaddr_in addr_client;
    socklen_t addrlen = sizeof(addr_client);
    
    int new_socket = accept(this->fd_server, (struct sockaddr *)&addr_client, &addrlen);
    
    if (new_socket < 0)
    {
        if (errno != EWOULDBLOCK && errno != EAGAIN)
        {
            std::cerr << "Accept failed: " << strerror(errno) << std::endl;
        }
        return -1;
    }

    // Set new socket to non-blocking
    int flags = fcntl(new_socket, F_GETFL, 0);
    fcntl(new_socket, F_SETFL, flags | O_NONBLOCK);

    // Disable Nagle's algorithm for better performance
    int nodelay = 1;
    setsockopt(new_socket, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));

    struct pollfd new_pollfd;
    new_pollfd.fd = new_socket;
    new_pollfd.events = POLLIN;
    new_pollfd.revents = 0;

    this->pollfds.push_back(new_pollfd);
    this->Clients.push_back(Client(new_socket, addr_client));
    
    std::cout << "\033[38;5;214m" << "****New client connected. Socket FD: " << new_socket << "\033[0m" << std::endl;
    return 0;
}


void Server::closeClientConnection(size_t index)
{
    // Client& client = this->Clients[index - 1];
    std::cout << "size befor" << this->pollfds.size() << std::endl;
    std::cout << "size befor" << this->Clients.size() << std::endl;
    std::cout << this->pollfds[index].fd << std::endl;
    close(this->pollfds[index].fd);
    this->pollfds.erase(this->pollfds.begin() + index);
    this->Clients.erase(this->Clients.begin() + index - 1);
    std::cout << "size after" << this->pollfds.size() << std::endl;
    std::cout << "size after" << this->Clients.size() << std::endl;
}

void Server::closeServer()
{
    for ( size_t i = 0; i < this->pollfds.size(); i++)
    {
        close(this->pollfds[i].fd);
    }
    this->pollfds.clear();
    this->Clients.clear();
}
//************************************************************************************ */

void Server::startServer() {
    while (true) {
        int poll_count = poll(this->pollfds.data(), this->pollfds.size(), -1);
        if (poll_count < 0) {
            std::cerr << "Poll failed: " << strerror(errno) << std::endl;
            continue;
        }

        for (size_t i = 0; i < this->pollfds.size(); ++i) {
            if (this->pollfds[i].revents & POLLERR) {
                std::cerr << "Error on socket: " << this->pollfds[i].fd << std::endl;
                closeClientConnection(i);
                continue;
            }

            if (this->pollfds[i].revents & POLLIN) {
                if (this->pollfds[i].fd == this->fd_server) {
                    if (acceptClient() != 0) {
                        std::cerr << "Failed to accept client connection" << std::endl;
                    }

                    if (this->pollfds[i].fd != 3) {
                        std::cout << "client fd 1: " << this->pollfds[i].fd << std::endl;
                        std::cout << "client fd 2: " << this->Clients[i - 1].get_client_id() << std::endl;
                    }
                    
                } else {
                    handleClientRead(i);
                    // Prepare to send response by switching to POLLOUT
                    if (this->Clients[i - 1].get_all_recv() == true) {
                        std::cout << "heerrre" << std::endl;
                        this->pollfds[i].events = POLLOUT;
                        // this->Clients[i - 1].set_all_recv(false);
                    }
                }
            }

            if (this->pollfds[i].revents & POLLOUT) {
                // this->Clients[i - 1].set_all_recv(true);
                handleClientWrite(i);
                this->Clients[i - 1].reset();
                // check if responce end pollin
                // if (this->Clients[i - 1].get_response().get_fileStream().eof()) {
                //     this->pollfds[i].events = POLLIN;
                //     if (this->Clients[i - 1].get_Alive() == false) {
                //         closeClientConnection(i);
                //     } else {
                //         this->Clients[i - 1].reset();
                //     }
                // }
                // Reset the event to POLLIN after sending response
                // if (this->Clients[i - 1].get_all_recv() == false) {
                //     this->pollfds[i].events = POLLIN;
                // }
            }
        }
    }
}

void Server::handleClientRead(size_t index)
{
    // std::cout << index << std::endl;
    char buffer[1024] = {0};
    int client_fd = this->pollfds[index].fd;
    Client& client = this->Clients[index - 1];
            (void)client;
    
    // Read data from client
    ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    std::cout << "bytes read : " << buffer << std::endl;
    
    if (bytes_read <= 0)
    {
        // Connection closed or error"
        if (bytes_read == 0 && client.get_Alive() == true)
        {
            std::cout << "\033[1;31m" << " client still alive and read all re " << client_fd << "\033[0m" << std::endl;
        }
        else 
        {
            std::cerr << "Recv error: " << strerror(errno) << std::endl;
            if (client.get_Alive() == false)
                closeClientConnection(index);
            return;
        }
        return;
    }
    std::string req(buffer, bytes_read);
    std::cout << "\033[1;32m" << req << "\033[0m" << std::endl;
    this->Clients[index - 1].get_request().set_s_request(req);
    check_request(this->Clients[index - 1]);

}

void Server::handleClientWrite(size_t index) {
    Client& client = this->Clients[index - 1];
    int client_fd = this->pollfds[index].fd;

    // Send the string response if not yet sent
    if (!client.get_request().is_string_req_send) {
        const std::string& response = client.get_response().get_response();
        ssize_t bytes_sent = send(client_fd, response.c_str(), response.size(), 0);
        if (bytes_sent < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                return; // Retry in next POLLOUT
            }
            std::cerr << "Send error: " << strerror(errno) << std::endl;
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
            ssize_t bytes_sent = send(client_fd, buffer, bytes_read, 0);
            if (bytes_sent < 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    return; // Retry later
                }
                std::cerr << "Send error: " << strerror(errno) << std::endl;
                closeClientConnection(index);
                return;
            }
        }



    //     if (fileStream.eof()) {
    //         if (client.get_Alive())
    //         {
    //             // Keep connection alive; reset after file response
    //             client.reset();
    //             client.set_all_recv(false);
    //             this->pollfds[index].events = POLLIN;
    //         } else {
    //             // Close connection after file response
    //             closeClientConnection(index);
    //             fileStream.close();
    //         }
    //     }
    // } else {
    //     // No file to send; reset after string response
    //     client.reset();
    //     client.set_all_recv(false);
    //     this->pollfds[index].events = POLLIN;
    }
        if (client.get_response().get_fileStream().eof()) {
            this->pollfds[index].events = POLLIN;
            std::cout << " keep alive is set to : " << client.get_Alive() << std::endl;
        if (!client.get_Alive()) {
            closeClientConnection(index);
        } else {
            client.reset();
        }
    }
}