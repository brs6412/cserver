#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <unistd.h>

#include "http_handler.h"
#include "server.h"

/**
 * Creates server socket and binds to address using port.
 *
 * Parameters:
 *      server: Empty pointer to an HttpServer struct
 *      port: port number to use
 *
 * Returns:
 *      0 for success, -1 if error occured
 */
int init_server(struct HttpServer *server, int port) {
    if (!server) {
        return -1;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        printf("Socket creation failed: %s...\n", strerror(errno));
        return -1;
    }
    
    struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
                                     .sin_port = htons(port),
                                     .sin_addr = { htonl(INADDR_ANY) },
                                    };

    if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
        printf("Bind failed: %s \n", strerror(errno));
        close(server_fd);
        return -1;
    }

    server->server_fd = server_fd;
    server->port = port;
    return 0;
}

/**
 * Starts listening using the given server info.
 *
 * Parameters:
 *      server: Pointer to an HttpServer struct.
 */
void start_server(struct HttpServer *server) {
    if (!server) {
        return -1;
    }

    int connection_backlog = MAX_BACKLOG;
    if (listen(server.server_fd, connection_backlog) != 0) {
        printf("Listen failed: %s \n", strerror(errno));
        return 1;
    }
    
    printf("Waiting for a client to connect...\n");
    int client_addr_len = sizeof(client_addr);
    
    while ((client_fd = accept(server.server_fd, (struct sockaddr *) &client_addr, &client_addr_len))) {
        printf("Client connected\n");
        handle_client(client_fd);
        close(client_fd);
    }
}

/**
 * Cleans up the given server (should never run, given the above while loop. Not a big deal.
 *
 * Parameters:
 *      server: Pointer to an HttpServer struct.
 *
 * Returns:
 *      0 on success, -1 if close() fails.
 */
int end_server(struct HttpServer *server) {
    if (close(server->server_fd) == -1) {
        printf("Error closing socket: %s \n", strerror(errno));
        return -1;
    }
    return 0;
}
