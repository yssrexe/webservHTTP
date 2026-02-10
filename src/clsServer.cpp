#include "../include/clsServer.hpp"


int clsServer::_CreateSocket()
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

int clsServer::_BindSocket(int server_fd,Config server)
{
    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_port = htons(server.ports[0]);
    if (inet_aton(server.host.c_str(), &address.sin_addr) == 0) {
        close(server_fd);
        throw std::runtime_error("invalid IP address");
    }

    if (bind(server_fd, (sockaddr *)&address, sizeof(address)) < 0)
    {
        close(server_fd);
        throw std::runtime_error("bind");
    }
    return (server_fd);
}

int clsServer::_StartListeningSocket(int server_fd)
{
    if (listen(server_fd, 50) < 0)
    {
        close(server_fd);
        throw std::runtime_error("listen");
    }
    return (server_fd);
}

void clsServer::_NonBlockingSocket(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
        throw std::runtime_error("fcntl");
}

void clsServer::_RegisterFdOnEpoll(int fd, uint32_t events)
{
     struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0)
        perror("epoll_ctl ADD server");
}

int clsServer::_ServerSetup(Config &Servers)
{
    int server_fd;

    server_fd = _CreateSocket();
    server_fd = _BindSocket(server_fd,Servers);
    server_fd = _StartListeningSocket(server_fd);
    _NonBlockingSocket(server_fd);
    return (server_fd);
}

