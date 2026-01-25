#include "../include/MyLabrary.hpp"
#include <string>
#include <sstream>
#include <ctime>
#include <sys/time.h>
 #include <sys/ioctl.h>
 #include <unistd.h>



namespace MySpace 
{
    std::string _Method[4] = {"GET", "DELETE", "POST", ""};
}

namespace MySpace
{


    std::string getStatusReason(int statusCode)
    {
        switch (statusCode)
        {
            case HTTP_SUCCESS:
                return "OK";

            case HTTP_NO_CONTENT:
                return "No Content";

            case HTTP_BAD_REQUEST:
                return "Bad Request";

            case HTTP_UNAUTHORIZED:
                return "Unauthorized";

            case HTTP_FORBIDDEN:
                return "Forbidden";

            case HTTP_NOT_FOUND:
                return "Not Found";

            case HTTP_METHOD_NOT_ALLOWED:
                return "Method Not Allowed";

            case HTTP_REQUEST_TIMEOUT:
                return "Request Timeout";

            case HTTP_CONFLICT:
                return "Conflict";

            case HTTP_PAYLOAD_TOO_LARGE:
                return "Payload Too Large";

            case HTTP_URI_TOO_LONG:
                return "URI Too Long";

            case HTTP_UNSUPPORTED_MEDIA_TYPE:
                return "Unsupported Media Type";

            case HTTP_INTERNAL_SERVER_ERROR:
                return "Internal Server Error";

            default:
                return "Unknown Status Code";
        }
    }

    bool isMultipartRequest(const std::string& contentType)
    {
        return contentType.find("multipart/form-data") != std::string::npos;
    }

    bool CheckIsCGI(MySpace::BufferRequest _Buffer)
    {
        if (_Buffer.BufferRead.RequestAtEnd.isRequestForCGI && _Buffer.BufferRead.isRouting && _Buffer.BufferRead.isComplete)
            return true;
        return false;
    }

    void EraseHearse(MySpace::BufferRequest& _Buffer)
    {
        _Buffer.BufferWrite.isfileOpen = true;

        char buf[40096];
        ssize_t bytesRead = read(_Buffer.BufferWrite.fd, buf, sizeof(buf));
        if (bytesRead > 0)
        {
            buf[bytesRead] = '\0';
            std::string temp(buf, bytesRead);
            size_t posLine = temp.find("\r\n\r\n");
            // std::cout << "pos "<<posLine<< " bytread " << bytesRead << std::endl;
            temp.erase(0, posLine + 4);
            _Buffer.BufferWrite.Buffer.append(temp);
        }
        else 
        {
            close(_Buffer.BufferWrite.fd);
            _Buffer.BufferWrite.isComplete = true;
        }
    }

    size_t getFileSize(const std::string& path)
    {
        struct stat st;
        if (stat(path.c_str(), &st) == 0)
            return st.st_size;
        return 0;
    }

    size_t getPipeSize(int pipeFd)
    {
        int availableBytes = 0;
        if (ioctl(pipeFd, FIONREAD, &availableBytes) == 0 && availableBytes > 0)
            return static_cast<size_t>(availableBytes);
        return 0;
    }
    
    bool endsWith(const std::string& str, const std::string& suffix)
    {
        if (str.length() < suffix.length())
            return false;

        return str.compare(
            str.length() - suffix.length(),
            suffix.length(),
            suffix
        ) == 0;
    }

    std::string getContentType(const std::string& path)
    {
        if (endsWith(path, ".html") || endsWith(path, ".htm"))
            return "text/html";
        if (endsWith(path, ".txt"))
            return "text/plain";
        if (endsWith(path, ".png"))
            return "image/png";
        if (endsWith(path, ".jpg") || endsWith(path, ".jpeg"))
            return "image/jpeg";
        if (endsWith(path, ".pdf"))
            return "application/pdf";
        if (endsWith(path, ".json"))
            return "application/json";

        if (endsWith(path, ".mp4"))
            return "video/mp4";
        if (endsWith(path, ".avi"))
            return "video/x-msvideo";
        if (endsWith(path, ".mov"))
            return "video/quicktime";
        if (endsWith(path, ".mkv"))
            return "video/x-matroska";
        if (endsWith(path, ".wmv"))
            return "video/x-ms-wmv";

        return "application/octet-stream";
    }

