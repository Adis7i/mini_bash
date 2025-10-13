#include <iostream>
#include <iomanip>
#include "../../include/utils/parser.hpp"
#include "../../include/utils/formatter.hpp"

using namespace utls::prsr;

    // Return how many argument were parsed
int Parser::multi_parse(
    bool (*token_check)(std::string_view& v),
    std::vector<std::string>::iterator& iter,
    const std::vector<std::string>::iterator& end,
    Option*& opt)
{
    
    std::string_view viewer;
    int left_to_parse = opt->narg;
    int parsed_count = 0;
    while ((iter != end) && (left_to_parse != 0)) {
        viewer = *iter;        
        if(token_check(viewer)) return parsed_count;
        opt->values.push_back(viewer.data());
        std::cout << "Push back : " << viewer << std::endl;
        ++iter;
        --left_to_parse;
        ++parsed_count;
    }
    return parsed_count;
}

    ParseFlowStat Parser::handle_flag(
        std::vector<std::string>::iterator& iter,
        const std::vector<std::string>::iterator& end,
        std::string_view& viewer, Option*& opt
    ){
        if((viewer == "-h") || (viewer == "--help")) {
            help();
            return ParseFlowStat::TrueHalt;
        }
        switch (viewer.find_first_not_of('-')) {
            case 1 : 
                if(viewer.size() != 2) { // do not allow multiple flag like -abc
                    std::cout << "viewer : " << viewer << std::endl;
                    parsing_status_ = ParsingStatus::UnexpectedToken;
                    return ParseFlowStat::TrueHalt;
                } // Invalid short flag

                if((opt = get_short_flag(viewer[1])) == nullptr) {
                    parsing_status_ = ParsingStatus::UnknownOption;
                    return ParseFlowStat::TrueHalt;
                    break;
                } // Unknown short flag                
                break;
            
            case 2 :
                if((opt = get_long_flag(viewer.data() + 2)) == nullptr){
                    parsing_status_ = ParsingStatus::UnknownOption;
                    return ParseFlowStat::TrueHalt;
                }
                break;

            default:
                if(viewer == "--") {
                    return ParseFlowStat::Halt;
                }
                break;
        }
        int narg_parsed = NArgType::CallOnly;

        if(opt->narg != NArgType::CallOnly) {
            narg_parsed = multi_parse([](std::string_view& viewer){
                return (viewer[0] == '-');
            }, iter, end, opt);
        }
        
        if(narg_parsed != opt->narg) {
            parsing_status_ = ParsingStatus::UnsatisfiedNarg;
            return ParseFlowStat::TrueHalt;
        } else opt->isCalled = true;

        return ParseFlowStat::Continue;
    }

    // Return a status wether the whole parsing should halt or not
    // true to continue, false to halt
    ParseFlowStat Parser::parse_option(
        std::vector<std::string>::iterator& iter,
        const std::vector<std::string>::iterator& end)
    {
        
        std::string_view viewer;
        Option* opt;
        ParseFlowStat flow;
        while(iter != end) {
            viewer = *iter;
            
            ++iter;
            if((viewer[0] != '-')) {
                
                if(subcom_lookup_.count(viewer.data()) != 0){
                    --iter;
                    return SubcomHalt;
                }
                dump.push_back(viewer.data());
                std::cout << "Dumping : " << viewer << std::endl;
                

            } else if((flow = handle_flag(iter, end, viewer, opt)) != Continue){
                return flow;
            }
        }
        return ParseFlowStat::Continue;
    }

    ParseFlowStat Parser::parse_posarg(){
        auto dump_iter = dump.begin();
        auto posarg_iter = posarg_.begin();
        Option* opt;
        
        
        while ((dump_iter != dump.end()) && (posarg_iter != posarg_.end())) {
            
            opt = *posarg_iter;

            int narg_parsed = multi_parse([](std::string_view& viewer){
                return false;
            }, dump_iter, dump.end(), opt);

            if(narg_parsed < opt->narg) {
                parsing_status_ = ParsingStatus::UnsatisfiedNarg;
                return ParseFlowStat::TrueHalt;
            }
            ++posarg_iter;
            opt->isCalled = true;
        }

        if(dump_iter != dump.end()){
            parsing_status_ = ParsingStatus::UnexpectedToken;
            return ParseFlowStat::TrueHalt;
        }

        if(posarg_iter != posarg_.end()){
            parsing_status_ = ParsingStatus::LeftToCall;
            return ParseFlowStat::TrueHalt;
        }

        return ParseFlowStat::Continue;
    }

    bool Parser::finalize(){
        // Function might be related from one to another since it is std::function
        // We don't want a half-completed user-defined procedure
        for(auto& opt : options_){
            if(opt.is_required && !opt.isCalled) {
                parsing_status_ = ParsingStatus::LeftToCall;
                return false;
            }
        }
        for(auto& opt : options_) {            
            if(opt.isCalled) opt.callback(opt);
        }
        return true;
    }

    void Parser::shortcut_print_narg(Option& opt) {
        switch (opt.narg)
        {
        case NArgType::Any :
            std::cout << "<Any>  " << opt.desc << "\n";
            break;
        
        case NArgType::CallOnly :
            std::cout << "<Call> " << opt.desc << "\n";
            break;

        default:
            std::cout << "<" << opt.narg << ">    " << opt.desc << "\n";
            break;
        }
    }
    
    void Parser::help() {
        std::cout << prolouge << "\nFlags : \n";
        for(auto& opt : options_){
            if(opt.opt_type == ArgType::Flag) {
                std::cout << (opt.is_required ? "[!]" : "[*]") << " " << opt.short_name << ", "
                    << opt.long_name << " ";
                shortcut_print_narg(opt);
            }
        }
        std::cout << "\nPositionals : \n";
        for(auto& opt : posarg_) {  
            std::cout << (opt->is_required ? "[!] " : "[*] ");
            std::cout << opt->long_name << " ";
            shortcut_print_narg(*opt);
        }
        std::cout << "\n" << epilouge << std::endl;
    }    
    
    // Cool name ain't it ?, Using bool as return type
    // This is to ease up a way for user to know if anythings wrong
    bool Parser::orchestra(
        std::vector<std::string>::iterator& beg,
        const std::vector<std::string>::iterator& end){
        
        parsing_status_ = ParsingStatus::Fine;
        bool parse_stat;
        ParseFlowStat stat;
        
        if((stat = parse_option(beg, end)) == TrueHalt) return false;
        if(parse_posarg() == TrueHalt) return false;
        
        parse_stat = finalize();

        if(!parse_stat) return parse_stat;
        else if (stat == SubcomHalt){
            auto sbc_iter = subcom_lookup_.find(*beg);
            
            if(sbc_iter != subcom_lookup_.end()) {
                ++beg;
                auto data_ptr = sbc_iter->second.get();
                data_ptr->isCalled = true;
                bool subparser_flow_stat = data_ptr->parser_obj.orchestra(beg, end);                
                if(subparser_flow_stat) data_ptr->callback(*data_ptr);
                print_parsing_message(data_ptr->parser_obj.parsing_status());
                return subparser_flow_stat;
            } else {
                parsing_status_ = ParsingStatus::UnexpectedFailure;
                return false;
            }
        }
        return true;
    }

    Option* Parser::get_short_flag(char _name) {
        auto it = short_lookup_.find(_name);
        return ((it == short_lookup_.end()) ? nullptr : it->second);
    }

    Option* Parser::get_long_flag(const char* _name) {
        auto it = long_lookup_.find(_name);
        return ((it == long_lookup_.end()) ? nullptr : it->second);
    }

    Option* Parser::get_posarg(const char* _name) {
        auto it = posarg_lookup_.find(_name);
        return ((it == posarg_lookup_.end()) ? nullptr : it->second);
    }
    
    Option* Parser::add_option(int _narg, char _short_name){
        if(options_.capacity() == options_.size()) {
            other_status_ = ParserErrorCode::StorageFull;
            return nullptr;
        }
        if(!utls::frmt::valid_short_name(_short_name) || (_narg < NArgType::Any)){
            other_status_ = ParserErrorCode::InvalArg;
            return nullptr;
        }

        options_.push_back(Option(_narg, _short_name, "", ArgType::Flag));
        Option* ptr = &options_.back();
        if(!short_lookup_.emplace(_short_name, ptr).second){
            options_.pop_back();
            other_status_ = ParserErrorCode::NameExist;
            return nullptr;
        }
        return ptr;
    }

    Option* Parser::add_option(int _narg, const char* _long_name){
        other_status_ = ParserErrorCode::Good;
        if(options_.capacity() == options_.size()){
            other_status_ = ParserErrorCode::StorageFull;
            return nullptr;
        }

        
        if(!utls::frmt::valid_long_name(_long_name) || (_narg < NArgType::Any)) {
            other_status_ = ParserErrorCode::InvalArg;
            return nullptr;
        }

        options_.push_back(Option(_narg, 0, _long_name, ArgType::Flag));
        Option* ptr = &options_.back();
        if(!long_lookup_.emplace(std::string(_long_name), ptr).second){
            options_.pop_back();
            return nullptr;
        }
        return ptr;
    }

    Option* Parser::add_option(int _narg, char _short_name, const char* _long_name){
        other_status_ = ParserErrorCode::Good;
        if(options_.capacity() == options_.size()){
            other_status_ = ParserErrorCode::StorageFull;
            return nullptr;
        }

        if(!utls::frmt::valid_long_name(_long_name) ||
           !utls::frmt::valid_short_name(_short_name) ||
           (_narg < NArgType::Any)) 
        {
            other_status_ = ParserErrorCode::InvalArg;
            return nullptr;
        }

        options_.push_back(Option(_narg, _short_name, _long_name, ArgType::Flag));
        Option* ptr = &options_.back();
        if(!long_lookup_.emplace(std::string(_long_name), ptr).second ||
           !short_lookup_.emplace(_short_name, ptr).second){
               options_.pop_back();
               return nullptr;
        }
        return ptr;
    }

    // basically have the same procedure as long flag overload variant of add_option
    // The difference is posarg are not allowed to be only call
    Option* Parser::add_posarg(int _narg, const char* _name){
        other_status_ = ParserErrorCode::Good;
        if((_narg == NArgType::CallOnly) || last_is_any_) {
            other_status_ = ParserErrorCode::InvalArg;
            return nullptr;
        } 
                
        if(options_.capacity() == options_.size()){
            other_status_ = ParserErrorCode::StorageFull;
            return nullptr;
        }

        if(!utls::frmt::valid_long_name(_name) || (_narg < NArgType::Any)) {
            other_status_ = ParserErrorCode::InvalArg;
            return nullptr;
        }

        options_.push_back(Option(_narg, 0, _name, ArgType::Positional));
        Option* ptr = &options_.back();
        ptr->is_required = true;
        if(!posarg_lookup_.emplace(std::string(_name), ptr).second){
            options_.pop_back();
            return nullptr;
        }
                
        if(_narg == NArgType::Any) last_is_any_ = true;
        posarg_.push_back(ptr);
        return ptr;
    }
    
    Subcom* Parser::add_subcom(const char* _name, size_t _alloc_arg){
        std::unique_ptr<Subcom> ptr = std::make_unique<Subcom>(_alloc_arg);
        Subcom* data_ptr = ptr.get();        
        if(!subcom_lookup_.emplace(std::string(_name), std::move(ptr)).second){
            
            other_status_ = ParserErrorCode::NameExist;
            return nullptr;
        }
        return data_ptr;
    }
    
    bool Parser::storage_full() const noexcept { return options_.capacity() == options_.size(); }
    bool Parser::last_is_any() const noexcept { return last_is_any_; }
    ParsingStatus Parser::parsing_status() const noexcept { return parsing_status_; }
    ParserErrorCode Parser::parser_status() const noexcept { return other_status_; }
    void Parser::reset() noexcept { // void reset() noexcept;
        for(auto& opt : options_){
            opt.values.clear();
        }

        for(auto& sbc : subcom_lookup_){
            sbc.second.get()->parser_obj.reset();
        }
        dump.clear();
        other_status_ = ParserErrorCode::Good;
        parsing_status_ = ParsingStatus::Fine;
    }

