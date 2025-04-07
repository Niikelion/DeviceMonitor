#include <catch2/catch_test_macros.hpp>
#include <DeviceMonitor/Devices.hpp>
#include <filesystem>
#include <functional>
#include <streambuf>
#include <fstream>

using namespace DeviceMonitor;

class MockFileDevice: public Devices::FileDevice {
public:
    MockFileDevice(
            unsigned long id,
            const std::string &file_name,
            const std::function<Monitor::Status(std::ifstream &)> &generator
    ) : Devices::FileDevice(id, file_name), generator(generator) {}

protected:
    Monitor::Status extract(std::ifstream &file) override {
        return generator(file);
    }

private:
    std::function<Monitor::Status(std::ifstream &)> generator;
};

TEST_CASE("FileDevice reads specified file and passes it to the extract method to construct status", "[unit]") {
    std::string file_name = "sample.txt";
    std::ofstream file{file_name};
    std::string file_content = "lorem ipsum dolores sit amet";
    file << file_content;
    file.close();

    MockFileDevice device { 0, file_name, [](std::ifstream &file) -> Monitor::Status {
        std::string content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return {{
                        {"some_key", Value{content}}
                }};
    } };

    auto status = device.fetch_current_state();
    REQUIRE(status.size() == 1);
    REQUIRE(status.contains("some_key"));
    REQUIRE(get_property<std::string>(status, "some_key") == file_content);

    std::filesystem::remove(file_name);
}