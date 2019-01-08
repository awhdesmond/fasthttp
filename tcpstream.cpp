#include <arpa/inet.h>
#include "tcpstream.h"

TCPStream::TCPStream(int conn) : _conn(conn) {}

TCPStream::~TCPStream()
{
    close(_conn);
}

ssize_t TCPStream::send(std::string message)
{
    return write(_conn, message.c_str(), message.length());
}

ssize_t TCPStream::receive(std::string& message)
{
    message = std::string(BUFSIZ, 0);
    return read(_conn, &message[0], BUFSIZ - 1);
}