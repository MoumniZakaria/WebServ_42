#pragma once

#include <iostream> 
#include <cstdlib>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <cstdlib>
#include "response/response.hpp"
#include "request/request.hpp"
#include "client/client.hpp"
#include "server/server.hpp"



extern std::string root;
extern std::string not_found;

bool check_request_line(std::string & line);
std::string removeslashes(const std::string& line);
bool out_root_dir(std::string &pa , std::string &res , Client &clinet);
bool is_upper(std::string line);
char	**ft_split(char const *s, char c);
void get_error_res(std::string &res , int status , Client &clinet);
std::string fill_response(std::ifstream& fileStream,  std::string& filePath  , Client &client );
void parse_request(Client &object);
// void _response(std::ifstream& fileStream , int status);
void check_request( Client & client );
void hanlde_post_request(Client &client);
void response_to_get(Client &client);




void  trim(std::string& str);
std::string generate_file_names(const std::string& extension) ;
void boundary(Client &clinet);
void chunked(Client &client);
void trim_non_printable(std::string &str);
void handle_boundary_chanked(Client & client);
