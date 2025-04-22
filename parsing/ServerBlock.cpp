/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerBlock.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zmoumni <zmoumni@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 00:36:22 by marvin            #+#    #+#             */
/*   Updated: 2025/04/22 12:15:36 by zmoumni          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../webserver.hpp"

ServerBlock::ServerBlock()
{
    brace_count = 0;
    // std::cout << "ServerBlock Default constructor called" << std::endl;
}
ServerBlock::~ServerBlock()
{
    brace_count = 0;
    // std::cout << "ServerBlock Destructor called" << std::endl;
}

ServerBlock::ServerBlock(const ServerBlock& other)
{
    *this = other;
    // std::cout << "ServerBlock Copy Constructor called" << std::endl;
    
}

ServerBlock& ServerBlock::operator=(const ServerBlock& other)
{
    // std::cout << "ServerBlock Copy assignment operator called" << std::endl;
    if (this != &other)
    {
        this->Host = other.Host;
        this->Port = other.Port;
        this->Server_names.clear();//maybe it's a shallow
        this->Server_names = other.Server_names;
        this->server_root = other.server_root;
        this->client_body_size = other.client_body_size;
        this->index = other.index;
        this->routes.clear();
        this->routes = other.routes;
        this->error_pages = other.error_pages;
        this->brace_count = other.brace_count;
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
    // std::cout << "set host called for : " << set_host << std::endl;
    Host = set_host;
}

std::string& ServerBlock::get_host(void)
{
    return Host;
}

void ServerBlock::set_port(int set_port)
{
    Port = set_port;
}

int ServerBlock::get_port(void)
{
    return Port;
}

void ServerBlock::set_server_names(std::vector<std::string> set_server_names)
{
    Server_names = set_server_names;
}

std::vector <std::string>  ServerBlock::get_server_names(void)
{
    return Server_names;
}

void ServerBlock::set_index(std::vector <std::string>  set_index)
{
    index = set_index;
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
    error_pages = set_error_pages;
}

std::map<int , std::string> ServerBlock::get_error_pages(void)
{
    return error_pages;
}

void ServerBlock::set_routes(std::vector<RouteBlock> set_routes)
{
    routes = set_routes;
}

std::vector<RouteBlock> ServerBlock::get_routes(void)
{
    return routes;
}