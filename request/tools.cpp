#include "../webserver.hpp"


void  trim(std::string& str) {
    size_t start = 0;
    size_t end = str.size();

    while (start < end && std::isspace(str[start])) {
        ++start;
    }

    while (end > start && std::isspace(str[end - 1])) {
        --end;
    }

    str = str.substr(start, end - start);
}

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


std::string fill_response(std::ifstream& fileStream,  std::string& filePath , Client &client , int status ) {

    fileStream.close();
    fileStream.open(filePath.c_str(), std::ios::ate);

    if (!fileStream.is_open()) {
        std::cout << "cat't open " << filePath << std::endl; 
        return "";
    }

    std::streampos fileSize = fileStream.tellg();
    fileStream.seekg(0, std::ios::beg);
    client.get_response().set_response_status(200);
    std::ostringstream response;
    response << "HTTP/1.1 ";
    response <<  status ;
    response << " OK\r\n";
    response << "Content-Type:" + getContentType(filePath)  + "\r\n";
    response << "Content-Length: " << fileSize << "\r\n";
    response << "Accept-Ranges: bytes\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";


    return response.str();
}


