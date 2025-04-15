#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define BUFFER          1024
#define HTTP_METHOD     7
#define MAX_BACKLOG     5
#define MAX_PATH_LENGTH 2048
#define NUM_ENDPOINTS   2
#define RESPONSE_LEN    50

const char *HTTP_200_OK = "HTTP/1.1 200 OK\r\n";
const char *HTTP_NOT_FOUND = "HTTP/1.1 404 Not Found\r\n";

int server_fd = -1;

void sigintHandler(int sig) {
    printf("\nExiting gracefully...\n");
    if (server_fd != -1) {
        close(server_fd);
        printf("Server socket closed.\n");
    }
    exit(0);
}

/**
 * Returns the MIME type when responding with content.
 */
const char *get_mime_type(const char *filename) {
    if (strstr(filename, ".html")) return "text/html";
    if (strstr(filename, ".css")) return "text/css";
    if (strstr(filename, ".js")) return "text/javascript";
    if (strstr(filename, ".jpg")) return "image/jpeg";
    return "application/octet-stream";
}

/**
 * Parses the request line for the HTTP verb and writes it to result.
 * 
 * Paramters:
 *      request: The HTTP request-line.
 *      result: Pointer to a character array where the first word of the request-line is written.
 *              The array must be of size HTTP_METHOD.
 */
void get_method(char *request, char *result) {
    if (request == NULL || result == NULL) {
        return;
    }
    char *ptr = strstr(request, " ");
    int i = 0;
    while (request[i] != *ptr && i < HTTP_METHOD) {
        strncat(result, &request[i], 1);
        i += 1;
    }
}

/**
 * Checks the public/ folder for the requested file and returns the file_path.
 * 
 * Parameters:
 *      target: The request-line request target.
 *      file_path: Pointer to a character array where the absolute file path is written.
 *                 The array must be of size MAX_PATH_LENGTH.
 * Returns:
 *      0 on success, -1 if the requested file is not found (TODO: return -1 if path traversal attack)
 */
int file_exists(const char *target, char *file_path) {
    if (strcmp(target, "/") == 0) {
        snprintf(file_path, MAX_PATH_LENGTH, "public/index.html");
        return 0;
    }

    struct stat buffer;

    snprintf(file_path, MAX_PATH_LENGTH, "public%s", target);
    if (stat(file_path, &buffer) == 0) {
        return 0;
    }
    return -1;
} 

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
 * Parses the HTTP request and sends the appropriate response. 
 * 
 * Parameters:
 *      client_fd:  The clients file descriptor 
 *      request:    The HTTP request (unmodified)
 */
int handle_get(int client_fd, char *request) {
    char response[RESPONSE_LEN];
    char *ptr = strtok(request, " ");
    char path[MAX_PATH_LENGTH];

    ptr = strtok(NULL, " ");

    if (ptr == NULL || file_exists(ptr, path) == -1){
        snprintf(response, sizeof(response), "HTTP/1.1 404 Not Found\r\n\r\n");
        if (send_all(client_fd, response, strlen(response)) == -1) {
            return -1;
        }
        perror("Requested file not found.");
        return 0;
    }        
    ptr = strtok(NULL, " ");
    if (ptr != NULL && strcmp(ptr, "HTTP/1.1") != 0) {
        snprintf(response, sizeof(response), "HTTP/1.1 400 Bad Request\r\n\r\n");
        if (send_all(client_fd, response, strlen(response)) == -1) {
            return -1;
        }
        perror("Bad request.");
        return 0;

    } 
    const char *header_template = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Connection: close\r\n"
        "\r\n";
    const char *content_type = get_mime_type(path);
    char http_header[BUFFER];
    snprintf(http_header, sizeof(http_header), header_template, content_type);
    if (send_all(client_fd, http_header, strlen(http_header)) == -1) {
        return -1;
    }
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        return -1;
    }
    char file_buf[BUFFER];
    size_t bytes_read;
    while((bytes_read = fread(file_buf, 1, sizeof(file_buf), file)) > 0) {
        if (send_all(client_fd, file_buf, bytes_read) == -1) {
            fclose(file);
            return -1;
        }
    }
    fclose(file); 
}

/**
 * Reads the client request and sends a response using the given file descriptor.
 */
void handle_client(int client_fd) {
    char buf[BUFFER];
    ssize_t received = recv(client_fd, buf, sizeof(buf), 0); 

    char method[HTTP_METHOD] = {0};
    get_method(buf, method);


    if (strcmp(method, "GET") == 0) {
        char *request_line = strtok(buf, "\r\n");
        if (request_line != NULL) {
            handle_get(client_fd, request_line);
        }
    }
}

int main() {
    int     client_fd, client_addr_len;
    struct  sockaddr_in client_addr;

    signal(SIGINT, sigintHandler);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        printf("Socket creation failed: %s...\n", strerror(errno));
        return 1;
    }
    
    struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
                                     .sin_port = htons(4221),
                                     .sin_addr = { htonl(INADDR_ANY) },
                                    };

    if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
        printf("Bind failed: %s \n", strerror(errno));
        return 1;
    }
    
    int connection_backlog = MAX_BACKLOG;
    if (listen(server_fd, connection_backlog) != 0) {
        printf("Listen failed: %s \n", strerror(errno));
        return 1;
    }
    
    printf("Waiting for a client to connect...\n");
    client_addr_len = sizeof(client_addr);
    
    while ((client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len))) {
        printf("Client connected\n");
        handle_client(client_fd);
        close(client_fd);
    }

    close(server_fd);

    return 0;
}
