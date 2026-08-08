
# Guide: add a language syntax file

Location: `src/syntax/`

- One file per language: `LANG.c` (example: `c.c`, `python.c`).
- `Makefile` builds `src/syntax/*.c` automatically.

API

```
void syntax_<lang>_build_styles(const char *line, syntax_style_t *styles, int len);
```

- Fill `styles[i]` for each character in `line` with a `STYLE_*` enum from `include/syntax.h`.

Guidelines

- Set all positions to `STYLE_NORMAL` first.
- Handle whole-line cases early: leading `#` (C preprocessor), leading `@` (Python decorator), shebangs.
- Strings: support single/double and triple quotes if language has them. Respect escape sequences.
- Numbers: support hex/bin/oct, floats, suffixes, and underscores where appropriate.
- Keywords: use a static keyword table and strcmp on the identifier slice.
- Heuristics: `ident(` → function name (STYLE_KEYWORD). Preprocessor tokens → STYLE_PREPROCESSOR.

  Note: Improvements to the syntax system itself are welcome. If your language requires functionality not currently supported (e.g. multiline state), feel free to implement it and submit a PR.

Wiring

- If `src/syntax.c` already dispatches to your language builder, add the file and function and it will be used.
- To add file-extension autodetect:
  1. Add `SYNTAX_MODE_X` to `syntax_mode_t` in `include/syntax.h`.
 2. Update `syntax_detect_mode()` in `src/syntax.c` to map extensions to your mode.
3. Add a forward declaration for your builder in `src/syntax.c`.

Colors

- Colors come from `syntax_init_colors()` and `syntax_color_for("keyword")`.
- Reuse `STYLE_*` so config controls colors centrally.

Testing

- Run `make` or `make install`. DO NOT PR WITHOUT THIS obv lmao

Minimal skeleton

```c
#include "syntax.h"
#include <ctype.h>
#include <string.h>

void syntax_mylang_build_styles(const char *line, syntax_style_t *styles, int len) {
    for (int i = 0; i < len; i++) styles[i] = STYLE_NORMAL;
    if (len > 0 && line[0] == '#') { for (int j = 0; j < len; j++) styles[j] = STYLE_COMMENT; return; }
}
```

  Note: this parser is per-line only. Multi-line tokens require buffer/state support.
  **Note: I will not be accepting vibe coded PRs.**

Open a PR and I'll review.
