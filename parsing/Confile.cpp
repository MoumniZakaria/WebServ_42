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

#include "../webserver.hpp"


int set_default_page()
{
    std::vector <std::string> index;
    if (access("html", F_OK) != 0)
        return (std::cerr << "Error default directory not found" << std::endl, 1);
    std::ofstream indexfile("html/index.html");
    if (!indexfile)
    return (std::cerr << "Error cloudn't create index.html file" << std::endl, 1);
    if (indexfile.is_open())
    {
        indexfile << "<!DOCTYPE html>\n"
        << "<html lang=\"en\">\n"
        << "<head>\n"
        << "    <meta charset=\"UTF-8\">\n"
        << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        << "    <title>Document</title>\n"
        << "</head>\n"
        << "<body>\n"
        << "    <center><h1>Welcome to webserv 0.1</h1></center>\n"
        << "</body>\n"
        << "</html>\n";
        indexfile.close();
    }
    else
        return (std::cerr << "Error failed to open indexfile" << std::endl, 1);
    return 0;
}


Confile::Confile()
{
    status = true;
    number_of_server = 0; 
}


Confile::Confile(const Confile& other)
{
    *this = other;
}

Confile& Confile::operator=(const Confile& other)
{
    if (this != &other)
    {
        this->servers = other.servers;
    }   
    this->status = other.status;
    this->number_of_server = other.number_of_server;
    return *this;
}



int check_dup_serv(std::vector <ServerBlock>& vector_serv, ServerBlock& current_server)
{
	size_t i = 0;
	if (vector_serv.empty())
		return 0;
	while (i < vector_serv.size())
	{
		if (vector_serv[i].get_host() == current_server.get_host() && 
			vector_serv[i].get_port() == current_server.get_port() 
			)
			return 1;
		i++;		
	}
	return 0;
}



ServerBlock Confile::pars_server(std::vector<std::string>& lines, size_t& i)
{
    ServerBlock server;

    size_t hold = i;
    if (check_line(lines, i))
    {
        status = false;
        return (server);
    }
    i = hold;
    status = fill_rest(server, lines, i);
    if (status == false)
        return (server);
    status = check_status(server);
    return server;
}

void Confile::set_server(std::vector<std::string> parts)
{
    size_t i = 0;
    if(set_default_page())
    {
        status = false;
        return ;
    }
    while( i < parts.size())
    {
        std::vector<std::string> words = get_words(parts[i]);
        if (words.empty())
            break;
        ServerBlock tmp_server = pars_server(parts, i);
        if (status == true)
        {
            servers.push_back(tmp_server);
            number_of_server++;
        }
        else
        {
            std::cerr << "Error Invalid config file" << std::endl;
            return ;
        }
    } 
}

std::vector <ServerBlock>& Confile::get_server()
{
    return servers;
}
