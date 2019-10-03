#pragma once
#include <string>
#include <vector>
#include <list>
#include "Matcher.h"

namespace dyj {
    using std::vector;
    using std::string;
    using std::list;
    class Tokenizer {
    public:
        Tokenizer();

        void feed(const string &_content);
        Token *get_token(void);

        static Tokenizer cTokenizer;
    private:
        class InitCTokenizer {
        public:
            static Tokenizer get_cTokenizer();
        };
        vector<MatcherBase *> matchers;
        string content;
        size_t cur;
        list<size_t> possible;

        char peek(void);
        char pop(void);
        void push(void);
        Token *retrive_token(void);
        void clear(void);
        void skip_spaces(void);
        bool time_to_get(void);
    };

}