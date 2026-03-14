#pragma once
#include "buffer.hpp"
#include "config.h"
#include <cstdint>
#include <cstddef>

struct BulletManager;
struct Player;

// ---------------------------------------------------------------------------
// 敌机类型
// ---------------------------------------------------------------------------
enum EnemyType : uint8_t {
    ENEMY_NONE   = 0,
    ENEMY_SMALL  = 1,  // 8x8,  快速直线，低血，直线弹
    ENEMY_MEDIUM = 2,  // 12x10, 迂回/俯冲，中血，扇形弹
    ENEMY_ELITE  = 3,  // 14x12, 正弦路径，高血，环形+跟踪弹
    ENEMY_BOSS   = 4,  // 48x32, 多阶段
};

// BOSS 阶段
enum BossPhase : uint8_t {
    BOSS_ENTER = 0, // 从顶部入场
    BOSS_P1,        // HP 100%-60%：左右摇摆+直线弹幕
    BOSS_P2,        // HP 60%-30%：扇形+环形弹
    BOSS_P3,        // HP 30%-0%：激光+跟踪弹+密集弹幕
    BOSS_DEAD,
};

// ---------------------------------------------------------------------------
// 敌机结构
// ---------------------------------------------------------------------------
struct Enemy {
    EnemyType type;
    int  x, y;           // 左下角坐标
    int  hp, max_hp;
    size_t score_value;

    // 通用 AI 计时
    int  move_timer;
    int  shoot_timer;
    int  move_dir;       // 小型/中型横向移动方向
    float move_phase;    // 精英正弦路径相位

    // BOSS 专用
    BossPhase boss_phase;
    int  boss_timer;     // 当前阶段已过帧数
    int  boss_target_x;  // 移动目标 X

    // 动画
    int  anim_timer;
    int  anim_frame;     // 0 或 1

    // 死亡闪烁
    int  death_timer;    // >0 时播放死亡动画
};

// 敌机死亡事件（用于触发道具掉落、爆炸）
struct EnemyDeathEvent {
    int x, y;
    EnemyType type;
    size_t score;
};

// ---------------------------------------------------------------------------
// 敌机管理器
// ---------------------------------------------------------------------------
struct EnemyManager {
    Enemy  enemies[MAX_ENEMIES];
    size_t num_enemies;
};

void enemy_manager_init(EnemyManager* em);

// 生成敌机
void enemy_spawn(EnemyManager* em, EnemyType type, int x, int y, float diff_mult);

// 每帧更新：AI 移动、射击、死亡动画
// 返回本帧死亡事件数
size_t enemy_update(EnemyManager* em,
                    BulletManager* bm,
                    const Player* player,
                    int screen_w, int screen_h,
                    EnemyDeathEvent* out_deaths,
                    size_t max_deaths);

// 对指定敌机施加伤害，返回 true = 已死亡
bool enemy_deal_damage(EnemyManager* em, int idx, int dmg);

void enemy_render(const EnemyManager* em, Buffer* buf);

// 获取敌机 AABB（供 bullet_update 使用）
void enemy_get_aabbs(const EnemyManager* em,
                     int* out_x, int* out_y,
                     int* out_w, int* out_h,
                     bool* out_alive,
                     int max_count, int* out_count);
