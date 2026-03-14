#pragma once
#include "buffer.hpp"
#include "config.h"
#include <cstdint>
#include <cstddef>

struct Player;
struct Wingman;

// ---------------------------------------------------------------------------
// 子弹
// ---------------------------------------------------------------------------
struct Bullet {
    float    x, y;
    float    vx, vy;
    bool     active;
    int      damage;
    uint32_t color;
    int      w, h;   // 精灵尺寸（像素）
};

// ---------------------------------------------------------------------------
// 跟踪导弹
// ---------------------------------------------------------------------------
struct Missile {
    float x, y;
    float vx, vy;
    float angle;       // 当前飞行方向（弧度，0=向上）
    int   target_idx;  // 锁定的敌机索引（-1=自由飞）
    bool  active;
    int   lifetime;    // 剩余存活帧
};

// ---------------------------------------------------------------------------
// 激光束
// ---------------------------------------------------------------------------
struct Laser {
    int  x;           // 激光列 X
    int  y_bottom;    // 起始 Y（玩家位置）
    int  y_top;       // 终止 Y（屏幕顶）
    int  duration;    // 剩余持续帧
    bool active;
    uint32_t color;
};

// ---------------------------------------------------------------------------
// 命中记录
// ---------------------------------------------------------------------------
struct HitRecord {
    int target_idx; // -1 = 玩家，>=0 = 敌机索引
    int damage;
};

// ---------------------------------------------------------------------------
// 子弹管理器
// ---------------------------------------------------------------------------
struct BulletManager {
    Bullet player_bullets[MAX_PLAYER_BULLETS];
    size_t num_player_bullets;

    Bullet enemy_bullets[MAX_ENEMY_BULLETS];
    size_t num_enemy_bullets;

    Missile missiles[MAX_MISSILES];
    size_t  num_missiles;

    Laser lasers[MAX_LASERS];
    size_t num_lasers;
};

void bullet_manager_init(BulletManager* bm);

// 玩家发射（根据 weapon_level 决定弹型）
// wingmen 传入 Wingman[2] 数组，可为 nullptr
void bullet_fire_player(BulletManager* bm,
                        int px, int py,
                        int weapon_level,
                        const Wingman* wingmen);

// 敌机弹幕接口
void bullet_fire_straight(BulletManager* bm, float x, float y,
                          float vx, float vy, int dmg, uint32_t color);
void bullet_fire_fan(BulletManager* bm, float x, float y,
                     int count, float center_angle_rad, float spread_rad,
                     float speed, int dmg, uint32_t color);
void bullet_fire_ring(BulletManager* bm, float x, float y,
                      int count, float speed, int dmg, uint32_t color);
void bullet_fire_laser(BulletManager* bm, int x, int y_bottom,
                       int screen_h, uint32_t color);
void bullet_fire_missile(BulletManager* bm, float x, float y);

// 每帧更新：移动 + 屏幕裁剪 + 碰撞检测
// ex[]/ey[]/ew[]/eh[]：敌机 AABB 数组（enemy_count 个）
// ealive[]：敌机是否存活
// 返回命中记录数量
size_t bullet_update(
    BulletManager* bm,
    int player_x, int player_y, int player_w, int player_h,
    bool player_invincible,
    int* ex, int* ey, int* ew, int* eh,
    bool* ealive, int enemy_count,
    int screen_w, int screen_h,
    HitRecord* out_hits, size_t max_hits);

void bullet_render(const BulletManager* bm, Buffer* buf);
