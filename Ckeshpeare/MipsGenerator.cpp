#include "MipsGenerator.h"
#include "Mars.h"
#include "debug.h"
#include <cassert>
#include <cctype>

namespace dyj {
    using std::string;
    typedef Jump::Type J;
    typedef Immediate::Type I;
    typedef Register::Type R;

    MipsGenerator::MipsGenerator(const std::vector<Quaternary> &_irs, Namer *_tagger) : irs(_irs), counter(0U), in_function(false), tagger(_tagger) {
        DP("inside constructor\n");
    }

    void MipsGenerator::parse(void) {
        typedef Quaternary::Type IR;
        const Quaternary *ir;
        while (has_next()) {
            ir = peek();
            DP("next is %d\n", ir->get_type());
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
            case IR::ARGUMENT:
                parse_function_call();
                break;
            case IR::CALL:
                assert(0);
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
        output += "j __main__\n";
        for (auto p : mips) {
            output += p->to_string() + "\n";
        }
    }

    const Quaternary *MipsGenerator::peek(void) {
        if (counter < irs.size()) {
            return &irs[counter];
        } else {
            return nullptr;
        }
    }

    const Quaternary *MipsGenerator::pop(void) {
        if (counter < irs.size()) {
            return &irs[counter++];
        } else {
            return nullptr;
        }
    }

    bool MipsGenerator::has_next(void) {
        return counter < irs.size();
    }

    void MipsGenerator::use(Mips::Registers reg, const std::string &var) {
        if (is_const(var)) {
            mips.push_back(new Immediate(I::LI, reg, Mips::ZERO, var));
        } else if (!is_global(var)) {
            size_t idx;
            int addr;
            if (local_name_to_pos.find(var) != local_name_to_pos.end()) {
                idx = local_name_to_pos.at(var);
                addr = idx_to_addr(idx);
                mips.push_back(new Immediate(I::LW, reg, Mips::SP, std::to_string(addr)));
            } else {
                idx = local_name_to_pos[var] = bytes.size();
                bytes.push_back(4);
                addr = idx_to_addr(idx);
                mips.push_back(new Immediate(I::LW, reg, Mips::SP, std::to_string(addr)));
            }
        } else {
            std::string label;
            if (global_name_to_pos.find(var) != global_name_to_pos.end()) {
                label = global_name_to_pos.at(var);
                mips.push_back(new Immediate(I::LW, reg, Mips::ZERO, label));
            } else {
                label = tagger->new_name();
                global_name_to_pos[var] = label;
                global_label_to_size[label] = 4;
                mips.push_back(new Immediate(I::LW, reg, Mips::ZERO, label));
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
                mips.push_back(new Immediate(I::SLL, Mips::T0, aid, "2"));
                mips.push_back(new Register(R::ADDU, Mips::GP, Mips::SP, Mips::T0));
                mips.push_back(new Immediate(I::LW, dst, Mips::GP, std::to_string(addr)));
            } else {
                DP("LOCAL ARRAY USED BEFORE DECLARE\n");
            }
        } else {
            std::string label;
            if (global_name_to_pos.find(arr) != global_name_to_pos.end()) {
                label = global_name_to_pos.at(arr);
                mips.push_back(new Immediate(I::SLL, Mips::T0, aid, "2"));
                mips.push_back(new Immediate(I::LW, dst, Mips::T0, label));
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
                mips.push_back(new Immediate(I::SW, reg, Mips::SP, std::to_string(addr)));
            } else {
                idx = local_name_to_pos[var] = bytes.size();
                bytes.push_back(4);
                addr = idx_to_addr(idx);
                mips.push_back(new Immediate(I::SW, reg, Mips::SP, std::to_string(addr)));
            }
        } else {
            std::string label;
            if (global_name_to_pos.find(var) != global_name_to_pos.end()) {
                label = global_name_to_pos.at(var);
                mips.push_back(new Immediate(I::SW, reg, Mips::ZERO, label));
            } else {
                label = tagger->new_name();
                global_name_to_pos[var] = label;
                global_label_to_size[label] = 4;
                mips.push_back(new Immediate(I::SW, reg, Mips::ZERO, label));
            }
        }
    }

    void MipsGenerator::def_array(Mips::Registers src, const std::string &arr, Mips::Registers aid) {
        if (!is_global(arr)) {
            size_t idx, addr;
            if (local_name_to_pos.find(arr) != local_name_to_pos.end()) {
                idx = local_name_to_pos.at(arr);
                addr = idx_to_addr(idx);
                mips.push_back(new Immediate(I::SLL, Mips::T0, aid, "2"));
                mips.push_back(new Register(R::ADDU, Mips::GP, Mips::SP, Mips::T0));
                mips.push_back(new Immediate(I::SW, src, Mips::GP, std::to_string(addr)));
            } else {
                DP("LOCAL ARRAY USED BEFORE DECLARE\n");
            }
        } else {
            std::string label;
            if (global_name_to_pos.find(arr) != global_name_to_pos.end()) {
                label = global_name_to_pos.at(arr);
                mips.push_back(new Immediate(I::SLL, Mips::T0, aid, "2"));
                mips.push_back(new Immediate(I::SW, src, Mips::T0, label));
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
        mips.push_back(new Immediate(I::ORI, Mips::A0, Mips::ZERO, "10"));
        mips.push_back(new Immediate(I::ORI, Mips::V0, Mips::ZERO, SYS_PRINT_CHAR));
        mips.push_back(new Jump(J::SYSCALL));
    }

    int MipsGenerator::idx_to_addr(size_t idx) {
        size_t ret = 0;
        for (size_t i = 0; i <= idx; ++i) {
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

    void MipsGenerator::parse_plus_and_minus() {
        const Quaternary *ir = pop();
        string d, s1, s2;
        d = ir->get_dest();
        s1 = ir->get_lhs();
        s2 = ir->get_rhs();

        if (ir->get_type() == Quaternary::PLUS) {
            if (is_const(s1)) {
                swap(s1, s2);            
                if (is_const(s1)) {
                    s1 = std::to_string(atoi(s1.c_str()) + atoi(s2.c_str()));
                    mips.push_back(new Immediate(I::LI, Mips::T0, Mips::ZERO, s1));
                    def(Mips::T0, d);
                    return;
                }
            }
            if (is_const(s2)) {
                use(Mips::T1, s1);
                mips.push_back(new Immediate(I::ADDIU, Mips::T0, Mips::T1, s2));
            } else {
                use(Mips::T1, s1);
                use(Mips::T2, s2);
                mips.push_back(new Register(R::ADDU, Mips::T0, Mips::T1, Mips::T2));
            }
            def(Mips::T0, d);
        } else {
            if (is_const(s1) && is_const(s2)) {
                s1 = std::to_string(atoi(s1.c_str()) - atoi(s2.c_str()));
                mips.push_back(new Immediate(I::LI, Mips::T0, Mips::ZERO, s1));
                def(Mips::T0, d);
                return;
            } else if (is_const(s2)) {
                use(Mips::T1, s1);
                mips.push_back(new Immediate(I::SUBIU, Mips::T0, Mips::T1, s2));
                def(Mips::T0, d);
            } else if (is_const(s1)) {
                use(Mips::T2, s2);
                mips.push_back(new Register(R::SUBU, Mips::T0, Mips::ZERO, Mips::T2));
                mips.push_back(new Immediate(I::ADDIU, Mips::T0, Mips::T0, s1));
                def(Mips::T0, d);
            } else {
                use(Mips::T1, s1);
                use(Mips::T2, s2);
                mips.push_back(new Register(R::SUBU, Mips::T0, Mips::T1, Mips::T2));
                def(Mips::T0, d);
            }
        }
    }

    void MipsGenerator::parse_time_and_divide() {
        const Quaternary *ir = pop();
        string d, s1, s2;
        d = ir->get_dest();
        s1 = ir->get_lhs();
        s2 = ir->get_rhs();

        use(Mips::T1, s1);
        use(Mips::T2, s2);
        if (ir->get_type() == Quaternary::TIME) {
            mips.push_back(new Register(R::MUL, Mips::T0, Mips::T1, Mips::T2));
        } else {
            mips.push_back(new Register(R::DIV, Mips::T0, Mips::T1, Mips::T2));
        }
        def(Mips::T0, d);
    }

    void MipsGenerator::parse_copy_and_negate() {
        const Quaternary *ir = pop();
        string d, s1, s2;
        d = ir->get_dest();
        s1 = ir->get_lhs();

        use(Mips::T1, s1);
        if (ir->get_type() == Quaternary::NEGATE) {
            mips.push_back(new Register(R::SUBU, Mips::T1, Mips::ZERO, Mips::T1));
        }
        def(Mips::T1, d);
    }

    void MipsGenerator::parse_condition() {
        const Quaternary *ir = pop();
        string d, s1, s2;
        d = ir->get_dest();
        s1 = ir->get_lhs();
        s2 = ir->get_rhs();

        use(Mips::T1, s1);
        use(Mips::T2, s2);
        switch (ir->get_type()) {
        case Quaternary::LESS:
            mips.push_back(new Register(R::SLT, Mips::T0, Mips::T1, Mips::T2));
            break;
        case Quaternary::LEQ:
            mips.push_back(new Register(R::SLE, Mips::T0, Mips::T1, Mips::T2));
            break;
        case Quaternary::EQU:
            mips.push_back(new Register(R::SEQ, Mips::T0, Mips::T1, Mips::T2));
            break;
        case Quaternary::NEQ:
            mips.push_back(new Register(R::SNE, Mips::T0, Mips::T1, Mips::T2));
            break;
        default:
            assert(0);
        }
        def(Mips::T0, d);
    }

    void MipsGenerator::parse_index() { // d = s1[s2]
        const Quaternary *ir = pop();
        string d, s1, s2;
        d = ir->get_dest();
        s1 = ir->get_lhs();
        s2 = ir->get_rhs();

        use(Mips::T2, s2);
        use_array(Mips::T1, s1, Mips::T2);
        def(Mips::T1, d);
    }

    void MipsGenerator::parse_element() { // d[s1] = s2
        const Quaternary *ir = pop();
        string d, s1, s2;
        d = ir->get_dest();
        s1 = ir->get_lhs();
        s2 = ir->get_rhs();

        use(Mips::T1, s1);
        use(Mips::T2, s2);
        def_array(Mips::T2, d, Mips::T1);
    }

    void MipsGenerator::parse_jumps() {
        const Quaternary *ir = pop();
        string d, label;
        
        label = ir->get_rhs();
        if (ir->get_type() == Quaternary::JUMP) {
            mips.push_back(new Jump(J::J, label));
        } else if (ir->get_type() == Quaternary::JUMP_IF) {
            d = ir->get_lhs();
            use(Mips::T0, d);
            mips.push_back(new Immediate(I::BGTZ, Mips::T0, Mips::ZERO, label));
        } else {
            d = ir->get_lhs();
            use(Mips::T0, d);
            mips.push_back(new Immediate(I::BEQ, Mips::T0, Mips::ZERO, label));
        }
    }

    void MipsGenerator::parse_label() {
        const Quaternary *ir = pop();
        string label = ir->get_rhs();

        mips.push_back(new Jump(J::LABEL, label));
    }

    void MipsGenerator::parse_function_call() {
        pop();
    }

    void MipsGenerator::parse_param() {
        pop();
    }

    void MipsGenerator::parse_set_return() {
        pop();
    }

    void MipsGenerator::parse_get_return() {
        pop();
    }

    void MipsGenerator::parse_return() {
        pop();
    }

    void MipsGenerator::parse_input() {
        const Quaternary *ir = pop();
        string d;
        d = ir->get_dest();

        if (ir->get_type() == Quaternary::READI) {
            mips.push_back(new Immediate(I::LI, Mips::V0, Mips::ZERO, SYS_READ_INT));
            mips.push_back(new Jump(J::SYSCALL));
            def(Mips::V0, d);
        } else {
            mips.push_back(new Immediate(I::LI, Mips::V0, Mips::ZERO, SYS_READ_CHAR));
            mips.push_back(new Jump(J::SYSCALL));
            def(Mips::V0, d);
        }
    }

    void MipsGenerator::parse_output() {
        const Quaternary *ir = pop();
        string s1, s2;

        s1 = ir->get_lhs();
        s2 = ir->get_rhs();
        if (ir->get_type() == Quaternary::PRINTC) {
            use(Mips::A0, s1);
            mips.push_back(new Immediate(I::LI, Mips::V0, Mips::ZERO, SYS_PRINT_CHAR));
            mips.push_back(new Jump(J::SYSCALL));
        } else if (ir->get_type() == Quaternary::PRINTI) {
            use(Mips::A0, s1);
            mips.push_back(new Immediate(I::LI, Mips::V0, Mips::ZERO, SYS_PRINT_INT));
            mips.push_back(new Jump(J::SYSCALL));
        } else if (ir->get_type() == Quaternary::PRINTS) {
            s2 = get_string_label(s1);
            mips.push_back(new Immediate(I::LI, Mips::V0, Mips::ZERO, SYS_PRINT_STRING));
            mips.push_back(new Immediate(I::LA, Mips::A0, Mips::ZERO, s2));
            mips.push_back(new Jump(J::SYSCALL));
        } else {
            s1 = get_string_label(s1);
            mips.push_back(new Immediate(I::LI, Mips::V0, Mips::ZERO, SYS_PRINT_STRING));
            mips.push_back(new Immediate(I::LA, Mips::A0, Mips::ZERO, s1));
            mips.push_back(new Jump(J::SYSCALL));
            use(Mips::A0, s1);
            mips.push_back(new Immediate(I::LI, Mips::V0, Mips::ZERO, SYS_PRINT_INT));
            mips.push_back(new Jump(J::SYSCALL));
        }
        // newline();
    }

    void MipsGenerator::parse_entry() {
        pop();
        mips.push_back(new Jump(J::LABEL, "__main__"));
    }

    void MipsGenerator::parse_exit() {
        DP("heiheihei exit\n");
        pop();
        mips.push_back(new Immediate(I::ORI, Mips::V0, Mips::ZERO, SYS_EXIT));
        mips.push_back(new Jump(J::SYSCALL));
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

    bool MipsGenerator::is_global(const std::string &name) {
        size_t pos = name.find_last_of('@');
        DP("%s\n", name.c_str());
        if (pos == std::string::npos) {
            return false;
        } else {
            return name.substr(pos) == "@0";
        }
    }

    bool MipsGenerator::is_const(const std::string &name) {
        return name.front() == '-' || name.front() == '+' || isdigit(name.front());
    }

    size_t align(size_t x) {
        return ((x + 3) >> 2) << 2;
    }

}

