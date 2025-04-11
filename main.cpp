#include "webserver.hpp"

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

std::string generate_file_names(const std::string &extension)
{
    static int index;
    std::string name;
    while (1)
    {
        std::ostringstream ss;

        ss << "file" << index << "." << extension;
        name = ss.str();
        if (access((root + "/" + name).c_str(), F_OK) != 0)
            return name;
        index++;
    }
    return NULL;
}

void chunked(Client &client)
{
    static std::string request;
    request += client.get_request().get_s_request();
    static std::string result;
    std::string line;
    while (true)
    {
        size_t pos = request.find("\r\n");

        if (pos == std::string::npos)
        {
            return;
        }

        line = request.substr(0, pos + 2);
        size_t size = hex_to_int(line);
        if (size == 0)
        {
            client.set_all_recv(true);
            client.get_request().set_s_request(result);
            request = result = "";
            hanlde_post_request(client);
            return;
        }
        std::string tmp = request.substr(pos + 2);
        if (tmp.size() < size)
        {
            return;
        }
        request = request.substr(pos + 2);
        result += request.substr(0, size);

        request = request.substr(size + 2);
    }
}

void fill_data_boudary(const std::string &tmp, Client &clinet , size_t index)
{
    std::istringstream ss(tmp);
    std::string line;
    std::ofstream file;

    std::getline(ss, line);
    if (index != 0)
        std::getline(ss, line);

    std::cout << line << std::endl;
    // return ;

    std::string key;
    if (line.find("Content-Disposition:") != std::string::npos)
    {
        size_t name_pos = line.find("name=\"");
        if (name_pos != std::string::npos)
        {
            size_t name_start = name_pos + 6;
            size_t name_end = line.find("\"", name_start);
            if (name_end != std::string::npos)
                key = line.substr(name_start, name_end - name_start);
            else
            {
                std::cerr << "Invalid boundary format 1" << std::endl;
                return;
            }
            size_t filename_pos = line.find("filename=\"");
            if (filename_pos != std::string::npos)
            {
                filename_pos += 10;
                size_t file_name_end = line.find("\"", filename_pos);
                if (file_name_end == std::string::npos)
                {
                    std::cout << "error" << std::endl;
                    exit(55);
                }
                std::string filename = line.substr(filename_pos, file_name_end - filename_pos);
                filename  = root + "/" + filename;
                file.open(filename.c_str());
                std::getline(ss, line);
                std::getline(ss, line);

                while (1)
                {
                    char c;
                    line = "";
                    while (ss.get(c))
                        line += c;
                    if (line.empty())
                        break;
                    file << line << std::flush;
                }
            }
            std::getline(ss, line);
            while (1)
            {
                char c;
                line = "";
                while (ss.get(c))
                {
                    line += c;
                }
                if (line.empty())
                    break;
                clinet.fill_map(key, line);
            }
        }
        else
        {
            std::cerr << "Invalid boundary format 3" << std::endl;
            return;
        }
    }
}


int check_if_have_new_boundary(std::string &buffer, const std::string &boundary, Client &client, size_t size)
{
    std::string boundaryWithPrefix = "--" + boundary;

    if (size >= buffer.size()) {
        return -1;
    }
    
    std::string tmp = buffer.substr(size);
    size_t pos = tmp.find(boundaryWithPrefix);
    if (pos == std::string::npos)
        return -1;
        
    size_t last_Boundary = size + pos + boundaryWithPrefix.size();
    if (last_Boundary + 2 <= buffer.size() &&
        buffer[last_Boundary] == '-' && buffer[last_Boundary + 1] == '-')
        client.set_all_recv(true);
        
    return static_cast<int>(pos + size);
}

void boundary(Client &client)
{
    static std::string buffer;
    static int i = 0;
    static std::string boundary;
    std::string tmp;
    static size_t size;
    static int flag ;
    
    buffer += client.get_request().get_s_request();
    
    if (i == 0)
    {
        
        std::istringstream ss(buffer);
        std::getline(ss, tmp);
        
        size_t pos = tmp.find_first_not_of("-");
        if (pos == std::string::npos) {
            std::cerr << "Invalid boundary format" << std::endl;
            return;
        }
        
        size_t end = tmp.find("\r");
        if (end == std::string::npos) {
            end = tmp.length();
        }
        
        boundary = tmp.substr(pos, end - pos);
        pos = buffer.find("\n");
        
        if (pos != std::string::npos) {
            buffer = buffer.substr(pos + 1);
        }
    }
    
    i++;
    
    while (true)
    {
        int index = check_if_have_new_boundary(buffer, boundary, client, size);
        if (index == -1)
            break;
        if (index == 0)
            std::cout << "here" << std::endl;
        else
        {
            tmp = buffer.substr(0, index - 2);
            if ((size_t)index < buffer.size()) {
                buffer = buffer.substr(index);
            } else {
                buffer.clear();
            }
            fill_data_boudary(tmp, client , flag);
            flag = 1;
        }
    }
    
    size = buffer.size();
    if (client.get_all_recv()) {
        buffer.clear();
        boundary.clear();
        i = flag = 0;
        size = 0;
    }
}




void handle_boundary_chanked(Client &client)
{
    static std::string request;
    request += client.get_request().get_s_request();
    static std::string result;
    std::string line;
    while (true)
    {
        size_t pos = request.find("\r\n");

        if (pos == std::string::npos)
        {
            return;
        }

        line = request.substr(0, pos + 2);
        size_t size = hex_to_int(line);
        if (size == 0)
        {
            client.set_all_recv(true);
            client.get_request().set_s_request(result);
            request = result = "";
            boundary(client);
            return;
        }
        std::string tmp = request.substr(pos + 2);
        if (tmp.size() < size)
        {
            return;
        }
        request = request.substr(pos + 2);
        result += request.substr(0, size);

        request = request.substr(size + 2);
    }
}

int main(int ac, char **av)
{

    (void)av;
    Server S1;
    if (ac == 2)
    {
        std::ifstream infile(av[1]);
        if (!infile.is_open())
        {
            std::cerr << "Error opening file!" << std::endl;
            return (1);
        }
        std::string part;
        std::vector<std::string> parts;
        // std::vector<std::string>::iterator it = parts.begin();

        while(std::getline(infile, part, ';'))
            parts.push_back(part);
        if (parts.empty())
            std::cerr << "Error empty file!" << std::endl;
        // for (size_t i = 0; i < parts.size(); ++i) {
        //     std::cout << "part " << i + 1 << ": " << parts[i] << std::endl;
        // }
        size_t i = 0;
	    while (parts.size() > i)
	    {
			// std::cout << "before replace: " << parts[i] << std::endl;
			std::replace(parts[i].begin() , parts[i].end() , '\n' , ' ');
			// std::replace(parts[i].begin(), parts[i].end(), '\r', ' ');
			// std::cout << "fter replace: " << parts[i] << std::endl;
			i++;
	    }
        // switch_parts(parts);
        // exit(1);
        Confile conf;

        conf.set_server(parts);
        if (conf.status == false)
            return (std::cout << "Fixe config file and try again!" << std::endl, 1);
        std::vector <ServerBlock> servers = conf.get_server();
        int port = servers[0].get_port();
        std::cout << "port: " << port << std::endl;
        for (size_t i = 0; i < conf.number_of_server ; i++)
        {
            S1.addServerConfig(servers[i].get_host(), servers[i].get_host(), servers[i].get_port());
        }
    }
    // return (0);
    // try
    // {
     
        
        S1.startServer();
    // }
    // catch (const std::exception &e)
    // {
    //     std::cerr << e.what() << '\n';
    // }
}