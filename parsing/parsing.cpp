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

#include "Confile.hpp"


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
	// for (size_t i = 0; i < words.size(); i++)
	// 	std::cout << "word " << i + 1 << ": " << words[i] << std::endl;
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

int check_line(std::vector<std::string>& lines, size_t& i)// should pass index i if we check an empty 
{
    std::vector<std::string> words = get_words(lines[i]);
    if (words.empty())
    {
        // std::cerr << "Error No Words!" << std::endl;
        words.clear();
        words = get_next_line_words(lines, i);
        if (words.empty())
        {
            std::cerr << "Error No Words anymore!" << std::endl;
            return 1;//return 1
        }
		std::cout << "servived : " << words[0] <<std::endl;
        // return 2;
    }
    //cases of anything else than server "server, server{, server {
    if (words.size() < 3)//should be just 2 words
		return (std::cerr << "invalid server structre " << std::endl, 1);
    if (words[0] != "server" && words[0] != "server{") //first word should be one of this
    {
		std::cerr << "invalid server name" << std::endl;
        return 1;
    }
    if (words.size() > 1 && words[0] == "server" && words[1] != "{")//in case server {
    {
		std::cerr << "invalid server name unwanted word: " << words[1] << std::endl;
        return 1;
    }
    if ( words.size() == 3 && words[0] != "server{")
    {
		return (std::cerr << "invalid server structre " << std::endl, 1);
    }
	size_t s = 1;
	if (words[0] == "server")
		s = 2;
	std::string rest;
	while (s < words.size())
	{
		rest += " ";
		rest += words[s];
		s++;
	}
	// std::cout << "rest" << rest << std::endl;
	lines[i] = rest; 
	// exit(0);
	// 	fill_exp(server, words, 1);
	// else if (words.size() > 3)
	// 	fill_exp(server, words, 2)
    return 0;
}

std::string get_root(std::vector <std::string> lines,size_t& i)
{
	// i++;//to move to next  line after {
	std::string root = "";
	// bool check_path = true;
	std::vector<std::string> words = get_next_line_words(lines, i);
	while (lines.size() > i)
	{

		if (words.empty())
		{
			std::cerr << "Error No Words anymore!" << std::endl;
			return root;//return 1
		}
		//this condition bellow need to get update after handling the case of root bellow locationblock
		if (words[0] == "{" || words[0] == "}")//to avoid consider root of locationBlock or onther serverBlock is the root of ServerBlock
			return (std::cout << "Error root path not found" << std::endl, root);
		if (words[0] == "root")
		{
			if (words.size() == 1)
				return (std::cout << "Error root path not found" << std::endl, root);
			if (words.size() > 2)
				return (std::cout << "too many root's path" << std::endl, root);
			//maybe you need to store words[1] in string before clear it
			if (check_path(words[1]) == false)
				return (std::cout << "root path not exist" << std::endl, root);
			if (check_type(words[1]) != 0)
				return (std::cout << "root path not directory" << std::endl, root);
			root = words[1];
			words.clear();
			// std::cout << "#==> root get parsed" << std::endl;
			return (root);
		}
		i++;
		words.clear();
		words = get_next_line_words(lines, i);
	}
	return root;
}

std::string trimstr(std::string str)
{
	std::string trimed;
	size_t len = str.length() - 1;
	if (str[len] == ';')
	{
		trimed = str.substr(0, len);
		std::cout << "trimed" << std::endl;
	}
	return trimed;
}

