#include "MipsGenerator.h"
#include "Mars.h"
#include "debug.h"
#include <cassert>
#include <cctype>
#include <climits>
#include <stack>

namespace dyj {
    using std::string;
    typedef Jump::Type J;
    typedef Immediate::Type I;
    typedef Register::Type R;

    std::set<Mips::Registers> MipsGenerator::available_regs{
        Mips::T0,
        Mips::T1,
        Mips::T2,
        Mips::T3,
        Mips::T4,
        Mips::T5,
        Mips::T6,
        Mips::T7,
        /*
        Mips::S0,
        Mips::S1,
        Mips::S2,
        Mips::S3,
        Mips::S4,
        Mips::S5,
        Mips::S6,
        Mips::S7,
        */
    };

    std::set<Mips::Registers> MipsGenerator::available_global_regs{
        Mips::S0,
        Mips::S1,
        Mips::S2,
        Mips::S3,
        Mips::S4,
        Mips::S5,
        Mips::S6,
        Mips::S7,
        Mips::T8,
        Mips::T9,
    };

    MipsGenerator::MipsGenerator(const std::vector<Quaternary> &_irs, Namer *_tagger, FlowGraph &fg) :
        collisions(fg.get_collisions()),
        irs(_irs),
        counter(0),
        in_function(false),
        tagger(_tagger),
        living_out(fg.get_living_out_list()),
        living_in(fg.get_living_in_list())
    {}

    MipsGenerator::MipsGenerator(
        const std::vector<Quaternary> &_irs, 
        Namer *_tagger, 
        std::vector<std::set<std::string> > &&_living_vars, 
        const std::set<std::pair<std::string, std::string> > &_collisions
    ) : 
        collisions(_collisions), 
        irs(_irs), 
        counter(0U), 
        in_function(false), 
        tagger(_tagger), 
        living_out(_living_vars) 
    {
        DP("inside constructor\n");
    }

    void MipsGenerator::parse(void) {
        typedef Quaternary::Type IR;
        const Quaternary *ir;
        distribute_global_regs();
        while (has_next()) {
            ir = peek();
            // DP("next is %d\n", ir->get_type());
            switch (ir->get_type()) {
            case IR::PLUS:
            case IR::MINUS:
                parse_plus_and_minus();
                break;
            case IR::TIME:
            case IR::DIVIDE:
                parse_time_and_divide();
                break;
            case IR::COPY:
            case IR::NEGATE:
                parse_copy_and_negate();
                break;
            case IR::LESS:
            case IR::LEQ:
            case IR::EQU:
            case IR::NEQ:
                parse_condition();
                break;
            case IR::INDEX:
                parse_index();
                break;
            case IR::ELEMENT:
                parse_element();
                break;
            case IR::JUMP:
            case IR::JUMP_IF:
            case IR::JUMP_UNLESS:
                parse_jumps();
                break;
            case IR::LABEL:
                parse_label();
                break;
            case IR::CALL:
            case IR::ARGUMENT:
                parse_function_call();
                break;
            case IR::PARAM:
                parse_param();
                break;
            case IR::SET_RETURN:
                parse_set_return();
                break;
            case IR::GET_RETURN:
                parse_get_return();
                break;
            case IR::RETURN:
                parse_return();
                break;
            case IR::READI:
            case IR::READC:
                parse_input();
                break;
            case IR::PRINTI:
            case IR::PRINTC:
            case IR::PRINTS:
            case IR::PRINTF:
                parse_output();
                break;
            case IR::ENTRY:
                parse_entry();
                break;
            case IR::EXIT:
                parse_exit();
                break;
            case IR::BEGIN:
            case IR::END:
                parse_scope();
                break;
            case IR::VAR:
                parse_def();
                break;
            }
        }
    }

    void MipsGenerator::dump(std::string &output) {
        output.clear();

        // .data
        output += ".data\n";
        // global variables
        for (auto p : global_name_to_pos) {
            std::string name, label;
            size_t size;
            name = p.first;
            label = p.second;
            size = global_label_to_size.at(label);
            output += label + ": .space " + std::to_string(size) + "\n";
        }
        // .asciiz
        for (auto p : literal_to_label) {
            std::string lit, lab;
            lit = p.first;
            lab = p.second;
            output += lab + ": .asciiz " + "\"" + lit + "\"" + "\n";
        }
        //
        output += ".text\n";
        output += "jal __main__\n";
        output += "j __main_end__\n";
        for (auto p : mips) {
            output += p.to_string() + "\n";
        }
    }

