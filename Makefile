CC ?= gcc
CPPFLAGS ?= -Iinclude
CFLAGS ?= -Wall -Wextra -std=c99
LDFLAGS ?=
DEPFLAGS := -MMD -MP
PKG_CONFIG ?= pkg-config

NCURSES_LIBS := $(shell $(PKG_CONFIG) --libs ncursesw 2>/dev/null || $(PKG_CONFIG) --libs ncurses 2>/dev/null || printf '%s\n' '-lncurses -ltinfo')
LDLIBS ?= $(NCURSES_LIBS)

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
