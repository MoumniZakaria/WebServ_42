/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RouteBlock.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 02:34:57 by marvin            #+#    #+#             */
/*   Updated: 2025/03/27 02:34:57 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../webserver.hpp"

RouteBlock::RouteBlock()
{
    root = "./html";
    methods.push_back("GET");
    methods.push_back("POST");
    methods.push_back("DELETE");
    autoindex = false;
    client_body_temp_path = "./upload";
}


RouteBlock::RouteBlock(const RouteBlock& other)
{
    *this = other;
}

RouteBlock& RouteBlock::operator=(const RouteBlock& other)
{
    if (this != &other)
    {
        this->URI = other.URI;
        this->root= other.root;
        this->methods = other.methods;
        this->autoindex = other.autoindex;
        this->index = other.index;
        this->cgi_extension = other.cgi_extension;
        this->client_body_temp_path = other.client_body_temp_path;
        this->redirections = other.redirections;
    }
    return *this;
}

void RouteBlock::set_URI(std::string set_uri)
{
    URI = set_uri;
}
std::string RouteBlock::get_uri(void)
{
    return URI;
}

void RouteBlock::set_root(std::string set_root)
{
    root = set_root;
}

std::string RouteBlock::get_root(void)
{
    return root;
}

void RouteBlock::set_methods(std::vector <std::string> set_methods)
{
    methods = set_methods;
}

std::vector <std::string> RouteBlock::get_methods(void)
{
    return methods;
}


void RouteBlock::set_autoindex(bool set_autoindex)
{
    autoindex = set_autoindex;
}

bool RouteBlock::get_autoindex(void)
{
    return autoindex;
}


void RouteBlock::set_index(std::vector <std::string>  set_index)
{
    index = set_index;
}
std::vector <std::string>  RouteBlock::get_index(void)
{
    return index;
}


void RouteBlock::set_cgi_ext( std::vector<std::string> set_cgi)
{
    cgi_extension = set_cgi;
}

std::vector<std::string> RouteBlock::get_cgi_ext(void)
{
    return cgi_extension;
}

void RouteBlock::set_client_body_temp_path(std::string tmp_path)
{
   client_body_temp_path = tmp_path;
}

std::string RouteBlock::get_client_body_temp_path(void)
{
    return client_body_temp_path;
}

void RouteBlock::set_redirections(std::map<int,std::string> set_redirection)
{
    redirections.insert(set_redirection.begin(), set_redirection.end());
}
std::map <int, std::string> RouteBlock::get_redirections(void)
{
    return redirections;
}