    bool MipsGenerator::peek_hole(void) {
        std::vector<Mips> old = std::move(mips);
        std::vector<bool> usd;
        mips.clear();
        size_t n = old.size();
        bool still_good = false;
        usd.resize(n);
        for (size_t i = 0; i < n; ++i) {
            Mips &x = old[i];
            if (usd[i]) {
                continue;
            }
            if (i + 1 < n) {
                Mips &y = old[i + 1];
                if (x.get_type() == Mips::LW && y.get_type() == Mips::SW && x.get_rd() == y.get_rt() && x.get_rs() == y.get_rs() && x.get_label() == y.get_label()) {
                    still_good = true;
                    usd[i + 1] = true;
                } else if (x.get_type() == Mips::SW && y.get_type() == Mips::LW && x.get_rt() == y.get_rd() && x.get_rs() == y.get_rs() && x.get_label() == y.get_label()) {
                    still_good = true;
                    usd[i + 1] = true;
                } else if (x.get_type() == Mips::SW && y.get_type() == Mips::LW && x.get_rt() != y.get_rd() && x.get_rs() == y.get_rs() && x.get_label() == y.get_label()) {
                    y = Mips::addu(y.get_rd(), x.get_rt(), Mips::ZERO);
                }
                bool flag = true;

                /*if (x.get_type() == Mips::SW) {
                    for (size_t j = i + 1; j < n; ++j) {
                        Mips &y = old[j];
                        if (usd[j]) {
                            continue;
                        } else if (y.is_jump() || y.get_rd() == x.get_rs() || y.get_type() == Mips::LABEL) {
                            flag = false;
                            break;
                        } else if (y.get_type() == Mips::SW && y.get_rs() == x.get_rs() && y.get_label() == x.get_label() && y.get_rt() == x.get_rt()) {
                            usd[j] = true;
                            still_good = true;
                        }
                    }
                } else if (x.get_type() == Mips::LW) {
                    for (size_t j = i + 1; j < n; ++j) {
                        Mips &y = old[j];
                        if (usd[j]) {
                            continue;
                        } else if (y.is_jump() || y.get_rd() == x.get_rs() || (y.get_type() != Mips::LW && y.get_rd() == x.get_rd()) || y.get_type() == Mips::LABEL) {
                            flag = false;
                            break;
                        } else if (y.get_type() == Mips::SW && (y.get_rs() == x.get_rs() && y.get_label() == x.get_label()) || y.get_rs() != x.get_rs()) {
                            break;
                        } else if (y.get_type() == Mips::LW && y.get_rs() == x.get_rs() && y.get_label() == x.get_label() && x.get_rd() == y.get_rd()) {
                            usd[j] = true;
                            still_good = true;
                        }
                    }
                }*/
            }
            mips.push_back(x);
        }
        return still_good;
    }

    const Quaternary *MipsGenerator::peek(void) {
        if (counter < irs.size()) {
            return &irs[counter];
        } else {
            return nullptr;
        }
    }

    const Quaternary *MipsGenerator::pop(void) {
        if (counter == irs.size()-1 || (counter + 1 < irs.size() && irs[counter].get_block_id() != irs[counter + 1].get_block_id())) {
            dump_live_dirty();
            clear_reginfo();
        }
        if (counter < irs.size()) {
            return &irs[counter++];
        } else {
            return nullptr;
        }
    }

    bool MipsGenerator::has_next(void) {
        return counter < irs.size();
    }

    static bool try_distribute(
        const std::map<std::string, std::vector<std::string> > &edges,
        std::map<std::string, int> &degs,
        std::set<std::string> &usd,
        std::stack<std::string> &stk,
        int K
    ) {
        bool flag = false;
        for (auto p : edges) {
            auto &x = p.first;
            auto &lst = p.second;
            if (usd.count(x) > 0) {
                continue;
            }
            if (degs[x] < K) {
                DP("%s will have\n", x.c_str());
                flag = true;
                stk.push(x);
                degs[x] = 0;
                for (auto &to : lst) {
                    if (usd.count(to) == 0) {
                        --degs[to];
                    }
                }
                usd.insert(x);
            }
        }
        return flag;
    }

