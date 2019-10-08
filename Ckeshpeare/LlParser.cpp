#include "LlParser.h"

namespace dyj{
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

    RecursiveDescentParser::RecursiveDescentParser(std::vector<Token *> &_tokens) : LlParser::LlParser(_tokens), tree(nullptr) {}

    Symbol *RecursiveDescentParser::parse(void) {
        return tree = parse_program();
    }

    Symbol *RecursiveDescentParser::get_tree(void) {
        return tree;
    }

    typedef std::vector<Symbol *> Symbols;

#define PARSE(_type, _name) Symbols symbols; \
    Symbol::Type type = (_type); \
    std::string name = (_name)

#define RETURN_PARSE() return new NonterminalSymbol(type, name, symbols)

#define GET(_token_type) do {\
        symbols.push_back(new TerminalSymbol(t = get(_token_type))); \
        if (!t) {\
            return nullptr; \
        } \
    } while(0)\

#define TAKE(_symbol_func) do {\
        symbols.push_back(s = _symbol_func());\
        if (!s) {\
            return nullptr;\
        }\
    } while (0);\

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
        if (t && t->get_type() == Token::STRCON) {
            symbols.push_back(new TerminalSymbol(pop()));
            RETURN_PARSE();
        } else {
            return nullptr;
        }
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
            && peek(2) && (peek(2)->get_type() == Token::COMMA || peek(2)->get_type() == Token::SEMICN)) {

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
        symbols.push_back(s = parse_main_function());
        if (!s) {
            return nullptr;
        }
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
        if (t && t->get_type() == Token::INTTK) {
            GET(Token::INTTK);
            GET(Token::IDENFR);
            GET(Token::ASSIGN);
            TAKE(parse_integer);
            while (peek() && peek()->get_type() == Token::COMMA) {
                GET(Token::COMMA);
                GET(Token::INTTK);
                GET(Token::IDENFR);
                GET(Token::ASSIGN);
                TAKE(parse_integer);
            }
        } else if (t && t->get_type() == Token::CHARTK) {
            GET(Token::CHARTK);
            GET(Token::IDENFR);
            GET(Token::ASSIGN);
            TAKE(parse_characters);
            while (peek() && peek()->get_type() == Token::COMMA) {
                GET(Token::COMMA);
                GET(Token::CHARTK);
                GET(Token::IDENFR);
                GET(Token::ASSIGN);
                TAKE(parse_characters);
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

    Symbol *RecursiveDescentParser::parse_declare_header(void) {
        PARSE(Symbol::DECLARE_HEADER, "<声明头部>");
        Token *t = peek();
        Symbol *s;
        if (t && t->get_type() == Token::INTTK) {
            GET(Token::INTTK);
        } else if (t && t->get_type() == Token::CHARTK) {
            GET(Token::CHARTK);
        } else {
            return nullptr;
        }
        GET(Token::IDENFR);
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_variable_declare(void) {
        PARSE(Symbol::VARIABLE_DECLARE, "<变量说明>");
        Token *t = peek();
        Symbol *s;
        do {
            TAKE(parse_variable_define);
            GET(Token::SEMICN);
        } while (peek() && (peek()->get_type() == Token::INTTK || peek()->get_type() == Token::CHARTK));
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_variable_define(void) {
        PARSE(Symbol::VARIABLE_DEFINE, "<变量定义>");
        Token *t = peek();
        Symbol *s;
        TAKE(parse_type_id);
        GET(Token::IDENFR);
        if (peek() && peek()->get_type() == Token::LBRACK) {
            GET(Token::LBRACK);
            TAKE(parse_unsigned_integer);
            GET(Token::RBRACK);
        }
        while (peek() && peek()->get_type() == Token::COMMA) {
            GET(Token::COMMA);
            GET(Token::IDENFR);
            if (peek() && peek()->get_type() == Token::LBRACK) {
                GET(Token::LBRACK);
                TAKE(parse_unsigned_integer);
                GET(Token::RBRACK);
            }
        }
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_type_id(void) {
        PARSE(Symbol::TYPE_ID, "");
        Token *t = peek();
        if (t && t->get_type() == Token::INTTK) {
            GET(Token::INTTK);
        } else if (t && t->get_type() == Token::CHARTK) {
            GET(Token::CHARTK);
        } else {
            return nullptr;
        }
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_function_with_return(void) {
        PARSE(Symbol::FUNCTION_WITH_RETURN, "<有返回值函数定义>");
        Token *t = peek();
        Symbol *s;

        TAKE(parse_declare_header);
        GET(Token::LPARENT);
        TAKE(parse_parameter_list);
        GET(Token::RPARENT);
        GET(Token::LBRACE);
        TAKE(parse_multiple_statement);
        GET(Token::RBRACE);

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_function_without_return(void) {
        PARSE(Symbol::FUNCTION_WITHOUT_RETURN, "<无返回值函数定义>");
        Token *t;
        Symbol *s;

        GET(Token::VOIDTK);
        GET(Token::IDENFR);
        GET(Token::LPARENT);
        TAKE(parse_parameter_list);
        GET(Token::RPARENT);
        GET(Token::LBRACE);
        TAKE(parse_multiple_statement);
        GET(Token::RBRACE);

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_multiple_statement(void) {
        PARSE(Symbol::MULTIPLE_STATEMENT, "<复合语句>");
        Token *t = peek();
        Symbol *s;
        if (t && t->get_type() == Token::CONSTTK) {
            TAKE(parse_const_declare);
        }
        if (peek(0) && (peek(0)->get_type() == Token::INTTK || peek(0)->get_type() == Token::CHARTK)
            && peek(1) && (peek(1)->get_type() == Token::IDENFR)
            && peek(2) && (peek(2)->get_type() == Token::COMMA || peek(2)->get_type() == Token::SEMICN)) {

            TAKE(parse_variable_declare);
        }
        TAKE(parse_block);
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_parameter_list(void) {
        PARSE(Symbol::PARAMETER_LIST, "<参数表>");
        Token *t = peek();
        Symbol *s;
        if (t && (t->get_type() == Token::INTTK || t->get_type() == Token::CHARTK)) {
            TAKE(parse_type_id);
            GET(Token::IDENFR);
            while (peek() && peek()->get_type() == Token::COMMA) {
                GET(Token::COMMA);
                TAKE(parse_type_id);
                GET(Token::IDENFR);
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
        GET(Token::RPARENT);
        GET(Token::LBRACE);
        TAKE(parse_multiple_statement);
        GET(Token::RBRACE);

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_expression(void) {
        PARSE(Symbol::EXPRESSION, "<表达式>");
        Token *t = peek();
        Symbol *s;

        if (t && t->get_type() == Token::PLUS) {
            GET(Token::PLUS);
        } else if (t && t->get_type() == Token::MINU) {
            GET(Token::MINU);
        }
        TAKE(parse_term);
        while (peek() && (peek()->get_type() == Token::PLUS || peek()->get_type() == Token::MINU)) {
            if (peek()->get_type() == Token::PLUS) {
                GET(Token::PLUS);
            } else {
                GET(Token::MINU);
            }
            TAKE(parse_term);
        }

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_term(void) {
        PARSE(Symbol::TERM, "<项>");
        Token *t = peek();
        Symbol *s;
        TAKE(parse_factor);
        while (peek() && (peek()->get_type() == Token::MULT || peek()->get_type() == Token::DIV)) {
            if (peek()->get_type() == Token::MULT) {
                GET(Token::MULT);
            } else {
                GET(Token::DIV);
            }
            TAKE(parse_factor);
        }

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_factor(void) {
        PARSE(Symbol::FACTOR, "<因子>");
        Token *t = peek();
        Symbol *s;

        if (t && t->get_type() == Token::IDENFR) {
            if (peek(1) && peek(1)->get_type() == Token::LPARENT) {
                TAKE(parse_call_with_return);
            } else {
                GET(Token::IDENFR);
                if (peek() && peek()->get_type() == Token::LBRACK) {
                    GET(Token::LBRACK);
                    TAKE(parse_expression);
                    GET(Token::RBRACK);
                }
            }
        } else if (t && t->get_type() == Token::LPARENT) {
            GET(Token::LPARENT);
            TAKE(parse_expression);
            GET(Token::RPARENT);
        } else if (t && (t->get_type() == Token::INTCON || t->get_type() == Token::PLUS || t->get_type() == Token::MINU)) {
            TAKE(parse_integer);
        } else if (t && t->get_type() == Token::CHARCON) {
            GET(Token::CHARCON);
        } else {
            return nullptr;
        }
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_statement(void) {
        PARSE(Symbol::STATEMENT, "<语句>");
        Token *t = peek();
        Symbol *s;
        if (t && t->get_type() == Token::IFTK) {
            TAKE(parse_if_statement);
        } else if (t && (t->get_type() == Token::WHILETK || t->get_type() == Token::DOTK || t->get_type() == Token::FORTK)) {
            TAKE(parse_loop_statement);
        } else if (t && t->get_type() == Token::LBRACE) {
            GET(Token::LBRACE);
            TAKE(parse_block);
            GET(Token::RBRACE);
        } else if (t && t->get_type() == Token::IDENFR) {
            if (peek(1) && peek(1)->get_type() == Token::ASSIGN) {
                TAKE(parse_assign_statement);
            } else if (function_table.find(t->get_content()) != function_table.end()) {
                if (function_table.at(t->get_content())) {
                    TAKE(parse_call_with_return);
                } else {
                    TAKE(parse_call_without_return);
                }
            } else {
                return nullptr;
            }
        } else if (t && t->get_type() == Token::SCANFTK) {
            TAKE(parse_scanf);
        } else if (t && t->get_type() == Token::PRINTFTK) {
            TAKE(parse_printf);
        } else if (t && t->get_type() == Token::RETURNTK) {
            TAKE(parse_return);
        }
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_assign_statement(void) {
        PARSE(Symbol::ASSIGN_STATEMENT, "<赋值语句>");
        Token *t = peek();
        Symbol *s;

        if (peek(1) && peek(1)->get_type() == Token::LBRACK) {
            GET(Token::IDENFR);
            GET(Token::LBRACK);
            TAKE(parse_expression);
            GET(Token::RBRACK);
            GET(Token::ASSIGN);
            TAKE(parse_expression);
        } else {
            GET(Token::IDENFR);
            GET(Token::ASSIGN);
            TAKE(parse_expression);
        }

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_if_statement(void) {
        PARSE(Symbol::IF_STATEMENT, "<条件表达式>");
        Token *t = peek();
        Symbol *s;

        GET(Token::IFTK);
        GET(Token::LPARENT);
        TAKE(parse_condition);
        GET(Token::RPARENT);
        TAKE(parse_statement);
        if (peek() && peek()->get_type() == Token::ELSETK) {
            GET(Token::ELSETK);
            TAKE(parse_statement);
        }

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_condition(void) {
        PARSE(Symbol::CONDITION, "<条件>");
        Token *t = peek();
        Symbol *s;

        TAKE(parse_expression);
        if (peek() && Token::is_relation(peek())) {
            symbols.push_back(new TerminalSymbol(pop()));
            TAKE(parse_expression);
        }

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_loop_statement(void) {
        PARSE(Symbol::LOOP_STATEMENT, "<循环语句>");
        Token *t = peek();
        Symbol *s;

        if (t && t->get_type() == Token::WHILETK) {
            GET(Token::WHILETK);
            GET(Token::LPARENT);
            TAKE(parse_condition);
            GET(Token::RPARENT);
            TAKE(parse_statement);
        } else if (t && t->get_type() == Token::DOTK) {
            GET(Token::DOTK);
            TAKE(parse_statement);
            GET(Token::WHILETK);
        } else if (t && t->get_type() == Token::FORTK) {
            GET(Token::FORTK);
            GET(Token::LPARENT);
            GET(Token::IDENFR);
            GET(Token::ASSIGN);
            TAKE(parse_expression);
            GET(Token::SEMICN);
            TAKE(parse_condition);
            GET(Token::SEMICN);
            GET(Token::IDENFR);
            GET(Token::ASSIGN);
            GET(Token::IDENFR);
            if (peek() && peek()->get_type() == Token::PLUS) {
                GET(Token::PLUS);
            } else if (peek() && peek()->get_type() == Token::MINU) {
                GET(Token::MINU);
            } else {
                return nullptr;
            }
            TAKE(parse_step);
            GET(Token::RPARENT);
            TAKE(parse_statement);
        } else {
            return nullptr;
        }

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_step(void) {
        PARSE(Symbol::STEP, "<步长>");
        Token *t;
        GET(Token::INTCON);
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_call_with_return(void) {
        PARSE(Symbol::CALL_WITH_RETURN, "<有返回值函数调用语句>");
        Token *t = peek();
        Symbol *s;

        GET(Token::IDENFR);
        GET(Token::LPARENT);
        TAKE(parse_argument_list);
        GET(Token::RPARENT);

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_call_without_return(void) {
        PARSE(Symbol::CALL_WITHOUT_RETURN, "<无返回值函数调用语句>");
        Token *t = peek();
        Symbol *s;

        GET(Token::IDENFR);
        GET(Token::LPARENT);
        TAKE(parse_argument_list);
        GET(Token::RPARENT);

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_argument_list(void) {
        PARSE(Symbol::ARGUMENT_LIST, "<值参数表>");
        Token *t = peek();
        Symbol *s;
        if (t) {
            switch (t->get_type()) {
            case Token::PLUS:
            case Token::MINU:
            case Token::IDENFR:
            case Token::INTCON:
            case Token::CHARCON:
            case Token::LPARENT:
                TAKE(parse_expression);
                while (peek() && peek()->get_type() == Token::COMMA) {
                    GET(Token::COMMA);
                    TAKE(parse_expression);
                }
            }
        }
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_block(void) {
        PARSE(Symbol::BLOCK, "<语句列>");
        Token *t = peek();
        Symbol *s;

        while (true) {
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
                    TAKE(parse_statement);
                    continue;
                }
            }
            break;
        }
        
        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_scanf(void) {
        PARSE(Symbol::SCANF, "<读语句>");
        Token *t = peek();
        Symbol *s;
        GET(Token::SCANFTK);
        GET(Token::LPARENT);
        GET(Token::IDENFR);
        while (peek() && peek()->get_type() == Token::COMMA) {
            GET(Token::COMMA);
            GET(Token::IDENFR);
        }
        GET(Token::RPARENT);

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_printf(void) {
        PARSE(Symbol::SCANF, "<写语句>");
        Token *t = peek();
        Symbol *s;
        GET(Token::PRINTFTK);
        GET(Token::LPARENT);
        if (peek() && peek()->get_type() == Token::STRCON) {
            GET(Token::STRCON);
            if (peek() && peek()->get_type() == Token::COMMA) {
                GET(Token::COMMA);
                TAKE(parse_expression);
            }
        } else {
            TAKE(parse_expression);
        }

        RETURN_PARSE();
    }

    Symbol *RecursiveDescentParser::parse_return(void) {
        PARSE(Symbol::RETURN, "<返回语句>");
        Token *t = peek();
        Symbol *s;

        GET(Token::RETURNTK);
        if (peek() && peek()->get_type() == Token::LPARENT) {
            GET(Token::LPARENT);
            TAKE(parse_expression);
            GET(Token::RPARENT);
        }

        RETURN_PARSE();
    }
}


