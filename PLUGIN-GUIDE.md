# vibs Plugin Guide

vibs plugins are Python files loaded from:

```text
~/.config/vibs/plugins/
```

Every `.py` file in that directory is loaded when vibs starts.

## Requirements

Building vibs with Python plugins requires Python development headers and the Python embedding configuration. The Makefile checks for either:

- `python3-embed` through `pkg-config`
- `python3-config --embed`

On Debian or Ubuntu, install the matching development package, for example:

```bash
sudo apt install python3-dev
```

On Fedora, use:

```bash
sudo dnf install python3-devel
```

## Your First Plugin

Create a file named `hello_plugin.py` in `~/.config/vibs/plugins/`:

```python
import vibs


def hello(args):
    target = args or "there"
    return f"Hello, {target}!"


vibs.command("Hello", hello)
```

Start vibs and run this command in command mode:

```text
:Hello world
```

The editor displays:

```text
Hello, world!
```

The same example is included in the repository at `examples/hello_plugin.py`.

## Commands

Register a command with `vibs.command(name, callback)`:

```python
import vibs


def count_lines(args):
    return f"Lines: {len(vibs.lines())}"


vibs.command("CountLines", count_lines)
```

Command callbacks receive the text after the command name. They may return a string to display in the status area.

```text
:CountLines
```

Command names are case-sensitive and should not include the leading `:`.

## Events

Register an event callback with `vibs.on(name, callback)`:

```python
import vibs


def after_save():
    print("vibs saved a file")


vibs.on("after_save", after_save)
```

Available events:

| Event | When it runs |
| --- | --- |
| `startup` | After the initial file is loaded and before the editor starts drawing |
| `file_open` | After a file is loaded, including the initial file |
| `before_save` | Immediately before saving |
| `after_save` | Immediately after saving |
| `text_changed` | Reserved for future text-change notifications |
| `cursor_move` | Reserved for future cursor-move notifications |
| `shutdown` | When the editor shuts down normally |

Event callbacks currently receive no arguments.

## Buffer API

Read the current buffer:

```python
import vibs


def show_current_line(args):
    line, column = vibs.cursor()
    return vibs.get_line(line)


vibs.command("CurrentLine", show_current_line)
```

Available functions:

```python
vibs.lines()                 # list of non-empty buffer content through the last used line
vibs.get_line(line)          # read a zero-based line
vibs.set_line(line, text)    # replace a zero-based line
vibs.cursor()                # return (line, column)
vibs.set_cursor(line, col)  # move the cursor
vibs.current_file()          # return the current filename or None
```

`vibs.set_line()` participates in the editor's undo history. Lines and cursor positions are zero-based.

## Example: Uppercase Current Line

```python
import vibs


def uppercase(args):
    line, column = vibs.cursor()
    text = vibs.get_line(line)
    vibs.set_line(line, text.upper())
    return "Uppercased current line"


vibs.command("Uppercase", uppercase)
```

Run it with:

```text
:Uppercase
```

## Diagnostics

The built-in command below reports whether Python initialized and how many plugin files loaded successfully:

```text
:PluginStatus
```

If a plugin raises an exception while loading or handling a command, vibs prints the Python traceback to its standard error output and continues running.

## Installing and Testing

From the repository root:

```bash
make clean
make
sudo make install
```

Copy the included example plugin into the user plugin directory:

```bash
mkdir -p ~/.config/vibs/plugins
cp examples/hello_plugin.py ~/.config/vibs/plugins/
```

Then launch vibs with a file and run:

```text
:PluginStatus
:Hello world
```

## Design Notes

The Python API is intentionally small and stable. Plugins should use `vibs` functions rather than accessing editor globals or internal C data structures directly.

The current runtime loads plugins at startup. A future plugin manager can add reload support, metadata, dependencies, key bindings, asynchronous jobs, syntax providers, and richer editor views without changing the basic command and buffer API.
