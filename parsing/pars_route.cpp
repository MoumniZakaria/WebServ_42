/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pars_route.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kid-ouis <kid-ouis@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/05 12:01:28 by kid-ouis          #+#    #+#             */
/*   Updated: 2025/05/29 17:47:58 by kid-ouis         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../webserver.hpp"


int check_first_line(RouteBlock& route, std::vector <std::string>& lines, size_t i, bool& status)
{
	int last_char;
	size_t s = 3;
	std::string URI;
	std::vector <std::string> words = get_words(lines[i]);
	if (words[0] != "location" && words[0] != "location{")
		return (1);//end of locationblock
	if (words.size() < 4)
			return (status = false, std::cout << "invalid location structre" << std::endl, 1);
	URI = words[1];
	last_char = words[1].length() - 1;
	if (words[1][last_char] == '{')
	{
		URI = words[1].substr(0, last_char);
		s = 2;
	}
	else if (words[2] != "{")
		return (status = false, std::cout << "invalid location structre" << std::endl, 1);
	if (URI.find('{') != std::string::npos || URI.find('}') != std::string::npos)
		return (status = false, std::cout << "Error unwanted braces" << std::endl, 1);
	if (URI[0] != '/')
		return (status = false, std::cout << "Error invalid uri" << std::endl, 1);	
	route.set_URI(URI);

	std::string rest;
	while (s < words.size())
	{
		rest += " ";
		rest += words[s];
		s++;
	}
	lines[i] = rest;
	return 0;
	
}

bool isinvec(std::vector<std::string> vec_methods, std::string method)
{
		for (std::vector <std::string>::iterator it = vec_methods.begin(); it != vec_methods.end(); ++it)
    	{
			if (*it == method)
				return true;
		}
	return false;
}

std::vector  <std::string> pars_methods(std::vector <std::string> words, bool& check)
{
	size_t i = 1;
	std::vector <std::string> methods;
	if (words.size() == 1)
		return (check = false, std::cout << "Error There is no methods" << std::endl, methods);
	while ( i < words.size())
	{
		if (!isinvec(methods, words[i]))
		{
			if (words[i].find('{') != std::string::npos || words[i].find('}') != std::string::npos)
				return (check = false, std::cout << "Error unwanted braces" << std::endl, methods);
			if (words[i] != "GET" && words[i] != "POST" && words[i] != "DELETE")
				return (check = false, std::cout << "ERROR unwanted method" << std::endl, methods);
			methods.push_back(words[i]);
		}
		i++;
	}
	return methods;	
}

bool pars_autoindex(std::vector <std::string> words, bool& check)
{
	if (words.size() == 1)
		return (check = false, std::cout << "Error no autoindex parameter" << std::endl, false);
	if (words.size() > 2)
		return (check = false, std::cout << "Error many autoindex parameters" << std::endl, false);
	if (words[1] == "on")
		return true;
	else if (words[1] == "off")
		return false;
	return (check = false, std::cout << "Error Invalid autoindex parameter" << std::endl, false);
}

std::vector <std::string> pars_cgi_ext(std::vector <std::string> words, bool& check)
{
	std::vector <std::string> extension;
	if (words.size() == 1)
		return (check = false , std::cout << "Error No cgi extension" << std::endl, extension);
	size_t i = 1;
	while (i < words.size())
	{
		if (words[i][0] != '.')
			return (check = false , std::cout << "Error Invalid extension" << std::endl, extension);
		if (words[i].find('{') != std::string::npos || words[i].find('}') != std::string::npos)
			return (check = false, std::cout << "Error unwanted brace" << std::endl, extension);
		extension.push_back(words[i]);
		i++;
	}
	return extension;
}

std::string pars_temp_path(std::vector <std::string> words,bool& check)
{
	std::string tmp_path;
	if (words.size() == 1)
		return (check= false, std::cout << "Error NO temp_path" << std::endl, tmp_path);
	if (words.size() > 2)
		return (check= false, std::cout << "Error too many temp_path" << std::endl, tmp_path);
	//check the path
	if (words[1].find('{') != std::string::npos || words[1].find('}') != std::string::npos)
		return (check = false, std::cout << "Error unwanted braces" << std::endl, tmp_path);
	if (check_path(words[1]) == false)
		return (check = false, std::cout << "Error client_body_temp_path not exist" << std::endl, tmp_path);
	if (check_type(words[1]) != 0)
		return (check  = false , std::cout << "Error client_body_temp_path not directory" << std::endl, tmp_path);
	tmp_path = words[1];
	return (tmp_path);
}

std::map <int, std::string> get_redirection(std::vector<std::string> words, bool &status)
{
    size_t i = 1;
    size_t j = 0;
    int code;
    std::map <int, std::string> redirection;
    if(words.size() != 3)
	{
        return (status = false, std::cout << "Error invalid number of arguments in return directive" << std::endl, redirection);
	}
	if (words[2].find('{') != std::string::npos || words[2].find('}') != std::string::npos)
		return (status = false, std::cout << "Error unwanted braces" << std::endl, redirection);
    while(j < words[i].size())
    {
        if (!isdigit(words[i][j]))
		{
			
            return (status = false, std:: cout << "Error invalid redirection's status code : " << i << std::endl , redirection);
		}
        j++;
    }
    std::stringstream ss(words[i]);
    ss >> code;
    if (code != 301 && code != 302 && code != 303 && code != 307 && code != 308)
        return (status = false, std:: cout << "Error invalid code" << std::endl , redirection);
    redirection[code] = words[2];
    i++;
    return redirection;
}


bool fill_route(RouteBlock& route, std::vector <std::string>& lines,  size_t& i)
{
	bool status = true;
	std::vector<std::string> words = get_words(lines[i]);
	while (i < lines.size())
	{
		if (status == false)
			return false;
		if (words[0] == "}" || words[0][0] == '}')
		{
			if (words[0] == "}" && words.size() == 1)
				return (std::cerr << "Error unexpected ';' after routeblock brace" << std::endl, false);
			if (words[0].length() > 1) // case of have }location or  }}
				words[0] = words[0].substr(1);
			else if (words[0] == "}" && words.size() > 1) //case of having } location
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
			return (status);
		}
		else if (words[0] == "allowed_methods")
			route.set_methods(pars_methods(words, status));
		else if (words[0] == "autoindex")
			route.set_autoindex(pars_autoindex(words, status));
		else if (words[0] == "index")
			route.set_index(pars_index(words, status));
		else if (words[0] == "cgi_extension")
			route.set_cgi_ext(pars_cgi_ext(words, status));
		else if (words[0] == "client_body_temp_path")
			route.set_client_body_temp_path(pars_temp_path(words, status));
		else if (words[0] == "root")
			route.set_root(get_root(words, status));
		else if (words[0] == "return")
            route.set_redirections(get_redirection(words, status));
		else
		{
			std::cout << "word: " << words[0] << " invalid element in routeblock" << std::endl;
			return false;
		}
		i++;
		words = get_words(lines[i]);
	}
	return status;
}


bool check_route_status(RouteBlock& route, bool status)
{
	if (route.get_autoindex() == true)
		std::cout << "auto index: on" << std::endl;
	else
		std::cout << "auto index: off" << std::endl;
	std::vector <std::string> indexs = route.get_index();
	if (indexs.empty())
	{
		std::cout << "here" << std::endl; 
		status = false;		
	}
	for (size_t x = 0; x < indexs.size(); ++x) {
			std::cout << "index " << x + 1 << ": " << indexs[x] << std::endl;
	}	
	std::vector <std::string> extension = route.get_cgi_ext();
	if (extension.empty())
		status = false;
	for (size_t j = 0; j < extension.size(); ++j) {
			std::cout << "extension " << j + 1 << ": " << extension[j] << std::endl;
	}	
	// std::string tmppath = ;
	std::cout << "tmp_path; "<< route.get_client_body_temp_path() << std::endl;
	return status;
}

std::vector <RouteBlock> pars_routes(std::vector <std::string>& lines, size_t& i, bool& status)
{
	size_t hold = i;
	std::vector <RouteBlock> vec_routes;
	
	while(i < lines.size())
	{
		if (status == false)
			break;
		RouteBlock route;
		if (check_first_line(route, lines, i, status))
		{
			return (vec_routes);
		}
		std::vector <std::string> words = get_words(lines[i]);
		hold  = i;
		i = hold;
		status = fill_route(route, lines , i);
		if (!status)
			return (status = false, std::cout << "not valid routeblock"  << std::endl, vec_routes);
		vec_routes.push_back(route);
	}
	 return vec_routes;
}