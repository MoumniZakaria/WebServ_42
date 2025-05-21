#include "./webserver.hpp"

#define CGI_TIMEOUT 20

int check_extension(std::string full_path)
{
    // size_t i = 0;
    if (full_path.size() >= 4)
    {
        size_t pos = full_path.size() - 4;
        std::string extension = full_path.substr(pos);
        if (extension != ".php")
        {
            extension = extension.substr(1);
            if (extension == ".py")
            {
                return 2;
            }
            else
                return 0;
        }
        else
        {
            return 1;
        }
        return 0;
    }
    return 0;
}

int exec_script(std::string full_path, char *envp[], const char *interpreter , Client &client,   const std::string& post_data)
{

    (void)client;
    char *argv[] = {(char *)interpreter, (char *)full_path.c_str(), NULL};

    int fd[2];
    int input_fd[2];
    
    if (pipe(fd) == -1)
    {
        std::cerr << "Pipe creation failed: " << strerror(errno) << std::endl;
        return -1;
    }


    bool has_post_data = !post_data.empty();
    if (has_post_data) {
        if (pipe(input_fd) == -1) {
            std::cerr << "Input pipe creation failed" << std::endl;
            close(fd[0]);
            close(fd[1]);
            return -1;
        }
    }


    // Set pipe to non-blocking mode
    int flags = fcntl(fd[0], F_GETFL, 0);
    fcntl(fd[0], F_SETFL, flags | O_NONBLOCK);

    pid_t pid;
    if ((pid = fork()) == -1)
    {
        std::cerr << "Fork failed: " << strerror(errno) << std::endl;
        close(fd[0]);
        close(fd[1]);
        if (has_post_data) 
        {
            close(input_fd[0]);
            close(input_fd[1]);
        }
        return -1;
    }

    if (pid == 0)
    {
        close(fd[0]);
        if (has_post_data)
        {
            close(input_fd[1]);
            if (dup2(input_fd[0], STDIN_FILENO) == -1)
            {
                std::cerr << "dup2 for stdin failed" << std::endl;
                close(input_fd[0]);
                close(fd[1]);
                exit(1);
            }
            close(input_fd[0]);
        }


        if (dup2(fd[1], STDOUT_FILENO) == -1)
        {
            std::cerr << "dup2 failed: " << strerror(errno) << std::endl;
            close(fd[1]);
            exit(1);
        }

        close(fd[1]);
        execve(argv[0], argv, envp);
        std::cerr << "execve failed: " << strerror(errno) << std::endl;
        exit(1);
    }

    close(fd[1]);
    if (has_post_data) 
    {
        close(input_fd[0]);
        ssize_t written = 0;
        size_t total_written = 0;
        size_t data_size = post_data.size();
        bool write_error = false;
        
        while (total_written < data_size && !write_error)
        {
            written = write(input_fd[1], post_data.c_str() + total_written, data_size - total_written);
            if (written > 0) {
                total_written += written;
            } else if (written == 0) {
                continue;
            } else {
                std::cerr << "Failed to write POST data" << std::endl;
                write_error = true;
                break;
            }
        }
        close(input_fd[1]);
    }


    struct pollfd pfd;
    pfd.fd = fd[0];
    pfd.events = POLLIN;

    char buffer[10000];
    std::string content;
    bool timeout_occurred = false;

    time_t start_time = time(NULL);

    while (true) 
    {
        // Check if we've exceeded our timeout
        if (time(NULL) - start_time > CGI_TIMEOUT) {
            std::cerr << "CGI script execution timed out after " << CGI_TIMEOUT << " seconds" << std::endl;
            kill(pid, SIGTERM);  // Try to terminate gracefully first
            usleep(100000);      // Wait a bit
            
            // Check if process ended
            int status;
            if (waitpid(pid, &status, WNOHANG) == 0) {
                // Process still running, force kill
                kill(pid, SIGKILL);
            }
            
            timeout_occurred = true;
            break;
        }
        
        // Poll with a short timeout to keep checking our execution time
        int poll_result = poll(&pfd, 1, 100);  // 100ms timeout for poll
        
        if (poll_result == -1) {
            // Poll error
            std::cerr << "Poll error" << std::endl;
            break;
        } else if (poll_result > 0) {
            // Data available to read
            ssize_t n = read(fd[0], buffer, sizeof(buffer));
            if (n > 0) {
                // Data read successfully
                content.append(buffer, n);
            } else if (n == 0) {
                // End of file
                break;
            }
             else 
            {
                    std::cerr << "Read error" << std::endl; //you should retrun -1 to serve error page
                    break;
            }
        }
        
        // Check if process has ended naturally
        int status;
        pid_t result = waitpid(pid, &status, WNOHANG);
        if (result == pid) {
            // Process has ended
            break;
        }
    }

    close(fd[0]);
    if (!content.empty())
        std::cout << "contetnt: " << content << std::endl;
    else
            std::cout << "contetnt emptyyyyyyyyyyy " << std::endl;
    int status;
    if (!timeout_occurred)
    {
        waitpid(pid, &status, 0);
    }

    if (timeout_occurred)
    {
        return (std::cerr << "504 Gateway Timeout: Script execution exceeded time limit" << std::endl, 3);
    }
    else if (WIFEXITED(status))
    {
        if (WEXITSTATUS(status) == 1)
        {
            return (std::cerr << "500 Internal Server Error: Script execution failed" << std::endl, 1);
        }
        else if (WEXITSTATUS(status) == 2)
        {
            return (std::cerr << "404 Not Found: Requested file not found" << std::endl, 2);
        }
    }
    else if (WIFSIGNALED(status))
    {
        return (std::cerr << "500 Internal Server Error: Script terminated by signal " << WTERMSIG(status) << std::endl, 1);
    }
    std::string str = client.get_response().get_response();
    str += content ;

    client.get_response().set_response(str);

    return 0;
}

