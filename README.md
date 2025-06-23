# vibs

A minimal vi-inspired terminal text editor written in C using ncurses.

## Features

- Basic vi-like navigation (h/j/k/l keys)
- Insert and normal modes (`i`/`a` to enter insert mode, `ESC` to exit)
- Simple command mode (`:` commands like `w`, `q`, `wq`)
- Basic line and word deletion (`dd`, `dw`)
- Status bar showing mode, cursor position, and filename
- Visual mode (Done) 
- Super simple and easy to configure configuration file (Done)

## Planned Features (TODO)

- Search and replace functionality
- Undo/Redo system
- Syntax highlighting
- More advanced text manipulation commands
- Plugins that can be made using Python

## Building

Requires ncurses library.

```bash
gcc -o vibs main.c buffer.c input.c render.c -lncurses
```

## Usage

Run `vibs` followed by the filename you want to edit:

```bash
./vibs filename.txt
```

To generate a configuration file, you must use the `-config` flag. By default it generates a config file in "~/.config/vibs", you can easily configure it since the configuration file is written in TOML (Tom's Obvious Minimal Language) 
