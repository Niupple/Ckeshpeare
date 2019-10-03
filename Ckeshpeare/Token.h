#pragma once
#include <string>

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

        Token();
        Token(Type _type, const std::string &_content);

        Type get_type() const;
        std::string get_content() const;
        std::string to_string() const;
        bool operator == (const Token &_rhs) const;
    private:
        Type type;
        std::string content;
    };
}