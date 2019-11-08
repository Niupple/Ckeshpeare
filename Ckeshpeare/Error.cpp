#include "Error.h"

namespace dyj {
    Error::Error(Type _type, Location _location) : type(_type), location(_location) {}

    Error::Type Error::get_type() {
        return type;
    }

    std::string Error::to_string() {
#ifdef _DEBUG
        return std::to_string(location.first) + " " + std::to_string(location.second) + " " + (char)type;
#else
        return std::to_string(location.first) + " " + (char)type;
#endif
    }

}
