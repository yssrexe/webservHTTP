#include "../../include/Config.hpp"
#include "../../include/Route.hpp"
#include <algorithm>
#include <sstream>



Servers::Servers(std::istream &file)
{
    std::string line;
    std::string allLines;
    bool foundOpeningBracket = false;
    bool foundClosingBracket = false;
    bool foundAtLeastOneServer = false;
    int bracketCount = 0;
    lineNumber = -1;
    
    while (std::getline(file, line))
    {
        if (line.empty() && !foundOpeningBracket)
            continue;
        if (!foundOpeningBracket)
        {
            std::string trimmed = line;
            trimmed.erase(std::remove(trimmed.begin(), trimmed.end(), ' '), trimmed.end());
            
            if (trimmed.find("SERVER=[") != std::string::npos)
            {
                for (size_t i = 0; i < line.length(); i++)
                {
                    if (line[i] == '[')
                        bracketCount++;
                }
                
                if (bracketCount > 1)
                    throw std::runtime_error("Multiple opening brackets '[' found");
                
                foundOpeningBracket = true;
                allLines += line + '\n';
            }
            continue;
        }
        if (foundOpeningBracket)
        {
            lineNumber++;
            for (size_t i = 0; i < line.length(); i++)
            {
                if (line[i] == '[')
                    bracketCount++;
                else if (line[i] == ']')
                    bracketCount--;
            }
            if (bracketCount < 0)
                throw std::runtime_error("Unexpected closing bracket ']'");
            if (bracketCount > 1)
                throw std::runtime_error("Multiple opening brackets '[' found");
            
            if (line.find(']') != std::string::npos)
            {
                foundClosingBracket = true;
                foundAtLeastOneServer = true;
                allLines += line;
                
                Config conf(allLines, lineNumber);
                this->servers.push_back(conf);
                
                allLines.clear();
                foundOpeningBracket = false;
                foundClosingBracket = false;
                bracketCount = 0;
                lineNumber = -1;
            }
            else
                allLines += line + '\n';
        }
    }
    if (!foundAtLeastOneServer)
        throw std::runtime_error("Missing 'SERVER = [' opening bracket");
    if (foundOpeningBracket && !foundClosingBracket)
        throw std::runtime_error("Missing closing bracket ']'");
    for (size_t i = 0; i < servers.size(); i++)
        if (servers[i].ports.empty() || servers[i].host.empty())
            throw std::runtime_error("Server should at least have host and port");
        
}

void Servers::checkServers()
{
    for (size_t it = 0; it < servers.size(); it++)
    {
        for (size_t it2 = it + 1; it2 < servers.size(); it2++)
        {
            if ((servers[it].ports == servers[it2].ports) && (servers[it].host == servers[it2].host))
                throw std::runtime_error("Config file : Address already is used");
        }

        Config &srv = servers[it];
        if (srv.allowed_methods.empty())
            throw std::runtime_error("Invalid server: at least one allowed HTTP method is required");
        if (srv.max_body_size <= 0)
            throw std::runtime_error("Invalid server: max_body_size directive is required");
        if (srv.error_pages.empty())
            throw std::runtime_error("Invalid server: error_pages directive is required");
        
        int required_codes[] = {200, 204, 400, 401, 403, 404, 405, 408, 409, 413, 414, 415, 500, 502, 504};
        int required_count = sizeof(required_codes) / sizeof(required_codes[0]);
        for (int i = 0; i < required_count; i++)
        {
            if (srv.error_pages.find(required_codes[i]) == srv.error_pages.end())
            {
                std::stringstream ss;
                ss << "Missing required error page for code: " << required_codes[i];
                throw std::runtime_error(ss.str());
            }

            std::string error_page_path = srv.error_pages[required_codes[i]];
            std::ifstream file(error_page_path.c_str());
            if (!file.good())
            {
                std::stringstream ss;
                ss << "Error page file does not exist: " << error_page_path 
                << " (for code: " << required_codes[i] << ")";
                throw std::runtime_error(ss.str());
            }
            file.close();
        }
        
        for (size_t iRoute = 0; iRoute < srv.routes.size(); iRoute++)
        {
            Route &rt = srv.routes[iRoute];
            if (rt.allowed_methods.empty())
                throw std::runtime_error("Invalid route: at least one allowed HTTP method is required");
            if (rt.rootPath.empty())
                throw std::runtime_error("Invalid route: root path directive is required");
            if (std::find(rt.allowed_methods.begin(), rt.allowed_methods.end(), "POST") != rt.allowed_methods.end() && rt.upload_dir.empty())
                throw std::runtime_error("Invalid route: POST method allowed but upload directory is not defined");
        }
    }
    
}
std::vector<std::string> ft_Csplite(std::string &value, char c)
{
    std::stringstream ss(value);
    std::string word;
    std::vector<std::string> params;
    while (std::getline(ss, word, c))
    {
        word = ft_trim(word);
        params.push_back(word);
    }
    return params;
}

