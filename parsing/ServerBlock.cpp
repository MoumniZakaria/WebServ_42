/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerBlock.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 00:36:22 by marvin            #+#    #+#             */
/*   Updated: 2025/03/27 00:36:22 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../webserver.hpp"


ServerBlock::ServerBlock()
{
    is_location_url = -1;
    client_body_size = 1048576;
    server_root = "./html";

}


ServerBlock::ServerBlock(const ServerBlock& other)
{
    *this = other; 
}

ServerBlock& ServerBlock::operator=(const ServerBlock& other)
{
    if (this != &other)
    {
        this->Host = other.Host;
        this->Port = other.Port;
        this->Server_names = other.Server_names;
        this->server_root = other.server_root;
        this->client_body_size = other.client_body_size;
        this->index = other.index;
        this->routes = other.routes;
        this->error_pages = other.error_pages;
    }
    return *this;
}


void ServerBlock::set_server_root(std::string server_root_arg)
{
    server_root = server_root_arg;
}

std::string& ServerBlock::get_server_root(void)
{
    return (server_root);
}

void ServerBlock::set_host(std::string set_host)
{
    Host = set_host;
}

std::string& ServerBlock::get_host(void)
{
    return Host;
}

void ServerBlock::set_port(int set_port)
{
    if (set_port > 0)
        Port.push_back(set_port);
}

std::vector <int> ServerBlock::get_port(void)
{
    return Port;
}

void ServerBlock::set_server_names(std::string set_server_names)
{
    Server_names = set_server_names;
    
}

std::string  ServerBlock::get_server_names(void)
{
    return Server_names;
}

void ServerBlock::set_index(std::vector <std::string>  set_index)
{
    size_t i = 0;
    while (i < set_index.size())
    {
        index.push_back(set_index[i]);
        i++;
    }
}
std::vector <std::string> ServerBlock::get_index(void)
{
    return index;
}

void ServerBlock::set_client_body_size(size_t set_client_body_size)
{
    client_body_size = set_client_body_size;
}

size_t ServerBlock::get_client_body_size(void)
{
    return client_body_size;
}

void ServerBlock::set_error_pages(std::map<int , std::string> set_error_pages)
{
    
    error_pages.insert(set_error_pages.begin(), set_error_pages.end());
}

std::map<int , std::string> ServerBlock::get_error_pages(void)
{
    return error_pages;
}

void ServerBlock::set_routes(std::vector<RouteBlock> set_routes)
{
    routes.insert(routes.end(), set_routes.begin(), set_routes.end());
}

std::vector<RouteBlock> ServerBlock::get_routes(void)
{
    return routes;
}


bool ServerBlock::is_valid_method(std::string path, std::string method)
{
    int i = 0;
    size_t pos;

    while (i < (int)routes.size())
    {
        pos = path.find(routes[i].get_root()+ routes[i].get_uri());
        if(pos != std::string::npos)
        {
            // std::cout << pos <<  "i find it " << std::endl;
            int j = 0;
            while(j < (int)routes[i].get_methods().size())
            {
                if(method == routes[i].get_methods()[j])
                    return true;
                j++;
            }
            return false;
        }
        i++;
    }
    return true;
}

std::string  ServerBlock::is_location_path(std::string path){
    int i = 0;

    path = path.substr(1);
    size_t pos = path.find("/");

    if (path.size() == 0)
        return "";
    path = "/" + path;
    if (pos == 0 || pos == std::string::npos){
        pos = path.size();
    }
    while (i < (int)routes.size())
    {
        if(!strncmp(path.c_str() , routes[i].get_uri().c_str() , pos)){
            is_location_url = i;
            return routes[i].get_root() + path;
        }
        i++;
    }
    return "";
}


std::string ServerBlock::find_error_page_path(int n){
    std::map<int , std::string>::iterator it = error_pages.begin();
    (void)n;
    while (it != error_pages.end())
    {
        if(it->first == n)
            return it->second;
        ++it;
    }
    return "NULL";
}

void ServerBlock::set_dafault_data()
{
    Host = "127.0.0.1";   
    Port.push_back(8080);
    Server_names = "test.com";
}

void ServerBlock::update_server_info(RouteBlock &route)
{
    server_root = route.get_root();
    index = route.get_index();
}