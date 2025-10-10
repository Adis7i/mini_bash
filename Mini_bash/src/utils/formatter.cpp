#include "../../include/utils/formatter.hpp"
#include <cmath>
#include <iostream>
// utls::frmt::notApi

std::string utls::frmt::normalize(std::string inp_path){
    if(inp_path.empty()){ return ""; }
    std::vector<std::string> path_token;
    std::string token;
    bool lislash = false;
    int n = inp_path.length();
    int left = 0;

    for(int i = 0; i <= n; i++){
        if(inp_path[i] == '/' || (i == n)){
            if(!lislash){
                token = inp_path.substr(left, i - left);
                if(token == "."){}
                else if (token == ".."){
                    if(path_token.back() != "") { // This is just some extra implementation on useful bugs
                        path_token.pop_back();
                    }
                }
                else {
                    path_token.push_back(token);
                }
                lislash = true;
            }
            left = i + 1;
        } else {
            lislash = false;
        }
    }

    token.clear();
    n = path_token.size();
    for(short i = 0; i < n; i++){
        token += path_token[i] + "/";
    }
    if((token.back() == '/') && (token.length() > 1)){token.pop_back();}
    return token;
}

std::vector<std::string> utls::frmt::split(std::string &inp_str, char sep){
    std::vector<std::string> res;
    std::string stripped_input = utls::frmt::strip(inp_str, sep);
    unsigned int left = 0;
    int right = -1;
    while((right = stripped_input.find(sep, right + 1)) != std::string::npos){
        res.push_back(stripped_input.substr(left, right - left));
        left = right + 1;
    }
    if(left != stripped_input.length()){
        res.push_back(stripped_input.substr(left, stripped_input.length() - left));
    }
    return res;
    
}

std::string utls::frmt::strip(const std::string &inp_str, char sep){
    int left = inp_str.find_first_not_of(sep);
    int right = inp_str.find_last_not_of(sep);
    if((left == std::string::npos) || (right == std::string::npos)){
        return "";
    }
    return inp_str.substr(left, right - left + 1);
}

bool utls::frmt::startswith(std::string &str, const char* sstr) {
    return str.find(sstr) == 0;
}

void show(std::vector<std::string>& token){
    std::cout << "[";
    for(auto c : token){
        std::cout << c << ", ";
    }
    std::cout << "]" << std::endl;
}

bool utls::frmt::valid_long_name(const char* _start){
    if(_start == nullptr) return false;
    unsigned char c;
    while((c = *_start) != '\0') {
        if(!valid_char_table[c]) return false;
        ++_start;
    }
    return true;
}

bool utls::frmt::valid_short_name(unsigned char name) {
    return (name == '_') ? false : (valid_char_table[name]);
}
