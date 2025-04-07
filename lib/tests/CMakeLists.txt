find_package(Catch2 3 REQUIRED)

#function to append extensions
FUNCTION(APPEND_EXT var source ext)
	SET(listVar "")
	FOREACH(f ${${source}})
		LIST(APPEND listVar "${f}.${ext}")
	ENDFOREACH(f)
	SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(APPEND_EXT)

set(TESTS
	MockDevice
	FileDevice
	Monitor
)

APPEND_EXT(TESTS TESTS "cpp")

add_executable(DeviceMonitorLibTests ${TESTS})
set_target_properties(DeviceMonitorLibTests PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:DeviceMonitorLibTests>"
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
)
target_link_libraries(DeviceMonitorLibTests PRIVATE Catch2::Catch2WithMain DeviceMonitorLib)

add_custom_command(TARGET DeviceMonitorLibTests POST_BUILD
	COMMAND DeviceMonitorLibTests [unit]
)

set_target_properties(DeviceMonitorLibTests PROPERTIES FOLDER "Tests")