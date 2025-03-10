#include "../webserver.hpp"

Client::Client() : client_id(-1) {
    
}


Client::Client(int fd, struct sockaddr_in Add)
{
    request_object = new Request();
    response_object = new Response();
    client_id = fd;
    Client_Addr = Add;
    keep_alive = true;
    all_recv = false;
    std::cout << "Client created with fd: " << client_id << std::endl;
    std::cout << "Client created with req: " << request_object << std::endl;
}
int  Client::get_client_id(){
    return client_id;
}

void  Client::set_client_id(int fd){
    client_id = fd;
}

Request & Client::get_request(){
    return *request_object;
}

void  Client::set_request(Request &R){

    request_object = &R;
}

Response&  Client::get_response(){
    return *response_object;
}

void Client::set_response(Response & R) {
        response_object = &R;
}

void Client::set_Alive(bool keep){
    keep_alive = keep;
}

bool Client::get_Alive(){
    return keep_alive;
}


bool Client::get_all_recv(){
    return all_recv;
}

void Client::set_all_recv(bool check){
    all_recv = check;
}