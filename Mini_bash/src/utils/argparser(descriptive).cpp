#include <vector>
#include <boost/regex.hpp>
#include <regex>
#include <bitset>
#include "../../include/utils/argparser(descriptive).hpp" // utls::frmt namespace
#include <iostream>
#include "../../include/utils/formatter.hpp"


bool is_posarg_subcom(const std::string& str){
    std::regex pattern("^[a-zA-Z_]+$");
    return std::regex_match(str, pattern);
}

void utls::prsr::ArgParser::_PrintSOC(){ // print series of commands
    if(parent_parser != nullptr){
        parent_parser->_PrintSOC();
    }
    std::cout << command << " ";
}
// _GetEntry
void utls::prsr::ArgParser::_PrintUsage(){
    std::cout << "usage: ";
    _PrintSOC();
    for(ArgEntry* entry : unduplicated_arg_data){
        std::cout << "[" << entry->meta.name << "] ";
    }
    std::cout << std::endl;
}
// Err_code
// Runtime Error Handler
void utls::prsr::ArgParser::_ErrorHandler(Err_code code, ArgEntry* issued_argument, const std::string& issued_token) {
    _PrintUsage();
    std::cerr << "Error: ";
    switch (code) {
        int a;
        case Err_code::ERROR_MISSING_ARG :
            std::cout << "The following argument are missing : \n";
            for(ArgEntry* entry : unduplicated_arg_data){
                if((entry->atr.flag & 3) == ArgFlag::IS_REQUIRED){
                    std::cout << "[" << entry->meta.name << "]";
                }
            }
            break;
            
        case Err_code::ERROR_MISSING_NARG :
            a = issued_argument->atr.narg;
            std::cout << "'" << issued_argument->meta.name << "' Requires " << a << " arguments";
            break;
        
        case Err_code::ERROR_UNKNOWN_ARG :
            std::cout << "Unknown argument : " << issued_token;
            break;
        
        case Err_code::ERROR_INVALID_INPUT_TO_ATTRIBUTE :
            std::cout << "Invalid attribute type for '" << issued_argument->meta.name << "' to it's input (check your input)";
            break;                
    }
    std::cout << std::endl;
    throw "Parse fail"; // Replaced for mini_bash feedback
}

void utls::prsr::ArgParser::_ArgCallback(){
    for(ArgEntry* entry : unduplicated_arg_data){
        entry->func();
    }
    func();
}

    void utls::prsr::ArgParser::_CheckAllRequiredArgIsCalled(){
        for(ArgEntry* entry : unduplicated_arg_data){
            if((entry->atr.flag & Diagnostic::REQUIRED_TO_CALLED_MASK) == ArgFlag::IS_REQUIRED){
                _ErrorHandler(Err_code::ERROR_MISSING_ARG);
            }
        }
    }

void utls::prsr::ArgParser::_ParseMultiple(std::vector<std::string>& tokens, unsigned int& token_ptr, ArgEntry& entry, std::string& curr_token){
    auto dec = (entry.atr.narg != -1) ? [](uint8_t& narg){ --narg; }:[](uint8_t& narg){};
    int nth_val = 0;
    std::cout << "\n";
    ++token_ptr;
    while((token_ptr < tokens.size()) && (entry.atr.narg != 0)){
        curr_token = tokens[token_ptr];

        if((curr_token[0] == '-') || (subcom_lookup.count(curr_token) != 0)){
            std::cout << "\nEncountered subcom or a flag\n";
            return;
        }
        if(nth_val < entry.values.size()){
            entry.values[nth_val] = curr_token;
        } else {
            entry.values.push_back(curr_token);
        }
        std::cout << "Inserted -> " << curr_token;
        utls::frmt::__show<std::string>(entry.values);
        ++nth_val;
        ++token_ptr;
        dec(entry.atr.narg);
    }
}

