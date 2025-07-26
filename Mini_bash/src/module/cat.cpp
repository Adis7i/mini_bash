#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <cerrno>
#include <string.h>
#include "../../include/module/cat.hpp"

#define BUFF_SIZE 1024

void cat(std::string path) {
    int path_fd;
    ssize_t bytes_readed = 0;
    char *line_buff;

    if((path_fd = open(path.c_str(), O_RDONLY)) != -1){
        line_buff = new char[BUFF_SIZE + 1];
        errno = 0;
        
        while((bytes_readed = read(path_fd, line_buff, BUFF_SIZE)) > 0){
            line_buff[bytes_readed] = '\0'; // Karena array itu 0-indexed jadi ujung array = (total elemen - 1)
            std::cout << line_buff;
        }
        if(errno != 0){
            std::cerr << "[Cat] file read : " << strerror(errno) << std::endl;
        }
        
        close(path_fd);
        delete[] line_buff;
        std::cout << "[Cat] Cleaned up !" << std::endl;
    } else {
        std::cerr << "[Cat] Open file : " << strerror(errno) << std::endl;
    }
}