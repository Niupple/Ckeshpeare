#pragma once
#include <string>
#include "Token.h"

namespace dyj {
    class MatcherBase {
    public:
        enum Status : int {
            DEFAULT,
            ERROR,
            MATCHED,
            FAILED
        };
        Status get_status(void) const;
        virtual Status feed(char c) = 0;
        virtual Status peek(char c) const = 0;
        virtual Token *get_token(void) = 0;
        virtual void clear(void) = 0;
    protected:
        MatcherBase();
        std::string buffer;
        Status status;
    };

    class IdentifierMatcher : public MatcherBase {
    public:
        IdentifierMatcher();

        Status feed(char c) override;
        Status peek(char c) const override;
        Token *get_token(void) override;
        void clear(void) override;
    private:
        enum State {
            DEFAULT,
            FIRST,
            ACCEPTING,
            FAILED
        };
        State state;
        void fail();
    };

    class KeywordMatcher : public MatcherBase {
    public:
        KeywordMatcher() = delete;
        KeywordMatcher(const std::string &, Token::Type);

        Status feed(char c) override;
        Status peek(char c) const override;
        Token *get_token(void) override;
        void clear(void) override;
    private:
        std::string keyword;
        Token::Type tokentype;
        int state;
    };

    class IntConstantMatcher : public MatcherBase {
    public:
        IntConstantMatcher();

        Status feed(char c) override;
        Status peek(char c) const override;
        Token *get_token(void) override;
        void clear(void) override;
    private:
        enum State {
            DEFAULT,
            ZERO,
            ACCEPTING,
            FAILED
        };
        State state;
        void fail();
    };

    class CharConstantMatcher : public MatcherBase {
    public:
        CharConstantMatcher();

        Status feed(char c) override;
        Status peek(char c) const override;
        Token *get_token(void) override;
        void clear(void) override;
    private:
        enum State {
            DEFAULT,
            LQUOTION,
            RQUOTION,
            ACCEPTING,
            FAILED
        };
        State state;
        void fail();
    };

    class StringConstantMatcher : public MatcherBase {
    public:
        StringConstantMatcher();

        Status feed(char c) override;
        Status peek(char c) const override;
        Token *get_token(void) override;
        void clear(void) override;
    private:
        enum State {
            DEFAULT,
            LQUOTION,
            CHARS,
            ACCEPTING,
            FAILED
        };
        State state;
        void fail();
    };

    bool is_addition(char c);
    bool is_multiplication(char c);
    bool is_char_in_char(char c);
    bool is_char_in_string(char c);
    bool is_alpha(char c);
    bool is_digit_nonzero(char c);
    bool is_digit(char c);
    bool is_space(char c);
}