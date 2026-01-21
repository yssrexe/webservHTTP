#include "../../include/cgi.hpp"

/*
"REQUEST_METHOD"
"SCRIPT_NAME"
"SCRIPT_FILENAME"
"QUERY_STRING"
"CONTENT_LENGTH"
"CONTENT_TYPE"
"SERVER_PROTOCOL"
"SERVER_NAME"
"SERVER_PORT"
"REQUEST_URI"
"REMOTE_ADDR"
"HTTP_HOST"
"HTTP_USER_AGENT"
"HTTP_COOKIE"
*/

// !request.getConfig().server_names.empty()
//                                        ? request.getConfig().server_names[0]: "localhost"));

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


void Cgi::handleCgiRequest(const Route &route)
{

    std::vector<std::string> tenv;
    tenv.push_back("REQUEST_METHOD=" + getMethod());
    tenv.push_back("SCRIPT_NAME=" + getTarget());
    tenv.push_back("SCRIPT_FILENAME=" + getScriptFileName(route , getTarget()));
    tenv.push_back("QUERY_STRING=" + getQuerys());
    tenv.push_back("CONTENT_LENGTH=" + getHeader("content-length"));
    tenv.push_back("CONTENT_TYPE=" + getHeader("content-type"));
    tenv.push_back("SERVER_PROTOCOL=" + getVersion());
    tenv.push_back("SERVER_NAME=" + (!server_names.empty() ? server_names[0] : "localhost"));
    tenv.push_back("SERVER_PORT=" + intToString(!ports.empty() ? ports[0] : 80));
    tenv.push_back("REMOTE_ADDR=127.0.0.1");
    tenv.push_back("REQUEST_URI=" + getTarget() + (getQuerys().empty() ? "" : "?" + getQuerys()));
    tenv.push_back("HTTP_HOST=" + getHeader("host"));
    tenv.push_back("HTTP_USER_AGENT=" + getHeader("user-agent"));
    tenv.push_back("HTTP_COOKIE=" + getHeader("cookie"));
    tenv.push_back("REDIRECT_STATUS=200");

    std::vector<char *> envp;
    for (std::vector<std::string>::const_iterator it = tenv.begin(); it != tenv.end(); it++)
    {
        //std::cout << *it << std::endl;
        envp.push_back(const_cast<char *>(it->c_str()));
    }
        
    envp.push_back(NULL);

    std::string interpreter;
    std::string fExten = getFileExtension(); 

    if (fExten == ".py")
        interpreter = "/usr/bin/python3";
    else if (fExten == ".php")
        interpreter = "/usr/bin/php-cgi";
    else if (fExten == ".sh")
        interpreter = "/bin/bash";
    else 
        throw std::runtime_error( "500 1");
    // create pipes
        int pipe_in[2];
    int pipe_out[2];

    if (!pipe(pipe_in) == 0 || !pipe(pipe_out) == 0)
        throw std::runtime_error( "500 2");
    pid_t pid = fork();
    if (pid < 0)
        throw std::runtime_error( "500 3");
    else if (pid == 0)
    {
        close(pipe_in[1]);
        dup2(pipe_in[0], STDIN_FILENO);
        close(pipe_in[0]);
        close(pipe_out[0]);
        dup2(pipe_out[1], STDOUT_FILENO);;
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
    if (getMethod() == "POST" || getMethod() == "GET")
    {
        if (!getBody().empty())
        {
            std::string body = getBody();
            write(pipe_in[1], body.c_str(), body.length());
        }
    }
    close(pipe_in[1]);
    std::string cgiOutput;
    char buffer[1024];
    ssize_t byteRead;
    time_t start_time = time(NULL);
    time_t max_wait = 1;
    while ((byteRead = read(pipe_out[0], buffer, 1024 - 1)) > 0) {
        buffer[byteRead] = '\0';
        cgiOutput += buffer;
        if (time(NULL) - start_time >= max_wait) {
            kill(pid, SIGKILL);
            break;
        }
    }
     if (byteRead < 0)
        std::cerr << "Read from CGI failed \n";
    close(pipe_out[0]);

    int statuspid;
    waitpid(pid, &statuspid, 0);

    if (WIFEXITED(statuspid) && WEXITSTATUS(statuspid) != 0)
        throw std::runtime_error( "500 4");
    
    
    std::string headers;
    std::string body;
    size_t headerEnd = cgiOutput.find("\r\n\r\n");
    std::vector<std::map<std::string, std::string> > collHead;
    if (headerEnd == std::string::npos)
        headerEnd = cgiOutput.find("\n\n");
    
    if (headerEnd != std::string::npos)
    {
        headers = cgiOutput.substr(0, headerEnd);
        body = cgiOutput.substr(headerEnd + (cgiOutput[headerEnd] == '\r' ? 4 : 2));

        // parsing dial herder line
        std::istringstream headerStream(headers);
        std::string line;
        while (std::getline(headerStream, line))
        {
            if (!line.empty() && line.find(':') != std::string::npos)
                collHead.push_back(parseValue(line));
        }
    }
    else
    {
        throw 502;
    }
    
    std::string content_type = "text/html";
    int content_len = body.length();
    std::string connection = "close";
    int status = 200;
    std::vector<std::string> setHeaderCookie;
    for (size_t i = 0; i < collHead.size() ; i++)
    {
        std::map<std::string, std::string>::iterator it = collHead[i].begin();
        if (it->first == "Content-Type")
            content_type = it->second;
        else if (it->first == "Content-Length")
            content_len = std::atoi(it->second.c_str());
        else if (it->first == "Status")
            status = std::atoi(it->second.c_str());
        else if (it->first == "Set-Cookie")
            setHeaderCookie.push_back("Set-Cookie: " + it->second + "\r\n");
        else if (it->first == "Connection")
            connection = it->second;
    }
    std::string statusMessage;
    switch (status) {
        case 200: statusMessage = "OK"; break;
        case 201: statusMessage = "Created"; break;
        case 204: statusMessage = "No Content"; break;
        case 301: statusMessage = "Moved Permanently"; break;
        case 302: statusMessage = "Found"; break;
        case 400: statusMessage = "Bad Cgi"; break;
        case 403: statusMessage = "Forbidden"; break;
        case 404: statusMessage = "Not Found"; break;
        case 500: statusMessage = "Internal Server Error"; break;
        case 502: statusMessage = "Bad Gateway"; break;
        default: statusMessage = "OK"; break;
    }

    
    response = getVersion() + " " + intToString(status) + " " + statusMessage + "\r\n";
    
    time_t now = time(NULL);
    struct tm* gmt = gmtime(&now);
    char dateBuffer[100];
    strftime(dateBuffer, sizeof(dateBuffer), "%a, %d %b %Y %H:%M:%S GMT", gmt);
    response += "Date: " + std::string(dateBuffer) + "\r\n";
    response += "Server: webserv/1.0\r\n";
    response += "Content-Type: " + content_type + "\r\n";
    response += "Content-Length: " + intToString(content_len) + "\r\n";
    response += "Connection: " + connection + "\r\n";
    for (std::vector<std::string>::iterator it = setHeaderCookie.begin(); it < setHeaderCookie.end(); it++)
        response += *it;
    response += "\r\n";
    response += body;
}

