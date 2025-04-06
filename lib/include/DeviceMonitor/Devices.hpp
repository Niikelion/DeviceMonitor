#pragma once

#include <string>
#include <functional>
#include <fstream>
#include "DeviceMonitor/Device.hpp"
#include "DeviceMonitor/Monitor.hpp"

namespace DeviceMonitor::Devices {
    class MockDevice: public Device {
    public:
        MockDevice(unsigned long id, const std::function<Monitor::Status()> &generator);
        Monitor::Status fetch_current_state() override;
    private:
        std::function<Monitor::Status()> generator;
    };

    class FileDevice: public Device {
    public:
        FileDevice(unsigned long id, const std::string &file_name);
        Monitor::Status fetch_current_state() override;
    protected:
        virtual Monitor::Status extract(std::ifstream&) = 0;
    private:
        std::string file_name;
    };
}