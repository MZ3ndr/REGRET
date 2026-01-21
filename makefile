CXX := clang++
LD  := clang++
SRC_DIR := src
INCLUDE_DIR := include
BUILD_DIR := build
BIN_DIR := bin
log_DIR := bin/logs
APP_NAME := debug
PKG_CONFIG ?= pkg-config
GLFW_LIBS := $(shell $(PKG_CONFIG) --libs glfw3 2>/dev/null)
GLFW_CFLAGS := $(shell $(PKG_CONFIG) --cflags glfw3 2>/dev/null)


CPPFLAGS := -I$(INCLUDE_DIR)
CXXFLAGS_DEBUG := -std=c++17 -g -Wall -Werror -DDEBUG -D_CRT_SECURE_NO_WARNINGS
DEPFLAGS := -MMD -MP

LDFLAGS :=
LDFLAGS_DEBUG := -g
LIBS :=
LIB_DIRS ?=
LDFLAGS += $(foreach d,$(LIB_DIRS),-L$(d))
CPPFLAGS += $(GLFW_CFLAGS)
LIBS += -lglfw -lvulkan



ifeq ($(OS),Windows_NT)
  TARGET_OS := windows
  REAL_BINARY := $(BIN_DIR)/$(APP_NAME).exe
  WRAPPER := $(REAL_BINARY)
else
  TARGET_OS := linux
  REAL_BINARY := $(BIN_DIR)/.$(APP_NAME).bin
  WRAPPER := $(BIN_DIR)/$(APP_NAME)
endif

SOURCES := $(shell find $(SRC_DIR) -type f -name '*.cpp')
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
DEPS := $(OBJECTS:.o=.d)

all: $(WRAPPER)
debug: all
$(REAL_BINARY): $(OBJECTS)
	@mkdir -p "$(BIN_DIR)"
	@mkdir -p "$(log_DIR)"
	$(LD) $(OBJECTS) -o $@ $(LDFLAGS) $(LIBS) $(LDFLAGS_DEBUG)

ifeq ($(TARGET_OS),linux)
$(WRAPPER): $(REAL_BINARY)
	@mkdir -p "$(BIN_DIR)"
	@tmp="$(WRAPPER).tmp"; \
	echo "Creating terminal wrapper: $(WRAPPER)"; \
	{ \
	  echo '#!/bin/sh'; \
	  echo 'DIR="$$(cd "$$(dirname "$$0")" && pwd)"'; \
	  echo ''; \
	  echo 'if command -v konsole >/dev/null 2>&1; then'; \
	  echo '  exec konsole --hold -e "$$DIR/.$(APP_NAME).bin"'; \
	  echo 'elif command -v gnome-terminal >/dev/null 2>&1; then'; \
	  echo '  exec gnome-terminal -- "$$DIR/.$(APP_NAME).bin"'; \
	  echo 'elif command -v xterm >/dev/null 2>&1; then'; \
	  echo '  exec xterm -hold -e "$$DIR/.$(APP_NAME).bin"'; \
	  echo 'else'; \
	  echo '  echo "No supported terminal emulator found (konsole/gnome-terminal/xterm)."'; \
	  echo '  echo "Run directly: $$DIR/.$(APP_NAME).bin"'; \
	  echo '  exit 1'; \
	  echo 'fi'; \
	} > "$$tmp"; \
	chmod +x "$$tmp"; \
	mv -f "$$tmp" "$(WRAPPER)"
endif

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p "$(dir $@)"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS_DEBUG) $(DEPFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	@rm -rf "$(BUILD_DIR)" "$(BIN_DIR)"

.PHONY: all debug clean
