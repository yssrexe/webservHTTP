#include "include/Route.hpp"
#include "include/Config.hpp"
#include "include/clsServer.hpp"
#include "include/MyLabrary.hpp"



bool parseConFileName(std::string fileName)
{
    std::vector<std::string> parts = ft_Csplite(fileName, '.');
    if (parts.size() != 2 || parts[0].length() < 1 || parts[1] != "conf")
        return false;
    return true;
}

int main(int argc, char **argv)
{
    try
    {
        std::string filePath = (argc > 1) ? argv[1] : "webserv.conf";
        if (!parseConFileName(filePath) || argc > 2)
            throw std::runtime_error("the Filename not valid or the number of arguments > 2");
        std::fstream file(filePath.c_str());
        if (!file.is_open())
            throw std::runtime_error("path of config file not valid");
        Servers serv(file);
        serv.checkServers();
        //here we gunna start ...
        std::cout << "Config file parsed successfully. Starting server..." << std::endl;
        // serv.servers[0].routes[0].redirect[301] = "https://www.google.com";
        // std::cout <<"output : " << serv.servers[0].routes[1].redirect[301] << std::endl;
        clsServer clsServers(serv.servers);
        clsServers.Run();
    }
    catch(const std::exception& e)
    {
        std::cerr <<  "Error: " << e.what() << '\n';
    }
    return 0;
}
