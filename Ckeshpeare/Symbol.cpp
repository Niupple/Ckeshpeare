#include "Symbol.h"

namespace dyj {
    Symbol::Symbol(void) {}

    Symbol::Symbol(std::string _name) : name(_name) {}

    std::string Symbol::to_string(void) const {
        return name;
    }

    TerminalSymbol::TerminalSymbol(void) : Symbol::Symbol(), token(nullptr) {}

    TerminalSymbol::TerminalSymbol(Token *_token) : Symbol::Symbol(std::string()), token(_token) {}

    Symbol::Terminal TerminalSymbol::get_terminal(void) const {
        return Terminal::TERMINAL;
    }

    std::string TerminalSymbol::to_string(void) const {
        return token->to_string();
    }

    Token *TerminalSymbol::get_token(void) const {
        return token;
    }

    const Symbol &TerminalSymbol::at(size_t _idx) const {
        return *this;
    }

    NonterminalSymbol::NonterminalSymbol(void) : type(DEFAULT) {}

    NonterminalSymbol::NonterminalSymbol(Symbol::Type _type, std::string _name, const std::vector<Symbol *> &_tokens) : Symbol::Symbol(_name), symbols(_tokens) , type(_type){}

    Symbol::Terminal NonterminalSymbol::get_terminal(void) const {
        return NONTERMINAL;
    }

    std::string NonterminalSymbol::to_string(void) const {
        std::string ret, cur = Symbol::to_string();
        for (Symbol *i : symbols) {
            ret += i->to_string() + "\n";
        }
        if (cur.size() > 0) {
            ret += cur;
            return ret;
        } else {
            return ret.substr(0, ret.size() - 1);
        }
    }

    Token *NonterminalSymbol::get_token(void) const {
        return symbols.back()->get_token();
    }

    const Symbol &NonterminalSymbol::at(size_t _idx) const {
        return *symbols.at(_idx);
    }

}
