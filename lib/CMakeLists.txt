add_library(DeviceMonitorLib)
target_include_directories(DeviceMonitorLib PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

set(DEVICEMONITOR_LIB_HEADERS
	include/DeviceMonitor/Monitor.hpp
	include/DeviceMonitor/Device.hpp
	include/DeviceMonitor/Devices.hpp
	include/DeviceMonitor/Value.hpp
)
target_sources(DeviceMonitorLib PRIVATE
	${DEVICEMONITOR_LIB_HEADERS}
	src/Monitor.cpp
	src/Device.cpp
	src/Devices.cpp
)
set_target_properties(DeviceMonitorLib PROPERTIES
	PUBLIC_HEADER "${DEVICEMONITOR_LIB_HEADERS}"
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
)

if (BUILD_TESTS)
	add_subdirectory(tests)
endif()