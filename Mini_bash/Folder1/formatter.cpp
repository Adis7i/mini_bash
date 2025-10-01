#include "formatter.hpp"
#include <cmath>
#include <iostream>

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

std::vector<std::string> utls::frmt::__split(std::string &inp_str, char sep){
    std::vector<std::string> res;
    std::string stripped_input = utls::frmt::__strip(inp_str, sep);
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

std::string utls::frmt::__strip(const std::string &inp_str, char sep){
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

// prototypes function

bool utls::frmt::str_bool(std::string val){
    val = utls::frmt::__strip(val, ' ');
    for(int i = 0; i < val.length(); i++){
        val[i] = std::tolower(val[i]);
    }
    if(val == "true"){
        return true;
    } else if (val == "false"){
        return false;
    } else {
        throw std::invalid_argument("");
    }
}

double utls::frmt::str_dob(std::string val){
    val = utls::frmt::__strip(val, ' ');
    double res = 0;
    short n_now;
    float div = 1;
    bool comma = false;

    for(int i = val.length() - 1; i >= 0; i--){
        n_now = val[i] - 48;
        if((0 <= n_now) && (n_now <= 9)){
            res += n_now * div;
            div *= 10;
        } else if ((n_now == -2) && !comma){
            res /= div;
            div = 1;
            comma = true;
        } else {
            std::cerr << "Invalid character found : " << val[i] << std::endl;
            throw "Invalid format";
        }
    
    }
    
    return res;
}

void __show(std::vector<std::string>& token){
    std::cout << "[";
    for(auto c : token){
        std::cout << c << ", ";
    }
    std::cout << "]" << std::endl;
}

int utls::frmt::str_int(std::string val){
    int res = 0;
    char c_now;
    for(int i = 0; i < val.length(); i++){
        
        c_now = val[i] - 48;
        if((0 <= c_now) && (c_now <= 9)){
            res = (res * 10) + c_now;
        } else {
            c_now += 48;
            std::cerr << "Invalid character found : " << c_now << std::endl;
            throw "Invalid format";
        }
       
    }
    return res;
}

