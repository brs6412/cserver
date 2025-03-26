#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define BUFFER          1024
#define HTTP_METHOD     7
#define MAX_BACKLOG     5
#define NUM_ENDPOINTS   2
#define RESPONSE_LEN    50

const char *HTTP_200_OK = "HTTP/1.1 200 OK\r\n";
const char *HTTP_NOT_FOUND = "HTTP/1.1 404 Not Found\r\n";

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
    struct stat buffer;
    int found = stat(target, &buffer); 
    if (found == 0) {
        snprintf(file_path, 2048, "public%s", target);
        printf("%s", file_path);
        return 0;
    }
    return -1;
} 

/**
 * Parses the HTTP request and determines if the request is valid.
 * 
 * Parameters:
 *      request: The HTTP request (unmodified)
 * 
 * Returns:
 *      0 on success, -1 if the request is invalid
 */
int handle_get(char *request) {
    char *line = strdup(request);
    char *ptr = strtok(line, " ");
    ptr = strtok(NULL, " ");
    printf("%s\n", ptr);
    if (!file_exists(ptr)) {
        return -1;
    }        
    ptr = strtok(NULL, " ");
    printf("%s", ptr); 
    return (ptr != NULL && strcmp(ptr, "HTTP/1.1") == 0) ? 0 : -1; 
}

/**
 * Reads the client request and sends a response using the given file descriptor.
 */
void handle_client(int client_fd) {
    char buf[BUFFER];
    ssize_t received = recv(client_fd, buf, sizeof(buf), 0); 

    char method[HTTP_METHOD] = {0};
    get_method(buf, method);

    char response[RESPONSE_LEN];
    strncpy(response, "HTTP/1.1 404 Not Found\r\n\r\n", RESPONSE_LEN - 1);

    if (strcmp(method, "GET") == 0) {
        char *request_line = strtok(buf, "\r\n");
        if (request_line != NULL && handle_get(request_line)) {
            strncpy(response, "HTTP/1.1 200 OK\r\n\r\n", RESPONSE_LEN - 1);
        }
    }

    printf("Sending %s", response);
    int sizeo = sizeof(response);
    int len = strlen(response);
    printf("%d %d\n",sizeo, len);
    if (send(client_fd, response, len, 0) == -1) {
        printf("Ran into error sending response: %s...\n", strerror(errno));
    }
}

int main() {
	int     client_fd, server_fd, client_addr_len;
	struct  sockaddr_in client_addr;

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
        // handle client
        close(client_fd);
    }

	close(server_fd);

	return 0;
}
