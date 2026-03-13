#ifndef GAME_H
#define GAME_H

#include <vector>
#include <cstdint>
#include "buffer.h"

// 前向声明，避免在头文件中直接包含 GLFW 头
struct GLFWwindow;

// 外星人结构体
struct Alien {
    size_t x, y;
    uint8_t type; // 0或1用于动画
};

// 子弹结构体
struct Bullet {
    size_t x, y;
    int dir; // 方向：1=上，-1=下
};

// 游戏状态结构体
struct GameState {
    bool running;
    size_t score;
    int player_lives;
    size_t player_x;
    std::vector<Alien> aliens;
    std::vector<Bullet> bullets;
    int alien_dir;
    size_t alien_move_timer;
    size_t alien_bullet_timer;
};

// 游戏逻辑函数
void init_game(GameState& state, const Buffer& buffer);
void update_game(GameState& state, const Buffer& buffer, GLFWwindow* window);
void render_game(const GameState& state, Buffer& buffer);

#endif // GAME_H