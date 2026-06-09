/**
 * 2D Terminal Graphics Editor
 * File: VISHNUPRIYA_R25EI057.c
 * Description: Interactive C application for drawing, deleting, and modifying
 *              geometric shapes on a 2D character-based canvas.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#endif

// Canvas and Shape Configuration
#define CANVAS_ROWS 24
#define CANVAS_COLS 80
#define MAX_SHAPES 100

// Character representation
#define CHAR_BG '_'
#define CHAR_FG '*'

// Shape Types
typedef enum {
    SHAPE_LINE,
    SHAPE_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_TRIANGLE
} ShapeType;

// Geometric Data Structures
typedef struct {
    int x1, y1;
    int x2, y2;
} LineData;

typedef struct {
    int x, y;
    int width, height;
    int fill; // 0 for outline, 1 for filled
} RectData;

typedef struct {
    int cx, cy;
    int r;
    int fill; // 0 for outline, 1 for filled
} CircleData;

typedef struct {
    int x1, y1;
    int x2, y2;
    int x3, y3;
} TriangleData;

typedef struct {
    int id;
    ShapeType type;
    int active;
    union {
        LineData line;
        RectData rect;
        CircleData circle;
        TriangleData triangle;
    } data;
} Shape;

// Global State
char canvas[CANVAS_ROWS][CANVAS_COLS];
Shape shapes[MAX_SHAPES];
int next_shape_id = 1;

// Virtual Terminal configuration for ANSI color support on Windows
void enable_virtual_terminal(void) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
}

// Clear canvas with background characters
void clear_canvas(void) {
    for (int r = 0; r < CANVAS_ROWS; r++) {
        for (int c = 0; c < CANVAS_COLS; c++) {
            canvas[r][c] = CHAR_BG;
        }
    }
}

// Safe bound-checked plot helper
void plot_pixel(int x, int y) {
    if (x >= 0 && x < CANVAS_COLS && y >= 0 && y < CANVAS_ROWS) {
        canvas[y][x] = CHAR_FG;
    }
}

// Helper function to pause and wait for the user to press Enter
void press_enter_to_continue(void) {
    printf("Press Enter to continue...");
    char buffer[10];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        // Prevent compiler warnings for ignored return values
    }
}

// Bresenham's Line Algorithm
void draw_line(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        plot_pixel(x1, y1);
        if (x1 == x2 && y1 == y2) {
            break;
        }
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

// Draw Rectangle (Outline or Solid Filled)
void draw_rectangle(int x, int y, int w, int h, int fill) {
    if (w <= 0 || h <= 0) return;

    if (fill) {
        for (int r = y; r < y + h; r++) {
            for (int c = x; c < x + w; c++) {
                plot_pixel(c, r);
            }
        }
    } else {
        // Horizontal lines
        for (int c = x; c < x + w; c++) {
            plot_pixel(c, y);
            plot_pixel(c, y + h - 1);
        }
        // Vertical lines
        for (int r = y; r < y + h; r++) {
            plot_pixel(x, r);
            plot_pixel(x + w - 1, r);
        }
    }
}

// Draw Circle (Outline or Filled scanlines)
void draw_circle(int cx, int cy, int r, int fill) {
    if (r < 0) return;
    if (r == 0) {
        plot_pixel(cx, cy);
        return;
    }

    if (fill) {
        for (int dy = -r; dy <= r; dy++) {
            for (int dx = -r; dx <= r; dx++) {
                if (dx * dx + dy * dy <= r * r) {
                    plot_pixel(cx + dx, cy + dy);
                }
            }
        }
    } else {
        // Midpoint circle outline algorithm
        int x = 0;
        int y = r;
        int d = 3 - 2 * r;
        
        while (y >= x) {
            plot_pixel(cx + x, cy + y);
            plot_pixel(cx - x, cy + y);
            plot_pixel(cx + x, cy - y);
            plot_pixel(cx - x, cy - y);
            plot_pixel(cx + y, cy + x);
            plot_pixel(cx - y, cy + x);
            plot_pixel(cx + y, cy - x);
            plot_pixel(cx - y, cy - x);

            if (d > 0) {
                y--;
                d = d + 4 * (x - y) + 10;
            } else {
                d = d + 4 * x + 6;
            }
            x++;
        }
    }
}

// Draw Triangle (Outline)
void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3) {
    draw_line(x1, y1, x2, y2);
    draw_line(x2, y2, x3, y3);
    draw_line(x3, y3, x1, y1);
}

// Redraw all active vector elements
void redraw_all_shapes(void) {
    clear_canvas();
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (shapes[i].active) {
            switch (shapes[i].type) {
                case SHAPE_LINE:
                    draw_line(shapes[i].data.line.x1, shapes[i].data.line.y1,
                              shapes[i].data.line.x2, shapes[i].data.line.y2);
                    break;
                case SHAPE_RECTANGLE:
                    draw_rectangle(shapes[i].data.rect.x, shapes[i].data.rect.y,
                                   shapes[i].data.rect.width, shapes[i].data.rect.height,
                                   shapes[i].data.rect.fill);
                    break;
                case SHAPE_CIRCLE:
                    draw_circle(shapes[i].data.circle.cx, shapes[i].data.circle.cy,
                                shapes[i].data.circle.r, shapes[i].data.circle.fill);
                    break;
                case SHAPE_TRIANGLE:
                    draw_triangle(shapes[i].data.triangle.x1, shapes[i].data.triangle.y1,
                                  shapes[i].data.triangle.x2, shapes[i].data.triangle.y2,
                                  shapes[i].data.triangle.x3, shapes[i].data.triangle.y3);
                    break;
            }
        }
    }
}

// Safe integer collector with terminal error reporting
int get_int_input(const char* prompt, int min_val, int max_val) {
    char buffer[100];
    int val;
    while (1) {
        printf("%s", prompt);
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            continue;
        }
        char* endptr;
        val = (int)strtol(buffer, &endptr, 10);
        if (endptr == buffer || (*endptr != '\n' && *endptr != '\0')) {
            printf("\033[1;31mError: Invalid number. Please enter a valid integer.\033[0m\n");
            continue;
        }
        if (val < min_val || val > max_val) {
            printf("\033[1;31mError: Value out of range (%d to %d).\033[0m\n", min_val, max_val);
            continue;
        }
        return val;
    }
}

// Format detailed string of shape attributes
void print_shape_details(const Shape* s) {
    switch (s->type) {
        case SHAPE_LINE:
            printf("  [\033[1;36mID: %02d\033[0m] Line: (%d, %d) -> (%d, %d)\n",
                   s->id, s->data.line.x1, s->data.line.y1, s->data.line.x2, s->data.line.y2);
            break;
        case SHAPE_RECTANGLE:
            printf("  [\033[1;36mID: %02d\033[0m] Rectangle: Top-Left (%d, %d), Width %d, Height %d (%s)\n",
                   s->id, s->data.rect.x, s->data.rect.y, s->data.rect.width, s->data.rect.height,
                   s->data.rect.fill ? "Filled" : "Outline");
            break;
        case SHAPE_CIRCLE:
            printf("  [\033[1;36mID: %02d\033[0m] Circle: Center (%d, %d), Radius %d (%s)\n",
                   s->id, s->data.circle.cx, s->data.circle.cy, s->data.circle.r,
                   s->data.circle.fill ? "Filled" : "Outline");
            break;
        case SHAPE_TRIANGLE:
            printf("  [\033[1;36mID: %02d\033[0m] Triangle: Vertices (%d, %d), (%d, %d), (%d, %d)\n",
                   s->id, s->data.triangle.x1, s->data.triangle.y1,
                   s->data.triangle.x2, s->data.triangle.y2,
                   s->data.triangle.x3, s->data.triangle.y3);
            break;
    }
}

// Display Canvas with coordinate numbering
void render_canvas(void) {
    // Top tens header
    printf("   ");
    for (int c = 0; c < CANVAS_COLS; c++) {
        if (c % 10 == 0) {
            printf("\033[1;32m%d\033[0m", c / 10);
        } else {
            printf(" ");
        }
    }
    printf("\n   ");
    
    // Top units header
    for (int c = 0; c < CANVAS_COLS; c++) {
        printf("\033[1;32m%d\033[0m", c % 10);
    }
    printf("\n");

    // Top border
    printf("  \033[1;34m+\033[0m");
    for (int c = 0; c < CANVAS_COLS; c++) printf("\033[1;34m-\033[0m");
    printf("\033[1;34m+\033[0m\n");

    // Row printing
    for (int r = 0; r < CANVAS_ROWS; r++) {
        printf("\033[1;32m%02d\033[0m\033[1;34m|\033[0m", r);
        for (int c = 0; c < CANVAS_COLS; c++) {
            if (canvas[r][c] == CHAR_FG) {
                printf("\033[1;33m%c\033[0m", canvas[r][c]); // Shapes highlighted in yellow
            } else {
                printf("\033[90m%c\033[0m", canvas[r][c]);  // Background in grey
            }
        }
        printf("\033[1;34m|\033[0m\033[1;32m%02d\033[0m\n", r);
    }

    // Bottom border
    printf("  \033[1;34m+\033[0m");
    for (int c = 0; c < CANVAS_COLS; c++) printf("\033[1;34m-\033[0m");
    printf("\033[1;34m+\033[0m\n");

    // Bottom units footer
    printf("   ");
    for (int c = 0; c < CANVAS_COLS; c++) {
        printf("\033[1;32m%d\033[0m", c % 10);
    }
    printf("\n   ");
    
    // Bottom tens footer
    for (int c = 0; c < CANVAS_COLS; c++) {
        if (c % 10 == 0) {
            printf("\033[1;32m%d\033[0m", c / 10);
        } else {
            printf(" ");
        }
    }
    printf("\n");
}

// Action: Add Shape
void add_shape(void) {
    int free_slot = -1;
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (!shapes[i].active) {
            free_slot = i;
            break;
        }
    }

    if (free_slot == -1) {
        printf("\033[1;31mError: Maximum shape count (%d) reached. Please delete some first.\033[0m\n", MAX_SHAPES);
        press_enter_to_continue();
        return;
    }

    printf("\n\033[1;35m--- Add Shape ---\033[0m\n");
    printf("1. Line\n");
    printf("2. Rectangle\n");
    printf("3. Circle\n");
    printf("4. Triangle\n");
    printf("5. Cancel\n");
    int type_choice = get_int_input("Select shape type: ", 1, 5);

    if (type_choice == 5) return;

    Shape new_shape;
    new_shape.id = next_shape_id++;
    new_shape.active = 1;

    switch (type_choice) {
        case 1: {
            new_shape.type = SHAPE_LINE;
            new_shape.data.line.x1 = get_int_input("Start X1 (0-79): ", 0, CANVAS_COLS - 1);
            new_shape.data.line.y1 = get_int_input("Start Y1 (0-23): ", 0, CANVAS_ROWS - 1);
            new_shape.data.line.x2 = get_int_input("End X2 (0-79): ", 0, CANVAS_COLS - 1);
            new_shape.data.line.y2 = get_int_input("End Y2 (0-23): ", 0, CANVAS_ROWS - 1);
            break;
        }
        case 2: {
            new_shape.type = SHAPE_RECTANGLE;
            new_shape.data.rect.x = get_int_input("Top-Left X (0-79): ", 0, CANVAS_COLS - 1);
            new_shape.data.rect.y = get_int_input("Top-Left Y (0-23): ", 0, CANVAS_ROWS - 1);
            new_shape.data.rect.width = get_int_input("Width (1-80): ", 1, CANVAS_COLS);
            new_shape.data.rect.height = get_int_input("Height (1-24): ", 1, CANVAS_ROWS);
            new_shape.data.rect.fill = get_int_input("Fill (0=Outline, 1=Filled): ", 0, 1);
            break;
        }
        case 3: {
            new_shape.type = SHAPE_CIRCLE;
            new_shape.data.circle.cx = get_int_input("Center X (0-79): ", 0, CANVAS_COLS - 1);
            new_shape.data.circle.cy = get_int_input("Center Y (0-23): ", 0, CANVAS_ROWS - 1);
            new_shape.data.circle.r = get_int_input("Radius (0-40): ", 0, 40);
            new_shape.data.circle.fill = get_int_input("Fill (0=Outline, 1=Filled): ", 0, 1);
            break;
        }
        case 4: {
            new_shape.type = SHAPE_TRIANGLE;
            new_shape.data.triangle.x1 = get_int_input("Vertex 1 X1 (0-79): ", 0, CANVAS_COLS - 1);
            new_shape.data.triangle.y1 = get_int_input("Vertex 1 Y1 (0-23): ", 0, CANVAS_ROWS - 1);
            new_shape.data.triangle.x2 = get_int_input("Vertex 2 X2 (0-79): ", 0, CANVAS_COLS - 1);
            new_shape.data.triangle.y2 = get_int_input("Vertex 2 Y2 (0-23): ", 0, CANVAS_ROWS - 1);
            new_shape.data.triangle.x3 = get_int_input("Vertex 3 X3 (0-79): ", 0, CANVAS_COLS - 1);
            new_shape.data.triangle.y3 = get_int_input("Vertex 3 Y3 (0-23): ", 0, CANVAS_ROWS - 1);
            break;
        }
    }

    shapes[free_slot] = new_shape;
    redraw_all_shapes();
    printf("\033[1;32mShape added successfully under ID %d!\033[0m\n", new_shape.id);
    press_enter_to_continue();
}

// Action: Delete Shape
void delete_shape(void) {
    int active_count = 0;
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (shapes[i].active) active_count++;
    }
    if (active_count == 0) {
        printf("\033[1;31mNo shapes currently active to delete.\033[0m\n");
        press_enter_to_continue();
        return;
    }

    printf("\n\033[1;35m--- Delete Shape ---\033[0m\n");
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (shapes[i].active) {
            print_shape_details(&shapes[i]);
        }
    }

    int id = get_int_input("Enter ID of shape to delete: ", 1, next_shape_id - 1);
    int found = 0;
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (shapes[i].active && shapes[i].id == id) {
            shapes[i].active = 0;
            found = 1;
            break;
        }
    }

    if (found) {
        redraw_all_shapes();
        printf("\033[1;32mShape ID %d deleted successfully.\033[0m\n", id);
    } else {
        printf("\033[1;31mError: Shape ID %d not found.\033[0m\n", id);
    }
    press_enter_to_continue();
}

// Action: Modify Shape
void modify_shape(void) {
    int active_count = 0;
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (shapes[i].active) active_count++;
    }
    if (active_count == 0) {
        printf("\033[1;31mNo shapes currently active to modify.\033[0m\n");
        press_enter_to_continue();
        return;
    }

    printf("\n\033[1;35m--- Modify Shape ---\033[0m\n");
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (shapes[i].active) {
            print_shape_details(&shapes[i]);
        }
    }

    int id = get_int_input("Enter ID of shape to modify: ", 1, next_shape_id - 1);
    Shape* target = NULL;
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (shapes[i].active && shapes[i].id == id) {
            target = &shapes[i];
            break;
        }
    }

    if (!target) {
        printf("\033[1;31mError: Shape ID %d not found.\033[0m\n", id);
        press_enter_to_continue();
        return;
    }

    printf("\nModifying Shape:\n");
    print_shape_details(target);

    switch (target->type) {
        case SHAPE_LINE: {
            target->data.line.x1 = get_int_input("New Start X1 (0-79): ", 0, CANVAS_COLS - 1);
            target->data.line.y1 = get_int_input("New Start Y1 (0-23): ", 0, CANVAS_ROWS - 1);
            target->data.line.x2 = get_int_input("New End X2 (0-79): ", 0, CANVAS_COLS - 1);
            target->data.line.y2 = get_int_input("New End Y2 (0-23): ", 0, CANVAS_ROWS - 1);
            break;
        }
        case SHAPE_RECTANGLE: {
            target->data.rect.x = get_int_input("New Top-Left X (0-79): ", 0, CANVAS_COLS - 1);
            target->data.rect.y = get_int_input("New Top-Left Y (0-23): ", 0, CANVAS_ROWS - 1);
            target->data.rect.width = get_int_input("New Width (1-80): ", 1, CANVAS_COLS);
            target->data.rect.height = get_int_input("New Height (1-24): ", 1, CANVAS_ROWS);
            target->data.rect.fill = get_int_input("New Fill (0=Outline, 1=Filled): ", 0, 1);
            break;
        }
        case SHAPE_CIRCLE: {
            target->data.circle.cx = get_int_input("New Center X (0-79): ", 0, CANVAS_COLS - 1);
            target->data.circle.cy = get_int_input("New Center Y (0-23): ", 0, CANVAS_ROWS - 1);
            target->data.circle.r = get_int_input("New Radius (0-40): ", 0, 40);
            target->data.circle.fill = get_int_input("New Fill (0=Outline, 1=Filled): ", 0, 1);
            break;
        }
        case SHAPE_TRIANGLE: {
            target->data.triangle.x1 = get_int_input("New Vertex 1 X1 (0-79): ", 0, CANVAS_COLS - 1);
            target->data.triangle.y1 = get_int_input("New Vertex 1 Y1 (0-23): ", 0, CANVAS_ROWS - 1);
            target->data.triangle.x2 = get_int_input("New Vertex 2 X2 (0-79): ", 0, CANVAS_COLS - 1);
            target->data.triangle.y2 = get_int_input("New Vertex 2 Y2 (0-23): ", 0, CANVAS_ROWS - 1);
            target->data.triangle.x3 = get_int_input("New Vertex 3 X3 (0-79): ", 0, CANVAS_COLS - 1);
            target->data.triangle.y3 = get_int_input("New Vertex 3 Y3 (0-23): ", 0, CANVAS_ROWS - 1);
            break;
        }
    }

    redraw_all_shapes();
    printf("\033[1;32mShape ID %d modified successfully!\033[0m\n", id);
    press_enter_to_continue();
}

// Action: Clear Canvas
void clear_all_shapes(void) {
    for (int i = 0; i < MAX_SHAPES; i++) {
        shapes[i].active = 0;
    }
    next_shape_id = 1;
    redraw_all_shapes();
    printf("\033[1;32mCanvas cleared.\033[0m\n");
    press_enter_to_continue();
}

// Action: Export Canvas to Text File
void save_to_file(void) {
    char filename[128];
    printf("\nEnter output filename (e.g. drawing.txt): ");
    if (!fgets(filename, sizeof(filename), stdin)) return;
    
    filename[strcspn(filename, "\n")] = '\0';
    if (strlen(filename) == 0) {
        printf("\033[1;31mInvalid filename.\033[0m\n");
        return;
    }

    FILE* fp = fopen(filename, "w");
    if (!fp) {
        printf("\033[1;31mError: Could not create/open file '%s' for writing.\033[0m\n", filename);
        press_enter_to_continue();
        return;
    }

    for (int r = 0; r < CANVAS_ROWS; r++) {
        for (int c = 0; c < CANVAS_COLS; c++) {
            fputc(canvas[r][c], fp);
        }
        fputc('\n', fp);
    }
    fclose(fp);

    printf("\033[1;32mDrawing saved successfully to '%s'!\033[0m\n", filename);
    press_enter_to_continue();
}

// Main execution block
int main(void) {
    enable_virtual_terminal();

    for (int i = 0; i < MAX_SHAPES; i++) {
        shapes[i].active = 0;
    }

    clear_canvas();

    // Seeding demo shapes
    shapes[0].id = next_shape_id++;
    shapes[0].type = SHAPE_RECTANGLE;
    shapes[0].active = 1;
    shapes[0].data.rect.x = 2;
    shapes[0].data.rect.y = 1;
    shapes[0].data.rect.width = 76;
    shapes[0].data.rect.height = 22;
    shapes[0].data.rect.fill = 0;

    shapes[1].id = next_shape_id++;
    shapes[1].type = SHAPE_CIRCLE;
    shapes[1].active = 1;
    shapes[1].data.circle.cx = 40;
    shapes[1].data.circle.cy = 12;
    shapes[1].data.circle.r = 6;
    shapes[1].data.circle.fill = 0;

    shapes[2].id = next_shape_id++;
    shapes[2].type = SHAPE_LINE;
    shapes[2].active = 1;
    shapes[2].data.line.x1 = 10;
    shapes[2].data.line.y1 = 4;
    shapes[2].data.line.x2 = 70;
    shapes[2].data.line.y2 = 20;

    redraw_all_shapes();

    while (1) {
        // Clear terminal screen and position cursor to home
        printf("\033[2J\033[H");

        printf("\033[1;36m================================================================================\033[0m\n");
        printf("\033[1;35m                      ★  2D TERMINAL GRAPHICS EDITOR  ★                        \033[0m\n");
        printf("\033[1;36m================================================================================\033[0m\n");

        render_canvas();

        // Print active objects list
        printf("\n\033[1;36mActive Vector Layers:\033[0m\n");
        int count = 0;
        for (int i = 0; i < MAX_SHAPES; i++) {
            if (shapes[i].active) {
                print_shape_details(&shapes[i]);
                count++;
            }
        }
        if (count == 0) {
            printf("  (no layers - canvas is empty)\n");
        }

        // Print interactive actions
        printf("\n\033[1;36mActions:\033[0m\n");
        printf("  \033[1;33m[1]\033[0m Add Shape        \033[1;33m[2]\033[0m Delete Shape     \033[1;33m[3]\033[0m Modify Shape     \033[1;33m[4]\033[0m Clear Canvas\n");
        printf("  \033[1;33m[5]\033[0m Save to Text File\033[1;33m[6]\033[0m Refresh Screen   \033[1;33m[7]\033[0m Exit\n");
        printf("--------------------------------------------------------------------------------\n");

        int action = get_int_input("Choose action: ", 1, 7);

        if (action == 7) {
            printf("\nExiting editor. Thank you for drawing!\n");
            break;
        }

        switch (action) {
            case 1:
                add_shape();
                break;
            case 2:
                delete_shape();
                break;
            case 3:
                modify_shape();
                break;
            case 4:
                clear_all_shapes();
                break;
            case 5:
                save_to_file();
                break;
            case 6:
                redraw_all_shapes();
                break;
            default:
                break;
        }
    }

    return 0;
}
