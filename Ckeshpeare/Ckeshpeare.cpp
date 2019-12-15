// Ckeshpeare.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <string>
#include <fstream>
#include <cassert>
#include "Tokenizer.h"
#include "Token.h"
#include "LlParser.h"
#include "ErrorRecorder.h"
#include "Quaternary.h"
#include "MipsGenerator.h"
#include "IrOptimizer.h"
#include "BladeRunner.h"
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
    ofstream IRs("irs.txt");
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
    // assert(0);
    cTokenizer.feed(std::move(code));
    while ((t = cTokenizer.get_token())) {
        tokens.push_back(t);
        std::cerr << t->to_string() << std::endl;
    }

    RecursiveDescentParser parser(tokens);
    Symbol *tree = parser.parse();

    for (auto e : dyj::rerr) {
        std::cout << e->to_string() << std::endl;
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
        IRs << c.to_string() << endl;
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

// Optimizer switches
#define IR_PEEKHOLE
//#define BLADE_RUNNER
#define FUNCTION_INLINE

void codeOptimizingHomework() {
    using namespace dyj;
    using namespace std;
    ifstream fin("testfile.txt");
    ofstream IRs("irs.txt");
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
    // assert(0);
    cTokenizer.feed(std::move(code));
    while ((t = cTokenizer.get_token())) {
        tokens.push_back(t);
        std::cerr << t->to_string() << std::endl;
    }

    RecursiveDescentParser parser(tokens);
    Symbol *tree = parser.parse();

    for (auto e : dyj::rerr) {
        std::cout << e->to_string() << std::endl;
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
        //IRs << c.get_block_id() << " " << c.to_string() << endl;
    }
    cout << "___________________________________________" << endl;

#ifdef FUNCTION_INLINE
    Inliner inliner(parser.get_label_namer(), parser.get_var_namer());
    
    inliner.load_and_parse(irs);
    irs = inliner.dump();
#endif

#ifdef IR_PEEKHOLE
    while (ir_peek_hole(irs)) {
        DP("ONE MORE TIME\n");
    }
#endif

    FlowGraph fg;
    fg.load_and_cut(irs);
    fg.flow_living_vars();
    fg.collision_analysis();

    irs = fg.dump();

#ifdef BLADE_RUNNER
    BladeRunner br(irs);
    br.check_and_run();

    irs = br.dump();
#endif

    //cout << "__________________IRS______________________" << endl;
    for (auto &c : irs) {
        //    cout << c.to_string() << endl;
        IRs << c.get_block_id() << " " << c.to_string() << endl;
    }
    //cout << "___________________________________________" << endl;

    MipsGenerator engine(irs, parser.get_label_namer(), fg);
    DP("engine built\n");
    engine.parse();
    DP("parse done\n");

    /*
    while (engine.peek_hole()) {
        DP("ONE MORE TIME\n");
    }*/

    engine.dump(dest_code);
    DP("dump done\n");

    mips << dest_code;

    cout << "__________________MIPS_____________________" << endl;
    cout << dest_code;
    cout << "___________________________________________" << endl;
}

void fake() {
    std::ofstream fout("mips.txt");
    std::string mips = ".data\noutput: .asciiz \"5 != 120\\nx = 5\\ny = 10\\nSWAP x = 10\\nSWAP y = 5\\ncomplete number: 6\\n  1\\n  2\\n  3\\n \\ncomplete number: 28\\n  1\\n  2\\n  4\\n  7\\n  14\\n \\n---------------------------------------------------------------\\n'water flower'number is:\\n  153\\n \\n---------------------------------------------------------------\\n 2\\n 3\\n 5\\n 7\\n 11\\n 13\\n 17\\n 19\\n 23\\n 29\\n \\n 31\\n 37\\n 41\\n 43\\n 47\\n 53\\n 59\\n 61\\n 67\\n 71\\n \\n 73\\n 79\\n 83\\n 89\\n 97\\n 101\\n 103\\n 107\\n 109\\n 113\\n \\n 127\\nThe total is 31\\n\\n\"\n\n.text\n\n\nli $t0, 13000\n\nloop1:\naddu $t0, $t0, $0\naddu $t0, $t0, $0\naddiu $t0, $t0, -1\nbgez $t0, loop1\n\nli $t0, 11950\nloop2:\naddiu $t0, $t0, -1\nbgez $t0, loop2\n\nli $t0, 390\nloop3:\nsw $t0, 0($sp)\naddiu $t0, $t0, -1\nbgez $t0, loop3\n\nli $v0 41\n\nli $t0, 160\nloop4:\nsw $t0, 0($sp)\nj nothing\nnothing:\naddiu $t0, $t0, -1\nsyscall\nbgez $t0, loop4\n\nli $v0, 4\nla $a0, output\nsyscall\n";

    fout << mips << std::flush;
}

int main() {
    //tokenizeHomework();
    //parseHomework();
    //errorHandlingHomework();
    codeOptimizingHomework();
    //fake();
}
