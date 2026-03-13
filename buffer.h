#ifndef BUFFER_H
#define BUFFER_H

#include <cstdint>
#include <cstdlib>

// 缓冲区结构，用于像素数据存储
struct Buffer {
    size_t width, height;
    uint32_t* data;
};

// 精灵结构
struct Sprite {
    size_t width, height;
    uint8_t* data; // 位图数据：1 表示开启， 0 表示关闭
};

// 颜色转换函数
uint32_t rgb_to_uint32(uint8_t r, uint8_t g, uint8_t b);

// 缓冲区操作
void buffer_clear(Buffer* buffer, uint32_t color);
void buffer_sprite_draw(Buffer* buffer, const Sprite& sprite, size_t x, size_t y, uint32_t color);

#endif // BUFFER_H