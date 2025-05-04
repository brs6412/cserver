#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "http_handler.h"
#include "http_status.h"

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
 * Parses the HTTP request and sends the appropriate response.
 *      Note: This function only parses the request and verifies the syntax.
 * 
 * Parameters:
 *      raw_request:    The raw HTTP request.
 *      request:        Empty HttpRequest struct to be built.
 *
 * Returns:
 *      0 on success, -1 if 400 Bad Request should be sent.
 */
int http_parse_request(char *raw_request, struct HttpRequest *request) {
    char request_line[REQUEST_LEN];
    const char *eol = strstr(raw_request, "\r\n");
    if (eol == NULL) {
        return -1;
    }

    size_t len = eol - raw_request;
    if (len >= sizeof(request_line)) {
        return -1;
    }
    
    strncpy(request_line, raw_request, len);
    request_line[len] = '\0';
 
    char *ptr = strtok(request_line, " ");
    if (ptr == NULL) {
        return -1;
    } else {
        strncpy(request->method, ptr, sizeof(request->method) - 1);
        request->method[sizeof(request->method) - 1] = '\0';
    } 

    ptr = strtok(NULL, " ");

    if (ptr == NULL) {
        return -1;
    } else {
        strncpy(request->path, ptr, sizeof(request->path) - 1);
        request->path[sizeof(request->path) - 1] = '\0';
    }

    ptr = strtok(NULL, " ");

    if (ptr == NULL || strcmp(ptr, "HTTP/1.1") != 0) {
        return -1;
    } else {
        strncpy(request->http_version, ptr, sizeof(request->http_version) - 1);
        request->http_version[sizeof(request->http_version) - 1] = '\0';
    }

    return 0;
}

int http_build_response(const struct HttpRequest *request, const char *http_status, char *http_header) {
    const char *header_template = 
        "%s %s\r\n"
        "Content-Type: %s\r\n"
        "Connection: close\r\n"
        "Content-Length: %d\r\n"
        "\r\n";
    const char *content_type = get_mime_type(request->path);
    int header_size = snprintf(
                        http_header, 
                        RESPONSE_LEN, 
                        header_template, 
                        request->http_version, 
                        http_status, 
                        content_type, 
                        request->file_size);
    return header_size;
}

