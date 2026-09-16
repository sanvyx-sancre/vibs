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
- Undo/Redo system (done) 

## Planned Features (TODO)

- Search and replace functionality
- Yanking/Putting (copy paste) 
- More advanced text manipulation commands  
- And the other core vi features
-------------------------
- Syntax highlighting  **(Partially Done)**
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

## Plugin system

vibs is being built around a small plugin API so simple plugins can be easy to write while advanced plugins can access editor operations through stable functions. The first layer supports registered commands and lifecycle events without exposing internal editor globals.

The planned Python interface will wrap this same API and support plugins such as formatters, linters, project tools, custom commands, and syntax providers. The current C-side boundary includes:

- custom colon commands with arguments
- startup, file-open, before-save, after-save, text-changed, cursor-move, and shutdown events
- reading and replacing buffer lines
- reading and changing the cursor
- reading the current filename

The Python runtime and user plugin discovery will be added on top of this boundary.

### Python plugins

Python development headers and the Python embed configuration are required to build the plugin runtime. The Makefile checks for `python3-embed` through `pkg-config` or `python3-config`.

Plugins are loaded from:

    ~/.config/vibs/plugins/*.py

To try the included example:

    mkdir -p ~/.config/vibs/plugins
    cp examples/hello_plugin.py ~/.config/vibs/plugins/
    make
    vibs example.txt

Inside vibs, run `:Hello world`. The command should display `Hello, world!`.

If a plugin does not appear to load, run `:PluginStatus`. It reports whether Python initialized, whether the plugin directory was found, and how many plugin files loaded successfully.
