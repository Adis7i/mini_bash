#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "../../include/module/move.h"

// I create this because the message was stored as rvalue and I didn't have to make an array
void _customsprint(const char* msg){ // I don't wanna use the extra formatting from stdio.h
    write(STDOUT_FILENO, (void*)msg, strlen(msg));
}

void fmove(const char* old_path, const char* new_path, bool _CrossDevice, bool _AllowOverwrite){    
    if(_CrossDevice){
        int fd_dest;
        int fd_src;
        int dest_flag = O_WRONLY | O_CREAT | __O_NOFOLLOW;
        if(_AllowOverwrite){
            dest_flag |= O_TRUNC;
        } else {
            dest_flag |= O_EXCL;
        }
        fd_dest = open(new_path, dest_flag, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if(fd_dest == -1){
            perror("move@openDest ");
            return;
        }

        struct stat src_stat;
        fd_src = open(old_path, O_RDONLY | __O_NOFOLLOW);
        if(fd_src == -1){
            perror("move@openSrc ");
            close(fd_dest);
            return;
        }

        if(fstat(fd_src, &src_stat) == -1){
            perror("move@SrcStatFetch ");
            close(fd_dest);
            close(fd_src);
            return;
        }
    
        long block_size = src_stat.st_blksize;
        if(block_size == 0) block_size = 4096;
        char* char_buff = (char*)malloc(block_size);
        errno = 0;
        ssize_t byte_read = 0;
        while((byte_read = read(fd_src, char_buff, block_size)) > 0){
            if(write(fd_dest, char_buff, byte_read) > 0) break;
        }
        free(char_buff);
        close(fd_dest);
        close(fd_src);
        if(errno != 0){
            perror("move@IO ");
            return;
        }
        remove(old_path);

    } else {
        if((access(new_path, F_OK) == 0) && !_AllowOverwrite){
            _customsprint("move@WelfareCheck : File exist...\n ");
            return;
        }
        if(rename(old_path, new_path) == -1){
            perror("move@move ");
        }
    }
}

