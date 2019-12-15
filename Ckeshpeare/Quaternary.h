#pragma once
#include <string>
#include <vector>
#include <set>

namespace dyj {
    using std::string;
    class Quaternary {
    public:
        enum Type {
            PLUS,       // +
            MINUS,      // -
            TIME,       // *
            DIVIDE,     // /
            COPY,       // =
            NEGATE,     // -
            LESS,       // <
            LEQ,        // <=
            EQU,        // ==
            NEQ,        // !=
            INDEX,      // =[]
            ELEMENT,    // []=
            JUMP,       // unconditioned jump
            JUMP_IF,    // jump on true
            JUMP_UNLESS,// jump on false
            LABEL,      // just label
            ARGUMENT,   // set a variable to argument
            CALL,       // call a function
            PARAM,      // retrive parameters
            SET_RETURN, // set return value before return
            GET_RETURN, // get return value of the latest function called
            RETURN,     // jump to caller
            READI,      // read int
            READC,      // read char
            PRINTI,     // print int
            PRINTC,     // print char
            PRINTS,     // print string
            PRINTF,     // print format
            ENTRY,      // entry point of program
            EXIT,       // end point of program
            BEGIN,
            END,
            VAR,
        };

        Quaternary() = delete;
        Quaternary(Type _type, const string &a = "", const string &b = "", const string &c = "", int _block_id = -1);

        Type get_type(void) const;
        string get_dest(void) const;
        string get_lhs(void) const;
        string get_rhs(void) const;
        string to_string(void) const;
        int get_block_id(void) const;

    public:
        std::set<std::string> get_defs(void) const;
        std::set<std::string> get_uses(void) const;

    public:
        bool is_jump(void) const;

    private:
        static std::vector<string> repr;

    private:
        Type type;
        string dest, lhs, rhs;
        int block_id;

    };

    typedef std::vector<Quaternary> IrList;

}
