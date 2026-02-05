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
#include <time.h>
#include <cerrno>


const int MAX_EVENTS = 64;

#define TIME_OUT_CLIENTS 5000

class clsServer
{
private:

    char buffer[4096];
    int client_fd;
    int fd;
    std::map<int, int> clientToServer;
    std::map<int, time_t> mapCheckTimeOut;
    std::map<int ,MySpace::BufferRequest> mapBuffers;
    struct epoll_event events[MAX_EVENTS];
    std::map<int, Config> mapServers;
    std::map<int, uint32_t> fdEventMask; 
    int epoll_fd;;

    void _RegisterFdOnEpoll(int fd, uint32_t events);
    bool _AcceptNewClient();
    void _removeFd(int fd);
    void _InitServers(std::vector<Config> Servers);
    int _ServerSetup(Config &Servers);
    int _CreateSocket();
    int _BindSocket(int server_fd,Config server);
    int _StartListeningSocket(int server_fd);
    void _NonBlockingSocket(int server_fd);
    void _MakeRespenseError(int error,MySpace::BufferRequest &Buffer,int fd_Client,Config ConfigServer);
   void _MakeRespenseCorrect(int fd_Client,MySpace::BufferRequest &Buffer,Config ConfigServer);
    void _Enable_epollout(int fd);
    void _CleanUpClientFd(int client_fd);
    bool _IsIconRequestSkip(std::string Request);
    void _ProcessEpollOutRequestStatus();
    void _ProcessEpollinRequestStatus();

    void CheckTimeOutClients();
     void _HandleTimeOutforNoCGI(int client_fd);
     void _HandleTimeOutforCGI(int client_fd);
    
public:
    
    clsServer(std::vector<Config> Servers);
    void Run();
    ~clsServer();

};





# endif 