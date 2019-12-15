#include "Quaternary.h"
#include "debug.h"
#include <cassert>

namespace dyj {
    Quaternary::Quaternary(Type _type, const string &a, const string &b, const string &c, int _block_id) : type(_type), dest(a), lhs(b), rhs(c), block_id(_block_id) {}

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

    int Quaternary::get_block_id(void) const {
        return block_id;
    }

    std::set<std::string> Quaternary::get_defs(void) const {
        switch (type) {
        case PLUS:
        case MINUS:
        case TIME:
        case DIVIDE:
        case COPY:
        case NEGATE:
        case LESS:
        case LEQ:
        case EQU:
        case NEQ:
        case INDEX:
        case PARAM:
        case GET_RETURN:
        case READI:
        case READC:
            return { dest };
        case VAR:
        case ELEMENT:
        case JUMP:
        case JUMP_IF:
        case JUMP_UNLESS:
        case LABEL:
        case ARGUMENT:
        case CALL:
        case SET_RETURN:
        case PRINTI:
        case PRINTC:
        case PRINTF:
        case PRINTS:
        case ENTRY:
        case EXIT:
        case BEGIN:
        case END:
        case RETURN:
            return {};
        default:
            DP("%s\n", to_string().c_str());
            assert(0);
        }
    }

    std::set<std::string> Quaternary::get_uses(void) const {
        switch (type) {
        case PLUS:
        case MINUS:
        case TIME:
        case DIVIDE:
        case LESS:
        case LEQ:
        case EQU:
        case NEQ:
            return { lhs, rhs };
        case COPY:
        case NEGATE:
        case PARAM:
        case JUMP_IF:
        case JUMP_UNLESS:
        case SET_RETURN:
        case PRINTI:
        case PRINTC:
            return { lhs };
        case INDEX:
        case PRINTF:
            return { rhs };
        case GET_RETURN:
        case READI:
        case READC:
        case VAR:
        case JUMP:
        case LABEL:
        case ARGUMENT:
        case CALL:
        case ENTRY:
        case EXIT:
        case BEGIN:
        case END:
        case RETURN:
        case PRINTS:
            return {};
        case ELEMENT:
            return { dest, lhs, rhs };
        default:
            assert(0);
        }
    }

    bool Quaternary::is_jump(void) const {
        switch (type) {
        case JUMP:
        case JUMP_IF:
        case JUMP_UNLESS:
        case RETURN:
        /*case CALL:
        case PRINTC:
        case PRINTF:
        case PRINTI:
        case PRINTS:
        case READI:
        case READC:*/
            return true;
        default:
            return false;
        }
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

