CC ?= gcc
CPPFLAGS ?= -Iinclude
CFLAGS ?= -Wall -Wextra -std=c99
LDFLAGS ?=
DEPFLAGS := -MMD -MP
PKG_CONFIG ?= pkg-config
PYTHON_CONFIG ?= python3-config

NCURSES_LIBS := $(shell $(PKG_CONFIG) --libs ncursesw 2>/dev/null || $(PKG_CONFIG) --libs ncurses 2>/dev/null || printf '%s\n' '-lncurses -ltinfo')
PYTHON_CFLAGS := $(shell $(PKG_CONFIG) --cflags python3-embed 2>/dev/null || $(PYTHON_CONFIG) --embed --cflags 2>/dev/null)
PYTHON_LIBS := $(shell $(PKG_CONFIG) --libs python3-embed 2>/dev/null || $(PYTHON_CONFIG) --embed --ldflags 2>/dev/null)
ifeq ($(strip $(PYTHON_CFLAGS)$(PYTHON_LIBS)),)
$(error Python 3 development files not found. Install Python headers and an embed config, such as python3-dev/python3-devel)
endif

CPPFLAGS += $(PYTHON_CFLAGS)
LDLIBS ?= $(NCURSES_LIBS)
LDLIBS += $(PYTHON_LIBS)

SRC := $(sort $(wildcard src/*.c) $(wildcard src/syntax/*.c))
OBJ := $(patsubst %.c,build/%.o,$(SRC))
DEPS := $(OBJ:.o=.d)

BIN_DIR := bin
TARGET := $(BIN_DIR)/vibs
INSTALL_DIR ?= /usr/local/bin

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

install: $(TARGET)
	@echo "Installing vibs to $(INSTALL_DIR)..."
	@install -d "$(INSTALL_DIR)"
	@install -m 755 "$(TARGET)" "$(INSTALL_DIR)/vibs"
	@echo "Installed as 'vibs'. You can now run it from anywhere."

uninstall:
	@echo "Removing vibs from $(INSTALL_DIR)..."
	@rm -f "$(INSTALL_DIR)/vibs"
	@echo "Uninstalled."

clean:
	rm -rf build $(BIN_DIR)

-include $(DEPS)

.PHONY: all install uninstall clean
