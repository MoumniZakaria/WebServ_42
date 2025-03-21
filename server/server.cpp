#include "../webserver.hpp"
#include <fcntl.h>
#include <sys/ioctl.h>

Server::Server()
{
    this->fd_server = createServer();
    
    struct pollfd server_poll;
    server_poll.fd = this->fd_server;
    server_poll.events = POLLIN;
    server_poll.revents = 0;
    
    this->pollfds.push_back(server_poll);
    this->bindServer();
    this->listenServer();
}

Server::~Server()
{
    this->closeServer();
}

void Server::startServer()
{
    while (true)
    {
        int poll_count = poll(this->pollfds.data(), this->pollfds.size(), -1);
        if (poll_count < 0)
        {
            std::cerr << "Poll failed: " << strerror(errno) << std::endl;
            continue;
        }
        
        // Iterate through pollfds to handle events
        for (size_t i = 0; i < this->pollfds.size(); ++i)
        {
            if (this->pollfds[i].revents & POLLERR)
            {
                // Handle error condition
                std::cerr << "Error on socket: " << this->pollfds[i].fd << std::endl;
                closeClientConnection(i);
                continue;
            }

            if (this->pollfds[i].revents & POLLIN)
            {
                if (this->pollfds[i].fd == this->fd_server)
                {
                    // New connection
                    if (acceptClient() != 0)
                    {
                        std::cerr << "Failed to accept client connection" << std::endl;
                    }
                    
                }
                else 
                {
                    // Existing client data
                    handleClientRead(i);
                    this->Clients[i - 1].set_all_recv(true);
                }
            }
            if ( i != 0 && this->Clients[i - 1].get_all_recv())
            {
                std::cout << "Ready to send response" << std::endl;
                // Ready to send response
                handleClientWrite(i);
                if (this->pollfds[i].revents & POLLOUT)
                {
                    // Handle outgoing data
                }
                // check this if it is non stopping sending of data
            }
        }
    }
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
    
    std::cout << "New client connected. Socket FD: " << new_socket << std::endl;
    return 0;
}

void Server::handleClientRead(size_t index)
{
    char buffer[1024] = {0};
    int client_fd = this->pollfds[index].fd;
    
    // Read data from client
    ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_read <= 0)
    {
        // Connection closed or error
        if (bytes_read == 0)
        {
            std::cout << "Client disconnected. Socket FD: " << client_fd << std::endl;
        }
        else 
        {
            std::cerr << "Recv error: " << strerror(errno) << std::endl;
        }
        closeClientConnection(index);
        return;
    }

    std::string req(buffer, bytes_read);

    // Set request data
    this->Clients[index - 1].get_request().set_s_request(req);
    std::cout << "Request2: " << this->Clients[index - 1].get_request().get_s_request() << std::endl;
    check_request(this->Clients[index - 1]);



    // Prepare to send response
    // if (this->Clients[index - 1].get_all_recv())
    // {
    //     this->pollfds[index].events = POLLOUT;
    // }
}

void Server::handleClientWrite(size_t index)
{
    Client& client = this->Clients[index - 1];
    int client_fd = this->pollfds[index].fd;
    
    std::string response = client.get_response().get_response();
    // std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 12\r\n\r\nHello World!";
    std::ifstream &fileStream = client.get_response().get_fileStream();
    // std::string file ;
    
    // Send headers
    ssize_t sent = send(client_fd, response.c_str(), response.length(), 0);
    if (sent < 0)
    {
        std::cerr << "Send error: " << strerror(errno) << std::endl;
        closeClientConnection(index);
        return;
    }

    // If file exists, send file contents
    if (fileStream.is_open())
    {
        // std::cout << "here" << std::endl;
        char file_buffer[1024];
        while (fileStream.read(file_buffer, sizeof(file_buffer) -1))
        {
            ssize_t bytes_sent = send(client_fd, file_buffer, fileStream.gcount(), 0);
            if (bytes_sent < 0)
            {
                std::cerr << "File send error: " << strerror(errno) << std::endl;
                closeClientConnection(index);
                return;
            }
        }
        // Send any remaining bytes
        if (fileStream.gcount() > 0)
        {
            ssize_t bytes_sent = send(client_fd, file_buffer, fileStream.gcount(), 0);
            if (bytes_sent < 0)
            {
                std::cerr << "Final file send error: " << strerror(errno) << std::endl;
                closeClientConnection(index);
                return;
            }
        }
        fileStream.close();
    }

    // Close connection if not keep-alive
    if (!client.get_Alive())
    {
        closeClientConnection(index);
    }
    else
    {
        // Reset for next request
        this->pollfds[index].events = POLLIN;
    }
}

void Server::closeClientConnection(size_t index)
{
    Client& client = this->Clients[index - 1];

    if (client.get_Alive())
    {
        // client.set_Alive(false);
        // this->pollfds[index].events = POLLIN;
        return;
    }
    close(this->pollfds[index].fd);
    this->pollfds.erase(this->pollfds.begin() + index);
    this->Clients.erase(this->Clients.begin() + index - 1);
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