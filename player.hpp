#pragma once
#include "buffer.hpp"
#include "config.h"
#include <cstdint>

struct InputState;
struct BulletManager;

// ---------------------------------------------------------------------------
// 僚机
// ---------------------------------------------------------------------------
struct Wingman {
    bool active;
    int  offset_x;    // 相对玩家中心的 X 偏移（负=左，正=右）
    int  fire_timer;
};

// ---------------------------------------------------------------------------
// 玩家飞船
// ---------------------------------------------------------------------------
struct Player {
    int  x, y;               // 飞船左下角坐标
    int  hp;                 // 当前血量
    int  shield;             // 当前护盾值
    int  invincible_timer;   // 无敌帧倒计时（>0=无敌）
    int  weapon_level;       // 武器等级 1-5
    int  fire_timer;         // 射击冷却倒计时
    bool slow_mode;          // 当前是否慢速
    Wingman wingmen[2];      // 最多 2 架僚机
};

// 初始化玩家（屏幕中下方）
void player_init(Player* p, int screen_w, int screen_h);

// 每帧更新：移动 + 射击冷却 + 无敌帧
// 返回 true 表示本帧应发射
bool player_update(Player* p, const InputState* input, int screen_w, int screen_h);

// 渲染玩家 + 僚机（无敌帧时闪烁）
void player_render(const Player* p, Buffer* buf, int frame_count);

// 受击处理：护盾优先吸收，护盾耗尽才扣 HP，无敌帧内免疫
// 返回 true = 玩家死亡
bool player_take_damage(Player* p, int dmg);

// 武器升级（最高 Lv5）
void player_weapon_up(Player* p);

// 护盾回复
void player_add_shield(Player* p, int amount);

// 回血
void player_add_hp(Player* p, int amount);

// 添加僚机（若已有 2 架则无效）
void player_add_wingman(Player* p);

// 获取飞船精灵宽高（用于碰撞）
int player_sprite_width();
int player_sprite_height();
