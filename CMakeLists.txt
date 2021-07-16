cmake_minimum_required(VERSION 3.4...3.18)
project(cmake_example)
set(PYBIND11_FINDPYTHON ON)
add_subdirectory(pybind11)
add_subdirectory(interface)
#find_package(pybind11 CONFIG REQUIRED)

option(QV2RAY_PYBIND_STUBGEN "Generate stubs" OFF)
if(QV2RAY_PYBIND_STUBGEN)
    add_compile_definitions(-DQV2RAY_PYBIND_STUBGEN=1)
endif()

add_executable(embedded_python src/main.cpp src/QJsonCaster.hpp src/QContainerCaster.hpp)
#add_library(QvPybind SHARED src/main.cpp src/QJsonCaster.hpp src/QContainerCaster.hpp)

target_link_libraries(embedded_python PUBLIC Qv2ray::QvPluginInterface pybind11::embed)
#target_link_libraries(QvPybind PUBLIC Qv2ray::QvPluginInterface pybind11::embed)

target_compile_definitions(embedded_python PRIVATE QT_NO_SIGNALS_SLOTS_KEYWORDS)
#target_compile_definitions(QvPybind PRIVATE QT_NO_SIGNALS_SLOTS_KEYWORDS QV2RAY_PYTHON_BINDING_LIBRARY)

