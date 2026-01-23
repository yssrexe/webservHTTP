#ifndef PREPARECGIRES_HPP
#define PREPARECGIRES_HPP
#include "cgi.hpp"
#include <sstream>
#include <ctime>
#include <unistd.h>

class PrepareCgiRes
{
private:
    std::string _body;
    std::string _headers;
public:
    PrepareCgiRes(MySpace::BufferRequest _buffer);
};


#endif
