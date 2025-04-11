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

#ifndef ROUTEBLOCK_HPP
#define ROUTEBLOCK_HPP
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
        // std::string redirection;
    public:
        RouteBlock();
        ~RouteBlock();
        RouteBlock(const RouteBlock& other);
        RouteBlock& operator=(const RouteBlock& other);
        void set_URI(std::string set_uri);
        void set_root(std::string set_root);
        void set_methods(std::vector <std::string> set_methods);
        std::string get_uri(void);
        std::string get_root(void);
        std::vector <std::string> get_methods(void);
        void set_autoindex(bool set_autoindex);
        bool get_autoindex(void);
        void set_index(std::vector <std::string>  set_index);
        std::vector <std::string>  get_index(void);
        void set_cgi_ext( std::vector<std::string> set_cgi);
         std::vector<std::string> get_cgi_ext(void);
        void set_client_body_temp_path(std::string tmp_path);
        std::string get_client_body_temp_path(void);
        // void set_redirection(std::string set_redirection);
        // std::string get_redirection(void);
};


#endif