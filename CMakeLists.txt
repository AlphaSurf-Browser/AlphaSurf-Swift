cmake_minimum_required(VERSION 3.10)
project(AlphaBrowser)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the CEF package
find_package(CEF REQUIRED)

# Specify the source files
set(SOURCES
    src/browser.cpp
)

# Create the executable
add_executable(AlphaBrowser ${SOURCES})

# Link the CEF libraries to the executable
target_link_libraries(AlphaBrowser ${CEF_LIBRARIES})

# You may not need to copy any DLLs if they are handled by the package manager.
