cmake_minimum_required(VERSION 3.0.0)
project(himawari_support)

set(SATDUMP_ROOT "/usr")

file(GLOB_RECURSE himawari_support_CPPS *.cpp)
add_library(himawari_support SHARED ${himawari_support_CPPS})
target_link_libraries(himawari_support PUBLIC satdump_core)
target_include_directories(himawari_support PUBLIC src ${SATDUMP_ROOT}/include/satdump .)

install(TARGETS himawari_support DESTINATION share/satdump/plugins)