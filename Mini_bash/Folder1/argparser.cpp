#include <vector>
#include <boost/regex.hpp>
#include <regex>
#include "formatter.hpp" // utls::frmt namespace
#include <iostream>
#include "argparser.hpp"

/**
 * 0 0 0 0 0 (read from the right)
 * 
 * 1st bit = is_called
 * 2nd bit = required
 * 3-4rd bit = take mode
 *      |-> 00 = store constant (doesn't make it do anything technically :P)
 *      |-> 01 = store certain nargs (number of arguments)
 *      |-> 10 = store any nargs 
 *    
 * 5th bit = Argument type
 *      |-> 1 = Positionals
 *      |-> 0 = Flag
 * 
 */

// User testimonies (mostly not used in the complex implementation)
bool is_long_flag(const std::string& str){
    std::regex pattern("^-[a-zA-Z]+[a-zA-Z0-9_-]*$");
    return std::regex_match(str, pattern);
}

bool is_short_flag(const std::string& str){
    std::regex pattern("^-[a-zA-Z]$");
    return std::regex_match(str, pattern);
}

bool is_posarg_subcom(const std::string& str){
    std::regex pattern("^[a-zA-Z_]+$");
    return std::regex_match(str, pattern);
}

    void ArgParser::_PrintSOC(){ // print series of commands
        if(parent_parser != nullptr){
            parent_parser->_PrintSOC();
        }
        std::cout << command << " ";
    }

    void ArgParser::_PrintUsage(){
        std::cout << "usage: ";
        _PrintSOC();
        for(ArgEntry* entry : unduplicated_arg_data){
            switch (entry->atr.flag & ArgFlagDiagnostic::ARGTYPE_MASK)
            {
            case ArgFlag::FLAG :
                std::cout << "[";
                if(!entry->meta.opt1.empty()){
                    std::cout << entry->meta.opt1 << ((!entry->meta.opt2.empty()) ? (" | " + entry->meta.opt2):"");
                } else {
                    std::cout << entry->meta.opt2;
                }
                std::cout << "] ";
                break;
            case ArgFlag::POSITIONAL :
                std::cout << "[" << entry->meta.opt1 << "] ";
            default:
                break;
            }
        }
    }
    // Runtime Error Handler
    void ArgParser::_ErrorHandler(ArgErrorCode code, ArgEntry* issued_argument, const std::string& issued_token) {
        _PrintUsage();
        std::cerr << "Error: ";
        switch (code) {
            case ArgErrorCode::ERROR_MISSING_ARG :
                std::cout << "The following argument are missing : ";
                for(ArgEntry* entry : unduplicated_arg_data){
                    if((entry->atr.flag & 3) == ArgFlag::IS_REQUIRED){
                        std::cout << ((!entry->meta.opt1.empty()) ? entry->meta.opt1:entry->meta.opt2) << " ";
                    }
                }
                break;
                
            case ArgErrorCode::ERROR_MISSING_NARG :
                std::cout << ((!issued_argument->meta.opt1.empty()) ? issued_argument->meta.opt1:issued_argument->meta.opt2) << " Required argument : " << issued_argument->atr.narg;
                break;
            
            case ArgErrorCode::ERROR_UNKNOWN_ARG :
                std::cout << "Unknown argument : " << issued_token;
                break;
            
            case ArgErrorCode::ERROR_INVALID_INPUT_TO_ATTRIBUTE :
                std::cout << "Invalid attribute type from " << ((!issued_argument->meta.opt1.empty()) ? issued_argument->meta.opt1:issued_argument->meta.opt2) << " to it's input (check your input)";
                break;                
        }
        std::cout << std::endl;
        throw std::runtime_error("Parse fail");
    }

    void ArgParser::_ArgCallback(){
        for(ArgEntry* entry : unduplicated_arg_data){
            entry->func();
        }
        func();
    }

    void ArgParser::_CheckAllRequiredArgIsCalled(){
        for(ArgEntry* entry : unduplicated_arg_data){
            if((entry->atr.flag & ArgFlagDiagnostic::REQUIRED_TO_CALLED_MASK) == ArgFlag::IS_REQUIRED){
                _ErrorHandler(ArgErrorCode::ERROR_MISSING_ARG);
            }
        }
    }

    void ArgParser::_ParseMultiple(std::vector<std::string>& tokens, unsigned int& token_ptr, ArgEntry& entry, std::string& curr_token){
        auto dec = (entry.atr.narg != -1) ? [](int& narg){ --narg; }:[](int& narg){};
        int nth_val = 0;

        while((token_ptr < tokens.size()) && (entry.atr.narg != 0)){
            curr_token = tokens[token_ptr];

            if((curr_token[0] == '-') || (subcom_lookup.count(curr_token) != 0)){
                return;
            }

            if(nth_val < entry.values.size()){
                entry.values[nth_val] = curr_token;
            } else {
                entry.values.push_back(curr_token);
            }

            ++token_ptr;
            ++nth_val;
        }
    }

    ArgEntry* ArgParser::_GetEntry(const std::string& token) {
        if (arg_lookup.count(token)) {
            return arg_lookup.at(token);
        }
        _ErrorHandler(ArgErrorCode::ERROR_UNKNOWN_ARG, nullptr, token);
        return nullptr;
    }
    // Actual logic, the params are initialized in parse_args()
    void ArgParser::_ParseFrom(
        std::vector<std::string>& tokens,
        unsigned int& token_ptr,
        unsigned int& positional_ptr,
        std::string& curr_token,
        ArgEntry* entry_buff)
    {
        while(token_ptr < tokens.size()){
            curr_token = tokens[token_ptr];
            
            if(curr_token[0] == '-'){ // curr_token is a possible flag
                
                size_t equal_pos = curr_token.find('=');
                if(equal_pos != std::string::npos){
                    entry_buff = _GetEntry(curr_token.substr(0, equal_pos));
                    
                    if((entry_buff->atr.flag & ArgFlagDiagnostic::TAKETYPE_MASK) == ArgFlag::STORE_CONST){
                        _ErrorHandler(ArgErrorCode::ERROR_INVALID_INPUT_TO_ATTRIBUTE, entry_buff);
                    }
                
                    if(entry_buff->values.empty()){
                        entry_buff->values.push_back(curr_token.substr(equal_pos + 1));
                    } else {
                        entry_buff->values[0] = curr_token.substr(equal_pos + 1);
                    }
                    ++token_ptr;

                } else {
                    entry_buff = _GetEntry(curr_token);
                    int old_narg = entry_buff->atr.narg;
                    
                    switch (entry_buff->atr.flag & ArgFlagDiagnostic::TAKETYPE_MASK)
                    {
                    case ArgFlag::STORE_CONST :
                        entry_buff->atr.flag = entry_buff->atr.flag | ArgFlagDiagnostic::IS_CALLED;
                        ++token_ptr;
                        break;

                    case ArgFlag::STORE_NARG :
                        _ParseMultiple(tokens, token_ptr, *entry_buff, curr_token);
                        if(entry_buff->atr.narg == 0){
                            entry_buff->atr.flag = entry_buff->atr.flag | ArgFlagDiagnostic::IS_CALLED;
                            entry_buff->atr.narg = old_narg; // Restore narg in case of --help
                        } else {
                            _ErrorHandler(ArgErrorCode::ERROR_MISSING_NARG, entry_buff);
                        }
                    
                    case ArgFlag::STORE_ANY :
                        _ParseMultiple(tokens, token_ptr, *entry_buff, curr_token);
                        entry_buff->atr.flag = entry_buff->atr.flag | ArgFlagDiagnostic::IS_CALLED;
                    default:
                        break;
                    }


                }

            } else if (subcom_lookup.count(curr_token) != 0){
                _CheckAllRequiredArgIsCalled();
                ++token_ptr;
                
                subcom_lookup.at(curr_token).instance._ParseFrom(tokens, token_ptr, positional_ptr, curr_token, entry_buff);
                _ArgCallback();
                return;

            } else {
                if(positional_ptr < ordered_positionals.size()){
                    entry_buff = ordered_positionals[positional_ptr];
                    int old_narg = entry_buff->atr.narg;
                    switch (entry_buff->atr.flag & ArgFlagDiagnostic::TAKETYPE_MASK)
                    {
                    case STORE_NARG :
                        _ParseMultiple(tokens, token_ptr, *entry_buff, curr_token);
                        if(entry_buff->atr.narg == 0){
                            ++positional_ptr;
                            entry_buff->atr.flag = entry_buff->atr.flag | ArgFlagDiagnostic::IS_CALLED;
                            entry_buff->atr.narg = old_narg;
                        }
                        // Don't throw exception if interrupted (ignore interruptions)
                    
                    case STORE_ANY :
                        _ParseMultiple(tokens, token_ptr, *entry_buff, curr_token);
                        entry_buff->atr.flag = entry_buff->atr.flag | ArgFlagDiagnostic::IS_CALLED;
                        // We don't know what condition "any narg" positional argument supposed to finish
                        // So we are NOT incrementing positional_ptr
                    default:
                        break;
                    }
                } else {
                    _ErrorHandler(ArgErrorCode::ERROR_UNKNOWN_ARG, nullptr, curr_token);
                }
            }

            // ++token_ptr are not used here, because it was already handled by the block code above
            
            
        }
    }

    void ArgParser::_AddPositional(std::string& opt, ArgEntry& entry){
        opt = utls::frmt::__strip(opt, ' ');
        if(is_posarg_subcom(opt)){
            switch (entry.atr.flag & ArgFlagDiagnostic::TAKETYPE_MASK)
            {
            case ArgFlag::STORE_ANY :
                entry.atr.narg = -1;
            case ArgFlag::STORE_NARG :
                if(entry.atr.narg != 0){
                    break;
                }
                
            default:
                throw std::invalid_argument("Invalid take type flag for " + opt);
                break;
            }

            ArgEntry* entry_ptr = new ArgEntry(entry);
            ordered_positionals.push_back(entry_ptr);
            arg_lookup.insert({opt, entry_ptr});
        } else {
            throw std::invalid_argument(opt + " does not match a positional argument pattern");
        }
    }

    void ArgParser::_FlagVerifyNarg(ArgEntry& entry){
        switch (entry.atr.flag & ArgFlagDiagnostic::TAKETYPE_MASK)
            {
            case ArgFlag::STORE_CONST :
                // We don't need to care about the nargs. but we do need to care abt constant type
                try{
                    std::any_cast<NoneType>(entry.constant_val);
                    throw std::invalid_argument("Constant type shouldn't be none");
                } catch (std::bad_any_cast& msg){}
                break;
            
            case ArgFlag::STORE_NARG :
                if(entry.atr.narg == 0){
                    throw std::invalid_argument("STORE_NARG flag shouldn't be paired with 0 nargs");
                }
                break;
            case ArgFlag::STORE_ANY :
                entry.atr.narg = -1;
                break;
            default:
                throw std::invalid_argument("Invalid take type flag");
                break;
            }

    }

    void ArgParser::_AddFlag(std::string& opt, ArgEntry& entry){
        size_t comma_pos = opt.find(',');

        std::regex short_pattern("^-[a-zA-Z]$");
        std::regex long_pattern("^--[a-zA-Z]+[a-zA-Z0-9_-]*$");
        if(comma_pos != std::string::npos){
            std::string short_flag = utls::frmt::__strip(opt.substr(0, comma_pos), ' ');
            std::string long_flag = utls::frmt::__strip(opt.substr(comma_pos + 1), ' ');

            if(!short_flag.empty() && !long_flag.empty()){
                if(std::regex_match(short_flag, short_pattern) && std::regex_match(long_flag, long_pattern)){
                    _FlagVerifyNarg(entry);
                    ArgEntry* entry_ptr = new ArgEntry(entry);
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
            opt = utls::frmt::__strip(opt, ' ');
            switch (opt.find_first_not_of('-'))
            {
            case 1: // short flag
                if(!std::regex_match(opt, short_pattern)){
                    throw std::invalid_argument("Invalid short flag pattern");
                } 
                break;
            
            case 2 : // long flag
                if(!std::regex_match(opt, long_pattern)){
                    throw std::invalid_argument("Invalid long flag pattern");
                }
                break;
            
            default:
                throw std::invalid_argument("Unknown flag format");
                break;
            }
            _FlagVerifyNarg(entry);
            ArgEntry* entry_ptr = new ArgEntry(entry);
            unduplicated_arg_data.push_back(entry_ptr);
            arg_lookup.insert({opt, entry_ptr});
        }

    }

    /**
     * @brief Add positional or flag into the parser
     * 
     * @note 
     * Enter the appropriate format for argument type as following :
     * 
     * - positionals = "name"
     * 
     * - single flag = "-v" or "--verbose"
     * 
     * - double flag = "-v, --verbose", "-p, --port" (using comma means you're commited to a double flag input)
     */
    void ArgParser::add_argument(
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
            if((entry.atr.flag & ArgFlagDiagnostic::ARGTYPE_MASK) == ArgFlag::FLAG){
                _AddFlag(opt, entry);
            } else {
                _AddPositional(opt, entry);
            }
        } else {
            throw std::invalid_argument("Invalid narg, the condition must be narg >= 0");
        }
    }

    const std::unordered_map<std::string, ArgEntry*>* ArgParser::get_arg_lookup() const noexcept {
        return &arg_lookup;
    }

    ArgParser* ArgParser::add_subcommand(std::string cmnd, std::string help, std::function<void()> fn){
        if(is_posarg_subcom(cmnd)){
            auto res = subcom_lookup.emplace(cmnd, SubComEntry(cmnd, fn, this, help));
            if(res.second){
                return &res.first->second.instance; // how do I return SubComEntry.ArgParser_instance ?
            } else {
                throw std::runtime_error("Failed inserting sub-command entry");
            }
        } else {
            throw std::invalid_argument("Invalid command regex pattern");
        }
    }

    void ArgParser::parse_main(std::vector<std::string> tokens){ 
        unsigned int token_ptr = 0;
        unsigned int positional_ptr = 0;
        ArgEntry* entry_buff;
        std::string curr_token;
        _ParseFrom(tokens, token_ptr, positional_ptr, curr_token, entry_buff);
        
    }



int main(){
    ArgParser parser;
    bool a;
    parser.add_argument("-v, --verbose", ArgFlag::FLAG | ArgFlag::STORE_CONST | ArgFlag::IS_REQUIRED, 0, "", "", [](){}, true);
    parser.parse_main({"-abc"});
    auto p = parser.get_arg_lookup();
    std::cout << "Parser test : (Store const => true)\n";
    std::cout << "long flag existence : " << std::boolalpha << p->count("--verbose") << "\n";
    std::cout << "short flag existence : " << std::boolalpha << p->count("-v") << "\n";
    a = std::any_cast<bool>(p->at("-v")->constant_val);
    std::cout << "Value is : " << a << std::endl;
    return 0;
}



