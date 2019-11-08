#include "LlParser.h"
#include "debug.h"
#include "ErrorRecorder.h"

namespace dyj {
    LlParser::LlParser(std::vector<Token *> &_tokens) : tokens(_tokens), cur(0U) {}

    Token *dyj::LlParser::peek(size_t _cnt) {
        if (cur + _cnt < tokens.size()) {
            return tokens[cur + _cnt];
        } else {
            // TODO: error
            return nullptr;
        }
    }

    Token *LlParser::pop(void) {
        if (cur < tokens.size()) {
            DP("pop %s\n", tokens[cur]->get_content().c_str());
            return tokens[cur++];
        } else {
            return nullptr;
        }
    }

    Token *LlParser::get(Token::Type _type) {
        Token *t = peek();
        if (t && t->get_type() == _type) {
            return pop();
        } else {
            return nullptr;
        }
    }

    bool LlParser::ended(void) {
        return cur == tokens.size();
    }

    RecursiveDescentParser::RecursiveDescentParser(std::vector<Token *> &_tokens) : LlParser::LlParser(_tokens), tree(nullptr) {}

    Symbol *RecursiveDescentParser::parse(void) {
        tree = parse_program();
        if (ended()) {
            return tree;
        } else {
            return (tree = nullptr);
        }
    }

    Symbol *RecursiveDescentParser::get_tree(void) {
        return tree;
    }

    typedef std::vector<Symbol *> Symbols;

#define PARSE(_type, _name) Symbols symbols; \
    DP("parsing %s:\n", _name); \
    Symbol::Type __type = (_type); \
    std::string __name = (_name)

#define RETURN_PARSE() return new NonterminalSymbol(__type, __name, symbols)

#define GET(_token_type) do {\
        DP("getting %u\n", _token_type); \
        symbols.push_back(new TerminalSymbol(t = get(_token_type))); \
        if (!t) {\
            DP("but failed\n"); \
            return nullptr; \
        } \
        DP("got\n"); \
    } while(0)

