#include <stdlib.h>
#include <stdio.h>
#include "httpserver.h"


class RootHandler : public RequestHandler {
    public:
        RootHandler() {}
        virtual void operator() (HttpRequest* req, HttpResponse* res) {
            // printf("GET / \n");
            res->sendFile("./index.html");
        }
};

int main(int argc, char* argv[])
{
    // Process command line arguments
    if (argc < 3) {
        printf("usage: %s <port>\n", argv[0]);
        exit(-1);
    }

    int port = atoi(argv[2]);

    HttpServer server = HttpServer(port);
    
    // Register handler and routes here
    server.registerHandler(GET, "/", new RootHandler());

    server.start();
}