#include "../webserver.hpp"


void parse_request(Client &client)
{
    std::string request_buffer = client.get_request().get_s_request();

    std::string res;
    std::string requestData = client.get_request().get_s_request();

    size_t bodyStart = requestData.find("\r\n\r\n");
    requestData = requestData.substr(bodyStart + 4);

    std::istringstream requestStream(request_buffer);
    std::string line;
    std::getline(requestStream, line);

    std::istringstream requestLine(line);
    if (!check_request_line(line)){
        get_error_res(res, 400);
        client.get_response().set_response(res);
        return ;
    }
    std::string method, path, version, error;
    requestLine >> method >> path >> version >> error;
    if (error.size() > 0 || !method.size() || !path.size() || !version.size())
    {
        get_error_res(res, 400);
        client.get_response().set_response(res);
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
            return ;
        }
        else
        {
            get_error_res(res, 405);
            return ;
        }
    }

    if (client.get_request().get_method() == "POST"){
        if (client.get_request().fill_headers_map(requestStream, res) == 0){
            return ;
        }
        hanlde_post_request(client , 1 , requestData);
        std::cout <<"\033[38;5;214m"<<"POST request ====> "<< method<< " "<<path<<" "<< version<<" "<<"\033[0m" << std::endl;
        return ;
    }

    if (client.get_request().get_method() == "DELETE"){

        std::cout <<"\033[1;31m"<<"DELETE request ====> "<< method<< " "<<path<<" "<< version<<" "<<"\033[0m" << std::endl;
        return ;
    }
    std::cout << "\033[34m" << "GET request ====> "<< method<<  " "<< path << " "<< version << " " << "\033[0m" << std::endl;
    if (client.get_request().get_version() != "HTTP/1.1")
    {
        if (strncmp(client.get_request().get_version().c_str(), "HTTP/", 5) > 0)
        {
            get_error_res(res, 400);
            client.get_response().set_response(res);
            return ;
        }
        else
        {
            get_error_res(res, 505);
            return ;
        }
    }
    std::string pa = client.get_request().get_path();
    if (pa[0] != '/')
    {
        get_error_res(res, 400);
        client.get_response().set_response(res);
        return ;
    }
    pa = removeslashes(pa);
    if (!out_root_dir(pa, res))
        return ;
    client.get_request().set_path(pa);

    if (client.get_request().fill_headers_map(requestStream, res) == 0)
        return ;

    if (client.get_request().get_path() == "/")
    {
        std::string p = "/index.html";
        client.get_request().set_path(p);
    }
    std::string pat = "www/" + client.get_request().get_path().substr(1);
    client.get_request().set_path(pat);

    struct stat path_stat;
    if (stat(client.get_request().get_path().c_str(), &path_stat) == -1)
    {
        std::cerr << "Error: stat field" << std::endl;
        res = "HTTP/1.1 404 not found\r\nContent-Type: text/html\r\n\r\n\
        <html><head><title>404 not found</title></head><body><center><h1>404 not found</h1></center>\
        <hr><center>42 webserv 0.1</center></body></html>";
        client.get_response().set_response(res);
        return ;
    }

    else if (S_ISDIR(path_stat.st_mode))
    {
        if (access(pat.c_str(), R_OK | W_OK | X_OK) == -1)
        {
            res = "HTTP/1.1 403 Forbidden\r\nContent-Type: text/html\r\n\r\n\
                <html><head><title>403 Forbidden</title></head><body><center><h1>403 Forbidden</h1></center>\
                <hr><center>42 webserv 0.1</center></body></html>";
                client.get_response().set_response(res);
            return ;
        }
        DIR *dir = opendir(pat.c_str());

        if (dir == NULL)
        {
            std::cerr << "Error opening directory: " << strerror(errno) << std::endl;
            res = "HTTP/1.1 404 not found\r\nContent-Type: text/html\r\n\r\n\
                <html><head><title>404 not found</title></head><body><center><h1>404 not found</h1></center>\
                <hr><center>42 webserv 0.1</center></body></html>";
                client.get_response().set_response(res);
            return ;
        }

        struct dirent *entry;
        res = "HTTP/1.1 200 OK\r\n";
        res += "Content-Type: text/html; charset=UTF-8\r\n";
        res += "Connection: close\r\n";
        res += "\r\n";

        res += "<html>\n<head>\n<title>Found Files</title>\n</head>\n<body>\n";
        res += "<h1>Found Files in Directory</h1>\n<ul>\n";
        while ((entry = readdir(dir)) != NULL)
        {
            std::string fileName = entry->d_name;
            res += "<li><a href=\"/" + pat.substr(4) +"/" + fileName + "\">" + fileName + "</a></li>\n";
            // 4 must be change by root dir size 
        }
        res += "</ul>\n</body>\n</html>\n";
        client.get_response().set_response(res);
        closedir(dir);
    }

    else if (S_ISREG(path_stat.st_mode)){
        res = fill_response(client.get_response().get_fileStream() , pat);
        client.get_response().set_response(res);
    }
    else
        res = "HTTP/1.1 404 not found\r\nContent-Type: text/html\r\n\r\n\
            <html><head><title>404 not found</title></head><body><center><h1>404 not found</h1></center>\
            <hr><center>42 webserv 0.1</center></body></html>";
            client.get_response().set_response(res);
    return ;
}


void check_request(Client & client){
    std::cout << "check_request" << std::endl;
    std::cout << "request : " << &client.get_request() << std::endl;
    std::cout << "response : " <<  &client.get_response()<< std::endl;


    // std::string method = client.get_request().get_method();

    // if (client.get_request().get_method().empty()){
    //     parse_request(client);
    // }
    // else{
    //     hanlde_post_request(client , 0 , "ssss");
    // }
}


