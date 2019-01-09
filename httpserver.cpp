#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include "httpserver.h"
#include "tcpstream.h"
#include "workqueue.h"

class WorkItem
{
    TCPStream* _stream;
 
  public:
    WorkItem(TCPStream* stream) : _stream(stream) {}
    ~WorkItem() { delete _stream; }
 
    TCPStream* getStream() { return _stream; }
};

class ConnectionThread : public Thread 
{
    workqueue<WorkItem*>& _queue; // combined queue
    std::map<std::tuple<HttpMethod, std::string>, RequestHandler*>* _handlers;

    public:
    ConnectionThread(workqueue<WorkItem*>& queue, std::map<std::tuple<HttpMethod, std::string>, RequestHandler*>* handlers) : _queue(queue), _handlers(handlers) {}
 
    void* run() {
        // Remove 1 item at a time and process it. Blocks if no items are 
        // available to process.
        while (1) {
            WorkItem* item = _queue.remove();
            TCPStream* stream = item->getStream();

            char reqBuf[BUFFERSIZE];
            memset(reqBuf, 0, BUFFERSIZE);

            while (1) {
                HttpRequest req;
                HttpResponse res;

                int numRead;
                int prevLen = 0;
                while ((numRead = stream->receive(reqBuf + prevLen, BUFFERSIZE - prevLen)) > 0) {
                    //TODO: Fix this weirdness
                    int pr = httpParseRequest((char*)std::string(reqBuf).c_str(), std::string(reqBuf).length(), &req); // Pray req is less than BUFFERSIZE

                    char tempbuf[BUFFERSIZE];
                    memset(tempbuf, 0, BUFFERSIZE);
                    memcpy(tempbuf, reqBuf + pr, BUFFERSIZE - pr);
                    memset(reqBuf, 0, BUFFERSIZE);
                    memcpy(reqBuf, tempbuf, BUFFERSIZE);
                    
                    prevLen = strlen(reqBuf); 

                    httpMakeResponse(&res);
                    // routeRequest(&req, &res); 

                    std::string responseString = httpSerialiseResponse(&res);
                    stream->send((char *) responseString.c_str(), responseString.length());
                }

                if (numRead == TCPStream::connectionTimedOut) {
                    printf("Connection timeout\n");
                    delete stream;
                    return NULL;
                }

                printf("Connection closed by remote peer\n");
                delete stream; 
                return NULL;
            }
        }
    }

};


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

    workqueue<WorkItem*> queue;
    for (int i = 0; i < NUM_WORKERS; i++) {
        ConnectionThread* thread = new ConnectionThread(queue, &_handlers);
        if (!thread) {
            printf("Could not create ConnectionHandler %d\n", i);
            exit(1);
        } 
        thread->start();
    }

    HttpServer::initServerSocket();
    while (1) {
        int conn = acceptConnection();
        TCPStream* stream = new TCPStream(conn);

        // Create work item
        WorkItem* item = new WorkItem(stream);
        if (!item) {
            printf("Could not create work item a connection\n");
            continue;
        }
        queue.add(item);
        
    }
}

int HttpServer::registerHandler(HttpMethod method, std::string path, RequestHandler* handler)
{
    _handlers.insert(std::make_pair(std::make_tuple(method, path), handler));
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

int HttpServer::routeRequest(HttpRequest* req, HttpResponse* res)
{
    RequestHandler* handler;
    if (req->method.compare("GET") == 0) {
        if(_handlers.find(std::make_tuple(GET, req->path)) != _handlers.end()) {
            handler = _handlers[std::make_tuple(GET, req->path)];
            (*handler)(req, res);
            return 0;
        } else {
            return -2; // handler not found
        }
    }
    else if (req->method.compare("POST") == 0){
        if(_handlers.find(std::make_tuple(POST, req->path)) != _handlers.end()) {
            handler = _handlers[std::make_tuple(POST, req->path)];
            (*handler)(req, res);
            return 0;
        } else {
            return -2;
        } 
    } else {
        return -1; // method not supported
    }
}