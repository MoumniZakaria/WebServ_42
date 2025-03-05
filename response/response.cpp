#include "../webserver.hpp"

std::ifstream & Response::get_fileStream(){
    return *fileStream;
}

void Response::set_fileStream(std::ifstream & object){
    fileStream = &object;
}

std::string & Response::get_response(){
    return response;
}
void Response::set_response(std::string &response){
    this->response = response; 
}
Response::~Response(){};

Response::Response()
{
    fileStream = NULL;
    response = "";
};