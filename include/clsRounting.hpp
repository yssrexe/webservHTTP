#ifndef CLSROUNTING_HPP
#define CLSROUNTING_HPP

#include "MyLabrary.hpp"
#include "Config.hpp"
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <istream>
#include <iomanip>
# include <vector>
#include "MyLabrary.hpp"
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <map>


class clsRounting
{
private:
    MySpace::BufferRequest _Buffer;
    Config _server;
    
    bool _isMethodAllowed(std::string& target,const std::vector<std::string> methods);
    const Route* _findRoute(const std::string& target);
    int _fileExists(const std::string& path,const Route* route);
    std::string _buildPath(const Route& route, std::string target);
    bool _CanWrite(const std::string& path);
    bool _CanRead(const std::string& path);
    bool isDirectory(const std::string& path);
    bool isFile(const std::string& path);
    void checkRoutingForPost();
    void sendRequestToTarget(std::string path);
    bool IsNotPostMethod();

    

public:

    MySpace::BufferRequest CheckRounting();
   clsRounting(MySpace::BufferRequest& Buffer,Config server);
    ~clsRounting();

};




#endif