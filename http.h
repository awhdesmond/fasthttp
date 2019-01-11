#pragma once

#include <time.h>
#include <string>
#include <map>
#include "time.h"
#include "filemanager.h"
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
#define HTTP_STATUS_MSG_PRECONDITION_FAILED "Preconditioned Failed"
#define HTTP_STATUS_CODE_PRECONDITION_FAILED 412
#define HTTP_STATUS_MSG_ISE "Internal Server Error"
#define HTTP_STATUS_CODE_ISE 500
#define HTTP_STATUS_MSG_NOT_IMPLEMENTED "Not Implemented"
#define HTTP_STATUS_CODE_NOT_IMPLEMENTED 501
#define HTTP_STATUS_MSG_SERVICE_UNAVALIABLE "Service Unavailable"
#define HTTP_STATUS_CODE_SERVICE_UNAVALIABLE 503

#define HTTP_HEADER_DATE "Date"
#define HTTP_HEADER_CONNECTION "Connection"
#define HTTP_HEADER_CONTENT_LENGTH "Content-Length"
#define HTTP_HEADER_CONTENT_TYPE "Content-Type"
#define HTTP_HEADER_IF_MODIFIED_SINCE "If-Modified-Since"

#define CONTENT_TYPE_FORM "application/x-www-form-urlencoded"
#define CONTENT_TYPE_JSON "application/json"
#define CONTENT_TYPE_XML "application/xml"
#define CONTENT_TYPE_PLAIN "text/plain"
#define CONTENT_TYPE_CSV "text/csv"
#define CONTENT_TYPE_HTML "text/html"


#define HTTP_DELIMETER "\r\n"

static FileManager fm;

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
    HttpRequest* req;

    int statusCode;
    std::string statusMsg;
    int version; // minor version
    std::map<std::string, std::string> headers;
    std::string body;

    // Returns file size, i.e. num bytes read
    int sendFile(std::string relativePathname) {
        
        // Checks for CONDITIONAL GET
        int fileSize;
        if ((fileSize = _checkForConditionalGet(relativePathname)) >= 0) {
            statusCode = HTTP_STATUS_CODE_NOT_MODIFIED;
            statusMsg = HTTP_STATUS_MSG_NOT_MODIFIED;
            return fileSize; 
        }
        
        int r;
        if ((r = fm.readFile(relativePathname, &body)) < 0) {
            return r;
        }

        std::string fileExt = extractFileExtension(&relativePathname);

        if (fileExt.compare(FILE_EXT_JSON) == 0) {
            headers.insert(std::make_pair(HTTP_HEADER_CONTENT_TYPE, CONTENT_TYPE_JSON));
        } else if (fileExt.compare(FILE_EXT_CSV) == 0) {
            headers.insert(std::make_pair(HTTP_HEADER_CONTENT_TYPE, CONTENT_TYPE_CSV));
        } else if ((fileExt.compare(FILE_EXT_HTML) == 0)) {
            headers.insert(std::make_pair(HTTP_HEADER_CONTENT_TYPE, CONTENT_TYPE_HTML));
        } else if ((fileExt.compare(FILE_EXT_XML) == 0)) {
            headers.insert(std::make_pair(HTTP_HEADER_CONTENT_TYPE, CONTENT_TYPE_XML));
        } else {
            headers.insert(std::make_pair(HTTP_HEADER_CONTENT_TYPE, CONTENT_TYPE_PLAIN));
        }

        return body.length();
    }

    int _checkForConditionalGet(std::string filename) {
        if (req->headers.find(HTTP_HEADER_IF_MODIFIED_SINCE) != req->headers.end()) {
            std::string dateString = req->headers[HTTP_HEADER_IF_MODIFIED_SINCE];
            
            struct stat fileStat;
            time_t headerTime;
            struct tm tmheader = {};
            
            char *end;
            if ((end = strptime(dateString.c_str(), "%a, %d %b %Y %H:%M:%S GMT", &tmheader)) == NULL) { 
                if ((end = strptime(dateString.c_str(), "%A, %d-%b-%y %H:%M:%S GMT", &tmheader)) == NULL) {
                    end = strptime(dateString.c_str(), "%A, %d-%b-%y %H:%M:%S GMT", &tmheader);
                }
            }
            
            if (end != NULL) { // OK
                headerTime = mktime(&tmheader);
                readFileStat(filename, &fileStat);
                if (difftime(headerTime, fileStat.st_mtime) > 0) {
                    return fileStat.st_size; 
                }
            }
        }
        return -1;
    }
};

// Returns number of bytes needed to for message
int httpParseRequest(char* reqBuf, size_t buflen, HttpRequest* req);

// Generate a default http response
int httpMakeResponse(HttpRequest* req, HttpResponse* res);
int httpMakeContinueResponse(HttpResponse* res);
int httpMakeBadRequestResponse(HttpResponse* res);
int httpMakeBadMethodResponse(HttpResponse* res);
int httpMakeNotFoundResponse(HttpResponse* res);
int httpMakeMissingHostHeaderResponse(HttpResponse* res);

std::string httpSerialiseResponse(HttpResponse* res);

void httpPrintRequest(HttpRequest* req);
void httpPrintResponse(HttpResponse* res);