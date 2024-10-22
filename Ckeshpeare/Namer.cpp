#include "Namer.h"

#include <chrono>

namespace dyj {
    Namer::Namer() {}

    RuthlessCounter::RuthlessCounter() : cnt(0) {}

    std::string RuthlessCounter::new_name(void) {
        return "__t" + std::to_string(cnt++);
    }

    size_t RuthlessCounter::max_size(void) {
        return SIZE_MAX;
    }

    size_t RuthlessCounter::size_left(void) {
        return SIZE_MAX - cnt;
    }

    NonsenceGenerator::NonsenceGenerator() {
        engine.seed(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    }

    std::string NonsenceGenerator::new_name(void) {
        std::uniform_int_distribution<size_t> ra(0, lhs.size() - 1), rb(0, mhs.size() - 1), rc(0, rhs.size() - 1);
        std::string tmp;

        tmp = "_" + lhs[ra(engine)] + "_" + mhs[rb(engine)] + "_" + rhs[rc(engine)];
        while (used.count(tmp) != 0) {
            tmp += "_and_" + lhs[ra(engine)] + "_" + mhs[rb(engine)] + "_" + rhs[rc(engine)];
        }
        used.insert(tmp);
        return tmp;
    }

    size_t NonsenceGenerator::max_size(void) {
        return SIZE_MAX;
    }

    size_t NonsenceGenerator::size_left(void) {
        return SIZE_MAX - used.size();
    }

    std::vector<std::string> NonsenceGenerator::lhs{
        "red",
        "blue",
        "green",
        "yellow",
        "colored",
        "colorful",
        "pale",
        "shinny",
        "pink",
        "stylish",
        "old_fashioned",
        "cutting_edge",
        "smoothy",
        "kinky"
    };

    std::vector<std::string> NonsenceGenerator::mhs{
        "irratating",
        "annoying",
        "freaking",
        "awsome",
        "amazing",
        "interesting",
        "boring",
        "socialist",
        "imperialist",
        "royal",
        "civil",
        "weird",
        "dummy",
        "amusing",
        "pretty",
        "good_looking",
        "ugly",
        "stink",
        "beautiful",
        "wonderful",
        "extrordinary",
        "unique",
        "special",
        "wooden",
        "metal",
        "plastic"
    };

    std::vector<std::string> NonsenceGenerator::rhs{
        "computer",
        "machine",
        "person",
        "chinese",
        "grapes",
        "apple",
        "chicken",
        "coke",
        "pants",
        "trousers",
        "shirts",
        "switch",
        "ps4",
        "cpu",
        "memory",
        "ssd",
        "keyboard",
        "headset",
        "earphone",
        "wire",
        "noodles",
        "calendar",
        "day",
        "week",
        "year",
        "tour",
        "trip",
        "life",
        "thing"
    };


    bool is_global(const std::string &name) {
        size_t pos = name.find_last_of('@');
        //DP("%s\n", name.c_str());
        if (pos == std::string::npos) {
            return false;
        } else {
            return name.substr(pos) == "@0";
        }
    }

    bool is_local(const std::string &name) {
        return is_user(name) & !is_global(name);
    }

    bool is_user(const std::string &name) {
        return name.find_last_of('@') != std::string::npos;
    }

    bool is_const(const std::string &name) {
        return name.size() > 0 && (name.front() == '-' || name.front() == '+' || isdigit(name.front()));
    }

    bool is_value(const std::string &name) {
        return is_const(name) || is_variable(name);
    }

    bool is_temp(const std::string &name) {
        return name.size() > 0 && !is_user(name) && name.substr(0, 3) == "__t";
    }

    bool is_variable(const std::string &name) {
        return is_temp(name) || is_user(name);
    }

}
