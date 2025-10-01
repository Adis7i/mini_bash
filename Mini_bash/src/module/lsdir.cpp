#include "abc/Mini_bash/include/macros.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/limits.h>
#include <cstring> // More stable library for C++
#include <cstdlib> 
#include <iomanip>
#include <iostream>


/**
 * Generally I prefer using C for library like this
 * but I switch to C++ for it's iomanip library
 */

enum lsFlag{
    FPERM = 1, // Print file permission
    FENTCO = 2, // Print file entry count
    FSIZE = 4 // Print file size
};

void _print_permission(mode_t _mode, char b[10]){
    // I'm just substituting the original S_IS* function definition into switch-case
    switch (_mode & 0170000) {
        case 0040000: // => file is a dir
            b[0] = 'd';
            break;

        case 0120000 : // => file is a symbolic link
            b[0] = 'l';
            break;
        
        case 0010000 : // => file is FIFO
            b[0] = 'f';
            break;
        
        case 0060000 : // => file is block special
            b[0] = 'b';
            break;
        
        case 0020000 : // => file is character special
            b[0] = 'c';
            break;

        default :
            b[0] = '-';
            break;
    }

    b[1] = ((_mode & S_IRUSR) == S_IRUSR) ? 'r' : '-';
    b[2] = ((_mode & S_IWUSR) == S_IWUSR) ? 'w' : '-';
    b[3] = ((_mode & S_IXUSR) == S_IXUSR) ? 'x' : '-';
    b[4] = ((_mode & S_IRGRP) == S_IRGRP) ? 'r' : '-';
    b[5] = ((_mode & S_IWGRP) == S_IWGRP) ? 'w' : '-';
    b[6] = ((_mode & S_IXGRP) == S_IXGRP) ? 'x' : '-';
    b[7] = ((_mode & S_IROTH) == S_IROTH) ? 'r' : '-';
    b[8] = ((_mode & S_IWOTH) == S_IWOTH) ? 'w' : '-';
    b[9] = ((_mode & S_IXOTH) == S_IXOTH) ? 'x' : '-';
    write(STDOUT_FILENO, (void*)b, 10);
}

void lsdir(const char* path, unsigned char _flag){
    
    DIR* dirp;
    struct dirent* entry;
    if((dirp = opendir(path)) == nullptr){
        std::perror("lsdir@opendir");
        
        return;
    }

    char* path_buf = (char*)std::malloc(PATH_MAX); // we don't want sudden program halt
    if(path_buf == nullptr) {
        std::perror("lsdir@allocate");
        return;
    }
    std::strcpy(path_buf, path);
    
    bool print_perm = (_flag & FPERM) == FPERM;
    bool print_entry_count = (_flag & FENTCO) == FENTCO;
    bool print_size = (_flag & FSIZE) == FSIZE;
    
    char* concat_point = path_buf + strlen(path_buf);
    // Replace null-terminator with '/' becasue it'll get concatenated anyways
    *concat_point = '/';
    ++concat_point;
    
    struct stat entry_statistics;
    char perm_code[11];
    perm_code[10] = ' ';

    while((entry = readdir(dirp)) != nullptr){
        if((std::strcmp(entry->d_name, "..") == 0) || (std::strcmp(entry->d_name, ".") == 0)){

            continue;
        }
        
        std::strcpy(concat_point, entry->d_name);
        if(lstat(path_buf, &entry_statistics) == -1){
            std::perror("lsdir@statFetch");
            std::free((void*)path_buf);
            return;
        }

        if(print_perm) _print_permission(entry_statistics.st_mode, perm_code);
        if(print_entry_count) std::cout << std::setfill(' ') << std::setw(2) << entry_statistics.st_nlink << " ";
        if(print_size) std::cout << std::setfill(' ') << std::setw(10) << entry_statistics.st_size << " ";
        std::printf("%s\n", entry->d_name);
    }
    std::free((void*)path_buf);
}