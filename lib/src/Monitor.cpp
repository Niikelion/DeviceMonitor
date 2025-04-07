#include <chrono>
#include "DeviceMonitor/Monitor.hpp"

namespace DeviceMonitor {
    Monitor::Monitor(float pooling_interval): pooling_interval(pooling_interval), nextId() {}
    void Monitor::start() {
        auto guard = std::lock_guard(infrequent_operations_mutex);
        // early return when thread was already started
        if (is_running.exchange(true)) return;
        // forget all previous data
        statuses.store(std::shared_ptr<StatusMap>());
        // start the thread
        pooling_thread = std::thread(&Monitor::pooling_loop, this);
    }
    void Monitor::stop() {
        auto guard = std::lock_guard(infrequent_operations_mutex);
        // early return when thread was already stopped
        if (!is_running.exchange(false)) return;
        // wait for the end of the thread
        if (pooling_thread.joinable()) pooling_thread.join();
    }
    std::shared_ptr<const Monitor::StatusMap> Monitor::get_statuses() {
        return statuses.load();
    }

    void Monitor::pooling_loop() {
        while (is_running.load()) {
            // capture start of the data pooling
            auto next_update = std::chrono::steady_clock::now() + std::chrono::duration<float, std::chrono::seconds::period>(pooling_interval);
            // create snapshot object
            auto new_statuses = std::make_shared<StatusMap>();
            auto& status_map = *new_statuses;
            // fetch data and store in the new snapshot
            for (const auto& device : devices) {
                try {
                    status_map[device.first] = device.second->fetch_current_state();
                }  catch (...) {
                    status_map.erase(device.first);
                }
            }
            // replace current snapshot with the new one
            statuses.store(new_statuses);
            // capture end of pooling
            auto now = std::chrono::steady_clock::now();
            auto diff = next_update - now;
            // if took less than expected, sleep
            if (diff.count() >= 0) std::this_thread::sleep_until(next_update);
        }
    }
}