#pragma once

#include "../webserver.hpp"

class Response
{
    private:
        std::ifstream *fileStream;
        std::string response;
    public:
        std::ifstream & get_fileStream();
        void set_fileStream(std::ifstream& object);
        std::string & get_response();
        void set_response(std::string& response);
        Response();
        ~Response();
};
