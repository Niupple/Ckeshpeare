#pragma once
#include <vector>
#include <string>
#include <map>
#include "Quaternary.h"
#include "Mips.h"
#include "IrManager.h"

namespace dyj {
    class MipsGenerator {
    public:
        MipsGenerator() = delete;
        MipsGenerator(const std::vector<Quaternary> &_irs, Namer *_tagger);

        void parse(void);
        void dump(std::string &output);

    private:
        const std::vector<Quaternary> &irs;
        std::vector<Mips *> mips;
        size_t counter;

        const Quaternary *peek(void);
        const Quaternary *pop(void);
        bool has_next(void);

    private:
        Namer *tagger;
        bool in_function;
        std::vector<size_t> bytes;
        std::map<std::string, size_t> local_name_to_pos;
        std::map<std::string, std::string> global_name_to_pos;
        std::map<std::string, size_t> global_label_to_size;
        std::map<std::string, std::string> literal_to_label;

    private:
        void use(Mips::Registers reg, const std::string &var);
        void use_array(Mips::Registers dst, const std::string &arr, Mips::Registers idx);
        void def(Mips::Registers reg, const std::string &var);
        void def_array(Mips::Registers src, const std::string &arr, Mips::Registers idx);
        void declare_array_in_function(const std::string &arr, size_t size);
        void declare_array_in_global(const std::string &arr, size_t size);
        void newline(void);
        int idx_to_addr(size_t idx);
        std::string get_string_label(const std::string &literal);

        void parse_plus_and_minus();
        void parse_time_and_divide();
        void parse_copy_and_negate();
        void parse_condition();
        void parse_index();
        void parse_element();
        void parse_jumps();
        void parse_label();
        void parse_function_call();
        void parse_param();
        void parse_set_return();
        void parse_get_return();
        void parse_return();
        void parse_input();
        void parse_output();
        void parse_entry();
        void parse_exit();
        void parse_scope();
        void parse_def();

    private:
        static bool is_global(const std::string &name);
        static bool is_const(const std::string &name);
    };

    size_t align(size_t x);
}
