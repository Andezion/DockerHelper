cmake_minimum_required(VERSION 3.10)
project(DockerManager CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(wxWidgets_CONFIG_EXECUTABLE /usr/bin/wx-config)

find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})
include_directories(${wxWidgets_INCLUDE_DIRS})

add_executable(docker_manager src/docker_manager.cpp)
target_link_libraries(docker_manager ${wxWidgets_LIBRARIES})

add_custom_command(TARGET docker_manager POST_BUILD
    COMMAND chmod +x ${CMAKE_SOURCE_DIR}/scripts/docker_info.sh
    COMMENT "Делаем docker_info.sh исполняемым"
)

install(TARGETS docker_manager DESTINATION bin)
install(PROGRAMS scripts/docker_info.sh DESTINATION bin)

message(STATUS "Конфигурация Docker Manager:")
message(STATUS "  C++ стандарт: ${CMAKE_CXX_STANDARD}")
message(STATUS "  wxWidgets версия: ${wxWidgets_VERSION_STRING}")
message(STATUS "  Компилятор: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
