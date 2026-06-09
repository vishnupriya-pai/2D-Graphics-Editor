#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "editor.h"

#ifdef _WIN32
#include <windows.h>
// Enable Virtual Terminal Processing for ANSI Escapes on Windows
void enable_virtual_terminal(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}
#else
void enable_virtual_terminal(void) {
    // Non-Windows terminals usually support ANSI sequences out of the box
}
#endif

// Safe integer input collector with validation
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
        // If no conversion happened or extra garbage characters are present (excluding newline)
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

// Print details of a specific shape
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

// Render canvas grid to console
void render_canvas(void) {
    // Header showing X tens values
    printf("   ");
    for (int c = 0; c < CANVAS_COLS; c++) {
        if (c % 10 == 0) {
            printf("\033[1;32m%d\033[0m", c / 10);
        } else {
            printf(" ");
        }
    }
    printf("\n   ");
    
    // Header showing X units values
    for (int c = 0; c < CANVAS_COLS; c++) {
        printf("\033[1;32m%d\033[0m", c % 10);
    }
    printf("\n");

    // Top border line
    printf("  \033[1;34m+\033[0m");
    for (int c = 0; c < CANVAS_COLS; c++) printf("\033[1;34m-\033[0m");
    printf("\033[1;34m+\033[0m\n");

    // Print rows
    for (int r = 0; r < CANVAS_ROWS; r++) {
        printf("\033[1;32m%02d\033[0m\033[1;34m|\033[0m", r);
        for (int c = 0; c < CANVAS_COLS; c++) {
            if (canvas[r][c] == CHAR_FG) {
                printf("\033[1;33m%c\033[0m", canvas[r][c]); // Yellow shapes
            } else {
                printf("\033[90m%c\033[0m", canvas[r][c]);  // Grey background
            }
        }
        printf("\033[1;34m|\033[0m\033[1;32m%02d\033[0m\n", r);
    }

    // Bottom border line
    printf("  \033[1;34m+\033[0m");
    for (int c = 0; c < CANVAS_COLS; c++) printf("\033[1;34m-\033[0m");
    printf("\033[1;34m+\033[0m\n");

    // Footer units values
    printf("   ");
    for (int c = 0; c < CANVAS_COLS; c++) {
        printf("\033[1;32m%d\033[0m", c % 10);
    }
    printf("\n   ");
    
    // Footer tens values
    for (int c = 0; c < CANVAS_COLS; c++) {
        if (c % 10 == 0) {
            printf("\033[1;32m%d\033[0m", c / 10);
        } else {
            printf(" ");
        }
    }
    printf("\n");
}

// Add a shape to the array
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
        printf("Press Enter to continue...");
        char temp[10];
        fgets(temp, sizeof(temp), stdin);
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
    printf("Press Enter to continue...");
    char temp[10];
    fgets(temp, sizeof(temp), stdin);
}

// Delete shape by ID
void delete_shape(void) {
    int active_count = 0;
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (shapes[i].active) active_count++;
    }
    if (active_count == 0) {
        printf("\033[1;31mNo shapes currently active to delete.\033[0m\n");
        printf("Press Enter to continue...");
        char temp[10];
        fgets(temp, sizeof(temp), stdin);
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
    printf("Press Enter to continue...");
    char temp[10];
    fgets(temp, sizeof(temp), stdin);
}

// Modify shape parameters
void modify_shape(void) {
    int active_count = 0;
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (shapes[i].active) active_count++;
    }
    if (active_count == 0) {
        printf("\033[1;31mNo shapes currently active to modify.\033[0m\n");
        printf("Press Enter to continue...");
        char temp[10];
        fgets(temp, sizeof(temp), stdin);
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
        printf("Press Enter to continue...");
        char temp[10];
        fgets(temp, sizeof(temp), stdin);
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
    printf("Press Enter to continue...");
    char temp[10];
    fgets(temp, sizeof(temp), stdin);
}

// Clear all active shapes
void clear_all_shapes(void) {
    for (int i = 0; i < MAX_SHAPES; i++) {
        shapes[i].active = 0;
    }
    next_shape_id = 1;
    redraw_all_shapes();
    printf("\033[1;32mCanvas cleared.\033[0m\n");
    printf("Press Enter to continue...");
    char temp[10];
    fgets(temp, sizeof(temp), stdin);
}

// Save the raw text output to a file
void save_to_file(void) {
    char filename[128];
    printf("\nEnter output filename (e.g. drawing.txt): ");
    if (!fgets(filename, sizeof(filename), stdin)) return;
    
    // Strip trailing newline
    filename[strcspn(filename, "\n")] = '\0';
    if (strlen(filename) == 0) {
        printf("\033[1;31mInvalid filename.\033[0m\n");
        return;
    }

    FILE* fp = fopen(filename, "w");
    if (!fp) {
        printf("\033[1;31mError: Could not create/open file '%s' for writing.\033[0m\n", filename);
        printf("Press Enter to continue...");
        char temp[10];
        fgets(temp, sizeof(temp), stdin);
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
    printf("Press Enter to continue...");
    char temp[10];
    fgets(temp, sizeof(temp), stdin);
}

int main(void) {
    // Setup virtual terminal to get colors in command line
    enable_virtual_terminal();

    // Clear shape records
    for (int i = 0; i < MAX_SHAPES; i++) {
        shapes[i].active = 0;
    }

    clear_canvas();

    // Seed some initial demo shapes to make the editor immediately lively
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
        // Clear terminal screen and return cursor to home
        printf("\033[2J\033[H");

        printf("\033[1;36m================================================================================\033[0m\n");
        printf("\033[1;35m                      ★  2D TERMINAL GRAPHICS EDITOR  ★                        \033[0m\n");
        printf("\033[1;36m================================================================================\033[0m\n");

        render_canvas();

        // Print list of shapes
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

        // Print Main Menu
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
