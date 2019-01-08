#include <chrono>
#include <ctime> 
#include <iostream>
#include "http.h"
#include "./vendor/picohttpparser.h"

int httpParseRequest(char* reqBuf, size_t buflen, HttpRequest* req)
{
    const char *method, *path;
    int minor_version;
    struct phr_header headers[100];
    size_t method_len, path_len, num_headers;
    
    int pr = phr_parse_request(reqBuf, buflen, &method, &method_len, &path, &path_len,
                               &minor_version, headers, &num_headers, 0);

    if (pr > 0) { // successfully parsed the request 
        req->method = std::string(method, (int)method_len);
        req->path = std::string(path, (int)path_len);
        req->version = minor_version;
        int i;
        for (i = 0; i != num_headers; ++i) {
            req->headers.insert(std::make_pair(headers[i].name, headers[i].value));   
        }
    }
    return pr; // positive is num bytes used, -1 is error, -2 is incomplete
}

int httpMakeResponse(HttpResponse* res)
{
    res->statusCode = HTTP_STATUS_CODE_OK;
    res->statusMsg = std::string(HTTP_STATUS_MSG_OK);
    res->version = 1;
    res->headers.insert(std::make_pair("Server", "HTTP Server"));

    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    info = gmtime(&rawtime); // get GMT time
    char timebuf[128];
    strftime (timebuf, 128, "%a, %d %b %Y %H:%M:%S GMT", info);
    res->headers.insert(std::make_pair("Date", std::string(timebuf)));

    // res->headers.insert(std::make_pair("Content-Type", "text/html; charset=UTF-8"));
    // res->headers.insert(std::make_pair("Connection", "Keep-Alive"));
    // res->headers.insert(std::make_pair("Keep-Alive", "timeout=3, max=120"));
    // res->headers.insert(std::make_pair("Last-Modified", "Tue, 08 Jan 2019 14:41:03 GMT"));

    return 0;
}

std::string httpSerialiseResponse(HttpResponse* res)
{
    std::string result;
    result = "HTTP/1.1 " + std::to_string(res->statusCode) + " " + res->statusMsg + HTTP_DELIMETER; // status line

    std::map<std::string, std::string>::iterator it = res->headers.begin();
    while(it != res->headers.end()) {
        result = result + it->first + ": " + it->second + HTTP_DELIMETER;
        it++;
    }

    result = result + "Content-Length: " + std::to_string(res->body.length()) + HTTP_DELIMETER;
    result = result + HTTP_DELIMETER;
    result = result + res->body;

    return result;
}


void httpPrintRequest(HttpRequest* req)
{
    printf("Request method is %s\n", req->method.c_str());
    printf("Request path is %s\n", req->path.c_str());
    printf("HTTP version is 1.%d\n", req->version);
    printf("Request Headers:\n");
    std::map<std::string, std::string>::iterator it = req->headers.begin();
    while(it != req->headers.end()) {
        printf("%s\n", (it->first + ": " + it->second).c_str());
        it++;
    }
}

void httpPrintResponse(HttpResponse* res)
{
    printf("Reponse status code is %d\n", res->statusCode);
    printf("Reponse status phrase is %s\n", res->statusMsg.c_str());
    printf("HTTP version is 1.%d\n", res->version);
    printf("Reponse Headers:\n");
    std::map<std::string, std::string>::iterator it = res->headers.begin();
    while(it != res->headers.end()) {
        printf("%s \n", (it->first + ": " + it->second).c_str());
        it++;
    }
}