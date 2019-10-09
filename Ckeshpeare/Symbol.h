#pragma once
#include <string>
#include <vector>
#include "Token.h"

namespace dyj {
    class Symbol {
    public:
        enum Terminal {
            TERMINAL,
            NONTERMINAL
        };

        enum Type {
            DEFAULT,
            OPERATOR_ADD,
            OPERATOR_MULTIPLY,
            OPERATOR_RELATION,
            ALPHA,
            NUMBERS,
            NUMBERS_EXCEPT_ZERO,
            CHARACTERS,
            STRING,
            PROGRAM,
            CONST_DECLARE,
            CONST_DEFINE,
            UNSIGNED_INTEGER,
            INTEGER,
            IDENTIFIER,
            DECLARE_HEADER,
            VARIABLE_DECLARE,
            VARIABLE_DEFINE,
            TYPE_ID,
            FUNCTION_WITH_RETURN,
            FUNCTION_WITHOUT_RETURN,
            MULTIPLE_STATEMENT,
            PARAMETER_LIST,
            MAIN_FUNCTION,
            EXPRESSION,
            TERM,
            FACTOR,
            STATEMENT,
            ASSIGN_STATEMENT,
            IF_STATEMENT,
            CONDITION,
            LOOP_STATEMENT,
            STEP,
            CALL_WITH_RETURN,
            CALL_WITHOUT_RETURN,
            ARGUMENT_LIST,
            BLOCK,
            SCANF,
            PRINTF,
            RETURN
        };

        Symbol(void);
        Symbol(std::string _name);

        virtual Terminal get_terminal(void) const = 0;
        virtual std::string to_string(void) const;
        virtual Token *get_token(void) const = 0;

    public:
        virtual const Symbol &at(size_t _idx) const = 0;

    private:
        std::string name;
    };

    class TerminalSymbol : public Symbol {
    public:

        TerminalSymbol(void);
        TerminalSymbol(Token *_token);

        Terminal get_terminal(void) const;
        std::string to_string(void) const;
        Token *get_token(void) const;

    private:
        Token *token;

    public:
        const Symbol &at(size_t _idx) const;

    };

    class NonterminalSymbol : public Symbol {
    public:
        NonterminalSymbol(void);
        NonterminalSymbol(Symbol::Type, std::string, const std::vector<Symbol *> &);

        Terminal get_terminal(void) const;
        std::string to_string(void) const;
        Token *get_token(void) const;

    private:
        std::vector<Symbol *> symbols;
        Type type;

    public:
        const Symbol &at(size_t _idx) const;
    };
}