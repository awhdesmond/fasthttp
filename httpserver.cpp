#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include "httpserver.h"
#include "connectionthread.h"
#include "vendor/yaml-cpp/yaml.h"
#include "epollqueue.h"

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

    YAML::Node config = YAML::LoadFile(".fasthttp.conf.yaml");
    int numThreads = config["threads"].as<int>();

    EpollQueue* epollqList[numThreads];

    for (int i = 0; i < numThreads; i++) {
        EpollQueue *epollq = new EpollQueue();
        epollqList[i] = epollq;
        ConnectionThread* thread = new ConnectionThread(epollq, &_handlers);
        if (!thread) {
            printf("Could not create ConnectionHandler %d\n", i);
            exit(1);
        } 
        thread->start();
    }

    HttpServer::initServerSocket();
    while (1) {
        int conn = acceptConnection();
        int randThreadIdx = rand() % numThreads;
        epollqList[randThreadIdx]->add(conn);
    }
}

int HttpServer::registerHandler(HttpMethod method, std::string path, RequestHandler* handler)
{
    _handlers.insert(std::make_pair(std::make_tuple(method, path), handler));
    return 0;
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
    int r = ::bind(_listenSocket, (struct sockaddr*)&addr, sizeof addr);
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
    socklen_t len = sizeof addr;
    memset(&addr, 0, sizeof addr);
    int sd = ::accept(_listenSocket, (struct sockaddr*) &addr, &len);
    if (sd < 0) {
        perror("accept() failed");
        return -1;
    }

    int opt = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);

    int sflags = fcntl(sd, F_GETFL, 0);
    sflags |= O_NONBLOCK;
    fcntl(sd, F_SETFL, sflags);

    return sd;
}