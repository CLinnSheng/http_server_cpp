CXX = g++
CXX_FLAGS = -Wall -Wextra -O3 -std=c++20 -Iinclude

SRC_DIR = src
OBJ_DIR = build/obj
BIN_DIR = build/bin

TARGET = $(BIN_DIR)/server

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC))

$(TARGET): $(OBJ)
	mkdir -p $(BIN_DIR)
	$(CXX) $(OBJ) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXX_FLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -rf build
