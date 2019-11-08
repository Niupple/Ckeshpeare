#include "Matcher.h"
#include "ErrorRecorder.h"
#include "debug.h"

namespace dyj {
    MatcherBase::Status MatcherBase::get_status(void) const {
        return status;
    }

    void MatcherBase::forward(char c) {
        if (c == '\n') {
            counter = Location(counter.first + 1, 1);
        } else {
            ++counter.second;
        }
    }

    MatcherBase::MatcherBase() : status(DEFAULT), counter(1, 1) {}

    IdentifierMatcher::IdentifierMatcher() : MatcherBase(), state(IdentifierMatcher::DEFAULT) {}

    MatcherBase::Status IdentifierMatcher::feed(char c) {
        switch (state) {
        case DEFAULT:
            if (is_alpha(c)) {
                buffer.push_back(c);
                state = FIRST;
                status = MATCHED;
            } else {
                fail();
            }
            break;
        case FIRST:
            if (is_alpha(c) || is_digit(c)) {
                buffer.push_back(c);
                state = ACCEPTING;
            } else {
                fail();
            }
            break;
        case ACCEPTING:
            if (is_alpha(c) || is_digit(c)) {
                buffer.push_back(c);
            } else {
                fail();
            }
            break;
        default:
            break;
        }
        //forward(c);
        return status;
    }

    MatcherBase::Status IdentifierMatcher::peek(char c) const {
        switch (state) {
        case DEFAULT:
            if (is_alpha(c)) {
                return MATCHED;
            } else {
                return Status::FAILED;
            }
            break;
        case FIRST:
            if (is_alpha(c) || is_digit(c)) {
                return MATCHED;
            } else {
                return Status::FAILED;
            }
            break;
        case ACCEPTING:
            if (is_alpha(c) || is_digit(c)) {
                return MATCHED;
            } else {
                return Status::FAILED;
            }
            break;
        default:
            return Status::FAILED;
            break;
        }
    }

    Token *IdentifierMatcher::get_token(void) {
        if (status != MATCHED) {
            return nullptr;
        } else {
            return new Token(Token::IDENFR, buffer, counter);
        }
    }

    void IdentifierMatcher::clear(void) {
        buffer.clear();
        state = DEFAULT;
        status = Status::DEFAULT;
    }

    void IdentifierMatcher::fail() {
        buffer.clear();
        state = FAILED;
        status = Status::FAILED;
    }

    bool is_addition(char c) {
        return c == '+' || c == '-';
    }

    bool is_multiplication(char c) {
        return c == '*' || c == '/';
    }

    bool is_char_in_char(char c) {
        return is_addition(c) || is_multiplication(c) || is_digit(c) || is_alpha(c);
    }

    bool is_char_in_string(char c) {
        return (c == 32) || (c == 33) || (c >= 35 && c <= 126);
    }

    bool is_alpha(char c) {
        return isalpha(c) || c == '_';
    }

    bool is_digit_nonzero(char c) {
        return isdigit(c) && c != '0';
    }

    bool is_digit(char c) {
        return isdigit(c);
    }

    bool is_space(char c) {
        return isspace(c);
    }

    KeywordMatcher::KeywordMatcher(const std::string &_keyword, Token::Type _type) : keyword(_keyword), tokentype(_type), state(0) {}

    MatcherBase::Status KeywordMatcher::feed(char c) {
        if (state >= 0) {
            if (state < (int)keyword.size() && c == keyword[state]) {
                ++state;
                buffer.push_back(c);
                if (state == (int)keyword.size()) {
                    status = MATCHED;
                }
            } else {
                buffer.clear();
                status = FAILED;
                state = -1;
            }
        }
        //forward(c);
        return status;
    }

    MatcherBase::Status KeywordMatcher::peek(char c) const {
        if (state >= 0) {
            if (state < (int)keyword.size() && c == keyword[state]) {
                if (state == (int)keyword.size()) {
                    return MATCHED;
                } else {
                    return Status::DEFAULT;
                }
            } else {
                return FAILED;
            }
        }
        return FAILED;
    }

    Token *KeywordMatcher::get_token(void) {
        if (status != MATCHED) {
            return nullptr;
        } else {
            return new Token(tokentype, buffer, counter);
        }
    }

    void KeywordMatcher::clear(void) {
        buffer.clear();
        state = 0;
        status = DEFAULT;
    }

    IntConstantMatcher::IntConstantMatcher() : MatcherBase(), state(DEFAULT) {}

    MatcherBase::Status IntConstantMatcher::feed(char c) {
        if (status != Status::FAILED) {
            auto p = next_state(c);
            state = p.first;
            status = p.second;
            buffer.push_back(c);
        }
        //forward(c);
        return status;
    }

    MatcherBase::Status IntConstantMatcher::peek(char c) const {
        return next_state(c).second;
    }

    Token *IntConstantMatcher::get_token(void) {
        if (status != MATCHED) {
            return nullptr;
        } else if (state == ERROR) {
            rerr.push_back(new Error(Error::LEXER_ERROR, counter));
            return new Token(Token::INTCON, buffer, counter, true);
        } else {
            return new Token(Token::INTCON, buffer, counter);
        }
    }

    void IntConstantMatcher::clear(void) {
        buffer.clear();
        state = DEFAULT;
        status = Status::DEFAULT;
    }

    void IntConstantMatcher::fail() {
        buffer.clear();
        state = FAILED;
        status = Status::FAILED;
    }

