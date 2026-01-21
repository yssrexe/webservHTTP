#ifndef ISERVER_HPP
# define ISERVER_HPP

#include "Route.hpp"
#include "Config.hpp"
#include "clsResponse.hpp"
#include "clsRounting.hpp"
#include <fcntl.h>
#include <vector>
#include <poll.h>
#include "clsRequest.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <map>
#include <sys/epoll.h>
#include "clsPostBodyFileHandler.hpp"

const int MAX_EVENTS = 64;



class clsServer
{
private:

    char buffer[4096];
    int client_fd;
    int fd;
    std::map<int, int> clientToServer;
    std::map<int ,MySpace::BufferRequest> mapBuffers;
    struct epoll_event events[MAX_EVENTS];
    std::map<int, Config> mapServers;
    std::map<int, uint32_t> fdEventMask; 
    int epoll_fd;;

    void _addFd(int fd, uint32_t events);
    bool acceptNewClient();
    void _removeFd(int fd);
    void _initServers(std::vector<Config> Servers);
    int _serverSetup(Config &Servers);
    int _createSocket();
    int _binding(int server_fd,Config server);
    int _listening(int server_fd);
    void NonBlockingSocket(int server_fd);
    void _RepenseError(int error,MySpace::BufferRequest &Buffer,int fd_Client,Config ConfigServer);
   void _RepenseCorrect(int fd_Client,MySpace::BufferRequest &Buffer,Config ConfigServer);
    void disable_epollout(int fd);
    void enable_epollout(int fd);
    bool is_skip(std::string Request);
    void processRequestAndRespond();
    void processEppillin();
    
public:
    
    clsServer(std::vector<Config> Servers);
    void Run();
    ~clsServer();

};





# endif 