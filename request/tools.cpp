#include "../webserver.hpp"
std::string root = "www";

void get_error_res(std::string &res, int status , Client &client)
{
    if (status == 400)
    {
        client.get_response().set_response_status(400);
        res = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n\
        <html><head><title>400 Bad Request</title></head><body><center><h1>400 Bad Request</h1></center>\
        <hr><center>42 webserv 0.1</center></body></html>";
    }
    else if (status == 405)
    {
        client.get_response().set_response_status(405);
        res = "HTTP/1.1 405 Not Allowed\r\nContent-Type: text/html\r\n\r\n\
        <html><head><title>405 Not Allowed</title></head><body><center><h1>405 Not Allowed</h1></center>\
        <hr><center>42 webserv 0.1</center></body></html>";
    }
    else if (status == 505)
    {
        client.get_response().set_response_status(505);
        res = "HTTP/1.1 505 HTTP Version Not Supported\r\nContent-Type: text/html\r\n\r\n\
        <html><head><title>505 HTTP Version Not Supported</title></head><body><center><h1>505 HTTP Version Not Supported</h1></center>\
        <hr><center>42 webserv 0.1</center></body></html>";
    }
}

// void get_default_index(std::string &res, std::string path , Client & client)
// {
//     std::ifstream f;
//     if (path == "/"){
//         std::string tmp = root + "/index.html";
//         f.open(tmp.c_str());
//     }
//     else
//         f.open(path.c_str());
//     std::string line;
//     client.get_response().set_response_status(200);
//     res = "HTTP/1.1 200 ok\r\nContent-Type: text/html\r\n\r\n";
//     while (std::getline(f, line))
//         res += line;
// }

std::string getContentType(std::string filePath) {
    std::map<std::string, std::string> mimeTypes;
    mimeTypes.insert(std::make_pair(".html", "text/html"));
    mimeTypes.insert(std::make_pair(".css", "text/css"));
    mimeTypes.insert(std::make_pair(".js", "application/javascript"));
    mimeTypes.insert(std::make_pair(".json", "application/json"));
    mimeTypes.insert(std::make_pair(".jpg", "image/jpeg"));
    mimeTypes.insert(std::make_pair(".jpeg", "image/jpeg"));
    mimeTypes.insert(std::make_pair(".png", "image/png"));
    mimeTypes.insert(std::make_pair(".gif", "image/gif"));
    mimeTypes.insert(std::make_pair(".mp4", "video/mp4"));
    mimeTypes.insert(std::make_pair(".pdf", "application/pdf"));
    mimeTypes.insert(std::make_pair(".txt", "text/plain"));
    mimeTypes.insert(std::make_pair(".zip", "application/zip"));
    mimeTypes.insert(std::make_pair(".mp3", "audio/mpeg"));
    
    size_t dotPos = filePath.find_last_of(".");
    if (dotPos != std::string::npos) {
        std::string ext = filePath.substr(dotPos);
        for (size_t i = 0; i < ext.length(); i++) {
            ext[i] = tolower(ext[i]);
        }

        if (mimeTypes.count(ext)) {
            return mimeTypes[ext];
        }
    }
    return "";
}


std::string fill_response(std::ifstream& fileStream,  std::string& filePath , Client &client) {
    fileStream.open(filePath.c_str(), std::ios::binary | std::ios::ate);
    
    if (!fileStream) {
        client.get_response().set_response_status(404);
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    }

    std::streampos fileSize = fileStream.tellg();
    fileStream.seekg(0, std::ios::beg);
    client.get_response().set_response_status(200);
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type:" + getContentType(filePath)  + "\r\n";
    response << "Content-Length: " << fileSize << "\r\n";
    response << "Accept-Ranges: bytes\r\n";
    response << "Connection: close\r\n";
    response << "X-Content-Type-Options: nosniff\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "\r\n";
    
    return response.str();
}