    static bool try_dispose(
        const std::map<std::string, std::vector<std::string> > &edges,
        std::map<std::string, int> &degs,
        std::set<std::string> &usd,
        std::stack<std::string> &stk,
        int K
    ) {
        bool flag = false;
        int max_deg = -1;
        std::string to_abandon;
        for (auto p : edges) {
            auto &x = p.first;
            auto &lst = p.second;
            if (usd.count(x) > 0) {
                continue;
            } else {
                flag = true;
                if (degs[x] > max_deg) {
                    max_deg = degs[x];
                    to_abandon = x;
                }
            }
        }
        auto &x = to_abandon;
        if (x == "") {
            return false;
        }
        auto &lst = edges.at(x);
        DP("giving up %s, deg = %d\n", x.c_str(), degs.at(x));
        degs[x] = 0;
        for (auto &to : lst) {
            if (usd.count(to) == 0) {
                --degs[to];
            }
        }
        usd.insert(x);
        return flag;
    }

    template<typename T>
    static std::set<T> subtract(const std::set<T> &a, const std::set<T> &b) {
        std::set<T> ret = a;
        for (auto p : b) {
            ret.erase(p);
        }
        return std::move(ret);
    }

    void MipsGenerator::distribute_global_regs(void) {
        std::stack<std::string> stk;
        std::set<std::string> usd;
        int K = available_global_regs.size();
        for (auto p : collisions) {
            DP("collision(%s, %s)\n", p.first.c_str(), p.second.c_str());
            add_edge(p.first, p.second);
            add_edge(p.second, p.first);
        }

        bool flag = true;
        while (flag) {
            while (try_distribute(edges, degs, usd, stk, K));
            flag = try_dispose(edges, degs, usd, stk, K);
        }

        while (!stk.empty()) {
            std::string var = stk.top();
            stk.pop();
            std::set<Mips::Registers> regs_no;
            for (auto to : edges.at(var)) {
                if (global_name_to_regs.count(to) > 0) {
                    regs_no.insert(global_name_to_regs.at(to));
                }
            }
            auto s = subtract(available_global_regs, regs_no);
            assert(s.size() > 0);
            global_name_to_regs[var] = *s.begin();
        }
#ifdef _DEBUG
        for (auto p : global_name_to_regs) {
            DP("(%s)->$%d\n", p.first.c_str(), p.second);
        }
#endif
    }

    void MipsGenerator::add_edge(const std::string &a, const std::string &b) {
        DP("add_edge(%s, %s)\n", a.c_str(), b.c_str());
        if (a != b) {
            edges[a].push_back(b);
            ++degs[a];
        } else {
            edges[a];
            degs[a];
        }
    }

    static inline void record_first_appear(std::map<std::string, size_t> &record, const std::string &name, size_t cur) {
        if (record.count(name) == 0) {
            record[name] = cur;
        }
    }

    std::string MipsGenerator::opt_find_replaced(int cur_block, const std::set<std::string> &on_hold) {
        string replaced;
        int n_pos = 0;
        std::map<std::string, size_t> next_appear;
        for (size_t i = counter; i < irs.size() && irs[i].get_block_id() == cur_block; ++i) {
            auto &ir = irs[i];
            record_first_appear(next_appear, ir.get_dest(), i);
            record_first_appear(next_appear, ir.get_lhs(), i);
            record_first_appear(next_appear, ir.get_rhs(), i);
        }
        for (auto &name : living_out[cur_block]) {
            record_first_appear(next_appear, name, INT_MAX);
        }
        for (auto &name_reg : var_to_reg) {
            record_first_appear(next_appear, name_reg.first, UINT_MAX);
        }
        for (auto &p : var_to_reg) {
            auto &name = p.first;
            if (on_hold.count(name) == 0 && next_appear.at(name) > n_pos) {
                replaced = name;
                n_pos = next_appear.at(name);
            }
        }
        assert(n_pos != 0);
        return replaced;
    }

    bool MipsGenerator::still_alive(const std::string &name) {
        for (size_t i = counter; i < irs.size() && irs[i].get_block_id() == irs[counter].get_block_id(); ++i) {
            if (irs[i].get_uses().count(name) > 0) {
                return true;
            } else if (irs[i].get_defs().count(name) > 0) {
                return false;
            }
        }
        return living_out[irs[counter].get_block_id()].count(name) > 0;
    }

