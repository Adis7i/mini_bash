/**
 * @file lsdir.hpp
 * @brief Declare lsdir
 * 
 * @author Adhyastha A.A
 * @date 2025-10-01
 * @copyright 2025 Adis7i
 */

#ifndef ADIS_LSDIR_HPP
#define ADIS_LSDIR_HPP
#include <dirent.h>
#include <string>
#include <sys/stat.h>
#include <vector>

enum lsFlag{
    FPERM = 1, // Print file permission
    FENTCO = 2, // Print file entry count
    FSIZE = 4 // Print file size
};

void lsdir(const char* path, unsigned char _flag);
void _print_permission(mode_t _mode, char b[10]);

#endif