#pragma once

#include <string>
#include <variant>
#include <optional>

namespace DeviceMonitor {
    typedef std::variant<float, int, bool, std::string> Value;
}