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

        Mips();
        virtual std::string to_string(void) const = 0;

    protected:
        static std::string reg(Registers r);
    };

    class Jump : public Mips {
    public:
        enum Type {
            J,
            JAL,
            LABEL,
            SYSCALL,
        };
        Jump() = delete;
        Jump(Type _type, const std::string &_label = "");
        std::string to_string(void) const override;

    private:
        static std::vector<std::string> repr;
        Type type;
        std::string label;
    };

    class Immediate : public Mips {
    public:
        enum Type {
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
        };

        Immediate() = delete;
        Immediate(Type _type, Registers _rs, Registers _rt, const std::string &_label);
        std::string to_string(void) const override;

    private:
        static std::vector<std::string> repr;
        std::string label;
        Type type;
        Registers rs, rt;
    };

    class Register : public Mips {
    public:
        enum Type {
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
        };

        Register() = delete;
        Register(Type _type, Registers rd, Registers rs, Registers rt);
        std::string to_string(void) const override;

    private:
        static std::vector<std::string> repr;
        Type type;
        Registers rs, rt, rd;
    };
}
