/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerBlock.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 23:54:48 by marvin            #+#    #+#             */
/*   Updated: 2025/03/26 23:54:48 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../webserver.hpp"

class ServerBlock{
    private:
        std::string                    Host;
        std::vector <int>              Port;
        std::string                    Server_names;
        std::string                    server_root;
        size_t                         client_body_size;
        std::vector <std::string>      index;
        std::vector<RouteBlock>        routes;
        std::map<int, std::string>     error_pages;
    public:
        int is_location_url;
        ServerBlock();
        ServerBlock(const ServerBlock& other);
        ServerBlock& operator=(const ServerBlock& other);
        void set_host(std::string set_host);
        void set_port(int set_port);
        void set_server_names(std::string set_server_names);
        void set_server_root(std::string server_root_arg);
        void set_client_body_size(size_t client_body_size);
        void set_index(std::vector <std::string>  set_index);
        void set_routes(std::vector<RouteBlock> routes);
        void set_error_pages(std::map<int , std::string> set_error_pages);
        std::string& get_server_root(void);
        std::string& get_host(void);
        std::vector <int> get_port(void);
        std::string  get_server_names(void);
        std::vector <std::string>  get_index(void);
        size_t get_client_body_size(void);
        std::map<int , std::string> get_error_pages(void);
        std::vector<RouteBlock> get_routes(void);
        bool is_valid_method(std::string path , std::string method);
        std::string find_error_page_path(int n);
        std::string  is_location_path(std::string path);
        void set_dafault_data(void);
        void update_server_info(RouteBlock &route);

};
