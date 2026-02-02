cmake_minimum_required(VERSION 3.10)
project(DockerManager)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(wxWidgets REQUIRED COMPONENTS core base)

if(wxWidgets_FOUND)
    include(${wxWidgets_USE_FILE})
else()
    message(FATAL_ERROR "wxWidgets не найден! Установите: sudo apt-get install libwxgtk3.0-gtk3-dev")
endif()

set(SOURCESокск 
    src/docker_manager.cpp
)

set(HEADERS
    src/docker_manager.h
)

add_executable(docker_manager ${SOURCES} ${HEADERS})

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
