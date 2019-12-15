#pragma once
#include <map>
#include <vector>
#include <stack>
#include <string>
#include <queue>
#include "Quaternary.h"

namespace dyj {
    class BladeRunner {
    public:
        BladeRunner() = delete;
        BladeRunner(const IrList &_irs);

        void check_and_run(void);
        IrList &&dump(void);

    private:
        bool check_runnable(void);
        void run(void);

    private:
        int use(const std::string &var);
        void def(const std::string &var, int v);
        std::string arr2var(const std::string &arr, int idx);
        size_t label2addr(const std::string &var);
        void push_arg(int arg);
        int pop_arg(void);
        void push_layer(size_t pc_old);
        size_t pop_layer(void);
        
    private:
        const IrList &irs;
        IrList outs;
        size_t entry;
        size_t pc;
        std::queue<int> args;
        int ret;
        std::stack<size_t> ra;
        std::vector<std::map<std::string, int> > vars;
        std::map<std::string, int> global_vars;
        std::map<std::string, size_t> labels;
    };
}
