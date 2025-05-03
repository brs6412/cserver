#ifndef SERVER_H
#define SERVER_H

#define MAX_BACKLOG     5
#define NUM_ENDPOINTS   2

struct HttpServer {
    int server_fd;
    int port;
};

int init_server(int port);
int start_server(struct HttpServer *server);
void end_server(struct HttpServer *server); 

#endif /* SERVER_H */
