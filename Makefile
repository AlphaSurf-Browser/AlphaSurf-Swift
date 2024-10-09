# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Required packages
PKG_CONFIG = pkg-config
GTK_FLAGS = $(shell $(PKG_CONFIG) --cflags --libs gtk+-3.0)
WEBKIT_FLAGS = $(shell $(PKG_CONFIG) --cflags --libs webkit2gtk-4.0)
JSONGLIB_FLAGS = $(shell $(PKG_CONFIG) --cflags --libs json-glib)

# Executable name
EXECUTABLE = $(BIN_DIR)/AlphaSurf

# Default target
all: $(EXECUTABLE)

# Link the executable
$(EXECUTABLE): $(OBJECTS)
	@mkdir -p $(BIN_DIR)  # Create bin directory if it doesn't exist
	$(CXX) $(OBJECTS) -o $@ $(GTK_FLAGS) $(WEBKIT_FLAGS) $(JSONGLIB_FLAGS)

# Compile source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)  # Create build directory if it doesn't exist
	$(CXX) $(CXXFLAGS) $(GTK_FLAGS) $(WEBKIT_FLAGS) $(JSONGLIB_FLAGS) -c $< -o $@

# Clean the build and bin directories
clean:
	rm -rf $(BUILD_DIR)/*
	rm -rf $(BIN_DIR)/*

.PHONY: all clean
