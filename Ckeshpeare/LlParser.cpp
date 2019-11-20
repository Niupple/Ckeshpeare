#include "LlParser.h"
#include "debug.h"
#include "ErrorRecorder.h"
#include "Namer.h"

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

    RecursiveDescentParser::RecursiveDescentParser(std::vector<Token *> &_tokens) : LlParser::LlParser(_tokens), tree(nullptr), label_namer(new NonsenceGenerator), temp_namer(new RuthlessCounter) {}

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

    Namer *RecursiveDescentParser::get_label_namer(void) {
        return label_namer;
    }

    std::vector<Quaternary> RecursiveDescentParser::get_irs(void) {
        return irs;
    }

    typedef std::vector<Symbol *> Symbols;

#define PARSE(_type, _name) Symbols symbols; \
    Symbol::Type __type = (_type); \
    std::string __name = (_name)

#define RETURN_PARSE() return new NonterminalSymbol(__type, __name, symbols)

#define GET(_token_type) do {\
        symbols.push_back(new TerminalSymbol(t = get(_token_type))); \
        if (!t) {\
            return nullptr; \
        } \
    } while(0)

#define TAKE(_symbol_func, ...) do {\
        symbols.push_back(s = _symbol_func(__VA_ARGS__));\
        if (!s) {\
            return nullptr;\
        }\
    } while (0)

    std::string RecursiveDescentParser::place_tag(void) {
        std::string name = label_namer->new_name();
        irs.emplace_back(Quaternary::LABEL, "", "", name);
        return name;
    }

    void RecursiveDescentParser::place_tag(const std::string &name) {
        irs.emplace_back(Quaternary::LABEL, "", "", name);
    }

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

    Symbol *RecursiveDescentParser::parse_characters(std::string &var) {
        PARSE(Symbol::CHARACTERS, "<字符>");
        Token *t = peek();
        if (t && t->get_type() == Token::CHARCON) {
            symbols.push_back(new TerminalSymbol(t = pop()));
            var = std::to_string((int)t->get_content().front());
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
            GET(Token::SEMICN);
        } while (peek() && peek()->get_type() == Token::CONSTTK);
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_const_define(void) {
        PARSE(Symbol::CONST_DEFINE, "<常量定义>");
        Token *t = peek();
        Symbol *s;
        FullHouse::CalType ctype;
        std::string value, name;
        bool ret;

        if (t && t->get_type() == Token::INTTK) {
            ctype = FullHouse::INT;
            GET(Token::INTTK);
            GET(Token::IDENFR);
            name = t->get_content();
            GET(Token::ASSIGN);
            TAKE(parse_integer, value);
            ret = full_house_table.declare_full_house(new FullHouse(name, ctype, FullHouse::VARIABLE, FullHouse::CONST, value));
            if (!ret) {
                rerr.push_back(new Error(Error::REDEFINITION, t->get_location()));
            }
            while (peek() && peek()->get_type() == Token::COMMA) {
                GET(Token::COMMA);
                GET(Token::IDENFR);
                name = t->get_content();
                GET(Token::ASSIGN);
                TAKE(parse_integer, value);
                ret = full_house_table.declare_full_house(new FullHouse(name, ctype, FullHouse::VARIABLE, FullHouse::CONST, value));
                if (!ret) {
                    rerr.push_back(new Error(Error::REDEFINITION, t->get_location()));
                }
            }
        } else if (t && t->get_type() == Token::CHARTK) {
            ctype = FullHouse::CHAR;
            GET(Token::CHARTK);
            GET(Token::IDENFR);
            name = t->get_content();
            GET(Token::ASSIGN);
            TAKE(parse_characters, value);
            ret = full_house_table.declare_full_house(new FullHouse(name, ctype, FullHouse::VARIABLE, FullHouse::CONST, value));
            if (!ret) {
                rerr.push_back(new Error(Error::REDEFINITION, t->get_location()));
            }
            while (peek() && peek()->get_type() == Token::COMMA) {
                GET(Token::COMMA);
                GET(Token::IDENFR);
                name = t->get_content();
                GET(Token::ASSIGN);
                TAKE(parse_characters, value);
                ret = full_house_table.declare_full_house(new FullHouse(t->get_content(), ctype, FullHouse::VARIABLE, FullHouse::CONST, value));
                if (!ret) {
                    rerr.push_back(new Error(Error::REDEFINITION, t->get_location()));
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

    Symbol *RecursiveDescentParser::parse_integer(std::string &value) {
        PARSE(Symbol::INTEGER, "<整数>");
        Token *t = peek();
        Symbol *s;
        value.clear();
        if (t && t->get_type() == Token::PLUS) {
            GET(Token::PLUS);
            value += t->get_content();
        } else if (t && t->get_type() == Token::MINU) {
            GET(Token::MINU);
            value += t->get_content();
        }
        TAKE(parse_unsigned_integer);
        value += s->get_token()->get_content();
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
            size_t size = atoi(s->get_token()->get_content().c_str());
            //DP("size = %llu\n", size);
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
            name = full_house_table.get_code_name(name);
            irs.emplace_back(Quaternary::VAR, name, std::to_string(size * 4));
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
                size_t size = atoi(s->get_token()->get_content().c_str());
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
                name = full_house_table.get_code_name(name);
                irs.emplace_back(Quaternary::VAR, name, std::to_string(size * 4));
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
        tmp = new FullHouse(name, ctype, btype, ttype);
        ret = full_house_table.declare_full_house(tmp);
        if (!ret) {
            rerr.push_back(new Error(Error::REDEFINITION, t->get_location()));
        }
        full_house_table.push_layer();
        place_tag(name);
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
        irs.emplace_back(Quaternary::RETURN);

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
        tmp = new FullHouse(name, ctype, btype, ttype);
        ret = full_house_table.declare_full_house(tmp);
        if (!ret) {
            rerr.push_back(new Error(Error::REDEFINITION, t->get_location()));
        }
        full_house_table.push_layer();
        place_tag(name);
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
        irs.emplace_back(Quaternary::RETURN);

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_multiple_statement(const FullHouse::CalType &ctin) {
        PARSE(Symbol::MULTIPLE_STATEMENT, "<复合语句>");
        Token *t = peek();
        Symbol *s;
        irs.emplace_back(Quaternary::BEGIN);
        if (t && t->get_type() == Token::CONSTTK) {
            TAKE(parse_const_declare);
        }
        if (peek(0) && (peek(0)->get_type() == Token::INTTK || peek(0)->get_type() == Token::CHARTK)
            && peek(1) && (peek(1)->get_type() == Token::IDENFR)
            && peek(2) && (peek(2)->get_type() == Token::COMMA || peek(2)->get_type() == Token::SEMICN || peek(2)->get_type() == Token::LBRACK)) {

            TAKE(parse_variable_declare);
        }
        TAKE(parse_block, ctin);
        irs.emplace_back(Quaternary::END);
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
            irs.emplace_back(Quaternary::PARAM, name);
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
                irs.emplace_back(Quaternary::PARAM, name);
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
        full_house_table.push_layer();
        GET(Token::LBRACE);
        irs.emplace_back(Quaternary::ENTRY);
        TAKE(parse_multiple_statement, FullHouse::VOID);
        irs.emplace_back(Quaternary::EXIT);
        GET(Token::RBRACE);
        full_house_table.pop_layer();

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_expression(FullHouse::CalType &ctype, std::string &var) {
        PARSE(Symbol::EXPRESSION, "<表达式>");
        Token *t = peek();
        Symbol *s;

        bool flag = false;
        int op = 1;
        FullHouse::CalType ctmp;
        std::string temp;

        var = temp_namer->new_name();
        if (t && t->get_type() == Token::PLUS) {
            flag = true;
            op = 1;
            GET(Token::PLUS);
        } else if (t && t->get_type() == Token::MINU) {
            flag = true;
            op = -1;
            GET(Token::MINU);
        }
        TAKE(parse_term, ctmp, temp);
        if (!flag) {
            ctype = ctmp;
            irs.emplace_back(Quaternary::COPY, var, temp);
        } else {
            ctype = FullHouse::INT;
            if (op == 1) {
                irs.emplace_back(Quaternary::COPY, var, temp);
            } else {
                irs.emplace_back(Quaternary::NEGATE, var, temp);
            }
        }
        while (peek() && (peek()->get_type() == Token::PLUS || peek()->get_type() == Token::MINU)) {
            if (peek()->get_type() == Token::PLUS) {
                ctype = FullHouse::INT;
                GET(Token::PLUS);
                op = 1;
            } else {
                ctype = FullHouse::INT;
                GET(Token::MINU);
                op = -1;
            }
            TAKE(parse_term, ctmp, temp);
            if (op == 1) {
                irs.emplace_back(Quaternary::PLUS, var, var, temp);
            } else {
                irs.emplace_back(Quaternary::MINUS, var, var, temp);
            }
        }

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_term(FullHouse::CalType &ctype, std::string &var) {
        PARSE(Symbol::TERM, "<项>");
        Token *t = peek();
        Symbol *s;

        FullHouse::CalType ctmp;
        std::string temp;
        bool is_mul = false;

        var = temp_namer->new_name();
        TAKE(parse_factor, ctmp, temp);
        irs.emplace_back(Quaternary::COPY, var, temp);
        ctype = ctmp;
        while (peek() && (peek()->get_type() == Token::MULT || peek()->get_type() == Token::DIV)) {
            if (peek()->get_type() == Token::MULT) {
                GET(Token::MULT);
                is_mul = true;
            } else {
                GET(Token::DIV);
                is_mul = false;
            }
            TAKE(parse_factor, ctmp, temp);
            ctype = FullHouse::INT;
            if (is_mul) {
                irs.emplace_back(Quaternary::TIME, var, var, temp);
            } else {
                irs.emplace_back(Quaternary::DIVIDE, var, var, temp);
            }
        }

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_factor(FullHouse::CalType &ctype, std::string &var) {
        PARSE(Symbol::FACTOR, "<因子>");
        Token *t = peek();
        Symbol *s;

        FullHouse *fh;
        std::string name, temp;
        FullHouse::CalType ctmp;

        if (t && t->get_type() == Token::IDENFR) {
            var = temp_namer->new_name();
            if (peek(1) && peek(1)->get_type() == Token::LPARENT) {
                TAKE(parse_call_with_return, fh);
                ctype = fh->get_calculation();
                irs.emplace_back(Quaternary::GET_RETURN, var);
            } else {
                GET(Token::IDENFR);
                name = t->get_content();
                fh = full_house_table.lookup_full_house(name);
                name = full_house_table.get_code_name(name);
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
                    TAKE(parse_expression, ctmp, temp);
                    if (ctmp != FullHouse::INT) {
                        rerr.push_back(new Error(Error::INDEX_ERROR, s->get_token()->get_location()));
                    }
                    if (peek() && peek()->get_type() != Token::RBRACK) {
                        rerr.push_back(new Error(Error::RBRACK_LOST, s->get_token()->get_location()));
                    } else {
                        GET(Token::RBRACK);
                    }
                    irs.emplace_back(Quaternary::INDEX, var, name, temp);
                } else {
                    irs.emplace_back(Quaternary::COPY, var, name);
                }
                ctype = fh ? fh->get_calculation() : FullHouse::VOID;
            }
        } else if (t && t->get_type() == Token::LPARENT) {
            GET(Token::LPARENT);
            TAKE(parse_expression, ctmp, var);
            if (peek() && peek()->get_type() != Token::RPARENT) {
                rerr.push_back(new Error(Error::RPARENT_LOST, s->get_token()->get_location()));
            } else {
                GET(Token::RPARENT);
            }
            ctype = FullHouse::INT;
        } else if (t && (t->get_type() == Token::INTCON || t->get_type() == Token::PLUS || t->get_type() == Token::MINU)) {
            TAKE(parse_integer, var);
            ctype = FullHouse::INT;
        } else if (t && t->get_type() == Token::CHARCON) {
            TAKE(parse_characters, var);
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

        std::string name, temp, rvar;
        FullHouse *fh;
        FullHouse::CalType ctmp, lhs, rhs;

        if (peek(1) && peek(1)->get_type() == Token::LBRACK) {
            GET(Token::IDENFR);
            name = t->get_content();
            fh = full_house_table.lookup_full_house(name);
            name = full_house_table.get_code_name(name);
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
            TAKE(parse_expression, ctmp, temp);
            if (ctmp != FullHouse::INT) {
                rerr.push_back(new Error(Error::INDEX_ERROR, s->get_token()->get_location()));
            }
            if (peek() && peek()->get_type() != Token::RBRACK) {
                rerr.push_back(new Error(Error::RBRACK_LOST, s->get_token()->get_location()));
            } else {
                GET(Token::RBRACK);
            }
            GET(Token::ASSIGN);
            TAKE(parse_expression, rhs, rvar);
            if (lhs != rhs) {
                DP("???mismatched types");
                rerr.push_back(new Error(Error::UNKNOWN_ERROR, s->get_token()->get_location()));
            }
            irs.emplace_back(Quaternary::ELEMENT, name, temp, rvar);
        } else {
            GET(Token::IDENFR);
            name = t->get_content();
            fh = full_house_table.lookup_full_house(name);
            name = full_house_table.get_code_name(name);
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
            TAKE(parse_expression, rhs, rvar);
            if (lhs != rhs) {
                DP("mismatched types");
                rerr.push_back(new Error(Error::UNKNOWN_ERROR, s->get_token()->get_location()));
            }
            irs.emplace_back(Quaternary::COPY, name, rvar);
        }

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_if_statement(const FullHouse::CalType &ctin) {
        PARSE(Symbol::IF_STATEMENT, "<条件语句>");
        Token *t = peek();
        Symbol *s;
        std::string var, else_label, end_label;

        GET(Token::IFTK);
        GET(Token::LPARENT);
        TAKE(parse_condition, var);
        else_label = label_namer->new_name();
        irs.emplace_back(Quaternary::JUMP_UNLESS, "", var, else_label);
        if (peek() && peek()->get_type() != Token::RPARENT) {
            rerr.push_back(new Error(Error::RPARENT_LOST, s->get_token()->get_location()));
        } else {
            GET(Token::RPARENT);
        }
        TAKE(parse_statement, ctin);
        if (peek() && peek()->get_type() == Token::ELSETK) {
            end_label = label_namer->new_name();
            irs.emplace_back(Quaternary::JUMP, "", "", end_label);
            place_tag(else_label);
            GET(Token::ELSETK);
            TAKE(parse_statement, ctin);
        } else {
            place_tag(else_label);
        }

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_condition(std::string &var) {
        PARSE(Symbol::CONDITION, "<条件>");
        Symbol *s;
        Token *t = peek();

        FullHouse::CalType lhs, rhs;
        std::string lvar, rvar;

        TAKE(parse_expression, lhs, lvar);
        if (peek() && Token::is_relation(peek())) {
            symbols.push_back(new TerminalSymbol(t = pop()));
            if (!t) {
                return nullptr;
            }
            TAKE(parse_expression, rhs, rvar);
            if (lhs != rhs) {
                rerr.push_back(new Error(Error::UNKNOWN_TYPE_CONDITION, s->get_token()->get_location()));
            }
            var = temp_namer->new_name();
            switch (t->get_type()) {
            case Token::LSS:
                irs.emplace_back(Quaternary::LESS, var, lvar, rvar);
                break;
            case Token::LEQ:
                irs.emplace_back(Quaternary::LEQ, var, lvar, rvar);
                break;
            case Token::GRE:
                irs.emplace_back(Quaternary::LESS, var, rvar, lvar);
                break;
            case Token::GEQ:
                irs.emplace_back(Quaternary::LEQ, var, rvar, lvar);
                break;
            case Token::EQL:
                irs.emplace_back(Quaternary::EQU, var, rvar, lvar);
                break;
            case Token::NEQ:
                irs.emplace_back(Quaternary::NEQ, var, rvar, lvar);
                break;
            default:
                return nullptr;
                break;
            }
        } else {
            var = lvar;
        }

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_loop_statement(const FullHouse::CalType &ctin) {
        PARSE(Symbol::LOOP_STATEMENT, "<循环语句>");
        Token *t = peek();
        Symbol *s;

        FullHouse *fh;
        std::string name, begin_label, end_label, lb, lc, temp;
        FullHouse::CalType lhs, rhs;

        if (t && t->get_type() == Token::WHILETK) {
            GET(Token::WHILETK);
            GET(Token::LPARENT);
            begin_label = place_tag();
            end_label = label_namer->new_name();
            TAKE(parse_condition, temp);
            if (peek() && peek()->get_type() != Token::RPARENT) {
                rerr.push_back(new Error(Error::RPARENT_LOST, s->get_token()->get_location()));
            } else {
                GET(Token::RPARENT);
            }
            irs.emplace_back(Quaternary::JUMP_UNLESS, "", temp, end_label);
            TAKE(parse_statement, ctin);
            irs.emplace_back(Quaternary::JUMP, "", "", begin_label);
            place_tag(end_label);
        } else if (t && t->get_type() == Token::DOTK) {
            GET(Token::DOTK);
            begin_label = place_tag();
            TAKE(parse_statement, ctin);
            if (peek() && peek()->get_type() != Token::WHILETK) {
                rerr.push_back(new Error(Error::WHILE_LOST, peek()->get_location()));
            } else {
                GET(Token::WHILETK);
            }
            GET(Token::LPARENT);
            TAKE(parse_condition, temp);
            if (peek() && peek()->get_type() != Token::RPARENT) {
                rerr.push_back(new Error(Error::RPARENT_LOST, s->get_token()->get_location()));
            } else {
                GET(Token::RPARENT);
            }
            irs.emplace_back(Quaternary::JUMP_IF, "", temp, begin_label);
        } else if (t && t->get_type() == Token::FORTK) {
            GET(Token::FORTK);
            GET(Token::LPARENT);
            TAKE(parse_assign_statement);
            GET(Token::SEMICN);
            begin_label = place_tag();
            end_label = label_namer->new_name();
            lb = label_namer->new_name();
            lc = label_namer->new_name();
            TAKE(parse_condition, temp);
            irs.emplace_back(Quaternary::JUMP_UNLESS, "", temp, end_label);
            irs.emplace_back(Quaternary::JUMP, "", "", lc);
            GET(Token::SEMICN);
            place_tag(lb);
            irs.emplace_back(Quaternary::JUMP, "", "", begin_label);
            TAKE(parse_assign_statement);
            GET(Token::RPARENT);
            place_tag(lc);
            TAKE(parse_statement, ctin);
            irs.emplace_back(Quaternary::JUMP, "", "", lb);
            place_tag(end_label);
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
        irs.emplace_back(Quaternary::CALL, name);
        if (fh && params.size() != fh->get_array_size()) {
            rerr.push_back(new Error(Error::UNMATCHED_PARAM_COUNT, t->get_location()));
        } else if (fh && !fh->same_params(params)) {
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
        irs.emplace_back(Quaternary::CALL, name);
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
        std::string var;
        std::vector<std::string> vars;

        args.clear();

        if (t) {
            switch (t->get_type()) {
            case Token::PLUS:
            case Token::MINU:
            case Token::IDENFR:
            case Token::INTCON:
            case Token::CHARCON:
            case Token::LPARENT:
                TAKE(parse_expression, ctype, var);
                vars.push_back(var);
                args.push_back(ctype);
                while (peek() && peek()->get_type() == Token::COMMA) {
                    GET(Token::COMMA);
                    TAKE(parse_expression, ctype, var);
                    vars.push_back(var);
                    args.push_back(ctype);
                }
                for (auto v : vars) {
                    irs.emplace_back(Quaternary::ARGUMENT, v);
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
        name = full_house_table.get_code_name(name);
        if (!fh) {
            DP("UNDEFINED\n");
            rerr.push_back(new Error(Error::UNDEFINED, t->get_location()));
        } else if (fh->get_constant() == FullHouse::CONST) {
            DP("CONST DISCARTED\n");
            rerr.push_back(new Error(Error::CONST_DISCARD, t->get_location()));
        } else if (fh->get_behavior() != FullHouse::VARIABLE) {
            DP("NOT VARIABLE\n");
            rerr.push_back(new Error(Error::UNKNOWN_ERROR, t->get_location()));
        } else {
            DP("here we are\n");
            if (fh->get_calculation() == FullHouse::INT) {
                irs.emplace_back(Quaternary::READI, name);
            } else {
                irs.emplace_back(Quaternary::READC, name);
            }
        }
        while (peek() && peek()->get_type() == Token::COMMA) {
            GET(Token::COMMA);
            GET(Token::IDENFR);
            name = t->get_content();
            fh = full_house_table.lookup_full_house(name);
            name = full_house_table.get_code_name(name);
            if (!fh) {
                rerr.push_back(new Error(Error::UNDEFINED, t->get_location()));
            } else if (fh->get_constant() == FullHouse::CONST) {
                rerr.push_back(new Error(Error::CONST_DISCARD, t->get_location()));
            } else if (fh->get_behavior() != FullHouse::VARIABLE) {
                rerr.push_back(new Error(Error::UNKNOWN_ERROR, t->get_location()));
            } else {
                if (fh->get_calculation() == FullHouse::INT) {
                    irs.emplace_back(Quaternary::READI, name);
                } else {
                    irs.emplace_back(Quaternary::READC, name);
                }
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
        std::string var, str;

        GET(Token::PRINTFTK);
        GET(Token::LPARENT);
        if (peek() && peek()->get_type() == Token::STRCON) {
            TAKE(parse_string);
            str = s->get_token()->get_content();
            if (peek() && peek()->get_type() == Token::COMMA) {
                GET(Token::COMMA);
                TAKE(parse_expression, ctype, var);
                irs.emplace_back(Quaternary::PRINTS, "", str);
                if (ctype == FullHouse::INT) {
                    irs.emplace_back(Quaternary::PRINTI, "", var);
                } else {
                    irs.emplace_back(Quaternary::PRINTC, "", var);
                }
                irs.emplace_back(Quaternary::PRINTS, "", "\\n");
            } else {
                irs.emplace_back(Quaternary::PRINTS, "", str);
                irs.emplace_back(Quaternary::PRINTS, "", "\\n");
            }
        } else {
            TAKE(parse_expression, ctype, var);
            if (ctype == FullHouse::INT) {
                irs.emplace_back(Quaternary::PRINTI, "", var);
                irs.emplace_back(Quaternary::PRINTS, "", "\\n");
            } else {
                irs.emplace_back(Quaternary::PRINTC, "", var);
                irs.emplace_back(Quaternary::PRINTS, "", "\\n");
            }
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

        std::string var;

        GET(Token::RETURNTK);
        if (peek() && peek()->get_type() == Token::LPARENT) {
            GET(Token::LPARENT);
            TAKE(parse_expression, ctype, var);
            irs.emplace_back(Quaternary::SET_RETURN, var);
            if (peek() && peek()->get_type() != Token::RPARENT) {
                rerr.push_back(new Error(Error::RPARENT_LOST, s->get_token()->get_location()));
            } else {
                GET(Token::RPARENT);
            }
        }

        RETURN_PARSE();
    }
}


