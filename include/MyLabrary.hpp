#ifndef MYLABRARY_HPP
#define MYLABRARY_HPP

#include <map>
#include "Route.hpp"

#define HTTP_BAD_REQUEST            400  // Invalid request syntax
#define HTTP_UNAUTHORIZED           401  // Authentication required
#define HTTP_FORBIDDEN              403  // Access denied
#define HTTP_NOT_FOUND              404  // Resource not found
#define HTTP_METHOD_NOT_ALLOWED     405  // Method not allowed for this resource
#define HTTP_REQUEST_TIMEOUT        408  // Client took too long
#define HTTP_CONFLICT               409  // Conflict with server state
#define HTTP_PAYLOAD_TOO_LARGE      413  // Body size exceeds limit
#define HTTP_URI_TOO_LONG           414  // URI too long
#define HTTP_UNSUPPORTED_MEDIA_TYPE 415  // Content-Type not supported
#define HTTP_SUCCESS                200 
#define HTTP_NO_CONTENT             204
#define HTTP_INTERNAL_SERVER_ERROR  500
#define HTTP_TIME_OUT_CGI          504
#define HTTP_TIME_OUT              408

#include <istream>
#include <iomanip>
# include <vector>
#include <sys/stat.h>

namespace MySpace
{

   extern std::string _Method[4];

    enum eState 
    {
        PARSE_NO_START,
        PARSE_REQUEST_LINE,
        PARSE_HEADERS,
        PARSE_BODY,
        REQUEST_DONE
    };

    enum eType
    {
        GET,
        DELETE,
        POSTE,
        UNKNOWN
    };

    struct DataRequestForGetDelete
    {
        std::string method;                  // GET, DELETE, POST
        bool isRequestForCGI;
        std::string target;                  // /index.html, /item/5
        std::string queryString;              // key1=value1&key2=value2
        std::string version;
        std::string TargetCGI;
        Route route;
      
        std::map<std::string, std::string> headers;  // key: value

    };

    struct sData
    {
        //response header
        int status;
        std::string connection;
        std::vector<std::string> setHeaderCookie;
        std::string Content_Type;
        std::string _headers;


        bool isRouting;
        MySpace::eState sState;
        bool isMultipart;
        bool parsingLineAndHeader;
        size_t ContentLength;
        bool CreateEnv;
        size_t ofset;
        size_t bodyBytesProcessed;
        int fd;
        
        std::map<std::string, std::string> cookieValues;
        bool isCookie;
        bool isComplete;
        bool isRedirection;
        bool isAutoIndex;
        std::vector<std::string> autoIndexList;
        int nbrRedirects;
        MySpace::DataRequestForGetDelete RequestAtEnd;
        std::string Buffer;
        bool isSendHeader;
        
        bool eraseHeadersDone;
        std::string boundary;
        bool forked;
        bool finishExc;
        pid_t _pid;
        int pipe_in_fd;
        int pipe_out_fd;
        std::string boundaryEnd;
        bool isfileOpen;
        
    };
    

    struct BufferRequest
    {
        MySpace::eType type;
        sData BufferRead;
        sData BufferWrite;
    };   

    size_t getPipeSize(int pipeFd);

    std::vector<std::string> getAutoIndexList(const std::string& path);
    std::string buildAutoIndexPage(const std::vector<std::string>& fileList, const std::string& path,std::string NameRoute);

     void EraseHearse(MySpace::BufferRequest& _Buffer);

    std::string extractBoundary(const std::string& contentType);

    bool CheckIsCGI(MySpace::BufferRequest _Buffer);
    
    MySpace::BufferRequest trimBoundaryFromBuffer(size_t& dataSize, MySpace::BufferRequest buffer);
    
    bool isMultipartRequest(const std::string& ContentType);

    std::string getStatusReason(int statusCode);
    bool IS_CGI(const std::string& target);
    std::string getContentType(const std::string& path);
    
    size_t      getFileSize(const std::string& path);

    std::string GetContentType(std::string& Buffer);

    bool endsWith(const std::string& str, const std::string& suffix);

    std::string getFirstWord(const std::string& s);

    std::vector<std::string> _Split(const std::string& str, const std::string& delimiter);

    std::vector<std::string> _Split(const std::string& str, char delimiter);

    eType _GetTypeRequest(std::string str);

    std::string generateUniqueFilename(const std::string& contentType);

    bool IS_CGI(const std::string& target);
    
    std::map<std::string, std::string> parseCookies(std::string cookiesHeader);

};


#endif