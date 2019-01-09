#pragma once

#include <string>
#include <map>
#include "utils.h"

#define HTTP_STATUS_MSG_CONTINUE "Continue"
#define HTTP_STATUS_CODE_CONTINUE 100
#define HTTP_STATUS_MSG_OK "OK"
#define HTTP_STATUS_CODE_OK 200
#define HTTP_STATUS_MSG_NOT_MODIFIED "Not Modified"
#define HTTP_STATUS_CODE_NOT_MODIFIED 304
#define HTTP_STATUS_MSG_BAD_REQUEST "Bad Request"
#define HTTP_STATUS_CODE_BAD_REQUEST 400
#define HTTP_STATUS_MSG_UNAUTHORIZED "Unauthorized"
#define HTTP_STATUS_CODE_UNAUTHORIZED 401
#define HTTP_STATUS_MSG_FORBIDDEN "Forbidden"
#define HTTP_STATUS_CODE_FORBIDDEN 403
#define HTTP_STATUS_MSG_NOT_FOUND "Not Found"
#define HTTP_STATUS_CODE_NOT_FOUND 404
#define HTTP_STATUS_MSG_ISE "Internal Server Error"
#define HTTP_STATUS_CODE_ISE 500
#define HTTP_STATUS_MSG_NOT_IMPLEMENTED "Not Implemented"
#define HTTP_STATUS_CODE_NOT_IMPLEMENTED 501
#define HTTP_STATUS_MSG_SERVICE_UNAVALIABLE "Service Unavailable"
#define HTTP_STATUS_CODE_SERVICE_UNAVALIABLE 503

#define HTTP_DELIMETER "\r\n"

enum HttpMethod {
    HEAD,
    GET,
    POST
};

struct HttpRequest
{
    std::string method;
    std::string path;
    int version; // minor version
    std::map<std::string, std::string> headers;
    std::string body;
};

struct HttpResponse
{
    int statusCode;
    std::string statusMsg;
    int version; // minor version
    std::map<std::string, std::string> headers;
    std::string body;

    // 1. load file contents into response body
    // 2. check file ext to specify content-type
    // Returns file size, i.e. num bytes read
    int sendFile(std::string relativePathname) { 
        int r;
        if ((r = readFileIntoString(relativePathname, &body)) < 0) {
            return r;
        }
        
        std::string fileExt = extractFileExtension(&relativePathname);

        if (fileExt.compare(CONTENT_TYPE_JSON) == 0) {
            headers.insert(std::make_pair("Content-Type", "application/json"));
        } else if (fileExt.compare(CONTENT_TYPE_CSV) == 0) {
            headers.insert(std::make_pair("Content-Type", "text/csv"));
        } else if ((fileExt.compare(CONTENT_TYPE_HTML) == 0)) {
            headers.insert(std::make_pair("Content-Type", "text/html"));
        } else if ((fileExt.compare(CONTENT_TYPE_XML) == 0)) {
            headers.insert(std::make_pair("Content-Type", "text/xml"));
        } else {
            headers.insert(std::make_pair("Content-Type", "text/plain"));
        }

        return body.length();
    }
};

// Returns number of bytes needed to for message
int httpParseRequest(char* reqBuf, size_t buflen, HttpRequest* req);

// Generate a default http response
int httpMakeResponse(HttpResponse* res);
std::string httpSerialiseResponse(HttpResponse* res);

void httpPrintRequest(HttpRequest* req);
void httpPrintResponse(HttpResponse* res);