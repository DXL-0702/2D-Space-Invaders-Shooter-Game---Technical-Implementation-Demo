#include "buffer.hpp"

uint32_t rgb_to_uint32(uint8_t r, uint8_t g, uint8_t b) {
    return (static_cast<uint32_t>(r) << 24) |
           (static_cast<uint32_t>(g) << 16) |
           (static_cast<uint32_t>(b) << 8) |
           255u;
}

void buffer_clear(Buffer* buffer, uint32_t color) {
    const size_t count = buffer->width * buffer->height;
    for (size_t i = 0; i < count; ++i) {
        buffer->data[i] = color;
    }
}

void buffer_sprite_draw(
    Buffer* buffer,
    const Sprite& sprite,
    size_t x,
    size_t y,
    uint32_t color) {
    for (size_t xi = 0; xi < sprite.width; ++xi) {
        for (size_t yi = 0; yi < sprite.height; ++yi) {
            const size_t sy = sprite.height - 1 + y - yi;
            const size_t sx = x + xi;

            if (sprite.data[yi * sprite.width + xi] &&
                sx < buffer->width &&
                sy < buffer->height) {
                buffer->data[sy * buffer->width + sx] = color;
            }
        }
    }
}

void buffer_draw_text(
    Buffer* buffer,
    const Sprite& text_spritesheet,
    const char* text,
    size_t x,
    size_t y,
    uint32_t color) {
    size_t xp = x;
    const size_t stride = text_spritesheet.width * text_spritesheet.height;
    Sprite glyph = text_spritesheet;

    for (const char* ch = text; *ch != '\0'; ++ch) {
        char c = static_cast<char>(*ch - 32);
        if (c < 0 || c >= 65) continue;

        glyph.data = text_spritesheet.data + static_cast<size_t>(c) * stride;
        buffer_sprite_draw(buffer, glyph, xp, y, color);
        xp += glyph.width + 1;
    }
}

void buffer_draw_number(
    Buffer* buffer,
    const Sprite& number_spritesheet,
    size_t number,
    size_t x,
    size_t y,
    uint32_t color) {
    uint8_t digits[64];
    size_t num_digits = 0;

    size_t current = number;
    do {
        digits[num_digits++] = static_cast<uint8_t>(current % 10);
        current /= 10;
    } while (current > 0);

    size_t xp = x;
    const size_t stride = number_spritesheet.width * number_spritesheet.height;
    Sprite glyph = number_spritesheet;

    for (size_t i = 0; i < num_digits; ++i) {
        const uint8_t digit = digits[num_digits - 1 - i];
        glyph.data = number_spritesheet.data + static_cast<size_t>(digit) * stride;
        buffer_sprite_draw(buffer, glyph, xp, y, color);
        xp += glyph.width + 1;
    }
}

bool sprite_overlap_check(
    const Sprite& sp_a, size_t x_a, size_t y_a,
    const Sprite& sp_b, size_t x_b, size_t y_b) {
    if (x_a < x_b + sp_b.width && x_a + sp_a.width > x_b &&
        y_a < y_b + sp_b.height && y_a + sp_a.height > y_b) {
        return true;
    }
    return false;
}