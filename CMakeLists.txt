cmake_minimum_required(VERSION 3.22)

set(CPM_DOWNLOAD_VERSION 0.35.5)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake")
    file(DOWNLOAD https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
endif()

include(${CPM_DOWNLOAD_LOCATION})

project(kaizermud)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#CPMAddPackage("gh:fmtlib/fmt#9.1.0")
CPMAddPackage("gh:Roblox/luau#0.570")

CPMAddPackage("gh:kunitoki/LuaBridge3#3.0-rc3")
SET(LUABRIDGE_TESTING OFF)
SET(LUAU_BUILD_CLI OFF)
SET(LUAU_BUILD_TESTS OFF)
SET(LUAU_BUILD_BENCHMARKS OFF)


find_package(Boost REQUIRED COMPONENTS system program_options json)

file(GLOB_RECURSE KAIZERMUD_INCLUDE include/*.h)
file(GLOB_RECURSE KAIZERMUD_SRC src/*.cpp)

include_directories(PUBLIC
        include
        ${luau_SOURCE_DIR}/Common/include
        ${luau_SOURCE_DIR}/VM/include
        ${luau_SOURCE_DIR}/Compiler/include
        ${Boost_INCLUDE_DIRS}
        ${LuaBridge_SOURCE_DIR}/Source
)

add_library(kaizermud ${KAIZERMUD_INCLUDE} ${KAIZERMUD_SRC})
add_executable(kaizer apps/kaizer.cpp)

target_link_libraries(kaizermud ${Boost_LIBRARIES} Luau.VM Luau.Common Luau.Compiler)
target_link_libraries(kaizer ${Boost_LIBRARIES} kaizermud)

add_executable(ktest apps/ktest.cpp)
target_link_libraries(ktest ${Boost_LIBRARIES} kaizermud)

SET(kaizer_link ${CMAKE_INSTALL_PREFIX}/bin/)
