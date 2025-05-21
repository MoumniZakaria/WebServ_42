/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RouteBlock.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 02:28:07 by marvin            #+#    #+#             */
/*   Updated: 2025/03/27 02:28:07 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "../webserver.hpp"

class RouteBlock{
    private: 
        std::string URI;
        std::string root;
        std::vector<std::string> methods;
        bool autoindex;
        std::vector <std::string>  index;
         std::vector<std::string> cgi_extension;  
        std::string client_body_temp_path;   
        std::map <int, std::string> redirections;
    public:
        RouteBlock();
        ~RouteBlock();
        RouteBlock(const RouteBlock& other);
        RouteBlock& operator=(const RouteBlock& other);
        void set_URI(std::string set_uri);
        void set_root(std::string set_root);
        void set_methods(std::vector <std::string> set_methods);
        void set_autoindex(bool set_autoindex);
        void set_index(std::vector <std::string>  set_index);
        void set_cgi_ext( std::vector<std::string> set_cgi);
        std::vector<std::string> get_cgi_ext(void);
        void set_client_body_temp_path(std::string tmp_path);
        std::string get_uri(void);
        std::string get_root(void);
        bool get_autoindex(void);
        std::vector <std::string>  get_index(void);
        std::vector <std::string> get_methods(void);
        std::string get_client_body_temp_path(void);
        void set_redirections(std::map<int, std::string> set_redirection);
        std::map <int, std::string> get_redirections(void);
};

