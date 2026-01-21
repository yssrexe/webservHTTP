#ifndef CLSPOSTBODYFILEHANDLER_HPP
# define CLSPOSTBODYFILEHANDLER_HPP

#include <istream>
#include<iostream>
#include <istream>
#include <iomanip>
# include <vector>
#include "MyLabrary.hpp"
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>


class clsPostBodyFileHandler
{
private:

    MySpace::BufferRequest& _Buffer;
    
public:
    clsPostBodyFileHandler(MySpace::BufferRequest& Buffer);
    MySpace::BufferRequest StreamToFileWriter();
    void sendRequestToTarget(std::string path);
    void EraseHeadersOnly();
    void EraseHeadersAndFirstBoundarie();
    ~clsPostBodyFileHandler();
};


#endif