#include "include/utils/argparser(descriptive).hpp"
#include <iostream>

int main(){
    utls::prsr::ArgParser parser;
    parser.add_argument("--files", utls::prsr::ArgFlag::FLAG | utls::prsr::ArgFlag::STORE_NARG | utls::prsr::ArgFlag::IS_REQUIRED, 2);
    parser.parse_main({"--files", "abc", "bca"});
    auto lookup = parser.get_arg_lookup();
    auto entry = parser._GetEntry("--files");
    for(auto elmnt : entry->values){
        std::cout << elmnt << " ";
    }
    std::cout << std::endl;
    return 0;
}