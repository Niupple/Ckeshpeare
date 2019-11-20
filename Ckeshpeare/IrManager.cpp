#include "IrManager.h"

namespace dyj {
    StringLiteralManager::StringLiteralManager() {}

    StringLiteralManager::StringLiteralManager(Namer *tagger, std::set<std::string> literals) {
        add_literals(tagger, literals.begin(), literals.end());
    }

    void StringLiteralManager::add_literal(Namer *tagger, const std::string &literal) {
        if (literal_to_tag.find(literal) == literal_to_tag.end()) {
            literal_to_tag[literal] = tagger->new_name();
        }
    }

    std::string StringLiteralManager::get_tag(const std::string &literal) const {
        return literal_to_tag.find(literal)->second;
    }

}
