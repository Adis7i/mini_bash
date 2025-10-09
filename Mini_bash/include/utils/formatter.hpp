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

        // Show what is in the array
        template<typename T>
        void __show(std::vector<T> arr) {
            std::cout << "[";
            for(T elmnt : arr){
                std::cout << elmnt << ", ";
            }
            std::cout << "]\n";
        }

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

        bool is_valid_posarg(const std::string& name);
        bool is_valid_short_flag(const std::string& name);
        bool is_valid_long_flag(const std::string& name);

        constexpr std::array<unsigned char, 256> make_lookup() {
            std::array<unsigned char, 256> arr{}; // Initializes all elements to 0

            for (int i = 0; i < 256; i++) {
                arr[i] = -1;
            }

            for (int i = 0; i <= 9; i++) {
                arr[48 + i] = i;
            }
            for (int i = 10; i <= 35; i++) {
                arr[55 + i] = i;
            }
            for (int i = 10; i <= 35; i++) {
                arr[87 + i] = i;
            }
            return arr;
    }
        constexpr std::array<unsigned char, 256> lookup = make_lookup();
        
        struct Decimal {
            long long mantissa;
            int exponent;
            char base;
            NumCastCode code;
            Decimal(const long long& __mantissa, const int __expo, const char& __base, NumCastCode __code) : mantissa(__mantissa), exponent(__expo), base(__base), code(__code) {}
        };

        template <typename T>
        struct Integral {
            T value;
            NumCastCode code;
            Integral(const T& __value, NumCastCode __code) : value(__value), code(__code) {}
        };
    
        /*Function within these namespace do NOT have any safety measure !,
        and only a part of util:: function API,
        use it at your own risk... */
        namespace notApi {
            // handles base
            template <typename __out_type>
            inline NumCastCode fc_ib(const char*& start, const char* end, char base, __out_type& buf){
                constexpr unsigned long long reg_max = std::numeric_limits<__out_type>::max();
            
                for(; start < end; start++){
                    int c = lookup[*start];
                    if(c == -1) { return NumCastCode::INCOMPLETE; }
                    if(c < base) {
                        if(buf > ((reg_max / base) - c)) return NumCastCode::INCOMPLETE; // check before multiply and addition
                        buf *= base;                    
                        buf += c; 
                    } else return NumCastCode::INCOMPLETE;
                }
                return NumCastCode::SUCCESS;
            }
    // fc_fb
            Decimal fc_fb(const char* start, const char* end, char base);
        }
        
        template <typename __out_type>
        typename std::enable_if<std::is_integral<__out_type>::value, Integral<__out_type>>::type
        fc(const char* start, const char* end, char base){
            if(start < end){
                if((2 <= base) && (base <= 36)){
                    __out_type buf = 0;
                    NumCastCode code;
                    char sign;
                
                    switch (*start)
                    {
                    case '-' :
                        sign = -1;
                        break;
                    
                    case '+':
                    default:
                        sign = 1;
                        break;
                    }
                
                    if(base <= 10){
                        code = notApi::fc_ib<__out_type>(start, end, base, buf);
                    } else code = notApi::fc_ib<__out_type>(start, end, base, buf);
                    return Integral<__out_type>(buf * sign, code);
                } else return Integral<__out_type>(-1, NumCastCode::INVALBASE);
            } else return Integral<__out_type>(-1, NumCastCode::INVALPTR);
        }

    }
}
#endif