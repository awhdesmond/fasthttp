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
        int conn = HttpServer::acceptConnection();

        printf("Connection on: %d\n", conn);
        TCPStream* stream = new TCPStream(conn);

        char reqBuf[BUFFERSIZE];
        memset(reqBuf, 0, BUFFERSIZE);
        while (1) {
            HttpRequest request;

            int numRead = stream->receive(reqBuf, BUFFERSIZE);
            if (numRead == 0) {
                printf("Connection closed by remote peer\n");
                close(stream->_conn);
                free(stream);  
                break;  
            }
            printf("Received Message: %s\n", reqBuf);
            int pr = httpParseRequest(reqBuf, BUFFERSIZE, &request); // Pray req is less than BUFFERSIZE
            
            // May have multiple HTTP requests due to HTTP/1.1 
            // pipelining within a TCP packet
            // TODO: optimize?
            char tempReqBuf[BUFFERSIZE];
            memset(tempReqBuf, 0, BUFFERSIZE);
            memcpy(tempReqBuf, reqBuf + pr, BUFFERSIZE - pr);
            memset(reqBuf, 0, BUFFERSIZE);
            memcpy(reqBuf, tempReqBuf, BUFFERSIZE - pr);

            // Router route request to handler
            stream->send((char*) std::string("Hello World!").c_str(), std::string("Hello World!").length());
        }
    }
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