/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Confile.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 00:46:52 by marvin            #+#    #+#             */
/*   Updated: 2025/03/27 00:46:52 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Confile.hpp"

Confile::Confile()
{
    // std::cout << "Confile Default constructor called" << std::endl;
    status = true;
    number_of_server = 0;
}
Confile::~Confile()
{
    // std::cout << "Confile Destructor called" << std::endl;
}

Confile::Confile(const Confile& other)
{
    *this = other;
    // std::cout << "Confile Copy Constructor called" << std::endl;

}

Confile& Confile::operator=(const Confile& other)
{
    // std::cout << "Confile Copy assignment operator called" << std::endl;
    if (this != &other)
    {
        this->servers.clear();
        this->servers = other.servers; //maybe this is a shallow copy and we need to apply a deep copy 
    }   
    this->status = other.status;
    this->number_of_server = other.number_of_server;
    return *this;
}

ServerBlock Confile::pars_server(std::vector<std::string>& lines, size_t& i)
{
    ServerBlock server;
    size_t hold = i;
    if (check_line(lines, i) == 1)
    {
        // std::cout << "checked serverline: " << lines[i] << "i: " << i << std::endl;
        status = false; //status is public memeber in confileclass
        return (std::cout << "!!!!! INVALID CONF FILE !!!!!!!" << std::endl, server);
    }
    // std::cout << "line after check" << std::endl;
       
    server.brace_count++;
    server.set_server_root(get_root(lines, i));
    std::string server_root = server.get_server_root();
    if (server_root.empty())
        return (status = false, server);
    // std::cout << "there is a root :" << server_root << "i: "<< i << std::endl;
    lines.erase(lines.begin() + i);
    // std::string server_root = trimstr(server.get_server_root());
    //maybe you need to check the path it's valid (acess) or not here 
    i = hold;
    // std::cout << "line to fill: " << lines[i] << std::endl;
    status = fill_rest(server, lines, i);
    // if (status == true)
       // status = check_status(server);
    // std::cout << "i after fill rest : "<< i << " lines : " << lines[i] << std::endl;

    // std::cout << "###########################################################" << std::endl;
    return server;
}

void Confile::set_server(std::vector<std::string> parts)
{
    size_t i = 0;
    while( i < parts.size())
    {
         std::vector<std::string> words = get_words(parts[i]);
        if (words.empty())
        {
            std::cout << "end of confile" << std::endl;
            break;
        }
        //  std::cout << "$$ server_pars in lines to pars_server: " << parts[i] << std::endl;
        ServerBlock tmp_server = pars_server(parts, i);
        // std::cout << "status: " << status << "and i : " << i << "line: " << parts[i] << std::endl;
        if (status == true)
        {
            number_of_server++;
            servers.push_back(tmp_server);
            i--;
        }
        else
        {
            std::cout << "Error Invalid config file" << std::endl;
            break;
        }
         i++;
    }
    if (status == true)
        std::cout << "confile created successfully" << std::endl;
    std::cout << "number of server is " << number_of_server << std::endl;

}

std::vector <ServerBlock>& Confile::get_server()
{
    return servers;
}


// std::cout << lines[0] << std::endl;
// std::vector<std::string> words = get_words(lines[0]);
// if (words.empty())
//     std::cerr << "Error No Words!" << std::endl;
// if (words.size() > 2)
//     std::cerr << "invalid server name unwanted word: " << words[2] << std::endl;
// if (words[0] != "server" && words[0] != "server{")
//     std::cerr << "invalid server name" << std::endl;
// if (words.size() > 1 && words[1] != "{")//in case server {
//     std::cerr << "invalid server name unwanted word: " << words[1] << std::endl;
// if (words[0] == "server")
// {
//     std::vector<std::string> words2 = get_words(lines[1]);
//     size_t i = 1;
//     std::cout << "##########################################" << std::endl;
//     while (lines.size() > i && words2.empty())
//     {
//         i++;
//         words2.clear();//maybe i need to clear the words2 before
//         words2 = get_words(lines[i]);
//     }
//     if (words2.empty())
//     {
//         std::cerr << "Error No Words anymore!" << std::endl;
//         return server;//return 1
//     }
//     std::cout << "size of words " << words2.size() << " word: " << words2[0] << std::endl;
//     if (!(words2.size() == 1) || !(words2[0] == "{"))
//         std::cerr << "++++invalid server block structure" << std::endl;
// }