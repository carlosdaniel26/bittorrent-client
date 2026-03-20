CC := gcc
CXX := g++
LIBS := -lssl -lcrypto -lcurl
CFLAGS := -Wall -Wextra -O2
CXXFLAGS := -Wall -Wextra -O2

TARGET := main
BUILD_DIR := build
TORRENT_FILE := torrents/debian.torrent 

SRC_C := $(wildcard src/*.c)
SRC_CPP := $(wildcard src/*.cpp)

OBJ_C := $(SRC_C:%.c=$(BUILD_DIR)/%.o)
OBJ_CPP := $(SRC_CPP:%.cpp=$(BUILD_DIR)/%.o)

OBJ := $(OBJ_C) $(OBJ_CPP)

all: $(BUILD_DIR)/$(TARGET)

# linking
$(BUILD_DIR)/$(TARGET): $(OBJ)
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
	@echo "(LINK) $@"

# C
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "(CC) $< -> $@"

# C++
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "(CXX) $< -> $@"

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

run: $(BUILD_DIR)/$(TARGET)
	@echo "(RUN) ./$(BUILD_DIR)/$(TARGET) $(TORRENT_FILE)"
	@./$(BUILD_DIR)/$(TARGET) $(TORRENT_FILE)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean run