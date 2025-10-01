/** 
 * @file formatter.hpp
 * @brief Declare internal utilities such as normalize, and split_token
 * 
 * @author Adhyastha Abiyyu Azlee
 * @date 2025-06-28
 * @copyright 2025 Adis7i
 */

#ifndef ADIS_UTILS_FORMATTER_HPP
#define ADIS_UTILS_FORMATTER_HPP

#include <string>
#include <vector>
#include <unordered_set>
namespace utls{
    namespace frmt{

        // Normalize POSIX-path
        std::string normalize(std::string inp_path);

        // strip leading and trailin of an input string
        std::string __strip(const std::string &inp_str, char sep);

        // split input string based of a separator
        std::vector<std::string> __split(std::string& , char);

        // Check if input string start with input sstr
        bool startswith(std::string &str, const char* sstr);

        // Convert string to integer
        int str_int(std::string val);

        // Convert string to boolean
        bool str_bool(std::string val);

        // Convert string to decimal (double)
        double str_dob(std::string val);

        // Check if an element is in vector
        template <typename T>
        bool in_vector(const std::vector<T>& __array, T compared_val){
            for(const T element : __array){
                if(element == compared_val){
                    return true;
                }
            }
            return false;
        }

    }
}
#endif