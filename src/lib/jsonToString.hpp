#pragma once

#include "json.hpp"

std::string jsonToString(const nlohmann::json& j) {
    std::stringstream ss;
    ss << j;
    return ss.str();
}