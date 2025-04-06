#pragma once

#include <unordered_map>
#include <string>
#include "DeviceMonitor/Value.hpp"

namespace DeviceMonitor {
    class Device {
    public:
        [[nodiscard]] unsigned long get_id() const;
        virtual std::unordered_map<std::string, Value> fetch_current_state() = 0;
        virtual ~Device() = default;
    protected:
        Device(unsigned long id);
    private:
        unsigned long id;
    };
}