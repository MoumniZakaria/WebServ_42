#include "../webserver.hpp"

std::string chunked_for_cgi(Client *client)
{
    static std::string request;
    request += client->get_request().get_s_request();
    static std::string result;
    std::string line;

    while (true)
    {
        size_t pos = request.find("\r\n");

        if (pos == std::string::npos)
            return "";

        line = request.substr(0, pos + 2);
        size_t size = hex_to_int(line);
        if (size == 0)
        {
            client->set_all_recv(true);
            client->get_request().set_s_request(result);
            std::string new_result = result;
            request = result = "";
            return new_result;
        }
        std::string tmp = request.substr(pos + 2);
        if (tmp.size() < size)
        {
            return "";
        }
        request = request.substr(pos + 2);
        result += request.substr(0, size);
        request = request.substr(size + 2);
    }
}

// void handle_x_www_form_urlencoded(Client &client)
// {
//     std::string tmp = client.get_request().get_s_request();
//     std::istringstream ss(tmp);
//     std::string line;
//     std::string key;
//     std::string value;

//     client.set_all_recv(true);
//     while (std::getline(ss, line, '&'))
//     {
//         size_t pos = line.find("=");
//         if (pos == std::string::npos)
//         {
//             std::cout << "error" << std::endl;
//             exit(21);
//         }
//         key = line.substr(0, pos);
//         value = line.substr(pos + 1);
//     }
// }

int hex_to_int(const std::string &hexStr)
{
    int result = 0;
    for (size_t i = 0; i < hexStr.length(); ++i)
    {
        char c = hexStr[i];
        if (c >= '0' && c <= '9')
            result = result * 16 + (c - '0');
        else if (c >= 'a' && c <= 'f')
            result = result * 16 + (c - 'a' + 10);
        else if (c >= 'A' && c <= 'F')
            result = result * 16 + (c - 'A' + 10);
    }
    return result;
}

std::string ft_generate_file_names(const std::string &extension, std::string dirname)
{
    static int index;
    std::string name;
    while (1)
    {
        std::ostringstream ss;

        ss << "file" << index << "." << extension;
        name = ss.str();
        name = dirname + "/" + name;
        if (access(name.c_str(), F_OK) != 0)
            return name;
        index++;
    }
    return NULL;
}


void chunked(Client &client)
{
    static std::string request;
    request += client.get_request().get_s_request();
    std::string line;

    while (true)
    {
        size_t pos = request.find("\r\n");
        if (pos == std::string::npos)
            return;

        line = request.substr(0, pos + 2);
        size_t size = hex_to_int(line);

        std::string tmp = request.substr(pos + 2);
        if (tmp.size() < size + 2)
            return;

        if (size == 0)
        {
            client.set_all_recv(true);
            request = "";
            return;
        }

        request = request.substr(pos + 2);

        std::string chunk_data = request.substr(0, size);

        client.get_request().set_s_request(chunk_data);
        hanlde_post_request(client);

        request = request.substr(size + 2);
    }
}

void trim_non_printable(std::string &str)
{
    size_t start = 0;
    while (start < str.length() && !std::isprint(str[start]))
        ++start;

    size_t end = str.length();
    while (end > start && !std::isprint(str[end - 1]))
        --end;
    str = str.substr(start, end - start);
}


