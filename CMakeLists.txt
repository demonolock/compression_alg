cmake_minimum_required(VERSION 3.26)
project(compression_algs C)

set(CMAKE_C_STANDARD 11)

add_executable(compression_algs main.c)

# Находим пакет zstd. Это требует, чтобы у вас был файл конфигурации для zstd на вашем компьютере.
include_directories("/usr/bin/zstd")
link_directories("/usr/bin/zstd")
# Связываем вашу цель с библиотекой zstd.
target_link_libraries(compression_algs zstd)
