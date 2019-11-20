#pragma once
#include <vector>
#include <string>

namespace dyj {
    class FullHouse {
    public:
        enum CalType {
            VOID,
            INT,
            CHAR
        };

        enum BehType {
            ILL,
            VARIABLE,
            FUNCTION,
            ARRAY
        };

        enum ConstType {
            CONST,
            NON_CONST
        };

        FullHouse() = delete;
        FullHouse(const std::string &_name, CalType _cal, BehType _beh, ConstType _const, const std::string &_value = "");

        void set_size(size_t _size);
        void append_param(CalType _param);

        CalType get_calculation() const;
        BehType get_behavior() const;
        ConstType get_constant() const;
        size_t get_array_size() const;
        std::string get_name() const;
        std::string get_value() const;

        bool same_params(const std::vector<CalType> &params) const;

    private:
        size_t arr_size;
        std::vector<CalType> params;
        std::string name;
        std::string value;
        CalType calculation_type;
        BehType behavioral_type;
        ConstType const_type;
    };
}