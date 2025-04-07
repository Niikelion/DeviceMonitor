#include <catch2/matchers/catch_matchers_all.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/catch_test_macros.hpp>
#include <DeviceMonitor/Devices.hpp>
#include <functional>
#include <atomic>
#include <thread>
#include <vector>

using namespace DeviceMonitor;

void wait_for_seconds(float seconds) {
    std::this_thread::sleep_for(std::chrono::duration<float, std::chrono::seconds::period>(seconds));
}

std::chrono::duration<float> measure(const std::function<void()> &computation) {
    auto start = std::chrono::steady_clock::now();
    computation();
    auto end = std::chrono::steady_clock::now();
    return end - start;
}

TEST_CASE("Monitor can be created and devices can be registered without collisions", "[unit]") {
    Monitor monitor{0.1f};

    auto &dev1 = monitor.register_device<Devices::MockDevice>([]() -> Monitor::Status {
        return {{
                        {"some_key", Value{4}}
                }};
    });

    auto &dev2 = monitor.register_device<Devices::MockDevice>([]() -> Monitor::Status {
        return {{
                        {"some_other_key", Value{"test"}}
                }};
    });

    REQUIRE(dev1.get_id() != dev2.get_id());

    auto status1 = dev1.fetch_current_state();

    REQUIRE(status1.size() == 1);
    REQUIRE(status1.contains("some_key"));
    REQUIRE(get_property<int>(status1, "some_key") == 4);

    auto status2 = dev2.fetch_current_state();

    REQUIRE(status2.size() == 1);
    REQUIRE(status2.contains("some_other_key"));
    REQUIRE(get_property<std::string>(status2, "some_other_key") == "test");
}

TEST_CASE("Monitor can be started and stopped multiple times without error","[unit]") {
    Monitor monitor{0.1f};

    SECTION("green path - single start and end") {
        REQUIRE_NOTHROW(monitor.start());
        REQUIRE_NOTHROW(monitor.stop());
    }

    SECTION("multiple starts and ends, count does not match") {
        REQUIRE_NOTHROW([&]() {
            for (int i = 0; i < 3; ++i) monitor.start();
        }());
        REQUIRE_NOTHROW([&]() {
            for (int i = 0; i < 2; ++i) monitor.stop();
        }());
    }

    SECTION("sequence of paired starts and ends") {
        REQUIRE_NOTHROW([&]() {
            for (int i = 0; i < 3; ++i) {
                monitor.start();
                monitor.stop();
            }
        });
    }
}

TEST_CASE("Monitor fetches the data at least one in 2x time of the interval", "[unit]") {
    float interval = GENERATE(0.1f, 0.2f, 0.5f);
    Monitor monitor{interval};
    auto &mock = monitor.register_device<Devices::MockDevice>([]() -> Monitor::Status {
        return {{
                        {"value", Value{20}}
                }};
    });

    monitor.start();
    wait_for_seconds(2 * interval);
    auto status = monitor.get_statuses();
    monitor.stop();

    REQUIRE(status->contains(mock.get_id()));

    auto mockStatus = status->at(mock.get_id());

    REQUIRE(mockStatus.size() == 1);
    REQUIRE(mockStatus.contains("value"));
    REQUIRE(get_property<int>(mockStatus, "value") == 20);
}

TEST_CASE("Monitor resets the status of the device when it throws error during data pooling", "[unit]") {
    float interval = 0.2f;
    Monitor monitor { interval };

    std::atomic<bool> shouldCrash = false;

    auto& mock = monitor.register_device<Devices::MockDevice>([&]() -> Monitor::Status {
        if (shouldCrash.load()) throw std::exception("Forced crash");

        return {{
                        {"value", Value{20}}
                }};
    });

    monitor.start();
    wait_for_seconds(2 * interval);
    auto status = monitor.get_statuses();

    REQUIRE(status->contains(mock.get_id()));

    auto mockStatus = status->at(mock.get_id());

    REQUIRE(mockStatus.size() == 1);

    shouldCrash = true;
    wait_for_seconds(2 * interval);
    status = monitor.get_statuses();

    REQUIRE_FALSE(status->contains(mock.get_id()));

    monitor.stop();
}

TEST_CASE("Monitor does not perform pooling more frequent than interval", "[unit]") {
    float interval = 0.1f;
    Monitor monitor { interval };

    std::atomic<int> counter = 0;

    monitor.register_device<Devices::MockDevice>([&]() -> Monitor::Status {
        counter.fetch_add(1);
        return {};
    });

    monitor.start();
    auto duration = measure([interval]() { wait_for_seconds(20 * interval); });
    monitor.stop();

    REQUIRE(counter.load() <= duration.count() / interval + 1);
}

TEST_CASE("Monitor runs calculations once an interval assuming fetching state takes less time than interval", "[unit]") {
    float interval = 0.2f;
    Monitor monitor { interval };

    std::atomic<int> counter = 0;

    monitor.register_device<Devices::MockDevice>([&]() -> Monitor::Status {
        counter.fetch_add(1);
        return {};
    });

    monitor.start();
    auto duration = measure([interval]() { wait_for_seconds(20 * interval); });
    monitor.stop();

    REQUIRE(counter.load() + 1 >= duration.count() / interval);
}

TEST_CASE("Monitor pools on separate thread than start was called", "[unit]") {
    float interval = 0.1f;
    Monitor monitor { interval };

    std::atomic<std::thread::id> thread_id;

    monitor.register_device<Devices::MockDevice>([&]() -> Monitor::Status {
        thread_id.store(std::this_thread::get_id());
        return {};
    });

    monitor.start();
    wait_for_seconds(2 * interval);
    monitor.stop();

    REQUIRE(thread_id.load() != std::this_thread::get_id());
}

TEST_CASE("get_statuses is thread-safe and can be called any time, even when monitor is not active") {
    float interval = 0.1f;
    Monitor monitor{interval};
    int num_threads = 10;

    std::vector<std::thread> read_threads(num_threads);
    std::atomic<int> counter;

    for (auto& thread : read_threads) {
        thread = std::thread([&]() {
            auto status = monitor.get_statuses();
            if (!status->empty()) counter.fetch_add(1);
        });
    }

    wait_for_seconds(10 * interval);

    monitor.register_device<Devices::MockDevice>([&]() -> Monitor::Status {
        return {{
                        {"value", Value{ "test" }}
                }};
    });

    monitor.start();
    wait_for_seconds(10 * interval);
    monitor.stop();
    wait_for_seconds(10 * interval);

    for (auto& thread : read_threads)
        if (thread.joinable()) thread.join();

    REQUIRE(counter.load() > 0);
}