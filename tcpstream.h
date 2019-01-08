#pragma once 

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>

using namespace std;

class TCPStream
{
    int _conn;
    public:
        friend class HttpServer;

        ~TCPStream();

        ssize_t send(char* buffer, size_t len);
        ssize_t receive(char* buffer, size_t len);

        enum {
            connectionClosed = 0,
            connectionReset = -1,
            connectionTimedOut = -2
        };

    private:
        TCPStream(int conn);
        TCPStream();
        TCPStream(const TCPStream& stream);
};
