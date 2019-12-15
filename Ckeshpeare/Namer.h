#pragma once
#include <unordered_set>
#include <string>
#include <random>
#include <vector>

namespace dyj {
    class Namer {
    public:
        virtual std::string new_name(void) = 0;
        virtual size_t max_size(void) = 0;
        virtual size_t size_left(void) = 0;

    protected:
        Namer();

    };

    class RuthlessCounter : public Namer {
    public:
        RuthlessCounter();

        std::string new_name(void) override;
        size_t max_size(void) override;
        size_t size_left(void) override;
    
    private:
        size_t cnt = 0;
    };

    class NonsenceGenerator : public Namer {
    public:
        NonsenceGenerator();

        std::string new_name(void) override;
        size_t max_size(void) override;
        size_t size_left(void) override;

    private:
        std::default_random_engine engine;
        std::unordered_set<std::string> used;
        static std::vector<std::string> lhs, mhs, rhs;
    };

    bool is_global(const std::string &name);
    bool is_local(const std::string &name);
    bool is_user(const std::string &name);
    bool is_temp(const std::string &name);
    bool is_variable(const std::string &name);
    bool is_const(const std::string &name);
    bool is_value(const std::string &name);
}
