#include "DeviceMonitor/Device.hpp"

namespace DeviceMonitor {
    unsigned long Device::get_id() const {
        return id;
    }

    Device::Device(unsigned long id) : id(id) {}
}