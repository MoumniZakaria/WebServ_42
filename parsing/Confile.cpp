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
        status = false; //status is public memeber in confileclass
        return (server);
    }
    i = hold;
    status = fill_rest(server, lines, i);
    if (status == false)
        return (server);
    status = check_status(server);
    // status = check_index(server.get_index(), server.get_server_root());
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
        {
            std::cerr << "end of confile"<< std::endl;
            break;
        }
        ServerBlock tmp_server = pars_server(parts, i);
        if (status == true)
        {
            // if (check_dup_serv(servers, tmp_server) || tmp_server.dupindex > 0)
            // {
            //     status = false;
            //     std::cerr << "Error duplicated infos invalid config file" << std::endl;//check resources
            //     return ;
            // }
            servers.push_back(tmp_server);
            number_of_server++;
        }
        else
        {
            std::cerr << "Error Invalid config file" << std::endl;//check resources
            return ;
        }
    }
    if (status == true)
        std::cout << "confile created successfully" << std::endl;
    std::cout << "number of server is " << number_of_server << std::endl;

}

std::vector <ServerBlock>& Confile::get_server()
{
    return servers;
}

// void Confile::set_default_server(std::vector <ServerBlock> default_servers)
// {
//     servers = default_servers;
// }