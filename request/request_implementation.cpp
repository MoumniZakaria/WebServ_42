#include "../webserver.hpp"

void parse_request(Client &client)
{
    client.get_request().set_parse_index(true);

    std::string &res = client.get_response().get_response();

    std::string requestData = client.get_request().get_s_request();
    std::string line;

    size_t bodyStart = requestData.find("\r\n\r\n");
    if (bodyStart == std::string::npos)
    {
        get_error_res(res, 400 , client);
        client.get_response().set_response(res);
        client.get_response().set_response_index(true);
        std::cout << "ha wahd l error";
        return;
    }
    std::string new_request = requestData.substr(bodyStart + 4);
    client.get_request().set_s_request(new_request);
    requestData = requestData.substr(0, bodyStart + 4);

    std::istringstream requestLine(requestData);
    std::getline(requestLine, line);
    std::istringstream Line(line);

    if (!check_request_line(line))
    {
        get_error_res(res, 400 , client);
        client.get_response().set_response(res);
        client.get_response().set_response_index(true);
        return;
    }

    std::string method, path, version, error;
    Line >> method >> path >> version >> error;

    if (error.size() > 0 || !method.size() || !path.size() || !version.size())
    {
        get_error_res(res, 400 , client);
        client.get_response().set_response(res);
        client.get_response().set_response_index(true);
        return;
    }

    client.get_request().set_method(method);
    client.get_request().set_path(path);
    client.get_request().set_version(version);

    if (client.get_request().get_method() != "GET" && client.get_request().get_method() != "POST" && client.get_request().get_method() != "DELETE")
    {
        if (!is_upper(client.get_request().get_method()))
        {
            get_error_res(res, 400 , client);
            client.get_response().set_response_index(true);
            return;
        }
        else
        {
            get_error_res(res, 405 , client);
            client.get_response().set_response_index(true);
            return;
        }
    }

    if (client.get_request().get_version() != "HTTP/1.1")
    {
        if (strncmp(client.get_request().get_version().c_str(), "HTTP/", 5) > 0)
        {
            get_error_res(res, 400 , client);
            client.get_response().set_response(res);
            client.get_response().set_response_index(true);
            return;
        }
        else
        {
            get_error_res(res, 505 , client);
            client.get_response().set_response(res);
            client.get_response().set_response_index(true);
            return;
        }
    }

    std::string pa = client.get_request().get_path();
    if (pa[0] != '/')
    {
        get_error_res(res, 400 , client);
        client.get_response().set_response(res);
        client.get_response().set_response_index(true);
        return;
    }
    pa = removeslashes(pa);
    if (!out_root_dir(pa, res , client))
    {
        client.get_response().set_response(res);
        client.get_response().set_response_index(true);
        return;
    }
    client.get_request().set_path(pa);
    if (client.get_request().fill_headers_map(requestLine, res , client) == 0)
    {
        client.get_response().set_response_index(true);
        return;
    }
    std::string tmp = client.get_request().get_map_values("Content-Length");


    if (method == "POST"){
    std::istringstream ss(tmp);
    size_t size;
    ss >> size;
    client.get_request().set_content_length(size);
    if (ss.fail()){
        client.get_response().set_response_index(true);
        get_error_res(res, 400 , client);
    }
}

}


void handle_x_www_form_urlencoded(Client &client){
    std::string tmp =  client.get_request().get_s_request();
    std::istringstream ss(tmp);
    std::string line;
    std::string key;
    std::string value;

    client.set_all_recv(true);
    while(std::getline(ss , line , '&')){
        size_t pos = line.find("=");
        if (pos == std::string::npos){
            std::cout << "error" << std::endl;
            exit (0);
        }
        key = line.substr(0 , pos);
        value = line.substr(pos + 1);
        std::cout << key << " -----------> "  << value << std::endl;
    }
}


void  handle_delete_request(std::string path)
{
    struct stat buf ;

    if (stat(path.c_str(), &buf) == -1) {
        std::cerr << "Failed to stat path: " << path << std::endl;
        throw std::runtime_error("Invalid path");
        // exit (0);
    }
    if (S_ISDIR(buf.st_mode)){
        DIR* dir = opendir(path.c_str());
        if (dir == NULL) {
            throw std::runtime_error("cannot open dir ");
        }
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (std::strcmp(entry->d_name, ".") == 0 || std::strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            std::string fullPath = std::string(path) + "/" + entry->d_name;
            handle_delete_request(fullPath);
        }
        if (remove(path.c_str())){
            throw std::runtime_error("remove field , can't remove dir ");
        }
        closedir(dir);
    }
    else{
        if (remove(path.c_str()) != 0){
            throw std::runtime_error("remove field , can't remove file ");
        }
    }
}


void check_request(Client &client)
{
    // std::cout << client.get_request().get_s_request() ;
    // return ;
    if (!client.get_request().get_parse_index())
    parse_request(client);
    
    if (client.get_response().get_response_index())
    return;
    
    const std::string method = client.get_request().get_method();
    const std::string content_type = client.get_request().get_map_values("Content-Type");
    const std::string transfer_encoding = client.get_request().get_map_values("Transfer-Encoding");
    
    if (method == "GET") {
        client.set_all_recv(true);
        response_to_get(client);
        std::cout << "\033[32m" << "Responsed by ====> " << client.get_response().get_response_status() <<  "\033[0m" << std::endl;
        return;
    }


    else if (method == "POST") {
        std::cout << "\033[38;5;214m" << "POST request ====> " << method << " "
                  << client.get_request().get_path() << " " 
                  << client.get_request().get_version() << " " << "\033[0m" << std::endl;
        client.get_response().set_response_status(200);
        std::string res = "HTTP/1.1 200 File uploaded successfully \r\nContent-Type: text/html\r\n\r\n\
            <html><head><title>200 File uploaded successfully </title></head><body><center><h1>200 File uploaded successfully </h1></center>\
            <hr><center>42 webserv 0.1</center></body></html>";
        client.get_response().set_response(res);
        

        std::string check = transfer_encoding;

        if (content_type.find("boundary=") != std::string::npos && check == "chunked") {
            handle_boundary_chanked(client);
            // return;
        }

        else if (content_type.find("boundary=") != std::string::npos) {
            boundary(client);
            // return;
        }

        else if (check == "chunked") {
            chunked(client);
            // return;
        }

        else if (check == "application/x-www-form-urlencoded") {
            handle_x_www_form_urlencoded(client);
            // return;
        }
        else{
            hanlde_post_request(client);
        }
        if (client.get_all_recv() == true){
            std::cout << "\033[32m" << "Responsed by ====> " << client.get_response().get_response_status() <<  "\033[0m" << std::endl;
        }
    }
    else if (method == "DELETE"){
        client.set_all_recv(true); // check ila chi mecrob 3amr l headers b ktar mn buffer size
        std::cout <<"\033[1;31m"<<"DELETE request ====> "<< method<< " "<<client.get_request().get_path()<<" "<< client.get_request().get_version()<<" "<<"\033[0m" << std::endl;
        client.get_response().set_response_status(200);
        std::string res = "HTTP/1.1 200 File deleted \r\nContent-Type: text/html\r\n\r\n\
            <html><head><title>200 File deleted </title></head><body><center><h1>200 File deleted </h1></center>\
            <hr><center>42 webserv 0.1</center></body></html>";
        std::string path = root  + client.get_request().get_path();
        handle_delete_request(path);
        std::cout << "\033[32m" << "Responsed by ====> " << client.get_response().get_response_status() <<  "\033[0m" << std::endl;
    }
}

