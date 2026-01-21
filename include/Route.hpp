#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <execution>
#include <sys/stat.h>
#include <algorithm>
#include <map>
#include "Config.hpp"

class Route
{
public:
    std::string fullLine;
    std::string name;
    std::string rootPath;
    std::string default_file;
    std::vector<std::string> allowed_methods;
    std::string upload_dir;
    std::vector<std::string> cgi_extention;
    std::string dir_listing;
    std::map<int, std::string> redirect;

    Route() {}
    Route(std::string line);
    void insertName(std::string &name);
    void insertRootPath(std::string &rootPath);
    void insertDefaultFile(std::string &defaultFile);
    void insertAllowedMethods(std::string &allowedMethods);
    void insertUploadDir(std::string &uploadDir);
    void insertCgiExtention(std::string &cgiExtention);
    void insertDirListing(std::string &dirListing);
    void insertRedirect(std::string &redirect);
};

std::string ft_trim(const std::string& s);
bool isNumber(const std::string& s);