    reg_triple MipsGenerator::def_use(const std::string &def, const std::string &lhs, const std::string &rhs, bool write_1, bool write_2, bool write_3) {
        Mips::Registers rd = Mips::ZERO, rs = Mips::ZERO, rt = Mips::ZERO;
        std::set<std::pair<std::string, bool> > on_use;
        std::set<std::string> on_hold;
        std::map<std::string, Mips::Registers> result;
        using std::make_pair;
        using std::get;
        int cur_block = peek()->get_block_id();

        if (is_value(def)) {
            on_use.insert(make_pair(def, write_1));
            on_hold.insert(def);
        }
        if (is_value(lhs)) {
            on_use.insert(make_pair(lhs, write_2));
            on_hold.insert(lhs);
        }
        if (is_value(rhs)) {
            on_use.insert(make_pair(rhs, write_3));
            on_hold.insert(rhs);
        }
        for (auto &p : on_use) {
            auto &name = p.first;
            bool write = p.second;
            register_local_variable(name);
            if (global_name_to_regs.count(name) > 0) { // has global register
                result[name] = global_name_to_regs.at(name);
            } else if (var_to_reg.count(name) > 0) {   // already has a local register
                result[name] = var_to_reg.at(name);
                reg_to_dirty[var_to_reg.at(name)] = reg_to_dirty[var_to_reg.at(name)] || write;
            } else if (reg_to_var.size() < available_regs.size()) { // doesn't have register, but registers not full
                bool found = false;
                for (auto reg : available_regs) {
                    if (reg_to_var.count(reg) == 0) {
                        found = true;
                        reg_to_var[reg] = name;
                        reg_to_dirty[reg] = write;
                        var_to_reg[name] = reg;
                        result[name] = reg;
                        if (!write) {
                            load(reg, name);
                        }
                        break;
                    }
                }
                assert(found);
            } else {    // doesn't have register, and registers are all full
                std::string replaced = opt_find_replaced(cur_block, on_hold);
                Mips::Registers reg = var_to_reg.at(replaced);
                DP("%d : %s -> %s\n", (int)reg, replaced.c_str(), name.c_str());
                if (reg_to_dirty[reg] && living_out[cur_block].count(replaced) > 0) {
                    store(reg, replaced);
                }
                reg_to_dirty[reg] = write;
                reg_to_var[reg] = name;
                var_to_reg[name] = reg;
                var_to_reg.erase(replaced);
                if (!write) {
                    load(reg, name);
                }
                result[name] = reg;
            }
        }
        if (is_value(def)) {
            rd = result.at(def);
        }
        if (is_value(lhs)) {
            rs = result.at(lhs);
        }
        if (is_value(rhs)) {
            rt = result.at(rhs);
        }
        return { rd, rs, rt };
    }

    void MipsGenerator::dump_live_dirty() {
        int cur_block = peek()->get_block_id();
        for (auto &p : reg_to_dirty) {
            auto &name = reg_to_var.at(p.first);
            auto &dirty = p.second;
            if (dirty && still_alive(name)) {
                store(p.first, name);
            }
        }
        reg_to_dirty.clear();
    }

    void MipsGenerator::clear_reginfo(void) {
        reg_to_dirty.clear();
        reg_to_var.clear();
        var_to_reg.clear();
    }

    void MipsGenerator::register_local_variable(const std::string &var, size_t size) {
        if (local_name_to_pos.find(var) == local_name_to_pos.end()) {
            local_name_to_pos[var] = bytes.size();
            bytes.push_back(4);
        }
    }

    void MipsGenerator::load(Mips::Registers reg, const std::string &name) {
        DP("load %s to %d\n", name.c_str(), reg);
        use(reg, name);
    }

    void MipsGenerator::store(Mips::Registers reg, const std::string &name) {
        DP("store %s from %d\n", name.c_str(), reg);
        def(reg, name);
    }

    void MipsGenerator::use(Mips::Registers reg, const std::string &var) {
        if (is_const(var)) {
            mips.push_back(Mips::li(reg, var));
        } else if (!is_global(var)) {
            size_t idx;
            int addr;
            if (local_name_to_pos.find(var) != local_name_to_pos.end()) {
                idx = local_name_to_pos.at(var);
                addr = idx_to_addr(idx);
                mips.push_back(Mips::lw(reg, Mips::FP, std::to_string(addr)));
            } else {
                idx = local_name_to_pos[var] = bytes.size();
                bytes.push_back(4);
                addr = idx_to_addr(idx);
                mips.push_back(Mips::lw(reg, Mips::FP, std::to_string(addr)));
            }
        } else {
            std::string label;
            if (global_name_to_pos.find(var) != global_name_to_pos.end()) {
                label = global_name_to_pos.at(var);
                mips.push_back(Mips::lw(reg, Mips::ZERO, label));
            } else {
                label = tagger->new_name();
                global_name_to_pos[var] = label;
                global_label_to_size[label] = 4;
                mips.push_back(Mips::lw(reg, Mips::ZERO, label));
            }
        }
    }

