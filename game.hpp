#pragma once

#include <cstddef>
#include <cstdint>
#include "buffer.hpp"

struct InputState;

#define GAME_MAX_BULLETS 256

// 外星人类型
enum AlienType : uint8_t {
    ALIEN_DEAD   = 0,
    ALIEN_TYPE_A = 1,  // 8x8,  30分
    ALIEN_TYPE_B = 2,  // 11x8, 20分
    ALIEN_TYPE_C = 3,  // 12x8, 10分
};

// 游戏阶段
enum GamePhase {
    PHASE_PLAYING,
    PHASE_WIN,
    PHASE_LOSE,
};

struct Alien {
    size_t x, y;
    uint8_t type;
};

struct Player {
    size_t x, y;
    int hp;   // 血量 0-100
};

struct Bullet {
    size_t x, y;
    int dir;  // 正=向上，负=向下
};

struct SpriteAnimation {
    bool loop;
    size_t num_frames;
    size_t frame_duration;
    size_t time;
    const Sprite** frames;
};

struct Game {
    size_t width, height;

    // --- 精灵组 ---
    // 外星人组
    size_t num_aliens;
    Alien*   aliens;
    uint8_t* death_counters;

    // 玩家组
    Player player;

    // 玩家子弹组
    Bullet player_bullets[GAME_MAX_BULLETS];
    size_t num_player_bullets;

    // 敌机子弹组
    Bullet enemy_bullets[GAME_MAX_BULLETS];
    size_t num_enemy_bullets;

    // --- 得分与状态 ---
    size_t    score;
    GamePhase phase;

    // --- 外星人移动 ---
    int    alien_dir;
    size_t alien_move_timer;

    // --- 敌机射击计时 ---
    size_t alien_shoot_timer;
};

void game_init(Game* game, size_t width, size_t height);
void game_update(Game* game, InputState* input);
void game_render(const Game* game, Buffer* buffer);
void game_shutdown(Game* game);
