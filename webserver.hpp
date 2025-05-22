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
#include <algorithm>
#include <sys/stat.h>
#include <netdb.h>


#include <sys/types.h>

#include <stddef.h>
#include <poll.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctime>


class Request;
class Response;
class Client;
class Server;
class Confile;
class RouteBlock;
class ServerBlock;


#include "./parsing/ServerBlock.hpp"
#include "./response/response.hpp"
#include "./request/request.hpp"
#include "./client/client.hpp"
#include "./server/server.hpp"
#include "./parsing/Confile.hpp"
#include "./parsing/RouteBlock.hpp"






//the global error pages 

extern std::string status_200;
extern std::string status_400;
extern std::string status_403;
extern std::string status_404;
extern std::string status_405;






bool check_request_line(std::string & line);
std::string removeslashes(const std::string& line);
bool out_root_dir(std::string &pa , std::string &res , Client &clinet);
bool is_upper(std::string line);
char	**ft_split(char const *s, char c);
void get_error_res(std::string &res , int status , Client &clinet);
std::string fill_response(std::ifstream& fileStream,  std::string& filePath  , Client &client , int status );
void parse_request(Client &object);
// void _response(std::ifstream& fileStream , int status);
void check_request( Client & client );
void hanlde_post_request(Client &client);
void response_to_get(Client &client);




void  trim(std::string& str);
std::string ft_generate_file_names(Client &client, const std::string& extension);
void boundary(Client &clinet);
void chunked(Client &client);
void trim_non_printable(std::string &str);
void handle_boundary_chanked(Client & client);

void set_response_error(Client *client , int status);


//parsing

std::vector<std::string> get_words(std::string& line);
int check_line(std::vector<std::string>& lines, size_t& i);
std::string get_root(std::vector <std::string> words,bool& status);
bool fill_rest(ServerBlock& server, std::vector<std::string>& lines, size_t& i);
std::string trimstr(std::string str);
std::vector <std::string> get_lines(std::vector <std::string> parts);
void switch_parts(std::vector <std::string> parts);
std::vector <RouteBlock> pars_routes(std::vector <std::string>& lines, size_t& i, bool& status);
std::string get_root(std::vector <std::string> words,bool& status);
std::vector <std::string> pars_index(std::vector <std::string> words, bool& status);
bool check_status(ServerBlock& server);
bool check_path(std::string &path);
size_t check_type(std::string &path);
bool check_index(std::vector <std::string> index, std::string root);
int	check_empt(const char *line);
int check_dup_serv(std::vector <ServerBlock>& vector_serv, ServerBlock& current_server);



int set_default_page();




//cgi 

int cgi_handler(Client &clinet , std::string body);