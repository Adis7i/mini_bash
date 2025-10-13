#include "../../include/module/cat.h"
#include "../../include/module/lsdir.hpp"
#include "../../include/module/move.h"
#include "../../include/utils/formatter.hpp"
#include "../../include/module/handlers.hpp"
#include <iostream>
// cpp
using namespace utls;




void init::lsdir_init(prsr::Parser& parser){
    std::cout << "Initializing lsdir parser" << std::endl;
    prsr::Option* opt;
    prsr::Subcom* sbc_ptr = parser.add_subcom("ls", 4);
    prsr::Parser& parser_obj = sbc_ptr->parser_obj;
    parser_obj.prolouge = "list entries within a folder";

    opt = parser_obj.add_posarg(1, "path");
    opt->desc = "Absolute POSIX-path to a folder";
    prsr::print_parser_message(parser_obj.parser_status());
    
    opt = parser_obj.add_option(prsr::NArgType::CallOnly, 'p', "perm");
    prsr::print_parser_message(parser_obj.parser_status());
    opt->desc = "Print permission code";
    
    opt = parser_obj.add_option(prsr::NArgType::CallOnly, 'c', "count_entry");
    prsr::print_parser_message(parser_obj.parser_status());
    opt->desc = "Print how many entries within a folder or file (1)";

    opt = parser_obj.add_option(prsr::NArgType::CallOnly, 's', "size");
    prsr::print_parser_message(parser_obj.parser_status());
    opt->desc = "Print how big a file/folder"; 

    sbc_ptr->callback = [](prsr::Subcom& dat){
        if(dat.parser_obj.parsing_status() == prsr::ParsingStatus::Fine){
            prsr::Option* perm = dat.parser_obj.get_short_flag('p');
            prsr::Option* count_entry = dat.parser_obj.get_short_flag('c');
            prsr::Option* size = dat.parser_obj.get_short_flag('s');
            prsr::Option* path = dat.parser_obj.get_posarg("path");
            if(!perm || !count_entry || !size || !path){ // If at least one pointers are null
                std::cerr << "[lsdir parse fail]" << std::endl;
                return;
            }
            unsigned char flag;
            if(perm->is_called()) flag = flag | lsFlag::FPERM;
            if(count_entry->is_called()) flag = flag | lsFlag::FENTCO;
            if(size->is_called()) flag = flag | lsFlag::FSIZE;
            lsdir(path->values[0].c_str(), flag);
        }
    };
}

void init::cat_init(prsr::Parser& parser){
    std::cout << "Initializing cat" << std::endl;
    prsr::Subcom* sbc_ptr = parser.add_subcom("cat", 1);
    prsr::Parser& parser_obj = sbc_ptr->parser_obj;
    parser_obj.prolouge = "\nLiterally a curios cat that wanders on a file !";
    parser_obj.add_posarg(1, "path");
    
    prsr::print_parser_message(parser_obj.parser_status());
    sbc_ptr->callback = [](prsr::Subcom& dat){
        cat(dat.parser_obj.get_posarg("path")->values[0].c_str());
    };
}

