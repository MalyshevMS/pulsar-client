#pragma once

#include "json.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

std::string jsonToString(const nlohmann::json& j) {
    std::stringstream ss;
    ss << j;
    return ss.str();
}