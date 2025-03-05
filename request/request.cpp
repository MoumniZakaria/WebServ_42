#include "../webserver.hpp"


Request::Request()
{
    // s_request = NULL;
};
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

void Request::set_s_request(std::string  req)
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

bool Request::fill_headers_map(std::istringstream &ob, std::string &res)
{
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
            // std::cout << "400 Bad requeste 1" << std::endl;
            get_error_res(res, 400);
            headers_map.clear();
            return false;
        }
        if (key[0] == 32)
        {
            std::cout << "400 Bad requeste 2 "<< "|" << (int)key[0] << "|"  << std::endl;
            get_error_res(res, 400);
            headers_map.clear();
            return false;
        }
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

size_t Request::get_length()
{
    return length;
}
void Request::set_length(size_t len)
{
    length = len;
}

int Request::get_bodyStart()
{
    return BodyStart;
}

void Request::set_bodyStart(int pos)
{
    BodyStart = pos;
}

bool out_root_dir(std::string &pa, std::string &res)
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
            get_error_res(res, 400);
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

std::string get_file_ex(std::string name){
    std::string str = "bat";
    if (name == "ation/pdf")
        str = "file.pdf";
    else if (name == "mp4")
        str = "file.mp4";
    else if (name == "png")
        str = "file.png";
    else if (name == "jpg")
        str = "file.jpg";
    else if (name == "jpeg")
        str = "file.jpeg";
    return str;
}

std::ofstream file; 

void hanlde_post_request(Client &client, int first , std::string req)
{

    if (first)
    {
        std::string content_type = client.get_request().get_map_values("Content-Type");
        std::string file_extension;
        if (content_type.size() > 7)
        {
            file_extension = content_type.substr(7);
            for (size_t i = 0; i < file_extension.size(); i++)
            {
                if (file_extension[i] < 32 || file_extension[i] > 126)
                {
                    file_extension.resize(i);
                    break;
                }
            }
        }
        std::cout << file_extension << std::endl;
        std::string file_name = get_file_ex(file_extension);
        file.open(file_name.c_str());
        if (!file.is_open())
        {
            std::cerr << "Error: Could not open file " << file_name << std::endl;
            return;
        }
        file  << req << std::flush;
            if (file.fail())
            {
                std::cerr << "Error: Failed to write to file " << file_name << std::endl;
                file.close();
                return;
            }
    }
    else
    {
        if (!file.is_open())
        {
            std::cerr << "Error: File is not open" << std::endl;
            exit (0);
            return;
        }
        file << client.get_request().get_s_request() << std::flush;
        if (file.fail())
        {
            std::cerr << "Error: Failed to write to file" << std::endl;
            file.close();
            return;
        }
    }
}