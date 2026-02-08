#include "../include/clsRequest.hpp"
#include <iostream>
   
   
void clsRequest::_parsingMethod(std::string Method)
{
   if (Method != MySpace::_Method[MySpace::GET]     &&
        Method != MySpace::_Method[MySpace::DELETE] &&
        Method != MySpace::_Method[MySpace::POSTE])
    {
        throw HTTP_METHOD_NOT_ALLOWED;
    }

}

void clsRequest::_parsingTarget(std::string target)
{
    if (target.empty())
        throw HTTP_BAD_REQUEST;
    if (target.size() > MAX_URI)
        throw HTTP_URI_TOO_LONG;
    if (target[0] != '/')
        throw HTTP_BAD_REQUEST;
    if (target.find('#') != std::string::npos)
        throw HTTP_BAD_REQUEST;
    if (target.find("..") != std::string::npos)
        throw HTTP_BAD_REQUEST;
}

void clsRequest::_ParsingVersion(std::string virsion)
{
    if (virsion != "HTTP/1.1" && virsion != "HTTP/1.0")
        throw HTTP_BAD_REQUEST;
}

void clsRequest::_parsingHeader(std::string header)
{
    std::vector<std::string> vOnHeader = MySpace::_Split(header,"\r\n") ;
    while (!vOnHeader.empty())
    {
        std::string line = vOnHeader.front();
        vOnHeader.erase(vOnHeader.begin());
        std::vector<std::string> vOnLine = MySpace::_Split(line,": ");
        if (0)
            throw HTTP_BAD_REQUEST;
        _BufferRequest.BufferRead.RequestAtEnd.headers[vOnLine[Key]] = vOnLine[Value];
    }
    if (_BufferRequest.BufferRead.RequestAtEnd.headers.count("Content-Length") > 0)
        _BufferRequest.BufferRead.ContentLength = std::atoi(_BufferRequest.BufferRead.RequestAtEnd.headers["Content-Length"].c_str());
    else
        _BufferRequest.BufferRead.ContentLength = 0;
    
    if (_BufferRequest.BufferRead.RequestAtEnd.headers.count("Content-Type") > 0)
        _BufferRequest.BufferRead.Content_Type = _BufferRequest.BufferRead.RequestAtEnd.headers["Content-Type"];
    
    else
        _BufferRequest.BufferRead.Content_Type = "";
    if (_BufferRequest.BufferRead.RequestAtEnd.headers.count("Cookie") > 0)
    {
        _BufferRequest.BufferRead.cookieValues = MySpace::parseCookies(_BufferRequest.BufferRead.RequestAtEnd.headers["Cookie"]);
        _BufferRequest.BufferRead.isCookie = true;
    }
    else
        _BufferRequest.BufferRead.isCookie = false;
}

clsRequest::clsRequest(MySpace::BufferRequest& BufferRequest) : _BufferRequest(BufferRequest)
{
}

void clsRequest::_ParsingLine(std::string Line)
{
    std::vector<std::string> SLine = MySpace::_Split(Line,' ');
    std::vector<std::string>  SForTargetandQuery = MySpace::_Split(SLine[1],'?');
    if (SLine.size() != 3)
        throw HTTP_BAD_REQUEST;
    _parsingMethod(SLine[0]);
    _parsingTarget(SLine[1]);
    _ParsingVersion(SLine[2]);
    _BufferRequest.BufferRead.RequestAtEnd.method = SLine[0];
    _BufferRequest.BufferRead.RequestAtEnd.target = SForTargetandQuery[0];
    _BufferRequest.BufferRead.RequestAtEnd.queryString = (SForTargetandQuery.size() > 1) ? SForTargetandQuery[1] : "";
    _BufferRequest.BufferRead.RequestAtEnd.version = SLine[2];
    _BufferRequest.BufferRead.RequestAtEnd.isRequestForCGI = MySpace::IS_CGI(_BufferRequest.BufferRead.RequestAtEnd.target);
}

void clsRequest::ParsingGetMethod()
{
    if (_BufferRequest.BufferRead.sState == MySpace::PARSE_HEADERS)
        return;
    size_t posLine =  _BufferRequest.BufferRead.Buffer.find("\r\n");
    if (posLine == std::string::npos)
       throw HTTP_BAD_REQUEST;
    size_t posHeadersEnd = _BufferRequest.BufferRead.Buffer.find("\r\n\r\n");
    if (posHeadersEnd == std::string::npos)
        throw HTTP_BAD_REQUEST;
    std::string requestLine = _BufferRequest.BufferRead.Buffer.substr(0, posLine);
    _ParsingLine(requestLine);
    size_t headersStart = posLine + 2;
    size_t headersLen   = posHeadersEnd - headersStart;
    std::string headers = _BufferRequest.BufferRead.Buffer.substr(headersStart, headersLen);
    _parsingHeader(headers);
    _BufferRequest.BufferRead.Buffer.erase(0,posHeadersEnd);
    _BufferRequest.BufferRead.Buffer.erase(0,4);
    _BufferRequest.BufferRead.Content_Type = _BufferRequest.BufferRead.RequestAtEnd.headers["Content-Type"];
    _BufferRequest.BufferRead.boundary = "--" + MySpace::extractBoundary(_BufferRequest.BufferRead.Content_Type);
    _BufferRequest.BufferRead.boundaryEnd = _BufferRequest.BufferRead.boundary + "--";
    _BufferRequest.BufferRead.isMultipart = MySpace::isMultipartRequest(_BufferRequest.BufferRead.Content_Type);
 
    _BufferRequest.BufferRead.parsingLineAndHeader = true;
    if (_BufferRequest.type == MySpace::GET || _BufferRequest.type == MySpace::DELETE)
    {
        _BufferRequest.BufferRead.sState = MySpace::PARSE_HEADERS;
        _BufferRequest.BufferRead.isComplete = true;
    }
}

void clsRequest::ParsingDeleteRequest()
{
    ParsingGetMethod();
}

void clsRequest::ParsingPosteRequest()
{
    if (!_BufferRequest.BufferRead.parsingLineAndHeader)
    {
        ParsingGetMethod();
        _BufferRequest.BufferRead.sState = MySpace::PARSE_HEADERS;
    }    
}

MySpace::BufferRequest clsRequest::ParsingRequest()
{
    if (_BufferRequest.type == MySpace::UNKNOWN)
        _BufferRequest.type = MySpace::_GetTypeRequest(_BufferRequest.BufferRead.Buffer);
    switch (_BufferRequest.type)
    {
        case MySpace::GET:
            ParsingGetMethod();
            break;
        
        case MySpace::DELETE:
            ParsingDeleteRequest();
            break;
        
        case MySpace::POSTE:
            ParsingPosteRequest();
            break;
        case MySpace::UNKNOWN:
            throw HTTP_BAD_REQUEST;
    }
    return _BufferRequest;
   
}

clsRequest::~clsRequest()
{
}
