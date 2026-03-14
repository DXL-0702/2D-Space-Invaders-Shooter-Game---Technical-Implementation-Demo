#pragma once
#include <cstddef>
#include <cstdint>

// ==========================================================================
// 全局数值配置 — 修改此文件即可调整游戏平衡性
// ==========================================================================

// --------------------------------------------------------------------------
// 窗口
// --------------------------------------------------------------------------
static const size_t SCREEN_W = 640;
static const size_t SCREEN_H = 480;

// --------------------------------------------------------------------------
// 玩家
// --------------------------------------------------------------------------
static const int PLAYER_SPEED_NORMAL  = 4;
static const int PLAYER_SPEED_SLOW    = 2;
static const int PLAYER_MAX_HP        = 100;
static const int PLAYER_MAX_SHIELD    = 60;
static const int PLAYER_INVINCIBLE_F  = 90;   // 受击后无敌帧数
static const int PLAYER_FIRE_RATE[5]  = {8, 6, 5, 4, 3}; // 各武器等级射击间隔
static const int WINGMAN_OFFSET_X     = 18;   // 僚机相对玩家中心的 X 偏移
static const int WINGMAN_FIRE_RATE    = 10;
static const int SHIELD_HIT_ABSORB    = 1;    // 护盾每次吸收的伤害次数

// --------------------------------------------------------------------------
// 子弹
// --------------------------------------------------------------------------
static const size_t MAX_PLAYER_BULLETS = 256;
static const size_t MAX_ENEMY_BULLETS  = 256;
static const size_t MAX_MISSILES       = 16;
static const size_t MAX_LASERS         = 4;
static const int PLAYER_BULLET_SPEED   = 7;
static const int ENEMY_BULLET_SPEED    = 3;
static const int MISSILE_SPEED         = 2;
static const float MISSILE_TURN_RAD    = 0.05f; // 每帧最大转向弧度
static const int MISSILE_LIFETIME      = 300;
static const int LASER_DURATION        = 60;    // 激光持续帧数

// --------------------------------------------------------------------------
// 敌机
// --------------------------------------------------------------------------
static const size_t MAX_ENEMIES      = 64;
static const int SMALL_HP            = 1;
static const int MEDIUM_HP           = 3;
static const int ELITE_HP            = 8;
static const int BOSS_HP_BASE        = 200;  // 普通难度 BOSS 血量
static const size_t SMALL_SCORE      = 100;
static const size_t MEDIUM_SCORE     = 300;
static const size_t ELITE_SCORE      = 800;
static const size_t BOSS_SCORE       = 5000;
static const int SMALL_FIRE_RATE     = 60;   // 小型杂兵射击间隔
static const int MEDIUM_FIRE_RATE    = 45;
static const int ELITE_FIRE_RATE     = 30;
static const int BOSS_FIRE_RATE      = 20;
static const int SMALL_MOVE_SPEED    = 2;
static const int MEDIUM_MOVE_SPEED   = 1;
static const int ELITE_MOVE_SPEED    = 2;
static const int BOSS_MOVE_SPEED     = 1;

// --------------------------------------------------------------------------
// 道具
// --------------------------------------------------------------------------
static const size_t MAX_ITEMS      = 32;
static const int ITEM_DROP_CHANCE  = 30;  // 百分比 0-100
static const int ITEM_LIFETIME     = 180; // 存在帧数
static const int ITEM_BLINK_START  = 60;  // 剩余多少帧开始闪烁
static const int ITEM_FALL_SPEED   = 1;   // 道具下落速度
static const int ITEM_HEAL_AMOUNT  = 30;  // 回血道具回复量
static const int ITEM_SHIELD_AMOUNT= 60;  // 护盾道具回复量

// --------------------------------------------------------------------------
// 特效
// --------------------------------------------------------------------------
static const size_t MAX_EXPLOSIONS   = 32;
static const size_t MAX_PARTICLES    = 128;
static const int EXPLOSION_FRAMES    = 4;    // 爆炸动画帧数
static const int EXPLOSION_FRAME_DUR = 4;    // 每帧持续多少游戏帧
static const int SHAKE_MAX_PIXELS    = 5;    // 最大屏幕震动像素
static const int SHAKE_DURATION      = 8;    // 震动持续帧数

// --------------------------------------------------------------------------
// 关卡 / 难度
// --------------------------------------------------------------------------
static const int NUM_STAGES       = 3;
static const int COMBO_TIMEOUT    = 180;  // 连杀超时重置帧数
static const int COMBO_BONUS_STEP = 10;   // 每连续击杀多少架额外奖励
static const size_t COMBO_BONUS   = 50;   // 额外奖励分

// 难度系数（影响敌机血量和弹速）
static const float DIFF_HP_MULT[3]    = {0.6f, 1.0f, 1.5f};  // 简单/普通/困难
static const float DIFF_SPEED_MULT[3] = {0.7f, 1.0f, 1.3f};
