#pragma once
#include "enemy.hpp"
#include "config.h"
#include <cstddef>

// 难度等级
enum Difficulty { DIFF_EASY=0, DIFF_NORMAL=1, DIFF_HARD=2 };

// 关卡状态
enum StagePhase {
    STAGE_WAVE,    // 正常波次生成敌机
    STAGE_BOSS,    // BOSS 战
    STAGE_CLEAR,   // 本关通关（等待过渡）
};

struct StageManager {
    int        stage;          // 当前关卡 1-3
    Difficulty diff;
    StagePhase phase;
    int        wave;           // 当前波次
    int        spawn_timer;    // 生成计时
    int        clear_timer;    // 通关过渡计时
    bool       boss_spawned;
    float      diff_hp_mult;   // 敌机血量倍率
    float      diff_spd_mult;  // 弹速倍率（传给 bullet）
};

void stage_init(StageManager* sm, int stage, Difficulty diff);

// 每帧更新：负责按波次生成敌机
// 当本关所有敌机消灭后推进到 BOSS 或 CLEAR
void stage_update(StageManager* sm, EnemyManager* em, int screen_w, int screen_h);

// 返回当前关卡背景颜色（用于 buffer_clear）
uint32_t stage_bg_color(int stage);
