# WebServ - HTTP Server Implementation

<div align="center">

![C++](https://img.shields.io/badge/C++-98-blue.svg)
![42 School](https://img.shields.io/badge/42-School-000000?logo=42)
![Linux](https://img.shields.io/badge/Linux-FCC624?logo=linux&logoColor=black)

*A robust HTTP/1.1 server implementation in C++98*

</div>

## 👥 Authors

**Yassir El Yasini** & **Ayoub Bouatrouss**

42 Network - School Project

---

## 📋 Table of Contents

- [About](#about)
- [Features](#features)
- [Architecture](#architecture)
- [Installation](#installation)
- [Configuration](#configuration)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [HTTP Methods](#http-methods)
- [CGI Support](#cgi-support)
- [Technical Details](#technical-details)

---

## 🎯 About

**WebServ** is a high-performance HTTP/1.1 server written in C++98 as part of the 42 school curriculum. This project implements core web server functionalities including request parsing, response generation, CGI execution, file uploads/downloads, and advanced routing capabilities.

The server is designed to handle multiple simultaneous connections efficiently using epoll event notification, making it suitable for real-world applications while maintaining strict C++98 standard compliance.

---

## ✨ Features

### Core Features
- ✅ **HTTP/1.1 Protocol** - Full implementation of HTTP/1.1 standard
- ✅ **Multiple HTTP Methods** - GET, POST, DELETE support
- ✅ **Non-blocking I/O** - Asynchronous event-driven architecture using epoll
- ✅ **Configuration File** - Flexible NGINX-like configuration syntax
- ✅ **Multiple Virtual Servers** - Host multiple websites on different ports
- ✅ **Custom Error Pages** - Personalized error responses (200-504)
- ✅ **Route-Based Handling** - Advanced URL routing system

### Advanced Features
- 🚀 **CGI Support** - Execute dynamic scripts (.py, .php, .sh)
- 📤 **File Upload** - Handle multipart/form-data uploads
- 📥 **File Download** - Serve static files efficiently
- 🗑️ **File Deletion** - DELETE method implementation
- 📂 **Directory Listing** - Auto-index for directories
- 🔄 **HTTP Redirections** - 301/302 redirect support
- ⏱️ **Timeout Handling** - Client timeout management (5s default)
- 📊 **Max Body Size** - Configurable request body size limits
- 🔐 **Method Restrictions** - Per-route HTTP method control

---

## 🏗️ Architecture

### Component Overview

```
┌─────────────────────────────────────────────────┐
│              Main Server (clsServer)            │
│         ┌──────────────────────────┐            │
│         │   Epoll Event Handler    │            │
│         └──────────┬───────────────┘            │
│                    │                             │
│         ┌──────────▼───────────────┐            │
│         │   Connection Manager     │            │
│         └──────────┬───────────────┘            │
│                    │                             │
│    ┌───────────────┼────────────────┐           │
│    │               │                │           │
│    ▼               ▼                ▼           │
│ ┌──────┐    ┌──────────┐    ┌─────────┐       │
│ │Request│───▶│ Routing  │───▶│Response │       │
│ │Parser │    │ Handler  │    │Builder  │       │
│ └──────┘    └─────┬────┘    └─────────┘       │
│                   │                             │
│          ┌────────┴────────┐                    │
│          │                 │                    │
│          ▼                 ▼                    │
│    ┌──────────┐      ┌─────────┐               │
│    │   CGI    │      │  Static │               │
│    │ Handler  │      │  Files  │               │
│    └──────────┘      └─────────┘               │
└─────────────────────────────────────────────────┘
```

### Key Components

| Component | File | Description |
|-----------|------|-------------|
| **Server Core** | `clsServer.cpp/hpp` | Main server loop, epoll management, connection handling |
| **Request Parser** | `clsRequest.cpp/hpp` | HTTP request parsing and validation |
| **Response Builder** | `clsResponse.cpp/hpp` | HTTP response generation |
| **Routing System** | `clsRounting.cpp/hpp` | URL routing and route matching |
| **Config Parser** | `Config.cpp/hpp` | Configuration file parser |
| **CGI Handler** | `cgi.hpp`, `handleCGI.cpp` | CGI script execution and communication |
| **File Handler** | `clsPostBodyFileHandler.cpp/hpp` | POST body and file upload handling |

---

## 🔧 Installation

### Prerequisites
- **Compiler**: g++ or clang++ with C++98 support
- **OS**: Linux (uses epoll)
- **Build Tool**: make

### Build Steps

```bash
# Clone the repository
git clone https://github.com/yssrexe/webservHTTP.git
cd webservHTTP

# Compile the project
make

# The executable 'webserv' will be created
```

### Makefile Commands

```bash
make        # Build the project
make clean  # Remove object files
make fclean # Remove object files and executable
make re     # Rebuild from scratch
```

---

## ⚙️ Configuration

### Configuration File Format

The server uses a custom configuration format similar to NGINX. Default config: `webserv.conf`

```properties
SERVER = [
    port = 8081;
    host = 127.0.0.1;
    allowed_methods = GET, POST, DELETE;
    max_body_size = 10485760;  # 10MB in bytes
    server_names = www.example.com, example.com;

    # Error page mappings
    error_pages = 404:www/html/404.html, 500:www/html/500.html;
    
    # Route definitions
    route = / : ROOT=www/html/view, DEFAULT_FILE=index.html, ALLOWED_METHODS=GET-POST;
    route = /uploads : ROOT=www/html/uploads, ALLOWED_METHODS=GET-POST-DELETE, UPLOAD_DIR=www/html/uploads;
    route = /cgi : ROOT=www/html/cgi, CGI_EXTENTION=.py-.sh-.php, ALLOWED_METHODS=GET-POST;
    route = /auto-index : ROOT=www/html/, DIR_LISTING=on, ALLOWED_METHODS=GET;
]
```

### Configuration Directives

| Directive | Description | Example |
|-----------|-------------|---------|
| `port` | Server listening port | `8081` |
| `host` | Server IP address | `127.0.0.1` |
| `allowed_methods` | Allowed HTTP methods | `GET, POST, DELETE` |
| `max_body_size` | Max request body size (bytes) | `10485760` |
| `server_names` | Virtual host names | `www.example.com` |
| `error_pages` | Custom error pages | `404:path/to/404.html` |
| `route` | Route definition | See route options below |

### Route Options

- **ROOT**: Document root directory
- **DEFAULT_FILE**: Default file to serve (index.html)
- **ALLOWED_METHODS**: Methods allowed for this route
- **UPLOAD_DIR**: Directory for file uploads
- **CGI_EXTENTION**: CGI script extensions
- **DIR_LISTING**: Enable directory listing (on/off)
- **REDIRECT**: HTTP redirection (format: `code-url`)

---

## 🚀 Usage

### Starting the Server

```bash
# Use default configuration (webserv.conf)
./webserv

# Use custom configuration file
./webserv path/to/custom.conf
```

### Testing the Server

```bash
# Test with curl
curl http://localhost:8081/

# Upload a file
curl -X POST -F "file=@myfile.txt" http://localhost:8081/uploads

# Delete a file
curl -X DELETE http://localhost:8081/delete/myfile.txt

# Test CGI
curl http://localhost:8081/cgi/test.py
```

### Browser Access

Open your browser and navigate to:
- `http://localhost:8081/` - Main page
- `http://localhost:8081/auto-index/` - Directory listing
- `http://localhost:8081/uploads/uploadTest.html` - Upload interface

---

## 📁 Project Structure

```
webservHTTP/
├── main.cpp                        # Entry point
├── Makefile                        # Build configuration
├── webserv.conf                    # Server configuration
│
├── include/                        # Header files
│   ├── clsServer.hpp              # Server core
│   ├── clsRequest.hpp             # Request parser
│   ├── clsResponse.hpp            # Response builder
│   ├── clsRounting.hpp            # Routing system
│   ├── Config.hpp                 # Config structures
│   ├── Route.hpp                  # Route structures
│   ├── cgi.hpp                    # CGI handler
│   ├── clsPostBodyFileHandler.hpp # POST body handler
│   ├── PrepareCgiRes.hpp          # CGI response
│   └── MyLabrary.hpp              # Utility functions
│
├── src/                           # Source files
│   ├── clsServer.cpp
│   ├── clsRequest.cpp
│   ├── clsResponse.cpp
│   ├── clsRounting.cpp
│   ├── clsPostBodyFileHandler.cpp
│   ├── MyLabrary.cpp
│   │
│   ├── ConfigParser/              # Configuration parsing
│   │   ├── Config.cpp
│   │   └── Route.cpp
│   │
│   └── cgi/                       # CGI handling
│       ├── handleCGI.cpp
│       └── ParserCgi/
│           └── RequestCgi.cpp
│
└── www/                           # Web content
    └── html/
        ├── *.html                 # Error pages (200-504)
        ├── view/                  # Main website pages
        ├── cgi/                   # CGI scripts
        ├── uploads/               # Upload directory
        ├── auto-index/            # Directory listing template
        └── delete/                # Test files
```

---

## 🌐 HTTP Methods

### GET
- Retrieve resources from the server
- Support for static files and CGI scripts
- Directory listing when enabled
- Query string parameter support

### POST
- File uploads (multipart/form-data)
- Form data submission
- CGI script processing with POST data
- Chunked transfer encoding support

### DELETE
- Remove files from specified directories
- Route-based permission control
- Returns appropriate status codes

---

## 🔌 CGI Support

### Supported CGI Extensions
- **Python** (.py)
- **PHP** (.php)
- **Shell Scripts** (.sh)

### CGI Environment Variables

The server sets standard CGI environment variables:
- `REQUEST_METHOD`
- `CONTENT_TYPE`
- `CONTENT_LENGTH`
- `QUERY_STRING`
- `PATH_INFO`
- `SCRIPT_FILENAME`
- `SERVER_PROTOCOL`
- `GATEWAY_INTERFACE`
- And more...

### CGI Example

```python
#!/usr/bin/env python3

print("Content-Type: text/html\r\n\r\n")
print("<html><body>")
print("<h1>Hello from CGI!</h1>")
print("</body></html>")
```

---

## 🔍 Technical Details

### Event Handling
- **Epoll** - Linux kernel's scalable I/O event notification mechanism
- **Non-blocking sockets** - All sockets set to O_NONBLOCK
- **Edge-triggered mode** - Efficient event handling
- **Max 64 concurrent events** - Configurable MAX_EVENTS

### Request Processing Pipeline

1. **Accept Connection** - New client accepted via epoll
2. **Read Request** - Non-blocking read into buffer
3. **Parse Request** - HTTP request parsing and validation
4. **Route Matching** - Find appropriate route handler
5. **Execute Handler** - Static file, CGI, or upload
6. **Build Response** - Generate HTTP response
7. **Send Response** - Non-blocking write to client
8. **Cleanup** - Close connection or keep-alive

### Timeout Management
- Client timeout: 5000ms (5 seconds)
- Automatic cleanup of inactive connections
- Separate handling for regular and CGI requests

### Error Handling
Comprehensive error page support for HTTP status codes:
- 2xx: Success (200, 204)
- 4xx: Client errors (400, 401, 403, 404, 405, 408, 409, 413, 414, 415)
- 5xx: Server errors (500, 502, 504)

### Memory Management
- RAII principles for resource management
- Proper cleanup in destructors
- Buffer size: 4096 bytes per connection
- No memory leaks (C++98 compliant)

### Standards Compliance
- **C++98 Standard** - Full compliance
- **HTTP/1.1** - RFC 2616 compliant
- **CGI/1.1** - Standard CGI interface
- **POSIX** - Linux system calls

---

## 🧪 Testing

### Test Pages Included
- `index.html` - Main landing page
- `uploadTest.html` - File upload interface
- `deleteTest.html` - File deletion testing
- `cgi.html` - CGI testing interface
- `login.html` / `register.html` - Authentication demos

### Sample CGI Scripts
- `test.py` - Basic CGI test
- `testCgifile.py` - Advanced CGI features
- `login.py` - Login handler
- `register.py` - Registration handler
- `logout.py` - Logout handler

---

## 📝 Notes

- Server runs on Linux only (epoll dependency)
- Requires proper file permissions for CGI scripts (chmod +x)
- Upload directory must have write permissions
- Configuration file must end with `.conf` extension
- Maximum URI length: 8192 bytes

---

## 🤝 Contributing

This project is part of the 42 school curriculum and serves as a learning exercise in:
- Network programming
- HTTP protocol implementation
- Event-driven architecture
- C++98 programming
- Server design patterns

---

## 📄 License

This project is part of the 42 school curriculum.

---

## 🎓 42 School Project

This project is created as part of the common core curriculum at 42 Network schools, focusing on:
- Understanding HTTP protocol at a low level
- Implementing a production-grade web server
- Handling concurrent connections efficiently
- Working with I/O multiplexing (epoll/kqueue/select)
- Parsing and validating HTTP requests
- Serving static and dynamic content
- Managing system resources properly

---

<div align="center">

**Made with ❤️ by Yassir El Yasini & Ayoub Bouatrouss**

*1337(42) School - 2026*

</div>