utls::prsr::ArgEntry* utls::prsr::ArgParser::_GetEntry(const std::string& token) {
    if (arg_lookup.count(token)) {
        std::cout << "F";
        return arg_lookup.at(token);
    }
    _ErrorHandler(Err_code::ERROR_UNKNOWN_ARG, nullptr, token);
    return nullptr;
}

void utls::prsr::ArgParser::_ParseFrom(
    // Actual logic, the params are initialized in parse_args()
    std::vector<std::string>& tokens,
    unsigned int& token_ptr,
    unsigned int& positional_ptr,
    std::string& curr_token,
    ArgEntry* entry_buff)
{
    while(token_ptr < tokens.size()){
        curr_token = tokens[token_ptr];
        std::cout << "Token : " << curr_token;
        if(curr_token[0] == '-'){ // curr_token is a possible flag
            std::cout << " F" << token_ptr;
            size_t equal_pos = curr_token.find('=');
            if(equal_pos != std::string::npos){
                std::cout << "V";
                entry_buff = _GetEntry(curr_token.substr(0, equal_pos));
                
                if((entry_buff->atr.flag & Diagnostic::TAKETYPE_MASK) == ArgFlag::STORE_CONST){
                    std::cout << "\nEntry take type is store const" << std::endl;
                    _ErrorHandler(Err_code::ERROR_INVALID_INPUT_TO_ATTRIBUTE, entry_buff);
                }
            
                if(entry_buff->values.empty()){
                    entry_buff->values.push_back(curr_token.substr(equal_pos + 1));
                } else {
                    entry_buff->values[0] = curr_token.substr(equal_pos + 1);
                }
                ++token_ptr;
                std::cout << "\n";
            } else {
                std::cout << "N";
                entry_buff = _GetEntry(curr_token);
                int old_narg = entry_buff->atr.narg;
                switch (entry_buff->atr.flag & Diagnostic::TAKETYPE_MASK)
                {
                case ArgFlag::STORE_CONST :
                    std::cout << "C";
                    entry_buff->atr.flag = entry_buff->atr.flag | Diagnostic::IS_CALLED;
                    ++token_ptr;
                    break;

                case ArgFlag::STORE_NARG :
                    std::cout << "N";
                    _ParseMultiple(tokens, token_ptr, *entry_buff, curr_token);
                    if(entry_buff->atr.narg == 0){
                        entry_buff->atr.flag = entry_buff->atr.flag | Diagnostic::IS_CALLED;
                        entry_buff->atr.narg = old_narg; // Restore narg in case of --help
                    } else {
                        _ErrorHandler(Err_code::ERROR_MISSING_NARG, entry_buff);
                    }
                    break;
                
                case ArgFlag::STORE_ANY :
                    std::cout << "A";
                    _ParseMultiple(tokens, token_ptr, *entry_buff, curr_token);
                    entry_buff->atr.flag = entry_buff->atr.flag | Diagnostic::IS_CALLED;
                    break;

                default:
                    break;
                }
                std::cout << "\n";

            }

        } else if (subcom_lookup.count(curr_token) != 0){
            _CheckAllRequiredArgIsCalled();
            ++token_ptr;
            
            subcom_lookup.at(curr_token).instance._ParseFrom(tokens, token_ptr, positional_ptr, curr_token, entry_buff);
            _ArgCallback();
            return;

        } else {
            if(positional_ptr < ordered_positionals.size()){
                std::cout << "\nEncountered Posarg > ";
                entry_buff = ordered_positionals[positional_ptr];
                int old_narg = entry_buff->atr.narg;
                switch (entry_buff->atr.flag & Diagnostic::TAKETYPE_MASK)
                {
                case STORE_NARG :
                    std::cout << "N";
                    _ParseMultiple(tokens, token_ptr, *entry_buff, curr_token);
                    if(entry_buff->atr.narg == 0){
                        ++positional_ptr;
                        entry_buff->atr.flag = entry_buff->atr.flag | Diagnostic::IS_CALLED;
                        entry_buff->atr.narg = old_narg;
                    }
                    // Don't throw exception if interrupted (ignore interruptions)
                
                case STORE_ANY :
                    std::cout << "A";
                    _ParseMultiple(tokens, token_ptr, *entry_buff, curr_token);
                    entry_buff->atr.flag = entry_buff->atr.flag | Diagnostic::IS_CALLED;
                    // We don't know what condition "any narg" positional argument supposed to finish
                    // So we are NOT incrementing positional_ptr
                default:
                    break;
                }
            } else {
                _ErrorHandler(Err_code::ERROR_UNKNOWN_ARG, nullptr, curr_token);
            }
        }

        // ++token_ptr are not used here, because it was already handled by the block code above
        
        
    }
}

