cmake_minimum_required(VERSION 3.0)

set(SRCS libserver.c)
add_library(server SHARED ${SRCS})
