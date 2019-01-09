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
            
            // if (n != 0) {
            //     printf("num conn: %d \n", n);
            // }            

            int i;
            for (i = 0; i < n; i++) {
                if (((eptr + i)->events & EPOLLERR) || ((eptr + i)->events & EPOLLHUP) || !((eptr + i)->events & EPOLLIN)) {
                    // printf("epoll error\n");
                    close((eptr + i)->data.fd);
                    continue;
                } 

                // printf("conn thread fd: %d \n", (eptr + i)->data.fd);

                int done = 0;
                // We must read whatever data is available
                // completely, as we are running in edge-triggered mode
                // and won't get a notification again for the same data.

                while (1) {
                    ssize_t bytesRead;
                    char buf[BUFSIZ];

                    bytesRead = read((eptr + i)->data.fd, buf, sizeof buf);
                    // printf("sd: %d \n", (eptr + i)->data.fd);
                    if (bytesRead < 0) {
                        if (errno != EAGAIN) {
                            perror("read()");
                            done = 1;
                        } else {
                            string res = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nDate: Wed, 09 Jan 2019 14:27:31 GMT\r\nServer: WebServer\r\nContent-Length: 200\r\n\r\naaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
                            if ((write ((eptr + i)->data.fd, res.c_str(), res.length())) == -1) {
                                perror("write()");
                            }
                        }
                        break;
                    }
                    else if (bytesRead == 0) {
                        done = 1;
                        break;
                    }
                }

                if (done) {
                    printf ("Closed connection on descriptor %d\n", (eptr + i)->data.fd);
                    _epollq->remove((eptr + i)->data.fd);
                }
            }
        }

        return NULL; // should not reach here
    }
};