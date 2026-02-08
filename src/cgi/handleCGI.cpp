#include "../../include/cgi.hpp"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <fcntl.h>


static std::string getFileNameURI(const std::string& path)
{
    size_t lastSlash = path.find_last_of('/');
    if (lastSlash != std::string::npos) {
        return path.substr(lastSlash + 1);
    }
    return path;
}

static std::string getScriptFileName(const Route &route, const std::string &target)
{
    std::string fname =  getFileNameURI(target);
    
    if (fname.empty() || fname == "")
        throw std::runtime_error("that's not a cgi request");
    return route.rootPath + '/' + fname;
}

static std::string intToString(int value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}


std::map<std::string, std::string> parseValue(std::string &line)
{
    std::map<std::string, std::string> result;
    size_t pos = line.find(':');
    if (pos != std::string::npos)
    {
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        size_t start = value.find_first_not_of(" \t");
        if (start != std::string::npos)
            value = value.substr(start);
        
        size_t end = value.find_last_not_of(" \t\r\n");
        if (end != std::string::npos)
            value = value.substr(0, end + 1);
        
        result[key] = value;
        line = key + ": " + value;
    }
    return result;
}

void Cgi::SetEnv()
{
    env_strings.clear();
    env_strings.push_back("REQUEST_METHOD=" + getMethod());
    env_strings.push_back("SCRIPT_NAME=" + getTarget());
    env_strings.push_back("SCRIPT_FILENAME=" + getScriptFileName(route , getTarget()));
    env_strings.push_back("QUERY_STRING=" + getQuerys());
    env_strings.push_back("CONTENT_LENGTH=" + getHeader("Content-length"));
    env_strings.push_back("CONTENT_TYPE=" + getHeader("Content-Type"));
    env_strings.push_back("SERVER_PROTOCOL=" + getVersion());
    env_strings.push_back("SERVER_NAME=" + (!server_names.empty() ? server_names[0] : "localhost"));
    env_strings.push_back("SERVER_PORT=" + intToString(!ports.empty() ? ports[0] : 80));
    env_strings.push_back("REMOTE_ADDR=127.0.0.1");
    env_strings.push_back("REQUEST_URI=" + getTarget() + (getQuerys().empty() ? "" : "?" + getQuerys()));
    env_strings.push_back("HTTP_HOST=" + getHeader("host"));
    env_strings.push_back("HTTP_USER_AGENT=" + getHeader("user-agent"));
    env_strings.push_back("HTTP_COOKIE=" + getHeader("cookie"));
    env_strings.push_back("REDIRECT_STATUS=200");
    
    envp.clear(); 
    for (std::vector<std::string>::iterator it = env_strings.begin(); it != env_strings.end(); it++)
    {
        envp.push_back(const_cast<char *>(it->c_str()));
    }
        
    envp.push_back(NULL);

    
    std::string fExten = getFileExtension(); 

    if (fExten == ".py")
        interpreter = "/usr/bin/python3";
    else if (fExten == ".php")
        interpreter = "/usr/bin/php-cgi";
    else if (fExten == ".sh")
        interpreter = "/bin/bash";
    else
    {
        std::cout << "hna 2" << std::endl;
        throw HTTP_INTERNAL_SERVER_ERROR;
    }
}

void Cgi::CreateChild()
{
    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
    {
        std::cout << "hna 3" << std::endl;
        throw HTTP_INTERNAL_SERVER_ERROR;
    }
        
    pid = fork();
    if (pid < 0)
    {
        std::cout << "Faild fork " << std::endl;
        {
            std::cout << "hna 4" << std::endl;
            throw HTTP_INTERNAL_SERVER_ERROR;
        }
        
    }
    else if (pid == 0)
    {
        close(pipe_in[1]);
        dup2(pipe_in[0], STDIN_FILENO);
        close(pipe_in[0]);
        close(pipe_out[0]);
        dup2(pipe_out[1], STDOUT_FILENO);
        close(pipe_out[1]);

        std::string scriptPath = getScriptFileName(route, getTarget());
        char *argv[] = {
            const_cast<char *>(interpreter.c_str()),
            const_cast<char *>(scriptPath.c_str()), 
            NULL
        };

        execve(interpreter.c_str(), argv, envp.data());
        perror("execve failed\n");
        exit(1);
    }
    close(pipe_in[0]);
    close(pipe_out[1]);

}

