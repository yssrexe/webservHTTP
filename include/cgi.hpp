#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <sys/socket.h>
#include "Config.hpp"
#include "Route.hpp"
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <unistd.h>
#include <sys/wait.h>
#include "MyLabrary.hpp"

class Cgi : public Config
{
private:
    std::string _method;
    
    std::string _target;
    std::string _querys;
    std::string _version;
    std::string _bodyFilePath;
    std::map<std::string, std::string> _headers;

public:
    int _fd_client;
    std::string response;
    Cgi(MySpace::DataRequestForGetDelete &Request, int fd_client);
    ~Cgi();
    
    // Getters
    std::string getMethod() const;
    std::string getTarget() const;
    std::string getQuerys() const;
    std::string getVersion() const;
    std::string getBodyFilePath() const;
    std::string getPath() const;
    std::string getURI() const;
    std::string getHeader(const std::string& key) const;
    const std::map<std::string, std::string>& getHeaders() const;
    std::string getBody() const;
    std::string getFileExtension() const;
    
    // Setters
    void setMethod(const std::string& method);
    void setTarget(const std::string& target);
    void setQuerys(const std::string& querys);
    void setVersion(const std::string& version);
    void setBodyFilePath(const std::string& filePath);
    void addHeader(const std::string& key, const std::string& value);
    void setHeaders(const std::map<std::string, std::string>& headers);
    
    MySpace::BufferRequest Cgi::handleCgiRequest(MySpace::BufferRequest _buffer);
    void clear();
    //bool isCgiRequest(const Route &route, std::string &errorMessage) const;
};

ssize_t sendAll(int socket_fd, const char* data, size_t length);

#endif
