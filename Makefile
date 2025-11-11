# Compiler and flags
CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

# Target executable
TARGET = main

# Source files
MAIN_SRC = main.cpp
LIB_SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Header files
HEADERS = $(wildcard $(INC_DIR)/*.h)

# Object files
LIB_OBJS = $(LIB_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Default target
all: $(TARGET)

# Link main.cpp with library object files to create executable
$(TARGET): $(MAIN_SRC) $(LIB_OBJS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(MAIN_SRC) $(LIB_OBJS)

# Compile library source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create obj directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Rebuild everything
rebuild: clean all

# Run the program
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean rebuild run
