// Ckeshpeare.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <string>
#include <fstream>
#include "Tokenizer.h"
#include "Token.h"
#include "LlParser.h"
#include "ErrorRecorder.h"
#include "Quaternary.h"
#include "MipsGenerator.h"
#include "debug.h"

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

void errorHandlingHomework() {
    using dyj::Token;
    using dyj::Tokenizer;
    using dyj::Symbol;
    using dyj::RecursiveDescentParser;
    Tokenizer &cTokenizer = Tokenizer::cTokenizer;
    std::string code, tmp, line;
    std::vector<std::string> lines;
    std::vector<Token *> tokens;
    Token *t;
    char c;

    while ((c = std::cin.get()) != EOF) {
        code += c;
        line += c;
        if (c == '\n') {
            lines.push_back(line);
            line.clear();
        }
    }
    std::cerr << code << std::endl;
    cTokenizer.feed(std::move(code));
    while ((t = cTokenizer.get_token())) {
        tokens.push_back(t);
        std::cerr << t->to_string() << std::endl;
    }

    RecursiveDescentParser parser(tokens);
    parser.parse();
    for (auto e : dyj::rerr) {
        if (e->get_type() != dyj::Error::UNKNOWN_ERROR) {
            std::cout << e->to_string() << std::endl;
        }
    }
    //std::cout << code << std::endl;
    //std::cout << lines[19] << std::endl;
}

void codeGeneratingHomework() {
    using namespace dyj;
    using namespace std;
    ifstream fin("testfile.txt");
    ofstream mips("mips.txt");
    Tokenizer &cTokenizer = Tokenizer::cTokenizer;
    string code, tmp, line, dest_code;
    vector<Token *> tokens;
    vector<string> lines;
    vector<Quaternary> irs;
    Token *t;
    char c;

    while ((c = fin.get()) != EOF) {
        code += c;
        line += c;
        if (c == '\n') {
            lines.push_back(line);
            line.clear();
        }
    }
    std::cerr << code << std::endl;
    cTokenizer.feed(std::move(code));
    while ((t = cTokenizer.get_token())) {
        tokens.push_back(t);
        std::cerr << t->to_string() << std::endl;
    }

    RecursiveDescentParser parser(tokens);
    Symbol *tree = parser.parse();

    for (auto e : dyj::rerr) {
        if (e->get_type() != dyj::Error::UNKNOWN_ERROR) {
            std::cout << e->to_string() << std::endl;
        }
    }
    if (!dyj::rerr.empty()) {
        return;
    }

    if (tree) {
        cout << tree->to_string() << endl;
    }
    irs = parser.get_irs();

    cout << "__________________IRS______________________" << endl;
    for (auto &c : irs) {
        cout << c.to_string() << endl;
    }
    cout << "___________________________________________" << endl;

    MipsGenerator engine(irs, parser.get_label_namer());
    DP("engine built\n");
    engine.parse();
    DP("parse done\n");
    engine.dump(dest_code);
    DP("dump done\n");

    mips << dest_code;
    
    cout << "__________________MIPS_____________________" << endl;
    cout << dest_code;
    cout << "___________________________________________" << endl;
}

int main() {
    //tokenizeHomework();
    //parseHomework();
    //errorHandlingHomework();
    codeGeneratingHomework();
}
