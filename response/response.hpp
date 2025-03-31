#pragma once

#include "../webserver.hpp"

class Response
{
    private:
        std::ifstream *fileStream;
        std::string response;
        bool Ready_to_send;
        int response_status;
    public:
        std::ifstream & get_fileStream();
        void set_fileStream(std::ifstream& object);
        std::string & get_response();
        void set_response(std::string& response);


        bool get_response_index();
        void set_response_index(bool index);


        int  get_response_status();
        void set_response_status(int  index);

        void reset()
        {
            fileStream->close();
            delete fileStream;
            fileStream = NULL;
            response.clear();
            Ready_to_send = false;
            response_status = 0;
        }
        Response();
        ~Response();
};
