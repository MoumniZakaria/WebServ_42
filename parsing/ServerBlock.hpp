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

#ifndef SERVERBLOCK_HPP
#define SERVERBLOCK_HPP

#include "../webserver.hpp"
#include "RouteBlock.hpp"

class RouteBlock;
class ServerBlock{
    private:
        std::string                    Host;
        int                             Port;
        std::vector<std::string>       Server_names;
        std::string                    server_root;
        size_t                         client_body_size;
        std::vector <std::string>      index;
        std::vector<RouteBlock>        routes;
        std::map<int, std::string>     error_pages;
        // std::string                    client_body_temp_path;
    public:
    size_t brace_count;
        ServerBlock();
        ~ServerBlock();
        ServerBlock(const ServerBlock& other);
        ServerBlock& operator=(const ServerBlock& other);
        void set_host(std::string set_host);
        void set_port(int set_port);
        void set_server_names(std::vector<std::string> set_server_names);
        void set_server_root(std::string server_root_arg);
        void set_client_body_size(size_t client_body_size);
        void set_index(std::vector <std::string>  set_index);
        void set_routes(std::vector<RouteBlock> routes);
        void set_error_pages(std::map<int , std::string> set_error_pages);
        std::string& get_server_root(void);
        std::string& get_host(void);
        int get_port(void);
        std::vector <std::string>  get_server_names(void);
        std::vector <std::string>  get_index(void);
        size_t get_client_body_size(void);
        std::map<int , std::string> get_error_pages(void);
        std::vector<RouteBlock> get_routes(void);

};
#endif