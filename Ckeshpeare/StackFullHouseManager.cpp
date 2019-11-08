#include "StackFullHouseManager.h"
#include "debug.h"

namespace dyj {
    StackFullHouseManager::StackFullHouseManager() : counter(0) {
        stack.emplace_back();
        index.push_back(counter++);
    }

    size_t StackFullHouseManager::get_layers(void) const {
        return counter;
    }

    void StackFullHouseManager::push_layer(void) {
        index.push_back(counter++);
        stack.emplace_back();
    }

    void StackFullHouseManager::pop_layer(void) {
        if (!index.empty()) {
            index.pop_back();
            stack.pop_back();
        }
    }

    bool StackFullHouseManager::declare_full_house(FullHouse *fh) {
        if (index.empty()) {
            return false;
        } else {
            Scope &current = stack.back();
            if (current.count(fh->get_name()) == 0) {
                DP("!!!declare %s\n", fh->get_name().c_str());
                current[fh->get_name()] = fh;
                return true;
            } else {
                return false;
            }
        }
        return true;
    }

    FullHouse *StackFullHouseManager::lookup_full_house(const std::string &name) const {
        for (auto p = stack.rbegin(); p != stack.rend(); ++p) {
            auto q = p->find(name);
            if (q != p->end()) {
                return q->second;
            }
        }
        return nullptr;
    }

}
