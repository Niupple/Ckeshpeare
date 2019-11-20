#pragma once
#include <string>
#include <map>
#include <set>
#include "Namer.h"

namespace dyj {

    class StringLiteralManager {
    public:
        StringLiteralManager();
        StringLiteralManager(Namer *tagger, std::set<std::string> literals);

        void add_literal(Namer *tagger, const std::string &literal);
        template<typename ItType>
        void add_literals(Namer *tagger, ItType begin, ItType end);

        std::string get_tag(const std::string &literal) const;

    private:
        std::map<std::string, std::string> literal_to_tag;
    };
    
    template<typename ItType>
    inline void StringLiteralManager::add_literals(Namer *tagger, ItType begin, ItType end) {
        for (auto p = begin; p != end; ++p) {
            add_literal(tagger, *p);
        }
    }

}
