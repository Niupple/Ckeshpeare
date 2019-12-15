#pragma once
#include <vector>
#include <set>
#include <map>
#include "Quaternary.h"
#include "Namer.h"

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
        void dump_append(std::vector<Quaternary> &ret, int block_id = -1) const;

    private:
        void get_use_def(void);

    private:
        int block_id = 0;
        std::vector<Quaternary> irs;
        std::vector<CodeBlock *> forward, backward;
        std::set<std::string> use, def;

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
        void flow_living_vars(void);
        void get_living_vars(void);
        void collision_analysis(void);

        std::vector<Quaternary> dump(void) const;
        std::vector<std::set<std::string> > &&get_living_var_list(void);
        std::vector<std::set<std::string> > get_living_in_list(void);
        std::vector<std::set<std::string> > get_living_out_list(void);
        std::set<std::pair<std::string, std::string> > &&get_collisions(void);

    private:
        void link(void);

        void confirm(void);
        void suspect(const std::string &a, const std::string &b);
        void dfs(CodeBlock *block, size_t id, const std::string &var);
        bool visit(CodeBlock *block, size_t id);
        void clear_for_new_var(void);

    private:
        size_t entry_block;
        std::vector<CodeBlock *> blocks;
        std::vector<std::set<std::string> > living_vars;
        std::map<CodeBlock *, std::set<std::string> > in_living, out_living;
        std::map<std::string, CodeBlock *> label_to_block;
        std::set<std::pair<CodeBlock *, size_t> > visited;
        std::set<std::pair<std::string, std::string> > collisions, suspects;
    };


    class Inliner {
    public:
        Inliner() = delete;
        Inliner(Namer *_label_namer, Namer *_var_namer);

        void load_and_parse(const IrList &_irs);
        IrList &&dump(void);

        ~Inliner();

    private:

        void parse_functions(void);
        void apply_all(void);
        void apply_inline(IrList &irs, const IrList &old);

        struct Function {
            std::string name;
            std::vector<std::string> params;
            IrList body;
            bool inlinable = true;

            void dump(IrList &output);
        };

        Function *get_function(const std::string &name);
        void inline_here(IrList &output, Function *func, const std::vector<std::string> &args);
        std::string translate_var(const std::string &var);
        std::string translate_label(const std::string &label);

        std::vector<Function *> functions;
        IrList irs;
        IrList main_function;
        IrList output;
        Namer *label_namer, *var_namer;

        size_t namer_counter;
        std::map<std::string, std::string> new_var, new_label;
    };

    bool ir_peek_hole(std::vector<Quaternary> &irs);

}

