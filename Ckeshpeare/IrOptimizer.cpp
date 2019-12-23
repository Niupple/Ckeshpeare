#include "IrOptimizer.h"
#include "MipsGenerator.h"
#include "debug.h"
#include "Namer.h"
#include <map>
#include <string>
#include <cassert>
#include <utility>

namespace dyj {
    CodeBlock::CodeBlock() {}

    void CodeBlock::append(const Quaternary &ir) {
        irs.push_back(ir);
    }

    void CodeBlock::add_edge(CodeBlock *rhs) {
        DP("link(%d, %d), %p - %p\n", block_id, rhs->block_id, this, rhs);
        forward.push_back(rhs);
        rhs->backward.push_back(this);
    }

    void CodeBlock::dag_optimize(void) {

    }

    void CodeBlock::dump_over(std::vector<Quaternary> &ret) const {
        ret.clear();
        dump_append(ret);
    }

    void CodeBlock::dump_append(std::vector<Quaternary> &ret, int block_id) const {
        for (auto &ir : irs) {
            ret.emplace_back(ir.get_type(), ir.get_dest(), ir.get_lhs(), ir.get_rhs(), block_id);
        }
    }

    void CodeBlock::get_use_def(void) {
        for (auto &ir : irs) {
            for (auto &var : ir.get_uses()) {
                if (!is_variable(var)) {
                    continue;
                }
                if (def.count(var) == 0 && use.count(var) == 0) {
                    use.insert(var);
                }
            }
            for (auto &var : ir.get_defs()) {
                if (!is_variable(var)) {
                    continue;
                }
                if (def.count(var) == 0 && use.count(var) == 0) {
                    def.insert(var);
                }
            }
        }
#ifdef _DEBUGi
        DP("use[%d]\n", block_id);
        for (auto &var : use) {
            DP("\t'%s'\n", var.c_str());
        }
        DP("def[%d]\n", block_id);
        for (auto &var : def) {
            DP("\t'%s'\n", var.c_str());
        }
#endif
    }

    FlowGraph::FlowGraph() : entry_block(0) {}

    void FlowGraph::load_and_cut(const std::vector<Quaternary> &irs) {
        size_t len = irs.size();
        CodeBlock *block = nullptr;

        for (size_t i = 0; i < len; ++i) {
            if (i == 0 || (i > 0 && irs[i - 1].is_jump()) || irs[i].get_type() == Quaternary::LABEL || irs[i].get_type() == Quaternary::ENTRY) {
                if (block) {
                    block->block_id = blocks.size();
                    blocks.push_back(block);
                }
                DP("-------------------------------------\n");
                block = new CodeBlock();
                if (irs[i].get_type() == Quaternary::LABEL) {
                    label_to_block[irs[i].get_rhs()] = block;
                } else if (irs[i].get_type() == Quaternary::ENTRY) {
                    entry_block = blocks.size();
                }
            }
            block->append(irs[i]);
            DP("%s\n", irs[i].to_string().c_str());
        }
        if (block) {
            block->block_id = blocks.size();
            blocks.push_back(block);
        }
        DP("-------------------------------------\n");
        link();
    }

    template <typename T>
    static void cupass(std::set<T> &a, const std::set<T> &b) {
        for (auto &x : b) {
            a.insert(x);
        }
    }

    template <typename T>
    static std::set<T> cup(const std::set<T> &a, const std::set<T> &b) {
        std::set<T> ret(a);
        cupass(ret, b);
        return ret;
    }

    template <typename T>
    static std::set<T> sub(const std::set<T> &a, const std::set<T> &b) {
        std::set<T> ret;
        for (auto &x : a) {
            if (b.count(x) == 0) {
                ret.insert(x);
            }
        }
        return std::move(ret);
    }

