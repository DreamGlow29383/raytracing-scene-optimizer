# --- Engine Makefile ---

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -fPIC -Iinclude
LDFLAGS  := -shared
LIBS     := -lglut -lGLU -lGL -lfreeimage -lassimp

BUILD_DIR := build
BIN_DIR   := bin/Release
TEST_DIR  := ../test

LIB_NAME  := libengine.so
TARGET    := $(BIN_DIR)/$(LIB_NAME)
TEST_EXE  := $(BIN_DIR)/test_runner

SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SRCS))

.PHONY: all engine clean test

all: engine

engine: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	@echo "Linking library..."
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: engine
	@echo "Compiling tests..."
	$(CXX) -std=c++17 -Iinclude $(TEST_DIR)/*.cpp -o $(TEST_EXE) -L$(BIN_DIR) -lengine $(LIBS)
	@echo "Running tests..."
	LD_LIBRARY_PATH=$(BIN_DIR) $(TEST_EXE)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)