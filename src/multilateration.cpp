#include "multilateration.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace {

struct Point {
    double x;
    double y;
};

constexpr std::array<Point, 4> farSensors = {{{1.37, 0.16}, {0.16, 0.16}, {0.75, 0.75}, {0.75, 1.37}}};
constexpr std::array<Point, 4> nearSensors = {{{0.16, 2.63}, {1.37, 2.63}, {0.75, 2.04}, {0.75, 1.41}}};
constexpr double tableWidth = 1.584;
constexpr double tableLength = 3.24;

std::vector<std::string> split(const std::string& line) {
    std::istringstream stream(line);
    std::vector<std::string> values;
    std::string value;
    while (stream >> value) {
        values.push_back(value);
    }
    return values;
}

double parseDouble(const std::string& value) {
    std::size_t consumed = 0;
    double parsed = std::stod(value, &consumed);
    if (consumed != value.size()) {
        throw std::invalid_argument("invalid sensor time");
    }
    return parsed;
}

int parseInt(const std::string& value) {
    std::size_t consumed = 0;
    int parsed = std::stoi(value, &consumed);
    if (consumed != value.size()) {
        throw std::invalid_argument("invalid sensor value");
    }
    return parsed;
}

double squared(Point point) {
    return point.x * point.x + point.y * point.y;
}

bool hasRankTwo(const std::array<std::array<double, 2>, 3>& matrix) {
    double scale = 0;
    for (const auto& row : matrix) {
        scale = std::max(scale, std::max(std::abs(row[0]), std::abs(row[1])));
    }
    double tolerance = 3 * std::numeric_limits<double>::epsilon() * scale * scale;
    for (std::size_t i = 0; i < matrix.size(); ++i) {
        for (std::size_t j = i + 1; j < matrix.size(); ++j) {
            double determinant = matrix[i][0] * matrix[j][1] - matrix[i][1] * matrix[j][0];
            if (std::abs(determinant) > tolerance) {
                return true;
            }
        }
    }
    return false;
}

Point multilaterate(const std::array<double, 4>& sensorTimes, const std::array<Point, 4>& sensors, double speed) {
    auto closest = std::min_element(sensorTimes.begin(), sensorTimes.end());
    std::size_t c = std::distance(sensorTimes.begin(), closest);

    std::array<double, 4> deltaTimes;
    for (std::size_t i = 0; i < sensorTimes.size(); ++i) {
        deltaTimes[i] = sensorTimes[i] - *closest;
    }

    std::vector<std::size_t> sensorIndexes;
    for (std::size_t i = 0; i < sensors.size(); ++i) {
        if (i != c) {
            sensorIndexes.push_back(i);
        }
    }

    std::array<std::array<double, 2>, 3> a = {};
    std::array<double, 3> b = {};
    std::size_t row = 0;
    bool rankTwo = false;

    for (std::size_t i : sensorIndexes) {
        for (std::size_t j : sensorIndexes) {
            if (row >= a.size()) {
                throw std::runtime_error("sensor times do not produce a two-dimensional solution");
            }

            a[row][0] = 2 * (speed * deltaTimes[j] * (sensors[i].x - sensors[c].x)
                - speed * deltaTimes[i] * (sensors[j].x - sensors[c].x));
            a[row][1] = 2 * (speed * deltaTimes[j] * (sensors[i].y - sensors[c].y)
                - speed * deltaTimes[i] * (sensors[j].y - sensors[c].y));
            b[row] = speed * deltaTimes[i] * (speed * speed * deltaTimes[j] * deltaTimes[j] - squared(sensors[j]))
                + (speed * deltaTimes[i] - speed * deltaTimes[j]) * squared(sensors[c])
                + speed * deltaTimes[j] * (squared(sensors[i]) - speed * speed * deltaTimes[i] * deltaTimes[i]);

            rankTwo = hasRankTwo(a);
            if (rankTwo) {
                break;
            }
            ++row;
        }
        if (rankTwo) {
            break;
        }
    }

    double aa = 0;
    double ab = 0;
    double bb = 0;
    double ax = 0;
    double bx = 0;
    for (std::size_t i = 0; i < a.size(); ++i) {
        aa += a[i][0] * a[i][0];
        ab += a[i][0] * a[i][1];
        bb += a[i][1] * a[i][1];
        ax += a[i][0] * b[i];
        bx += a[i][1] * b[i];
    }

    double determinant = aa * bb - ab * ab;
    if (determinant == 0) {
        throw std::runtime_error("sensor times do not produce a two-dimensional solution");
    }
    return {(ax * bb - bx * ab) / determinant, (bx * aa - ax * ab) / determinant};
}

double defaultRandomOffset() {
    static thread_local std::mt19937 generator(std::random_device{}());
    static thread_local std::uniform_real_distribution<double> distribution(-0.15, 0.15);
    return distribution(generator);
}

}

MultilaterationResult calculateSerialLine(const std::string& line, RandomOffset randomOffset) {
    std::vector<std::string> values = split(line);
    if (values.empty() || values[0].size() != 1 || values[0].find_first_not_of("ABCD") != std::string::npos) {
        throw std::invalid_argument("expected an A, B, C, or D serial line");
    }

    char lineType = values[0][0];
    if (lineType == 'A' || lineType == 'B') {
        if (values.size() != 5) {
            throw std::invalid_argument("expected four sensor times");
        }
        std::array<double, 4> sensorTimes;
        for (std::size_t i = 0; i < sensorTimes.size(); ++i) {
            sensorTimes[i] = parseDouble(values[i + 1]) / 1000000;
        }
        TableSide side = lineType == 'A' ? TableSide::Near : TableSide::Far;
        Point position = side == TableSide::Near
            ? multilaterate(sensorTimes, nearSensors, 470)
            : multilaterate(sensorTimes, farSensors, 322);
        bool valid = position.x > 0 && position.x < tableWidth && position.y > 0 && position.y < tableLength;
        return {side, position.x, position.y, valid};
    }

    if (values.size() < 3 || values.size() % 2 == 0) {
        throw std::invalid_argument("expected sensor ID and time pairs");
    }

    int firstSensor = lineType == 'C' ? 1 : 5;
    int closestSensor = 0;
    int closestTime = std::numeric_limits<int>::max();
    for (std::size_t i = 1; i < values.size(); i += 2) {
        int sensor = parseInt(values[i]);
        int time = parseInt(values[i + 1]);
        if (sensor < firstSensor || sensor >= firstSensor + 4) {
            throw std::invalid_argument("sensor ID does not match side");
        }
        if (time < closestTime) {
            closestSensor = sensor;
            closestTime = time;
        }
    }

    const auto& sensors = lineType == 'C' ? nearSensors : farSensors;
    Point sensor = sensors[closestSensor - firstSensor];
    RandomOffset offset = randomOffset ? randomOffset : defaultRandomOffset;
    return {lineType == 'C' ? TableSide::Near : TableSide::Far, sensor.x + offset(), sensor.y + offset(), false};
}

const char* tableSideName(TableSide side) {
    return side == TableSide::Near ? "near" : "far";
}
