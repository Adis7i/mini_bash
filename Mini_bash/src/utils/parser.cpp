#include "../../include/utils/parser.hpp"

void util::parser::profMap(const util::parser::arg_s* profile){
    
    std::cout << "[ Map Profile ]\nisCalled : " << profile->called <<
        "\nNarg : " << profile->narg << "\nisRequired : " << profile->required << "\nVal => ";
    
    for(auto dat : profile->val){
        std::cout << dat << " ";
    }
    std::cout << std::endl;
}

void util::parser::print_parse_status(ParsingStat status){
    std::cout << "(Parsing) ";
    switch (status)
    {
    case ParsingStat::FINE :
        std::cout << "Parsing running smoothly" << std::endl;
        break;
    
    case ParsingStat::INSUFFICIENT_NARG_PASSED :
        std::cout << "You didn't pass enough narg" << std::endl;
        break;

    case ParsingStat::NO_OP :
        std::cout << "This parser haven't done anything yet" << std::endl;
        break;
    
    case ParsingStat::UNCALLED_REQUIRED :
        std::cout << "There's still some required argument left to call" << std::endl;
        break;
    
    case ParsingStat::UNKOWN_TOKEN :
        std::cout << "This parser fetched an unknown token" << std::endl;
        break;

    default:
        std::cout << "Idk bro you gave me the wrong thing" << std::endl;
        break;
    }
}

void util::parser::print_status(ParserStat status){
    std::cout << "(Parser) ";
    switch (status)
    {
    case ParserStat::CONSTRUCT_GOOD :
        std::cout << "Construct good !" << std::endl;
        break;
    
    case ParserStat::GOOD :
        std::cout << "Operation good ! " << std::endl;
        break;
    
    case ParserStat::FETCH_DATA_BAN :
        std::cout << "You're on fetch phase of the cycle !" << std::endl;
        break;
    
    case ParserStat::OVERFLOW_BAN :
        std::cout << "Storage is full !" << std::endl;
        break;
    
    case ParserStat::INVAL_ARG :
        std::cout << "You entered the wrong arg for the methods" << std::endl;
        break;
    
    case ParserStat::NAME_EXIST :
        std::cout << "Such a name exist !" << std::endl;
        break;

    default:
        std::cout << "idk Bro" << std::endl;
        break;
    }
}