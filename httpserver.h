#pragma once

#include <string>
#include "http.h"

#define BUFFERSIZE 4096

class RequestHandler {
    public:
        RequestHandler() {};
        virtual void operator() (HttpRequest req, HttpResponse res) = 0;
};

class HttpServer
{
    // HttpRouter with paths and handler

    int _listenSocket;
    int _port;
    bool _listening;

    public:
        HttpServer(int port);
        ~HttpServer();

        int start();
        // int registerHandler(HttpMethod method, std::string path, RequestHandler handler);

    private:
        int initServerSocket();
        int acceptConnection();
        int routeRequest(HttpRequest req);
};