#define TAKE(_symbol_func, ...) do {\
        DP("taking:\n"); \
        symbols.push_back(s = _symbol_func(__VA_ARGS__));\
        if (!s) {\
            DP("but failed\n"); \
            return nullptr;\
        }\
        DP("got\n"); \
    } while (0)

    Symbol *RecursiveDescentParser::parse_operator_add(void) {
        PARSE(Symbol::OPERATOR_ADD, "<加法运算符>");
        Token *t = peek();
        if (t && (t->get_type() == Token::PLUS || t->get_type() == Token::MINU)) {
            symbols.push_back(new TerminalSymbol(pop()));
            RETURN_PARSE();
        } else {
            return nullptr;
        }
    }

    Symbol *RecursiveDescentParser::parse_operator_multiply(void) {
        PARSE(Symbol::OPERATOR_MULTIPLY, "<乘法运算符>");
        Token *t = peek();
        if (t && (t->get_type() == Token::MULT || t->get_type() == Token::DIV)) {
            symbols.push_back(new TerminalSymbol(pop()));
            RETURN_PARSE();
        } else {
            return nullptr;
        }
    }

    Symbol *RecursiveDescentParser::parse_operator_relation(void) {
        PARSE(Symbol::OPERATOR_RELATION, "<关系运算符>");
        Token *t = peek();
        if (t) {
            switch (t->get_type()) {
            case Token::LSS:
            case Token::LEQ:
            case Token::GRE:
            case Token::GEQ:
            case Token::NEQ:
            case Token::EQL:
                symbols.push_back(new TerminalSymbol(pop()));
                RETURN_PARSE();
            default:
                break;
            }
            return nullptr;
        } else {
            return nullptr;
        }
    }

    Symbol *RecursiveDescentParser::parse_characters(void) {
        PARSE(Symbol::CHARACTERS, "<字符>");
        Token *t = peek();
        if (t && t->get_type() == Token::CHARCON) {
            symbols.push_back(new TerminalSymbol(pop()));
            RETURN_PARSE();
        } else {
            return nullptr;
        }
    }

    Symbol *RecursiveDescentParser::parse_string(void) {
        PARSE(Symbol::STRING, "<字符串>");
        Token *t = peek();
        GET(Token::STRCON);
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_program(void) {
        PARSE(Symbol::PROGRAM, "<程序>");
        Token *t = peek();
        Symbol *s;
        if (t && t->get_type() == Token::CONSTTK) {
            TAKE(parse_const_declare);
        }
        if (peek(0) && (peek(0)->get_type() == Token::INTTK || peek(0)->get_type() == Token::CHARTK)
            && peek(1) && (peek(1)->get_type() == Token::IDENFR)
            && peek(2) && (peek(2)->get_type() == Token::COMMA || peek(2)->get_type() == Token::SEMICN || peek(2)->get_type() == Token::LBRACK)) {

            TAKE(parse_variable_declare);
        }
        while (peek(0) && (peek(0)->get_type() == Token::INTTK || peek(0)->get_type() == Token::CHARTK || peek(0)->get_type() == Token::VOIDTK)
            && peek(1) && (peek(1)->get_type() == Token::IDENFR)) {

            if (peek(0)->get_type() == Token::VOIDTK) {
                symbols.push_back(s = parse_function_without_return());
            } else {
                symbols.push_back(s = parse_function_with_return());
            }
            if (!s) {
                return nullptr;
            }
        }
        TAKE(parse_main_function);
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_const_declare(void) {
        PARSE(Symbol::CONST_DECLARE, "<常量说明>");
        Token *t = peek();
        Symbol *s;
        do {
            GET(Token::CONSTTK);
            TAKE(parse_const_define);
            if (peek() && peek()->get_type() != Token::SEMICN) {
                rerr.push_back(new Error(Error::SEMI_LOST, s->get_token()->get_location()));
            } else {
                GET(Token::SEMICN);
            }
        } while (peek() && peek()->get_type() == Token::CONSTTK);
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_const_define(void) {
        PARSE(Symbol::CONST_DEFINE, "<常量定义>");
        Token *t = peek();
        Symbol *s;
        FullHouse::CalType ctype;
        bool ret;
        if (t && t->get_type() == Token::INTTK) {
            ctype = FullHouse::INT;
            GET(Token::INTTK);
            GET(Token::IDENFR);
            ret = full_house_table.declare_full_house(new FullHouse(t->get_content(), ctype, FullHouse::VARIABLE, FullHouse::CONST));
            if (!ret) {
                rerr.push_back(new Error(Error::REDEFINITION, t->get_location()));
            }
            GET(Token::ASSIGN);
            if (peek() && peek()->get_type() == Token::CHARCON) {
                rerr.push_back(new Error(Error::CONST_INITIALIZATION_ERROR, peek()->get_location()));
                pop();
            } else {
                TAKE(parse_integer);
            }
            while (peek() && peek()->get_type() == Token::COMMA) {
                GET(Token::COMMA);
                GET(Token::IDENFR);
                ret = full_house_table.declare_full_house(new FullHouse(t->get_content(), ctype, FullHouse::VARIABLE, FullHouse::CONST));
                if (!ret) {
                    rerr.push_back(new Error(Error::REDEFINITION, t->get_location()));
                }
                GET(Token::ASSIGN);
                if (peek() && peek()->get_type() == Token::CHARCON) {
                    rerr.push_back(new Error(Error::CONST_INITIALIZATION_ERROR, peek()->get_location()));
                    pop();
                } else {
                    TAKE(parse_integer);
                }
            }
        } else if (t && t->get_type() == Token::CHARTK) {
            ctype = FullHouse::CHAR;
            GET(Token::CHARTK);
            GET(Token::IDENFR);
            ret = full_house_table.declare_full_house(new FullHouse(t->get_content(), ctype, FullHouse::VARIABLE, FullHouse::CONST));
            if (!ret) {
                rerr.push_back(new Error(Error::REDEFINITION, t->get_location()));
            }
            GET(Token::ASSIGN);
            if (peek() && peek()->get_type() != Token::CHARCON) {
                rerr.push_back(new Error(Error::CONST_INITIALIZATION_ERROR, peek()->get_location()));
                pop();
            } else {
                GET(Token::CHARCON);
            }
            while (peek() && peek()->get_type() == Token::COMMA) {
                GET(Token::COMMA);
                GET(Token::IDENFR);
                ret = full_house_table.declare_full_house(new FullHouse(t->get_content(), ctype, FullHouse::VARIABLE, FullHouse::CONST));
                if (!ret) {
                    rerr.push_back(new Error(Error::REDEFINITION, t->get_location()));
                }
                GET(Token::ASSIGN);
                if (peek() && peek()->get_type() != Token::CHARCON) {
                    rerr.push_back(new Error(Error::CONST_INITIALIZATION_ERROR, peek()->get_location()));
                    pop();
                } else {
                    GET(Token::CHARCON);
                }
            }
        } else {
            return nullptr;
        }
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_unsigned_integer(void) {
        PARSE(Symbol::UNSIGNED_INTEGER, "<无符号整数>");
        Token *t = peek();
        GET(Token::INTCON);
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_integer(void) {
        PARSE(Symbol::INTEGER, "<整数>");
        Token *t = peek();
        Symbol *s;
        if (t && t->get_type() == Token::PLUS) {
            GET(Token::PLUS);
        } else if (t && t->get_type() == Token::MINU) {
            GET(Token::MINU);
        }
        TAKE(parse_unsigned_integer);
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_declare_header(FullHouse::CalType &ctype, std::string &name) {
        PARSE(Symbol::DECLARE_HEADER, "<声明头部>");
        Token *t = peek();
        if (t && t->get_type() == Token::INTTK) {
            GET(Token::INTTK);
            ctype = FullHouse::INT;
        } else if (t && t->get_type() == Token::CHARTK) {
            GET(Token::CHARTK);
            ctype = FullHouse::CHAR;
        } else {
            return nullptr;
        }
        GET(Token::IDENFR);
        name = t->get_content();
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_variable_declare(void) {
        PARSE(Symbol::VARIABLE_DECLARE, "<变量说明>");
        Token *t = peek();
        Symbol *s;
        do {
            TAKE(parse_variable_define);
            if (peek() && peek()->get_type() != Token::SEMICN) {
                rerr.push_back(new Error(Error::SEMI_LOST, s->get_token()->get_location()));
            } else {
                GET(Token::SEMICN);
            }
        } while (peek() && (peek()->get_type() == Token::INTTK || peek()->get_type() == Token::CHARTK) &&
            peek(2) && peek(2)->get_type() != Token::LPARENT);
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_variable_define(void) {
        PARSE(Symbol::VARIABLE_DEFINE, "<变量定义>");
        Token *t = peek();
        Symbol *s;

        FullHouse::CalType ctype;
        FullHouse::BehType btype;
        FullHouse::ConstType ttype = FullHouse::NON_CONST;
        std::string name;
        bool ret;

        TAKE(parse_type_id, ctype);
        GET(Token::IDENFR);
        name = t->get_content();
        if (peek() && peek()->get_type() == Token::LBRACK) {
            btype = FullHouse::ARRAY;
            GET(Token::LBRACK);
            TAKE(parse_unsigned_integer);
            size_t size = atoi(t->get_content().c_str());
            if (peek() && peek()->get_type() != Token::RBRACK) {
                rerr.push_back(new Error(Error::RBRACK_LOST, s->get_token()->get_location()));
            } else {
                GET(Token::RBRACK);
            }
            auto tmp = new FullHouse(name, ctype, btype, ttype);
            tmp->set_size(size);
            ret = full_house_table.declare_full_house(tmp);
            if (!ret) {
                rerr.push_back(new Error(Error::REDEFINITION, t->get_location()));
            }
        } else {
            btype = FullHouse::VARIABLE;
            ret = full_house_table.declare_full_house(new FullHouse(name, ctype, btype, ttype));
            if (!ret) {
                rerr.push_back(new Error(Error::REDEFINITION, t->get_location()));
            }
        }
        while (peek() && peek()->get_type() == Token::COMMA) {
            GET(Token::COMMA);
            GET(Token::IDENFR);
            name = t->get_content();
            if (peek() && peek()->get_type() == Token::LBRACK) {
                btype = FullHouse::ARRAY;
                GET(Token::LBRACK);
                TAKE(parse_unsigned_integer);
                size_t size = atoi(t->get_content().c_str());
                if (peek() && peek()->get_type() != Token::RBRACK) {
                    rerr.push_back(new Error(Error::RBRACK_LOST, s->get_token()->get_location()));
                } else {
                    GET(Token::RBRACK);
                }
                auto tmp = new FullHouse(name, ctype, btype, ttype);
                tmp->set_size(size);
                ret = full_house_table.declare_full_house(tmp);
                if (!ret) {
                    rerr.push_back(new Error(Error::REDEFINITION, t->get_location()));
                }
            } else {
                btype = FullHouse::VARIABLE;
                ret = full_house_table.declare_full_house(new FullHouse(name, ctype, btype, ttype));
                if (!ret) {
                    rerr.push_back(new Error(Error::REDEFINITION, t->get_location()));
                }
            }
        }
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_type_id(FullHouse::CalType &ctype) {
        PARSE(Symbol::TYPE_ID, "");
        Token *t = peek();
        if (t && t->get_type() == Token::INTTK) {
            GET(Token::INTTK);
            ctype = FullHouse::INT;
        } else if (t && t->get_type() == Token::CHARTK) {
            GET(Token::CHARTK);
            ctype = FullHouse::CHAR;
        } else {
            return nullptr;
        }
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_function_with_return(void) {
        PARSE(Symbol::FUNCTION_WITH_RETURN, "<有返回值函数定义>");
        Token *t = peek();
        Symbol *s;

        FullHouse::CalType ctype;
        FullHouse::BehType btype = FullHouse::FUNCTION;
        FullHouse::ConstType ttype = FullHouse::NON_CONST;
        std::string name;
        FullHouse *tmp;
        bool ret;

        TAKE(parse_declare_header, ctype, name);
        //function_table[s->at(1).get_token()->get_content()] = true;
        tmp = new FullHouse(name, ctype, btype, ttype);
        ret = full_house_table.declare_full_house(tmp);
        if (!ret) {
            rerr.push_back(new Error(Error::REDEFINITION, t->get_location()));
        }
        full_house_table.push_layer();
        GET(Token::LPARENT);
        TAKE(parse_parameter_list, tmp);
        if (peek() && peek()->get_type() != Token::RPARENT) {
            rerr.push_back(new Error(Error::RPARENT_LOST, s->get_token()->get_location()));
        } else {
            GET(Token::RPARENT);
        }
        GET(Token::LBRACE);
        TAKE(parse_multiple_statement, ctype);
        GET(Token::RBRACE);
        full_house_table.pop_layer();

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_function_without_return(void) {
        PARSE(Symbol::FUNCTION_WITHOUT_RETURN, "<无返回值函数定义>");
        Token *t = peek();
        Symbol *s;

        FullHouse::CalType ctype = FullHouse::VOID;
        FullHouse::BehType btype = FullHouse::FUNCTION;
        FullHouse::ConstType ttype = FullHouse::NON_CONST;
        std::string name;
        FullHouse *tmp;
        bool ret;

        GET(Token::VOIDTK);
        GET(Token::IDENFR);
        name = t->get_content();
        //function_table[t->get_content()] = false;
        tmp = new FullHouse(name, ctype, btype, ttype);
        ret = full_house_table.declare_full_house(tmp);
        if (!ret) {
            rerr.push_back(new Error(Error::REDEFINITION, t->get_location()));
        }
        full_house_table.push_layer();
        GET(Token::LPARENT);
        TAKE(parse_parameter_list, tmp);
        if (peek() && peek()->get_type() != Token::RPARENT) {
            rerr.push_back(new Error(Error::RPARENT_LOST, s->get_token()->get_location()));
        } else {
            GET(Token::RPARENT);
        }
        GET(Token::LBRACE);
        TAKE(parse_multiple_statement, ctype);
        GET(Token::RBRACE);
        full_house_table.pop_layer();

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_multiple_statement(const FullHouse::CalType &ctin) {
        PARSE(Symbol::MULTIPLE_STATEMENT, "<复合语句>");
        Token *t = peek();
        Symbol *s;
        if (t && t->get_type() == Token::CONSTTK) {
            TAKE(parse_const_declare);
        }
        if (peek(0) && (peek(0)->get_type() == Token::INTTK || peek(0)->get_type() == Token::CHARTK)
            && peek(1) && (peek(1)->get_type() == Token::IDENFR)
            && peek(2) && (peek(2)->get_type() == Token::COMMA || peek(2)->get_type() == Token::SEMICN || peek(2)->get_type() == Token::LBRACK)) {

            TAKE(parse_variable_declare);
        }
        TAKE(parse_block, ctin);
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_parameter_list(FullHouse *fh) {
        PARSE(Symbol::PARAMETER_LIST, "<参数表>");
        Token *t = peek();
        Symbol *s;

        FullHouse::CalType ctype;
        FullHouse::BehType btype = FullHouse::VARIABLE;
        FullHouse::ConstType ttype = FullHouse::NON_CONST;
        std::string name;
        bool ret;

        if (t && (t->get_type() == Token::INTTK || t->get_type() == Token::CHARTK)) {
            TAKE(parse_type_id, ctype);
            fh->append_param(ctype);
            GET(Token::IDENFR);
            name = t->get_content();
            ret = full_house_table.declare_full_house(new FullHouse(name, ctype, btype, ttype));
            if (!ret) {
                rerr.push_back(new Error(Error::REDEFINITION, t->get_location()));
            }
            while (peek() && peek()->get_type() == Token::COMMA) {
                GET(Token::COMMA);
                TAKE(parse_type_id, ctype);
                fh->append_param(ctype);
                GET(Token::IDENFR);
                name = t->get_content();
                ret = full_house_table.declare_full_house(new FullHouse(name, ctype, btype, ttype));
                if (!ret) {
                    rerr.push_back(new Error(Error::REDEFINITION, t->get_location()));
                }
            }
        }
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_main_function(void) {
        PARSE(Symbol::MAIN_FUNCTION, "<主函数>");
        Token *t = peek();
        Symbol *s;

        GET(Token::VOIDTK);
        GET(Token::MAINTK);
        GET(Token::LPARENT);
        if (peek() && peek()->get_type() != Token::RPARENT) {
            rerr.push_back(new Error(Error::RPARENT_LOST, t->get_location()));
        } else {
            GET(Token::RPARENT);
        }
        GET(Token::LBRACE);
        TAKE(parse_multiple_statement, FullHouse::VOID);
        GET(Token::RBRACE);

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_expression(FullHouse::CalType &ctype) {
        PARSE(Symbol::EXPRESSION, "<表达式>");
        Token *t = peek();
        Symbol *s;

        bool flag = false;
        FullHouse::CalType ctmp;

        if (t && t->get_type() == Token::PLUS) {
            flag = true;
            GET(Token::PLUS);
        } else if (t && t->get_type() == Token::MINU) {
            flag = true;
            GET(Token::MINU);
        }
        TAKE(parse_term, ctmp);
        if (!flag) {
            ctype = ctmp;
        } else {
            ctype = FullHouse::INT;
        }
        while (peek() && (peek()->get_type() == Token::PLUS || peek()->get_type() == Token::MINU)) {
            if (peek()->get_type() == Token::PLUS) {
                ctype = FullHouse::INT;
                GET(Token::PLUS);
            } else {
                ctype = FullHouse::INT;
                GET(Token::MINU);
            }
            TAKE(parse_term, ctmp);
        }

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_term(FullHouse::CalType &ctype) {
        PARSE(Symbol::TERM, "<项>");
        Token *t = peek();
        Symbol *s;

        FullHouse::CalType ctmp;

        TAKE(parse_factor, ctmp);
        ctype = ctmp;
        while (peek() && (peek()->get_type() == Token::MULT || peek()->get_type() == Token::DIV)) {
            if (peek()->get_type() == Token::MULT) {
                GET(Token::MULT);
            } else {
                GET(Token::DIV);
            }
            TAKE(parse_factor, ctmp);
            ctype = FullHouse::INT;
        }

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_factor(FullHouse::CalType &ctype) {
        PARSE(Symbol::FACTOR, "<因子>");
        Token *t = peek();
        Symbol *s;

        FullHouse *fh;
        std::string name;
        FullHouse::CalType ctmp;

        if (t && t->get_type() == Token::IDENFR) {
            if (peek(1) && peek(1)->get_type() == Token::LPARENT) {
                TAKE(parse_call_with_return, fh);
                ctype = fh->get_calculation();
            } else {
                GET(Token::IDENFR);
                name = t->get_content();
                fh = full_house_table.lookup_full_house(name);
                if (!fh) {
                    rerr.push_back(new Error(Error::UNDEFINED, t->get_location()));
                } else if (fh->get_behavior() == FullHouse::ILL) {
                    DP("???is void\n");
                    rerr.push_back(new Error(Error::UNKNOWN_ERROR, t->get_location()));
                }
                if (peek() && peek()->get_type() == Token::LBRACK) {
                    if (fh && fh->get_behavior() == FullHouse::VARIABLE) {
                        DP("???not an array\n");
                        rerr.push_back(new Error(Error::UNKNOWN_ERROR, t->get_location()));
                    }
                    GET(Token::LBRACK);
                    TAKE(parse_expression, ctmp);
                    if (ctmp != FullHouse::INT) {
                        rerr.push_back(new Error(Error::INDEX_ERROR, s->get_token()->get_location()));
                    }
                    if (peek() && peek()->get_type() != Token::RBRACK) {
                        rerr.push_back(new Error(Error::RBRACK_LOST, s->get_token()->get_location()));
                    } else {
                        GET(Token::RBRACK);
                    }
                }
                ctype = fh ? fh->get_calculation() : FullHouse::VOID;
            }
        } else if (t && t->get_type() == Token::LPARENT) {
            GET(Token::LPARENT);
            TAKE(parse_expression, ctmp);
            if (peek() && peek()->get_type() != Token::RPARENT) {
                rerr.push_back(new Error(Error::RPARENT_LOST, s->get_token()->get_location()));
            } else {
                GET(Token::RPARENT);
            }
            ctype = FullHouse::INT;
        } else if (t && (t->get_type() == Token::INTCON || t->get_type() == Token::PLUS || t->get_type() == Token::MINU)) {
            TAKE(parse_integer);
            ctype = FullHouse::INT;
        } else if (t && t->get_type() == Token::CHARCON) {
            GET(Token::CHARCON);
            ctype = FullHouse::CHAR;
        } else {
            DP("???really not known\n");
            rerr.push_back(new Error(Error::UNKNOWN_ERROR, peek()->get_location()));
            return nullptr;
        }
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_statement(const FullHouse::CalType &ctin) {
        PARSE(Symbol::STATEMENT, "<语句>");
        Token *t = peek();
        Symbol *s;

        FullHouse::CalType ctype;

        if (t && t->get_type() == Token::IFTK) {
            TAKE(parse_if_statement, ctin);
        } else if (t && (t->get_type() == Token::WHILETK || t->get_type() == Token::DOTK || t->get_type() == Token::FORTK)) {
            TAKE(parse_loop_statement, ctin);
        } else if (t && t->get_type() == Token::LBRACE) {
            GET(Token::LBRACE);
            TAKE(parse_block, ctin);
            GET(Token::RBRACE);
        } else if (t && t->get_type() == Token::IDENFR) {
            if (peek(1) && (peek(1)->get_type() == Token::ASSIGN || peek(1)->get_type() == Token::LBRACK)) {
                TAKE(parse_assign_statement);
                if (peek() && peek()->get_type() != Token::SEMICN) {
                    rerr.push_back(new Error(Error::SEMI_LOST, s->get_token()->get_location()));
                } else {
                    GET(Token::SEMICN);
                }
            } else {
                // TODO: loop up and solve
                std::string name = t->get_content();
                FullHouse *fh = full_house_table.lookup_full_house(name);
                if (!fh) {
                    rerr.push_back(new Error(Error::UNDEFINED, t->get_location()));
                    return nullptr;
                } else if (fh->get_calculation() == FullHouse::VOID) {
                    TAKE(parse_call_without_return, fh);
                } else {
                    TAKE(parse_call_with_return, fh);
                }
            }
        } else if (t && t->get_type() == Token::SCANFTK) {
            TAKE(parse_scanf);
            if (peek() && peek()->get_type() != Token::SEMICN) {
                rerr.push_back(new Error(Error::SEMI_LOST, s->get_token()->get_location()));
            } else {
                GET(Token::SEMICN);
            }
        } else if (t && t->get_type() == Token::PRINTFTK) {
            TAKE(parse_printf);
            if (peek() && peek()->get_type() != Token::SEMICN) {
                rerr.push_back(new Error(Error::SEMI_LOST, s->get_token()->get_location()));
            } else {
                GET(Token::SEMICN);
            }
        } else if (t && t->get_type() == Token::RETURNTK) {
            TAKE(parse_return, ctype);
            if (ctin == FullHouse::VOID && ctype != ctin) {
                rerr.push_back(new Error(Error::VOID_RETURN_DISMATCH, s->get_token()->get_location()));
            } else if (ctin != ctype) {
                DP("UNVOID DISMATCH %d, %d\n", ctin, ctype);
                rerr.push_back(new Error(Error::UNVOID_RETURN_DISMATCH, s->get_token()->get_location()));
            }
            if (peek() && peek()->get_type() != Token::SEMICN) {
                rerr.push_back(new Error(Error::SEMI_LOST, peek()->get_location()));
            } else {
                GET(Token::SEMICN);
            }
        } else if (t && t->get_type() == Token::SEMICN) {
            if (peek() && peek()->get_type() != Token::SEMICN) {
                rerr.push_back(new Error(Error::SEMI_LOST, peek()->get_location()));
            } else {
                GET(Token::SEMICN);
            }
        }
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_assign_statement(void) {
        PARSE(Symbol::ASSIGN_STATEMENT, "<赋值语句>");
        Token *t = peek();
        Symbol *s;

        std::string name;
        FullHouse *fh;
        FullHouse::CalType ctmp, lhs, rhs;

        if (peek(1) && peek(1)->get_type() == Token::LBRACK) {
            GET(Token::IDENFR);
            name = t->get_content();
            fh = full_house_table.lookup_full_house(name);
            lhs = FullHouse::VOID;
            if (!fh) {
                rerr.push_back(new Error(Error::UNDEFINED, t->get_location()));
            } else if (fh->get_constant() == FullHouse::CONST) {
                rerr.push_back(new Error(Error::CONST_DISCARD, t->get_location()));
            } else if (fh->get_behavior() != FullHouse::ARRAY) {
                DP("???not an array\n");
                rerr.push_back(new Error(Error::UNKNOWN_ERROR, t->get_location()));
            } else {
                lhs = fh->get_calculation();
            }
            GET(Token::LBRACK);
            TAKE(parse_expression, ctmp);
            if (ctmp != FullHouse::INT) {
                rerr.push_back(new Error(Error::INDEX_ERROR, s->get_token()->get_location()));
            }
            if (peek() && peek()->get_type() != Token::RBRACK) {
                rerr.push_back(new Error(Error::RBRACK_LOST, s->get_token()->get_location()));
            } else {
                GET(Token::RBRACK);
            }
            GET(Token::ASSIGN);
            TAKE(parse_expression, rhs);
            if (lhs != rhs) {
                DP("???mismatched types");
                rerr.push_back(new Error(Error::UNKNOWN_ERROR, s->get_token()->get_location()));
            }
        } else {
            GET(Token::IDENFR);
            name = t->get_content();
            fh = full_house_table.lookup_full_house(name);
            lhs = FullHouse::VOID;
            if (!fh) {
                rerr.push_back(new Error(Error::UNDEFINED, t->get_location()));
            } else if (fh->get_constant() == FullHouse::CONST) {
                rerr.push_back(new Error(Error::CONST_DISCARD, t->get_location()));
            } else if (fh->get_behavior() != FullHouse::VARIABLE) {
                DP("???not an variable\n");
                rerr.push_back(new Error(Error::UNKNOWN_ERROR, t->get_location()));
            } else {
                lhs = fh->get_calculation();
            }
            GET(Token::ASSIGN);
            TAKE(parse_expression, rhs);
            if (lhs != rhs) {
                DP("mismatched types");
                rerr.push_back(new Error(Error::UNKNOWN_ERROR, s->get_token()->get_location()));
            }
        }

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_if_statement(const FullHouse::CalType &ctin) {
        PARSE(Symbol::IF_STATEMENT, "<条件语句>");
        Token *t = peek();
        Symbol *s;

        GET(Token::IFTK);
        GET(Token::LPARENT);
        TAKE(parse_condition);
        if (peek() && peek()->get_type() != Token::RPARENT) {
            rerr.push_back(new Error(Error::RPARENT_LOST, s->get_token()->get_location()));
        } else {
            GET(Token::RPARENT);
        }
        TAKE(parse_statement, ctin);
        if (peek() && peek()->get_type() == Token::ELSETK) {
            GET(Token::ELSETK);
            TAKE(parse_statement, ctin);
        }

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_condition(void) {
        PARSE(Symbol::CONDITION, "<条件>");
        Symbol *s;

        FullHouse::CalType lhs, rhs;

        TAKE(parse_expression, lhs);
        if (peek() && Token::is_relation(peek())) {
            symbols.push_back(new TerminalSymbol(pop()));
            TAKE(parse_expression, rhs);
            if (lhs != rhs) {
                rerr.push_back(new Error(Error::UNKNOWN_TYPE_CONDITION, s->get_token()->get_location()));
            }
        }

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_loop_statement(const FullHouse::CalType &ctin) {
        PARSE(Symbol::LOOP_STATEMENT, "<循环语句>");
        Token *t = peek();
        Symbol *s;

        FullHouse *fh;
        std::string name;
        FullHouse::CalType lhs, rhs;

        if (t && t->get_type() == Token::WHILETK) {
            GET(Token::WHILETK);
            GET(Token::LPARENT);
            TAKE(parse_condition);
            if (peek() && peek()->get_type() != Token::RPARENT) {
                rerr.push_back(new Error(Error::RPARENT_LOST, s->get_token()->get_location()));
            } else {
                GET(Token::RPARENT);
            }
            TAKE(parse_statement, ctin);
        } else if (t && t->get_type() == Token::DOTK) {
            GET(Token::DOTK);
            TAKE(parse_statement, ctin);
            if (peek() && peek()->get_type() != Token::WHILETK) {
                rerr.push_back(new Error(Error::WHILE_LOST, peek()->get_location()));
            } else {
                GET(Token::WHILETK);
            }
            GET(Token::LPARENT);
            TAKE(parse_condition);
            if (peek() && peek()->get_type() != Token::RPARENT) {
                rerr.push_back(new Error(Error::RPARENT_LOST, s->get_token()->get_location()));
            } else {
                GET(Token::RPARENT);
            }
        } else if (t && t->get_type() == Token::FORTK) {
            GET(Token::FORTK);
            GET(Token::LPARENT);
            GET(Token::IDENFR);
            name = t->get_content();
            fh = full_house_table.lookup_full_house(name);
            if (!fh) {
                rerr.push_back(new Error(Error::UNDEFINED, t->get_location()));
            } else if (fh->get_constant() == FullHouse::CONST) {
                rerr.push_back(new Error(Error::CONST_DISCARD, t->get_location()));
            } else if (fh->get_behavior() != FullHouse::VARIABLE) {
                DP("???not an variable\n");
                rerr.push_back(new Error(Error::UNKNOWN_ERROR, t->get_location()));
            }
            lhs = fh ? fh->get_calculation() : FullHouse::VOID;
            GET(Token::ASSIGN);
            TAKE(parse_expression, rhs);
            if (lhs != rhs) {
                DP("???mismatched types\n");
                rerr.push_back(new Error(Error::UNKNOWN_ERROR, s->get_token()->get_location()));
            }
            GET(Token::SEMICN);
            TAKE(parse_condition);
            GET(Token::SEMICN);
            GET(Token::IDENFR);
            name = t->get_content();
            fh = full_house_table.lookup_full_house(name);
            if (!fh) {
                rerr.push_back(new Error(Error::UNDEFINED, t->get_location()));
            } else if (fh->get_constant() == FullHouse::CONST) {
                rerr.push_back(new Error(Error::CONST_DISCARD, t->get_location()));
            } else if (fh->get_behavior() != FullHouse::VARIABLE) {
                DP("???not an variable\n");
                rerr.push_back(new Error(Error::UNKNOWN_ERROR, t->get_location()));
            }
            GET(Token::ASSIGN);
            GET(Token::IDENFR);
            name = t->get_content();
            fh = full_house_table.lookup_full_house(name);
            if (!fh) {
                rerr.push_back(new Error(Error::UNDEFINED, t->get_location()));
            } else if (fh->get_behavior() != FullHouse::VARIABLE) {
                DP("???not an variable\n");
                rerr.push_back(new Error(Error::UNKNOWN_ERROR, t->get_location()));
            }
            if (peek() && peek()->get_type() == Token::PLUS) {
                GET(Token::PLUS);
            } else if (peek() && peek()->get_type() == Token::MINU) {
                GET(Token::MINU);
            } else {
                return nullptr;
            }
            TAKE(parse_step);
            if (peek() && peek()->get_type() != Token::RPARENT) {
                rerr.push_back(new Error(Error::RPARENT_LOST, s->get_token()->get_location()));
            } else {
                GET(Token::RPARENT);
            }
            TAKE(parse_statement, ctin);
        } else {
            return nullptr;
        }

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_step(void) {
        PARSE(Symbol::STEP, "<步长>");
        Symbol *s;
        TAKE(parse_unsigned_integer);
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_call_with_return(FullHouse *&ret) {
        PARSE(Symbol::CALL_WITH_RETURN, "<有返回值函数调用语句>");
        Token *t = peek();
        Symbol *s;

        FullHouse *fh;
        std::string name;
        std::vector<FullHouse::CalType> params;

        GET(Token::IDENFR);
        name = t->get_content();
        fh = full_house_table.lookup_full_house(name);
        if (!fh) {
            rerr.push_back(new Error(Error::UNDEFINED, t->get_location()));
        } else if (fh->get_calculation() == FullHouse::VOID) {
            rerr.push_back(new Error(Error::UNKNOWN_ERROR, t->get_location()));
        }
        GET(Token::LPARENT);
        TAKE(parse_argument_list, params);
        if (peek() && peek()->get_type() != Token::RPARENT) {
            rerr.push_back(new Error(Error::RPARENT_LOST, s->get_token()->get_location()));
        } else {
            GET(Token::RPARENT);
        }
        if (params.size() != fh->get_array_size()) {
            rerr.push_back(new Error(Error::UNMATCHED_PARAM_COUNT, t->get_location()));
        } else if (!fh->same_params(params)) {
            rerr.push_back(new Error(Error::UNMATCHED_PARAM_TYPE, t->get_location()));
        }
        ret = fh;

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_call_without_return(FullHouse *&ret) {
        PARSE(Symbol::CALL_WITHOUT_RETURN, "<无返回值函数调用语句>");
        Token *t = peek();
        Symbol *s;

        FullHouse *fh;
        std::string name;
        std::vector<FullHouse::CalType> params;

        GET(Token::IDENFR);
        name = t->get_content();
        fh = full_house_table.lookup_full_house(name);
        GET(Token::LPARENT);
        TAKE(parse_argument_list, params);
        if (peek() && peek()->get_type() != Token::RPARENT) {
            rerr.push_back(new Error(Error::RPARENT_LOST, s->get_token()->get_location()));
        } else {
            GET(Token::RPARENT);
        }
        if (params.size() != fh->get_array_size()) {
            rerr.push_back(new Error(Error::UNMATCHED_PARAM_COUNT, t->get_location()));
        } else if (!fh->same_params(params)) {
            rerr.push_back(new Error(Error::UNMATCHED_PARAM_TYPE, t->get_location()));
        }
        ret = fh;

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_argument_list(std::vector<FullHouse::CalType> &args) {
        PARSE(Symbol::ARGUMENT_LIST, "<值参数表>");
        Token *t = peek();
        Symbol *s;

        FullHouse::CalType ctype;

        args.clear();

        if (t) {
            switch (t->get_type()) {
            case Token::PLUS:
            case Token::MINU:
            case Token::IDENFR:
            case Token::INTCON:
            case Token::CHARCON:
            case Token::LPARENT:
                TAKE(parse_expression, ctype);
                args.push_back(ctype);
                while (peek() && peek()->get_type() == Token::COMMA) {
                    GET(Token::COMMA);
                    TAKE(parse_expression, ctype);
                    args.push_back(ctype);
                }
                break;
            default:
                break;
            }
        }
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_block(const FullHouse::CalType &ctin) {
        PARSE(Symbol::BLOCK, "<语句列>");
        Token *t = peek();
        Symbol *s;

        while ((t = peek()) && peek()->get_type() != Token::RBRACE) {
            if (t) {
                switch (t->get_type()) {
                case Token::IFTK:
                case Token::FORTK:
                case Token::DOTK:
                case Token::WHILETK:
                case Token::LBRACE:
                case Token::IDENFR:
                case Token::SCANFTK:
                case Token::PRINTFTK:
                case Token::RETURNTK:
                case Token::SEMICN:
                    TAKE(parse_statement, ctin);
                    continue;
                default:
                    break;
                }
            }
            break;
        }

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_scanf(void) {
        PARSE(Symbol::SCANF, "<读语句>");
        Token *t = peek();

        std::string name;
        FullHouse *fh;

        GET(Token::SCANFTK);
        GET(Token::LPARENT);
        GET(Token::IDENFR);
        name = t->get_content();
        fh = full_house_table.lookup_full_house(name);
        if (!fh) {
            rerr.push_back(new Error(Error::UNDEFINED, t->get_location()));
        } else if (fh->get_constant() == FullHouse::CONST) {
            rerr.push_back(new Error(Error::CONST_DISCARD, t->get_location()));
        } else if (fh->get_behavior() != FullHouse::FUNCTION) {
            rerr.push_back(new Error(Error::UNKNOWN_ERROR, t->get_location()));
        }
        while (peek() && peek()->get_type() == Token::COMMA) {
            GET(Token::COMMA);
            GET(Token::IDENFR);
            name = t->get_content();
            fh = full_house_table.lookup_full_house(name);
            if (!fh) {
                rerr.push_back(new Error(Error::UNDEFINED, t->get_location()));
            } else if (fh->get_constant() == FullHouse::CONST) {
                rerr.push_back(new Error(Error::CONST_DISCARD, t->get_location()));
            } else if (fh->get_behavior() == FullHouse::FUNCTION) {
                rerr.push_back(new Error(Error::UNKNOWN_ERROR, t->get_location()));
            }
        }
        GET(Token::RPARENT);

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_printf(void) {
        PARSE(Symbol::SCANF, "<写语句>");
        Token *t = peek();
        Symbol *s;

        FullHouse::CalType ctype;

        GET(Token::PRINTFTK);
        GET(Token::LPARENT);
        if (peek() && peek()->get_type() == Token::STRCON) {
            TAKE(parse_string);
            if (peek() && peek()->get_type() == Token::COMMA) {
                GET(Token::COMMA);
                TAKE(parse_expression, ctype);
            }
        } else {
            TAKE(parse_expression, ctype);
        }
        if (peek() && peek()->get_type() != Token::RPARENT) {
            rerr.push_back(new Error(Error::RPARENT_LOST, s->get_token()->get_location()));
        } else {
            GET(Token::RPARENT);
        }

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_return(FullHouse::CalType &ctype) {
        PARSE(Symbol::RETURN, "<返回语句>");
        Token *t = peek();
        Symbol *s;

        GET(Token::RETURNTK);
        if (peek() && peek()->get_type() == Token::LPARENT) {
            GET(Token::LPARENT);
            TAKE(parse_expression, ctype);
            if (peek() && peek()->get_type() != Token::RPARENT) {
                rerr.push_back(new Error(Error::RPARENT_LOST, s->get_token()->get_location()));
            } else {
                GET(Token::RPARENT);
            }
        }

        RETURN_PARSE();
    }
}


