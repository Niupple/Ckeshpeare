#include "FullHouse.h"

namespace dyj {
    FullHouse::FullHouse(const std::string &_name, CalType _cal, BehType _beh, ConstType _const, const std::string &_value) : arr_size(0), name(_name), value(_value), calculation_type(_cal), behavioral_type(_beh), const_type(_const) {}

    void FullHouse::set_size(size_t _size) {
        arr_size = _size;
    }

    void FullHouse::append_param(CalType _param) {
        params.push_back(_param);
        arr_size = params.size();
    }

    FullHouse::CalType FullHouse::get_calculation() const {
        return calculation_type;
    }

    FullHouse::BehType FullHouse::get_behavior() const {
        return behavioral_type;
    }

    FullHouse::ConstType FullHouse::get_constant() const {
        return const_type;
    }

    size_t FullHouse::get_array_size() const {
        return arr_size;
    }

    std::string FullHouse::get_name() const {
        return name;
    }

    std::string FullHouse::get_value() const {
        return value;
    }

    bool FullHouse::same_params(const std::vector<CalType> &_params) const {
        return params == _params;
    }

}