#include "webserver.hpp"

// #define PORT 4444

// void trim_non_printable(std::string &str)
// {
//     size_t start = 0;
//     while (start < str.length() && !std::isprint(str[start]))
//         ++start;

//     size_t end = str.length();
//     while (end > start && !std::isprint(str[end - 1]))
//         --end;
//     str = str.substr(start, end - start);
// }

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

// /// /////////////////////////// chanked
// static int size = 0;
// static int writed = 0;
// std::ofstream file1;

// int hex_to_int(const std::string &hexStr)
// {
//     int result = 0;
//     for (size_t i = 0; i < hexStr.length(); ++i)
//     {
//         char c = hexStr[i];
//         if (c >= '0' && c <= '9')
//             result = result * 16 + (c - '0');
//         else if (c >= 'a' && c <= 'f')
//             result = result * 16 + (c - 'a' + 10);
//         else if (c >= 'A' && c <= 'F')
//             result = result * 16 + (c - 'A' + 10);
//     }
//     return result;
// }

// void chunked(Client &client)
// {
//     static int d;
//     if (!d)
//     {
//         file1.open("file.pm4");
//         d = 9;
//     }
//     std::cout << "ok" << std::endl;
//     std::string request = client.get_request().get_s_request();
//     size_t i = 0;
//     std::string chunk_size;

//     const int get_chunk_size = 0;
//     const int read_from_chunk = 1;
//     const int chunk_end = 2;
//     int state;
//     if (size == 0)
//         state = get_chunk_size;
//     else
//         state = read_from_chunk;

//     while (i < request.length())
//     {
//         if (state == get_chunk_size)
//         {
//             if (request[i] == '\r' && i + 1 < request.length() && request[i + 1] == '\n')
//             {
//                 size = hex_to_int(chunk_size);
//                 chunk_size = "";
//                 i += 2;

//                 if (size == 0)
//                     break;
//                 state = read_from_chunk;
//                 writed = 0;
//             }
//             else
//             {
//                 chunk_size += request[i];
//                 i++;
//             }
//         }
//         else if (state == read_from_chunk)
//         {
//             file1 << request[i] << std::flush;
//             i++;
//             writed++;

//             if (writed >= size)
//                 state = chunk_end;
//         }
//         else if (state == chunk_end)
//         {
//             if (request[i] == '\r' && i + 1 < request.length() && request[i + 1] == '\n')
//             {
//                 i += 2;
//                 state = get_chunk_size;
//             }
//             else
//             {
//                 std::cerr << "the chunk data must be end by \r\n"
//                           << std::endl;
//                 exit(0);
//             }
//         }
//     }
// }

// int check_if_have_new_boundary(std::string &buffer, std::string boundary, Client &client)
// {
//     boundary = "--" + boundary;
//     size_t pos = buffer.find(boundary);

//     if (pos == std::string::npos)
//         return -1;

//     size_t last_Boundary = pos + boundary.size();

//     if (last_Boundary + 2 < buffer.size() &&
//         buffer[last_Boundary] == '-' && buffer[last_Boundary + 1] == '-')
//     {
//         client.get_request().set_request_end(true);
//     }

//     return static_cast<int>(pos);
// }

// void fill_data_boudary(const std::string &tmp, Client &clinet)
// {
//     std::istringstream ss(tmp);
//     std::string line;
//     std::ofstream file;

//     std::getline(ss, line);

