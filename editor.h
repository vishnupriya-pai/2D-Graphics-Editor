#ifndef EDITOR_H
#define EDITOR_H

#define CANVAS_ROWS 24
#define CANVAS_COLS 80
#define MAX_SHAPES 100

// Character representation
#define CHAR_BG '_'
#define CHAR_FG '*'

typedef enum {
    SHAPE_LINE,
    SHAPE_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_TRIANGLE
} ShapeType;

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

// Canvas representation
extern char canvas[CANVAS_ROWS][CANVAS_COLS];

// Shape database
extern Shape shapes[MAX_SHAPES];
extern int next_shape_id;

// Basic canvas functions
void clear_canvas(void);
void render_canvas(void);
void plot_pixel(int x, int y);

// Drawing algorithms
void draw_line(int x1, int y1, int x2, int y2);
void draw_rectangle(int x, int y, int w, int h, int fill);
void draw_circle(int cx, int cy, int r, int fill);
void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3);

// Shape management
void redraw_all_shapes(void);

#endif // EDITOR_H
