#include "../webserver.hpp"

std::ifstream &Response::get_fileStream()
{
    return *fileStream;
}

void Response::set_fileStream(std::ifstream &object)
{
    fileStream = &object;
}

std::string &Response::get_response()
{
    return response;
}
void Response::set_response(std::string &response)
{
    this->response = response;
}

bool Response::get_response_index()
{
    return Ready_to_send;
}

void Response::set_response_index(bool index)
{
    Ready_to_send = index;
}

int Response::get_response_status()
{
    return response_status;
}
void Response::set_response_status(int index)
{
    response_status = index;
}

Response::~Response() {};

Response::Response()
{
    fileStream = new std::ifstream();
    response = "";
    Ready_to_send = false;
};


std::string check_auto_index(Client &client , int *index){

    int i = 0;
    *index = 2;

    while(i < (int)client.server_client_obj.get_routes().size()){
        if (client.get_request().get_path() == (client.server_client_obj.get_routes()[i].get_root() + client.server_client_obj.get_routes()[i].get_uri()))
        {
            if (!client.server_client_obj.get_routes()[i].get_autoindex()){
                *index = 1;
                set_response_error(&client , 403);
                return  "done";
            }
        }
        i++;
    }
    return "";
}



void response_to_get(Client &client)
{
    std::cout << "\033[34m" << "GET request ====> " << client.get_request().get_method() << " " << client.get_request().get_path() << " " << "\033[0m" << std::endl;
    std::string res = client.get_response().get_response();
    std::string pat =  client.get_request().get_path();


    client.get_request().set_path(pat);
    struct stat path_stat;


    if (stat(client.get_request().get_path().c_str(), &path_stat) == -1)
    {
        set_response_error(&client , 404);
        return;
    }
    
    else if (S_ISDIR(path_stat.st_mode))
    {
        if (access(pat.c_str(), R_OK | W_OK | X_OK) == -1)
        {
            set_response_error(&client , 403);
            return;
        }
        DIR *dir = opendir(pat.c_str());
        
        if (dir == NULL)
        {
            set_response_error(&client , 404);
            return;
        }
        int flag = 0;
        struct stat default_file;
        std::string str ;
        
        



        if (!client.server_client_obj.get_index().size() && client.server_client_obj.is_location_url == -1){            // i don't have a indexes and the url is not location 
            str =  client.get_request().get_path() + "/" + "index.html";
        }
        
        else if (client.server_client_obj.is_location_url  > -1){                          //  if i have locaion 
            if (client.server_client_obj.get_routes()[client.server_client_obj.is_location_url].get_index().size() == 0)
                str =  client.get_request().get_path() + "/" + "index.html";
            else {
                for (size_t i = 0 ; i < client.server_client_obj.get_routes()[client.server_client_obj.is_location_url].get_index().size() ; i++){
                    str = client.get_request().get_path() + "/" + client.server_client_obj.get_routes()[client.server_client_obj.is_location_url].get_index()[i];
                    if (stat(str.c_str(), &default_file) == 0)
                        break ;
                }
            }
        }

        else if (client.server_client_obj.is_location_url == -1 && client.server_client_obj.get_index().size()) { 
            for (size_t i = 0 ; i < client.server_client_obj.get_index().size() ; i++){
                str = client.get_request().get_path() + "/" + client.server_client_obj.get_index()[i];
                if (stat(str.c_str(), &default_file) == 0)
                    break ;
            }
        }
        
        std::cout << str << std::endl;
        
        
        if (stat(str.c_str(), &default_file) == -1)
        {
            flag = 0;
            check_auto_index(client , &flag);
            if (flag == 1)
            return;
        }
        
        if (flag == 2 && client.server_client_obj.is_location_url == -1){
            set_response_error(&client , 403);
            return ;
        }
        else if (client.server_client_obj.is_location_url  > -1 && !client.server_client_obj.get_routes()[client.server_client_obj.is_location_url].get_autoindex()){
            if (flag == 2){
                set_response_error(&client , 404);
                return ;
            }
            res = fill_response(client.get_response().get_fileStream(), str, client , 200);
            client.get_response().set_response(res);
            return ;
        }
        else if (flag == 0){
            res = fill_response(client.get_response().get_fileStream(), str, client , 200);
            client.get_response().set_response(res);
            return ;
        }

        struct dirent *entry;
        client.get_response().set_response_status(200);
        res = "HTTP/1.1 200 OK\r\n";
        res += "Content-Type: text/html; charset=UTF-8\r\n";
        res += "Connection: close\r\n";
        res += "\r\n";

        res += "<html>\n<head>\n<title>Found Files</title>\n";
        res += "<style>\n";
        res += "  body { font-family: Arial, sans-serif; background-color: #f4f4f4; color: #333; }\n";
        res += "  h1 { color: #4CAF50; }\n";
        res += "  ul { list-style-type: none; padding: 0; }\n";
        res += "  li { background-color: #fff; border: 1px solid #ddd; margin: 5px 0; padding: 10px; border-radius: 5px; }\n";
        res += "  a { text-decoration: none; color: #007BFF; font-size: 18px; }\n";
        res += "  a:hover { color: #0056b3; }\n";
        res += "  .container { max-width: 800px; margin: 0 auto; padding: 20px; background-color: #fff; border-radius: 8px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }\n";
        res += "</style>\n";
        res += "</head>\n<body>\n";
        res += "<div class='container'>\n";
        res += "<h1>Found Files in Directory</h1>\n<ul>\n";



        while ((entry = readdir(dir)) != NULL)
        {
            std::string fileName = entry->d_name;
            if (client.server_client_obj.is_location_url > -1){
                res += "<li><a href=\"" + client.get_request().get_path().substr(client.server_client_obj.get_routes()[client.server_client_obj.is_location_url].get_root().size() ) + "/" + fileName + "\">" + fileName + "</a></li>\n";
            }
            else{
                res += "<li><a href=\"" + client.get_request().get_path().substr(client.server_client_obj.get_server_root().size()) + "/" + fileName + "\">" + fileName + "</a></li>\n";
            }
        }

        res += "</ul>\n</div>\n</body>\n</html>\n";
        client.get_response().set_response(res);
        closedir(dir);
    }

    else if (S_ISREG(path_stat.st_mode))
    {
        res = fill_response(client.get_response().get_fileStream(), pat, client , 200);
        client.get_response().set_response(res);
    }
    else
    {
        set_response_error(&client , 404);
        return ;
    }
    client.get_response().set_response(res);
}