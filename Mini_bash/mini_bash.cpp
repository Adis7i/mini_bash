#include "include/module/handlers.hpp"
#include "include/utils/formatter.hpp"
#include "include/utils/parser.hpp"
#include <string>
#include <vector>


int main(){
    utls::prsr::Parser parser(1);
    init::lsdir_init(parser);
    init::cat_init(parser);
    std::string arg;
    while(true){
        std::cout << "Enter argument => ";
        std::getline(std::cin, arg);
        if(arg == "exit") break;
        std::vector<std::string> argv = utls::frmt::split(arg, ' ');
        auto beg = argv.begin();
        parser.orchestra(beg, argv.end());
        utls::prsr::print_parsing_message(parser.parsing_status());
        parser.reset();
    }
    
}