void utls::prsr::print_parsing_message(ParsingStatus status) {
    std::cout << "Error: ";
    switch (status) {
        case ParsingStatus::UnexpectedToken:
            std::cout << "Unexpected token provided" << std::endl;
            break;

        case ParsingStatus::UnknownOption:
            std::cout << "Unknown option" << std::endl;
            break;

        case ParsingStatus::UnsatisfiedNarg:
            std::cout << "Missing argument for an option/posarg" << std::endl;
            break;

        case ParsingStatus::LeftToCall:
            std::cout << "Required option/posarg was not called" << std::endl;
            break;

        case ParsingStatus::Fine:
            std::cout << "Parsing completed successfully" << std::endl;
            break;

        case ParsingStatus::UnexpectedFailure :
            std::cout << "Unexpected failure" << std::endl;
            break;
        
        default:
            std::cout << "What am I supposed to tell if you give me sum random things ?" << std::endl;
            break;
    }
}

void utls::prsr::print_parser_message(ParserErrorCode status){
    std::cout << "Parser : ";
    switch (status)
    {
    case ParserErrorCode::NameExist :
        std::cout << "Name exist ! " << std::endl;
        break;
    
    case ParserErrorCode::Good :
        std::cout << "Good !" << std::endl;
        break;
    
    case ParserErrorCode::InvalArg :
        std::cout << "Invalid argument ! " << std::endl;
        break;
    
    case ParserErrorCode::StorageFull :
        std::cout << "Storage full !" << std::endl;
        break;
    
    case ParserErrorCode::AnyBeforeNumbered :
        std::cout << "Posarg with Narg type of 'Any' are forbid to be not at the end of the posarg order" << std::endl;
        break;

    default:
        std::cout << "Unknown Error Code" << std::endl;
        break;
    }
}