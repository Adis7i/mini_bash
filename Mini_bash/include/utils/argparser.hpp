#include <unordered_map>
#include <string>
#include <any>
#include <functional>
#include <stdint.h>
enum ArgFlag {
    STORE_CONST = 0,
    STORE_NARG = 1 << 2,
    STORE_ANY = 1 << 3,
    IS_REQUIRED = 1 << 1,
    FLAG = 1 << 4,
    POSITIONAL = 0,
};

enum ArgFlagDiagnostic {
    TAKETYPE_MASK = 3 << 2,
    ARGTYPE_MASK = 1 << 4,
    IS_CALLED = 1,
    REQUIRED_TO_CALLED_MASK = 3
};

enum ArgErrorCode {
    ERROR_MISSING_ARG,
    ERROR_UNKNOWN_ARG,
    ERROR_MISSING_NARG,
    ERROR_INVALID_INPUT_TO_ATTRIBUTE
};

struct AgrAtr {
    uint8_t flag = 0;
    uint8_t narg = 0; // 255+ more arguments ?, you must be joking !
};

struct ArgMeta {
    std::string opt1;
    std::string opt2;
    std::string metavar;
    std::string help;
};

struct ArgEntry {
    ArgMeta meta;
    AgrAtr atr;
    std::vector<std::string> values;
    std::any constant_val;
    std::any default_val;
    std::function<void()> func;
};

struct NoneType {};

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
    void _ErrorHandler(ArgErrorCode code, ArgEntry* issued_argument = nullptr, const std::string& issued_token = "");

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
     * @brief Get flag or posarg entry pointer
     */
    ArgEntry* _GetEntry(const std::string& token);

    /**
     * @brief Main logic of the parsing
     * 
     * Using reference to avoid stack overflow from a function state. if subcommand is too much
     */
    void _ParseFrom(std::vector<std::string>& tokens, unsigned int& token_ptr, unsigned int& positional_ptr, std::string& curr_token, ArgEntry* entry_buff);

    // Three functions below are pretty self-explanatory
    void _AddPositional(std::string& opt, ArgEntry& entry);
    void _FlagVerifyNarg(ArgEntry& entry);
    void _AddFlag(std::string& opt, ArgEntry& entry);

public:
    void add_argument(
        std::string opt,
        uint8_t flag,
        uint8_t narg = 0,
        std::string metavar = "",
        std::string desc = "",
        std::function<void()> fn = [](){},
        std::any constant = NoneType {},
        std::any def = NoneType {}
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



struct SubComEntry {
    ArgParser instance;
    std::string desc;

    SubComEntry(std::string& cmnd, std::function<void()>& fn, ArgParser* parent, std::string& help)
        : instance(cmnd, fn, parent), desc(help) {}
};