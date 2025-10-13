
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
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include <array>
#include <stdexcept>
#include <memory>

namespace utls {
namespace prsr {

// enum class OptionsTypeTag { Boolean, Integer, Float, String };
enum class ParsingStatus { 
    UnexpectedToken, 
    UnknownOption,
    Fine, 
    UnsatisfiedNarg, 
    LeftToCall,
    UnexpectedFailure
};
enum class ParserErrorCode { Good, InvalArg, NameExist, StorageFull, AnyBeforeNumbered };
enum NArgType { Any = -1, CallOnly = 0 };
enum ParseFlowStat {
    TrueHalt,
    Halt,
    Continue,
    SubcomHalt // Special only for parse_option
};
enum class ArgType { Positional, Flag };

class Parser;

struct Option {
    private :
    bool isCalled = false;
    friend class Parser;
    public :
    std::function<void(Option&)> callback = [](Option&){};
    std::string desc;
    std::vector<std::string> values;
    bool is_required = false;

    const char short_name;
    const std::string long_name;
    const int narg;
    const ArgType opt_type;

    bool is_called() const noexcept { return isCalled; }

    Option(
        int _narg,
        char _short_name,
        const std::string& _long_name,
        ArgType opt_type_)
            : short_name(_short_name), long_name(_long_name),
              narg(_narg), opt_type(opt_type_) {}
    
    Option() 
        : short_name('\0'),
          long_name(""),
          narg(0),
          opt_type(ArgType::Flag)
    {}
};

struct Subcom;

class Parser {  
    private :
    std::unordered_map<std::string, Option*> long_lookup_;
    std::unordered_map<char, Option*> short_lookup_;
    std::unordered_map<std::string, Option*> posarg_lookup_;
    std::unordered_map<std::string, std::unique_ptr<Subcom>> subcom_lookup_;
    std::vector<Option*> posarg_;
    std::vector<Option> options_;
    std::vector<std::string> dump;

    ParsingStatus parsing_status_;
    ParserErrorCode other_status_;
    
    bool last_is_any_;

    // Return how many argument were parsed
    int multi_parse(
        bool (*token_check)(std::string_view& v),
        std::vector<std::string>::iterator& iter,
        const std::vector<std::string>::iterator& end,
        Option*& opt);

    ParseFlowStat handle_flag(
        std::vector<std::string>::iterator& iter,
        const std::vector<std::string>::iterator& end,
        std::string_view& viewer, Option*& opt
    );
    // Return a status wether the whole parsing should halt or not
    // true to continue, false to halt
    ParseFlowStat parse_option(
        std::vector<std::string>::iterator& iter,
        const std::vector<std::string>::iterator& end);

    ParseFlowStat parse_posarg();

    bool finalize();

    void shortcut_print_narg(Option& opt);

    public :
    std::string prolouge;
    std::string epilouge;

    void help();

    // Cool name ain't it ?, Using bool as return type
    // This is to ease up a way for user to know if anythings wrong
    bool orchestra(
        std::vector<std::string>::iterator& beg,
        const std::vector<std::string>::iterator& end);

    // Parser stores a fixed number of Option objects.
    // After allocation in the constructor, no further growth is allowed.
    Parser(size_t _alloc_arg){
        if(_alloc_arg <= 0) throw std::invalid_argument("_alloc_arg is not valid...");
        options_.reserve(_alloc_arg + 1); // plus help
        last_is_any_ = false;
        // Placeholder
        auto opt = add_option(NArgType::CallOnly, 'h', "help");
        opt->desc = "Print help and show this message";
    }

    Option* get_short_flag(char _name);

    Option* get_long_flag(const char* _name);

    Option* get_posarg(const char* _name);
    
    /**
     * @brief Adds an short flag. pointer management responsibility and consequences are held by the user
     * 
     * @return
     * Upon successful operation, this function shall return a pointer pointing to a data.
     * Upon a failure, this function shall return a ```nullptr```
     * 
     * @note
     * Enter format such as 
     * short_name => 'a', 'p', 'h', etc.
     */
    Option* add_option(int _narg, char _short_name);

    /**
     * @brief Adds an short flag. pointer management responsibility and consequences are held by the user
     * 
     * @return
     * Upon successful operation, this function shall return a pointer pointing to a data.
     * Upon a failure, this function shall return a ```nullptr```
     * 
     * @note
     * Enter format such as 
     * long_name => 'port', 'output', 'file', etc.
     */
    Option* add_option(int _narg, const char* _long_name);

    /**
     * @brief Adds an short flag. pointer management responsibility and consequences are held by the user
     * 
     * @return
     * Upon successful operation, this function shall return a pointer pointing to a data.
     * Upon a failure, this function shall return a ```nullptr```
     * 
     * @note
     * Enter format such as 
     * long_name => 'port', 'output', 'file', etc.
     * short_name => 'a', 'b', 'c'
     */
    Option* add_option(int _narg, char _short_name, const char* _long_name);

    // basically have the same procedure as long flag overload variant of add_option
    // The difference is posarg are not allowed to be only call
    Option* add_posarg(int _narg, const char* _name);

    Subcom* add_subcom(const char* _name, size_t _alloc_arg);
    
    bool storage_full() const noexcept;
    bool last_is_any() const noexcept;
    ParsingStatus parsing_status() const noexcept;
    ParserErrorCode parser_status() const noexcept;
    // Empties value, switch isCalled to false, and switch class status to Fine/Good
    void reset() noexcept;
};

struct Subcom {
    private :
    bool isCalled = false;
    friend class Parser;

    public :
    std::string name;
    std::function<void(Subcom&)> callback = [](Subcom&){};
    Parser parser_obj;

    bool is_called() const noexcept { return isCalled; }
    Subcom(size_t alloc_opt) : parser_obj(alloc_opt) {}
};

void print_parsing_message(ParsingStatus status);
void print_parser_message(ParserErrorCode status);

}
}
#endif
