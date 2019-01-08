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

ssize_t TCPStream::receive(char* buffer, size_t len)
{
    return read(_conn, buffer, len);
}