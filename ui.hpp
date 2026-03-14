#pragma once
#include "buffer.hpp"
#include "player.hpp"
#include <cstddef>

// 评分等级
enum Grade { GRADE_S=0, GRADE_A, GRADE_B, GRADE_C };

// 关卡结算数据
struct ClearStats {
    size_t score;
    int    kills;
    int    hp_remaining;
    int    time_frames;   // 通关用时（帧数）
    int    shots_fired;
    int    shots_hit;
    Grade  grade;
};

// 计算评分
Grade ui_calc_grade(const ClearStats* s);

// 渲染 HUD（每帧调用）
void ui_render_hud(Buffer* buf,
                   size_t score, int combo,
                   const Player* player,
                   int stage, int weapon_level,
                   int frame_count);

// 渲染暂停界面
void ui_render_pause(Buffer* buf);

// 渲染结算界面
void ui_render_clear(Buffer* buf, const ClearStats* s);

// 渲染主菜单
void ui_render_title(Buffer* buf, int selected_diff, int frame_count);

// 渲染胜负横幅
void ui_render_result(Buffer* buf, bool win, size_t score);
