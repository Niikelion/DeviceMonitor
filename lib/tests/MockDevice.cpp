#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <DeviceMonitor/Devices.hpp>
#include <functional>

using namespace DeviceMonitor;

TEST_CASE("MockDevice uses provided function to construct current status", "[unit]") {
    SECTION("returned values are visible from outside and have expected types and values") {
        unsigned long long id = 7;

        Devices::MockDevice static_device = {7, []() -> Monitor::Status {
            return {{
                            {"a", Value{15}},
                            {"b", Value{8}},
                            {"c", Value{5.6f}},
                            {"d", Value{1.3f}},
                            {"e", Value{true}},
                            {"f", Value{false}},
                            {"g", Value{"test"}},
                            {"h", Value{""}}
                    }};
        }};

        CHECK(static_device.get_id() == id);

        auto status = static_device.fetch_current_state();

        using Catch::Matchers::WithinULP;

        REQUIRE(status.size() == 8);
        REQUIRE(status.contains("a"));
        REQUIRE(get_property<int>(status, "a") == 15);
        REQUIRE(status.contains("b"));
        REQUIRE(get_property<int>(status, "b") == 8);
        REQUIRE(status.contains("c"));
        REQUIRE_THAT(get_property<float>(status, "c"), WithinULP(5.6f, 0));
        REQUIRE(status.contains("d"));
        REQUIRE_THAT(get_property<float>(status, "d"), WithinULP(1.3f, 0));
        REQUIRE(status.contains("e"));
        REQUIRE(get_property<bool>(status, "e") == true);
        REQUIRE(status.contains("f"));
        REQUIRE(get_property<bool>(status, "f") == false);
        REQUIRE(status.contains("g"));
        REQUIRE(get_property<std::string>(status, "g") == "test");
        REQUIRE(status.contains("h"));
        REQUIRE(get_property<std::string>(status, "h").empty());
    }

    SECTION("passed function is run every time fetch_current_state is called") {
        int value = 5;

        Devices::MockDevice dynamic_device = {9, [&value]() -> Monitor::Status {
            return {{
                            {"value", Value{value}}
                    }};
        }};

        auto status = dynamic_device.fetch_current_state();

        REQUIRE(get_property<int>(status, "value") == value);

        value = 19;
        status = dynamic_device.fetch_current_state();

        REQUIRE(get_property<int>(status, "value") == value);
    }
}