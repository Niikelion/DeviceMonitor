#pragma once

#include <mutex>
#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <concepts>
#include <unordered_map>
#include "DeviceMonitor/Device.hpp"

namespace DeviceMonitor {
    template<class T, class U>
    concept Derived = std::is_base_of<U, T>::value;

    class Monitor {
    private:
    public:
        using Status = std::unordered_map<std::string, Value>;
        using StatusMap = std::unordered_map<unsigned long, Status>;

        Monitor(float pooling_interval);

        void start();
        void stop();
        std::shared_ptr<const StatusMap> get_statuses();

        template<Derived<Device> T, typename ...Args> T& register_device(Args&&... args) {
            auto guard = std::lock_guard(infrequent_operations_mutex);
            // obtain next id
            auto id = nextId++;
            // construct and return device representation
            auto device = std::make_unique<T>(id, std::forward<Args>(args)...);
            auto& device_ref = *device;
            devices.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(device_ref.get_id()),
                    std::forward_as_tuple(std::move(device))
            );
            return device_ref;
        }

    private:
        float pooling_interval;
        // mutex that ensures that less frequent operations(stop, start and register_device) are thread-safe
        unsigned long nextId;
        std::mutex infrequent_operations_mutex;
        std::atomic<bool> is_running = false;
        std::thread pooling_thread;
        std::unordered_map<unsigned long, std::unique_ptr<Device>> devices;
        // NOTE: performance can be improved by pooling responses
        // NOTE: to do so we need to wrap StatusMap in the struct and when last shared_ptr is deleted, return StatusMap into the pool.
        std::atomic<std::shared_ptr<StatusMap>> statuses;
        void pooling_loop();
    };

    template<typename T> T get_property(const Monitor::Status &status, const std::string &name) noexcept(false) {
        return std::get<T>(status.at(name));
    }
}