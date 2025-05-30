#pragma once

#include "../webserver.hpp"

class Request
{
    private:
        std::string method; 
        std::string version; 
        std::string path;
        std::map<std::string,std::string> headers_map;
        std::string s_request;
        bool index;
        size_t content_length;
        
    public:
        std::ofstream file;

        int redirection;
        std::string Location;
        // int is_error;


        bool is_string_req_send;
        const std::string& get_method();
        const std::string& get_version();
        const std::string& get_path();
        void set_method(std::string & name);
        void set_path(std::string& name);
        void set_version(std::string& name);
        void set_s_request(std::string req);
        std::string get_s_request();
        size_t get_content_length();
        void set_content_length(size_t length);
        
        std::string get_map_values(std::string key);
        bool fill_headers_map(std::istringstream &obj , std::string &res , Client &client);


        // void print_headers(){
        //     std::map<std::string , std::string >::iterator it = headers_map.begin();
        //     while(it != headers_map.end()){
        //         std::cout << it->first << " ---------------> " << it->second << std::endl;
        //         ++it;
        //     }
        // }

        std::map<std::string,std::string> get_headers_map(){
            return headers_map;
        }

        bool get_parse_index();
        void set_parse_index(bool index);

        Request();
        ~Request();
};
