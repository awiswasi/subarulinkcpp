cmake_minimum_required(VERSION 3.14)
project(subarulinkcpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(Threads REQUIRED)
find_package(nlohmann_json 3.11.2 REQUIRED)

# CPR Configuration
set(CPR_ENABLE_SSL ON)
add_subdirectory(cpr)

include_directories(${PROJECT_SOURCE_DIR}/include)

# Create library
add_library(subarulink
        src/controller.cpp
        src/constants.cpp
        src/connection.cpp
)

target_link_libraries(subarulink
        PRIVATE
        cpr
        nlohmann_json::nlohmann_json
)

# Create executable
add_executable(subarulink_client
        src/main.cpp
)

target_link_libraries(subarulink_client
        PRIVATE
        subarulink
        cpr
        nlohmann_json::nlohmann_json
        ${CMAKE_THREAD_LIBS_INIT}
)
