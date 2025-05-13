#ifndef SERVER_H
#define SERVER_H

#define MAX_BACKLOG     5
#define NUM_ENDPOINTS   2

#define MAX_DIR_LEN 256

struct HttpServer {
    int server_fd;
    int port;
    int max_backlog;
    char serve_dir[MAX_DIR_LEN];
    char logfile[MAX_DIR_LEN];
};

int init_server(struct HttpServer *server);
void start_server(struct HttpServer *server);
int end_server(struct HttpServer *server); 

#endif /* SERVER_H */