    std::pair<IntConstantMatcher::State, MatcherBase::Status> IntConstantMatcher::next_state(char c) const {
        switch (state) {
        case DEFAULT:
            if (is_digit_nonzero(c)) {
                return std::make_pair(ACCEPTING, MATCHED);
            } else if (c == '0') {
                return std::make_pair(ZERO, MATCHED);
            } else {
                return std::make_pair(FAILED, Status::FAILED);
            }
        case ZERO:
            if (is_alpha(c) || is_digit(c)) {
                return std::make_pair(ERROR, MATCHED);
            } else {
                return std::make_pair(FAILED, Status::FAILED);
            }
        case ERROR:
            if (is_alpha(c) || is_digit(c)) {
                return std::make_pair(ERROR, MATCHED);
            } else {
                return std::make_pair(FAILED, Status::FAILED);
            }
        case ACCEPTING:
            if (is_digit(c)) {
                return std::make_pair(ACCEPTING, MATCHED);
            } else if (is_alpha(c)) {
                return std::make_pair(ERROR, MATCHED);
            } else {
                return std::make_pair(FAILED, Status::FAILED);
            }
        default:
            return std::make_pair(FAILED, Status::FAILED);
        }
    }

    CharConstantMatcher::CharConstantMatcher() : MatcherBase(), state(DEFAULT) {}

    MatcherBase::Status CharConstantMatcher::feed(char c) {
        if (state != FAILED) {
            auto p = next_state(c);
            state = p.first;
            status = p.second;
            buffer.push_back(c);
        }
        //forward(c);
        return status;
    }

    MatcherBase::Status CharConstantMatcher::peek(char c) const {
        return next_state(c).second;
    }

    Token *CharConstantMatcher::get_token(void) {
        if (status != MATCHED) {
            return nullptr;
        } else if (state != ACCEPTING) {
            rerr.push_back(new Error(Error::LEXER_ERROR, counter));
            return new Token(Token::CHARCON, buffer, counter, true);
        } else {
            return new Token(Token::CHARCON, buffer.substr(1, buffer.size() - 2), counter);
        }
    }

    void CharConstantMatcher::clear(void) {
        buffer.clear();
        state = DEFAULT;
        status = MatcherBase::DEFAULT;
    }

    void CharConstantMatcher::fail() {
        buffer.clear();
        state = FAILED;
        status = Status::FAILED;
    }

    std::pair<CharConstantMatcher::State, MatcherBase::Status> CharConstantMatcher::next_state(char c) const {
        switch (state) {
        case DEFAULT:
            if (c == '\'') {
                return std::make_pair(LQUOTION, Status::DEFAULT);
            } else {
                return std::make_pair(FAILED, Status::FAILED);
            }
        case LQUOTION:
            if (is_char_in_char(c)) {
                return std::make_pair(RQUOTION, Status::DEFAULT);
            } else if (c == '\n') {
                return std::make_pair(FAILED, Status::FAILED);
            } else {
                DP("%c is not a char in char\n", c);
                return std::make_pair(ERROR, Status::MATCHED);
            }
        case RQUOTION:
            if (c == '\'') {
                return std::make_pair(ACCEPTING, MATCHED);
            } else {
                return std::make_pair(FAILED, Status::FAILED);
            }
        case ACCEPTING:
            return std::make_pair(FAILED, Status::FAILED);
        case ERROR:
            if (c == '\'') {
                return std::make_pair(ERQUOTION, MATCHED);
            } else if (c == '\n') {
                return std::make_pair(FAILED, Status::FAILED);
            } else {
                return std::make_pair(ERROR, Status::MATCHED);
            }
        case ERQUOTION:
            return std::make_pair(FAILED, Status::FAILED);
        default:
            return std::make_pair(FAILED, Status::FAILED);
        }
    }

    StringConstantMatcher::StringConstantMatcher() : MatcherBase(), state(DEFAULT) {}

    MatcherBase::Status StringConstantMatcher::feed(char c) {
        if (state != FAILED) {
            auto p = next_state(c);
            state = p.first;
            status = p.second;
            buffer.push_back(c);
        }
        //forward(c);
        return status;
    }

    MatcherBase::Status StringConstantMatcher::peek(char c) const {
        return next_state(c).second;
    }

    Token *StringConstantMatcher::get_token(void) {
        if (status != MATCHED) {
            return nullptr;
        } else if (state != ACCEPTING) {
            rerr.push_back(new Error(Error::LEXER_ERROR, counter));
            return new Token(Token::STRCON, buffer, counter, true);
        } else {
            return new Token(Token::STRCON, buffer.substr(1, buffer.size() - 2), counter);
        }
    }

    void StringConstantMatcher::clear(void) {
        buffer.clear();
        state = DEFAULT;
        status = MatcherBase::DEFAULT;
    }

    void StringConstantMatcher::fail() {
        buffer.clear();
        state = FAILED;
        status = MatcherBase::FAILED;
    }

    std::pair<StringConstantMatcher::State, MatcherBase::Status> StringConstantMatcher::next_state(char c) const {
        switch (state) {
        case DEFAULT:
            if (c == '"') {
                return std::make_pair(CHARS, Status::MATCHED);
            } else {
                return std::make_pair(FAILED, Status::FAILED);
            }
        case CHARS:
            if (is_char_in_string(c)) {
                return std::make_pair(CHARS, Status::DEFAULT);
            } else if (c == '"') {
                return std::make_pair(ACCEPTING, MATCHED);
            } else if (c == '\n') {
                return std::make_pair(FAILED, Status::FAILED);
            } else {
                return std::make_pair(ERROR, Status::MATCHED);
            }
        case ERROR:
            if (c == '"') {
                return std::make_pair(ERQUOTION, MATCHED);
            } else if (c == '\n') {
                return std::make_pair(FAILED, Status::FAILED);
            } else {
                return std::make_pair(ERROR, MATCHED);
            }
        case ACCEPTING:
            return std::make_pair(FAILED, Status::FAILED);
        default:
            return std::make_pair(FAILED, Status::FAILED);
        }
    }

}

