#pragma once

#include <string>
#include <map>
#include <tuple>
#include "http.h"
#include "tcpstream.h"
#include "thread.h"

#define BUFFERSIZE 1024
#define NUM_WORKERS 1

class RequestHandler {
    public:
        RequestHandler() {};
        virtual void operator() (HttpRequest* req, HttpResponse* res) = 0;
};

class HttpServer
{
    // HttpRouter with paths and handler

    int _listenSocket;
    int _port;
    bool _listening;

    std::map<std::tuple<HttpMethod, std::string>, RequestHandler*> _handlers;

    public:
        HttpServer(int port);
        ~HttpServer();

        int start();
        int registerHandler(HttpMethod method, std::string path, RequestHandler* handler);

    private:
        int initServerSocket();
        int acceptConnection();
        
};