#ifndef PREPARECGIRES_HPP
#define PREPARECGIRES_HPP
#include "cgi.hpp"


class PrepareCgiRes
{
private:
    std::string _body;
    std::string _headers;
public:
    PrepareCgiRes();
    ~PrepareCgiRes();
};


#endif