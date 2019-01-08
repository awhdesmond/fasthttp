#pragma once

#include <string>
#include <map>

enum HttpMethod {
    HEAD,
    GET
};

struct HttpRequest
{
    std::string method;
    std::string path;
    int version;
    std::map<std::string, std::string> headers;
    std::string body;
};

struct HttpResponse
{
    int statusCode;
    std::string statusMsg;
    int version;
    std::map<std::string, std::string> headers;
    std::string body;
};

// Returns number of bytes needed to for message
int httpParseRequest(char* reqBuf, size_t buflen, HttpRequest* req);