    void MipsGenerator::use_array(Mips::Registers dst, const std::string &arr, Mips::Registers aid) {
        if (!is_global(arr)) {
            size_t idx;
            int addr;
            if (local_name_to_pos.find(arr) != local_name_to_pos.end()) {
                idx = local_name_to_pos.at(arr);
                addr = idx_to_addr(idx);
                mips.push_back(Mips::sll(Mips::AT, aid, "2"));
                mips.push_back(Mips::addu(Mips::GP, Mips::FP, Mips::AT));
                mips.push_back(Mips::lw(dst, Mips::GP, std::to_string(addr)));
            } else {
                DP("LOCAL ARRAY USED BEFORE DECLARE\n");
            }
        } else {
            std::string label;
            if (global_name_to_pos.find(arr) != global_name_to_pos.end()) {
                label = global_name_to_pos.at(arr);
                mips.push_back(Mips::sll(Mips::AT, aid, "2"));
                mips.push_back(Mips::lw(dst, Mips::AT, label));
            } else {
                DP("GLOBAL ARRAY USED BEFORE DECLARE\n");
            }
        }
    }

    void MipsGenerator::def(Mips::Registers reg, const std::string &var) {
        if (!is_global(var)) {
            size_t idx;
            int addr;
            if (local_name_to_pos.find(var) != local_name_to_pos.end()) {
                idx = local_name_to_pos.at(var);
                addr = idx_to_addr(idx);
                mips.push_back(Mips::sw(reg, Mips::FP, std::to_string(addr)));
            } else {
                idx = local_name_to_pos[var] = bytes.size();
                bytes.push_back(4);
                addr = idx_to_addr(idx);
                mips.push_back(Mips::sw(reg, Mips::FP, std::to_string(addr)));
            }
        } else {
            std::string label;
            if (global_name_to_pos.find(var) != global_name_to_pos.end()) {
                label = global_name_to_pos.at(var);
                mips.push_back(Mips::sw(reg, Mips::ZERO, label));
            } else {
                label = tagger->new_name();
                global_name_to_pos[var] = label;
                global_label_to_size[label] = 4;
                mips.push_back(Mips::sw(reg, Mips::ZERO, label));
            }
        }
    }

    void MipsGenerator::def_array(Mips::Registers src, const std::string &arr, Mips::Registers aid) {
        if (!is_global(arr)) {
            size_t idx;
            int addr;
            if (local_name_to_pos.find(arr) != local_name_to_pos.end()) {
                idx = local_name_to_pos.at(arr);
                addr = idx_to_addr(idx);
                mips.push_back(Mips::sll(Mips::AT, aid, "2"));
                mips.push_back(Mips::addu(Mips::GP, Mips::FP, Mips::AT));
                mips.push_back(Mips::sw(src, Mips::GP, std::to_string(addr)));
            } else {
                DP("LOCAL ARRAY USED BEFORE DECLARE\n");
            }
        } else {
            std::string label;
            if (global_name_to_pos.find(arr) != global_name_to_pos.end()) {
                label = global_name_to_pos.at(arr);
                mips.push_back(Mips::sll(Mips::AT, aid, "2"));
                mips.push_back(Mips::sw(src, Mips::AT, label));
            } else {
                DP("GLOBAL ARRAY USED BEFORE DECLARE\n");
            }
        }
    }

    void MipsGenerator::declare_array_in_function(const std::string &arr, size_t size) {
        size_t idx;
        idx = local_name_to_pos[arr] = bytes.size();
        bytes.push_back(size);
    }

    void MipsGenerator::declare_array_in_global(const std::string &arr, size_t size) {
        std::string label;
        label = tagger->new_name();
        global_name_to_pos[arr] = label;
        global_label_to_size[label] = size;
    }

    void MipsGenerator::newline(void) {
        mips.push_back(Mips::ori(Mips::A0, Mips::ZERO, "10"));
        mips.push_back(Mips::ori(Mips::V0, Mips::ZERO, SYS_PRINT_CHAR));
        mips.push_back(Mips::syscall());
    }

    int MipsGenerator::idx_to_addr(size_t idx) {
        size_t ret = 0;
        for (size_t i = 0; i < idx; ++i) {
            ret += bytes[i];
        }
        return -((int)ret);
    }

    std::string MipsGenerator::get_string_label(const std::string &literal) {
        if (literal_to_label.find(literal) == literal_to_label.end()) {
            literal_to_label[literal] = tagger->new_name();
        }
        return literal_to_label.at(literal);
    }

#define GET(t, id) (std::get<(id)>(t))

