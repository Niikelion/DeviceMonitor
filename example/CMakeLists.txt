find_package(nlohmann_json CONFIG REQUIRED)

add_executable(DeviceMonitorExample)
target_sources(DeviceMonitorExample PRIVATE src/main.cpp)
target_link_libraries(DeviceMonitorExample PRIVATE DeviceMonitorLib nlohmann_json::nlohmann_json)
set_target_properties(DeviceMonitorExample PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
)