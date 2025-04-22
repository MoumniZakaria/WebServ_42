#include "../webserver.hpp"

Client::Client() : client_id(-1) {
}

Client::Client(int fd, struct sockaddr_in Add, ServerBlock obj)
{
    (void)obj;
    ServerBlock tmp_srvblock_obj = obj;
    request_object = new Request();
    response_object = new Response();
    client_id = fd;
    Client_Addr = Add;
    keep_alive = true;
    all_recv = false;
    // server_client_obj = tmp_srvblock_obj;
    // std::cout << "Client created with fd: " << client_id << std::endl;
    // std::cout << "Client created with req: " << request_object << std::endl;
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

void Client::fill_map(std::string key , std::string value){
    form_data[key] = value;
}


void Client::print_map(){
    std::map<std::string , std::string >::iterator it = form_data.begin();
    while(it != form_data.end()){
        std::cout << it->first << " ---------> " << it->second << std::endl;
        ++it ;
    }
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

void Client::reset() {
    // Reset the client state
    // client_id = -1;
    //free address
    // Client_Addr = {};
    delete request_object;
    delete response_object;
    request_object = new Request();
    response_object = new Response();
    keep_alive = true;
    all_recv = false;
}