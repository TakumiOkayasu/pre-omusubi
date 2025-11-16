# Compiler and flags
CXX = clang++
CXXFLAGS = -std=c++14 -Wall -Wextra -Iinclude

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

# Target executable
TARGET = main

# Source files
MAIN_SRC = main.cpp
LIB_SRCS = $(shell find $(SRC_DIR) -type f -name '*.cpp')

# Header files
HEADERS = $(shell find $(INC_DIR) -type f \( -name '*.h' -o -name '*.hpp' \))

# Object files (maintain directory structure in obj/)
LIB_OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(LIB_SRCS))

# Default target
all: $(TARGET)

# Link main.cpp with library object files to create executable
$(TARGET): $(MAIN_SRC) $(LIB_OBJS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(MAIN_SRC) $(LIB_OBJS)

# Compile library source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Rebuild everything
rebuild: clean all

# Run the program
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean rebuild run