    void MipsGenerator::parse_plus_and_minus() {
        const Quaternary *ir = peek();
        string d, s1, s2;
        Mips::Registers rd, rs, rt;
        reg_triple t;

        d = ir->get_dest();
        s1 = ir->get_lhs();
        s2 = ir->get_rhs();

        if (ir->get_type() == Quaternary::PLUS) {
            if (is_const(s1)) {
                swap(s1, s2);
                if (is_const(s1)) {
                    s1 = std::to_string(atoi(s1.c_str()) + atoi(s2.c_str()));
                    t = def_use(d);
                    rd = GET(t, 0);
                    mips.push_back(Mips::li(rd, s1));
                    return;
                }
            }
            if (is_const(s2)) {
                //use(Mips::T1, s1);
                t = def_use(d, s1);
                rd = GET(t, 0);
                rs = GET(t, 1);
                mips.push_back(Mips::addiu(rd, rs, s2));
            } else {
                t = def_use(d, s1, s2);
                rd = GET(t, 0);
                rs = GET(t, 1);
                rt = GET(t, 2);
                mips.push_back(Mips::addu(rd, rs, rt));
            }
        } else {
            if (is_const(s1) && is_const(s2)) {
                s1 = std::to_string(atoi(s1.c_str()) - atoi(s2.c_str()));
                t = def_use(d);
                rd = GET(t, 0);
                mips.push_back(Mips::li(rd, s1));
            } else if (is_const(s2)) {
                t = def_use(d, s1);
                rd = GET(t, 0);
                rs = GET(t, 1);
                mips.push_back(Mips::subiu(rd, rs, s2));
            } else {
                t = def_use(d, s1, s2);
                rd = GET(t, 0);
                rs = GET(t, 1);
                rt = GET(t, 2);
                mips.push_back(Mips::subu(rd, rs, rt));
            }
        }
        pop();
    }

    void MipsGenerator::parse_time_and_divide() {
        const Quaternary *ir = peek();
        string d, s1, s2;
        reg_triple t;
        Mips::Registers rd, rs, rt;
        d = ir->get_dest();
        s1 = ir->get_lhs();
        s2 = ir->get_rhs();

        t = def_use(d, s1, s2);

        rd = GET(t, 0);
        rs = GET(t, 1);
        rt = GET(t, 2);

        if (ir->get_type() == Quaternary::TIME) {
            mips.push_back(Mips::mul(rd, rs, rt));
        } else {
            mips.push_back(Mips::div(rd, rs, rt));
        }
        pop();
    }

    void MipsGenerator::parse_copy_and_negate() {
        const Quaternary *ir = peek();
        string d, s1, s2;
        reg_triple t;
        Mips::Registers rd, rs, rt;
        d = ir->get_dest();
        s1 = ir->get_lhs();

        t = def_use(d, s1);

        rd = GET(t, 0);
        rs = GET(t, 1);

        if (ir->get_type() == Quaternary::NEGATE) {
            mips.push_back(Mips::subu(rd, Mips::ZERO, rs));
        } else {
            mips.push_back(Mips::addu(rd, Mips::ZERO, rs));
        }
        pop();
    }

    void MipsGenerator::parse_condition() {
        const Quaternary *ir = peek();
        string d, s1, s2;
        reg_triple t;
        Mips::Registers rd, rs, rt;

        d = ir->get_dest();
        s1 = ir->get_lhs();
        s2 = ir->get_rhs();

        t = def_use(d, s1, s2);

        rd = GET(t, 0);
        rs = GET(t, 1);
        rt = GET(t, 2);

        switch (ir->get_type()) {
        case Quaternary::LESS:
            mips.push_back(Mips::slt(rd, rs, rt));
            break;
        case Quaternary::LEQ:
            mips.push_back(Mips::sle(rd, rs, rt));
            break;
        case Quaternary::EQU:
            mips.push_back(Mips::seq(rd, rs, rt));
            break;
        case Quaternary::NEQ:
            mips.push_back(Mips::sne(rd, rs, rt));
            break;
        default:
            assert(0);
        }
        pop();
    }

    void MipsGenerator::parse_index() { // d = s1[s2]
        const Quaternary *ir = peek();
        string d, s1, s2;
        reg_triple t;
        Mips::Registers rd, rs, rt;

        d = ir->get_dest();
        s1 = ir->get_lhs();
        s2 = ir->get_rhs();

        t = def_use(d, "", s2);

        rd = GET(t, 0);
        rt = GET(t, 2);

        use_array(rd, s1, rt);
        pop();
    }