void Cgi::WritePostBodyToPipe()
{
    if (_method != "POST" || !_Buffer.BufferRead.ContentLength)
    {
        close(pipe_in[1]);
        return;
    }

    if (_Buffer.BufferRead.ContentLength > static_cast<size_t>(max_body_size))
    {
        close(pipe_in[1]);
        std::cout << "hna 5" << std::endl;
        throw HTTP_INTERNAL_SERVER_ERROR;
    }

    std::string filePath = _Buffer.BufferRead.RequestAtEnd.TargetCGI;
    int fd = open(filePath.c_str(), O_RDONLY);
    if (fd < 0)
    {
        close(pipe_in[1]);
        std::cout << "hna 6" << std::endl;

        throw HTTP_INTERNAL_SERVER_ERROR;
    }

    char buffer[8192];
    ssize_t bytesRead;
    size_t totalWritten = 0;

    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0)
    {
        if (totalWritten + bytesRead > _Buffer.BufferRead.ContentLength)
        {
            bytesRead = _Buffer.BufferRead.ContentLength - totalWritten;
        }

        ssize_t bytesWritten = write(pipe_in[1], buffer, bytesRead);
        if (bytesWritten < 0)
        {
            close(fd);
            close(pipe_in[1]);
        std::cout << "hna 7" << std::endl;

            throw HTTP_INTERNAL_SERVER_ERROR;
        }
        totalWritten += bytesWritten;
        if (totalWritten >= _Buffer.BufferRead.ContentLength)
            break;
    }

    if (bytesRead < 0)
    {
        close(fd);
        close(pipe_in[1]);
        std::cout << "hna 8" << std::endl;

        throw HTTP_INTERNAL_SERVER_ERROR;
    }

    close(fd);
    close(pipe_in[1]);
    _Buffer.BufferRead.ofset = totalWritten;
}

MySpace::BufferRequest Cgi::handleCgiRequest(MySpace::BufferRequest buffer)
{
    route = buffer.BufferRead.RequestAtEnd.route;
    _Buffer = buffer;

    if (!_Buffer.BufferRead.CreateEnv)
    {
        SetEnv();
        _Buffer.BufferRead.CreateEnv = true;
    }

    if (!_Buffer.BufferRead.forked)
    {
        CreateChild();
        _Buffer.BufferRead.forked = true;
        _Buffer.BufferRead._pid = pid;
        _Buffer.BufferRead.pipe_in_fd = pipe_in[1];
        _Buffer.BufferRead.pipe_out_fd = pipe_out[0];
        WritePostBodyToPipe();
    }
    else
    {
        pid = _Buffer.BufferRead._pid;
        pipe_in[1] = _Buffer.BufferRead.pipe_in_fd;
        pipe_out[0] = _Buffer.BufferRead.pipe_out_fd;
    }
    int waitResult = waitpid(pid, &statuspid, WNOHANG);
    if (waitResult > 0 && WIFEXITED(statuspid)) 
    {
        if (WEXITSTATUS(statuspid) != 0)
        {
            std::cout << "hna 8" << std::endl;
            throw HTTP_INTERNAL_SERVER_ERROR;
        }
            
        _Buffer.BufferRead.finishExc = true;
        _Buffer.BufferRead.isComplete = true;
        _Buffer.BufferWrite.isfileOpen = true;
        _Buffer.BufferWrite.fd = pipe_out[0]; // _Buffer.BufferWrite.fd
    }
    return _Buffer;
}

