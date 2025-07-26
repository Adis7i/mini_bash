#include <string>
#include "../include/common_types.hpp"

bool FSArg::addPath(std::string path){
    if(path1.empty()){
        path1 = path;
        return true;
    } else if(path2.empty()){
        path2 = path;
        return true;
    }
    return false;
}

const std::string* FSArg::gpath1() const {
    return &path1;
}

const std::string* FSArg::gpath2() const {
    return &path2;
}