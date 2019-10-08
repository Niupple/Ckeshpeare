#include "Symbol.h"

namespace dyj {
    Symbol::Symbol(void) {}

    Symbol::Symbol(std::string _name) : name(_name) {}

    std::string Symbol::to_string(void) {
        return name;
    }

    TerminalSymbol::TerminalSymbol(void) : Symbol::Symbol(), token(nullptr) {}

    TerminalSymbol::TerminalSymbol(Token *_token) : Symbol::Symbol(std::string()), token(_token) {}

    Symbol::Terminal TerminalSymbol::get_terminal(void) {
        return Terminal::TERMINAL;
    }

    std::string TerminalSymbol::to_string(void) {
        return token->to_string();
    }

    NonterminalSymbol::NonterminalSymbol(void) : type(DEFAULT) {}

    NonterminalSymbol::NonterminalSymbol(Symbol::Type _type, std::string _name, const std::vector<Symbol *> &_tokens) : Symbol::Symbol(_name), type(_type), symbols(_tokens) {}

    Symbol::Terminal NonterminalSymbol::get_terminal(void) {
        return NONTERMINAL;
    }

    std::string NonterminalSymbol::to_string(void) {
        std::string ret;
        for (Symbol *i : symbols) {
            ret += i->to_string() + "\n";
        }
        ret += Symbol::to_string();
        return ret;
    }

}