    void FlowGraph::flow_living_vars(void) {
        for (auto &block : blocks) {
            block->get_use_def();
        }
        bool flag = true;
        while (flag) {
            flag = false;
            DP("one more time!\n");
            for (auto i = blocks.rbegin(); i != blocks.rend(); ++i) {
                CodeBlock *block = *i;
                auto &out = out_living[block];
                auto &in = in_living[block];
                for (auto succ : block->forward) {
                    cupass(out, in_living[succ]);
                }
                auto old_in = in;
                in = cup(block->use, sub(out, block->def));
                if (old_in != in) {
                    flag = true;
                }
            }
        }
    }

    static void check_and_set_living(const std::string &name, std::set<std::string> &usd, std::set<std::string> &record) {
        if (is_user(name) || (is_temp(name) && usd.count(name) != 0)) {
            record.insert(name);
        }
    }

    void FlowGraph::get_living_vars(void) {
        // a naive and conservative version
        size_t n = blocks.size();
        std::set<std::string> usd;
        for (int i = 0; i < n; ++i) {
            living_vars.emplace_back();
        }
        for (int i = n - 1; i >= 0; --i) {
            CodeBlock *b = blocks[i];
            size_t m = b->irs.size();
            for (int j = m - 1; j >= 0; --j) {
                Quaternary &ir = b->irs[j];
                check_and_set_living(ir.get_dest(), usd, living_vars[i]);
                check_and_set_living(ir.get_lhs(), usd, living_vars[i]);
                check_and_set_living(ir.get_rhs(), usd, living_vars[i]);
            }
            for (size_t j = 0; j < m; ++j) {
                Quaternary &ir = b->irs[j];
                if (is_variable(ir.get_dest())) {
                    usd.insert(ir.get_dest());
                }
                if (is_variable(ir.get_lhs())) {
                    usd.insert(ir.get_lhs());
                }
                if (is_variable(ir.get_rhs())) {
                    usd.insert(ir.get_rhs());
                }
            }
        }
    }

    std::vector<Quaternary> FlowGraph::dump(void) const {
        std::vector<Quaternary> ret;
        for (size_t i = 0; i < blocks.size(); ++i) {
            const CodeBlock *b = blocks[i];
            b->dump_append(ret, i);
        }
        return ret;
    }

    std::vector<std::set<std::string>> &&FlowGraph::get_living_var_list(void) {
        return std::move(living_vars);
    }

    std::vector<std::set<std::string>> FlowGraph::get_living_in_list(void) {
        std::vector<std::set<std::string>> ret;
        for (auto block : blocks) {
            ret.push_back(in_living[block]);
#ifdef _DEBUGi
            DP("IN[%d] = \n", block->block_id);
            for (auto &var : in_living[block]) {
                DP("\t'%s'\n", var.c_str());
            }
#endif
        }
        return ret;
    }

    std::vector<std::set<std::string>> FlowGraph::get_living_out_list(void) {
        std::vector<std::set<std::string>> ret;
        for (auto block : blocks) {
            ret.push_back(out_living[block]);
#ifdef _DEBUGi
            DP("OUT[%d] = \n", block->block_id);
            for (auto &var : out_living[block]) {
                DP("\t'%s'\n", var.c_str());
            }
#endif
        }
        return ret;
    }

    std::set<std::pair<std::string, std::string>> &&FlowGraph::get_collisions(void) {
        return std::move(collisions);
    }

    void FlowGraph::link(void) {
        for (size_t i = 0; i < blocks.size(); ++i) {
            CodeBlock *block = blocks[i];
            auto &last_ir = block->irs.at(block->irs.size() - 1);
            switch (last_ir.get_type()) {
            case Quaternary::JUMP_IF:
            case Quaternary::JUMP_UNLESS:
                if (i + 1 < blocks.size()) {
                    block->add_edge(blocks[i + 1]);
                }
            case Quaternary::JUMP:
                block->add_edge(label_to_block.at(last_ir.get_rhs()));
                break;
            case Quaternary::RETURN:
                break;
            default:
                if (i + 1 < blocks.size()) {
                    block->add_edge(blocks[i + 1]);
                }
            }
        }
    }

