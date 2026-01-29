#include "../include/clsPostBodyFileHandler.hpp"


void clsPostBodyFileHandler::sendRequestToTarget(std::string path)
{
    if (_Buffer.BufferRead.isRouting == false)
        return;
    if (!_Buffer.BufferRead.isfileOpen) 
    {
        _Buffer.BufferRead.fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (_Buffer.BufferRead.fd < 0)
        {

            std::cout << "Failed to open file for upload: " <<_Buffer.BufferRead.fd << path  << std::endl;
            _Buffer.BufferRead.isComplete = true;
            return;
        }
        
        _Buffer.BufferRead.isfileOpen = true;
    }
    
    while(!_Buffer.BufferRead.Buffer.empty()) 
    {
        size_t originalSize = _Buffer.BufferRead.Buffer.size();
        size_t dataSize = originalSize;
        size_t trimmedAway = 0;

        if (_Buffer.BufferRead.isMultipart)
        {
            _Buffer = MySpace::trimBoundaryFromBuffer(dataSize, _Buffer);
            size_t afterTrimSize = _Buffer.BufferRead.Buffer.size();
            if (originalSize >= afterTrimSize)
                trimmedAway = originalSize - afterTrimSize;
        }

        if (dataSize == 0)
        {
            if (trimmedAway > 0)
                _Buffer.BufferRead.bodyBytesProcessed += trimmedAway;
            break;
        }

        const char* data = _Buffer.BufferRead.Buffer.data();
        ssize_t written = write(_Buffer.BufferRead.fd, data, dataSize);
        if (written < 0) 
        {   
            close(_Buffer.BufferRead.fd);
            _Buffer.BufferRead.isComplete = true;
            return;
        }
        _Buffer.BufferRead.ofset += written;
        _Buffer.BufferRead.bodyBytesProcessed += (trimmedAway + written);
        _Buffer.BufferRead.Buffer.erase(0, written);
    }

    if (_Buffer.BufferRead.isMultipart && !_Buffer.BufferRead.isComplete && _Buffer.BufferRead.ContentLength > 0)
    {
        size_t pending = _Buffer.BufferRead.Buffer.size();
        if (_Buffer.BufferRead.bodyBytesProcessed + pending >= _Buffer.BufferRead.ContentLength)
        {
            _Buffer.BufferRead.bodyBytesProcessed += pending;
            _Buffer.BufferRead.Buffer.clear();
            _Buffer.BufferRead.isComplete = true;
        }
    }
    
    if ((_Buffer.BufferRead.ContentLength > 0 && _Buffer.BufferRead.bodyBytesProcessed >= _Buffer.BufferRead.ContentLength) || _Buffer.BufferRead.isComplete)
    {
        close(_Buffer.BufferRead.fd);
        _Buffer.BufferRead.isComplete = true;
    }
}

void clsPostBodyFileHandler::EraseHeadersOnly()
{
    if (_Buffer.BufferRead.eraseHeadersDone)
        return;
    size_t pos = _Buffer.BufferRead.Buffer.find("\r\n\r\n");
    if (pos != std::string::npos)
    {

        _Buffer.BufferRead.Buffer.erase(0, pos + 4);
        _Buffer.BufferRead.eraseHeadersDone = true;
    }
}

void clsPostBodyFileHandler::EraseHeadersAndFirstBoundarie()
{
    if (_Buffer.BufferRead.eraseHeadersDone)
        return;
    size_t pos = _Buffer.BufferRead.Buffer.find("\r\n\r\n");
    if (pos != std::string::npos)
    {
        _Buffer.BufferRead.Content_Type = MySpace::GetContentType(_Buffer.BufferRead.Buffer);
        _Buffer.BufferRead.Buffer.erase(0, pos + 4);
        _Buffer.BufferRead.eraseHeadersDone = true;
    }
}

MySpace::BufferRequest clsPostBodyFileHandler::StreamToFileWriter()
{

    if (_Buffer.BufferRead.isComplete)
        return _Buffer;
    if (_Buffer.BufferRead.isMultipart)
        EraseHeadersAndFirstBoundarie();
    if (_Buffer.BufferRead.RequestAtEnd.isRequestForCGI)    
        sendRequestToTarget(_Buffer.BufferRead.RequestAtEnd.TargetCGI);
    else
        sendRequestToTarget(_Buffer.BufferRead.RequestAtEnd.target);
    return _Buffer;
}

clsPostBodyFileHandler::clsPostBodyFileHandler(MySpace::BufferRequest& Buffer) : _Buffer(Buffer)
{

}

clsPostBodyFileHandler::~clsPostBodyFileHandler()
{

}