void utls::prsr::ArgParser::_AddPositional(std::string& opt, ArgEntry& entry){
    opt = utls::frmt::__strip(opt, ' ');
    if(is_posarg_subcom(opt)){
        switch (entry.atr.flag & Diagnostic::TAKETYPE_MASK)
        {
        case ArgFlag::STORE_ANY :
            entry.atr.narg = -1;
            break;

        case ArgFlag::STORE_NARG :
            if(entry.atr.narg > 0){
                break;
            }
            
        default: //stdexcept if "initialization process" process
            throw std::invalid_argument("Invalid take type flag for " + opt);
            break;
        }
        entry.meta.name = opt;
        ArgEntry* entry_ptr = new ArgEntry(entry);
        entry_ptr->meta.name = opt;
        ordered_positionals.push_back(entry_ptr);
        arg_lookup.insert({opt, entry_ptr});
    } else {
        throw std::invalid_argument(opt + " does not match a positional argument pattern");
    }
}

void utls::prsr::ArgParser::_FlagVerifyNarg(ArgEntry& entry){

    std::cout << "Entry : " << std::bitset<8>(entry.atr.flag) << "\n" << "Mask : " << std::bitset<8>(TAKETYPE_MASK) << "\n";
    switch (entry.atr.flag & Diagnostic::TAKETYPE_MASK)
        {
        case ArgFlag::STORE_CONST :
            // We don't need to care about the nargs. but we do need to care abt constant type
            try{
                std::any_cast<None>(entry.constant_val);
                throw std::invalid_argument("Constant type shouldn't be none");
            } catch (std::bad_any_cast& msg){}
            std::cout << "Qualified opt as store const\n";
            break;
        
        case ArgFlag::STORE_NARG :
            if(entry.atr.narg == 0){
                throw std::invalid_argument("STORE_NARG flag shouldn't be paired with 0 nargs");
            }
            std::cout << "Qualified opt as store narg";
            break;

        case ArgFlag::STORE_ANY :
            entry.atr.narg = -1;
            std::cout << "Qualified opt as store any";
            break;

        default:
            throw std::invalid_argument("Invalid take type flag");
            break;
        }

}

