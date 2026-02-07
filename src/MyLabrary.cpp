#include "../include/MyLabrary.hpp"
#include <string>
#include <sstream>
#include "../include/cgi.hpp"
#include <ctime>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <dirent.h>



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

    std::string GetContentType(std::string& Buffer)
    {
        std::size_t pos = Buffer.find("\r\n\r\n");
        if (pos == std::string::npos)
            return "";
        std::string metadata = Buffer.substr(0, pos);
        size_t contentTypePos = metadata.find("Content-Type: ");
        if (contentTypePos == std::string::npos)
            return "";
        size_t start = contentTypePos + 13;
        size_t end = metadata.find("\r\n", start);
        if (end == std::string::npos)
            end = metadata.length();
        return metadata.substr(start, end - start);
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
            std::string headers = temp.substr(0, posLine);
            std::vector<std::map<std::string, std::string> > collHead;
            std::istringstream headerStream(headers);
            std::string line;
            while (std::getline(headerStream, line))
            {
                if (!line.empty() && line.find(':') != std::string::npos)
                    collHead.push_back(parseValue(line));
            }

            for (size_t i = 0; i < collHead.size() ; i++)
            {
                std::map<std::string, std::string>::iterator it = collHead[i].begin();
                if (it->first == "Content-Type")
                    _Buffer.BufferWrite.Content_Type = it->second;
                else if (it->first == "Status")
                    _Buffer.BufferWrite.status = std::atoi(it->second.c_str());
                else if (it->first == "Set-Cookie")
                    _Buffer.BufferWrite.setHeaderCookie.push_back("Set-Cookie: " + it->second + "\r\n");
                else if (it->first == "Connection")
                    _Buffer.BufferWrite.connection = it->second;
            }
            std::string statusMessage;
            switch (_Buffer.BufferWrite.status) {
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
            // std::cout << "status code : " << intToString(_Buffer.BufferWrite.status) << std::endl;
            _Buffer.BufferWrite._headers = _Buffer.BufferRead.RequestAtEnd.version + " " + intToString(_Buffer.BufferWrite.status) + " " + statusMessage + "\r\n";
            _Buffer.BufferWrite._headers += "Server: webserv/1.0\r\n";
            _Buffer.BufferWrite._headers += "Content-Type: " + _Buffer.BufferWrite.Content_Type + "\r\n";
            _Buffer.BufferWrite._headers += "Connection: " + _Buffer.BufferWrite.connection + "\r\n";
            
            for (std::vector<std::string>::iterator it = _Buffer.BufferWrite.setHeaderCookie.begin(); it < _Buffer.BufferWrite.setHeaderCookie.end(); it++)
                _Buffer.BufferWrite._headers += *it;
            _Buffer.BufferWrite._headers += "\r\n";
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

std::string MySpace::buildAutoIndexPage(const std::vector<std::string>& fileList, const std::string& path,std::string NameRoute)
{
    std::stringstream html;
    html << "<!DOCTYPE html><html><head><title>Index of " << path << "</title>";
    html << "<style>";
    html << "body { font-family: Arial, sans-serif; margin: 20px; background-color: #f5f5f5; }";
    html << "h1 { color: #333; }";
    html << "ul { list-style: none; padding: 0; }";
    html << "li { padding: 10px; margin: 5px 0; background-color: #fff; border-radius: 4px; transition: all 0.3s; }";
    html << "li:hover { background-color: #e8f4f8; transform: translateX(5px); box-shadow: 0 2px 8px rgba(0,0,0,0.1); }";
    html << "a { text-decoration: none; color: #0066cc; font-weight: 500; }";
    html << "a:hover { color: #0052a3; text-decoration: underline; }";
    html << "</style></head><body>";
    html << "<h1>Index of " << path << "</h1><ul>";
    
    for (size_t i = 0; i < fileList.size(); ++i)
    {
        std::string file = fileList[i];
        std::string fullPath = NameRoute;
        if (fullPath[fullPath.length() - 1] != '/')
            fullPath += "/";
        fullPath += file;
        html << "<li><a href=\"" << fullPath << "\">" << file << "</a></li>";
    }
    
    html << "</ul></body></html>";
    return html.str();
}

std::vector<std::string> MySpace::getAutoIndexList(const std::string& path)
{
    std::vector<std::string> fileList;
    DIR* dir = opendir(path.c_str());
    if (dir)
    {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL)
        {
            if (std::string(entry->d_name) != "." && std::string(entry->d_name) != "..")
                fileList.push_back(entry->d_name);
        }
        closedir(dir);
    }
    return fileList;
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

std::map<std::string, std::string> MySpace::parseCookies(std::string cookieHeader)
{
    std::map<std::string, std::string> cookies;

    if (cookieHeader.empty())
        return cookies;

    std::vector<std::string> cookiePairs = _Split(cookieHeader, "; ");

    for (size_t i = 0; i < cookiePairs.size(); ++i)
    {
        size_t pos = cookiePairs[i].find('=');
        if (pos != std::string::npos)
        {
            std::string key = cookiePairs[i].substr(0, pos);
            std::string value = cookiePairs[i].substr(pos + 1);

            size_t keyStart = key.find_first_not_of(" \t");
            size_t keyEnd = key.find_last_not_of(" \t");
            if (keyStart != std::string::npos && keyEnd != std::string::npos)
                key = key.substr(keyStart, keyEnd - keyStart + 1);
            
            size_t valueStart = value.find_first_not_of(" \t");
            size_t valueEnd = value.find_last_not_of(" \t");
            if (valueStart != std::string::npos && valueEnd != std::string::npos)
                value = value.substr(valueStart, valueEnd - valueStart + 1);
            cookies[key] = value;
        }
    }
    return cookies;
}



