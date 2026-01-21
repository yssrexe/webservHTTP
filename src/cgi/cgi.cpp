#include "../../include/Config.hpp"
#include "../../include/Route.hpp"
#include "../../include/cgi.hpp"

void initializeRequest(Cgi& request)
{
    // POST method
    request.setMethod("POST");
    request.setTarget("/cgi/user.php");
    request.setQuerys("");
    request.setVersion("HTTP/1.1");
    
    request.addHeader("Host", "127.0.0.1:7331");
    request.addHeader("User-Agent", "curl/8.5.0");
    request.addHeader("Accept", "*/*");
    request.addHeader("Content-Type", "application/json");
    request.addHeader("Content-Length", "30");
    
    request.setBodyFilePath("www/html/cgi/body_data.txt");
}

// int main()
// {
//     Cgi request;
    
//     ssize_t result;
//     try
//     {
//         initializeRequest(request);
//         Route route("route = /cgi : ROOT=www/html/cgi, DEFAULT_FILE=cgi.html, ALLOWED_METHODS=GET-DELETE, UPLOAD_DIR=www/html/uploads/, CGI_EXTENTION =.php - .sh - .py");
        
//         // Validate if the request is a valid CGI request
//         std::string errorMessage;
//         if (!request.isCgiRequest(route, errorMessage))
//         {
//             std::cerr << "CGI Validation Failed: " << errorMessage << std::endl;
//             return 1;
//         }
        
//         std::cout << "CGI Validation Passed!" << std::endl;
        
//         request.handleCgiRequest(route);
//     }
//     catch(const std::exception& e)
//     {
//         std::cerr << "error : " << e.what() << '\n';
//     }
//     close(1);
//     return 0;
// }
