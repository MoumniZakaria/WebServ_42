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

std::string ft_generate_file_names( Client &client ,const std::string& extension)
{
    static int index;
    std::string name;
    while (1)
    {
        std::ostringstream ss;

        ss << "file" << index << "." << extension;
        name = ss.str();
        if (access((client.server_client_obj.get_server_root() + "/" + name).c_str(), F_OK) != 0)
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
                filename  =  clinet.server_client_obj.get_server_root() + "/" + filename;
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
                        break ;
                    file << line << std::flush;
                }
            }
            else{
                set_response_error(&clinet ,  415);
                return ;
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
            std::cerr << "Invalid boundary format " << std::endl;
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
        buffer[last_Boundary] == '-' && buffer[last_Boundary + 1] == '-'){
        client.set_all_recv(true);
        std::cout << "my job done here " << std::endl;
        // exit(15);
    }

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
        if (index == -1){
            break;
        }
        if (index == 0){
            /////////////////////////////////////////
            buffer = buffer.substr(boundary.size());
        }
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
    static std::string backup;
    std::string request = backup + client.get_request().get_s_request();
    backup.clear();
    
    while (true)
    {
       
        size_t pos = request.find("\r\n");
        if (pos == std::string::npos)
        {
           
            backup = request;
            return;
        }
        
        
        std::string line_size = request.substr(0, pos);
        size_t size = hex_to_int(line_size);
        
        if (size == 0)
        {
            client.set_all_recv(true);
            client.get_request().set_s_request(request);
            backup.clear();
            boundary(client);
            return;
        }
        
       
        if (request.size() < pos + 2 + size + 2)  
        {
            backup = request;
            return;
        }
        
        std::string chunk = request.substr(pos + 2, size);
        
        client.get_request().set_s_request(chunk);
        boundary(client);
        
        request = request.substr(pos + 2 + size + 2); 
        
        if (request.empty())
        {
            return;
        }
    }
}


int get_parts(char **av, std::vector <std::string>& parts)
{
    std::ifstream infile(av[1]);
    if (!infile.is_open())
    {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }
    std::string line;
    std::vector<std::string> lines;
    while(std::getline(infile, line, '\n'))
        lines.push_back(line);
    size_t x = lines.size() - 1;
    while (x > 0)
    {
        if (!lines[x].empty())
            break;
        x--;
    }
    std::string newlastline = trimstr(lines[x]);
    if (newlastline.find(';') != std::string::npos)
        return ( 1);
    infile.clear();
    infile.seekg(0, std::ios::beg);

    std::string part;
    while(std::getline(infile, part, ';'))
        parts.push_back(part);
    if (parts.empty())
        std::cerr << "Error empty file!" << std::endl;
    for (size_t y = 0; y < parts.size(); y++)
    {
        if (parts[y].empty() || !check_empt(parts[y].c_str()))
            return (std::cout << "Error Double ';' in configuration file" << std::endl, 1);
    }
    size_t i = 0;
    while (parts.size() > i)
    {
        std::replace(parts[i].begin() , parts[i].end() , '\n' , ' ');
        i++;
    }
    infile.close();
    return 0;
}


