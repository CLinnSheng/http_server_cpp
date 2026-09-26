CXX = g++
CXXFLAGS = -Wall -Wextra -O3 -std=c++20 -Iinclude -pthread
LDFLAGS = -pthread

SRC_DIR = src
OBJ_DIR = build/obj
BIN_DIR = build/bin

TARGET = $(BIN_DIR)/main

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC))

$(TARGET): $(OBJ)
	mkdir -p $(BIN_DIR)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -rf build
