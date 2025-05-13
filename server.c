#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <fcntl.h>

#include "http_handler.h"
#include "http_status.h"
#include "server.h"

#define CONFIG_PATH "/etc/cserver/cserver.conf"

/**
 * Creates server socket and binds to address using port.
 *
 * Parameters:
 *      server: Empty pointer to an HttpServer struct
 *
 * Returns:
 *      0 for success, -1 if error occured
 */
int init_server(struct HttpServer *server) {
    if (!server) {
        return -1;
    }

    FILE *fp = fopen(CONFIG_PATH, "r");
    if (!fp) {
        printf("Error opening config file.");
        return -1;
    }

    char line[256];
    while(fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;
        if (line[0] == '#' || line[0] == '\0') {
            continue;
        }
        char *key = strtok(line, "=");
        char *val = strtok(NULL, "=");

        if (!key || !val) {
            printf("Incomplete config file. Please verify.");
        }

        if (strcmp(key, "port") == 0) {
            server->port = atoi(val);
        } else if (strcmp(key, "max_clients") == 0) {
            server->max_backlog = atoi(val);
        } else if (strcmp(key, "logfile") == 0) {
            strncpy(server->logfile, val, sizeof(server->logfile) - 1);
        } else if (strcmp(key, "serve_dir") == 0) {
            strncpy(server->serve_dir, val, sizeof(server->serve_dir) - 1);
        }
    }
    fclose(fp);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        printf("Socket creation failed: %s...\n", strerror(errno));
        return -1;
    }
    
    struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
                                     .sin_port = htons(server->port),
                                     .sin_addr = { htonl(INADDR_ANY) },
                                    };

    int addr_size = sizeof(serv_addr);
    if (bind(server_fd, (struct sockaddr *) &serv_addr, addr_size) != 0) {
        printf("Bind failed: %s \n", strerror(errno));
        close(server_fd);
        return -1;
    }

    server->server_fd = server_fd;
    return 0;
}

/**
 * Clears the entire buffer by sending to the given file descriptor.
 *
 * Parameters:
 *      client_fd:  the client file descriptor
 *      buffer:     the given buffer to send
 *      length:     length of the buffer
 *
 * Returns:
 *      number of bytes sent, -1 if error occurs
 */
ssize_t send_all(int client_fd, const void *buffer, size_t length) {
    ssize_t total = 0;
    const char *buf_ptr = (const char *)buffer;

    while (total < length) {
        ssize_t sent = send(client_fd, buf_ptr + total, length - total, 0);
        if (sent == -1) {
            perror("Error sending data");
            return -1;
        }
        total += sent;
    }
    return total;
}

/**
* Reads the client request and sends a response using the given file descriptor.
*/
void handle_client(struct HttpServer *server, int client_fd) {
    int response_size;
    char buf[REQUEST_LEN];
    char response_buf[RESPONSE_LEN];

    ssize_t received = recv(client_fd, buf, sizeof(buf), 0); 
    if (received < 0) {
        perror("recv");
        return;
    }

    struct HttpRequest request = { .file_size = 0 };
    int valid_request = http_parse_request(buf, &request);
    if (valid_request == -1) {
        response_size = http_build_response(
                                &request, 
                                HTTP_STATUS_BAD_REQUEST, 
                                response_buf);
        send_all(client_fd, response_buf, response_size);
        return;
    }
    
    char target[MAX_DIR_LEN];
    strncpy(target, request.path, sizeof(target) - 1);
    target[sizeof(target) - 1] = '\0';

    if (strcmp(target, "/") == 0) {
        snprintf(
            request.path, 
            sizeof(request.path), 
            "%s/index.html", 
            server->serve_dir);
    } else {
        snprintf(
            request.path, 
            sizeof(request.path) - 1, 
            "%s%s", 
            server->serve_dir, 
            target);
    }

    int file_fd = open(request.path, O_RDONLY);
    if (file_fd == -1) {
        response_size = http_build_response(&request, HTTP_STATUS_NOT_FOUND, response_buf);
        send_all(client_fd, response_buf, response_size);
        return;
    }

    struct stat st;
    if (fstat(file_fd, &st) == -1) {
        perror("fstat");
        close(file_fd);
        return;
    }

    request.file_size = st.st_size;
    
    response_size = http_build_response(&request, HTTP_STATUS_OK, response_buf);
    send_all(client_fd, response_buf, response_size);
    memset(response_buf, 0, sizeof(response_buf));

    ssize_t bytes_read;
    while((bytes_read = read(file_fd, response_buf, sizeof(response_buf))) > 0) {
        if (send_all(client_fd, response_buf, bytes_read) == -1) {
            close(file_fd);
            return;
        }
    }
    close(file_fd);     
}

/**
 * Starts listening using the given server info.
 *
 * Parameters:
 *      server: Pointer to an HttpServer struct.
 */
void start_server(struct HttpServer *server) {
    if (!server) {
        return;
    }

    if (listen(server->server_fd, server->max_backlog) != 0) {
        printf("Listen failed: %s \n", strerror(errno));
        return;
    }
    
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    printf("Waiting for a client to connect...\n");
    
    while ((client_fd = accept(server->server_fd, (struct sockaddr *) &client_addr, &client_addr_len))) {
        printf("Client connected\n");
        handle_client(server, client_fd);
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

