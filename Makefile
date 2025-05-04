# Variables
CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -MMD -Wno-unused-parameter
LDFLAGS := -lz
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin
INCLUDE_DIR := include
TARGET := $(BIN_DIR)/MinecraftServer


# Source and object files
SRC := $(shell find $(SRC_DIR) -name '*.cpp')
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJ:.o=.d)

# Rules
.PHONY: default clean directories debug fast clang linux windows

# Either linux or windows must be defined, but never both
default: linux

# Use these defines for platform specific code
linux: CXXFLAGS += -D LINUX
linux: directories $(TARGET)

windows: CXXFLAGS += -D WINDOWS
windows: directories $(TARGET)

# Debug target
debug: CXXFLAGS += -D DEBUG -g
debug: directories $(TARGET)

# Optimizations included
fast: CXXFLAGS += -Ofast
fast: directories $(TARGET)

# build using clang & llvm
clang: CXX := clang++ 
clang: directories $(TARGET)

# Link the final executable
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

# Compile each source file into an object file
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Create necessary directories
directories:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)

# Clean up
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) $(DEPS) $(TARGET)

# Include dependency files
-include $(DEPS)
