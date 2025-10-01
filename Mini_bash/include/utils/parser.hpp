
/**
 * @file parser.hpp
 * @brief Definition and declaration of class parser
 * 
 * @author Adhyastha A.A
 * @date 2025-10-01
 * @copyright Adis7i
 */

#ifndef ADIS_PARSER_HPP
#define ADIS_PARSER_HPP
#include "util.cpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <type_traits>
#include <iostream>
#include <iomanip>

// Base struct for flag and posargs
namespace util {
namespace parser {

enum ArgType{FLAG, POSITIONAL};

/**
 * @brief Base struct to store option rule, and behavior
 */
struct arg_s {
    std::string name1, name2, desc;
    bool required, called;
    int narg_p; // .narg_p
    const int narg;
    std::vector<std::string> val;
    ArgType type;
    arg_s(ArgType Type, const bool& required_,
        const int& narg_,
        const std::string& _desc = "",
        const std::string& _name = "",
        const std::string& _name2 = "")
        : required(required_), narg_p(narg_), 
        narg(narg_), desc(_desc), name1(_name), name2(_name2),
        type(Type) {
        called = false;
        val.reserve(narg_);
    }
};

/**
 * @brief Describe everything that happened except the parsing process
 */
enum ParserStat { 
    OVERFLOW_BAN, CONSTRUCT_GOOD,
    GOOD, INVAL_ARG, NAME_EXIST,
    FETCH_DATA_BAN
};

/**
 * @brief Describe what happened in the parsing process
 */
enum ParsingStat { 
    UNKOWN_TOKEN, UNCALLED_REQUIRED,
    INSUFFICIENT_NARG_PASSED, FINE,
    NO_OP
};

struct LongFlag {}; // Add a flag with a long flag name format
struct TwinFlag {}; // To add a short and long flag pointing to the same data
struct ShortFlag {}; // Add a flag with a short flag name format
void dummy(){}

/**
 * @brief An argumen parsing class
 */
template <size_t _posarg_count>
class ArgParser {
    private :
    std::unordered_map<std::string, arg_s*> _lookup;
    std::vector<arg_s> _reservoir;
    arg_s* posarg[_posarg_count];
    int _posarg_added; // this points to total element or 1 past the last index
    int _posarg_ptr;    
    bool _user_fetched_data;
    ParserStat _vital_process_code;
    ParsingStat _parse_process_code;
    std::string desc;
    
    /**
     * @brief Also a parsing function but for inserting data
     * @note This function always stop 
     */
    bool multi_parse(
        const std::vector<std::string>& tokens,
        size_t& i,
        std::string& iter_token,
        arg_s& profile
    ) {
        for(; (i < tokens.size()) && (profile.narg_p > 0); i++){
            iter_token = tokens[i];
            
            if(iter_token[0] == '-') return true;
            profile.val.push_back(iter_token);
            
            --profile.narg_p;
            
        }
        
        return false;
    }

    void finalize(){
        for(auto a : _reservoir){
            if(a.required && !a.called){
                _parse_process_code = ParsingStat::UNCALLED_REQUIRED;
                _cleanup_on_parse_failure();
                break;
            }
        }
    }

    /**
     * A function that can recursively called upon enterin subcommands
     * without risking a stack overflow by using reference parameters
     */
    void _parse_from_ref(
        const std::vector<std::string>& tokens,
        size_t& i, // this is needed in case of subcoms
        std::string& iter_token
    ) {
        
        while(i < tokens.size()){
            iter_token = tokens[i];
            while (true) { // I put this on here because multi_parse signals
                if(iter_token[0] == '-'){
                    
                    
                    auto it = _lookup.find(iter_token);
                    
                    if(it == _lookup.end()){
                        _parse_process_code = ParsingStat::UNKOWN_TOKEN;
                        _cleanup_on_parse_failure();
                        
                        return;
                    }
                    
                    arg_s& profile_ref = *(it->second);
                    ++i;
                    bool possible_known_arg = multi_parse(tokens, i, iter_token, profile_ref);
                    

                    if(profile_ref.val.size() != profile_ref.narg) {
                        
                        _parse_process_code = ParsingStat::INSUFFICIENT_NARG_PASSED;
                        _cleanup_on_parse_failure();
                        return;
                    }

                    profile_ref.called = true;
                    
                    if(possible_known_arg) continue;


                } else if (_posarg_ptr < _posarg_added) {
                    
                    arg_s& profile_ref = *posarg[_posarg_ptr];
                    bool possible_known_arg = multi_parse(tokens, i, iter_token, profile_ref);

                    if(profile_ref.val.size() == profile_ref.narg) {
                        ++_posarg_ptr;
                        profile_ref.called = true;
                        
                    } else 

                    if(possible_known_arg) continue;
                } else {
                    _parse_process_code = ParsingStat::UNKOWN_TOKEN;
                    _cleanup_on_parse_failure();
                    return;
                }
                break;
            }
        }
        _parse_process_code = ParsingStat::FINE;
    }

    void _cleanup_on_parse_failure() noexcept {
        
        for(auto& it : _reservoir){
            // this is so that this parser could be used again after failure
            it.val.clear();
            it.called = false;
            it.narg_p = it.narg;
        }
    }

    bool _basic_safeguard(){ // returns true if basic safety check are passed
        if(reservoir_is_full()) {
            _vital_process_code = ParserStat::OVERFLOW_BAN;
            return false;
        }
        if(_user_fetched_data){
            _vital_process_code = ParserStat::FETCH_DATA_BAN;
            return false;
        }
        return true;
    }

    public :
    
