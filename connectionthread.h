#pragma once

#include <tuple>
#include <map>

#include "http.h"
#include "epollqueue.h"

using namespace std;

class ConnectionThread : public Thread 
{
    EpollQueue* _epollq;
    map<tuple<HttpMethod, string>, RequestHandler*>* _handlers;

    public:
    ConnectionThread(EpollQueue* epollq, map<tuple<HttpMethod, string>, RequestHandler*>* handlers) : _epollq(epollq), _handlers(handlers) {}

    void* run() {
        printf("Connection thread is up and running\n");
        
        while (1) { // Event loop    
            int n = _epollq->wait();
            struct epoll_event* eptr = _epollq->getEventsPtr();
        
            int i;
            for (i = 0; i < n; i++) {
                if (((eptr + i)->events & EPOLLERR) || ((eptr + i)->events & EPOLLHUP) || !((eptr + i)->events & EPOLLIN)) {
                    close((eptr + i)->data.fd);
                    continue;
                } 

                HttpRequest req;
                HttpResponse res;
                char reqBuf[BUFFERSIZE];
                char tempbuf[BUFFERSIZE];
                memset(tempbuf, 0, BUFFERSIZE);
                memset(reqBuf, 0, BUFFERSIZE);
                
                int prevLen = 0;
                int done = 0;
                // We must read whatever data is available
                // completely, as we are running in edge-triggered mode
                // and won't get a notification again for the same data.
                while (1) {
                    ssize_t bytesRead;
                    bytesRead = read((eptr + i)->data.fd, reqBuf + prevLen, BUFFERSIZE - prevLen);

                    if (bytesRead == -1) {
                        // If errno == EAGAIN, that means we have read all
                        // data. So go back to the main loop.
                        if (errno != EAGAIN) {
                            perror ("read");
                            done = 1;
                        }
                        break;
                    }
                    else if (bytesRead == 0) { 
                        // End of file. The remote has closed the connection.
                        done = 1;
                        break;
                    }

                    int pr = httpParseRequest(reqBuf, BUFFERSIZE, &req);                    
                    memcpy(tempbuf, reqBuf + pr, BUFFERSIZE - pr);
                    memset(reqBuf, 0, BUFFERSIZE);
                    memcpy(reqBuf, tempbuf, BUFFERSIZE - pr);

                    prevLen = strlen(reqBuf);      

                    httpMakeResponse(&res);
                    routeRequest(&req, &res); 

                    // std::string resStr = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nDate: Wed, 09 Jan 2019 14:27:31 GMT\r\nServer: WebServer\r\nContent-Length: 200\r\n\r\naaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"; 
                    std::string resStr = httpSerialiseResponse(&res, &req);
                    // printf("%s\n", resStr.c_str())
                    
                    if ((write ((eptr + i)->data.fd, resStr.c_str(), resStr.length())) == -1) {
                        perror("write()");
                    }
                }
                if (done) {
                    printf ("Closed connection on descriptor %d\n", (eptr + i)->data.fd);
                    // Closing the descriptor will make epoll remove it
                    // from the set of descriptors which are monitored.
                    _epollq->remove((eptr + i)->data.fd);
                }
            }
        }

        return NULL; // should not reach here
    }

    private:
    int routeRequest(HttpRequest* req, HttpResponse* res)
    {
        RequestHandler* handler;
        std::tuple<HttpMethod, std::string> handlerKey;
        if (req->method.compare("GET") == 0 || req->method.compare("HEAD") == 0) {
            handlerKey = std::make_tuple(GET, req->path);
            if(_handlers->find(handlerKey) != _handlers->end()) {
                handler = (*_handlers)[handlerKey];
                (*handler)(req, res);
                return 0;
            } else {
                return -2; // handler not found
            }
        }
        else if (req->method.compare("POST") == 0){
            handlerKey = std::make_tuple(POST, req->path);
            if(_handlers->find(handlerKey) != _handlers->end()) {
                handler = (*_handlers)[handlerKey];
                (*handler)(req, res);
                return 0;
            } else {
                return -2;
            } 
        }
        else {
            return -1; // method not supported
        }
    }
};