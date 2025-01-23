#ifndef BAR_H
#define BAR_H

#include <string>
#include <nlohmann/json.hpp>

using nlohmann::json;

class Bar {
public:
    std::string T;
    std::string S;
    double o = 0.0;
    double h = 0.0;
    double l = 0.0;
    double c = 0.0;
    double v = 0.0;
    std::string t;
    double n = 0.0;
    double vw = 0.0;

    Bar() = default;

    Bar(const json& j) {
        if (j.contains("T") && j["T"].is_string()) { T = j["T"]; }
        if (j.contains("S") && j["S"].is_string()) { S = j["S"]; }
        if (j.contains("o") && j["o"].is_number()) { o = j["o"]; }
        if (j.contains("h") && j["h"].is_number()) { h = j["h"]; }
        if (j.contains("l") && j["l"].is_number()) { l = j["l"]; }
        if (j.contains("c") && j["c"].is_number()) { c = j["c"]; }
        if (j.contains("v") && j["v"].is_number()) { v = j["v"]; }
        if (j.contains("t") && j["t"].is_string()) { t = j["t"]; }
        if (j.contains("n") && j["n"].is_number()) { n = j["n"]; }
        if (j.contains("vw") && j["vw"].is_number()) { vw = j["vw"]; }
    }
};

#endif
