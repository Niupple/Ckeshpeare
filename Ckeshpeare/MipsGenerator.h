#pragma once
#include <vector>
#include <string>
#include <map>
#include <set>
#include <tuple>
#include "Quaternary.h"
#include "Mips.h"
#include "IrManager.h"
#include "IrOptimizer.h"
#include "Namer.h"

namespace dyj {
    typedef std::tuple<Mips::Registers, Mips::Registers, Mips::Registers> reg_triple;

    class MipsGenerator {
    public:
        MipsGenerator() = delete;
        MipsGenerator(const std::vector<Quaternary> &_irs, Namer *_tagger, FlowGraph &fg);
        MipsGenerator(const std::vector<Quaternary> &_irs, Namer *_tagger, std::vector<std::set<std::string> > &&_living_vars = std::vector<std::set<std::string> >(), const std::set<std::pair<std::string, std::string> > &_collisions = std::set<std::pair<std::string, std::string> >());

        void parse(void);
        void dump(std::string &output);

        bool peek_hole(void);

    private:
        const std::vector<Quaternary> &irs;
        std::vector<Mips> mips;
        size_t counter;

        const Quaternary *peek(void);
        const Quaternary *pop(void);
        bool has_next(void);

    private:
        void distribute_global_regs(void);
        void add_edge(const std::string &a, const std::string &b);
        static std::set<Mips::Registers> available_global_regs;
        std::set<std::pair<std::string, std::string> > collisions;
        std::map<std::string, std::vector<std::string> > edges;
        std::map<std::string, int> degs;
        std::map<std::string, Mips::Registers> global_name_to_regs;

    private:
        bool in_function;
        Namer *tagger;
        std::vector<size_t> bytes;
        std::map<std::string, size_t> local_name_to_pos;
        std::map<std::string, std::string> global_name_to_pos;
        std::map<std::string, size_t> global_label_to_size;
        std::map<std::string, std::string> literal_to_label;

        std::vector<std::set<std::string> > living_out, living_in;
        static std::set<Mips::Registers> available_regs;
        std::map<Mips::Registers, std::string> reg_to_var;
        std::map<Mips::Registers, bool> reg_to_dirty;
        std::map<std::string, Mips::Registers> var_to_reg;

        reg_triple def_use(const std::string &def, const std::string &lhs = "", const std::string &rhs = "", bool write_1 = true, bool write_2 = false, bool write_3 = false);
        std::string opt_find_replaced(int cur_block, const std::set<std::string> &on_hold);
        bool still_alive(const std::string &name);
        void dump_live_dirty(void);
        void dump_global(const std::string &name);
        void clear_reginfo(void);
        void register_local_variable(const std::string &var, size_t size = 4);
        void load(Mips::Registers reg, const std::string &name);
        void store(Mips::Registers reg, const std::string &name);

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

    };

    size_t align(size_t x);
}
