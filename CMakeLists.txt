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

CPMAddPackage("gh:fmtlib/fmt#9.1.0")
CPMAddPackage("gh:Roblox/luau#0.570")
CPMAddPackage("gh:SRombauts/SQLiteCpp#3.2.1")
CPMAddPackage("gh:kunitoki/LuaBridge3#3.0-rc3")
CPMAddPackage("gh:skypjack/entt#v3.11.1")

SET(LUABRIDGE_TESTING OFF)
SET(LUAU_BUILD_CLI OFF)
SET(LUAU_BUILD_TESTS OFF)
SET(LUAU_BUILD_BENCHMARKS OFF)

find_package(Boost REQUIRED COMPONENTS system program_options json)
find_package(SQLite3 REQUIRED)

file(GLOB_RECURSE KAIZERMUD_INCLUDE include/*.h)
file(GLOB_RECURSE KAIZERMUD_SRC src/*.cpp)

SET(KAIZERMUD_INCLUDE_DIRS
        include
        ${luau_SOURCE_DIR}/Common/include
        ${luau_SOURCE_DIR}/VM/include
        ${luau_SOURCE_DIR}/Compiler/include
        ${Boost_INCLUDE_DIRS}
        ${LuaBridge_SOURCE_DIR}/Source
        ${SQLiteCpp_SOURCE_DIR}/include
        ${FMT_SOURCE_DIR}/include
        ${entt_SOURCE_DIR}/single_include
)

add_library(kaizermud ${KAIZERMUD_INCLUDE} ${KAIZERMUD_SRC})
target_include_directories(kaizermud PUBLIC ${KAIZERMUD_INCLUDE_DIRS})
add_executable(kaizer apps/kaizer.cpp)
target_include_directories(kaizer PUBLIC ${KAIZERMUD_INCLUDE_DIRS})

set(KAIZERMUD_LINK_LIBRARIES ${Boost_LIBRARIES} Luau.VM Luau.Common Luau.Compiler SQLiteCpp sqlite3 fmt::fmt)

target_link_libraries(kaizermud ${KAIZERMUD_LINK_LIBRARIES})
target_link_libraries(kaizer ${KAIZERMUD_LINK_LIBRARIES} kaizermud)

add_executable(ktest apps/ktest.cpp)
target_link_libraries(ktest ${KAIZERMUD_LINK_LIBRARIES} kaizermud )

SET(kaizer_link ${CMAKE_INSTALL_PREFIX}/bin/)
set(CMAKE_CXX_STANDARD 23)