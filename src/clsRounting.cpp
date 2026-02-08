#include "../include/clsRounting.hpp"

clsRounting::clsRounting(MySpace::BufferRequest& Buffer,Config server) : _Buffer(Buffer) , _server(server)
{

}

bool clsRounting::_isMethodAllowed(std::string& target,const std::vector<std::string> methods)
{
    for (size_t i = 0; i < methods.size(); i++)
    {
        if (methods[i] == target)
            return true;
    }
    return false;
}

std::string clsRounting::_buildPath(const Route& route, std::string target)
{
    std::string relative;

    if (target.empty())
        target = "/";

    if (route.name == "/")
        relative = target;
    else
        relative = target.substr(route.name.length());

    if (relative.empty() || relative == "/")
        return route.rootPath + "/" + route.default_file;

    if (relative[0] != '/')
        relative = "/" + relative;

    return route.rootPath + relative;
}

bool clsRounting::isFile(const std::string& path)
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0)
        return false;
    return S_ISREG(st.st_mode);
}

bool clsRounting::isDirectory(const std::string& path)
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0)
        return false;
    return S_ISDIR(st.st_mode);
}

bool clsRounting::_CanRead(const std::string& path)
{
    return access(path.c_str(), R_OK) == 0;
}

bool clsRounting::_CanWrite(const std::string& path)
{
    return access(path.c_str(), W_OK) == 0;
}

int clsRounting::_fileExists(const std::string& path, const Route* route)
{
    if (_Buffer.type == MySpace::GET)
    {
        if (isFile(path) && _CanRead(path))
            return HTTP_SUCCESS;
        else if (isDirectory(path))
        {
            if (route->dir_listing == "on")
            {
                _Buffer.BufferRead.isAutoIndex = true;
                _Buffer.BufferRead.autoIndexList = MySpace::getAutoIndexList(path);
                return HTTP_SUCCESS;
            }
            else
             return HTTP_FORBIDDEN;
        }
        return HTTP_NOT_FOUND;
    }
    else if (_Buffer.type == MySpace::DELETE)
    {
        if (!isFile(path))
            return HTTP_NOT_FOUND;
        return HTTP_SUCCESS;
    }

    return HTTP_SUCCESS;
}

const Route* clsRounting::_findRoute(const std::string& target)
{
    const Route* best = NULL;
    size_t bestLen = 0;

    for (size_t i = 0; i < _server.routes.size(); ++i)
    {
        const Route& r = _server.routes[i];
        const std::string& name = r.name;

        if (target.compare(0, name.length(), name) != 0)
            continue;

        if (target.length() > name.length() && name[name.length() - 1] != '/' && target[name.length()] != '/')
            continue;

        if (name.length() > bestLen)
        {
            best = &r;
            bestLen = name.length();
        }
    }

    return best;
}

void clsRounting::checkRoutingForPost()
{
    if (_Buffer.BufferRead.isRouting || (_Buffer.BufferRead.isMultipart && !_Buffer.BufferRead.eraseHeadersDone))
        return;

    if (!_isMethodAllowed(_Buffer.BufferRead.RequestAtEnd.method, _server.allowed_methods))
        throw HTTP_METHOD_NOT_ALLOWED;

    const Route* route = _findRoute(_Buffer.BufferRead.RequestAtEnd.target);
    if (!route)
        throw HTTP_NOT_FOUND;

    if (!_isMethodAllowed(_Buffer.BufferRead.RequestAtEnd.method, route->allowed_methods))
        throw HTTP_METHOD_NOT_ALLOWED;


    _Buffer.BufferRead.isRouting = true;
    _Buffer.BufferRead.RequestAtEnd.route = *route;
    _Buffer.BufferRead.RequestAtEnd.target = "" + _Buffer.BufferRead.RequestAtEnd.route.upload_dir + "/" + MySpace::generateUniqueFilename(_Buffer.BufferRead.Content_Type);
}

bool clsRounting::IsNotPostMethod()
{
    if (_Buffer.type != MySpace::POSTE || (_Buffer.type == MySpace::POSTE && _Buffer.BufferRead.RequestAtEnd.isRequestForCGI))
        return true;
    return false;
}

MySpace::BufferRequest clsRounting::CheckRounting()
{
    if (_Buffer.BufferRead.ContentLength > (size_t)_server.max_body_size)
        throw HTTP_PAYLOAD_TOO_LARGE;
    if (IsNotPostMethod())
    {
     
        if (_Buffer.BufferRead.isRouting || !_Buffer.BufferRead.parsingLineAndHeader)
            return _Buffer;

        _Buffer.BufferRead.isRouting = true;
        if (!_isMethodAllowed(_Buffer.BufferRead.RequestAtEnd.method, _server.allowed_methods))
            throw HTTP_METHOD_NOT_ALLOWED;
        const Route* route = _findRoute(_Buffer.BufferRead.RequestAtEnd.target);
        if (!route)
            throw HTTP_NOT_FOUND;
        
        if (!_isMethodAllowed(_Buffer.BufferRead.RequestAtEnd.method, route->allowed_methods))
            throw HTTP_METHOD_NOT_ALLOWED;
        
        _Buffer.BufferRead.RequestAtEnd.route = *route;
        
        if (route->redirect.size())
        {
            std::map<int, std::string>::const_iterator it = route->redirect.begin();
            
            _Buffer.BufferRead.RequestAtEnd.target = it->second;
            _Buffer.BufferRead.nbrRedirects = it->first;
            _Buffer.BufferRead.isRedirection = true;
            return _Buffer;
        }
        _Buffer.BufferRead.RequestAtEnd.target = _buildPath(*route,_Buffer.BufferRead.RequestAtEnd.target);        
        int Result =  _fileExists(_Buffer.BufferRead.RequestAtEnd.target,route);
        if (Result != HTTP_SUCCESS)
            throw Result;
        if (_Buffer.type == MySpace::POSTE && _Buffer.BufferRead.RequestAtEnd.isRequestForCGI)
            _Buffer.BufferRead.RequestAtEnd.TargetCGI = "" + route->upload_dir + "/" + MySpace::generateUniqueFilename(_Buffer.BufferRead.Content_Type);
        
    }
    else
        checkRoutingForPost();
    

    return _Buffer;
}

clsRounting::~clsRounting()
{

}