    std::string extractBoundary(const std::string& contentType)
    {
        std::string boundaryPrefix = "boundary=";
        size_t pos = contentType.find(boundaryPrefix);
        if (pos != std::string::npos)
        {
            return contentType.substr(pos + boundaryPrefix.length());
        }
        return "";
    }

    MySpace::BufferRequest trimBoundaryFromBuffer(size_t& dataSize, MySpace::BufferRequest buffer)
    {
        std::string boundaryEnd = buffer.BufferRead.boundaryEnd;
        size_t boundaryPos = buffer.BufferRead.Buffer.find(boundaryEnd);
        
        if (boundaryPos == std::string::npos)
        {
            size_t safeSize = buffer.BufferRead.Buffer.size();
            if (safeSize > boundaryEnd.length())
            {
                dataSize = safeSize - boundaryEnd.length();
            }
            else
                dataSize = 0;
            return buffer;
        }
        
        size_t trimPos = boundaryPos;
        if (trimPos >= 2 && buffer.BufferRead.Buffer[trimPos - 2] == '\r' && buffer.BufferRead.Buffer[trimPos - 1] == '\n')
        {
            trimPos -= 2;
        }
        
        std::string trimmedData = buffer.BufferRead.Buffer.substr(0, trimPos);
        buffer.BufferRead.Buffer = trimmedData;
        dataSize = buffer.BufferRead.Buffer.size();
        buffer.BufferRead.isComplete = true;
        
        return buffer;
    }

    
}

std::string MySpace::getFirstWord(const std::string& s)
{
    size_t pos = s.find(' ');
    if (pos == std::string::npos)
        return s;
    return s.substr(0, pos);
}

bool MySpace::IS_CGI(const std::string& target)
{
    std::vector<std::string> cgiExtensions;
    cgiExtensions.push_back(".cgi");
    cgiExtensions.push_back(".pl");
    cgiExtensions.push_back(".py");
    cgiExtensions.push_back(".sh");
    cgiExtensions.push_back(".php");
    cgiExtensions.push_back(".rb");

    for (size_t i = 0; i < cgiExtensions.size(); ++i)
    {
        if (MySpace::endsWith(target, cgiExtensions[i]))
            return true;
    }
    return false;
}

std::vector<std::string> MySpace::_Split(const std::string& str, const std::string& delimiter) 
{
    std::vector<std::string> result;
    if (str.empty() || delimiter.empty()) {
        result.push_back(str);
        return result;
    }
    size_t start = 0;
    size_t end = str.find(delimiter);
    while (end != std::string::npos) 
    {
        result.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    result.push_back(str.substr(start));
    return result;
}

std::vector<std::string> MySpace::_Split(const std::string& str, char delimiter) 
{
    std::vector<std::string> result;
    if (str.empty()) 
    {
        result.push_back(str);
        return result;
    }
    size_t start = 0;
    size_t end = str.find(delimiter);
    while (end != std::string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }
    result.push_back(str.substr(start));
    return result;
}

MySpace::eType MySpace::_GetTypeRequest(std::string str)
{
    std::string word = MySpace::getFirstWord(str);
    if (word == "GET")
        return MySpace::GET;
    else if (word == "DELETE")
        return MySpace::DELETE;
    else if (word == "POST")
        return MySpace::POSTE;
    return MySpace::UNKNOWN;
}

std::string MySpace::generateUniqueFilename(const std::string& contentType)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    std::stringstream ss;
    ss << "upload_" << tv.tv_sec << "_" << tv.tv_usec;
    
    std::string extension = ".bin";
    if (contentType.find("image/jpeg") != std::string::npos || contentType.find("image/jpg") != std::string::npos)
        extension = ".jpg";
    else if (contentType.find("image/png") != std::string::npos)
        extension = ".png";
    else if (contentType.find("image/gif") != std::string::npos)
        extension = ".gif";
    else if (contentType.find("video/mp4") != std::string::npos)
        extension = ".mp4";
    else if (contentType.find("video/") != std::string::npos)
        extension = ".mp4";
    else if (contentType.find("audio/") != std::string::npos)
        extension = ".mp3";
    else if (contentType.find("text/plain") != std::string::npos)
        extension = ".txt";
    else if (contentType.find("text/html") != std::string::npos)
        extension = ".html";
    else if (contentType.find("application/pdf") != std::string::npos)
        extension = ".pdf";
    else if (contentType.find("application/json") != std::string::npos)
        extension = ".json";
    else if (contentType.find("application/xml") != std::string::npos)
        extension = ".xml";
    
    ss << extension;
    return ss.str();
}



