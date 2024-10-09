cmake_minimum_required(VERSION 3.10)

# Set the project name
project(AlphaSurf)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk+-3.0)
pkg_check_modules(WEBKIT REQUIRED webkit2gtk-4.0)

# Include directories
include_directories(${GTK_INCLUDE_DIRS} ${WEBKIT_INCLUDE_DIRS})

# Specify the source files
set(SOURCES
    browser.cpp
)

# Create the executable
add_executable(AlphaSurf ${SOURCES})

# Link the libraries
target_link_libraries(AlphaSurf ${GTK_LIBRARIES} ${WEBKIT_LIBRARIES})
