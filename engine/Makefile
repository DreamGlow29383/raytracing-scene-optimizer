CONFIG ?= Debug

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -fPIC -Iinclude -I../dependencies -I../dependencies/imgui -I../dependencies/imgui/backends -I../dependencies/imgui-filediag -g
LDFLAGS  := -shared
LIBS     := -lGLEW -lglut -lGLU -lGL -lfreeimage -lassimp

BUILD_DIR := build
BIN_DIR   := bin/$(CONFIG)
TEST_DIR  := ../test

LIB_NAME  := libengine.so
TARGET    := $(BIN_DIR)/$(LIB_NAME)
TEST_EXE  := $(BIN_DIR)/test_runner

# Engine sources - find all .cpp files in current directory
ENGINE_SRCS := $(wildcard *.cpp)
ENGINE_OBJS := $(addprefix $(BUILD_DIR)/, $(ENGINE_SRCS:.cpp=.o))

# ImGui sources
IMGUI_DIR := ../dependencies/imgui
IMGUI_BACKENDS_DIR := $(IMGUI_DIR)/backends
IMGUI_FILE_DIALOG_DIR := ../dependencies/imgui-filediag

IMGUI_SRCS := $(IMGUI_DIR)/imgui.cpp \
              $(IMGUI_DIR)/imgui_draw.cpp \
              $(IMGUI_DIR)/imgui_widgets.cpp \
              $(IMGUI_DIR)/imgui_tables.cpp \
              $(IMGUI_BACKENDS_DIR)/imgui_impl_glut.cpp \
              $(IMGUI_BACKENDS_DIR)/imgui_impl_opengl3.cpp \
              $(IMGUI_FILE_DIALOG_DIR)/ImGuiFileDialog.cpp

# Create object file names for ImGui sources
IMGUI_OBJS := $(BUILD_DIR)/imgui.o \
              $(BUILD_DIR)/imgui_draw.o \
              $(BUILD_DIR)/imgui_widgets.o \
              $(BUILD_DIR)/imgui_tables.o \
              $(BUILD_DIR)/imgui_impl_glut.o \
              $(BUILD_DIR)/imgui_impl_opengl3.o \
              $(BUILD_DIR)/ImGuiFileDialog.o

# All object files
ALL_OBJS := $(ENGINE_OBJS) $(IMGUI_OBJS)

.PHONY: all engine clean test

all: engine

engine: $(TARGET)

$(TARGET): $(ALL_OBJS)
	@mkdir -p $(BIN_DIR)
	@echo "Linking library..."
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

# Compile engine files
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling engine $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile ImGui core files
$(BUILD_DIR)/imgui.o: $(IMGUI_DIR)/imgui.cpp
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling ImGui core $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/imgui_draw.o: $(IMGUI_DIR)/imgui_draw.cpp
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling ImGui draw $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/imgui_widgets.o: $(IMGUI_DIR)/imgui_widgets.cpp
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling ImGui widgets $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/imgui_tables.o: $(IMGUI_DIR)/imgui_tables.cpp
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling ImGui tables $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile ImGui backends
$(BUILD_DIR)/imgui_impl_glut.o: $(IMGUI_BACKENDS_DIR)/imgui_impl_glut.cpp
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling ImGui GLUT backend $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/imgui_impl_opengl3.o: $(IMGUI_BACKENDS_DIR)/imgui_impl_opengl3.cpp
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling ImGui OpenGL3 backend $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile ImGuiFileDialog
$(BUILD_DIR)/ImGuiFileDialog.o: $(IMGUI_FILE_DIALOG_DIR)/ImGuiFileDialog.cpp
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling ImGuiFileDialog $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: engine
	@echo "Compiling tests..."
	$(CXX) -std=c++17 -Iinclude $(TEST_DIR)/*.cpp -o $(TEST_EXE) -L$(BIN_DIR) -lengine $(LIBS)
	@echo "Running tests..."
	LD_LIBRARY_PATH=$(BIN_DIR) $(TEST_EXE)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)