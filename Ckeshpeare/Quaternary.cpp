#include "Quaternary.h"

namespace dyj {
    Quaternary::Quaternary(Type _type, const string &a, const string &b, const string &c) : type(_type), dest(a), lhs(b), rhs(c) {}

    Quaternary::Type Quaternary::get_type(void) const {
        return type;
    }

    string Quaternary::get_dest(void) const {
        return dest;
    }

    string Quaternary::get_lhs(void) const {
        return lhs;
    }

    string Quaternary::get_rhs(void) const {
        return rhs;
    }

    string Quaternary::to_string(void) const {
        return repr[type] + " " + dest + " " + lhs + " " + rhs;
    }

    std::vector<std::string> Quaternary::repr {
        "PLUS",       // +
        "MINUS",      // -
        "TIME",       // *
        "DIVIDE",     // /
        "COPY",       // =
        "NEGATE",     // -
        "LESS",       // <
        "LEQ",        // <=
        "EQU",        // ==
        "NEQ",        // !=
        "INDEX",      // =[]
        "ELEMENT",    // []=
        "JUMP",       // unconditioned jump
        "JUMP_IF",    // jump on true
        "JUMP_UNLESS",// jump on false
        "LABEL",      // sheer label
        "ARGUMENT",   // set a variable to argument
        "CALL",       // call a function
        "PARAM",      // retrive parameters
        "SET_RETURN", // set return value before return
        "GET_RETURN", // get return value of the latest function called
        "RETURN",     // jump to caller
        "READI",      // read int
        "READC",      // read char
        "PRINTI",     // print int
        "PRINTC",     // print char
        "PRINTS",     // print string
        "PRINTF",     // print format
        "ENTRY",      // entry point of program
        "EXIT",       // end point of program
        "BEGIN",
        "END",
        "VAR",
    };

}

