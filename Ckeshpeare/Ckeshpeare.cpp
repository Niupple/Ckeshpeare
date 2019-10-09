// Ckeshpeare.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <string>
#include "Tokenizer.h"
#include "Token.h"
#include "LlParser.h"

void tokenizeHomework() {
    using dyj::Token;
    using dyj::Tokenizer;
    Tokenizer &cTokenizer = Tokenizer::cTokenizer;
    std::string code, tmp;
    while (std::getline(std::cin, tmp)) {
        code += tmp;
    }
    std::cerr << code << std::endl;
    Token *t;
    cTokenizer.feed(std::move(code));
    while ((t = cTokenizer.get_token())) {
        std::cout << t->to_string();
    }
}

void parseHomework() {
    using dyj::Token;
    using dyj::Tokenizer;
    using dyj::Symbol;
    using dyj::RecursiveDescentParser;
    Tokenizer &cTokenizer = Tokenizer::cTokenizer;
    std::string code, tmp;
    std::vector<Token *> tokens;
    Token *t;

    while (std::getline(std::cin, tmp)) {
        code += tmp;
    }
    std::cerr << code << std::endl;
    cTokenizer.feed(std::move(code));
    while ((t = cTokenizer.get_token())) {
        tokens.push_back(t);
        std::cerr << t->to_string() << std::endl;
    }

    RecursiveDescentParser parser(tokens);
    Symbol *tree = parser.parse();
    if (tree) {
        std::cout << tree->to_string() << std::endl;
    }
}

int main() {
    std::cerr << "Hello World!\n";
    freopen("testfile.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
    //tokenizeHomework();
    parseHomework();
}
