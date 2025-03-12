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
        size_t length;
        int BodyStart;
        bool index;


        bool request_end;
        
    public:
        const std::string& get_method();
        const std::string& get_version();
        const std::string& get_path();
        void set_method(std::string & name);
        void set_path(std::string& name);
        void set_version(std::string& name);
        void set_s_request(std::string req);
        std::string get_s_request();
        size_t get_length();
        void set_length(size_t len);
        int get_bodyStart();
        void set_bodyStart(int pos);
        std::string get_map_values(std::string key);
        bool fill_headers_map(std::istringstream &obj , std::string &res);



        bool get_request_end();
        void set_request_end(bool index);


        bool get_parse_index();
        void set_parse_index(bool index);

        Request();
        ~Request();
};
