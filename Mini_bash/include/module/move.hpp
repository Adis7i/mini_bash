/**
 * @file move.hpp
 * @brief Declaration of class move
 * 
 * @author Adhyastha Abiyyu Azlee
 * @date 2025-06-29
 * @copyright 2025 Adis7i
 */

#ifndef ADIS_MOVE_HPP
#define ADIS_MOVE_HPP

#define DCDEV 1 // Cross Device move
#define DFOVR 2 // Force Overwrite
#define IDTIG 4 // Incomplete Data Transfer Ignore


#include <string>

/**
 * @brief 'Move' file to another directories, or renaming it
 * 
 * This class provide the ability to move file from another partition to another
 * 
 * @note Do not work with this class as it only used for part of my project
 */
class move{
    private :
        std::string old_path;
        std::string new_path;
        bool idtig; 
        int dflag = 0;

    public :
        // Rest of this function is self-explanatory by it's name :)

        move(std::string __old_path, std::string __new_path, int __dflag);

        void __MoveCrossDevice();

        void __Move();

        void feature_main();
};

#endif