//     std::string key;
//     // std::cout << line << std::endl;
//     if (line.find("Content-Disposition:") != std::string::npos)
//     {
//         size_t name_pos = line.find("name=\"");
//         if (name_pos != std::string::npos)
//         {
//             size_t name_start = name_pos + 6;
//             size_t name_end = line.find("\"", name_start);
//             if (name_end != std::string::npos)
//             {
//                 key = line.substr(name_start, name_end - name_start);
//                 // std::cout << "Extracted key: " << key << std::endl;
//             }
//             else
//             {
//                 std::cerr << "Invalid boundary format 1" << std::endl;
//                 return;
//             }
//             size_t filename_pos = line.find("filename=\"");
//             if (filename_pos != std::string::npos)
//             {
//                 filename_pos += 10;
//                 size_t file_name_end = line.find("\"", filename_pos);
//                 if (file_name_end == std::string::npos)
//                 {
//                     std::cout << "error" << std::endl;
//                     exit(55);
//                 }
//                 std::string filename = line.substr(filename_pos, file_name_end - filename_pos);
//                 file.open(filename.c_str());
//                 std::getline(ss, line);
//                 std::getline(ss, line);
//                 while (1)
//                 {
//                     char c;
//                     line = "";
//                     while (ss.get(c))
//                         line += c;
//                     if (line.empty())
//                         break;
//                     file << line << std::flush;
//                 }
//                 // exit(0);
//             }
//             std::getline(ss, line);
//             while (1)
//             {
//                 char c;
//                 line = "";
//                 while (ss.get(c))
//                 {
//                     line += c;
//                 }
//                 if (line.empty())
//                     break;
//                 clinet.fill_map(key, line);
//             }
//         }
//         else
//         {
//             std::cerr << "Invalid boundary format 3" << std::endl;
//             return;
//         }
//     }
// }

// void boundary(Client &client)
// {
//     static std::string buffer;
//     static int i = 0;
//     static std::string boundary;
//     std::string tmp;

//     buffer += client.get_request().get_s_request();
//     if (i == 0)
//     {
//         size_t pos = buffer.find("\r\n\r\n");
//         if (pos == std::string::npos)
//         {
//             std::cerr << "Error: No headers found!" << std::endl;
//             exit(55);
//             ;
//         }

//         std::string headers = buffer.substr(0, pos);
//         buffer = buffer.substr(pos + 4);

//         size_t boundary_pos = headers.find("boundary=");
//         if (boundary_pos == std::string::npos)
//         {
//             std::cerr << "Error: Boundary not found in headers!" << std::endl;
//             return;
//         }

//         boundary_pos += 9;
//         size_t boundary_end = headers.find("\r\n", boundary_pos);
//         if (boundary_end == std::string::npos)
//         {
//             std::cerr << "Error: Invalid header format!" << std::endl;
//             return;
//         }

//         boundary = headers.substr(boundary_pos, boundary_end - boundary_pos);

//         size_t first_boundary_pos = buffer.find(boundary);
//         if (first_boundary_pos != std::string::npos)
//         {
//             buffer = buffer.substr(first_boundary_pos + boundary.size() + 2);
//         }

//         i = 1;
//     }

//     while (true)
//     {
//         int index = check_if_have_new_boundary(buffer, boundary, client);
//         if (index == -1)
//         {
//             // client.print_map();
//             // std::cout << "Request done or no boundary found." << std::endl;
//             return;
//         }
//         else if (index == 0)
//             buffer = buffer.substr(boundary.size() + 4);
//         else
//         {
//             tmp = buffer.substr(0, index - 2);
//             buffer = buffer.substr(index);
//             fill_data_boudary(tmp, client);
//         }
//     }
// }

// void handleClient(int client_fd, Client &client)
// {
//     char request[1000];
//     memset(request, 0, 1000);
//     ssize_t bytes_received = 0;
//     std::ifstream fileStream;
//     Request req;
//     Response res;
//     std::string response;

//     client.set_response(res);
//     res.set_fileStream(fileStream);
//     res.set_response(response);
//     client.set_request(req);
//     while ((bytes_received = recv(client_fd, request, 1000, 0)) > 0)
//     {
//         std::string tmp(request, bytes_received);
//         req.set_s_request(tmp);
//         // check_request(client);
//         boundary(client);
//         memset(request, 0, 1000);
//         // if (client.get_request().get_request_end()){
//         //     std::cout << "here";
//         //     break ;
//         // }
//     }
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


int main(int ac, char **av)
{
    (void)ac;
    (void)av;
    try
    {
        Server S1;
        S1.startServer();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}