int cgi_handler(Client &client , std::string body)
{

    char *envp[] = {

        (char *)"GATEWAY_INTERFACE=CGI/1.1",
        (char *)"SERVER_PROTOCOL=HTTP/1.1",
        (char *)"SERVER_SOFTWARE=CustomCppServer/1.0",
        (char *)"REQUEST_METHOD=POST",         // or POST
        (char *)"PATH_INFO=/some/extra/path", // optional
        (char *)"PATH_TRANSLATED=/var/www/html/some/extra/path",
        (char *)"SCRIPT_NAME=/cgi-bin/hello.py",
        (char *)"SCRIPT_FILENAME=/var/www/html/cgi-bin/hello.py",
        (char *)"QUERY_STRING=name=alice&age=22",                 // for GET
        (char *)"CONTENT_TYPE=application/x-www-form-urlencoded", // for POST
        (char *)"CONTENT_LENGTH=29",                              // bytes of POST body

        (char *)"SERVER_NAME=localhost",
        (char *)"SERVER_PORT=8080",
        (char *)"REMOTE_ADDR=127.0.0.1",
        (char *)"REMOTE_PORT=55321",

        (char *)"HTTP_HOST=localhost:8080",
        (char *)"HTTP_USER_AGENT=Mozilla/5.0 (Test Agent)",
        (char *)"HTTP_ACCEPT=text/html,application/xhtml+xml",
        (char *)"HTTP_ACCEPT_LANGUAGE=en-US,en;q=0.9",
        (char *)"HTTP_COOKIE=session=abcdef1234567890",
        (char *)"HTTP_CONNECTION=keep-alive",
        NULL};

    // check_methods(method, routes[i]);
    // if its valid execute script

    int code = check_extension(client.get_request().get_path());
    const char *interpreter;
    if (code > 0)
    {
        if (code == 1)
        {
            // change redirect status from envp to 1;
            interpreter = "/usr/bin/php";
        }
        else if (code == 2)
            interpreter = "/usr/bin/python3";
        int status = exec_script(client.get_request().get_path(), envp, interpreter , client, body);
        return status;
    }
    else
        std::cout << "serve it as regular file" << std::endl;
    return 0;
}

// int main()
// {
//     int status = cgi_handler();
//     if (status == 3)
//         std::cout << "serve 504 Gateway" << std::endl;
//     else if (status == 1)
//         std::cout << "serve 500 Internal Server Error" << std::endl;
//     else if (status == 2)
//         std::cout << "404 Not Found Error" << std::endl;
//     return (0);
// }

// struct pollfd pfd;
// pfd.fd = fd[0];
// pfd.events = POLLIN;
// int ret = poll(&pfd, 1, 3000);  // timeout in milliseconds
// if (ret == -1)
// {
//     perror("poll error");
//     close(fd[0]);
//     kill(pid, SIGKILL);
//     waitpid(pid, NULL, 0);
//     return -1;
// }
// else if (ret == 0)
// {
//     fprintf(stderr, "CGI script timed out (poll).\n");
//     kill(pid, SIGKILL);
//     waitpid(pid, NULL, 0);
//     close(fd[0]);
//     return -1;
// }
// char buffer[4096];
// ssize_t n;
// while ((n = read(fd[0], buffer, sizeof(buffer) - 1)) > 0) {
// 	buffer[n] = '\0';
// 	printf("%s", buffer);
// }