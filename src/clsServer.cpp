#include "../include/clsServer.hpp"


int clsServer::_createSocket()
{
    int server_fd;
    int opt = 1;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        throw std::runtime_error("socket");
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("setsockopt");
    return server_fd;
}

int clsServer::_binding(int server_fd,Config server)
{
    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_port = htons(server.ports[0]);
    if (inet_aton(server.host.c_str(), &address.sin_addr) == 0) {
        close(server_fd);
        throw std::runtime_error("invalid IP address");
    }
    //address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (sockaddr *)&address, sizeof(address)) < 0)
    {
        close(server_fd);
        throw std::runtime_error("bind");
    }
    return (server_fd);
}

int clsServer::_listening(int server_fd)
{
    if (listen(server_fd, 50) < 0)
    {
        close(server_fd);
        throw std::runtime_error("listen");
    }
    return (server_fd);
}

void clsServer::NonBlockingSocket(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
        throw std::runtime_error("fcntl");
}

void clsServer::_addFd(int fd, uint32_t events)
{
     struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0)
        perror("epoll_ctl ADD server");
}

int clsServer::_serverSetup(Config &Servers)
{
    int server_fd;

    server_fd = _createSocket();
    server_fd = _binding(server_fd,Servers);
    server_fd = _listening(server_fd);
    NonBlockingSocket(server_fd);
    return (server_fd);
}

void  clsServer::_initServers(std::vector<Config> Servers)
{
    for (size_t i = 0; i < Servers.size(); ++i)
    {
        try
        {
            int key = _serverSetup(Servers[i]);
            mapServers.insert(std::make_pair(key, Servers[i]));
            _addFd(key,EPOLLIN);
           
        }
        catch (const std::exception& e)
        {
            std::cerr << "Server init failed: " << e.what() << std::endl;
        }
   
    }
}

clsServer::~clsServer()
{

}

clsServer::clsServer(std::vector<Config> Servers)
{
    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0)
        throw std::runtime_error("epoll_create1");
    _initServers(Servers);
}

void clsServer::_RepenseError(int error,MySpace::BufferRequest &Buffer,int fd_Client,Config ConfigServer)
{
    clsResponse ErrorResponse(error,Buffer,fd_Client,ConfigServer);
    ErrorResponse.SendResponse();
}

void clsServer::_RepenseCorrect(int fd_Client,MySpace::BufferRequest &Buffer,Config ConfigServer)
{
    clsResponse Response(fd_Client,Buffer,ConfigServer);
    Response.SendResponse();
}

void clsServer::disable_epollout(int fd)
{
    uint32_t events = fdEventMask[fd];
    if (!(events & EPOLLOUT)) 
        return;

    events &= ~EPOLLOUT;

    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;

    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
    fdEventMask[fd] = events;
}

void clsServer::enable_epollout(int fd)
{
    uint32_t events = fdEventMask[fd];
    if (events & EPOLLOUT) 
        return;
    events |= EPOLLOUT;
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;

    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
    fdEventMask[fd] = events;
}

bool clsServer::is_skip(const std::string Request)
{
    size_t endLine = Request.find("\r\n");
    if (endLine == std::string::npos)
        return false;

    std::string firstLine = Request.substr(0, endLine);
    std::vector<std::string> vLine = MySpace::_Split(firstLine, ' ');

    if (vLine.size() < 2)
        return false;

    return (vLine[1] == "/favicon.ico");
}

MySpace::sData initData()
{
    MySpace::sData Data;

    Data.ContentLength = 0;
    Data.isRouting = false;
    Data.fd = -1;
    Data.ofset = 0;
    Data.isComplete = false;
    Data.sState = MySpace::PARSE_NO_START;
    Data.isSendHeader = false;
    Data.isfileOpen = false;
    Data.parsingLineAndHeader = false;
    Data.isComplete = false;
    Data.isMultipart = false;
    Data.eraseHeadersDone = false;
    Data.isRedirection = false;
    return Data;
}

MySpace::BufferRequest initBuffer()
{
    MySpace::BufferRequest Buffer;

    Buffer.BufferRead = initData();
    Buffer.BufferWrite = initData();
    Buffer.type = MySpace::UNKNOWN ;

    return Buffer;
}

bool clsServer::acceptNewClient()
{
    client_fd = accept(fd, NULL, NULL);
    if (client_fd < 0)
        return false;
    NonBlockingSocket(client_fd);
    _addFd(client_fd,EPOLLIN);
    clientToServer[client_fd] = fd;
    return true;
}

void clsServer::processRequestAndRespond()
{
    try
    {
        _RepenseCorrect(fd,mapBuffers[fd],mapServers[clientToServer[fd]]);
    }
    catch(int StatusCode)
    {
        _RepenseError(StatusCode,mapBuffers[fd],fd,mapServers[clientToServer[fd]]);
        close(fd);
        mapBuffers.erase(fd);
        disable_epollout(fd);
    }
    if(mapBuffers[fd].BufferWrite.isComplete)
    {
        disable_epollout(fd);
        close(fd);
        mapBuffers.erase(fd);
    }
}

void clsServer::processEppillin()
{
    if (mapBuffers[fd].BufferRead.isComplete == true && mapBuffers[fd].BufferRead.isRouting == true)
        return;
    try
    {
        clsRequest Request(mapBuffers[fd]);
        mapBuffers[fd] = Request.ParsingRequest();

        clsRounting Rounting(mapBuffers[fd],mapServers[clientToServer[fd]]);
        mapBuffers[fd] = Rounting.CheckRounting();

        if (MySpace::CheckIsCGI(mapBuffers[fd]))
        {
            Cgi cgi(mapBuffers[fd].BufferRead.RequestAtEnd,fd);
            mapBuffers[fd] = cgi.handleCgiRequest(mapBuffers[fd]);
        }
        if (mapBuffers[fd].type == MySpace::POSTE)
        {
            clsPostBodyFileHandler clsPostBodyFileHandler(mapBuffers[fd]);
            mapBuffers[fd] = clsPostBodyFileHandler.StreamToFileWriter();
            if (mapBuffers[fd].BufferRead.isComplete)
                enable_epollout(fd);
        }
        
        
    }
    catch(int StatusCode)
    {
        _RepenseError(StatusCode,mapBuffers[fd],fd,mapServers[clientToServer[fd]]);
        close(fd);
        disable_epollout(fd);
        mapBuffers.erase(fd);
    }
}

void  clsServer::Run()
{
    while (true)
    {
        int ready = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (ready < 0)
        {
            perror("epoll_wait");
            break;
        }
        for (int i = 0; i < ready; ++i)
        {
            label:
            fd = events[i].data.fd;
            if (mapServers.count(fd))
            {
                if(acceptNewClient() == false) 
                    continue;
            }        
            else                  
            {
                if (events[i].events & EPOLLIN)
                {
                    std::memset(buffer, 0, sizeof(buffer));
                    ssize_t bytes = recv(fd, buffer, sizeof(buffer), 0);
                    if (bytes <= 0)
                    {
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                        close(fd);
                        continue;
                    }
                    std::string chunk(buffer, bytes);
                    if (is_skip(chunk))
                        goto label;

                    if (!mapBuffers.count(fd))
                        mapBuffers[fd] = initBuffer();
                    mapBuffers[fd].BufferRead.Buffer.append(chunk);

                    processEppillin();
                    if (mapBuffers[fd].BufferRead.isComplete == true)
                        enable_epollout(fd);
                }
                if (events[i].events & EPOLLOUT)
                    processRequestAndRespond();
                
            }
        
        }
    }
} 
