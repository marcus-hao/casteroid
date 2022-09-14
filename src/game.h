#ifndef GAME_H
#define GAME_H

#include <cstdint>

typedef struct {
    uint_fast16_t x;
    uint_fast16_t y;
} vec2ui;

typedef struct {
    int_fast16_t x;
    int_fast16_t y;
} vec2i;

typedef struct {
    vec2i offset;
    vec2i bounds;

    int_fast16_t top() { return offset.y; }
    int_fast16_t bot() { return offset.y + bounds.y; }
    int_fast16_t left() { return offset.x; }
    int_fast16_t right() { return offset.x + bounds.x; }

    int_fast16_t width() { return bounds.x; };
    int_fast16_t height() { return bounds.y; };

    bool contains(vec2i a) { return (a.x >= offset.x && a.x < right()) &&
                                    (a.y >= offset.y && a.y < bot()); }
    
} rect;

int init();
void run();
void close();

#endif