/**
 * @file argparser.hpp
 * @brief Declare struct, class (and it's function). for parser 
 * 
 * @author Adhyastha Abiyyu Azlee
 * @date 2025-07-26
 * @copyright 2025 Adis7i
 */

#ifndef ADIS_UTILS_PARSER_HPP
#define ADIS_UTILS_PARSER_HPP

#include <unordered_map>
#include <string>
#include <any>
#include <functional>
#include <stdint.h>
/**
 * 0 0 0 0 0 (read from the right)
 * 1100011
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

namespace utls{
    namespace prsr {
    /**
     * @brief Enum for flag attributes
     */
    enum ArgFlag {
        STORE_CONST = 0x0,
        STORE_NARG = 0x4,
        STORE_ANY = 0x8,
        IS_REQUIRED = 0x2,
        FLAG = 0x10,
        POSITIONAL = 0x0,
    };

    /**
     * @brief Enum to filter out desired flag
     */
    enum Diagnostic {
        TAKETYPE_MASK = 0xC,
        ARGTYPE_MASK = 0x10,
        IS_CALLED = 0x1,
        REQUIRED_TO_CALLED_MASK = 0x3
    };

    /**
     * @brief Error code
     */
    enum Err_code {
        ERROR_MISSING_ARG,
        ERROR_UNKNOWN_ARG,
        ERROR_MISSING_NARG,
        ERROR_INVALID_INPUT_TO_ATTRIBUTE
    };

    struct Attr {
        uint8_t flag = 0;
        uint8_t narg = 0; // 255+ more arguments ?, you must be joking !
    };

    struct Meta {
        std::string name; // This is for print usage function, so it doesn't need to do lot of if's
        std::string metavar;
        std::string help;
    };

    struct ArgEntry {
        Meta meta;
        Attr atr;
        std::vector<std::string> values;
        std::any constant_val;
        std::any default_val;
        std::function<void()> func;
    };

    struct None {}; // dummy type for constant_val type checking

    class ArgParser {
    private:
        std::unordered_map<std::string, ArgEntry*> arg_lookup;
        std::unordered_map<std::string, struct SubComEntry> subcom_lookup;
        std::vector<ArgEntry*> unduplicated_arg_data;
        std::vector<ArgEntry*> ordered_positionals;
        std::string command;
        ArgParser* parent_parser;
        std::function<void()> func;

        /**
         * @brief Print series of commands such as "git commit", "git push", etc.
         */
        void _PrintSOC();

        /**
         * @brief Print usage
         * 
         * Print such as "usage: ls [-h | --help][-l][--full-time]"
         */
        void _PrintUsage();

        /**
         * @brief Throws error upon parsing
         */
        void _ErrorHandler(Err_code code, ArgEntry* issued_argument = nullptr, const std::string& issued_token = "");

        /**
         * @brief Call function on flags, positionals, and this->func
         */
        void _ArgCallback();

        // You can see the name
        void _CheckAllRequiredArgIsCalled();

        /**
         * @brief Insert values for (narg > 0) flag, or posarg
         */
        void _ParseMultiple(std::vector<std::string>& tokens, unsigned int& token_ptr, ArgEntry& entry, std::string& curr_token);

        /**
         * @brief Main logic of the parsing
         * 
         * Using reference to avoid stack overflow from a function state. if subcommand is too much
         */
        void _ParseFrom(std::vector<std::string>& tokens, unsigned int& token_ptr, unsigned int& positional_ptr, std::string& curr_token, ArgEntry* entry_buff);

        // rest of the functions below are pretty self-explanatory
        void _AddPositional(std::string& opt, ArgEntry& entry);
        void _FlagVerifyNarg(ArgEntry& entry);
        void _AddFlag(std::string& opt, ArgEntry& entry);

    public:
        /**
         * @brief Get flag or posarg entry pointer
         */
        ArgEntry* _GetEntry(const std::string& token);

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
        void add_argument(
            std::string opt,
            uint8_t flag,
            uint8_t narg = 0,
            std::string metavar = "",
            std::string desc = "",
            std::function<void()> fn = [](){},
            std::any constant = None {},
            std::any def = None {}
        );

        const std::unordered_map<std::string, ArgEntry*>* get_arg_lookup() const noexcept;
        const std::unordered_map<std::string, SubComEntry>* get_subcom_lookup() const noexcept;


        ArgParser* add_subcommand(std::string cmnd, std::string help, std::function<void()> fn = [](){});

        void parse_main(std::vector<std::string> tokens);

        ArgParser(std::string comnd = "", std::function<void()> fn = [](){}, ArgParser* prsr_parent = nullptr)
            : command(comnd), func(fn), parent_parser(prsr_parent) {}

        ~ArgParser() noexcept {
            for (auto entry : unduplicated_arg_data)
                delete entry;
        }
    };


    // Subcommand entry
    struct SubComEntry {
        ArgParser instance;
        std::string desc;

        SubComEntry(std::string& cmnd, std::function<void()>& fn, ArgParser* parent, std::string& help)
            : instance(cmnd, fn, parent), desc(help) {}
    };
    }

}
#endif