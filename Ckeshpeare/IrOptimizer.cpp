#include "IrOptimizer.h"
#include "MipsGenerator.h"
#include "debug.h"
#include <map>
#include <string>

namespace dyj {
    CodeBlock::CodeBlock() {}

    void CodeBlock::append(const Quaternary &ir) {
        irs.push_back(ir);
    }

    void CodeBlock::dag_optimize(void) {

    }

    void CodeBlock::dump_over(std::vector<Quaternary> &ret) const {
        ret.clear();
        dump_append(ret);
    }

    void CodeBlock::dump_append(std::vector<Quaternary> &ret) const {
        for (auto &ir : irs) {
            ret.push_back(ir);
        }
    }

    FlowGraph::FlowGraph() {}

    void FlowGraph::load_and_cut(const std::vector<Quaternary> &irs) {
        size_t len = irs.size();
        CodeBlock *block = nullptr;

        for (size_t i = 0; i < len; ++i) {
            if (i == 0 || (i > 0 && irs[i - 1].is_jump()) || irs[i].get_type() == Quaternary::LABEL) {
                if (block) {
                    blocks.push_back(block);
                }
                DP("-------------------------------------\n");
                block = new CodeBlock();
            }
            block->append(irs[i]);
            DP("%s\n", irs[i].to_string().c_str());
        }
        blocks.push_back(block);
        DP("-------------------------------------\n");
    }

    std::vector<Quaternary> FlowGraph::dump(void) const {
        std::vector<Quaternary> ret;
        for (auto b : blocks) {
            b->dump_append(ret);
        }
        return ret;
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
            for (j = i + 1; j < n; ++j) {
                const Quaternary &y = old[j];
                DP("[%s] v [%s] : \n", x.to_string().c_str(), y.to_string().c_str());
                if (x.get_dest() == y.get_lhs() && y.get_type() == Quaternary::COPY && is_local(x.get_dest()) && last[y.get_lhs()] == j) {
                    DP("True1\n");
                    x = Quaternary(x.get_type(), y.get_dest(), x.get_lhs(), x.get_rhs());
                    usd[j] = true;
                    still_good = true;
                    DP("[%s]\n", x.to_string().c_str());
                } else if (x.get_type() == Quaternary::COPY && x.get_dest() == y.get_lhs() && is_local(x.get_dest()) && last[y.get_lhs()] == j) {
                    DP("True2\n");
                    usd[j] = true;
                    x = Quaternary(y.get_type(), y.get_dest(), x.get_lhs(), y.get_rhs());
                    still_good = true;
                    DP("[%s]\n", x.to_string().c_str());
                } else if (x.get_type() == Quaternary::COPY && x.get_dest() == y.get_rhs() && is_local(x.get_dest()) && last[y.get_rhs()] == j) {
                    DP("True3\n");
                    usd[j] = true;
                    x = Quaternary(y.get_type(), y.get_dest(), y.get_lhs(), x.get_lhs());
                    still_good = true;
                    DP("[%s]\n", x.to_string().c_str());
                } else {
                    DP("False\n");
                    break;
                }
            }
            irs.push_back(x);
        }
        return still_good;
    }

}
