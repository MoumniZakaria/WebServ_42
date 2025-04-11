/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pars_route.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kid-ouis <kid-ouis@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/05 12:01:28 by kid-ouis          #+#    #+#             */
/*   Updated: 2025/04/09 17:46:30 by kid-ouis         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../webserver.hpp"

#include "Confile.hpp"

int check_first_line(RouteBlock& route, std::vector <std::string>& lines, size_t i, bool& status)
{
	int last_char;
	size_t s = 3;
	std::string URI;
	std::vector <std::string> words = get_words(lines[i]);
	if (words[0] != "location" && words[0] != "location{")
		return (std::cout << "end of locationblock" << std::endl, 1);
	if (words.size() < 4)
			return (status = false, std::cout << "invalid location structre" << std::endl, 1);
	last_char = words[1].length() - 1;
	if (words[1][last_char] == '{')
	{
		URI = words[1].substr(0, last_char);
		// std::cout << "uri: " << URI << std::endl;	
		route.set_URI(URI);
		s = 2;
	}
	else if (words[2] != "{")
			return (status = false, std::cout << "invalid location structre" << std::endl, 1);
		std::string rest;
	while (s < words.size())
	{
		rest += " ";
		rest += words[s];
		s++;
	}
	// std::cout << "rest" << rest << std::endl;
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
	// size_t j = i;
	std::vector <std::string> methods;
	if (words.size() == 1)
		return (check = false, std::cout << "Error There is no methods" << std::endl, methods);
	while ( i < words.size())
	{
		if (!isinvec(methods, words[i]))
		{
			if (words[i] != "GET" && words[i] != "POST" && words[i] != "DELETE")
				return (check = false, std::cout << "ERROR unwanted method" << std::endl, methods);
			methods.push_back(words[i]);
		}
		i++;
	}
		// for (std::vector <std::string>::iterator it = methods.begin(); it != methods.end(); ++it)
    	// {
		// 	std::cout << "method: " << *it << std::endl;
		// }
			// std::cout << "#==> methods get parsed" << std::endl;
	return methods;	
}

bool pars_autoindex(std::vector <std::string> words, bool& check)
{
		// std::cout << "#==> autoindex get parsed" << std::endl;
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
		extension.push_back(words[i]);
		i++;
	}
		// std::cout << "#==> extension get parsed" << std::endl;
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
		return (std::cout << "Error unwanted braces" << std::endl, tmp_path);
	if (check_path(words[1]) == false)
		return (std::cout << "Error client_body_temp_path not exist" << std::endl, tmp_path);
	if (check_type(words[1]) != 0)
		return (std::cout << "Error client_body_temp_path not directory" << std::endl, tmp_path);
	tmp_path = words[1];
		// std::cout << "#==> temp_path get parsed" << std::endl;
	return (tmp_path);
}



bool fill_route(RouteBlock& route, std::vector <std::string>& lines,  size_t& i)
{
	bool status = true;
	std::vector<std::string> words = get_words(lines[i]);
	while (i < lines.size())
	{
		if (words[0] == "}" || words[0][0] == '}')
		{
			// std::cout << "***line to end routeblock " << lines[i] << "i:" << i << std::endl;
			if (words[0] == "}" && words.size() == 1) //case of  we have just }
				return (std::cout << "end of RouteBlock and  unexpected ';' after brace" << std::endl, false);//false cause even if the routeblock filled if there is char after brace it's error
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
			// std::cout << "rest: " << rest << std::endl;
			lines[i] = rest;
			return (std::cout << "end of RouteBlock" << std::endl, status);				
		}
		else if (words[0] == "allowed_methods")
			route.set_methods(pars_methods(words, status));
		else if (words[0] == "autoindex")
			route.set_autoindex(pars_autoindex(words, status));
		else if (words[0] == "index")
			route.set_index(pars_index(words, route.get_root(), status));
		else if (words[0] == "cgi_extension")
			route.set_cgi_ext(pars_cgi_ext(words, status));
		else if (words[0] == "client_body_temp_path")
			route.set_client_body_temp_path(pars_temp_path(words, status));
		else
		{
			std::cout << "word: " << words[0] << " invalid element in routeblock" << std::endl;
			return false;
		}
			
		i++;
		words.clear();
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
	// bool filled = true;
	std::vector <RouteBlock> vec_routes;
	
	// std::cout << "@@@@ parsing root started @@@@" << std::endl;
	while(i < lines.size())
	{
		if (status == false)
			break;
		// std::cout <<
		// std::cout << "line to check in pars_route: " << lines[i] << " *i : " << i << std::endl;
		RouteBlock route;
		if (check_first_line(route, lines, i, status))
			return (vec_routes);
		// std::cout << "line after chckline: " << lines[i] << " i: " << i  << std::endl;
		std::vector <std::string> words = get_words(lines[i]);
		hold  = i;
		route.set_root(get_root(lines, i));
		lines.erase(lines.begin() + i);
		std::string root = route.get_root();
		if(root.empty())
		{
			i = hold;
			status = false;
			return vec_routes;
		}
		// std::cout << "root: " << root << std::endl;
		i = hold;//back to first line in routeblock after getting root;
		// std::cout << "fill route in i: " << i << " line: "<< lines[i]<< std::endl;
		status = fill_route(route, lines , i);
		// std::cout << "filled after fill route: " << filled << std::endl;
		// filled = check_status_route();
		if (!status)
			return (status = false, std::cout << "not valid routeblock"  << std::endl, vec_routes);
		//we need to check if the element rewuired for location are ffiled even if status true
		// std::cout << "route block end in " << lines[i] << " *i : " << i << std::endl;

			// break;
		vec_routes.push_back(route);
		// i++;
		// std::cout << "====================> next routeblock : "  << std::endl;
	}
	 return vec_routes;
}