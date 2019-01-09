#include <arpa/inet.h>
#include "tcpstream.h"

TCPStream::TCPStream(int conn) : _conn(conn) {}

TCPStream::~TCPStream()
{
    close(_conn);
}

ssize_t TCPStream::send(char* buffer, size_t len)
{
    return write(_conn, buffer, len);
}

ssize_t TCPStream::receive(char* buffer, size_t len, int timeout) 
{
    if (timeout <= 0) return read(_conn, buffer, len);

    if (waitForReadEvent(timeout) == true)
    {
        return read(_conn, buffer, len);
    }
    return connectionTimedOut;

}

bool TCPStream::waitForReadEvent(int timeout)
{
    fd_set sdset;
    struct timeval tv;

    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    FD_ZERO(&sdset);
    FD_SET(_conn, &sdset);
    if (select(_conn + 1, &sdset, NULL, NULL, &tv) > 0)
    {
        return true;
    }
    return false;
}

