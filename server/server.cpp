#include "../webserver.hpp"


Server::Server()
{
    this->fd_server = createServer();
    struct tmp_pollfd;

    struct pollfd tmp_pollfd;
    
    tmp_pollfd.fd = this->fd_server;
    tmp_pollfd.events = POLLIN;
    tmp_pollfd.revents = 0;
    
    this->pollfds.push_back(tmp_pollfd);
    this->bindServer();
    this->listenServer();
}
Server::Server(int fd)
{
    (void)fd;
    this->fd_server = createServer();
    struct tmp_pollfd;

    struct pollfd tmp_pollfd;
    
    tmp_pollfd.fd = this->fd_server;
    tmp_pollfd.events = POLLIN;
    tmp_pollfd.revents = 0;
    
    this->pollfds.push_back(tmp_pollfd);
    this->bindServer();
    this->listenServer();
}

Server::~Server()
{
    this->closeServer();
}


int Server::get_fd_server()
{
    return this->fd_server;
}

void Server::set_fd_server(int fd)
{
    this->fd_server = fd;
}

void Server::set_addr_server(struct sockaddr_in addr)
{
    this->addr_server = addr;
}

struct sockaddr_in Server::get_addr_server()
{
    return this->addr_server;
}

void Server::startServer()
{

    while (true)
    {
        int poll_count = poll(this->pollfds.data(), this->pollfds.size(), -1);
        if (poll_count < 0)
        {
            std::cout << "poll faild" << std::endl;
            continue;
            // throw std::runtime_error("poll failed");
        }
        
        // Iterate in reverse to safely handle removals
        for (int i = static_cast<int>(this->pollfds.size()) - 1; i >= 0; --i)
        {
            std::cout << "another ONE" << std::endl;
            // std::ifstream fileStream;
            // Request req = Request();
            // Response res = Response();
            // std::string response;

            // this->Clients[i].set_response(res);
            // res.set_fileStream(fileStream);
            // res.set_response(response);
            // this->Clients[i].set_request(req);

            if (this->pollfds[i].revents & POLLIN)
            {


                if (this->pollfds[i].fd == this->fd_server)
                {
                    // Handle new connection
                    std::cout << "New Connection added successfully" << std::endl;
                    if (this->acceptClient() == 1)
                    {
                        std::cerr << "Accept failed" << std::endl;
                        continue;
                    }
                    else
                    {

                    }
                }
                else {
                    char buffer[5000] = {0};
                    int valread = recv(this->pollfds[i].fd, buffer, sizeof(buffer)-1, 0);
                    std::cout <<  "val of recv          " << valread << std::endl;


                    sleep(1);
                    if (valread <= 0)
                    {
                        if (!this->Clients[i - 1].get_Alive())
                        {
                            std::cout << "Connection closed for fd " << this->pollfds[i].fd << std::endl;
                            close(this->pollfds[i].fd);
                            this->pollfds.erase(this->pollfds.begin() + i);
                            this->Clients.erase(this->Clients.begin() + i);
                        }
                        continue;
                       
                    }
                    
                    else if (valread > 0)
                    {
                        // this->Clients[i - 1].get_request().set_s_request(buffer);
                        // std::cout << this->Clients[i - 1].get_request().get_s_request() << std::endl;
                        check_request(this->Clients[i - 1]);

                        // std::cout << "\n------------------------------------" << std::endl;
                        // std::cout << "from : " << this->pollfds[i].fd << std::endl;
                        // std::cout << "Received: " << buffer << std::endl;
                        std::cout << "------------------------------------" << std::endl;

                        // std::string response = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
                        // send(this->pollfds[i].fd, response.c_str(), response.size(), 0);
                    }
             
        
                }
            }
        }
    }
}

int Server::createServer()
{
    this->fd_server = socket(AF_INET, SOCK_STREAM, 0);
    if (this->fd_server == 0)
    {
        throw std::runtime_error("socket failed");
    }
    int opt = 1;
    if (setsockopt(this->fd_server, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        throw std::runtime_error("setsockopt failed");
    }
    return this->fd_server;
}

void Server::bindServer()
{
    memset((struct sockaddr *)&this->addr_server, 0, sizeof(this->addr_server));

    this->addr_server.sin_family = AF_INET;
    this->addr_server.sin_addr.s_addr = INADDR_ANY;
    this->addr_server.sin_port = htons(PORT);

    if (bind(this->fd_server, (struct sockaddr *)&this->addr_server, sizeof(this->addr_server)) < 0)
    {
        throw std::runtime_error("bind failed");
    }
    
}

void Server::listenServer()
{
    if (listen(this->fd_server, MAX_CLIENTS) < 0)
    {
        throw std::runtime_error("listen failed");
    }
}

int  Server::acceptClient()
{
    int new_socket;
    struct sockaddr_in addr_client;
    int addrlen = sizeof(addr_client);
    new_socket = accept(this->fd_server, (struct sockaddr *)&addr_client, (socklen_t *)&addrlen);
    if (new_socket < 0)
    {
        return 1;
        throw std::runtime_error("accept failed");
    }

    struct pollfd new_fd;
    new_fd.fd = new_socket;
    new_fd.events = POLLIN;
    this->pollfds.push_back(new_fd);
    this->Clients.push_back(Client(new_socket, addr_client));
    Clients[Clients.size() - 1].set_Alive(true);

    return 0;
}

void Server::closeServer()
{
    close(this->fd_server);
}

// \r\n\r\n