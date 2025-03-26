#include <stdio.h>
#include <string.h>

int main() {
    char str[] = "GET /index.html HTTP/1.1\r\n";
    char *myPtr = strtok(str, " ");
    while (myPtr != NULL) {
        int res = strcmp(myPtr, "HTTP/1.1\r\n");
        printf("%d\n", res);
        myPtr = strtok(NULL, " ");
    }
    return 0;
}
