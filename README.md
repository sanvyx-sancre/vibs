# Vibs

A minimal vi-inspired terminal text editor written in C using ncurses.

## Features

- Basic vi-like navigation (h/j/k/l keys)
- Insert and normal modes (`i` to enter insert mode, `ESC` to exit)
- Simple command mode (`:` commands like `w`, `q`, `wq`)
- Basic line and word deletion (`dd`, `dw`)
- Status bar showing mode, cursor position, and filename

## Planned Features (TODO)

- Visual mode
- Search and replace functionality
- Undo/Redo system
- Syntax highlighting
- Configurable keybindings
- More advanced text manipulation commands

## Building

Requires ncurses library.

```bash
gcc -o vibs main.c buffer.c input.c -lncurses
