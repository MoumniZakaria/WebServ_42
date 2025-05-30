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
        if (access(pat.c_str(), R_OK ) == -1)
        {
            set_response_error(&client , 403);
            return;
        }
        DIR *dir = opendir(pat.c_str());
        
        if (dir == NULL)
        {
            set_response_error(&client , 404);
            closedir(dir);
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
        
        if (stat(str.c_str(), &default_file) == -1)
        {
            flag = 0;
            check_auto_index(client , &flag);
            if (flag == 1){
                closedir(dir);
                return;
            }
        }
        
        if (flag == 2 && client.server_client_obj.is_location_url == -1){
            set_response_error(&client , 403);
            closedir(dir);
            return ;
        }
        else if (client.server_client_obj.is_location_url  > -1 && !client.server_client_obj.get_routes()[client.server_client_obj.is_location_url].get_autoindex()){
            if (flag == 2){
                set_response_error(&client , 404);
                closedir(dir);
                return ;
            }
            res = fill_response(client.get_response().get_fileStream(), str, client , 200);
            client.get_response().set_response(res);
            closedir(dir);
            return ;
        }
        else if (flag == 0){
            res = fill_response(client.get_response().get_fileStream(), str, client , 200);
            client.get_response().set_response(res);
            closedir(dir);
            return ;
        }

        struct dirent *entry;
        std::string found_files;
        client.get_response().set_response_status(200);
        
        found_files = "<html>\n<head>\n<title>Found Files</title>\n";
        found_files += "<style>\n";
        found_files += "  body { font-family: Arial, sans-serif; background-color: #f4f4f4; color: #333; }\n";
        found_files += "  h1 { color: #4CAF50; }\n";
        found_files += "  ul { list-style-type: none; padding: 0; }\n";
        found_files += "  li { background-color: #fff; border: 1px solid #ddd; margin: 5px 0; padding: 10px; border-radius: 5px; }\n";
        found_files += "  a { text-decoration: none; color: #007BFF; font-size: 18px; }\n";
        found_files += "  a:hover { color: #0056b3; }\n";
        found_files += "  .container { max-width: 800px; margin: 0 auto; padding: 20px; background-color: #fff; border-radius: 8px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }\n";
        found_files += "</style>\n";
        found_files += "</head>\n<body>\n";
        found_files += "<div class='container'>\n";
        found_files += "<h1>Found Files in Directory</h1>\n<ul>\n";
        

        
        while ((entry = readdir(dir)) != NULL)
        {
            std::string fileName = entry->d_name;
            if (client.server_client_obj.is_location_url > -1){
                found_files += "<li><a href=\"" + client.get_request().get_path().substr(client.server_client_obj.get_routes()[client.server_client_obj.is_location_url].get_root().size() ) + "/" + fileName + "\">" + fileName + "</a></li>\n";
            }
            else{
                found_files += "<li><a href=\"" + client.get_request().get_path().substr(client.server_client_obj.get_server_root().size()) + "/" + fileName + "\">" + fileName + "</a></li>\n";
            }
        }
        
        std::ostringstream oss;
        oss << found_files.size();
        std::string content_length = oss.str();



        found_files += "</ul>\n</div>\n</body>\n</html>\n";
        res = "HTTP/1.1 200 OK\r\n";
        res += "Content-Length: ";
        res += content_length;
        res += "\r\n";
        res += "Content-Type: text/html; charset=UTF-8\r\n";
        if (!client.get_Alive())
            res += "Connection: close\r\n";
        else
            res += "Connection: keep-alive\r\n";
        res += "\r\n";
        res += found_files;

        client.get_response().set_response(res);
        closedir(dir);
    }


    else if (S_ISREG(path_stat.st_mode))
    {
        struct stat fileStat;

        if (stat(pat.c_str(), &fileStat) < 0){
            set_response_error(&client ,404);
            return ;
        }
        
        if (!(fileStat.st_mode & S_IRUSR)){
            set_response_error(&client ,403);
            return ;
        }
                
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




std::string get_file_name(Client *client){

    if (client->server_client_obj.is_location_url != -1){

        std::string dir = client->server_client_obj.get_routes()[client->server_client_obj.is_location_url].get_client_body_temp_path();

        struct stat statbuf;

        if (stat(dir.c_str(), &statbuf) == -1) {
            set_response_error(client , 502);
            return "";
        }
        
        if (!S_ISDIR(statbuf.st_mode)) {
            set_response_error(client , 502);
            return "";
        }
        
        if ((statbuf.st_mode & S_IWUSR) == 0) { 
            set_response_error(client , 502);
            return "";
        }

        return dir;
    }
    else{

        std::string dir = "./upload";

        struct stat statbuf;

        if (stat(dir.c_str(), &statbuf) == -1) {
            set_response_error(client , 502);
            return "";
        }
        
        if (!S_ISDIR(statbuf.st_mode)) {
            set_response_error(client , 502);
            return "";
        }
        
        if ((statbuf.st_mode & S_IWUSR) == 0) { 
            set_response_error(client , 502);
            return "";
        }
        return dir ;
        
    }
    return "";
}