#pragma once
#include <string>
#include "Location.h"

namespace dyj {
    class Token {
    public:
        enum Type : int {
            DEFAULT,
            ERROR,
            IDENFR,
            INTCON,
            CHARCON,
            STRCON,
            CONSTTK,
            INTTK,
            CHARTK,
            VOIDTK,
            MAINTK,
            IFTK,
            ELSETK,
            DOTK,
            WHILETK,
            FORTK,
            SCANFTK,
            PRINTFTK,
            RETURNTK,
            PLUS,
            MINU,
            MULT,
            DIV,
            LSS,
            LEQ,
            GRE,
            GEQ,
            EQL,
            NEQ,
            ASSIGN,
            SEMICN,
            COMMA,
            LPARENT,
            RPARENT,
            LBRACK,
            RBRACK,
            LBRACE,
            RBRACE
        };
        static std::string type_to_string(Type);
        static bool is_relation(Token *_t);

        Token();
        Token(Type _type, const std::string &_content, Location _location, bool _is_error = false);

        Type get_type() const;
        std::string get_content() const;
        std::string to_string() const;
        bool operator == (const Token &_rhs) const;
        Location get_location() const;
    private:
        Type type;
        std::string content;
        Location location;
        bool is_error;
    };
}