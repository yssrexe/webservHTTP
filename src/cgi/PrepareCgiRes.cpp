#include "../../include/PrepareCgiRes.hpp"


static std::string intToString(int value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

static std::map<std::string, std::string> parseValue(std::string &line)
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

PrepareCgiRes::PrepareCgiRes(MySpace::BufferRequest _buffer)
{
    int pipe_out_fd = _buffer.BufferWrite.fd;
    
    std::string cgiOutput;
    char buffer[1024];
    ssize_t byteRead;
    time_t start_time = time(NULL);
    time_t max_wait = 1;
    
    while ((byteRead = read(pipe_out_fd, buffer, 1024 - 1)) > 0) {
        buffer[byteRead] = '\0';
        cgiOutput += buffer;
        if (time(NULL) - start_time >= max_wait) {
            break;
        }
    }
    
    if (byteRead < 0)
        throw 500;
    
    close(pipe_out_fd);
    

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


    std::string response;
    response = _buffer.BufferRead.RequestAtEnd.version + " " + intToString(status) + " " + statusMessage + "\r\n";
    
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
    
    _headers = headers;
    _body = body;
}
