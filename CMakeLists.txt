cmake_minimum_required(VERSION 3.10)
project(abx_exchange_client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(nlohmann_json REQUIRED)

# Add executable
add_executable(abx_client 
    src/main.cpp
)

# Include directories
target_include_directories(abx_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(abx_client PRIVATE
    nlohmann_json::nlohmann_json
    pthread
)

# Set compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(abx_client PRIVATE -Wall -Wextra -Wpedantic)
endif()
