#include "../webserver.hpp"


void parse_request(Client &client)
{
    client.get_request().set_parse_index(true);

    std::string res = client.get_response().get_response();

    std::string requestData = client.get_request().get_s_request();
    std::string line;

    size_t bodyStart = requestData.find("\r\n\r\n");
    if (bodyStart == std::string::npos){
        get_error_res(res, 400);
        client.get_response().set_response_index(true);
        std::cout << "ha wahd l error";
        return;
    }
    std::string new_request = requestData.substr(bodyStart + 4);
    client.get_request().set_s_request(new_request);
    requestData = requestData.substr(0 , bodyStart + 4);




    std::istringstream requestLine(requestData);
    std::getline(requestLine, line);
    std::istringstream Line(line);


    if (!check_request_line(line)){
        get_error_res(res, 400);
        client.get_response().set_response(res);
        client.get_response().set_response_index(true);
        return ;
    }

    std::string method, path, version, error;
    Line >> method >> path >> version >> error;


    if (error.size() > 0 || !method.size() || !path.size() || !version.size())
    {
        get_error_res(res, 400);
        client.get_response().set_response(res);
        client.get_response().set_response_index(true);
        return ;
    }

    client.get_request().set_method(method);
    client.get_request().set_path(path);
    client.get_request().set_version(version);

    if (client.get_request().get_method() != "GET" && client.get_request().get_method() != "POST" && client.get_request().get_method() != "DELETE")
    {
        if (!is_upper(client.get_request().get_method()))
        {
            get_error_res(res, 400);
            client.get_response().set_response_index(true);
            return ;
        }
        else
        {
            get_error_res(res, 405);
            client.get_response().set_response_index(true);
            return ;
        }
    }



    // if (client.get_request().get_method() == "POST"){
    //     if (client.get_request().fill_headers_map(requestStream, res) == 0){
    //         return ;
    //     }
    //     hanlde_post_request(client , 1 , requestData);
    //     std::cout <<"\033[38;5;214m"<<"POST request ====> "<< method<< " "<<path<<" "<< version<<" "<<"\033[0m" << std::endl;
    //     return ;
    // }

    // if (client.get_request().get_method() == "DELETE"){

    //     std::cout <<"\033[1;31m"<<"DELETE request ====> "<< method<< " "<<path<<" "<< version<<" "<<"\033[0m" << std::endl;
    //     return ;
    // }


    
    if (client.get_request().get_version() != "HTTP/1.1")
    {
        if (strncmp(client.get_request().get_version().c_str(), "HTTP/", 5) > 0)
        {
            get_error_res(res, 400);
            client.get_response().set_response(res);
            client.get_response().set_response_index(true);
            return ;
        }
        else
        {
            get_error_res(res, 505);
            client.get_response().set_response_index(true);
            return ;
        }
    }


    std::string pa = client.get_request().get_path();
    if (pa[0] != '/')
    {
        get_error_res(res, 400);
        client.get_response().set_response(res);
        client.get_response().set_response_index(true);
        return ;
    }
    pa = removeslashes(pa);
    if (!out_root_dir(pa, res)){
        client.get_response().set_response_index(true);
        return ;
    }
    client.get_request().set_path(pa);
    if (client.get_request().fill_headers_map(requestLine, res) == 0){
        client.get_response().set_response_index(true);
        return ;
    }
    
}


void check_request(Client & client){
    if (!client.get_request().get_parse_index())
        parse_request(client);
    if (client.get_response().get_response_index())
        return ;
    if (client.get_request().get_method() == "GET")
        response_to_get(client);
    
    // else if (client.get_request().get_method() == "POST"){
    //     std::string check = client.get_request().get_map_values("Content-Type");
    //     size_t pos = check.find("boundary=");
    //     if (pos != std::string::npos){
    //         // boundary(client);
    //         return ;
    //     }
    //     check = client.get_request().get_map_values("Transfer-Encoding");
    //     trim_non_printable(check);
    //     if (check == " chunked"){
    //         chunked(client);
    //         return ;
    //     }

    //     hanlde_post_request(client);
    //     return ;
    // }

}


