/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_tools.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kid-ouis <kid-ouis@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/09 14:58:14 by kid-ouis          #+#    #+#             */
/*   Updated: 2025/04/09 17:46:39 by kid-ouis         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../webserver.hpp"

bool check_path(std::string &path)
{
	struct stat info;
	if (stat(path.c_str(), &info) == 0)
		return true;
	return false;	
}

size_t check_type(std::string &path)
{
	struct stat info;
	if (stat(path.c_str(), &info) == 0)
	{
		if (S_ISREG(info.st_mode))
			return 1;
		else if (S_ISDIR(info.st_mode))
			return 0;
	}
	return 2;	
}

int check_index(std::vector <std::string> &index, std::string root)
{
	index.erase(index.begin());
	std::string tmp_path;
	size_t i = 0;
	while (i < index.size())
	{
		tmp_path = root + "/" + index[i];
		if (check_path(tmp_path) == true)
		{
			if (check_type(tmp_path) != 1)
				return (std::cout << "Error index not file" << std::endl, 1);
			if (access(tmp_path.c_str(), R_OK) != 0)
				return (std::cout << "Error index read permission denied" << std::endl, 1);
		}
		else
			return (std::cout << "Error index file not found" << std::endl, 1);
		i++;
	}
	return 0;
}