    void FlowGraph::collision_analysis(void) {
        for (size_t i = entry_block; i < blocks.size(); ++i) {
            CodeBlock *block = blocks[i];
            for (size_t i = 0; i < block->irs.size(); ++i) {
                for (auto &var : block->irs[i].get_defs()) {
                    collisions.insert({ var, var });
                    clear_for_new_var();
                    dfs(block, i + 1, var);
                }
            }
        }
    }

    void FlowGraph::confirm(void) {
        //DP("confirmed\n");
        for (auto p : suspects) {
            collisions.insert(p);
        }
        suspects.clear();
    }

    void FlowGraph::suspect(const std::string &a, const std::string &b) {
        //DP("suspect(%s, %s)\n", a.c_str(), b.c_str());
        if (is_const(a) || is_const(b)) {
            return;
        }
        if (a > b) {
            suspects.insert({ b, a });
        } else if (b > a) {
            suspects.insert({ a, b });
        }
    }

    void FlowGraph::dfs(CodeBlock *block, size_t id, const std::string &var) {
        DP("dfs at (%p, %u, %s)\n", block, (unsigned)id, var.c_str());
        for (size_t i = id; i < block->irs.size(); ++i) {
            if (!visit(block, i)) {
                return;
            }
            const Quaternary &ir = block->irs[i];
            for (auto v : ir.get_uses()) {
                if (v != var) {
                    suspect(v, var);
                }
            }
            for (auto v : ir.get_defs()) {
                if (v != var) {
                    suspect(v, var);
                }
            }
            if (ir.get_uses().count(var) > 0) {
                confirm();
            }
            if (ir.get_defs().count(var) > 0) {
                return;
            }
        }
        for (auto nxt : block->forward) {
            if (visited.count({ nxt, 0 }) > 0) {
                if (in_living[nxt].count(var) > 0) {
                    confirm();
                }
            } else {
                dfs(nxt, 0, var);
            }
        }
    }

    bool FlowGraph::visit(CodeBlock *block, size_t id) {
        auto p = std::make_pair(block, id);
        if (visited.count(p) > 0) {
            return false;
        } else {
            visited.insert(p);
            return true;
        }
    }

    void FlowGraph::clear_for_new_var(void) {
        suspects.clear();
        visited.clear();
    }

    bool ir_peek_hole(std::vector<Quaternary> &irs) {
        bool still_good = false;
        std::vector<Quaternary> old = std::move(irs);
        std::vector<bool> usd;
        std::map<std::string, int> first, last;
        irs.clear();
        size_t n = old.size();
        usd.resize(n);
        for (size_t i = 0; i < n; ++i) {
            const Quaternary &ir = old[i];
            last[ir.get_dest()] = i;
            last[ir.get_lhs()] = i;
            last[ir.get_rhs()] = i;
        }
        for (int i = n - 1; i > 0; --i) {
            const Quaternary &ir = old[i];
            first[ir.get_dest()] = i;
            first[ir.get_lhs()] = i;
            first[ir.get_rhs()] = i;
        }
        for (size_t i = 0; i < n; ++i) {
            if (usd[i]) {
                continue;
            }
            usd[i] = true;
            Quaternary &x = old[i];
            size_t j;
            if (i + 1 < n && x.get_type() == Quaternary::JUMP && old[i + 1].get_type() == Quaternary::LABEL && x.get_rhs() == old[i + 1].get_rhs()) {
                continue;
            }
            if (x.get_type() == Quaternary::COPY && i == last[x.get_dest()] && is_temp(x.get_dest())) {
                continue;
            }
            for (j = i + 1; j < n; ++j) {
                const Quaternary &y = old[j];
                // DP("[%s] v [%s] : \n", x.to_string().c_str(), y.to_string().c_str());
                if (x.get_type() != Quaternary::CALL && x.get_dest() == y.get_lhs() && y.get_type() == Quaternary::COPY && is_temp(x.get_dest()) && last[y.get_lhs()] == j) {
                    // DP("True1\n");
                    x = Quaternary(x.get_type(), y.get_dest(), x.get_lhs(), x.get_rhs(), x.get_block_id());
                    usd[j] = true;
                    still_good = true;
                    DP("[%s]\n", x.to_string().c_str());
                } else if (x.get_type() == Quaternary::COPY && x.get_dest() == y.get_lhs() && is_temp(x.get_dest()) && last[y.get_lhs()] == j) {
                    // DP("True2\n");
                    usd[j] = true;
                    x = Quaternary(y.get_type(), y.get_dest(), x.get_lhs(), y.get_rhs(), x.get_block_id());
                    still_good = true;
                    DP("[%s]\n", x.to_string().c_str());
                } else if (x.get_type() == Quaternary::COPY && x.get_dest() == y.get_rhs() && is_temp(x.get_dest()) && last[y.get_rhs()] == j) {
                    // DP("True3\n");
                    usd[j] = true;
                    x = Quaternary(y.get_type(), y.get_dest(), y.get_lhs(), x.get_lhs(), x.get_block_id());
                    still_good = true;
                    DP("[%s]\n", x.to_string().c_str());
                } else {
                    // DP("False\n");
                    break;
                }
            }
            irs.push_back(x);
        }

        return still_good;
    }

