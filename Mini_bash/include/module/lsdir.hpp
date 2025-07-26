/**
 * @file lsdir.hpp
 * @brief class Declaration (lsdir) for listing directories
 * 
 * @author Adhyastha Abiyyu Azlee
 * @date 2025-06-28
 * @copyright 2025 Adis7i
 */

#ifndef ADIS_LSDIR_HPP
#define ADIS_LSDIR_HPP
#include <dirent.h>
#include <string>
#include <sys/stat.h>
#include <vector>

#define FHINT 1 // Print file type hint
#define FPERM 2 // Print file permission
#define FENCO 4 // Print file entry count

/**
 * @brief list directory entries
 *
 * This class shall list all of directory entries,
 * with good redability upon it's common metadata such as permission, size, entry count, etc.
 * further more this module are NOT for programmer to work with
 * 
 * @note This feature are not supposed to be a class, so don't copy nor distribute it in any kind and Only use it ONCE
*/
class lsdir {
    typedef void (lsdir::*lsdir_task_mem)();

    private:
        // 
        char *lpath;
        DIR *dirp;
        struct dirent *entry;
        struct stat meta{};
        std::string path;
        std::string entry_path;

        // Task list, so while iterating dir we don't have to check conditions everytime
        std::vector<lsdir_task_mem> task_list;

    public:
        /** 
         * @brief Constructor to make new lsdir object
         * 
         * @param inp_path to specify where to list dir
         * @param __sflag option for what operation to execute, as some of them could be heavy
        */
        lsdir(std::string inp_path, int __sflag);
        ~lsdir();

        /**
         * @brief API to execute main feature
         *
        */
        void feature_main();

        /**
         * @brief To print name without hint
         * this is just a filler if FHINT not chosen, or none of the option was chosen
        */
        void __PrintName();

        /**
         * @brief Print permission in bit mode
         */
        void __PrintPerm();

        /**
         * @brief Print highlighted file size, and name(according to file type)
         */
        void __PrintHint();

        /**
         * @brief Count how many entries are in a directory, print 1 entry if file
         */
        void __PrintEntryCount();

};

#endif