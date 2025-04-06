#include <DeviceMonitor/Monitor.hpp>
#include <DeviceMonitor/Devices.hpp>
#include <nlohmann/json.hpp>
#include <thread>
#include <typeindex>
#include <iostream>
#include <unordered_map>

using Json = nlohmann::json;

class JsonFileDevice: public DeviceMonitor::Devices::FileDevice {
public:
    JsonFileDevice(
            unsigned long id,
            const std::string &file_name,
            const std::unordered_map<std::string, std::type_index> &properties
    ): DeviceMonitor::Devices::FileDevice(id, file_name), properties(properties) {}
protected:
    DeviceMonitor::Monitor::Status extract(std::ifstream &file) override {
        Json data = Json::parse(file);
        DeviceMonitor::Monitor::Status status;

        if (!data.is_object()) return status;

        for (const auto& property : properties) {
            if (!data.contains(property.first)) throw std::exception("Missing property");

            if (property.second == typeid(float)) status[property.first] = DeviceMonitor::Value(static_cast<float>(data[property.first]));
            if (property.second == typeid(int)) status[property.first] = DeviceMonitor::Value(static_cast<int>(data[property.first]));
            if (property.second == typeid(bool)) status[property.first] = DeviceMonitor::Value(static_cast<bool>(data[property.first]));
            if (property.second == typeid(std::string)) status[property.first] = DeviceMonitor::Value(static_cast<std::string>(data[property.first]));
        }

        return status;
    }
private:
    std::unordered_map<std::string, std::type_index> properties;
};

int main(int argc, char* argv[]) {
    DeviceMonitor::Monitor monitor = {0.5 };
    const auto& lamp = monitor.register_device<JsonFileDevice>("lamp.json", std::unordered_map<std::string, std::type_index> {
        { "voltage", typeid(float) },
        { "current", typeid(float) }
    });
    unsigned long lamp_id = lamp.get_id();

    monitor.start();
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        auto status = monitor.get_statuses();
        if (!status) {
            std::cout << "Waiting for status...\n";
            continue;
        }
        if (!status->contains(lamp_id)) {
            std::cout << "Waiting for lamp...\n";
            continue;
        }

        auto lamp_status = status->at(lamp_id);

        auto voltage = DeviceMonitor::get_property<float>(lamp_status, "voltage");
        auto current = DeviceMonitor::get_property<float>(lamp_status, "current");

        std::cout << "voltage: " << voltage << " current: " << current << "\n";
    }
    monitor.stop();

    return 0;
}