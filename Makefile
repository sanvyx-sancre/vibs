CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -std=c99
LDFLAGS = -lncurses

SRC = $(wildcard src/*.c)
BIN_DIR = bin
TARGET = $(BIN_DIR)/vibs
INSTALL_DIR = /usr/local/bin

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

install: $(TARGET)
	@echo "Installing vibs to $(INSTALL_DIR)..."
	@sudo cp $(TARGET) $(INSTALL_DIR)
	@echo "Installed as 'vibs'. You can now run it from anywhere."

uninstall:
	@echo "Removing vibs from $(INSTALL_DIR)..."
	@sudo rm -f $(INSTALL_DIR)/vibs
	@echo "Uninstalled."

clean:
	rm -f $(TARGET)

.PHONY: all install uninstall clean
