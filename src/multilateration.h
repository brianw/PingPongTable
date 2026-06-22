#pragma once

#include <functional>
#include <string>

enum class TableSide {
    Near,
    Far,
};

struct MultilaterationResult {
    TableSide side;
    double x;
    double y;
    bool valid;
};

using RandomOffset = std::function<double()>;

MultilaterationResult calculateSerialLine(const std::string& line, RandomOffset randomOffset = {});
const char* tableSideName(TableSide side);
