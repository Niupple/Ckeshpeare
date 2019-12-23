#include <cassert>
#include "BladeRunner.h"
#include "Namer.h"
#include "debug.h"

namespace dyj {
    BladeRunner::BladeRunner(const IrList &_irs) : irs(_irs), entry(0), pc(0), ret(0) {}

    void BladeRunner::check_and_run(void) {
        if (check_runnable()) {
            run();
        } else {
            outs = irs;
        }
    }

    IrList &&BladeRunner::dump(void) {
        return std::move(outs);
    }

    bool BladeRunner::check_runnable(void) {
        size_t n = irs.size();
        for (size_t i = 0; i < n; ++i) {
            const Quaternary &ir = irs[i];
            if (ir.get_type() == Quaternary::READC || ir.get_type() == Quaternary::READI) {
                return false;
            } else if (ir.get_type() == Quaternary::ENTRY) {
                entry = i;
            } else if (ir.get_type() == Quaternary::EXIT) {
                exit = i;
            } else if (ir.get_type() == Quaternary::LABEL) {
                labels[ir.get_rhs()] = i;
            }
        }
        return true;
    }

    void BladeRunner::run(void) {
        bool flag = true;
        pc = entry;
        outs.emplace_back(Quaternary::ENTRY);
        outs.emplace_back(Quaternary::BEGIN);
        while (flag) {
            const Quaternary &ir = irs[pc];
            const std::string &dest = ir.get_dest(), &lhs = ir.get_lhs(), &rhs = ir.get_rhs();
            //DP("@%u : %s\n", pc, ir.to_string().c_str());
            switch (ir.get_type()) {
            case Quaternary::PLUS:
                def(dest, use(lhs) + use(rhs));
                break;
            case Quaternary::MINUS:
                def(dest, use(lhs) - use(rhs));
                break;
            case Quaternary::TIME:
                def(dest, use(lhs) * use(rhs));
                break;
            case Quaternary::DIVIDE:
                def(dest, use(lhs) / use(rhs));
                break;
            case Quaternary::NEGATE:
                def(dest, -use(lhs));
                break;
            case Quaternary::COPY:
                def(dest, use(lhs));
                break;
            case Quaternary::LESS:
                def(dest, use(lhs) < use(rhs));
                break;
            case Quaternary::LEQ:
                def(dest, use(lhs) <= use(rhs));
                break;
            case Quaternary::EQU:
                def(dest, use(lhs) == use(rhs));
                break;
            case Quaternary::NEQ:
                def(dest, use(lhs) != use(rhs));
                break;
            case Quaternary::INDEX:
                def(dest, use(arr2var(lhs, use(rhs))));
                break;
            case Quaternary::ELEMENT:
                def(arr2var(dest, use(lhs)), use(rhs));
                break;
            case Quaternary::JUMP:
                pc = label2addr(rhs);
                continue;
            case Quaternary::JUMP_IF:
                if (use(lhs)) {
                    pc = label2addr(rhs);
                    continue;
                }
                break;
            case Quaternary::JUMP_UNLESS:
                if (!use(lhs)) {
                    pc = label2addr(rhs);
                    continue;
                }
                break;
            case Quaternary::LABEL:
            case Quaternary::BEGIN:
            case Quaternary::END:
            case Quaternary::VAR:
                break;
            case Quaternary::ENTRY:
                push_layer(exit - 1);
                break;
            case Quaternary::ARGUMENT:
                push_arg(use(lhs));
                break;
            case Quaternary::CALL:
                push_layer(pc);
                pc = label2addr(dest);
                break;
            case Quaternary::PARAM:
                def(dest, pop_arg());
                break;
            case Quaternary::SET_RETURN:
                ret = use(lhs);
                break;
            case Quaternary::GET_RETURN:
                def(dest, ret);
                break;
            case Quaternary::RETURN:
                pc = pop_layer();
                break;
            case Quaternary::READC:
            case Quaternary::READI:
                assert(0);
            case Quaternary::PRINTC:
                outs.emplace_back(Quaternary::PRINTC, "", std::to_string(use(lhs)));
                break;
            case Quaternary::PRINTI:
                outs.emplace_back(Quaternary::PRINTI, "", std::to_string(use(lhs)));
                break;
            case Quaternary::PRINTS:
                outs.push_back(ir);
                break;
            case Quaternary::PRINTF:
                outs.emplace_back(Quaternary::PRINTF, "", std::to_string(use(lhs)));
                break;
            case Quaternary::EXIT:
                flag = false;
                break;
            case Quaternary::COMMENT:
                break;
            }
            ++pc;
        }
        outs.emplace_back(Quaternary::END);
        outs.emplace_back(Quaternary::EXIT);
    }

    int BladeRunner::use(const std::string &var) {
        if (is_const(var)) {
            return std::stoi(var);
        } else if (is_global(var)) {
            return global_vars[var];
        } else {
            return vars.back()[var];
        }
    }

    void BladeRunner::def(const std::string &var, int v) {
        if (is_global(var)) {
            global_vars[var] = v;
        } else {
            vars.back()[var] = v;
        }
    }

    std::string BladeRunner::arr2var(const std::string &arr, int idx) {
        if (is_global(arr)) {
            return arr + "__" + std::to_string(idx) + "@0";
        } else {
            return arr + "__" + std::to_string(idx);
        }
    }

    size_t BladeRunner::label2addr(const std::string &var) {
        return labels.at(var);
    }

    void BladeRunner::push_arg(int arg) {
        args.push(arg);
    }

    int BladeRunner::pop_arg(void) {
        int ret = args.front();
        args.pop();
        return ret;
    }

    void BladeRunner::push_layer(size_t pc_old) {
        ra.push(pc_old);
        vars.emplace_back();
    }

    size_t BladeRunner::pop_layer(void) {
        size_t ret = ra.top();
        ra.pop();
        vars.pop_back();
        return ret;
    }

}