std::string pars_host(std::vector<std::string> words, bool& status)
{

	std::string host = "";
	if (words.size() > 2 || words.size() == 1)
		return (status = false, std::cout << "Error Invalid Host" << std::endl, host);
	// std::cout << "$$$$: " << words[1] << std::endl; 
	// std::string address = trimstr(words[1]); // to trim ";"
	std::string address = words[1];
	// if (address.empty())
	// 	return (std::cout << "there is no ';' at the end of line" << std::endl, host);
	size_t i = 0;
	while(address[i])
	{
		if (address.length() > i + 1)
		{
			if (address[i] == '.' && address[i  + 1] == '.')
				return (status = false, std::cout << "Error Invalid Host" << std::endl, host);
		}
		i++;
	}
	std::vector <std::string> octets = split_words(address);
	size_t x = 0;
	int numb;
	if (octets.size() != 4)
		return (status = false, std::cout << "Error invalid host" << std::endl, host);
	while (octets.size() > x)
	{
		size_t j = 0;
		while(octets[x][j])
		{
			if (!isdigit(octets[x][j]))
				return (status = false, std::cout << "Error not digit" << std::endl, host);
			if (octets[x][0] == '0' && octets[x].length() > 1)
				return (status = false, std::cout << "Error invalid octet" << std::endl, host);
			j++;
		}
		std::stringstream ss(octets[x]);
		ss >> numb;
		// std::cout << "numb is : " << numb << std::endl;
		if (numb > 255 || numb < 0)
			return (status = false, std::cout << "Error invalid octet" << std::endl, host);
		x++;
	}
	host = address;
	// std::cout << "#==> host get parsed" << std::endl;
	return host;
}

