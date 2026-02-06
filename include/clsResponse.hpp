#ifndef CLSRESPONSE_HPP
#define CLSRESPONSE_HPP

#include <iostream>
#include <fcntl.h>   
#include <string.h>
#include "Route.hpp"
#include "Config.hpp"
#include "MyLabrary.hpp"
#include "clsResponse.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <map>
#include <stdint.h>
#include <sys/epoll.h>




class clsResponse
{
private:
    MySpace::BufferRequest &_Buffer;
    bool _HappenError;
    int _NumberError;
    int _fd_Clieant;
    Config _ConfigServer;
    std::string readFile(const std::string& path);
    std::string buildResponse(int code,const std::string file);
    void sendchunks(const std::string path);
    void sendHeader(const std::string file);
    void _performentDelete(std::string path);
    void _performentPost();
    bool CheckIsCGI();
    void sendRedirect(const std::string& redirectPath, int statusCode = 301); // for redirect responses
    void sendHeaderCGI();
    void BuildResponseAutoIndex();
    void BuildResponseCgi();

public:

    void SendResponse();
    clsResponse(int NumberError,MySpace::BufferRequest &Buffer,int fd_Clieant,Config ConfigServer);
    clsResponse(int fd_Client,MySpace::BufferRequest &Buffer,Config ConfigServer);
    ~clsResponse();
};



#endif