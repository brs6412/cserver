# C HTTP Server

This is a simple HTTP server written in C from scratch. It serves static files for my personal website, which I built using React and bundled with `npm run build`.

## Why I built this

I wanted to learn how HTTP servers work at a lower level, without relying on frameworks or libraries. My goals were to:

- Understand network programming and how sockets work in C
- Practice using important system calls like `socket`, `bind`, `listen`, `accept`, `read`, and `write`
- Get more comfortable reading and using manpages
- See how static file serving actually works behind the scenes

## How it works

The server listens on port 4221 and handles basic HTTP GET requests. It serves files out of the directory listed in `cserver.conf`, defaulting to `/var/www/html`.

When a request comes in, the server:

1. Parses the HTTP request line
2. Maps the requested path to a file under the 'serve_dir'. Directory traversal IS detected by comapring the resolved file path with the given 'serve_dir'.
3. Sets the appropriate `Content-Type` header based on the file extension
4. Sends the response headers, and then sends the file as it reads it in chunks

There’s no support for POST requests or routing logic, it’s purely for serving static files.

## How to run it

```
gcc main.c server.c http_handler.c -o server
./server
```

Alternatively, support for CMake has been added:
```
mkdir build
cd build
cmake ..
make
./server
```

## Configuration File

The server can be configured using the `cserver.conf` file, which allows you to set various parameters. The configuration file will be installed in `/etc/cserver` and can be configured accordingly.

Here’s an example of what the `cserver.conf` file looks like:

```
port=8080
max_clients=100
logfile=server.log
serve_dir=/var/www/html
```

## Future Improvements

There are a few things I'd like to add or improve over time:

- ~~Refactor C code (I know it's not pretty, but it's mine and it works)~~ (Refactored into more modular code and made use of structs)
- Create a debian package that handles systemd service setup and ~~binary installation~~
- ~~Detect and prevent directory traversal attacks~~
- Add basic logging for requests and server activity
- ~~Use configuration files instead of hardcoded values for better flexibility~~
