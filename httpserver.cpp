#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include "httpserver.h"
#include "tcpstream.h"

HttpServer::HttpServer(int port) : _listenSocket(0), _port(port), _listening(false) {}
HttpServer::~HttpServer() {
    if (_listenSocket > 0) {
        close(_listenSocket);
    }
}

int HttpServer::start()
{
    if (_listening) {
        return 0;
    }
    HttpServer::initServerSocket();
    while (1) {
        int conn = acceptConnection();
        printf("Connection on: %d\n", conn);
        TCPStream* stream = new TCPStream(conn);
        processConnection(stream);
    }
}

int HttpServer::registerHandler(HttpMethod method, std::string path, RequestHandler* handler)
{
    handlers.insert(std::make_pair(std::make_tuple(method, path), handler));
}

int HttpServer::initServerSocket()
{
    // Create a tcp listening socket
    _listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(_listenSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt); 

    // Bind to local addr and port
    int r = ::bind(_listenSocket, (struct sockaddr*)&addr, sizeof(addr));
    if (r != 0) {
        perror("bind() failed");
        return r;
    }

    // Start listening for incoming connections
    r = listen(_listenSocket, SOMAXCONN);
    if (r != 0) {
        perror("listen() failed");
        return r;
    }
    _listening = true;
    return r;
}

// Returns the newly accepted connection socket descriptor
int HttpServer::acceptConnection()
{
    if (_listening == false) {
        return -1;
    }

    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    memset(&addr, 0, sizeof(addr));
    int sd = ::accept(_listenSocket, (struct sockaddr*) &addr, &len);
    if (sd < 0) {
        perror("accept() failed");
        return -1;
    }
    return sd;
}

int HttpServer::processConnection(TCPStream* stream)
{
    char reqBuf[BUFFERSIZE];
    memset(reqBuf, 0, BUFFERSIZE);
    while (1) {
        HttpRequest req;
        HttpResponse res;

        int numRead = stream->receive(reqBuf, BUFFERSIZE);
        if (numRead == 0) {
            printf("Connection closed by remote peer\n");
            close(stream->_conn);
            free(stream);  
            return 0;
        }

        //TODO: Fix this weirdness
        int pr = httpParseRequest((char*)std::string(reqBuf).c_str(), std::string(reqBuf).length(), &req); // Pray req is less than BUFFERSIZE

        char tempbuf[BUFFERSIZE];
        memset(tempbuf, 0, BUFFERSIZE);
        memcpy(tempbuf, reqBuf + pr, BUFFERSIZE - pr);
        memset(reqBuf, 0, BUFFERSIZE);
        memcpy(reqBuf, tempbuf, BUFFERSIZE);

        httpMakeResponse(&res);
        routeRequest(&req, &res); 

        std::string responseString = httpSerialiseResponse(&res);
        printf("%s \n", responseString.c_str());

        stream->send((char *) responseString.c_str(), responseString.length());
    }
}

int HttpServer::routeRequest(HttpRequest* req, HttpResponse* res)
{
    RequestHandler* handler;
    if (req->method.compare("GET") == 0) {
        if(handlers.find(std::make_tuple(GET, req->path)) != handlers.end()) {
            handler = handlers[std::make_tuple(GET, req->path)];
            (*handler)(req, res);
            return 0;
        } else {
            return -2; // handler not found
        }
    }
    else if (req->method.compare("POST") == 0){
        if(handlers.find(std::make_tuple(POST, req->path)) != handlers.end()) {
            handler = handlers[std::make_tuple(POST, req->path)];
            (*handler)(req, res);
            return 0;
        } else {
            return -2;
        } 
    } else {
        return -1; // method not supported
    }
}