int pars_port(std::vector<std::string> words, bool& status)
{
	// std::cout << "@@@" << std::endl;
	if (words.size() == 1 || words.size() > 2)
		return -1;
	std::string port = words[1];
	int i = 0;
	while(port[i])
	{
		if (!isdigit(port[i]))
			return (status = false, std::cout << "Error port include non-digit" << std::endl, -1);
		i++;
	}
	// std::cout << "pars port: " << port << std::endl;
	std::stringstream ss(port);
	int portnumbr;
	ss >> portnumbr;
	if (portnumbr < 1024 || portnumbr > 65535)
		return (status = false, std::cout << "Error Invalid port's range" << std::endl, -1);
	// std::cout << "#==> port get parsed" << std::endl;
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


std::vector<std::string> pars_server_names(std::vector<std::string> words, bool& status)
{
	std::vector<std::string> names;
	
	if (words.size() == 1)
	return (status = false, std::cout << "Error No serverNames" << std::endl, names);
	for (size_t i = 0; i < words.size(); i++)
	{
		if (words[i].find('{') != std::string::npos || words[i].find('}') != std::string::npos)
		return (status = false, std::cout << "Error unwanted braces" << std::endl, names);
	}
	names = words;
	
	names.erase(names.begin());
	// std::cout << "#==> server name get parsed" << std::endl;
	return names;
}

std::vector <std::string> pars_index(std::vector <std::string> words,std::string root, bool& status)
{
	std::vector<std::string> vec_index;
	if (words.size() == 1)
		return (status = false, std::cout << "Error no Index" << std::endl, vec_index);
	for (size_t i = 0; i < words.size(); i++)
	{
		if (words[i].find('{') != std::string::npos || words[i].find('}') != std::string::npos)
		return (status = false, std::cout << "Error unwanted braces" << std::endl, vec_index);
	}
	if (check_index(words, root))
		return (vec_index);
	vec_index = words;
	// vec_index.erase(vec_index.begin());

	// std::cout << "#==> index get parsed" << std::endl;
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
			std::cout << "Error invalid size" << std::endl;
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
	int i = 1;
	int j = 0;
	int code;
	std::map<int, std::string> pages;
	if (words.size() < 3)
		return (status = false, std::cout << "Error invalid error_pages directive" << std::endl, pages);
	int  last = words.size() - 1 ;
	if (check_path(words[last]) == false)
		return(std::cout << "Error error_page path not exist" << std::endl, pages);
	if (check_type(words[last]) != 1)
		return(std::cout << "Error error_page path not file" << std::endl, pages);
	if (access(words[last].c_str(), R_OK) != 0)
		return(std::cout << "Error error_page path file : read permission denied" << std::endl, pages);
	if (words[last].find('{') != std::string::npos || words[last].find('}') != std::string::npos)
		return (status = false, std::cout << "Error unwanted braces" << std::endl, pages);

	while (i < last)
	{
		if (!isdigit(words[i][j]))
			return (status = false, std:: cout << "Error invalid code" << std::endl , pages);
		std::stringstream ss(words[i]);
		ss >> code;
		if (code != 200 && code != 400 && code != 405 && code != 403 && code != 401 && code != 404)
			return (status = false, std:: cout << "Error invalid code" << std::endl , pages);
		pages[code] = words[last];
		i++;
	}
	// std::cout << "#==> error_pages get parsed" << std::endl;
	return pages;
}

bool check_status(ServerBlock& server)
{
	std::string tmp = server.get_host();
	if (tmp.empty())
		return false;
	std::cout << "host is : " << tmp << std::endl;

	int port = server.get_port();
	if (port < 0)
		return false;
	std::cout << "port: " << port << std::endl;

	std::vector<std::string> names  = server.get_server_names();
	if (names.empty())
	return false;
	for (size_t x = 0; x < names.size(); ++x) {
		std::cout << "name " << x + 1 << ": " << names[x] << std::endl;
	}
	
	std::vector<std::string> index  = server.get_index();
	if (index.empty())
		return false;
	for (size_t x = 0; x < index.size(); ++x) {
            std::cout << "index " << x + 1 << ": " << index[x] << std::endl;
    }

	size_t s = server.get_client_body_size();
	std::cout << "size: " << s << std::endl;
	std::map<int , std::string> pages = server.get_error_pages();

	for (std::map<int, std::string>::iterator it = pages.begin(); it != pages.end(); ++it) {
        std::cout << it->first << " => " << it->second << std::endl;
    }

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
		// std::cout << "line in to check in fill_rest loop : " << lines[i] << " i: " << i << std::endl;
		// if (i == 5)
		// 	std::cout << "line after 5: " << lines[i + 1] << " i: " << i << std::endl;
		if (words[0] == "}" || words[0][0] == '}')
		{
			// if (i + 1 ==  lines.size()) //case of lastline
			if (words[0] == "}" && words.size() == 1) //case of  we have just }
			{
				lines[i] = " ";
				// lines.erase(lines.begin() + i);
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
			// std::cout << "rest" << rest << std::endl;
			lines[i] = rest;
			return (std::cout << "end of ServerBlock" << std::endl, status);				
		}
		//you need to read abou the order of config file element to see in the condition below is valid or not 
		// if (lines[i].find('{') != std::string::npos)//to avoid consider root of locationBlock or onther serverBlock is the root of ServerBlock
		// 	return (std::cout << "Error unwanted braces" << std::endl, false);
		else if (words[0] == "host")
			server.set_host(pars_host(words, status));// handle case if host is a domain and check if it include '{' or '}'
		else if (words[0] == "listen")
			server.set_port(pars_port(words, status));//
		else if (words[0] == "server_name")
			server.set_server_names(pars_server_names(words, status));
		else if (words[0] == "index")
			server.set_index(pars_index(words, server.get_server_root(), status));
		else if (words[0] == "client_max_body_size")
			server.set_client_body_size(pars_cbs(words, status));
		else if (words[0] == "error_page")
			server.set_error_pages(pars_error_pages(words, status));
		else if (words[0] == "location")
		{
			server.set_routes(pars_routes(lines, i, status));
 			i--;
			// 	for (size_t z = 0; z < words.size(); ++z) {
			// std::cout << "words_loc " << z + 1 << ": " << words[z] << std::endl;}
			// exit(0);
		}
		else
		{
			std::cout << "words: " << words[0] << " invalid element in serverblock" << std::endl;
			return false;
		}
		// if (i == 7)
		// 	break;
		i++;
		words.clear();// check if that necessaire
		words = get_words(lines[i]);
	}
	return status;
}

void  switch_parts(std::vector <std::string> parts)
{
	size_t i = 0;
	std::vector <std::string> lines;

	while (parts.size() > i)
	{
			// std::cout << "before replace: " << parts[i] << std::endl;
			std::replace(parts[i].begin() , parts[i].end() , '\n' , ' ');
			// std::replace(parts[i].begin(), parts[i].end(), '\r', ' ');
			// std::cout << "fter replace: " << parts[i] << std::endl;
			i++;
	}
			// exit(1);

}

//200 400 405 403 401  404 