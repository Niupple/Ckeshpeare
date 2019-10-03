#include "Token.h"

namespace dyj {
    std::string Token::type_to_string(Type _type) {
        switch (_type) {
        case DEFAULT:   return "DEFAULT";
        case ERROR:     return "ERROR";
        case IDENFR:    return "IDENFR";
        case INTCON:    return "INTCON";
        case CHARCON:   return "CHARCON";
        case STRCON:    return "STRCON";
        case CONSTTK:   return "CONSTTK";
        case INTTK:     return "INTTK";
        case CHARTK:    return "CHARTK";
        case VOIDTK:    return "VOIDTK";
        case MAINTK:    return "MAINTK";
        case IFTK:      return "IFTK";
        case ELSETK:    return "ELSETK";
        case DOTK:      return "DOTK";
        case WHILETK:   return "WHILETK";
        case FORTK:     return "FORTK";
        case SCANFTK:   return "SCANFTK";
        case PRINTFTK:  return "PRINTFTK";
        case RETURNTK:  return "RETURNTK";
        case PLUS:      return "PLUS";
        case MINU:      return "MINU";
        case MULT:      return "MULT";
        case DIV:       return "DIV";
        case LSS:       return "LSS";
        case LEQ:       return "LEQ";
        case GRE:       return "GRE";
        case GEQ:       return "GEQ";
        case EQL:       return "EQL";
        case NEQ:       return "NEQ";
        case ASSIGN:    return "ASSIGN";
        case SEMICN:    return "SEMICN";
        case COMMA:     return "COMMA";
        case LPARENT:   return "LPARENT";
        case RPARENT:   return "RPARENT";
        case LBRACK:    return "LBRACK";
        case RBRACK:    return "RBRACK";
        case LBRACE:    return "LBRACE";
        case RBRACE:    return "RBRACE";
        default:        return "WTF???";
        }
    }

    Token::Token() {
        type = Token::DEFAULT;
    }

    Token::Token(Type _type, const std::string &_content) : type(_type), content(_content) {}

    Token::Type Token::get_type(void) const {
        return type;
    }

    std::string Token::get_content(void) const {
        return content;
    }

    std::string Token::to_string() const {
        return Token::type_to_string(type) + " " + content + "\n";
    }

    bool Token::operator==(const Token &_rhs) const    {
        return type == _rhs.type && content == _rhs.content;
    }
}
