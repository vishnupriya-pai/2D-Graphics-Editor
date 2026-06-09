#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "editor.h"

// Canvas state
char canvas[CANVAS_ROWS][CANVAS_COLS];

// Shape database state
Shape shapes[MAX_SHAPES];
int next_shape_id = 1;

// Fill the canvas with background character
void clear_canvas(void) {
    for (int r = 0; r < CANVAS_ROWS; r++) {
        for (int c = 0; c < CANVAS_COLS; c++) {
            canvas[r][c] = CHAR_BG;
        }
    }
}

// Safely draw a character if coordinates are inside boundary
void plot_pixel(int x, int y) {
    if (x >= 0 && x < CANVAS_COLS && y >= 0 && y < CANVAS_ROWS) {
        canvas[y][x] = CHAR_FG;
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

// Draw Rectangle (filled or outlined)
void draw_rectangle(int x, int y, int w, int h, int fill) {
    if (w <= 0 || h <= 0) return;

    if (fill) {
        for (int r = y; r < y + h; r++) {
            for (int c = x; c < x + w; c++) {
                plot_pixel(c, r);
            }
        }
    } else {
        // Draw top and bottom borders
        for (int c = x; c < x + w; c++) {
            plot_pixel(c, y);
            plot_pixel(c, y + h - 1);
        }
        // Draw left and right borders
        for (int r = y; r < y + h; r++) {
            plot_pixel(x, r);
            plot_pixel(x + w - 1, r);
        }
    }
}

// Draw Circle (filled or outlined)
void draw_circle(int cx, int cy, int r, int fill) {
    if (r < 0) return;
    if (r == 0) {
        plot_pixel(cx, cy);
        return;
    }

    if (fill) {
        for (int dy = -r; dy <= r; dy++) {
            for (int dx = -r; dx <= r; dx++) {
                // Approximate circle mapping for grid
                // Adjusting coefficient for terminal cell aspect ratio (typically height > width, e.g., 2:1)
                // Since columns are narrow and rows are tall, a physical circle looks like a vertical ellipse.
                // We stretch the X axis (or squash the Y axis) to make it look rounder on typical terminal grids.
                // Let's use standard scaling: x_aspect = 2.0. So we check dx^2 + (dy * 2.0)^2 <= r^2.
                // Actually, standard equations dx*dx + dy*dy <= r*r is fine if we ignore aspect ratio, 
                // but let's provide a slightly compensated circular equation for terminals, or keep it standard.
                // Let's keep it standard math, but allow the user to see standard mathematical circle.
                if (dx * dx + dy * dy <= r * r) {
                    plot_pixel(cx + dx, cy + dy);
                }
            }
        }
    } else {
        // Midpoint circle algorithm
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

// Draw Triangle (draw outline lines)
void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3) {
    draw_line(x1, y1, x2, y2);
    draw_line(x2, y2, x3, y3);
    draw_line(x3, y3, x1, y1);
}

// Redraw all active shapes onto a cleared canvas
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
