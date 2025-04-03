#pragma once 

#include "../webserver.hpp"

class Client{
    private:
        int client_id;
        struct sockaddr_in Client_Addr;
        Request *request_object;
        Response *response_object;
        std::map<std::string , std::string> form_data;
        bool keep_alive;
        bool all_recv;
        
    public:
        void set_client_id(int fd);
        int get_client_id();
        Request  & get_request();
        void  set_request(Request & R);
        Response& get_response();
        void  set_response(Response & R);

        void set_Alive(bool keep);
        bool get_Alive();

        void set_all_recv(bool check);
        bool get_all_recv();

        Client(int fd, struct sockaddr_in Add);

        void fill_map(std::string key , std::string value);
        void print_map();
        Client();
        ~Client(){};
        void reset();
};