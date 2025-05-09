cmake_minimum_required(VERSION 3.14)
project(MyProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)


file(GLOB SRC_FILES "${CMAKE_SOURCE_DIR}/*.cpp")


add_executable(MyProject ${SRC_FILES})


find_package(flatbuffers REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(concurrentqueue REQUIRED)
find_package(spdlog REQUIRED)
target_link_libraries(MyProject PRIVATE 
    flatbuffers::flatbuffers 
    nlohmann_json::nlohmann_json 
    concurrentqueue::concurrentqueue
    spdlog::spdlog  
)
target_include_directories(MyProject PRIVATE 
    ${CMAKE_SOURCE_DIR}/src 
)
