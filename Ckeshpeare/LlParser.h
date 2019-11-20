#pragma once
#include <string>
#include <vector>
#include <map>
#include "Token.h"
#include "Symbol.h"
#include "StackFullHouseManager.h"
#include "Quaternary.h"
#include "Namer.h"

namespace dyj {
    class LlParser {
    public:
        LlParser(void) = delete;

    protected:
        LlParser(std::vector<Token *> &_tokens);

        Token *peek(size_t _cnt = 0);
        Token *pop(void);
        Token *get(Token::Type _type);

        bool ended(void);
        
    private:
        const std::vector<Token *> &tokens;
        size_t cur;
    };

    class RecursiveDescentParser : public LlParser {
    public:
        RecursiveDescentParser(void) = delete;
        RecursiveDescentParser(std::vector<Token *> &_tokens);

        Symbol *parse(void);
        Symbol *get_tree(void);

        Namer *get_label_namer(void);

        std::vector<Quaternary> get_irs(void);

    private:
        Symbol *tree;
        StackFullHouseManager full_house_table;
        std::vector<Quaternary> irs;
        Namer *label_namer, *temp_namer;

    private:
        std::string place_tag(void);
        void place_tag(const std::string &name);

    private:
        Symbol *parse_operator_add(void);
        Symbol *parse_operator_multiply(void);
        Symbol *parse_operator_relation(void);
        Symbol *parse_characters(std::string &var);
        Symbol *parse_string(void);
        Symbol *parse_program(void);
        Symbol *parse_const_declare(void);
        Symbol *parse_const_define(void);
        Symbol *parse_unsigned_integer(void);
        Symbol *parse_integer(std::string &);
        Symbol *parse_declare_header(FullHouse::CalType &, std::string &);
        Symbol *parse_variable_declare(void);
        Symbol *parse_variable_define(void);
        Symbol *parse_type_id(FullHouse::CalType &ctype);
        Symbol *parse_function_with_return(void);
        Symbol *parse_function_without_return(void);
        Symbol *parse_multiple_statement(const FullHouse::CalType &ctin);
        Symbol *parse_parameter_list(FullHouse *fh);
        Symbol *parse_main_function(void);
        Symbol *parse_expression(FullHouse::CalType &ctype, std::string &var);
        Symbol *parse_term(FullHouse::CalType &ctype, std::string &var);
        Symbol *parse_factor(FullHouse::CalType &ctype, std::string &var);
        Symbol *parse_statement(const FullHouse::CalType &ctin);
        Symbol *parse_assign_statement(void);
        Symbol *parse_if_statement(const FullHouse::CalType &ctin);
        Symbol *parse_condition(std::string &var);
        Symbol *parse_loop_statement(const FullHouse::CalType &ctin);
        Symbol *parse_step(void);
        Symbol *parse_call_with_return(FullHouse *&ret);
        Symbol *parse_call_without_return(FullHouse *&ret);
        Symbol *parse_argument_list(std::vector<FullHouse::CalType> &args);
        Symbol *parse_block(const FullHouse::CalType &ctin);
        Symbol *parse_scanf(void);
        Symbol *parse_printf(void);
        Symbol *parse_return(FullHouse::CalType &ctype);
    };
}