void utls::prsr::ArgParser::_AddFlag(std::string& opt, ArgEntry& entry){
    size_t comma_pos = opt.find(',');
        
    std::regex short_pattern("^-[a-zA-Z]$");
    std::regex long_pattern("^--[a-zA-Z]+[a-zA-Z0-9_-]*$");
    if(comma_pos != std::string::npos){
        std::cout << "Opt is two flag\n";
        std::string short_flag = utls::frmt::__strip(opt.substr(0, comma_pos), ' ');
        std::string long_flag = utls::frmt::__strip(opt.substr(comma_pos + 1), ' ');
        if(!short_flag.empty() && !long_flag.empty()){
            if(std::regex_match(short_flag, short_pattern) && std::regex_match(long_flag, long_pattern)){
                _FlagVerifyNarg(entry);
                entry.meta.name = short_flag + " | " + long_flag;
                ArgEntry* entry_ptr = new ArgEntry(entry); // more than one instance are using the same entry
                unduplicated_arg_data.push_back(entry_ptr);
                arg_lookup.insert({short_flag, entry_ptr});
                arg_lookup.insert({long_flag, entry_ptr});
            } else {
                throw std::invalid_argument(opt + " Does not match long and short flag pattern");
            }
        } else {
            throw std::invalid_argument("Empty option '" + opt + "' are not permitted");
        }
    } else {
        std::cout << "opt is a single flag\n";
        opt = utls::frmt::__strip(opt, ' ');
        switch (opt.find_first_not_of('-'))
        {
        case 1: // short flag
            if(!std::regex_match(opt, short_pattern)){
                throw std::invalid_argument("Invalid short flag pattern");
            }
            std::cout << "Qualified pattern as short flag pattern\n";
            break;
        
        case 2 : // long flag
            if(!std::regex_match(opt, long_pattern)){
                throw std::invalid_argument("Invalid long flag pattern");
            }
            std::cout << "Qualified pattern as long flag pattern\n";
            break;
        
        default:
            throw std::invalid_argument("Unknown flag format");
            break;
        }
        entry.meta.name = opt;
        _FlagVerifyNarg(entry);
        ArgEntry* entry_ptr = new ArgEntry(entry);
        unduplicated_arg_data.push_back(entry_ptr);
        arg_lookup.insert({opt, entry_ptr});
    }
}

    void utls::prsr::ArgParser::add_argument(
        std::string opt,
        uint8_t flag,
        uint8_t narg,
        std::string metavar,
        std::string desc,
        std::function<void()> fn,
        std::any constant,
        std::any def
    ){
        metavar = utls::frmt::__strip(metavar, ' ');
        ArgEntry entry {};
        entry.atr.flag = flag & -2; // 2's complement, -2 represent in byte ...11110
        entry.atr.narg = narg;
        entry.meta.metavar = metavar;
        entry.meta.help = desc;
        entry.func = fn;
        entry.constant_val = constant;
        entry.default_val = def;
        // Make narg check easier for the add_positional or add_flag
        if(narg >= 0){
            std::cout << "Entered flag is : " << std::bitset<8>(entry.atr.flag) << "\n";
            if((entry.atr.flag & Diagnostic::ARGTYPE_MASK) == ArgFlag::FLAG){
                std::cout << "Adding : " << opt << " as flag\n"; 
                _AddFlag(opt, entry);
            } else {
                std::cout << "Adding : " << opt << " as positional\n";
                _AddPositional(opt, entry);
            }
        } else {
            throw std::invalid_argument("Invalid narg, the condition must be narg >= 0");
        }
    }

    const std::unordered_map<std::string, utls::prsr::ArgEntry*>* utls::prsr::ArgParser::get_arg_lookup() const noexcept {
        return &arg_lookup;
    }

    const std::unordered_map<std::string, utls::prsr::SubComEntry>* utls::prsr::ArgParser::get_subcom_lookup() const noexcept {
        return &subcom_lookup;
    }

    utls::prsr::ArgParser* utls::prsr::ArgParser::add_subcommand(std::string cmnd, std::string help, std::function<void()> fn){
        if(is_posarg_subcom(cmnd)){
            auto res = subcom_lookup.emplace(cmnd, SubComEntry(cmnd, fn, this, help));
            if(res.second){
                return &res.first->second.instance;
            } else {
                throw std::runtime_error("Failed inserting sub-command entry");
            }
        } else {
            throw std::invalid_argument("Invalid command regex pattern");
        }
    }

    void utls::prsr::ArgParser::parse_main(std::vector<std::string> tokens){ 
        unsigned int token_ptr = 0;
        unsigned int positional_ptr = 0;
        ArgEntry* entry_buff;
        std::string curr_token;
        _ParseFrom(tokens, token_ptr, positional_ptr, curr_token, entry_buff);
        
    }






