#include "../webserver.hpp"

void extract_cockies(){
    std::ifstream file("db_cgi.txt");

    if (!file.is_open()) {
        std::cerr << "Error: Could not open the file!" << std::endl;
        return ;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::string user, key;

        size_t user_pos = line.find("user");
        size_t key_pos = line.find("key");

        if (user_pos != std::string::npos && key_pos != std::string::npos) {
            size_t user_start = user_pos + 5; 
            size_t user_end = line.find(" ", user_start);
            if (user_end == std::string::npos) {
                user_end = line.length();  
            }
            user = line.substr(user_start, user_end - user_start);

            size_t key_start = key_pos + 4; 
            key = line.substr(key_start);

            std::cout << "User: " << user << std::endl;
            std::cout << "Key: " << key << std::endl;
        } else {
            std::cerr << "Error: 'user' or 'key' not found in the line!" << std::endl;
        }
    }

    file.close();
}