cmake_minimum_required(VERSION 3.23)

project(dlogger LANGUAGES CXX)

add_library(dlogger INTERFACE)
add_library(dlogger::dlogger ALIAS dlogger)

target_include_directories(dlogger INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
