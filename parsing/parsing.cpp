/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 06:27:47 by marvin            #+#    #+#             */
/*   Updated: 2025/03/27 06:27:47 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../webserver.hpp"


std::vector <std::string> get_lines(std::vector <std::string> parts) 
{
	size_t i = 0;
	std::string line;
	std::vector <std::string> lines;
	while (parts.size() > i)
	{
		std::stringstream ss(parts[i]);
		while (std::getline(ss, line))
			lines.push_back(line);
		i++;
	}
    // for (size_t i = 0; i < lines.size(); ++i) {
    //	  std::cout << "line " << i + 1 << ": " << lines[i] << std::endl;
	// }
	return lines;
}

std::vector<std::string> get_words(std::string& line)
{
    std::string tmp;
	std::stringstream ss(line);
	std::vector<std::string> words;
	while(ss >> tmp)
	{
		words.push_back(tmp);
	}
    return words;
}

std::vector<std::string> split_brace(std::string line)
{
    std::vector<std::string> words;
    size_t found = line.find("{");
    if (found != std::string::npos)
    {
        words.push_back(line.substr(0, found));
        words.push_back(line.substr(found + 1)); // keeps the first brace and the rest
    }
    else
    {
        words.push_back(line); // no brace found, return the whole line
    }
    return words;
}


std::vector<std::string> split_words(std::string line)
{
	std::string tmp = "";
	std::vector<std::string> words;
    size_t pos = 0;
    size_t found = line.find(".", pos);
    while (found != std::string::npos)
	{
        // std::cout << found << " and " << pos << std::endl;
	    tmp = line.substr(pos, found - pos);
        // std::cout << tmp << std::endl;
        words.push_back(tmp);
		pos = found + 1;
		found = line.find(".", pos);
	}
    if (pos != 0)// condition to avoid case of spilting string like this "" it will fill vector with nothing 
    {
        tmp = line.substr(pos);
        words.push_back(tmp);
    }
	// for (size_t i = 0; i < words.size(); i++)
	// 	std::cout << "word " << i + 1 << ": " << words[i] << std::endl;
    return words;
}

std::vector<std::string> get_next_line_words(std::vector<std::string> lines, size_t &i)
{
    // size_t i = 1;
    std::vector<std::string> words = get_words(lines[i]);
    // std::cout << "##########################################" << std::endl;
    while (lines.size() > i && words.empty())
    {
        i++;
        words.clear();//maybe i need to clear the words before
        words = get_words(lines[i]);
    }
    return words;
    // if (words.empty())
    // {
    //     std::cerr << "Error No Words anymore!" << std::endl;
    //     return 1;//return 1
    // }
}


// int check_line(std::vector<std::string>& lines, size_t& i)// should pass index i if we check an empty 
// {
//     std::vector<std::string> words = get_words(lines[i]);
//     //cases of anything else than server "server, server{, server {
//     if (words.size() == 1)//should be just 2 words
// 		return (std::cerr << "invalid server structre " << std::endl, 1);
//     if (words[0] != "server" && words[0] != "server{") //first word should be one of this
//     {
// 		size_t pos = lines[0].find('{');
// 		if (pos != std::string::npos)
// 		{
// 			std::string updated_line = lines[i].substr(0, pos);
// 			updated_line = trimstr(updated_line);
// 			std::cout << "upadated line: |" << updated_line << std::endl;
// 			if (updated_line != "server")
// 			{
// 				std::cerr << "invalid server name" << std::endl;
// 				return 1;
// 			}
// 			lines[i] = lines[i].substr(pos + 1);
// 			std::cout << "line upadated: " << lines[i] << std::endl;
// 			return (0);
// 		}
//     }
//     if (words.size() > 1 && words[0] == "server" && words[1] != "{")//in case server {
//     {
// 		std::cerr << "invalid server name unwanted word: " << words[1] << std::endl;
//         return 1;
//     }
//     if ( words.size() == 3 && words[0] != "server{")
//     {
// 		return (std::cerr << "invalid server structre " << std::endl, 1);
//     }
// 	size_t s = 1;
// 	if (words[0] == "server")
// 		s = 2;
// 	std::string rest;
// 	while (s < words.size())
// 	{
// 		rest += " ";
// 		rest += words[s];
// 		s++;
// 	}
// 	// std::cout << "rest" << rest << std::endl;
// 	lines[i] = rest; 
//     return 0;
// }

int check_line(std::vector<std::string>& lines, size_t& i)
{
	std::vector <std::string> words = split_brace(lines[i]);
	if (words.size() < 2)
		return (std::cout << "Error invalid server struct" << std::endl, 1);
	std::string first_word = trimstr(words[0]);
	if (first_word != "server")
		return (std::cout << "Error unwanted word : " << words[0] << std::endl, 1);
	size_t x = 1;
	std::string rest;
	while (x < words.size())
	{
		rest += " ";
		rest += words[x];
		x++;
	}
	lines[i] = rest;
	return 0;
}

std::string get_root(std::vector <std::string> words,bool& status)
{
	std::string root;
	if (words.size() == 1)
		return (status = false, std::cout << "Error root path not found" << std::endl, root);
	if (words.size() > 2)
		return (status = false, std::cout << "Error too many root's path" << std::endl, root);
	if (check_path(words[1]) == false)
		return (status = false, std::cout << "Error root path not exist" << std::endl, root);
	if (check_type(words[1]) != 0)
		return (status = false, std::cout << "Error root path not directory" << std::endl, root);
	root = words[1];
	return (root);
}

std::string trimstr(std::string str)
{
	std::string trimed;
	size_t i = 0;
	while (i < str.size() && str[i] == ' ')
		i++;
	size_t len = str.size() - 1;
	while (len >= i && str[len] == ' ')
		len--;
	trimed = str.substr(i , len - i + 1);
	return trimed;
}

std::string int_to_string(int value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

std::string convert_hostname(std::string host_name, bool& status)
{
	struct addrinfo hints, *res;
	std::string ip_address;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_STREAM;

    int status_code = getaddrinfo(host_name.c_str(), NULL, &hints, &res);
    if (status_code != 0)
    {
        std::cerr << "Failed to get hostname: " << host_name << "\n";
		status = false;
        return ip_address;
    }
	struct sockaddr_in *ipv4 =  (struct sockaddr_in *)res->ai_addr;
	unsigned char* bytes = (unsigned char*)&(ipv4->sin_addr);
	ip_address = int_to_string(bytes[0]) + "." + 
				 int_to_string(bytes[1]) + "." +
				 int_to_string(bytes[2]) + "." +
				 int_to_string(bytes[3]);
    freeaddrinfo(res);
    return (ip_address);
}


std::string pars_host(std::vector<std::string> words, bool& status)
{

	std::string host;
	if (words.size() > 2 || words.size() == 1)
		return (status = false, std::cerr << "Error Invalid structre"<<  std::endl, host);
	std::string address = words[1];
	size_t i = 0;
	while(address[i])
	{
		if (address.length() > i + 1)
		{
			if (address[i] == '.' && address[i  + 1] == '.')
				return (convert_hostname(address, status));
		}
		i++;
	}
	std::vector <std::string> octets = split_words(address);
	size_t x = 0;
	int numb;
	if (octets.size() != 4)
		return (convert_hostname(address, status));
	while (octets.size() > x)
	{
		size_t j = 0;
		while(octets[x][j])
		{
			if (!isdigit(octets[x][j]))
				return (convert_hostname(address, status));
			if (octets[x][0] == '0' && octets[x].length() > 1)
				return (convert_hostname(address, status));
			j++;
		}
		std::stringstream ss(octets[x]);
		ss >> numb;
		if (numb > 255 || numb < 0)
			return (convert_hostname(address, status));
		x++;
	}
	host = address;
	return host;
}

int pars_port(std::vector<std::string> words, bool& status)
{
    if (words.size() != 2)
    {
        std::cerr << "Error invalid listen direcitve (too many ports)" << std::endl;
        status = false;
        return (-1);
    }
    std::string port = words[1];
    int i = 0;
    while(port[i])
    {
        if (!isdigit(port[i]))
            return (status = false, std::cout << "Error port include non-digit" << std::endl, -1);
        i++;
    }
    std::stringstream ss(port);
    int portnumbr;
    ss >> portnumbr;
    if (portnumbr < 1 || portnumbr > 65535)
        return (status = false, std::cout << "Error Invalid port's range" << std::endl, -1);
    return portnumbr;
}

int check_words(std::string str)
{
	size_t x = 0;
	while(x < str.size())
	{
		if (!isdigit(str[x]) && str[x] != '.')
			return 1;
	}
	return 0;

}


std::string pars_server_names(std::vector<std::string> words, bool& status)
{
	std::string names;
	
	if (words.size() == 1)
	return (status = false, std::cout << "Error No serverNames" << std::endl, names);
	if (words.size() > 2)
	return (status = false, std::cout << "Error too many serverNames" << std::endl, names);
		if (words[1].find('{') != std::string::npos || words[1].find('}') != std::string::npos)
			return (status = false, std::cout << "Error unwanted braces" << std::endl, names);
	// names = pars_host(words, status);
	// exit(1);
	// return (names);
	names = words[1];
	// names.erase(names.begin());
	
	// std::cout << "#==> server name get parsed" << std::endl;
	return names;
}

std::vector <std::string> pars_index(std::vector <std::string> words, bool& status)
{
	std::vector<std::string> vec_index;
	if (words.size() == 1)
		return (status = false, std::cout << "Error no Index" << std::endl, vec_index);
	for (size_t i = 0; i < words.size(); i++)
	{
		if (words[i].find('{') != std::string::npos || words[i].find('}') != std::string::npos)
		return (status = false, std::cout << "Error unwanted braces" << std::endl, vec_index);
	}
	vec_index = words;
	vec_index.erase(vec_index.begin());
	return vec_index;
}

size_t pars_cbs(std::vector <std::string> words, bool& status)
{
	size_t bytes = 0;
	if (words.size() == 1)
		return (status = false, std::cout << "Error no Size" << std::endl, bytes);
	if (words.size() > 2)
		return (status = false, std::cout << "Error too many Sizes" << std::endl, bytes);
	size_t i = 0;
	size_t len = words[1].length() - 1;
	while (i < len)
	{
		if (!isdigit(words[1][i]))
		{
			if (words[1].find('{') != std::string::npos || words[1].find('}') != std::string::npos)
				return (status = false, std::cout << "Error unwanted braces" << std::endl, bytes);
			return (status = false, std::cout << "Error invalid size" << std::endl, bytes);
		}
		i++;
	}
	if (isdigit(words[1][i]))
	{
		std::stringstream ss(words[1]);
		ss >> bytes;
		return bytes;
	}		
	else if (words[1][i] == 'G' || words[1][i] == 'g')
	{
		words[1][i] = '\0';
		std::stringstream ss(words[1]);
		ss >> bytes;
		bytes *= 1024 * 1024 * 1024;
	}
	else if (words[1][i] == 'M' || words[1][i] == 'm')
	{
		words[1][i] = '\0';
		std::stringstream ss(words[1]);
		ss >> bytes;
		bytes *= 1024 * 1024;
	}
	else if (words[1][i] == 'K' || words[1][i] == 'k')
	{
		words[1][i] = '\0';
		std::stringstream ss(words[1]);
		ss >> bytes;
		bytes *= 1024;
	}
	else
		return (status = false, std::cout << "Error invalid unit" << std::endl, bytes);

	// std::cout << "#==> client body size get parsed" << std::endl;
return bytes;	
}

std::map <int, std::string> pars_error_pages(std::vector <std::string> words, bool& status)
{
    size_t i = 1;
    size_t j = 0;
	int code;
	std::map<int, std::string> pages;
	if (words.size() < 3)
		return (status = false, std::cout << "Error invalid error_pages directive" << std::endl, pages);
	size_t  last = words.size() - 1 ;
	if (words[last].find('{') != std::string::npos || words[last].find('}') != std::string::npos)
		return (status = false, std::cout << "Error unwanted braces" << std::endl, pages);
	if (check_path(words[last]) == false)
		return(status = false, std::cout << "Error error_page path not exist" << std::endl, pages);
	if (check_type(words[last]) != 1)
		return(status = false, std::cout << "Error error_page path not file" << std::endl, pages);
	if (access(words[last].c_str(), R_OK) != 0)
		return(status = false, std::cout << "Error error_page path file : read permission denied" << std::endl, pages);

	while (i < last)
	{
        j = 0;
        while(j < words[i].size())
        {
            if (!isdigit(words[i][j]))
				return (status = false, std:: cout << "Error invalid code" << std::endl , pages);
            j++;
        }
		std::stringstream ss(words[i]);
		ss >> code;
		if (code != 200 && code != 201 && code != 400 && code != 405 && code != 403 && code != 401 && code != 404 && code != 204 && code != 415)
			return (status = false, std:: cout << "Error invalid code" << std::endl , pages);
		pages[code] = words[last];
		i++;
	}
	return pages;
}

bool check_status(ServerBlock& server)
{
    std::string tmp = server.get_host();
    if (tmp.empty())
        return (std::cerr << "Error host directive not found" << std::endl, false);

    std::vector <int> port = server.get_port();
    if (port.empty())
        return (std::cerr << "Error listen directive not found" << std::endl, false);
    return true;
}

bool fill_rest(ServerBlock& server, std::vector<std::string> &lines, size_t &i)
{
	bool status = true;

	std::vector<std::string> words = get_words(lines[i]);
	while (lines.size() > i)
	{
		if (status == false)
			break;
		if (words[0] == "}" || words[0][0] == '}')
		{
			if (words[0] == "}" && words.size() == 1) //case of  we have just } "this for handling }; in first server with conf of more than 1 server"
			{
				if (i < lines.size() - 1)
				{
					std::cout << "+++case of first server" << std::endl;
					status = false;
				}
				lines[i] = " ";
				return (std::cout << "end of serverBlock " << std::endl, status);
			}
			if (words[0].length() > 1) // case of have }server
				words[0] = words[0].substr(1);
			else if (words[0] == "}" && words.size() > 1) //case of having } server
				words[0] = " ";
			std::string rest;
			size_t s = 0;
			while (s < words.size())
			{
				rest += words[s];
				rest += " ";
				s++;
			}
			lines[i] = rest;
			return (std::cout << "end of ServerBlock" << std::endl, status);				
		}
		else if (words[0] == "host")
			server.set_host(pars_host(words, status));// handle case if host is a domain and check if it include '{' or '}'
		else if (words[0] == "listen")
			server.set_port(pars_port(words, status));//
		else if (words[0] == "server_name")
			server.set_server_names(pars_server_names(words, status));
		else if (words[0] == "index")
			server.set_index(pars_index(words, status));
		else if (words[0] == "client_max_body_size")
			server.set_client_body_size(pars_cbs(words, status));
		else if (words[0] == "error_page")
			server.set_error_pages(pars_error_pages(words, status));
		else if (words[0] == "root")
			server.set_server_root(get_root(words, status));
		else if (words[0] == "location")
		{
			server.set_routes(pars_routes(lines, i, status));
 			i--;
			std::vector<RouteBlock> routes = server.get_routes();
		}
		else
		{
			std::cout << "words: " << words[0] << " invalid element in serverblock" << std::endl;
			return false;
		}
		i++;
		words.clear();// check if that necessaire
		words = get_words(lines[i]);
	}
	return status;
}
