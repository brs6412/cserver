#include <netinet/in.h>

#include "http_handler.h"
#include "server.h"

int main() {
    struct HttpServer server;
    init_server(&server);
    start_server(&server);
    end_server(&server);
    return 0;
}

