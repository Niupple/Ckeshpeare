#include "Tokenizer.h"
#include "debug.h"
#include <cassert>

namespace dyj {
    Tokenizer::Tokenizer() : cur(0) {}

    Tokenizer Tokenizer::InitCTokenizer::get_cTokenizer() {
        DP("get_cTokenizer is called\n");
        Tokenizer ct;
        vector<MatcherBase *> &mat = ct.matchers;
        mat.push_back(new IntConstantMatcher());
        mat.push_back(new CharConstantMatcher());
        mat.push_back(new StringConstantMatcher());
        mat.push_back(new KeywordMatcher("const", Token::CONSTTK));
        mat.push_back(new KeywordMatcher("int", Token::INTTK));
        mat.push_back(new KeywordMatcher("char", Token::CHARTK));
        mat.push_back(new KeywordMatcher("void", Token::VOIDTK));
        mat.push_back(new KeywordMatcher("main", Token::MAINTK));
        mat.push_back(new KeywordMatcher("if", Token::IFTK));
        mat.push_back(new KeywordMatcher("else", Token::ELSETK));
        mat.push_back(new KeywordMatcher("do", Token::DOTK));
        mat.push_back(new KeywordMatcher("while", Token::WHILETK));
        mat.push_back(new KeywordMatcher("for", Token::FORTK));
        mat.push_back(new KeywordMatcher("scanf", Token::SCANFTK));
        mat.push_back(new KeywordMatcher("printf", Token::PRINTFTK));
        mat.push_back(new KeywordMatcher("return", Token::RETURNTK));
        mat.push_back(new KeywordMatcher("+", Token::PLUS));
        mat.push_back(new KeywordMatcher("-", Token::MINU));
        mat.push_back(new KeywordMatcher("*", Token::MULT));
        mat.push_back(new KeywordMatcher("/", Token::DIV));
        mat.push_back(new KeywordMatcher("<", Token::LSS));
        mat.push_back(new KeywordMatcher("<=", Token::LEQ));
        mat.push_back(new KeywordMatcher(">", Token::GRE));
        mat.push_back(new KeywordMatcher(">=", Token::GEQ));
        mat.push_back(new KeywordMatcher("==", Token::EQL));
        mat.push_back(new KeywordMatcher("!=", Token::NEQ));
        mat.push_back(new KeywordMatcher("=", Token::ASSIGN));
        mat.push_back(new KeywordMatcher(";", Token::SEMICN));
        mat.push_back(new KeywordMatcher(",", Token::COMMA));
        mat.push_back(new KeywordMatcher("(", Token::LPARENT));
        mat.push_back(new KeywordMatcher(")", Token::RPARENT));
        mat.push_back(new KeywordMatcher("[", Token::LBRACK));
        mat.push_back(new KeywordMatcher("]", Token::RBRACK));
        mat.push_back(new KeywordMatcher("{", Token::LBRACE));
        mat.push_back(new KeywordMatcher("}", Token::RBRACE));
        mat.push_back(new IdentifierMatcher());
        DP("mat len is %u\n", ct.matchers.size());
        return std::move(ct);
    }

    void Tokenizer::feed(const string &_content) {
        content = _content;
        DP("cTokenizer has a vector of length %u\n", matchers.size());
    }

    Token *Tokenizer::get_token(void) {
        DP("IN get_token:\n");
        skip_spaces();
        clear();
        do {
            push();
        } while (!time_to_get());
        Token *ret = retrive_token();
        if (ret) {
            DP("got token : %s", ret->to_string().c_str());
        }
        return ret;
    }

    char Tokenizer::peek(void) {
        if (cur < content.size()) {
            return content[cur];
        } else {
            return EOF;
        }
    }

    char Tokenizer::pop(void) {
        if (cur < content.size()) {
            return content[cur++];
        } else {
            return EOF;
        }
    }

    void Tokenizer::push(void) {
        for (auto p = possible.begin(); p != possible.end(); ) {
            DP("push %c to %u, ", peek(), *p);
            if (matchers[*p]->feed(peek()) == MatcherBase::FAILED) {
                DP("Failed\n");
                p = possible.erase(p);
            } else {
                DP("OKed\n");
                ++p;
            }
        }
        pop();
    }

    Token *Tokenizer::retrive_token(void) {
        for (auto p : possible) {
            if (matchers[p]->get_status() == MatcherBase::MATCHED) {
                return matchers[p]->get_token();
            }
        }
        //assert(0);
        return nullptr;
    }

    void Tokenizer::clear(void) {
        for (auto p : matchers) {
            p->clear();
        }
        possible.clear();
        for (size_t i = 0; i < matchers.size(); ++i) {
            possible.push_back(i);
        }
    }

    void Tokenizer::skip_spaces(void) {
        char c;
        while ((c = peek()) != EOF && is_space(c)) {
            pop();
        }
    }

    bool Tokenizer::time_to_get(void) {
        DP("checking time to get:\n");
        if (peek() == EOF) {
            return true;
        } else {
            for (auto p = possible.begin(); p != possible.end(); ++p) {
                DP("peeking %u:\n", *p);
                if(matchers[*p]->peek(peek()) != MatcherBase::FAILED) {
                    DP("peeking %u, not failed\n", *p);
                    return false;
                }
            }
            return true;
        }
    }

    Tokenizer Tokenizer::cTokenizer = Tokenizer::InitCTokenizer::get_cTokenizer();

}
