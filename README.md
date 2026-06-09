# 2D Terminal Graphics Editor in C

An interactive, vector-to-raster 2D graphics editor written in standard C. The application draws shapes (lines, rectangles, circles, and triangles) on an `80x24` text-based canvas using mathematical rasterization algorithms, styled with retro ANSI terminal colors.

---

## Features
- **Vector-to-Raster Grid**: Store active shapes in an object list, rendering them dynamically to a 2D character array of `*` (foreground) and `_` (background).
- **Geometric Drawing Algorithms**:
  - **Lines**: Bresenham's Line Algorithm.
  - **Rectangles**: Outlined or solid-filled.
  - **Circles**: Midpoint Circle Algorithm (hollow) and geometric inequality checks (filled).
  - **Triangles**: Draw connected linear edges between three arbitrary vertices.
- **Layer Management**:
  - **Add Shape**: Interactively create new shapes with custom coordinates.
  - **Modify Shape**: Edit the parameters/coordinates of any active shape by its ID.
  - **Delete Shape**: Remove a layer, clearing and redrawing the canvas automatically.
  - **Clear Canvas**: Reset the list of shapes.
- **File Export**: Save the rasterized drawing as a raw `.txt` file.
- **Console Optimization**: Supports ANSI colors natively in Unix terminals and Windows CMD/PowerShell (via Windows Console Virtual Terminal Processing).

---

## File Structure
- `editor.h`: Coordinate boundary definitions, shape structure models, and functions declarations.
- `draw.c`: Low-level rasterization logic and mathematical line/shape drawing algorithms.
- `main.c`: Application event loop, TUI menus, grid rendering, and safe input parsing.

---

## Build and Run Instructions

### Prerequisites
Make sure you have GCC installed.

### Compilation
Compile the project using the command line:
```bash
gcc -O2 main.c draw.c -o editor.exe
```

### Execution
Run the compiled executable:
```powershell
.\editor.exe
```