Config::Config(): max_body_size(-1)
{
}

Config::Config(std::string &lines, int numLines): max_body_size(-1)
{
    std::stringstream ss(lines);
    std::string line;
    std::string key;
    std::string value;
    getline(ss, line);
    
    for (int i = 0; i < numLines; i++)
    {
        getline(ss, line);
        if (line.empty())
            continue;
        if (line.find('=') == std::string::npos)
            throw std::runtime_error("Line missing '=' separator");

        std::stringstream line_ss(line);
        if (std::getline(line_ss, key, '=') && std::getline(line_ss, value))
        {
            key = ft_trim(key);
            value = ft_trim(value);
            
            if (!value.empty() && value[value.length() - 1] == ';')
                value = value.substr(0, value.length() - 1);
            else
                throw std::runtime_error("Missing ';' at the end or Value is empty");
            std::pair<std::string, std::string> configPair = std::make_pair(key, value);
            if (key == "port")
                insertPorts(configPair.second);
            else if (key == "host")
                insertHost(configPair.second);
            else if (key == "allowed_methods")
                insertAllwedMethods(configPair.second);
            else if (key == "max_body_size")
                insertBodySize(configPair.second);
            else if (key == "server_names")
                insertServerNames(configPair.second);
            else if (key == "error_pages")
                insertErrorPages(configPair.second);
            else if (key == "route")
                insertRoutes(line);
            else
                throw std::runtime_error("invalid configuration key");
        }
    }
}

void Config::insertPorts(std::string &ports)
{
    std::vector<std::string> pts = ft_Csplite(ports, ',');
    for (size_t i = 0; i < pts.size(); i++)
    {
        if (!isNumber(pts[i].c_str()))
            throw std::runtime_error("Port should be digits");
        double numprt = std::atof(pts[i].c_str());
        if (numprt > 65535 || numprt < 1024)
            throw std::runtime_error("Port is out of range (1024 - 65535)");
        this->ports.push_back(static_cast<int>(numprt));
    }
}

void Config::insertHost(std::string &host)
{
    std::vector<std::string> hst = ft_Csplite(host, '.');
    if (hst.size() != 4)
        throw std::runtime_error("host not valid");
    for (size_t i = 0; i < hst.size(); i++)
    {
        if (!isNumber(hst[i]) || hst[i].length() > 3)
            throw std::runtime_error("host not valid");
        int num = std::atoi(hst[i].c_str());
        if (num > 255 || num < 0)
            throw std::runtime_error("host not valid");
    }
    this->host = ft_trim(host);
}
void Config::insertAllwedMethods(std::string &allowedMethods)
{
    std::vector<std::string> almds = ft_Csplite(allowedMethods, ',');
    for (size_t i = 0; i < almds.size(); i++)
    {
        if (almds[i] != "GET" && almds[i] != "POST" && almds[i] != "DELETE")
            throw std::runtime_error("Allowed Methods not valid");
    }
    this->allowed_methods = almds;
}
void Config::insertBodySize(std::string &bodySise)
{
    bodySise = ft_trim(bodySise);
    if (!isNumber(bodySise) || bodySise.length() > 10)
        throw std::runtime_error("body size not valid");    
    this->max_body_size = std::atoi(bodySise.c_str());
}
void Config::insertServerNames(std::string &serverName)
{
    std::vector<std::string> srv = ft_Csplite(serverName, ',');
    for (size_t i = 0; i < srv.size(); i++)
    {
        std::vector<std::string> tmp = ft_Csplite(srv[i], '.');
        if (tmp.size() != 3 || tmp[0] != "www")
            throw std::runtime_error("Server Name not valid");
    }
    this->server_names = srv;
}
void Config::insertErrorPages(std::string &errorPage)
{
    std::vector<std::string> errs = ft_Csplite(errorPage, ',');
    for (size_t i = 0; i < errs.size(); i++)
    {
        std::vector<std::string> tmp = ft_Csplite(errs[i], ':');
        if (tmp.size() != 2 || !isNumber(tmp[0]) || tmp[0].length() != 3)
            throw std::runtime_error("Error pages not valid");
        int code = std::atoi(tmp[0].c_str());
        std::string Cpath = tmp[1];
        this->error_pages.insert(std::make_pair(code, Cpath));
    }
}
void Config::insertRoutes(std::string &routes)
{
    routes = ft_trim(routes);
    if (routes[routes.length() - 1] == ';')
        routes = routes.substr(0, routes.length() - 1);
    Route rt(routes);
    this->routes.push_back(rt);
}
