#include <netinet/in.h>

#include "http_handler.h"
#include "server.h"

#define PORT 4221

int main() {
    // Create HttpServer
    struct HttpServer server;
    init_server(&server, PORT);
    start_server(&server);
    end_server(&server);
    return 0;
}