void  clsServer::_InitServers(std::vector<Config> Servers)
{
    for (size_t i = 0; i < Servers.size(); ++i)
    {
        try
        {
            int key = _ServerSetup(Servers[i]);
            mapServers.insert(std::make_pair(key, Servers[i]));
            _RegisterFdOnEpoll(key,EPOLLIN);
           
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
    _InitServers(Servers);
}

void clsServer::_MakeRespenseError(int error,MySpace::BufferRequest &Buffer,int fd_Client,Config ConfigServer)
{
    clsResponse ErrorResponse(error,Buffer,fd_Client,ConfigServer);
    ErrorResponse.SendResponse();
}

void clsServer::_MakeRespenseCorrect(int fd_Client,MySpace::BufferRequest &Buffer,Config ConfigServer)
{
    clsResponse Response(fd_Client,Buffer,ConfigServer);
    Response.SendResponse();
}

void clsServer::_Enable_epollout(int fd)
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

void clsServer::_CleanUpClientFd(int client_fd)
{
   
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
    close(client_fd);

    if (mapBuffers.count(client_fd))
        mapBuffers.erase(client_fd);
    if (clientToServer.count(client_fd))
        clientToServer.erase(client_fd);
    if (fdEventMask.count(client_fd))
        fdEventMask.erase(client_fd);
}
 
bool clsServer::_IsIconRequestSkip(const std::string Request)
{
    size_t endLine = Request.find("\r\n");
    if (endLine == std::string::npos)
        return false;

    std::string firstLine = Request.substr(0, endLine);
    std::vector<std::string> vLine = MySpace::_Split(firstLine, ' ');

    if (vLine.size() < 2)
        return false;

    if (vLine[1] == "/favicon.ico")
    {
        mapCheckTimeOut.erase(fd);
        return (true);
    }
    return (false);
}

MySpace::sData _InitBuffer()
{
    MySpace::sData Data;


    Data.ContentLength = 0;
    Data.Content_Type = "text/html";
    Data.connection = "close";
    Data.status = 200;
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
    Data.CreateEnv = false;
    Data.forked = false;
    Data.isRedirection = false;
    Data.finishExc = false;
    Data.bodyBytesProcessed = 0;
    Data.RequestAtEnd.isRequestForCGI = false;
    Data.isAutoIndex = false;
    return Data;
}

MySpace::BufferRequest _InitBuffersWithFlags()
{
    MySpace::BufferRequest Buffer;

    Buffer.BufferRead = _InitBuffer();
    Buffer.BufferWrite = _InitBuffer();
    Buffer.type = MySpace::UNKNOWN ;

    return Buffer;
}

bool clsServer::_AcceptNewClient()
{
    client_fd = accept(fd, NULL, NULL);
    if (client_fd < 0)
        return false;
    mapCheckTimeOut[client_fd] = time(NULL);
    _NonBlockingSocket(client_fd);
    _RegisterFdOnEpoll(client_fd,EPOLLIN);
    clientToServer[client_fd] = fd;
    return true;
}

void clsServer::_ProcessEpollOutRequestStatus()
{
    try
    {
        _MakeRespenseCorrect(fd,mapBuffers[fd],mapServers[clientToServer[fd]]);
    }
    catch(int StatusCode)
    {
        _MakeRespenseError(StatusCode,mapBuffers[fd],fd,mapServers[clientToServer[fd]]);
        _CleanUpClientFd(fd);
        return;
    }
    if(mapBuffers[fd].BufferWrite.isComplete)
    {
        _CleanUpClientFd(fd);
    }
}

void clsServer::_ProcessEpollinRequestStatus()
{
    if (mapBuffers[fd].BufferRead.isComplete == true && mapBuffers[fd].BufferRead.isRouting == true)
        return;
    try
    {
        clsRequest Request(mapBuffers[fd]);
        mapBuffers[fd] = Request.ParsingRequest();

        clsRounting Rounting(mapBuffers[fd],mapServers[clientToServer[fd]]);
        mapBuffers[fd] = Rounting.CheckRounting();
        
        if (mapBuffers[fd].type == MySpace::POSTE)
        {
            clsPostBodyFileHandler clsPostBodyFileHandler(mapBuffers[fd]);
            mapBuffers[fd] = clsPostBodyFileHandler.StreamToFileWriter();
            if (mapBuffers[fd].BufferRead.isComplete)
                _Enable_epollout(fd);
        }
        
        
    }
    catch(int StatusCode)
    {
        _MakeRespenseError(StatusCode,mapBuffers[fd],fd,mapServers[clientToServer[fd]]);
        _CleanUpClientFd(fd);
    }
}

void clsServer::deletedExpiredClients()
{
    for (size_t i = 0; i < fdExpired.size(); ++i)
    {
        int client_fd = fdExpired[i];
        if (mapCheckTimeOut.count(client_fd))
            mapCheckTimeOut.erase(client_fd);
    }
    fdExpired.clear();
}

void clsServer::_HandleTimeOutforCGI(int client_fd)
{
    if (kill(mapBuffers[client_fd].BufferRead._pid, SIGTERM) == -1)
    {
        std::cerr << "Failed to kill CGI process with PID: " << mapBuffers[client_fd].BufferRead._pid << std::endl;
    }
    int status;
    waitpid(mapBuffers[client_fd].BufferRead._pid, &status, 0);
    clsResponse ErrorResponse(HTTP_TIME_OUT_CGI, mapBuffers[client_fd], client_fd, mapServers[clientToServer[client_fd]]);
    ErrorResponse.SendResponse();
    close(mapBuffers[client_fd].BufferRead.pipe_in_fd);
    close(mapBuffers[client_fd].BufferRead.pipe_out_fd);
    _CleanUpClientFd(client_fd);
}
    
void clsServer::_HandleTimeOutforNoCGI(int client_fd)
{
    if (!clientToServer.count(client_fd))
        return;
    MySpace::BufferRequest buffer = _InitBuffersWithFlags();
    clsResponse ErrorResponse(HTTP_TIME_OUT, buffer, client_fd, mapServers[clientToServer[client_fd]]);
    ErrorResponse.SendResponse();
    _CleanUpClientFd(client_fd);
}

void clsServer::CheckTimeOutClients()
{
    for (std::map<int ,time_t>::iterator it = mapCheckTimeOut.begin(); it != mapCheckTimeOut.end();++it)
    {
        int client_fd = it->first;
        time_t currentTime = time(NULL);
        double elapsedMilliseconds =  (currentTime -  it->second) * 1000.0;
        if (elapsedMilliseconds >= TIME_OUT_CLIENTS)
        {
            if (mapBuffers.count(client_fd) && mapBuffers[client_fd].BufferRead.RequestAtEnd.isRequestForCGI == true)
            {
                _HandleTimeOutforCGI(client_fd);
            }
            else if (mapBuffers.count(client_fd) == 0)
            {
                _HandleTimeOutforNoCGI(client_fd);
            }
          
        }
    }
    deletedExpiredClients();
}


void  clsServer::Run()
{
    signal(SIGPIPE, SIG_IGN);
    while (1)
    {
        int ready = epoll_wait(epoll_fd, events, MAX_EVENTS, TIME_OUT_CLIENTS);
        if (ready < 0)
            continue;

        for (int i = 0; i < ready; ++i)
        {
            label:
            fd = events[i].data.fd;
            if (mapServers.count(fd))
            {
                if(_AcceptNewClient() == false) 
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
                        _CleanUpClientFd(fd);
                        continue;
                    }
                    std::string chunk(buffer, bytes);
                    if (_IsIconRequestSkip(chunk))
                        goto label;

                    if (!mapBuffers.count(fd))
                    {
                        mapBuffers[fd] = _InitBuffersWithFlags();
                    }
                    mapBuffers[fd].BufferRead.Buffer.append(chunk);

                    _ProcessEpollinRequestStatus();
                    if (mapBuffers[fd].BufferRead.isComplete == true  )
                    {
                        _Enable_epollout(fd);
                    }
                }

                if (mapCheckTimeOut.count(fd) && mapBuffers[fd].BufferRead.RequestAtEnd.isRequestForCGI == false)
                {
                   mapCheckTimeOut.erase(fd);
                }
                if (events[i].events & EPOLLOUT)
                {
                    _ProcessEpollOutRequestStatus();
                }
                
            }
        
        }
        if (!mapCheckTimeOut.empty())
            CheckTimeOutClients();
    }
} 