    void help(){
        
        
        for(int i = 0; i < _posarg_added; i++){
            std::cout << "- " << std::left << std::setfill(' ') << std::setw(10)
             << posarg[i]->name1 << posarg[i]->desc << "\n";
        }

        std::cout << "\nFlags :\n";
        for(auto i : _reservoir){
            if(i.type == ArgType::FLAG){
                std::cout << (i.required ? "[!] " : "[*] ") << i.name1;
                if(!i.name2.empty()){
                    std::cout << ", " << i.name2;
                }
                std::cout << "  " << i.desc << "\n";
                
            }
        }
    }
    
    ArgParser(size_t _alloc_num, const std::string& _desc = "") : desc(_desc){
        if(_alloc_num < _posarg_count) {
            _vital_process_code = ParserStat::OVERFLOW_BAN;
            return;
        }
        _reservoir.reserve(_alloc_num);
        _vital_process_code = ParserStat::CONSTRUCT_GOOD;
        _parse_process_code = ParsingStat::NO_OP;
        _user_fetched_data = false;
        _posarg_added = 0;
        _posarg_ptr = 0;
    }

    void parse(const std::vector<std::string>& tokens){
        std::string iter_token = "";
        size_t i = 0;
        _parse_from_ref(tokens, i, iter_token);
        finalize();
    }

    const arg_s* get(const std::string& name){
        auto entry = _lookup.find(name);
        if(entry != _lookup.end()){
            _user_fetched_data = true;
            return entry->second;
        }
        return nullptr;
    }

    bool reservoir_is_full(){
        return _reservoir.capacity() == _reservoir.size();
    }

    bool reserve_map(size_t n){
        bool resized_the_map = false;
        if(n > _lookup.size()){
            resized_the_map = true;
            _lookup.reserve(n);
        }
        return resized_the_map;
    }

    template <typename T>
    typename std::enable_if<std::is_same<ShortFlag, T>::value, void>::type
    add_flag(bool is_required, int narg, const std::string& name, const std::string& desc = ""){
        if(narg < 0 || name.empty() || !util::is_valid_short_flag(name)) {
            _vital_process_code = ParserStat::INVAL_ARG;
            return;
        }
        if(_basic_safeguard()){
            auto status_pair = _lookup.emplace(name, nullptr);
            if(status_pair.second) {
                _reservoir.push_back(arg_s(ArgType::FLAG, is_required, narg, desc, name));
                status_pair.first->second = &_reservoir.back();                
                _vital_process_code = ParserStat::GOOD;
            } else _vital_process_code = ParserStat::NAME_EXIST;
        }
    }

    template <typename T>
    typename std::enable_if<std::is_same<LongFlag, T>::value, void>::type
    add_flag(bool is_required, int narg, const std::string& name, const std::string& desc = ""){
        if(narg < 0 || name.empty() || !util::is_valid_long_flag(name)) {
            _vital_process_code = ParserStat::INVAL_ARG;
            return;
        }
        if(_basic_safeguard()){
            auto status_pair = _lookup.emplace(name, nullptr);
            if(status_pair.second) {
                _reservoir.push_back(arg_s(ArgType::FLAG, is_required, narg, desc, name));
                status_pair.first->second = &_reservoir.back();                
                _vital_process_code = ParserStat::GOOD;
            } else _vital_process_code = ParserStat::NAME_EXIST;
        }
    }

    template <typename T>
    typename std::enable_if<std::is_same<TwinFlag, T>::value, void>::type
    add_flag(bool is_required, int narg, const std::string& short_name, const std::string& long_name, const std::string& desc = ""){
        if(narg < 0 || (short_name.empty() && long_name.empty())
        || (!util::is_valid_long_flag(long_name) || !util::is_valid_short_flag(short_name)))
        {
            _vital_process_code = ParserStat::INVAL_ARG;
            return;
        }

        if(_basic_safeguard()){
            if (_lookup.count(short_name) || _lookup.count(long_name)) {
                _vital_process_code = ParserStat::NAME_EXIST;
                return;
            }
            _reservoir.push_back(arg_s(ArgType::FLAG, is_required, narg, desc, short_name, long_name));
            _lookup.emplace(short_name, &_reservoir.back());
            _lookup.emplace(long_name, &_reservoir.back());
        }

    }

    void add_posarg(const std::string& name, int narg, const std::string& desc = ""){
        if(narg <= 0 || name.empty() || !util::is_valid_posarg(name)) {
            _vital_process_code = ParserStat::INVAL_ARG;
            return;
        }
        
        if(reservoir_is_full() || (_posarg_added == _posarg_count)) {
            _vital_process_code = ParserStat::OVERFLOW_BAN;
            return;
        }
        if(_user_fetched_data){
            _vital_process_code = ParserStat::FETCH_DATA_BAN;
            return;
        }

        auto status_pair = _lookup.emplace(name, nullptr);
        if(status_pair.second) {
            _reservoir.push_back(arg_s(ArgType::POSITIONAL, true, narg, desc, name));
            status_pair.first->second = &_reservoir.back();
            posarg[_posarg_added] = &_reservoir.back(); // _posarg_added before increment literally points to the index that hasn't been filled
            ++_posarg_added;
            _vital_process_code = ParserStat::GOOD;
        } else _vital_process_code = ParserStat::NAME_EXIST;
        return;
    }

    ParserStat status() const noexcept {
        return _vital_process_code;
    }

    ParsingStat parse_stat() const noexcept {
        return _parse_process_code;
    }
};

    void print_status(ParserStat status);
    void print_parse_status(ParsingStat status);
    void profMap(const util::parser::arg_s* profile);
}

}
#endif
