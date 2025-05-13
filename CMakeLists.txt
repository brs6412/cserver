cmake_minimum_required(VERSION 3.10)
project(server)

add_executable(server main.c server.c http_handler.c)

install(TARGETS server DESTINATION bin)
