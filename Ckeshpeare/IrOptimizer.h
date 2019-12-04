#pragma once
#include <vector>
#include "Quaternary.h"

namespace dyj {
    class CodeBlock {
    public:
        CodeBlock();

        // construction
        void append(const Quaternary &ir);
        void add_edge(CodeBlock *);

        // optimization
        void dag_optimize(void);

        // output
        void dump_over(std::vector<Quaternary> &ret) const;
        void dump_append(std::vector<Quaternary> &ret) const;

    private:
        std::vector<Quaternary> irs;
        std::vector<CodeBlock *> nexts;

    private:
        struct node {
            size_t id;
            std::vector<node *> to;
        };

    private:
        friend class FlowGraph;
    };

    class FlowGraph {
    public:
        FlowGraph();

        void load_and_cut(const std::vector<Quaternary> &irs);

        std::vector<Quaternary> dump(void) const;

    private:
        std::vector<CodeBlock *> blocks;
    };

    bool ir_peek_hole(std::vector<Quaternary> &irs);

}

