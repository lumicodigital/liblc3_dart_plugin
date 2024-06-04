# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.12)

project(liblc3_dart_plugin_library VERSION 0.0.1 LANGUAGES C)

set(liblc3_root ${CMAKE_CURRENT_LIST_DIR}/../liblc3)
set(liblc3_src ${liblc3_root}/src)
set(liblc3_include ${liblc3_root}/include)
set(liblc3_srcs ${liblc3_src}/attdet.c
        ${liblc3_src}/bits.c
        ${liblc3_src}/bwdet.c
        ${liblc3_src}/energy.c
        ${liblc3_src}/lc3.c
        ${liblc3_src}/ltpf.c
        ${liblc3_src}/mdct.c
        ${liblc3_src}/plc.c
        ${liblc3_src}/sns.c
        ${liblc3_src}/spec.c
        ${liblc3_src}/tables.c
        ${liblc3_src}/tns.c)

add_library(liblc3_dart_plugin SHARED
    ${liblc3_srcs}
    "decoder.c"
)

# Specify private include directories
target_include_directories(liblc3_dart_plugin PRIVATE
    ${liblc3_include}
)

set_target_properties(liblc3_dart_plugin PROPERTIES
  PUBLIC_HEADER decoder.h
  OUTPUT_NAME "liblc3_dart_plugin"
)

target_compile_definitions(liblc3_dart_plugin PUBLIC DART_SHARED_LIB)

target_compile_options(liblc3_dart_plugin PRIVATE -Wall -Wdouble-promotion -O2 -ffast-math)

