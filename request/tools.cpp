#include "../webserver.hpp"

int check_uri(Client *client)
{

    std::string path = client->get_request().get_path();
    struct stat statbuf;

    if (stat(path.c_str(), &statbuf) == -1)
    {
        set_response_error(client, 404);
        return 0;
    }

    if (!S_ISDIR(statbuf.st_mode))
    {
        set_response_error(client, 405);
        return 0;
    }
    return 1;

}
std::string get_response_title(int status)
{

    if (status == 400)
        return "Bad Request";
    if (status == 404)
        return "Not Found";
    if (status == 403)
        return "Forbidden";
    if (status == 201)
        return "Created";
    if (status == 405)
        return "Method Not Allowed";
    if (status == 415)
        return "Unsupported Media Type";
    if (status == 505)
        return "HTTP Version Not Supported";
    if (status == 413)
        return "Payload Too Large";
    if (status == 502)
        return "Bad Gateway";
    if (status == 500)
        return "	Internal Server Error";
    if (status == 504)
        return "	Gateway Timeout";
    else
        return "ok";
}

void trim(std::string &str)
{
    size_t start = 0;
    size_t end = str.size();

    while (start < end && std::isspace(str[start]))
    {
        ++start;
    }

    while (end > start && std::isspace(str[end - 1]))
    {
        --end;
    }

    str = str.substr(start, end - start);
}

std::string getContentType(std::string path)
{
    size_t dot = path.find_last_of(".");
    if (dot != std::string::npos)
    {
        std::string ext = path.substr(dot);

        if (ext == ".html")
            return "text/html";
        else if (ext == ".css")
            return "text/css";
        else if (ext == ".js")
            return "application/javascript";
        else if (ext == ".json")
            return "application/json";
        else if (ext == ".jpg" || ext == ".jpeg")
            return "image/jpeg";
        else if (ext == ".png")
            return "image/png";
        else if (ext == ".gif")
            return "image/gif";
        else if (ext == ".mp4")
            return "video/mp4";
        else if (ext == ".pdf")
            return "application/pdf";
        else if (ext == ".txt")
            return "text/plain";
        else if (ext == ".zip")
            return "application/zip";
        else if (ext == ".mp3")
            return "audio/mpeg";
    }
    return "";
}

std::string fill_response(std::ifstream &fileStream, std::string &filePath, Client &client, int status)
{

    fileStream.close();
    fileStream.open(filePath.c_str(), std::ios::ate);

    if (!fileStream.is_open())
    {
        std::cout << "cat't open " << filePath << std::endl;
        return "";
    }

    std::streampos fileSize = fileStream.tellg();
    fileStream.seekg(0, std::ios::beg);
    client.get_response().set_response_status(200);
    std::ostringstream response;

    response << "HTTP/1.1 ";

    int redirection = client.get_request().redirection;

    if (redirection != -1)
    {
        if (redirection == 301)
        {
            response << "301 Moved Permanently\r\n";
            response << "Location: ";
            response << client.get_request().Location;
            response << " \r\n";
        }
        else if (redirection == 302)
        {
            response << "302 Found\r\n";
            response << "Location: ";
            response << client.get_request().Location;
            response << " \r\n";
        }
    }
    else
    {
        response << status;
        response << " " + get_response_title(status) + " \r\n";
    }

    response << "Content-Type:" + getContentType(filePath) + "\r\n";
    response << "Content-Length: " << fileSize << "\r\n";
    response << "Accept-Ranges: bytes\r\n";
    if (client.get_Alive())
        response << "Connection: keep-alive\r\n";
    else
        response << "Connection: close\r\n";
    response << "\r\n";

    return response.str();
}

bool check_if_have_cgi(Client &client)
{

    bool bol = false;
    std::string path = client.get_request().get_path();
    size_t pos = path.find_last_of('.');

    if (pos != std::string::npos)
    {
        std::string ex = path.substr(pos);
        if (ex == ".php" || ex == ".py")
        {
            if (client.server_client_obj.is_location_url > -1)
            {
                std::vector<std::string> vec = client.server_client_obj.get_routes()[client.server_client_obj.is_location_url].get_cgi_ext();
                for (size_t i = 0; i < vec.size(); i++)
                {
                    static std::string body_cgi;
                    if (vec[i] == ex)
                    {
                        bol = true;
                        if (client.get_request().get_method() == "POST")
                        {
                            std::string content_type = client.get_request().get_map_values("Content-Type");
                            std::string chunk = client.get_request().get_map_values("Transfer-Encoding");

                            if (chunk == "chunked")
                            {
                                std::string tmp = chunked_for_cgi(&client);
                                if (!tmp.empty())
                                    body_cgi += tmp;
                                if (client.get_all_recv())
                                {
                                    cgi_handler(client, body_cgi, path);
                                    body_cgi = "";
                                }
                            }
                            else
                            {
                                body_cgi += client.get_request().get_s_request();
                                if (client.get_request().get_content_length() == body_cgi.size())
                                {

                                    std::cout << "123  " << body_cgi << std::endl;
                                    std::cout << "123  " << path << std::endl;
                                    cgi_handler(client, body_cgi, path);
                                    body_cgi = "";
                                    client.set_all_recv(true);
                                }
                            }
                        }
                        else if (client.get_request().get_method() == "GET")
                        {
                            client.set_all_recv(true);
                            cgi_handler(client, "", path);
                        }
                        else
                        {
                            return (set_response_error(&client, 415), 1);
                            return bol;
                        }
                        if (client.get_Alive())
                        {
                            client.get_response().set_response_index(true);
                            client.get_response().set_response_status(200);
                        }
                    }
                }
            }
            else
            {
                bol = true;
                static std::string body_cgi;
                if (client.get_request().get_method() == "POST")
                {
                    std::string content_type = client.get_request().get_map_values("Content-Type");
                    std::string chunk = client.get_request().get_map_values("Transfer-Encoding");

                    if (chunk == "chunked")
                    {
                        std::string tmp = chunked_for_cgi(&client);
                        if (!tmp.empty())
                            body_cgi += tmp;
                        if (client.get_all_recv())
                        {
                            cgi_handler(client, body_cgi, path);
                            body_cgi = "";
                        }
                    }
                    else
                    {
                        body_cgi += client.get_request().get_s_request();
                        if (client.get_request().get_content_length() == body_cgi.size())
                        {
                            cgi_handler(client, body_cgi, path);
                            body_cgi = "";
                            client.set_all_recv(true);
                        }
                    }
                }
                else if (client.get_request().get_method() == "GET")
                {
                    cgi_handler(client, "", path);
                    client.set_all_recv(true);
                }
                else
                {
                    return (set_response_error(&client, 405), 1);
                    return bol;
                }
                if (client.get_Alive())
                {
                    client.get_response().set_response_index(true);
                    client.get_response().set_response_status(200);
                }
            }
        }
    }
    return bol;
}