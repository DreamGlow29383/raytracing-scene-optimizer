# --- Client Makefile ---

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -I../engine
ENGINE_LIB_DIR := ../engine/bin/$(CONFIG)
LDFLAGS  := -L$(ENGINE_LIB_DIR) -lengine -Wl,-rpath,'$$ORIGIN'
LIBS     := -lfreeimage -lGL -lGLU -lGLEW -lglut -lassimp

BUILD_DIR := build
BIN_DIR   := bin/$(CONFIG)

EXE_NAME  := client
TARGET    := $(BIN_DIR)/$(EXE_NAME)

SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SRCS))

.PHONY: all client clean

all: client

client: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	@echo "Linking client..."
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)