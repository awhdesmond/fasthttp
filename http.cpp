#include <chrono>
#include <ctime> 
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
        req->method = std::string(method);
        req->path = std::string(path);
        req->version = minor_version;
        int i;
        for (i = 0; i != num_headers; ++i) {
            req->headers.insert(std::make_pair(headers[i].name, headers[i].value));   
        }

        printf("request is %d bytes long\n", pr);
        printf("method is %.*s\n", (int)method_len, method);
        printf("path is %.*s\n", (int)path_len, path);
        printf("HTTP version is 1.%d\n", minor_version);
        printf("headers:\n");
        for (i = 0; i != num_headers; ++i) {
            printf("%.*s: %.*s\n", (int)headers[i].name_len, headers[i].name,
                (int)headers[i].value_len, headers[i].value);
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

    printf("%s \n", timebuf);

    return 0;
}