#pragma once
#include "Token.h"
#include <string>

namespace dyj {
    class Error {
    public:
        enum Type : char {
            LEXER_ERROR = 'a',
            REDEFINITION = 'b',
            UNDEFINED = 'c',
            UNMATCHED_PARAM_COUNT = 'd',
            UNMATCHED_PARAM_TYPE = 'e',
            UNKNOWN_TYPE_CONDITION = 'f',
            VOID_RETURN_DISMATCH = 'g',
            UNVOID_RETURN_DISMATCH = 'h',
            INDEX_ERROR = 'i',
            CONST_DISCARD = 'j',
            SEMI_LOST = 'k',
            RPARENT_LOST = 'l',
            RBRACK_LOST = 'm',
            WHILE_LOST = 'n',
            CONST_INITIALIZATION_ERROR = 'o',
            UNKNOWN_ERROR = '*',
            NO_ERROR = '-'
        };
        
        Error() = delete;
        Error(Type _type, Location _location);

        Type get_type();
        std::string to_string();

    private:
        Type type;
        Location location;
    };
}
