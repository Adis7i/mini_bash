#include <cerrno>
#include <limits.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "../../include/macros.hpp" 
#include "../../include/module/lsdir.hpp" 


lsdir::lsdir(std::string inp_path, int __fflag) : path(inp_path)
{
    lpath = new char[PATH_MAX + 1];
    if(lpath == nullptr){ throw "BAD ALLOC"; }
    if ((__fflag & FPERM) == FPERM)
    {
        task_list.push_back(&lsdir::__PrintPerm);
    }
    if ((__fflag & FENCO) == FENCO)
    {
        task_list.push_back(&lsdir::__PrintEntryCount);
    }
    if ((__fflag & FHINT) == FHINT)
    {
        task_list.push_back(&lsdir::__PrintHint);
    }
    else
    {
        task_list.push_back(&lsdir::__PrintName);
    }

    dirp = opendir(inp_path.c_str());
    if (dirp == nullptr)
    {
        std::cerr << "[lsdir] Opendir failed" << std::endl;
        throw "Opendir error";
    }
}


lsdir::~lsdir()
{
    delete[] lpath;
    if (dirp != nullptr) { 
        closedir(dirp);
    }
    std::cout << "Cleaned up !" << std::endl;
}


void lsdir::feature_main() 
{
    unsigned long total_size = 0;
    while ((entry = readdir(dirp)) != nullptr)
    {
        if (strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".") == 0)
        {
            continue;
        }
        entry_path = path + "/" + entry->d_name;
        if (lstat(entry_path.c_str(), &meta) != -1)
        {
            total_size += meta.st_size;
            for (int i = 0; i < task_list.size(); i++)
            {
                (this->*task_list[i])();
            }
        }
        else
        {
            std::cerr << "[lsdir] Fetching " << entry->d_name << " Metadata failed : " << strerror(errno);
        }
        std::cout << std::endl;
    }
    std::cout << "Total size : " << total_size << " Bytes" << std::endl;
}


void lsdir::__PrintPerm() 
{
    int mode = meta.st_mode;
    std::cout << ((S_ISDIR(mode)) ? "d" : "-");
    std::cout << (((mode & S_IRUSR) == S_IRUSR) ? "r" : "-");
    std::cout << (((mode & S_IWUSR) == S_IWUSR) ? "w" : "-");
    std::cout << (((mode & S_IXUSR) == S_IXUSR) ? "x" : "-");
    std::cout << (((mode & S_IRGRP) == S_IRGRP) ? "r" : "-");
    std::cout << (((mode & S_IWGRP) == S_IWGRP) ? "w" : "-");
    std::cout << (((mode & S_IXGRP) == S_IXGRP) ? "x" : "-");
    std::cout << (((mode & S_IROTH) == S_IROTH) ? "r" : "-");
    std::cout << (((mode & S_IWOTH) == S_IWOTH) ? "w" : "-");
    std::cout << (((mode & S_IXOTH) == S_IXOTH) ? "x " : "- ");
}


void lsdir::__PrintName() 
{
    std::cout << entry->d_name;
}


void lsdir::__PrintHint() 
{
    long size = meta.st_size; 
    int mode = meta.st_mode;
    std::cout << " " << std::right;
    if (size <= MB(1)) 
    {
        std::cout << GREEN;
    }
    else if (size <= MB(100))
    {
        std::cout << YELLOW;
    }
    else if (size <= MB(1024)) 
    {
        std::cout << RED;
    }
    else
    {
        std::cout << BOLD << RED;
    }
    std::cout << std::setw(9) << size << RESET << std::left << " ";

    if (S_ISLNK(mode))
    {
        ssize_t bytes_given;
        std::cout << BOLD << CYAN << entry->d_name << RESET << " -> ";
        if ((bytes_given = readlink(entry_path.c_str(), lpath, PATH_MAX)) != -1)
        {
            lpath[bytes_given] = '\0';
            std::cout << lpath;
        }
    }
    else if (S_ISREG(mode))
    {
        std::cout << entry->d_name;
    }
    else if (S_ISDIR(mode))
    {
        std::cout << BOLD << BLUE << entry->d_name << RESET;
    }
    else if (S_ISBLK(mode))
    {
        std::cout << BOLD << YELLOW << entry->d_name << RESET;
    }
    else if (S_ISSOCK(mode))
    {
        std::cout << BOLD << GREEN << entry->d_name << RESET;
    }
    else if (S_ISFIFO(mode))
    {
        std::cout << BOLD << RED << entry->d_name << RESET;
    }
}


void lsdir::__PrintEntryCount() 
{
    int res = 0;
    DIR *subdirp = opendir(entry_path.c_str());
    if (subdirp != nullptr)
    {
        while (readdir(subdirp) != nullptr)
        {
            ++res;
        }
        closedir(subdirp);
    }
    else
    {
        res = 1;
    }
    std::cout << res;
}