// void print_content(Confile Configuration)
// {
//     std::vector <ServerBlock> servers = Configuration.get_server();
//     std::cout << "##### server 1 #####" << std::endl;
//     std::cout << "port: " << servers[0].get_port() << std::endl;
//     std::cout << "host: " << servers[0].get_host() << std::endl;
//     std::cout << "server root: " << servers[0].get_server_root() << std::endl;
//     std::vector<std::string> indx = servers[0].get_index();
//     std::cout << "index size: " << indx.size() << std::endl;
//     for (size_t x = 0; x < indx.size() ; x++)
//     {
//         std::cout << "index " << x << ": "<< indx[x] << std::endl;
//     } 
//     std::cout << "clientbodymax: " << servers[0].get_client_body_size() << std::endl;
//     std::map <int, std::string> error_pages = servers[0].get_error_pages();
//     for (std::map<int, std::string>::iterator it = error_pages.begin(); it != error_pages.end(); ++it) {
//         std::cout << "Error code: " << it->first << " => Page: " << it->second << std::endl;
//     }
//     //location block
//     std::vector<RouteBlock>        routes = servers[0].get_routes();
//     std::cout << "size of routes: "  << routes.size() << std::endl;
//     std::cout << "***route 1 of server 1: " << std::endl;
//     std::cout << "uri: " << routes[0].get_uri() << std::endl; 
//     std::cout << "location's root: " << routes[0].get_root() << std::endl;
//     std::vector<std::string> methods = routes[0].get_methods();
//     for (size_t i = 0; i < methods.size(); ++i) {
//         std::cout << "Method: " << methods[i] << std::endl;
//     }
//     std::cout << "autoindex: " << routes[0].get_autoindex() << std::endl; 
//     std::map<int , std::string> redirection = routes[0].get_redirections();
//     if (redirection.empty())
//     std::cout << "there is no redirection" << std::endl;
//     for (std::map<int, std::string>::iterator it2 = redirection.begin(); it2 != redirection.end(); ++it2) {
//         std::cout << "status code: " << it2->first << " => target URL: " << it2->second << std::endl;
//     }
//     std::cout << "***route 2 of server 1: " << std::endl;
//     std::cout << "uri: " << routes[1].get_uri() << std::endl; 
//     std::cout << "location's root: " << routes[1].get_root() << std::endl;
//     std::vector<std::string> methods2 = routes[1].get_methods();
//     for (size_t i = 0; i < methods2.size(); ++i) {
//         std::cout << "Method: " << methods2[i] << std::endl;
//     }
//     std::cout << "autoindex: " << routes[1].get_autoindex() << std::endl; 
//     std::map<int , std::string> redirection2 = routes[1].get_redirections();
//     if (redirection.empty())
//     std::cout << "there is no redirection" << std::endl;
//     for (std::map<int, std::string>::iterator it3 = redirection2.begin(); it3 != redirection2.end(); ++it3) {
//         std::cout << "status code: " << it3->first << " => target URL: " << it3->second << std::endl;
//     }
// }



int main(int ac, char **av)
{

    (void)av;
    Server S1;
    if (ac == 2)
    {

        std::vector <std::string> parts;
        if (get_parts(av, parts))
             return 1;
        Confile conf;

        conf.set_server(parts);
        if (conf.status == false)
            return (1);
        if (conf.status == false)
            return (std::cout << "Fixe config file and try again!" << std::endl, 1);
        S1.number_of_servers = conf.number_of_server;
        std::vector <ServerBlock> servers = conf.get_server();
       
        S1.server_block_obj = servers;
        for (size_t i = 0; i < conf.number_of_server ; i++)
        {
            // S1.addServerConfig(servers[i].get_host(), servers[i].get_port() , servers[i].get_server_names());
            S1.addServerConfig(i, servers[i].get_host(), servers[i].get_port(), servers[i].get_server_names());
        }
        // size of server_configs size:
        std::cout << "server_configs size: " << S1.getServerCount() << std::endl;
    }
    // else
    // {
    //     std::vector <ServerBlock> servers;
    //     ServerBlock default_server;
    //     default_server.set_dafault_data();
    
    //     servers.push_back(default_server);
    //     if(set_default_page())
    //         return 1;
    //     S1.number_of_servers = 1;
    //     S1.server_block_obj = servers;
    //     S1.addServerConfig(servers[0].get_host(), servers[0].get_port(), servers[0].get_server_names());
    //     // std::cout << "Usage: ./webserv <config_file>" << std::endl;
    // }
    S1.startServer();
    
}


// std::vector <int> portt = conf.get_server()[i].get_port();
// std::cout << "portt size: " << portt.size() << std::endl;
// for (size_t j = 0; j < portt.size(); j++)
// {
//     std::cout << "portt[" << j << "] = " << portt[j] << std::endl;
// }
// S1.addServerConfig(servers[i].get_host(), servers[i].get_port(), servers[i].get_server_names());