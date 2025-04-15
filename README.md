# C HTTP Server

This is a simple HTTP server written in C from scratch. It serves static files for my personal website, which I built using React and bundled with `npm run build`.

## Why I built this

I wanted to learn how HTTP servers work at a lower level, without relying on frameworks or libraries. My goals were to:

- Understand network programming and how sockets work in C
- Practice using important system calls like `socket`, `bind`, `listen`, `accept`, `read`, and `write`
- Get more comfortable reading and using manpages
- See how static file serving actually works behind the scenes

## How it works

The server listens on port 4221 and handles basic HTTP GET requests. It serves files out of the `build/` directory, which contains the static output from my React app.

When a request comes in, the server:

1. Parses the HTTP request line
2. Maps the requested path to a file under the `build/` directory
3. Reads the file and sends the contents back to the client
4. Sets the appropriate `Content-Type` header based on the file extension

There’s no support for POST requests or routing logic — it’s purely for serving static files.

## How to run it

```bash
gcc main.c -o server
./server
```

## Future Improvements

There are a few things I'd like to add or improve over time:

- Create a debian packages that handles systemd service setup and binary installation
- Include update script that allows any user to pull site updates from the repo
- Detect and prevent directory traversal attacks
- Add basic logging for requests and server activity
