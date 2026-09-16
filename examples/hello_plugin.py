import vibs


# return a message that the editor can display
def hello(args):
    target = args or "there"
    return f"Hello, {target}!"


# demonstrate an event callback
def report_open():
    path = vibs.current_file() or "[unnamed]"
    print(f"vibs plugin opened {path}")


vibs.command("Hello", hello)
vibs.on("file_open", report_open)