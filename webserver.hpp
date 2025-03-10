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


bool check_request_line(std::string & line);
std::string removeslashes(const std::string& line);
bool out_root_dir(std::string &pa , std::string &res);
bool is_upper(std::string line);
char	**ft_split(char const *s, char c);
void get_error_res(std::string &res , int status);
std::string fill_response(std::ifstream& fileStream,  std::string& filePath);
void parse_request(Client &object);
void _response(std::ifstream& fileStream , int status);
void check_request( Client & client );
void hanlde_post_request(Client &client , int first , std::string req);
