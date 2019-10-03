// Ckeshpeare.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <string>
#include "Tokenizer.h"
#include "Token.h"

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

int main() {
    std::cerr << "Hello World!\n";
    freopen("testfile.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
    tokenizeHomework();
}
