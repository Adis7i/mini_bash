#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../include/module/cat.h"

#define BUFF_SIZE 1024

void customsprint_(const char* msg){
    write(STDOUT_FILENO, (void*)msg, strlen(msg));
}

void cat(const char* path) {
    int path_fd;
    ssize_t bytes_readed = 0;
    char *line_buff;

    if((path_fd = open(path, O_RDONLY)) != -1){
        line_buff = (char*)malloc(BUFF_SIZE + 1);
        errno = 0;
        
        while((bytes_readed = read(path_fd, line_buff, BUFF_SIZE)) > 0){
            line_buff[bytes_readed] = '\0';
            if(write(STDOUT_FILENO, line_buff, bytes_readed) == -1) break;
        }
        if(errno != 0){
            perror("cat@IO ");
        }
        
        close(path_fd);
        free(line_buff);
    } else {
        perror("cat@OpenFile");
    }
}