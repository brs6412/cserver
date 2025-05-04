#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H

#define METHOD_LEN      8 
#define VERSION_LEN     16
#define TYPE_LEN        32
#define MAX_PATH_LEN    256 
#define REQUEST_LEN     4096
#define RESPONSE_LEN    8192 

struct HttpRequest {
    char method[METHOD_LEN];
    char http_version[VERSION_LEN];
    char path[MAX_PATH_LEN];
    off_t file_size;
};

int http_parse_request(char *raw_request, struct HttpRequest *request);
int http_build_response(const struct HttpRequest *request, const char *http_status, char *http_header);

#endif /* HTTP_HANDLER_H */
