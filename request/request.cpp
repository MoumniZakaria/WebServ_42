#include "../webserver.hpp"

Request::Request()
{
    index = false;
    is_string_req_send = false;

}
Request::~Request() {};

const std::string &Request::get_method()
{
    return method;
}

const std::string &Request::get_path()
{
    return path;
}

const std::string &Request::get_version()
{
    return version;
}

void Request::set_method(std::string &name)
{
    method = name;
}

void Request::set_s_request(std::string req)
{
    // free(s_request);
    s_request = req;
    // std::cout << s_request;
}

std::string Request::get_s_request()
{
    return s_request;
}

void Request::set_path(std::string &name)
{
    path = name;
}

void Request::set_version(std::string &name)
{
    version = name;
}

bool Request::get_parse_index()
{
    return index;
}

void Request::set_parse_index(bool index)
{
    this->index = index;
}

size_t Request::get_content_length()
{
    return content_length;
}
void Request::set_content_length(size_t length)
{
    content_length = length;
}

bool is_allowed_char(char c)
{
    if (c != '/' && c != '?' && c != '=' && c != '&' && c != '#' && c != '-' && c != '_' && c != ':' && c != '.' && c != ' ' && (int)c != 13 && c != '(' && c != ')' && c != '[' && c != ']')
        return false;
    return true;
}

bool check_request_line(std::string &line)
{
    int i = 0;
    while (line[i])
    {
        // if (!isalnum(line[i]) && !is_allowed_char(line[i]))
        {
            // std::cout << "400 bad request ||" << line[i] << " (ASCII: " << (int)line[i] << ")" << std::endl;
            // return false;
        }
        i++;
    }
    return true;
}

std::string removeslashes(const std::string &line)
{
    std::string result;
    for (size_t i = 0; i < line.size(); ++i)
    {
        result += line[i];
        if (line[i] == '/')
        {
            while (i + 1 < line.size() && line[i + 1] == '/')
            {
                ++i;
            }
        }
    }
    return result;
}

bool Request::fill_headers_map(std::istringstream &ob, std::string &res, Client &client)
{
    client.set_Alive(false);
    std::string line, key, value;
    while (std::getline(ob, line))
    {
        if (line.empty())
            continue;
        std::size_t pos = line.find(':');

        if (pos != std::string::npos)
        {
            key = line.substr(0, pos);
            value = line.substr(pos + 1);
        }
        else
            key = line.substr(pos + 1);
        if (key.empty())
        {
            get_error_res(res, 400, client);
            headers_map.clear();
            return false;
        }
        if (key[0] == 32)
        {
            // std::cout << "400 Bad requeste 2 "<< "|" << (int)key[0] << "|"  << std::endl;
            get_error_res(res, 400, client);
            headers_map.clear();
            return false;
        }
        trim_non_printable(value);
        trim(value);
        if (key == "Connection" && value == "keep-alive")
            client.set_Alive(true);
        headers_map[key] = value;
    }
    return true;
}

std::string Request::get_map_values(std::string key)
{
    std::map<std::string, std::string>::iterator it = headers_map.begin();
    while (it != headers_map.end())
    {
        if (it->first == key)
            return it->second;
        ++it;
    }
    return "NULL";
}

bool out_root_dir(std::string &pa, std::string &res, Client &client)
{
    char **str = ft_split(pa.c_str(), '/');
    int entry = 0;
    int sorty = 0;
    for (int i = 0; str[i]; i++)
    {
        if (strcmp(str[i], "..") == 0)
            sorty++;
        else
            entry++;
        if (sorty > entry)
        {
            get_error_res(res, 400, client);
            return false;
        }
    }
    std::vector<std::string> vec;

    for (int i = 0; str[i]; i++)
    {
        vec.push_back(str[i]);
        if (strcmp(str[i], "..") == 0)
        {
            vec.pop_back();
            vec.pop_back();
        }
    }
    pa = "/";
    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i)
    {
        oss << vec[i];
        if (i != vec.size() - 1)
        {
            oss << "/";
        }
    }
    pa += oss.str();
    int i = 0;
    while(str[i]){
        free(str[i]);
        i++;
    }
    free(str);
    str = NULL;
    return true;
}

bool is_upper(std::string line)
{
    for (std::size_t i = 0; i < line.size(); i++)
    {
        if (!(line[i] >= 'A' && line[i] <= 'Z'))
            return false;
    }
    return true;
}


void hanlde_post_request(Client &client)
{
    static int first;
    static size_t writed;

    if (!first)
    {
        first = 10;
        std::string content_type = client.get_request().get_map_values("Content-Type");
        size_t pos = content_type.find("/");
        std::string extension = content_type.substr(pos + 1);
        trim_non_printable(extension);

        std::string file_name = root + "/" + generate_file_names(extension);

        client.get_request().file.open(file_name.c_str());
        if (!client.get_request().file.is_open())
        {
            std::cerr << "Error: Could not open file " << file_name << std::endl;
            return;
        }
        client.get_request().file << client.get_request().get_s_request() << std::flush;
        if (client.get_request().file.fail())
        {
            std::cerr << "Error: Failed to write to file " << file_name << std::endl;
            client.get_request().file.close();
            return;
        }
        writed += client.get_request().get_s_request().size();
        if (writed >= client.get_request().get_content_length()){
            client.set_all_recv(true);
            first = writed = 0;
        }

    }
    else
    {
        if (!client.get_request().file.is_open())
        {
            std::cerr << "Error: File is not open" << std::endl;
            exit(0);
            return;
        }
        client.get_request().file << client.get_request().get_s_request() << std::flush;
        writed += client.get_request().get_s_request().size();
        if (client.get_request().file.fail())
        {
            std::cerr << "Error: Failed to write to file" << std::endl;
            client.get_request().file.close();
            return;
        }
        if (writed >= client.get_request().get_content_length()){
            client.set_all_recv(true);
            first = writed = 0;
        }
    }
}