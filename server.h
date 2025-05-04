#ifndef SERVER_H
#define SERVER_H

#define MAX_BACKLOG     5
#define NUM_ENDPOINTS   2

struct HttpServer {
    int server_fd;
    int port;
};

int init_server(struct HttpServer *server, int port);
void start_server(struct HttpServer *server);
int end_server(struct HttpServer *server); 

#endif /* SERVER_H */
