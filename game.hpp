#pragma once
#include "buffer.hpp"
#include "player.hpp"
#include "enemy.hpp"
#include "bullet.hpp"
#include "item.hpp"
#include "ui.hpp"
#include "stage.hpp"
#include "config.h"
#include <cstddef>

struct InputState;

// 游戏全局阶段
enum GamePhase {
    PHASE_TITLE,    // 主菜单
    PHASE_PLAYING,  // 游戏中
    PHASE_PAUSE,    // 暂停
    PHASE_CLEAR,    // 关卡结算
    PHASE_WIN,      // 全通关胜利
    PHASE_LOSE,     // 失败
};

// 滚动星空粒子
struct Star { int x,y,speed; uint8_t brightness; };

struct Game {
    int       width, height;
    GamePhase phase;
    Difficulty difficulty;
    int       stage;        // 当前关卡 1-3
    int       frame_count;

    // 得分
    size_t score;
    int    combo;
    int    combo_timer;
    int    kills;
    int    shots_fired;
    int    shots_hit;
    int    stage_timer;

    // 屏幕震动
    int    shake_timer;
    int    shake_x, shake_y;

    // 子系统
    Player        player;
    EnemyManager  enemies;
    BulletManager bullets;
    ItemManager   items;
    StageManager  stage_mgr;

    // 星空
    Star stars[128];

    // 结算
    ClearStats clear_stats;

    // 菜单
    int menu_diff_sel;
};

void game_init(Game* g, int width, int height);
void game_update(Game* g, InputState* input);
void game_render(const Game* g, Buffer* buf);
void game_shutdown(Game* g);