    void MipsGenerator::parse_element() { // d[s1] = s2
        const Quaternary *ir = peek();
        string d, s1, s2;
        reg_triple t;
        Mips::Registers rd, rs, rt;
        d = ir->get_dest();
        s1 = ir->get_lhs();
        s2 = ir->get_rhs();

        t = def_use("", s1, s2);

        rs = GET(t, 1);
        rt = GET(t, 2);

        def_array(rt, d, rs);
        pop();
    }

    void MipsGenerator::parse_jumps() {
        const Quaternary *ir = peek();
        string d, label;
        reg_triple t;
        Mips::Registers rd, rs, rt;

        label = ir->get_rhs();
        if (ir->get_type() == Quaternary::JUMP) {
            dump_live_dirty();
            mips.push_back(Mips::j(label));
        } else if (ir->get_type() == Quaternary::JUMP_IF) {
            t = def_use("", ir->get_lhs());
            rs = GET(t, 1);
            dump_live_dirty();
            mips.push_back(Mips::bne(rs, Mips::ZERO, label));
        } else {
            t = def_use("", ir->get_lhs());
            rs = GET(t, 1);
            dump_live_dirty();
            mips.push_back(Mips::beq(rs, Mips::ZERO, label));
        }
        pop();
    }

    void MipsGenerator::parse_label() {
        const Quaternary *ir = peek();
        string label = ir->get_rhs();

        mips.push_back(Mips::label(label));
        pop();
    }

    void MipsGenerator::parse_function_call() {
        std::vector<std::string> args;
        const Quaternary *q = peek();
        int n_args = 0, frame_size = 0;
        reg_triple t;
        Mips::Registers rd, rs, rt;

        while (q->get_type() == Quaternary::ARGUMENT) {
            q = pop();
            args.push_back(q->get_lhs());
            register_local_variable(q->get_lhs());
            q = peek();
        }
        n_args = args.size();
        frame_size = idx_to_addr(bytes.size());
        mips.push_back(Mips::sw(Mips::SP, Mips::FP, std::to_string(frame_size)));
        mips.push_back(Mips::addiu(Mips::SP, Mips::FP, std::to_string(frame_size - 4)));
        for (int i = 0; i < n_args; ++i) {
            t = def_use("", args[i]);
            rs = GET(t, 1);
            mips.push_back(Mips::sw(rs, Mips::SP, std::to_string(-i * 4)));
            if (i < 4) {
                mips.push_back(Mips::addu(Mips::args(i), rs, Mips::ZERO));
            }
        }
        dump_live_dirty();
        clear_reginfo();
        mips.push_back(Mips::sw(Mips::RA, Mips::SP, std::to_string(-n_args * 4)));
        mips.push_back(Mips::addiu(Mips::FP, Mips::SP, std::to_string(-(n_args + 1) * 4)));
        q = pop();
        mips.push_back(Mips::jal(q->get_dest()));
        mips.push_back(Mips::addiu(Mips::FP, Mips::SP, std::to_string(4 - frame_size)));
        mips.push_back(Mips::lw(Mips::RA, Mips::SP, std::to_string(-n_args * 4)));
        mips.push_back(Mips::lw(Mips::SP, Mips::FP, std::to_string(frame_size)));
        if ((q = peek()) && q->get_type() == Quaternary::GET_RETURN) {
            q = peek();
            t = def_use(q->get_dest());
            rd = GET(t, 0);
            mips.push_back(Mips::addu(rd, Mips::ZERO, Mips::V0));
            pop();
        }
    }

    void MipsGenerator::parse_param() {
        const Quaternary *q = peek();
        size_t i = 0;
        reg_triple t;
        Mips::Registers rd, rs, rt;

        while ((q = peek()) && q->get_type() == Quaternary::PARAM) {
            t = def_use(q->get_dest());
            rd = GET(t, 0);
            if (i < 4) {
                mips.push_back(Mips::addu(rd, Mips::ZERO, Mips::args(i)));
            } else {
                mips.push_back(Mips::lw(rd, Mips::SP, std::to_string(i * 4)));
            }
            pop();
            ++i;
        }
    }

    void MipsGenerator::parse_set_return() {
        const Quaternary *ir = peek();
        reg_triple t;
        Mips::Registers rd, rs, rt;

        t = def_use("", ir->get_lhs());

        rs = GET(t, 1);

        mips.push_back(Mips::addu(Mips::V0, Mips::ZERO, rs));
        pop();
    }

