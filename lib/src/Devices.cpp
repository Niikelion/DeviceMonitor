#include <fstream>
#include <DeviceMonitor/Devices.hpp>

namespace DeviceMonitor::Devices {
    MockDevice::MockDevice(
            unsigned long id,
            std::function<Monitor::Status()> const& generator
    ): Device(id), generator(generator) {}
    Monitor::Status MockDevice::fetch_current_state() {
        return std::move(generator());
    }

    FileDevice::FileDevice(
            unsigned long id,
            const std::string &file_name
    ): Device(id), file_name(file_name) {}
    Monitor::Status FileDevice::fetch_current_state() {
        std::ifstream file(file_name, std::ios::in);

        try {
            auto status = extract(file);
            file.close();
            return std::move(status);
        } catch(const std::exception& e) {
            if (file.is_open()) file.close();
            throw e;
        }
    }
}