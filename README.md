# vibs

A minimal vi-inspired terminal text editor written in C using ncurses.

## Features

- Basic vi-like navigation (h/j/k/l keys)  
- Insert and normal modes (`i`/`a` to enter insert mode, `ESC` to exit)  
- Simple command mode (`:` commands like `w`, `q`, `wq`)  
- Basic line and word deletion (`dd`, `dw`)  
- Status bar showing mode, cursor position, and filename  
- Visual mode (done)  
- Super simple and easy-to-configure configuration file (done)  

## Planned Features (TODO)

- Search and replace functionality
- Yanking/Putting (copy paste) 
- Undo/Redo system
- More advanced text manipulation commands  
- And the other core vi features
-------------------------
- Syntax highlighting  
- Plugin system with Python support  

## Building

Requires ncurses library.
To install:

    sudo make install
## Usage

Run `vibs` followed by the filename you want to edit:

    vibs filename.txt

To generate a default configuration file, use the `-config` flag:

    vibs -config

By default, this creates the config file at `~/.config/vibs/config.toml`.  
The config file uses TOML format (Tom's Obvious Minimal Language), making it really SIMPLE to configure.
