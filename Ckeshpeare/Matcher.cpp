#include "Matcher.h"

namespace dyj {
    MatcherBase::Status MatcherBase::get_status(void) const {
        return status;
    }

    MatcherBase::MatcherBase() : status(DEFAULT) {}
    
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
            if (is_alpha(c)) {
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
            return new Token(Token::IDENFR, buffer);
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
                if (state == keyword.size()) {
                    status = MATCHED;
                }
            } else {
                buffer.clear();
                status = FAILED;
                state = -1;
            }
        }
        return status;
    }

    MatcherBase::Status KeywordMatcher::peek(char c) const {
        if (state >= 0) {
            if (state < (int)keyword.size() && c == keyword[state]) {
                if (state == keyword.size()) {
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
            return new Token(tokentype, buffer);
        }
    }

    void KeywordMatcher::clear(void) {
        buffer.clear();
        state = 0;
        status = DEFAULT;
    }

    IntConstantMatcher::IntConstantMatcher() : MatcherBase(), state(DEFAULT) {}

    MatcherBase::Status IntConstantMatcher::feed(char c) {
        switch (state) {
        case DEFAULT:
            if (is_digit_nonzero(c)) {
                state = ACCEPTING;
                status = MATCHED;
                buffer.push_back(c);
            } else if (c == '0') {
                state = ZERO;
                status = MATCHED;
                buffer.push_back(c);
            } else {
                fail();
            }
            break;
        case ZERO:
            fail();
            break;
        case ACCEPTING:
            if (is_digit(c)) {
                buffer.push_back(c);
            } else {
                fail();
            }
            break;
        default:
            break;
        }
        return status;
    }

    MatcherBase::Status IntConstantMatcher::peek(char c) const {
        switch (state) {
        case DEFAULT:
            if (is_digit_nonzero(c)) {
                return MATCHED;
            } else if (c == '0') {
                return MATCHED;
            } else {
                return Status::FAILED;
            }
            break;
        case ZERO:
            return Status::FAILED;
            break;
        case ACCEPTING:
            if (is_digit(c)) {
                return MATCHED;
            } else {
                return Status::FAILED;
            }
            break;
        default:
            return Status::FAILED;
        }
    }

    Token *IntConstantMatcher::get_token(void) {
        if (status != MATCHED) {
            return nullptr;
        } else {
            return new Token(Token::INTCON, buffer);
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

    CharConstantMatcher::CharConstantMatcher() : MatcherBase(), state(DEFAULT) {}

    MatcherBase::Status CharConstantMatcher::feed(char c) {
        switch (state) {
        case DEFAULT:
            if (c == '\'') {
                buffer.push_back(c);
                state = LQUOTION;
            } else {
                fail();
            }
            break;
        case LQUOTION:
            if (is_char_in_char(c)) {
                buffer.push_back(c);
                state = RQUOTION;
            } else {
                fail();
            }
            break;
        case RQUOTION:
            if (c == '\'') {
                buffer.push_back(c);
                state = ACCEPTING;
                status = MATCHED;
            } else {
                fail();
            }
            break;
        case ACCEPTING:
            fail();
            break;
        default:
            break;
        }
        return status;
    }

    MatcherBase::Status CharConstantMatcher::peek(char c) const {
        switch (state) {
        case DEFAULT:
            if (c == '\'') {
                return Status::DEFAULT;
            } else {
                return Status::FAILED;
            }
            break;
        case LQUOTION:
            if (is_char_in_char(c)) {
                return Status::DEFAULT;
            } else {
                return Status::FAILED;
            }
            break;
        case RQUOTION:
            if (c == '\'') {
                return MATCHED;
            } else {
                return Status::FAILED;
            }
            break;
        case ACCEPTING:
            return Status::FAILED;
            break;
        default:
            return Status::FAILED;
            break;
        }
    }

    Token *CharConstantMatcher::get_token(void) {
        if (status != MATCHED) {
            return nullptr;
        } else {
            return new Token(Token::CHARCON, buffer.substr(1, buffer.size() - 2));
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

    StringConstantMatcher::StringConstantMatcher() : MatcherBase(), state(DEFAULT) {}

    MatcherBase::Status StringConstantMatcher::feed(char c) {
        switch (state) {
        case DEFAULT:
            if (c == '"') {
                buffer.push_back(c);
                state = LQUOTION;
            } else {
                fail();
            }
            break;
        case LQUOTION:
            if (is_char_in_string(c)) {
                buffer.push_back(c);
                state = CHARS;
            } else {
                fail();
            }
            break;
        case CHARS:
            if (is_char_in_string(c)) {
                buffer.push_back(c);
            } else if (c == '"') {
                buffer.push_back(c);
                state = ACCEPTING;
                status = MATCHED;
            } else {
                fail();
            }
            break;
        case ACCEPTING:
            fail();
            break;
        default:
            break;
        }
        return status;
    }

    MatcherBase::Status StringConstantMatcher::peek(char c) const {
        switch (state) {
        case DEFAULT:
            if (c == '"') {
                return Status::DEFAULT;
            } else {
                return Status::FAILED;
            }
            break;
        case LQUOTION:
            if (is_char_in_string(c)) {
                return Status::DEFAULT;
            } else {
                return Status::FAILED;
            }
            break;
        case CHARS:
            if (is_char_in_string(c)) {
                return Status::DEFAULT;
            } else if (c == '"') {
                return Status::MATCHED;
            } else {
                return Status::FAILED;
            }
            break;
        case ACCEPTING:
            return Status::FAILED;
            break;
        default:
            return Status::FAILED;
            break;
        }
    }

    Token *StringConstantMatcher::get_token(void) {
        if (status != MATCHED) {
            return nullptr;
        } else {
            return new Token(Token::STRCON, buffer.substr(1, buffer.size() - 2));
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

}

