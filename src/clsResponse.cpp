#include "../include/clsResponse.hpp"
#include "../include/cgi.hpp"
#include <sstream>


std::string clsResponse::readFile(const std::string& path)
{
    std::ifstream file(path.c_str());
    if (!file.is_open())
    {
        return "open file problem \n";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string clsResponse::buildResponse(int code,const std::string file)
{
    std::string body = readFile(file);
    std::stringstream response;
    response << "HTTP/1.1 " << code << " " << MySpace::getStatusReason(code) << "\r\n";
    response << "Content-Type: "<< MySpace::getContentType(file) << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;

    return response.str();
}

clsResponse::clsResponse(int NumberError,MySpace::BufferRequest &Buffer,int fd_Clieant,Config ConfigServer) : _Buffer(Buffer)
{
    _HappenError = true;
    _NumberError = NumberError;
    _ConfigServer = ConfigServer;
    _fd_Clieant = fd_Clieant;

}

clsResponse::clsResponse(int fd_Client,MySpace::BufferRequest &Buffer,Config ConfigServer) : _Buffer(Buffer)
{
    _HappenError = false;
    _ConfigServer = ConfigServer;
    _fd_Clieant = fd_Client;
}

void clsResponse::sendchunks(const std::string path)
{
    if (!_Buffer.BufferRead.isRouting || !_Buffer.BufferRead.isSendHeader)
        return;
    if (_Buffer.BufferRead.RequestAtEnd.isRequestForCGI && !_Buffer.BufferRead.finishExc)
        return;
    
    if (!_Buffer.BufferWrite.isfileOpen) 
    {
        _Buffer.BufferWrite.fd = open(path.c_str(), O_RDONLY);
        if (_Buffer.BufferWrite.fd < 0)
        {
            std::cout << "fails path " << path << std::endl;
            throw HTTP_UNSUPPORTED_MEDIA_TYPE;
        }
        _Buffer.BufferWrite.isfileOpen = true;
    }

    if(!_Buffer.BufferWrite.Buffer.empty()) 
    {
        ssize_t sent = send(
            _fd_Clieant,
            _Buffer.BufferWrite.Buffer.data(),
            _Buffer.BufferWrite.Buffer.size(),
            0);

        if (sent < 0) 
        {    
            close(_Buffer.BufferWrite.fd);
            _Buffer.BufferWrite.isComplete = true;
            return;
        }
        _Buffer.BufferWrite.ofset += sent;
        _Buffer.BufferWrite.Buffer.erase(0, sent);
    }

    if (_Buffer.BufferWrite.Buffer.empty() && !_Buffer.BufferWrite.isComplete)
    {
        char buf[40096];
        ssize_t bytesRead = read(_Buffer.BufferWrite.fd, buf, sizeof(buf));
        if (bytesRead > 0)
            _Buffer.BufferWrite.Buffer.append(buf, bytesRead);
        else 
        {
            close(_Buffer.BufferWrite.fd);
            _Buffer.BufferWrite.isComplete = true;
        }
    }

    if (_Buffer.BufferWrite.isComplete && _Buffer.BufferWrite.Buffer.empty())
     {
         close(_Buffer.BufferWrite.fd);;
    }
}

void clsResponse::sendHeader(const std::string file)
{
    if (_Buffer.BufferRead.isSendHeader)
        return;   
    size_t fileSize = MySpace::getFileSize(file);
    _Buffer.BufferRead.ContentLength = fileSize;
    _Buffer.BufferRead.isSendHeader = true;
    std::stringstream ss;
    ss << fileSize;
    std::string fileSizeStr = ss.str();
    std::string header;
    header += "HTTP/1.1 200 OK\r\n";
    header += "Content-Type: " + MySpace::getContentType(file) + "\r\n";
    header += "Content-Length: " + fileSizeStr + "\r\n";
    header += "Connection: keep-alive\r\n";
    header += "\r\n";
    if(-1 == send(_fd_Clieant, header.c_str(), header.size(), 0))
        _Buffer.BufferRead.isSendHeader = false;
}

void clsResponse::_performentDelete(std::string path)
{
    std::stringstream response ;


    response << "HTTP/1.1 " << HTTP_SUCCESS << " " << MySpace::getStatusReason(HTTP_SUCCESS) << "\r\n";
    response << "Content-Type: "<< MySpace::getContentType(path) << "\r\n";
    response << "Content-Length: " << 0 << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";

    if (std::remove(path.c_str()) != 0)
    {
        std::cout << "hna 1" << std::endl;
        throw HTTP_INTERNAL_SERVER_ERROR;
    }
        
    if(send(_fd_Clieant, response.str().c_str(), response.str().size(), 0) <= 0)
        std::cout << "failded send in deleled "<< std::endl;
     _Buffer.BufferWrite.isComplete = true;
    
}

void clsResponse::_performentPost()
{
    std::stringstream response;

    response << "HTTP/1.1 201 Created\r\n";
    response << "Content-Length: 0\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";

    send(_fd_Clieant, response.str().c_str(), response.str().size(), 0);

    _Buffer.BufferWrite.isComplete = true;
}

void clsResponse::sendRedirect(const std::string& redirectPath, int statusCode)
{
    std::stringstream response;
    std::string statusMessage;
    
    if (statusCode == 301)
        statusMessage = "Moved Permanently";
    else if (statusCode == 302)
        statusMessage = "Found";
    else if (statusCode == 303)
        statusMessage = "See Other";
    else if (statusCode == 307)
        statusMessage = "Temporary Redirect";
    else
        statusMessage = "Moved";
    
    response << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
    response << "Location: " << redirectPath << "\r\n";
    response << "Content-Length: 0\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    
    send(_fd_Clieant, response.str().c_str(), response.str().size(), 0);
    _Buffer.BufferWrite.isComplete = true;
}

void clsResponse::sendHeaderCGI()
{
    if (_Buffer.BufferRead.isSendHeader)
        return;

    _Buffer.BufferRead.isSendHeader = true;
    if(-1 == send(_fd_Clieant, _Buffer.BufferWrite._headers.c_str(), _Buffer.BufferWrite._headers.size(), 0))
        _Buffer.BufferRead.isSendHeader = false;
}

void clsResponse::BuildResponseAutoIndex()
{
    std::string body = MySpace::buildAutoIndexPage(_Buffer.BufferRead.autoIndexList, _Buffer.BufferRead.RequestAtEnd.target, _Buffer.BufferRead.RequestAtEnd.route.name);
        std::stringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: text/html\r\n";
        response << "Content-Length: " << body.size() << "\r\n";
        response << "Connection: close\r\n";
        response << "\r\n";
        response << body;

        send(_fd_Clieant, response.str().c_str(), response.str().size(), 0);
        _Buffer.BufferWrite.isComplete = true;
}

void clsResponse::BuildResponseCgi()
{
    Cgi cgi(_Buffer.BufferRead.RequestAtEnd, 0, _ConfigServer);
        switch (_Buffer.type)
        {
            case MySpace::GET:
            case MySpace::POSTE:
                _Buffer = cgi.handleCgiRequest(_Buffer);
                if (_Buffer.BufferRead.finishExc)
                {
                    MySpace::EraseHearse(_Buffer);
                    sendHeaderCGI();
                    sendchunks(_Buffer.BufferWrite.RequestAtEnd.target);
                    _Buffer.BufferWrite.isComplete = true;
                }
                break;
            case MySpace::DELETE:
                _performentDelete(_Buffer.BufferRead.RequestAtEnd.target);
                break;
            case MySpace::UNKNOWN:
                throw HTTP_BAD_REQUEST;
                break;
        }
}

void clsResponse::SendResponse()
{

    std::string response;

    if (_HappenError)
    {
        response = buildResponse(_NumberError,_ConfigServer.error_pages.at(_NumberError));
        send(_fd_Clieant, response.c_str(), response.size(), 0);

    }
    else  if (_Buffer.BufferRead.isRedirection)
    {
        sendRedirect(_Buffer.BufferRead.RequestAtEnd.target,_Buffer.BufferRead.nbrRedirects);
    }
    else if (_Buffer.BufferRead.isAutoIndex)
    {
        BuildResponseAutoIndex();
    }
    else if (MySpace::CheckIsCGI(_Buffer))
    {
        BuildResponseCgi();
    }
    else if (_Buffer.BufferRead.isRouting && _Buffer.BufferRead.isComplete)
    {
        switch (_Buffer.type)
        {
            case MySpace::GET:
                sendHeader(_Buffer.BufferRead.RequestAtEnd.target);
                sendchunks(_Buffer.BufferRead.RequestAtEnd.target);
                break;
            case MySpace::DELETE:
                _performentDelete(_Buffer.BufferRead.RequestAtEnd.target);
                break;
            case MySpace::POSTE:
                _performentPost();
                _Buffer.BufferWrite.isComplete = true;
                break;
            case MySpace::UNKNOWN:
                throw HTTP_BAD_REQUEST;
                break;
        
        }
    }
}

clsResponse::~clsResponse()
{

    
}
 