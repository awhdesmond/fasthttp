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
            struct epoll_event* eqptr = _epollq->getEventsPtr();
        
            int i;
            for (i = 0; i < n; i++) {
                struct epoll_event *eptr = (eqptr + i);
                int conn = eptr->data.fd;
                if ((eptr->events & EPOLLERR) || (eptr->events & EPOLLHUP) || !(eptr->events & EPOLLIN)) {
                    close(conn);
                    continue;
                } 

                HttpRequest req;
                HttpResponse res;
                char reqbuf[BUFFERSIZE], tempbuf[BUFFERSIZE];
                memset(tempbuf, 0, BUFFERSIZE);
                memset(reqbuf, 0, BUFFERSIZE);
                
                int prevLen = 0, done = 0;
                // We must read whatever data is available
                // completely, as we are running in edge-triggered mode
                // and won't get a notification again for the same data.
                while (1) {
                    ssize_t bytesRead;
                    bytesRead = read(conn, reqbuf + prevLen, BUFFERSIZE - prevLen);

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

                    int pr = httpParseRequest(reqbuf, BUFFERSIZE, &req);
                    if (pr > 0) { // request read
                        // May have multiple HTTP Requests in a single TCP packet
                        memcpy(tempbuf, reqbuf + pr, BUFFERSIZE - pr);
                        memset(reqbuf, 0, BUFFERSIZE);
                        memcpy(reqbuf, tempbuf, BUFFERSIZE - pr);

                        prevLen = strlen(reqbuf);      

                        httpMakeResponse(&res);
                        if (!checkRequestForHostHeader(&req)) { // check for host header
                            httpMakeMissingHostHeaderResponse(&res);
                        } else {
                            routeRequest(&req, &res); 
                        }

                        if (req.headers["Connection"].compare("close") == 0) {
                            done = 1; // Client request for close connection
                        }
                    } 
                    else if (pr == -2) { // partial valid request;
                        httpMakeContinueResponse(&res);
                    } else { // error; send 400 Bad Request
                        httpMakeBadRequestResponse(&res);
                    }

                    // std::string resStr = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nDate: Wed, 09 Jan 2019 14:27:31 GMT\r\nServer: WebServer\r\nContent-Length: 200\r\n\r\naaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"; 
                    std::string resStr = httpSerialiseResponse(&res, &req);
                    
                    if ((write (conn, resStr.c_str(), resStr.length())) == -1) {
                        perror("write()");
                    }                    
                }
                if (done) {
                    printf ("Closed connection on descriptor %d\n", conn);
                    // Closing the descriptor will make epoll remove it
                    // from the set of descriptors which are monitored.
                    _epollq->remove(conn);
                }
            }
        }

        return NULL; // should not reach here
    }

    private:
    bool checkRequestForHostHeader(HttpRequest* req)
    {
        if (req->version == 1) { // HTTP/1.1
            return req->headers.find("Host") != req->headers.end();
        }
        return true;
    }

    int routeRequest(HttpRequest* req, HttpResponse* res)
    {
        RequestHandler* handler;
        std::tuple<HttpMethod, std::string> handlerKey;
        if (req->method.compare("GET") == 0 || req->method.compare("HEAD") == 0) {
            handlerKey = std::make_tuple(GET, req->path);
        }
        else if (req->method.compare("POST") == 0){
            handlerKey = std::make_tuple(POST, req->path); 
        }
        else {
            return -1; // method not supported
        }

        if(_handlers->find(handlerKey) != _handlers->end()) {
            handler = (*_handlers)[handlerKey];
            (*handler)(req, res);
            return 0;
        } else {
            return -2; // handler not found
        }
    }
};