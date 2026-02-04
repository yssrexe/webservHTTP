#pragma once

#include "Route.hpp"
#include <vector>
#include <map>
#include <fstream> 
#include <iostream> 
#include <string> 

class Route;
class Config;

class Servers
{
public:
    int lineNumber;
    std::vector<Config> servers;
    Servers(std::istream &file);
    void checkServers();
};

class Config
{
public:
    Config();
    Config(std::string &lines, int numLines);
    std::vector<int> ports;
    std::string host;
    std::vector<std::string> allowed_methods;
    long long max_body_size;
    std::vector<std::string> server_names;
    std::map<int, std::string> error_pages;

    std::vector<Route> routes;

    void insertPorts(std::string &ports);
    void insertHost(std::string &host);
    void insertAllwedMethods(std::string &allowedMethods);
    void insertBodySize(std::string &bodySise);
    void insertServerNames(std::string &serverName);
    void insertErrorPages(std::string &errorPage);

     void insertRoutes(std::string &routes);
};

std::vector<std::string> ft_Csplite(std::string &value, char c);