    Inliner::Inliner(Namer *_label_namer, Namer *_var_namer) : label_namer(_label_namer), var_namer(_var_namer), namer_counter(1) {}

    void Inliner::load_and_parse(const IrList &_irs) {
        irs = _irs;
        parse_functions();
        apply_all();
    }

    IrList &&Inliner::dump(void) {
        return std::move(output);
    }

    Inliner::~Inliner() {
        for (auto func : functions) {
            delete func;
        }
    }

    void Inliner::parse_functions(void) {
        bool in_function = false;
        int n = irs.size();
        Function *func = nullptr;
        int i;
        for (i = 0; i < n; ++i) {
            Quaternary &ir = irs[i];
            if (ir.get_type() == Quaternary::VAR) {
                header.push_back(ir);
            } else {
                break;
            }
        }
        for (; i < n; ++i) {
            Quaternary &ir = irs[i];
            DP("IR: %s\n", ir.to_string().c_str());
            if (ir.get_type() == Quaternary::ENTRY) {
                DP("all functions\n");
                break;
            }
            if (!in_function) {
                if (ir.get_type() == Quaternary::BEGIN) {
                    DP("a function has begun\n");
                    in_function = true;
                    func = nullptr;
                }
            } else {
                if (func == nullptr) {
                    func = new Function();
                    assert(ir.get_type() == Quaternary::LABEL);
                    func->name = ir.get_rhs();
                    DP("naming %s\n", func->name.c_str());
                } else if (ir.get_type() == Quaternary::PARAM) {
                    func->params.push_back(ir.get_dest());
                } else if (ir.get_type() == Quaternary::END) {
                    DP("end of that function\n");
                    functions.push_back(func);
                    func = nullptr;
                    in_function = false;
                } else {
                    func->body.push_back(ir);
                }
                if (ir.get_type() == Quaternary::CALL) {
                    if (ir.get_dest() == func->name) {
                        DP("not inlinable!\n");
                        func->inlinable = false;
                    }
                }
            }
        }
        for (; i < n; ++i) {
            main_function.push_back(irs[i]);
            DP("main: %s\n", irs[i].to_string().c_str());
        }
    }

    void Inliner::apply_all(void) {
        output.insert(output.end(), header.begin(), header.end());
        for (auto func : functions) {
            IrList new_irs;
            apply_inline(new_irs, func->body);
            func->body = new_irs;
            if (!func->inlinable) {
                func->dump(output);
            }
        }
        apply_inline(output, main_function);
    }

