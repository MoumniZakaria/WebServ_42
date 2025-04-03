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

void response_to_get(Client &client)
{
    std::cout << "\033[34m" << "GET request ====> " << client.get_request().get_method() << " " << client.get_request().get_path() << " " << "\033[0m" << std::endl;
    std::string res = client.get_response().get_response();

    if (client.get_request().get_path() == "/")
    {
        std::string p = "/index.html";
        client.get_request().set_path(p);
    }

    std::string pat = root + "/" + client.get_request().get_path().substr(1);
    client.get_request().set_path(pat);

    struct stat path_stat;

    if (stat(client.get_request().get_path().c_str(), &path_stat) == -1)
    {
        std::string error_page_path = "www/errors/404.html";
        res = fill_response(client.get_response().get_fileStream(), error_page_path, client);
        client.get_response().set_response_status(404);
        client.get_response().set_response(res);

        return;
    }

    else if (S_ISDIR(path_stat.st_mode))
    {
        if (access(pat.c_str(), R_OK | W_OK | X_OK) == -1)
        {
            std::string error_page_path = "www/errors/404.html";
            res = fill_response(client.get_response().get_fileStream(), error_page_path, client);
            client.get_response().set_response_status(403);
            client.get_response().set_response(res);
            return;
        }
        DIR *dir = opendir(pat.c_str());

        if (dir == NULL)
        {
            std::string error_page_path = "www/errors/404.html";
            res = fill_response(client.get_response().get_fileStream(), error_page_path, client);
            client.get_response().set_response_status(404);
            client.get_response().set_response(res);
            return;
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
            res += "<li><a href=\"/" + pat.substr(4) + "/" + fileName + "\">" + fileName + "</a></li>\n";
            // 4 must be changed to adjust for the root directory size
        }

        res += "</ul>\n</div>\n</body>\n</html>\n";
        client.get_response().set_response(res);
        closedir(dir);
    }

    else if (S_ISREG(path_stat.st_mode))
    {
        res = fill_response(client.get_response().get_fileStream(), pat, client);
        client.get_response().set_response(res);
    }
    else
    {
        client.get_response().set_response_status(404);
        res = "HTTP/1.1 404 not found\r\nContent-Type: text/html\r\n\r\n\
            <html><head><title>404 not found</title></head><body><center><h1>404 not found</h1></center>\
            <hr><center>42 webserv 0.1</center></body></html>";
    }
    client.get_response().set_response(res);
}