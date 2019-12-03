#include "Mips.h"

namespace dyj {
    Mips::Mips() {}

    std::string Mips::reg(Registers r) {
        return "$" + std::to_string(r);
    }

    Mips::Registers Mips::args(size_t i) {
        switch (i) {
        case 0:
            return A0;
        case 1:
            return A1;
        case 2:
            return A2;
        case 3:
            return A3;
        default:
            return ZERO;
        }
    }

    Jump::Jump(Type _type, const std::string &_label) : type(_type), label(_label) {}

    std::string Jump::to_string(void) const {
        switch (type) {
        case J:
        case JAL:
            return repr[type] + " " + label;
        case LABEL:
            return label + ":";
        case SYSCALL:
            return repr[type];
        }
    }

    std::vector<std::string> Jump::repr{
        "j",
        "jal",
        ":",
        "syscall",
    };

    Immediate::Immediate(Type _type, Registers _rs, Registers _rt, const std::string &_label) : type(_type), rs(_rs), rt(_rt), label(_label) {}

    std::string Immediate::to_string(void) const {
        switch (type) {
        case ADDIU:
        case SUBIU:
        case ANDI:
        case ORI:
        case XORI:
        case LUI:
        case SLTI:
        case SLTIU:
        case BEQ:
        case BNE:
        case SLL:
        case SRL:
        case SRA:
            return repr[type] + " " + reg(rs) + " " + reg(rt) + " " + label;
        case LW:
        case SW:
            return repr[type] + " " + reg(rs) + " " + label + "(" + reg(rt) + ")";
        case LI:
        case LA:
        case BLEZ:
        case BGTZ:
        case BLTZ:
        case BGEZ:
            return repr[type] + " " + reg(rs) + " " + label;
        }
    }

    std::vector<std::string> Immediate::repr{
        "addiu",
        "subiu",
        "andi",
        "ori",
        "xori",
        "lui",
        "slti",
        "sltu",
        "beq",
        "bne",
        "blez",
        "bgtz",
        "bltz",
        "bgez",
        "lw",
        "sw",
        "li",
        "la",
        "sll",
        "srl",
        "sra",
    };

    Register::Register(Type _type, Registers _rd, Registers _rs, Registers _rt) : type(_type), rd(_rd), rs(_rs), rt(_rt) {}

    std::string Register::to_string(void) const {
        switch (type) {
            case ADDU:
            case SUBU:
            case MULT:
            case MUL:
            case DIV:
            case SLT:
            case SLE:
            case SEQ:
            case SNE:
            case SLLV:
            case SRLV:
            case SRAV:
            case AND:
            case OR:
            case XOR:
            case NOR:
                return repr[type] + " " + reg(rd) + " " + reg(rs) + " " + reg(rt);
            case MFHI:
            case MFLO:
            case MTHI:
            case MTLO:
                return repr[type] + " " + reg(rd);
            case JR:
                return repr[type] + " " + reg(rs);
        }
    }

    std::vector<std::string> Register::repr{
        "addu",
        "subu",
        "mult",
        "mul",
        "div",
        "slt",
        "sle",
        "seq",
        "sne",
        "sllv",
        "srlv",
        "srav",
        "and",
        "or",
        "xor",
        "nor",
        "mfhi",
        "mflo",
        "mthi",
        "mtlo",
        "jr",
    };

}