    void MipsGenerator::parse_get_return() {

        DP("here ar get return!\n");
        const Quaternary *ir = peek();
        reg_triple t;
        Mips::Registers rd, rs, rt;

        t = def_use(ir->get_dest());

        rd = GET(t, 0);

        mips.push_back(Mips::addu(rd, Mips::ZERO, Mips::V0));
        pop();
    }

    void MipsGenerator::parse_return() {
        peek();
        dump_live_dirty();
        mips.push_back(Mips::jr(Mips::RA));
        pop();
    }

    void MipsGenerator::parse_input() {
        const Quaternary *ir = peek();
        string d;
        reg_triple t;
        Mips::Registers rd, rs, rt;
        t = def_use(ir->get_dest());
        rd = GET(t, 0);

        if (ir->get_type() == Quaternary::READI) {
            mips.push_back(Mips::ori(Mips::V0, Mips::ZERO, SYS_READ_INT));
        } else {
            mips.push_back(Mips::ori(Mips::V0, Mips::ZERO, SYS_READ_CHAR));
        }
        mips.push_back(Mips::syscall());
        mips.push_back(Mips::addu(rd, Mips::ZERO, Mips::V0));
        pop();
    }

    void MipsGenerator::parse_output() {
        const Quaternary *ir = peek();
        string s1, s2;
        reg_triple t;
        Mips::Registers rd, rs, rt;

        s1 = ir->get_lhs();
        s2 = ir->get_rhs();
        if (ir->get_type() == Quaternary::PRINTC) {
            t = def_use("", s1);
            rs = GET(t, 1);
            mips.push_back(Mips::addu(Mips::A0, Mips::ZERO, rs));
            mips.push_back(Mips::ori(Mips::V0, Mips::ZERO, SYS_PRINT_CHAR));
            mips.push_back(Mips::syscall());
        } else if (ir->get_type() == Quaternary::PRINTI) {
            t = def_use("", s1);
            rs = GET(t, 1);
            mips.push_back(Mips::addu(Mips::A0, Mips::ZERO, rs));
            mips.push_back(Mips::ori(Mips::V0, Mips::ZERO, SYS_PRINT_INT));
            mips.push_back(Mips::syscall());
        } else if (ir->get_type() == Quaternary::PRINTS) {
            s2 = get_string_label(s1);
            mips.push_back(Mips::ori(Mips::V0, Mips::ZERO, SYS_PRINT_STRING));
            mips.push_back(Mips::la(Mips::A0, s2));
            mips.push_back(Mips::syscall());
        } else {
            s1 = get_string_label(s1);
            mips.push_back(Mips::ori(Mips::V0, Mips::ZERO, SYS_PRINT_STRING));
            mips.push_back(Mips::la(Mips::A0, s1));
            mips.push_back(Mips::syscall());
            t = def_use("", s2);
            rs = GET(t, 1);
            mips.push_back(Mips::addu(Mips::A0, Mips::ZERO, rs));
            mips.push_back(Mips::ori(Mips::V0, Mips::ZERO, SYS_PRINT_INT));
            mips.push_back(Mips::syscall());
        }
        // newline();
        pop();
    }

    void MipsGenerator::parse_entry() {
        pop();
        mips.push_back(Mips::label("__main__"));
        mips.push_back(Mips::addu(Mips::FP, Mips::SP, Mips::ZERO));
    }

    void MipsGenerator::parse_exit() {
        DP("heiheihei exit\n");
        pop();
        mips.push_back(Mips::label("__main_end__"));
        mips.push_back(Mips::ori(Mips::V0, Mips::ZERO, SYS_EXIT));
        mips.push_back(Mips::syscall());
    }

    void MipsGenerator::parse_scope() {
        const Quaternary *ir = pop();

        switch (ir->get_type()) {
        case Quaternary::BEGIN:
            in_function = true;
            break;
        case Quaternary::END:
            in_function = false;
            bytes.clear();
            local_name_to_pos.clear();
            break;
        default:
            assert(0);
        }
    }

    void MipsGenerator::parse_def() {
        const Quaternary *ir = pop();
        string d, s1;
        size_t size;

        d = ir->get_dest();
        s1 = ir->get_lhs();
        size = atoi(s1.c_str());
        size = align(size);

        if (in_function) {
            declare_array_in_function(d, size);
        } else {
            declare_array_in_global(d, size);
        }
    }


    size_t align(size_t x) {
        return ((x + 3) >> 2) << 2;
    }

}

