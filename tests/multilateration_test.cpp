#include "multilateration.h"

#include <nlohmann/json.hpp>

#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

bool closeEnough(double actual, double expected) {
    return std::abs(actual - expected) <= 1e-9;
}

}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: multilateration_test FIXTURES\n";
        return 2;
    }

    std::ifstream input(argv[1]);
    nlohmann::json cases;
    input >> cases;
    int failures = 0;

    for (const auto& testCase : cases) {
        std::vector<double> offsets = testCase.value("random_offsets", std::vector<double>{});
        std::size_t offsetIndex = 0;
        RandomOffset randomOffset;
        if (!offsets.empty()) {
            randomOffset = [&offsets, &offsetIndex]() {
                return offsets.at(offsetIndex++);
            };
        }

        MultilaterationResult actual = calculateSerialLine(testCase["input"], randomOffset);
        const auto& expected = testCase["expected"];
        bool passed = std::string(tableSideName(actual.side)) == expected["side"].get<std::string>()
            && actual.valid == expected["valid"].get<bool>()
            && closeEnough(actual.x, expected["x"].get<double>())
            && closeEnough(actual.y, expected["y"].get<double>());
        if (!passed) {
            ++failures;
            std::cerr << testCase["input"].get<std::string>() << "\n"
                << "  expected " << expected.dump() << "\n"
                << "  actual   {\"side\":\"" << tableSideName(actual.side)
                << "\",\"x\":" << actual.x << ",\"y\":" << actual.y
                << ",\"valid\":" << std::boolalpha << actual.valid << "}\n";
        }
    }

    const std::vector<std::string> invalidLines = {
        "", "A 1 2 3", "B 1 2 3 4 5", "C 1 2 3", "D 1 100", "A one 2 3 4",
    };
    for (const std::string& line : invalidLines) {
        try {
            calculateSerialLine(line);
            ++failures;
            std::cerr << "expected invalid line to fail: " << line << "\n";
        } catch (const std::exception&) {
        }
    }

    if (failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    std::cout << cases.size() + invalidLines.size() << " tests passed\n";
}
