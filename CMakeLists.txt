cmake_minimum_required(VERSION 3.15)
project(arb_detector)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(detect
    src/main.cpp
    src/exchange_api.cpp
    src/graph.cpp
    src/bellman_ford.cpp
)

target_include_directories(detect PRIVATE headers)
target_link_libraries(detect PRIVATE cpr::cpr nlohmann_json::nlohmann_json)