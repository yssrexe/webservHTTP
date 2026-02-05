#ifndef CLSREQUEST_HPP
#define CLSREQUEST_HPP

#include <istream>
#include <iomanip>
# include <vector>
#include "MyLabrary.hpp"
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include "cgi.hpp"

#define Key 0
#define Value 1



static const size_t MAX_URI = 8192;


class clsRequest
{

private:

    MySpace::BufferRequest _BufferRequest;

    void _parsingMethod(std::string Method);
    void _parsingTarget(std::string target);
    void _ParsingVersion(std::string virsion);
    void _parsingHeader(std::string header);
    void _ParsingLine(std::string Line);
    void ParsingGetMethod();
    void ParsingDeleteRequest();
    void ParsingPosteRequest();
    bool openedTarget();
    void sendRequestToTarget(std::string path);

public:

    clsRequest(MySpace::BufferRequest& BufferRequest);
    MySpace::BufferRequest ParsingRequest();

    
    ~clsRequest();
};



# endif

