# Variables
CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -MMD -Wno-unused-parameter
LDFLAGS := -lz
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin
INCLUDE_DIR := include
TARGET := $(BIN_DIR)/MinecraftServer

TEST_DIR := tests
TEST_BUILD_DIR := build/tests
TEST_TARGET := $(BIN_DIR)/TestSuite

# Source and object files
SRC := $(shell find $(SRC_DIR) -name '*.cpp')
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJ:.o=.d)

# Test source and object files
TEST_SRC := $(shell find $(TEST_DIR) -name '*.cpp')
TEST_OBJ := $(TEST_SRC:$(TEST_DIR)/%.cpp=$(TEST_BUILD_DIR)/%.o)
TEST_DEPS := $(TEST_OBJ:.o=.d)

# Rules
.PHONY: default clean directories debug fast clang linux windows test test-directories

# Either linux or windows must be defined, but never both
default: linux

# Use these defines for platform specific code
linux: LDFLAGS += -lssl -lcrypto
linux: CXXFLAGS += -D LINUX
linux: directories $(TARGET)

windows: CXXFLAGS += -D WINDOWS
windows: directories $(TARGET)

# Debug target
debug: CXXFLAGS += -D DEBUG -g
debug: linux

# Optimizations included
fast: CXXFLAGS += -Ofast
fast: linux

# build using clang & llvm
clang: CXX := clang++ 
clang: linux

# Test targets
test: CXXFLAGS += -D LINUX
test: LDFLAGS += -lssl -lcrypto
test: test-directories $(TEST_TARGET) 

# Link the final executable
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

$(TEST_TARGET): $(TEST_OBJ)
	$(CXX) $(TEST_OBJ) $(LIB_OBJ) -o $@ $(LDFLAGS)

# Compile each source file into an object file
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Compile each test file into an object file
$(TEST_BUILD_DIR)/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Create necessary directories
directories:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)

test-directories:
	mkdir -p $(TEST_BUILD_DIR) $(BIN_DIR)

# Clean up
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) $(DEPS) $(TARGET)

# Include dependency files
-include $(DEPS)