void create_file(std::string &buffer, Client &client, int flag)
{
    static std::ofstream file;
    static bool file_created = false;
    

    if (flag == 2)
    {
        if (file.is_open()) {
            file.close();
        }
        file_created = false;
        return;
    }
    
    if (flag == 1)
    {
        if (file.is_open()) {
            file.write(buffer.c_str(), buffer.size());
            file.flush();
        }
        return;
    }
    
    
    std::istringstream ss(buffer);
    std::string line;
    std::getline(ss, line);
    
    if (line.find("Content-Disposition:") != std::string::npos)
    {
        size_t filename_pos = line.find("filename=\"");

        if (filename_pos != std::string::npos)
        {
            filename_pos += 10;
            size_t file_name_end = line.find("\"", filename_pos);
            if (file_name_end == std::string::npos)
            {
                set_response_error(&client, 400);
                return;
            }
            
            std::string dirname = get_file_name(&client);
            std::string filename = dirname + "/" + line.substr(filename_pos, file_name_end - filename_pos);
            
            if (filename.empty() || dirname.empty())
            {
                client.set_all_recv(true);
                return;
            }
            
            if (file.is_open()) {
                file.close();
            }
            
            file.open(filename.c_str(), std::ios::out | std::ios::binary);
            if (!file.is_open()) {
                set_response_error(&client, 500);
                return;
            }
            file_created = true;
            
            while (std::getline(ss, line)) {
                if (line.empty() || line == "\r") {
                    break;
                }
            }
            
            std::string remaining_content;
            std::string temp_line;
            bool first_line = true;
            while (std::getline(ss, temp_line)) {
                if (!first_line) {
                    remaining_content += "\n";
                }
                remaining_content += temp_line;
                first_line = false;
            }
            
            if (!remaining_content.empty()) {
                file.write(remaining_content.c_str(), remaining_content.size());
                file.flush();
            }
        }
        else
            set_response_error(&client, 415);
    }
}

int check_if_have_new_boundary(const std::string &buffer, const std::string &boundary, Client &client, size_t start_pos)
{
    std::string boundaryWithPrefix = "--" + boundary;
    
    if (start_pos >= buffer.size())
    {
        return -1;
    }
    
    size_t pos = buffer.find(boundaryWithPrefix, start_pos);
    
    if (pos == std::string::npos)
        return -1;
    
    size_t boundary_end = pos + boundaryWithPrefix.size();
    
    if (boundary_end + 1 < buffer.size() &&
        buffer[boundary_end] == '-' && buffer[boundary_end + 1] == '-')
        client.set_all_recv(true);
    
    return static_cast<int>(pos);
}

void boundary(Client &client)
{
    static std::string buffer;
    static int call_count = 0;
    static std::string boundary_str;
    static bool processing_file = false;
    
    buffer += client.get_request().get_s_request();
    
    if (call_count == 0)
    {
        size_t first_line_end = buffer.find('\n');
        if (first_line_end == std::string::npos) {
            return;
        }
        
        std::string first_line = buffer.substr(0, first_line_end);
        
        if (!first_line.empty() && first_line[first_line.length() - 1] == '\r') {
            first_line = first_line.substr(0, first_line.length() - 1);
        }
        
        if (first_line.length() > 2 && first_line.substr(0, 2) == "--") {
            boundary_str = first_line.substr(2);
        } else {
            return;
        }
        
        buffer = buffer.substr(first_line_end + 1);
    }
    call_count++;
    
    while (true)
    {
        int boundary_pos = check_if_have_new_boundary(buffer, boundary_str, client, 0);
        
        if (boundary_pos == -1)
        {
            if (!processing_file) {
                create_file(buffer, client, 0);
                processing_file = true;
            } else {
                create_file(buffer, client, 1);
            }
            buffer.clear();
            break;
        }
        
        if (boundary_pos == 0)
        {
            if (processing_file) {
                create_file(buffer, client, 2);
                processing_file = false;
            }
            
            std::string boundaryWithPrefix = "--" + boundary_str;
            size_t skip_size = boundaryWithPrefix.size();
            
            if (skip_size < buffer.size() && buffer[skip_size] == '\r') skip_size++;
            if (skip_size < buffer.size() && buffer[skip_size] == '\n') skip_size++;
            
            buffer = buffer.substr(skip_size);
            continue;
        }
        else
        {
            std::string data_chunk = buffer.substr(0, boundary_pos);
            
            if (data_chunk.length() >= 2 && 
                data_chunk.substr(data_chunk.length() - 2) == "\r\n") {
                data_chunk = data_chunk.substr(0, data_chunk.length() - 2);
            }
            
            if (!processing_file) {
                create_file(data_chunk, client, 0);
                processing_file = true;
            } else {
                create_file(data_chunk, client, 1);
            }
            create_file(data_chunk, client, 2);
            processing_file = false;
            
            buffer = buffer.substr(boundary_pos);
        }
    }
    
    if (client.get_all_recv())
    {
        if (processing_file) {
            create_file(buffer, client, 2);
        }
        buffer.clear();
        boundary_str.clear();
        call_count = 0;
        processing_file = false;
    }
}

