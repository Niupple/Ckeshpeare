#pragma once
#include <map>
#include <string>
#include <vector>
#include <stack>
#include "FullHouse.h"

namespace dyj {

    typedef std::map<std::string, FullHouse *> Scope;
    class StackFullHouseManager {
    public:
        StackFullHouseManager();

        size_t get_layers(void) const;
        void push_layer(void);
        void pop_layer(void);
        bool declare_full_house(FullHouse *fh);
        FullHouse *lookup_full_house(const std::string &name) const;
        std::string get_code_name(const std::string &name) const;

    private:
        size_t counter;
        std::vector<Scope> stack;
        std::vector<size_t> index;
    };
}
