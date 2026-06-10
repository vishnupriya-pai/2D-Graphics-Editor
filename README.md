# 2D Terminal Graphics Editor

An interactive, vector-to-raster 2D graphics engine and TUI (Text User Interface) application written in standard C. The application maintains an in-memory vector object table and mathematically rasterizes geometric primitives onto an $80 \times 24$ character grid canvas using classical computer graphics rasterization algorithms. The visual output features retro hardware styling powered entirely via raw ANSI terminal escape sequences.
##Interactive Controls
On execution, the editor pre-seeds three sample demo layers (a border rectangle, a central circle, and a diagonal splitting line) and opens up an action-driven runtime prompt:

[1] Add Shape: Launch sub-prompts to introduce a Line, Rectangle, Circle, or Triangle with coordinate validation boundaries.

[2] Delete Shape: Purges an active vector element layer by targeted numeric ID, initiating an immediate canvas redraw step.

[3] Modify Shape: Targets coordinates or parameters of an existing layer component and adapts its grid structure.

[4] Clear Canvas: De-allocates the active shape tracker arrays completely and resets the canvas coordinate layout.

[5] Save to Text File: Flushes and dumps the absolute raw raster ASCII string arrays into a standard .txt document.

[6] Refresh Screen: Forces a hard clear execution pass (\033[2J) and system redraw pass.

[7] Exit: Terminates the execution sequence and exits gracefully.


---

## Technical Specifications & System Architecture

The application implements a full interactive rendering loop inside a terminal subsystem. It utilizes structural data abstractions to mimic standard vector layers:

* **Canvas Buffer:** A fixed $24 \times 80$ contiguous two-dimensional primitive character array (`char canvas[24][80]`).
* **Vector Object Management:** Dynamically managed array structures track active elements via tagged algebraic unions (`ShapeType` enums mapping down to coordinate payloads: `LineData`, `RectData`, `CircleData`, and `TriangleData`).
* **State Redraw Pass:** Upon structural vector mutations (additions, updates, deletions), the application flushes the canvas buffer using `clear_canvas()` and iterates across the object array to sequentially execute discrete mathematical plotting algorithms into the raster grid.

---

## Algorithms Implemented

### 1. Line Drawing: Bresenham's Line Algorithm
The engine avoids heavy floating-point math arithmetic by utilizing **Bresenham's Line Algorithm**. The algorithm calculates incremental integer error factors ($e_2$) to determine whether to advance coordinates across the major or minor drawing axes:

$$err = \Delta x - \Delta y$$

### 2. Circle Generation: Midpoint Circle Algorithm
* **Hollow Outlines:** Implemented via the **Midpoint Circle Algorithm (Bresenham's Circle)**. It relies on 8-way symmetry, computing only a single octant arc segment and mapping structural offsets across all remaining octants using mirror translations:
    $(\pm x, \pm y)$ and $(\pm y, \pm x)$
* **Filled Regions:** Utilizes an absolute geometric boundary scan checking bounding-box offsets against the algebraic radius limit equation:
    $$\Delta x^2 + \Delta y^2 \le r^2$$

### 3. Rectangles & Triangles
* **Rectangles:** Outline renders plot discrete horizontal boundary limits and vertical line limits. Solid fill modes trigger multi-row nested scanline iteration loops.
* **Triangles:** Renders edge-connections sequentially linking three arbitrary spatial vertices $(x_1, y_1)$, $(x_2, y_2)$, and $(x_3, y_3)$ through back-to-back Bresenham line passes.

---

## Interface Layout and ANSI Coloring

The application outputs an organized dashboard interface styled directly with embedded ANSI escape sequences:

* **Coordinate Axes:** Numbered green borders (`\033[1;32m`) render a clear, live column/row indexing frame (tens on top, units below).
* **Raster Elements:** Active geometric plots (`*`) flash in bright yellow (`\033[1;33m`).
* **Background Canvas:** Blank coordinates (`_`) render in dark-grey styling (`\033[90m`) to maintain high-contrast legibility.
* **Windows Emulation Compatibility:** Includes built-in API support using `SetConsoleMode` with the `ENABLE_VIRTUAL_TERMINAL_PROCESSING` flag, ensuring seamless cross-platform ANSI color processing natively inside modern Windows Command Prompts and PowerShell terminals.

---

## Project File Details

* **File Name:** `VISHNUPRIYA_R25EI057.c`
* **Composition:** Consolidated, single-source distribution file containing primitive data definitions, global canvas state matrices, graphics drawing math functions, safe standard I/O buffer filters (`fgets` validation), and the execution lifecycle `while(1)` control framework.

---

## Compilation and Execution

### Prerequisites
A modern standard C compiler (e.g., GCC or Clang) must be accessible within your system path configuration.

### Compilation
Build the binary with an optimization flag (`-O2`) for clean performance:

```bash
gcc -O2 VISHNUPRIYA_R25EI057.c -o terminal_editor
Execution
Run the compiled executable directly from your preferred shell environment:

Linux / macOS:

Bash
./terminal_editor
Windows (PowerShell / CMD):

PowerShell
.\terminal_editor.exe


