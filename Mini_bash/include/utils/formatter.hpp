/** 
 * @file formatter.hpp
 * @brief Declare internal utilities such as normalize, and split_token
 * 
 * @author Adhyastha A.A
 * @date 2025-06-28
 * @copyright 2025 Adis7i
 */

#ifndef ADIS_UTILS_FORMATTER_HPP
#define ADIS_UTILS_FORMATTER_HPP

#include <string>
#include <vector>
#include <iostream>
#include <array>
#include <limits>

namespace utls{
    namespace frmt{
        enum NumCastCode {INVALBASE, INVALPTR, INCOMPLETE, SUCCESS};
        // Normalize POSIX-path
        std::string normalize(std::string inp_path);

        // strip leading and trailin of an input string
        std::string strip(const std::string &inp_str, char sep);

        // split input string based of a separator
        std::vector<std::string> split(std::string& , char);

        // Check if input string start with input sstr
        bool startswith(std::string &str, const char* sstr);

        // Show what is in the array
        template<typename T>
        void show(std::vector<T> arr) {
            std::cout << "[";
            for(T elmnt : arr){
                std::cout << elmnt << ", ";
            }
            std::cout << "]\n";
        }

        // Check if an element is in vector
        template <typename T>
        bool in_vector(const std::vector<T>& array, T compared_val){
            for(const T element : array){
                if(element == compared_val){
                    return true;
                }
            }
            return false;
        }

        constexpr std::array<bool, 256> _make_table(){
            std::array<bool, 256> data {};
            
            for(int i = 0; i < 256; i++){ data[i] = false; }

            for(unsigned char c = '0'; c <= '9'; c++){
                data[c] = true;
            }

            for(unsigned char c = 'A'; c <= 'Z'; c++){
                data[c] = true;
            }

            for(unsigned char c = 'a'; c <= 'z'; c++){
                data[c] = true;
            }

            data['_'] = true;
            return data;
        }

        constexpr std::array<bool, 256> valid_char_table = _make_table();
        bool valid_short_name(unsigned char name);
        bool valid_long_name(const char* _start);
    }
}
#endif