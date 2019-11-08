#pragma once
#include <vector>
#include <string>
#include "Error.h"

namespace dyj {
    typedef std::vector<Error *> ErrorRecorder;

    extern ErrorRecorder rerr;
}
