# Variables
CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -g -MMD # -Wno-unused-parameter
LDFLAGS := 
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
.PHONY: all clean directories

all: directories $(TARGET)

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
