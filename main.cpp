#include <stdlib.h>
#include <stdio.h>
#include "httpserver.h"


int main(int argc, char* argv[])
{
    // Process command line arguments
    if (argc < 3) {
        printf("usage: %s <workers> <port>\n", argv[0]);
        exit(-1);
    }

    int port = atoi(argv[2]);

    HttpServer* server = new HttpServer(port);
    // Register handler and routes here

    server->start();
}