    void Inliner::apply_inline(IrList &irs, const IrList &old) {
        int n = old.size();
        std::vector<std::string> args;
        Function *func = nullptr;
        for (int i = 0; i < n; ++i) {
            const Quaternary &ir = old[i];
            DP("IR: %s\n", ir.to_string().c_str());
            if (ir.get_type() == Quaternary::ARGUMENT || ir.get_type() == Quaternary::CALL) {
                DP("a function call !\n");
                args.clear();
                while (i < n && old[i].get_type() == Quaternary::ARGUMENT) {
                    args.push_back(old[i++].get_lhs());
                }
                DP("IR: %s\n", old[i].to_string().c_str());
                assert(old[i].get_type() == Quaternary::CALL);
                func = get_function(old[i].get_dest());
                DP("call for %s\n", func->name.c_str());
                if (func->inlinable) {
                    DP("inlinable!\n");
                    inline_here(irs, func, args);
                } else {
                    for (int j = 0; j < args.size(); ++j) {
                        irs.emplace_back(Quaternary::ARGUMENT, "", args[j]);
                    }
                    irs.emplace_back(Quaternary::CALL, func->name);
                }
            } else {
                irs.push_back(ir);
            }
        }
    }

    Inliner::Function *Inliner::get_function(const std::string &name) {
        for (size_t i = 0; i < functions.size(); ++i) {
            if (functions[i]->name == name) {
                return functions[i];
            }
        }
        return nullptr;
    }

    void Inliner::inline_here(IrList &output, Function *func, const std::vector<std::string> &args) {
        std::string end_label = label_namer->new_name();

        for (size_t i = 0; i < args.size(); ++i) {
            output.emplace_back(Quaternary::COPY, translate_var(func->params[i]), args[i]);
        }
        for (size_t i = 0; i < func->body.size(); ++i) {
            Quaternary &ir = func->body[i];
            switch (ir.get_type()) {
            case Quaternary::JUMP:
            case Quaternary::JUMP_IF:
            case Quaternary::JUMP_UNLESS:
                output.emplace_back(ir.get_type(), translate_var(ir.get_dest()), translate_var(ir.get_lhs()), translate_label(ir.get_rhs()));
                break;
            case Quaternary::LABEL:
                output.emplace_back(Quaternary::LABEL, "", "", translate_label(ir.get_rhs()));
                break;
            case Quaternary::RETURN:
                output.emplace_back(Quaternary::JUMP, "", "", end_label);
                break;
            case Quaternary::CALL:
                output.push_back(ir);
                break;
            default:
                output.emplace_back(ir.get_type(), translate_var(ir.get_dest()), translate_var(ir.get_lhs()), translate_var(ir.get_rhs()));
                break;
            }
        }
        output.emplace_back(Quaternary::LABEL, "", "", end_label);
        ++namer_counter;
        new_var.clear();
        new_label.clear();
    }

    std::string Inliner::translate_var(const std::string &var) {
        if (is_global(var)) {
            return var;
        } else if (is_user(var)) {
            return var + "@" + std::to_string(namer_counter);
        } else if (is_temp(var)) {
            if (new_var.count(var) == 0) {
                new_var[var] = var_namer->new_name();
            }
            return new_var.at(var);
        } else {
            return var;
        }
    }

    std::string Inliner::translate_label(const std::string &label) {
        if (new_label.count(label) == 0) {
            new_label[label] = label_namer->new_name();
        }
        return new_label.at(label);
    }

    void Inliner::Function::dump(IrList &output) {
        output.emplace_back(Quaternary::BEGIN);
        output.emplace_back(Quaternary::LABEL, "", "", name);
        for (auto &param : params) {
            output.emplace_back(Quaternary::PARAM, param);
        }
        for (auto &ir : body) {
            output.push_back(ir);
        }
        output.emplace_back(Quaternary::END);
        output.emplace_back(Quaternary::RETURN);
    }

}
