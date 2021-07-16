cmake_minimum_required(VERSION 3.4...3.18)
project(cmake_example)

add_subdirectory(pybind11)
add_subdirectory(interface)
#find_package(pybind11 CONFIG REQUIRED)

add_executable(embedded_python src/main.cpp src/QJsonCaster.hpp)
target_link_libraries(embedded_python PUBLIC Qv2ray::QvPluginInterface pybind11::embed)
target_compile_definitions(embedded_python PRIVATE QT_NO_SIGNALS_SLOTS_KEYWORDS)

