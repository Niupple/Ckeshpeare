#pragma once
#include <string>
#include <vector>

namespace dyj {
    class Mips {
    public:
        enum Registers {
            ZERO,
            AT,
            V0,
            V1,
            A0,
            A1,
            A2,
            A3,
            T0,
            T1,
            T2,
            T3,
            T4,
            T5,
            T6,
            T7,
            S0,
            S1,
            S2,
            S3,
            S4,
            S5,
            S6,
            S7,
            T8,
            T9,
            K0,
            K1,
            GP,
            SP,
            FP,
            RA
        };

        enum Type {
            J,
            JAL,
            LABEL,
            SYSCALL,
            ADDIU,
            SUBIU,
            ANDI,
            ORI,
            XORI,
            LUI,
            SLTI,
            SLTIU,
            BEQ,
            BNE,
            BLEZ,
            BGTZ,
            BLTZ,
            BGEZ,
            LW,
            SW,
            LI,
            LA,
            SLL,
            SRL,
            SRA,
            ADDU,
            SUBU,
            MULT,
            MUL,
            DIV,
            SLT,
            SLE,
            SEQ,
            SNE,
            SLLV,
            SRLV,
            SRAV,
            AND,
            OR,
            XOR,
            NOR,
            MFHI,
            MFLO,
            MTHI,
            MTLO,
            JR,
        };

        Mips();
        Mips(Type _type, Registers rd = ZERO, Registers rs = ZERO, Registers rt = ZERO, const std::string &__label = "");
        static Registers args(size_t i);
        std::string to_string(void) const;

        Mips::Type get_type(void) const;
        Mips::Registers get_rd(void) const;
        Mips::Registers get_rs(void) const;
        Mips::Registers get_rt(void) const;
        std::string get_label(void) const;

    public:
        static Mips j(const std::string &label);
        static Mips jal(const std::string &label);
        static Mips label(const std::string &label);
        static Mips syscall();
        static Mips addiu(Registers rd, Registers rs, const std::string &label);
        static Mips subiu(Registers rd, Registers rs, const std::string &label);
        static Mips andi(Registers rd, Registers rs, const std::string &label);
        static Mips ori(Registers rd, Registers rs, const std::string &label);
        static Mips xori(Registers rd, Registers rs, const std::string &label);
        static Mips lui(Registers rd, Registers rs, const std::string &label);
        static Mips slti(Registers rd, Registers rs, const std::string &label);
        static Mips sltiu(Registers rd, Registers rs, const std::string &label);
        static Mips beq(Registers rs, Registers rt, const std::string &label);
        static Mips bne(Registers rs, Registers rt, const std::string &label);
        static Mips sll(Registers rd, Registers rs, const std::string &label);
        static Mips srl(Registers rd, Registers rs, const std::string &label);
        static Mips sra(Registers rd, Registers rs, const std::string &label);
        static Mips lw(Registers rt, Registers base, const std::string &label);
        static Mips sw(Registers rt, Registers base, const std::string &label);
        static Mips li(Registers rs, const std::string &label);
        static Mips la(Registers rs, const std::string &label);
        static Mips blez(Registers rs, const std::string &label);
        static Mips bgtz(Registers rs, const std::string &label);
        static Mips bltz(Registers rs, const std::string &label);
        static Mips bgez(Registers rs, const std::string &label);
        static Mips addu(Registers rd, Registers rs, Registers rt);
        static Mips subu(Registers rd, Registers rs, Registers rt);
        static Mips mult(Registers rd, Registers rs, Registers rt);
        static Mips mul(Registers rd, Registers rs, Registers rt);
        static Mips div(Registers rd, Registers rs, Registers rt);
        static Mips slt(Registers rd, Registers rs, Registers rt);
        static Mips sle(Registers rd, Registers rs, Registers rt);
        static Mips seq(Registers rd, Registers rs, Registers rt);
        static Mips sne(Registers rd, Registers rs, Registers rt);
        static Mips sllv(Registers rd, Registers rs, Registers rt);
        static Mips srlv(Registers rd, Registers rs, Registers rt);
        static Mips srav(Registers rd, Registers rs, Registers rt);
        static Mips And(Registers rd, Registers rs, Registers rt);
        static Mips Or(Registers rd, Registers rs, Registers rt);
        static Mips Xor(Registers rd, Registers rs, Registers rt);
        static Mips nor(Registers rd, Registers rs, Registers rt);
        static Mips jr(Registers rs);

    public:
        bool is_jump(void) const;

    private:
        Type type;
        Registers rs, rt, rd;
        std::string _label;
        static std::vector<std::string> repr;
        static std::string reg(Registers r);
    };

    typedef Mips Jump;
    typedef Mips Immediate;
    typedef Mips Register;

}
