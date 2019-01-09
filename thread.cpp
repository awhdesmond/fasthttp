#include "thread.h"

Thread::Thread() : _tid(0), _running(false), _detached(false) {}

Thread::~Thread()
{
    if (_running && !_detached) {
        pthread_detach(_tid);
    }
    if (_running) {
        pthread_cancel(_tid);
    }
}

static void* runThread(void* arg);

int Thread::start()
{
    int r = pthread_create(&_tid, NULL, runThread, this);
    if (r == 0) {
        _running = true;
    }
    return r;
}

static void* runThread(void* arg)
{
    return ((Thread*)arg)->run();
}

int Thread::join()
{
    int r = -1;
    if (_running) {
        r = pthread_join(_tid, NULL);
        if (r == 0) {
            _detached = true;
        }
    }
    return r;
}

int Thread::detach()
{
    int r = -1;
    if (_running && !_detached) {
        r = pthread_detach(_tid);
        if (r == 0) {
            _detached = true;
        }
    }

    return r;
}

pthread_t Thread::self() {
    return _tid;
}
