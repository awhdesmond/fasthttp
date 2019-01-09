#pragma once

#include <pthread.h>
#include <string.h>
#include <sys/epoll.h>

#define EPOLL_MAX_EVENTS 128
#define EPOLL_WAIT_TIMEOUT 200

using namespace std;

class EpollQueue
{
    int _epfd; // epoll descriptor
    struct epoll_event event;
    struct epoll_event events[EPOLL_MAX_EVENTS];
        
    pthread_mutex_t  _mutex;

    public:
    EpollQueue() {
        _epfd = epoll_create1(0);
        
        if (_epfd < 0) {
            perror("epoll_create() failed");
            exit(1);
        }

        memset(&event, 0, sizeof event);
        pthread_mutex_init(&_mutex, NULL);
    }

    ~EpollQueue() {
        pthread_mutex_destroy(&_mutex);
    }

    int getepfd() {
        return _epfd;
    }

    struct epoll_event* getEventsPtr() {
        return events;
    }

    int wait(int timeout = EPOLL_WAIT_TIMEOUT) {
        // pthread_mutex_lock(&_mutex);
        int n = epoll_wait(_epfd, events, EPOLL_MAX_EVENTS, timeout);
        // pthread_mutex_unlock(&_mutex);
        return n;
    }

    void add(int sd) {
        pthread_mutex_lock(&_mutex);
        event.data.fd = sd;
        event.events = EPOLLIN | EPOLLET; // read events using edge triggered mode

        // printf("add sd: %d \n", sd);

        if ((epoll_ctl(_epfd, EPOLL_CTL_ADD, sd, &event)) < 0) {
            perror("epoll_ctl() failed 123");
            exit(1);
        }
        
        pthread_mutex_unlock(&_mutex);
    }

    void remove(int sd) {
        pthread_mutex_lock(&_mutex);
        // Closing the descriptor will make epoll remove it
        // from the set of descriptors which are monitored.
        close(sd); 
        pthread_mutex_unlock(&_mutex);
    }
};