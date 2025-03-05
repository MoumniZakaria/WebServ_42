#include "webserver.hpp"

// // #define PORT 4444

// bool setupSocket(int &server_fd, struct sockaddr_in &server_addr)
// {
//     server_fd = socket(AF_INET, SOCK_STREAM, 0);
//     if (server_fd == -1)
//     {
//         std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
//         return false;
//     }

//     // Set socket options
//     int opt = 1;
//     if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
//     {
//         std::cerr << "Setsockopt failed: " << strerror(errno) << std::endl;
//         return false;
//     }

//     server_addr.sin_family = AF_INET;
//     server_addr.sin_addr.s_addr = INADDR_ANY;
//     server_addr.sin_port = htons(PORT);

//     if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
//     {
//         std::cerr << "Bind failed: " << strerror(errno) << std::endl;
//         return false;
//     }

//     if (listen(server_fd, 10) == -1)
//     {
//         std::cerr << "Listen failed: " << strerror(errno) << std::endl;
//         return false;
//     }

//     return true;
// }

// // int hex_to_int(std::string line){
// //     int result = 0;
// //     std::stringstream ss;
// //     ss << std::hex << line;
// //     ss >> result;
// //     return result;
// // }

// // void save_chunked_data_to_file(std::string& chunked_data) {
// //     static int i ;
// //     if (!i){
// //         int pos = chunked_data.find("\r\n\r\n");
// //         chunked_data = chunked_data.substr(pos + 4);
// //     }
// //     i = 10;
// //     std::cout << chunked_data  <<std::endl;
// //     std::cout <<" ------------------------------------ " << std::endl;
// //     return ;
// //     std::stringstream data_stream(chunked_data);  
// //     std::ofstream output_file("output_data.txt");

// //     if (!output_file.is_open()) {
// //         std::cerr << "Error opening file for writing." << std::endl;
// //         return;
// //     }

// //     std::string line;
// //     while (true) {

// //         if (!getline(data_stream, line)) {
// //             std::cerr << "Error reading chunk size." << std::endl;
// //             break;
// //         }

// //         size_t pos = line.find("\r\n");
// //         if (pos != std::string::npos) {
// //             line.erase(pos);
// //         }

// //         int chunk_size = hex_to_int(line);
// //         if (chunk_size == 0) {
// //             break;
// //         }
// //         // std::string test ;
// //         // data_stream >> test;
// //         char* buffer = new char[chunk_size];
// //         data_stream.read(buffer, chunk_size);
// //         std::cout << buffer  << "  ___> "  << std::endl;



// //         delete [] buffer;

// //         if (!getline(data_stream, line)) {
// //             std::cerr << "Error reading CRLF after chunk." << std::endl;
// //             break;
// //         }
// //     }
// //     output_file.close(); 
// // }

// void handleClient(int client_fd, Client &client)
// {
//     char request[2000];
//     memset(request, 0,2000);
//     ssize_t bytes_received = 0;
//     std::ifstream fileStream;
//     Request req;
//     Response res;
//     std::string response;

//     client.set_response(res);
//     res.set_fileStream(fileStream);
//     res.set_response(response);
//     client.set_request(req);
//     while ((bytes_received = recv(client_fd, request, 2000 , 0)) > 0)
//     {
//         std::string tmp(request, bytes_received);
//         req.set_s_request(tmp);
//         check_request(client);
//         // save_chunked_data_to_file(tmp);
//         // std::cout << req.get_s_request();
//         memset(request, 0, 2000);
//         // break;
//     }
//     std::cout << "here" << std::endl;
//     if (send(client_fd, client.get_response().get_response().c_str(), client.get_response().get_response().length(), 0) == -1)
//     {
//         std::cerr << "Failed to send headers: " << strerror(errno) << std::endl;
//         return;
//     }

//     char send_buffer[8192];
//     size_t total_sent = 0;

//     while (fileStream.good() && !fileStream.eof())
//     {
//         fileStream.read(send_buffer, sizeof(send_buffer));
//         size_t bytes_read = fileStream.gcount();
//         if (bytes_read == 0)
//             break;

//         size_t bytes_sent = 0;
//         while (bytes_sent < bytes_read)
//         {
//             ssize_t result = send(client_fd, send_buffer + bytes_sent, bytes_read - bytes_sent, 0);

//             if (result <= 0)
//             {
//                 fileStream.close();
//                 return;
//             }

//             bytes_sent += result;
//             total_sent += result;
//         }
//     }

//     fileStream.close();
// }

// int main()
// {
//     int server_fd = -1;
//     struct sockaddr_in server_addr;
//     Client client;

//     if (!setupSocket(server_fd, server_addr))
//     {
//         return 1;
//     }

//     std::cout << "Server listening on port " << PORT << "...\n";

//     while (true)
//     {
//         struct sockaddr_in client_addr;
//         socklen_t client_addr_len = sizeof(client_addr);

//         int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
//         if (client_fd == -1)
//         {
//             std::cerr << "Accept failed: " << strerror(errno) << std::endl;
//             continue;
//         }

//         int flag = 1;
//         setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

//         handleClient(client_fd, client);
//         close(client_fd);
//     }
//     close(server_fd);
//     return 0;
// }


#include <signal.h>


int main(int ac, char **av)
{
    // signal(SIGPIPE, SIG_IGN);
    (void)ac;
    (void)av;
    try
    {
        std::cout << "Server is running on port 8080" << std::endl;
        Server SRV;
        SRV.startServer();
        
    }
    catch(const std::exception& e)
    {
        std::cout <<  "ya zamel ana f catsh"<< std::endl;
        std::cerr << e.what() << '\n';
    }
    
    return 0;
}