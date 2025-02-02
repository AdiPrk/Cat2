# Compiler
CXX      = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -I$(SRC_DIR)

# Libraries
LDFLAGS = -lenet

# Directories
SRC_DIR   = src
BUILD_DIR = build

# Static Library (relative path)
STATIC_LIB = ../Common/Common.a

# Precompiled header settings
PCH_DIR  = $(SRC_DIR)/PCH
PCH_H    = $(PCH_DIR)/pch.h
PCH_GCH  = $(PCH_DIR)/pch.h.gch

# Find all .cpp files in src/ automatically.
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
# Uncomment the following line if you want to compile pch.cpp too:
# SRCS += $(wildcard $(PCH_DIR)/*.cpp)

OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

# Output binary
TARGET = myserver

# Default rule: first ensure the static lib is built, then build myserver
all: $(STATIC_LIB) $(TARGET)

# Rule to build the static library by calling the makefile in its directory.
# $(dir $(STATIC_LIB)) returns the directory "../Common/"
$(STATIC_LIB):
	$(MAKE) -C $(dir $(STATIC_LIB))

# Link the target (include both the static lib and the ENet library)
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(STATIC_LIB) $(LDFLAGS)

# Rule to build the precompiled header
$(PCH_GCH): $(PCH_H)
	$(CXX) $(CXXFLAGS) -x c++-header $< -o $@

# Compile .cpp files to .o; each source depends on the precompiled header
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(PCH_GCH) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Ensure build/ directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean build files locally and in the ../Common directory
clean:
	$(MAKE) -C $(dir $(STATIC_LIB)) clean
	rm -rf $(BUILD_DIR) $(TARGET) $(PCH_GCH)
