/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Confile.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 23:50:39 by marvin            #+#    #+#             */
/*   Updated: 2025/03/26 23:50:39 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../webserver.hpp"

class Confile{
    private:
        std::vector <ServerBlock> servers;
    public:
        size_t number_of_server;
        bool status;
        Confile();
        ~Confile();
        Confile(const Confile& other);
        Confile& operator=(const Confile& other);

        ServerBlock pars_server(std::vector<std::string>& lines, size_t &i);
        void set_server(std::vector<std::string> lines);
        // void set_default_server(std::vector <ServerBlock> default_servers);
        std::vector <ServerBlock>& get_server();
};
