#pragma once

#include <cstddef>
#include <cstdint>

// CPU-side pixel buffer and sprite primitives

struct Buffer {
    size_t width, height;
    uint32_t* data;
};

struct Sprite {
    size_t width, height;
    uint8_t* data; // bitmap: 1 = on, 0 = off
};

uint32_t rgb_to_uint32(uint8_t r, uint8_t g, uint8_t b);

void buffer_clear(Buffer* buffer, uint32_t color);

void buffer_sprite_draw(
    Buffer* buffer,
    const Sprite& sprite,
    size_t x,
    size_t y,
    uint32_t color);

// Text / number drawing and sprite overlap helpers (implemented in buffer.cpp)
void buffer_draw_text(
    Buffer* buffer,
    const Sprite& text_spritesheet,
    const char* text,
    size_t x,
    size_t y,
    uint32_t color);

void buffer_draw_number(
    Buffer* buffer,
    const Sprite& number_spritesheet,
    size_t number,
    size_t x,
    size_t y,
    uint32_t color);

bool sprite_overlap_check(
    const Sprite& sp_a, size_t x_a, size_t y_a,
    const Sprite& sp_b, size_t x_b, size_t y_b);

