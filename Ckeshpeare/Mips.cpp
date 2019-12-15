#include "Mips.h"

namespace dyj {
    Mips::Mips() {}

    Mips::Mips(Type _type, Registers rd, Registers rs, Registers rt, const std::string &__label) : type(_type), rd(rd), rs(rs), rt(rt), _label(__label) {}

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

    std::vector<std::string> Mips::repr{
        "j",
        "jal",
        ":",
        "syscall",
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

    std::string Mips::to_string(void) const {
        switch (type) {
        case J:
        case JAL:
            return repr[type] + " " + _label;
        case LABEL:
            return _label + ":";
        case SYSCALL:
            return repr[type];
        case ADDIU:
        case SUBIU:
        case ANDI:
        case ORI:
        case XORI:
        case LUI:
        case SLTI:
        case SLTIU:
        case SLL:
        case SRL:
        case SRA:
            return repr[type] + " " + reg(rd) + " " + reg(rs) + " " + _label;
        case BEQ:
        case BNE:
            return repr[type] + " " + reg(rs) + " " + reg(rt) + " " + _label;
        case LW:
            return repr[type] + " " + reg(rd) + " " + _label + "(" + reg(rs) + ")";
        case SW:
            return repr[type] + " " + reg(rt) + " " + _label + "(" + reg(rs) + ")";
        case LI:
        case LA:
            return repr[type] + " " + reg(rd) + " " + _label;
        case BLEZ:
        case BGTZ:
        case BLTZ:
        case BGEZ:
            return repr[type] + " " + reg(rs) + " " + _label;
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

    Mips::Type Mips::get_type(void) const {
        return type;
    }

    Mips::Registers Mips::get_rd(void) const {
        return rd;
    }

    Mips::Registers Mips::get_rs(void) const {
        return rs;
    }

    Mips::Registers Mips::get_rt(void) const {
        return rt;
    }

    std::string Mips::get_label(void) const {
        return _label;
    }

    Mips Mips::j(const std::string &label) {
        return Mips(J, ZERO, ZERO, ZERO, label);
    }

    Mips Mips::jal(const std::string &label) {
        return Mips(JAL, ZERO, ZERO, ZERO, label);
    }

    Mips Mips::label(const std::string &label) {
        return Mips(LABEL, ZERO, ZERO, ZERO, label);
    }

    Mips Mips::syscall() {
        return Mips(SYSCALL);
    }

    Mips Mips::addiu(Registers rd, Registers rs, const std::string &label) {
        return Mips(ADDIU, rd, rs, ZERO, label);
    }

    Mips Mips::subiu(Registers rd, Registers rs, const std::string &label) {
        return Mips(SUBIU, rd, rs, ZERO, label);
    }

    Mips Mips::ori(Registers rd, Registers rs, const std::string &label) {
        return Mips(ORI, rd, rs, ZERO, label);
    }

    Mips Mips::lui(Registers rd, Registers rs, const std::string &label) {
        return Mips(LUI, rd, rs, ZERO, label);
    }

    Mips Mips::beq(Registers rs, Registers rt, const std::string &label) {
        return Mips(BEQ, ZERO, rs, rt, label);
    }

    Mips Mips::bne(Registers rs, Registers rt, const std::string &label) {
        return Mips(BNE, ZERO, rs, rt, label);
    }

    Mips Mips::sll(Registers rd, Registers rs, const std::string &label) {
        return Mips(SLL, rd, rs, ZERO, label);
    }

    Mips Mips::lw(Registers rt, Registers base, const std::string &label) {
        return Mips(LW, rt, base, ZERO, label);
    }

    Mips Mips::sw(Registers rt, Registers base, const std::string &label) {
        return Mips(SW, ZERO, base, rt, label);
    }

    Mips Mips::li(Registers rs, const std::string &label) {
        return Mips(LI, rs, ZERO, ZERO, label);
    }

    Mips Mips::la(Registers rs, const std::string &label) {
        return Mips(LA, rs, ZERO, ZERO, label);
    }

    Mips Mips::blez(Registers rs, const std::string &label) {
        return Mips(BLEZ, ZERO, rs, ZERO, label);
    }

    Mips Mips::bgtz(Registers rs, const std::string &label) {
        return Mips(BGTZ, ZERO, rs, ZERO, label);
    }

    Mips Mips::addu(Registers rd, Registers rs, Registers rt) {
        return Mips(ADDU, rd, rs, rt);
    }

    Mips Mips::subu(Registers rd, Registers rs, Registers rt) {
        return Mips(SUBU, rd, rs, rt);
    }

    Mips Mips::mul(Registers rd, Registers rs, Registers rt) {
        return Mips(MUL, rd, rs, rt);
    }

    Mips Mips::div(Registers rd, Registers rs, Registers rt) {
        return Mips(DIV, rd, rs, rt);
    }

    Mips Mips::slt(Registers rd, Registers rs, Registers rt) {
        return Mips(SLT, rd, rs, rt);
    }

    Mips Mips::sle(Registers rd, Registers rs, Registers rt) {
        return Mips(SLE, rd, rs, rt);
    }

    Mips Mips::seq(Registers rd, Registers rs, Registers rt) {
        return Mips(SEQ, rd, rs, rt);
    }

    Mips Mips::sne(Registers rd, Registers rs, Registers rt) {
        return Mips(SNE, rd, rs, rt);
    }

    Mips Mips::jr(Registers rs) {
        return Mips(JR, ZERO, rs);
    }

    bool Mips::is_jump(void) const {
        switch (type) {
        case J:
        case JAL:
        case SYSCALL:
        case BEQ:
        case BNE:
        case JR:
        case BLEZ:
        case BGTZ:
        case BGEZ:
        case BLTZ:
            return true;
        default:
            return false;
        }
    }

}
