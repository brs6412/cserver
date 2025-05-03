#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H

#define FIELD_LEN       8 
#define STATUS_LEN      16 
#define TYPE_LEN        32
#define MAX_PATH_LEN    256 
#define REQUEST_LEN     4096
#define RESPONSE_LEN    8192 

struct HttpRequest {
    char method[FIELD_LEN];
    char http_version[FIELD_LEN];
    char path[MAX_PATH_LEN];
};

struct HttpResponse {
    char status[STATUS_LEN];   
    char content_type[TYPE_LEN];
    int content_len;
    char connection[FIELD_LEN];
};

int handle_client(int client_fd);

#endif /* HTTP_HANDLER_H */
