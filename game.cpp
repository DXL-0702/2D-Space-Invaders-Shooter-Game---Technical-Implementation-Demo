#include "game.hpp"
#include "input.hpp"
#include "config.h"
#include <cstdlib>
#include <ctime>
#include <cstring>

// ---------------------------------------------------------------------------
// 星空初始化
// ---------------------------------------------------------------------------
static void stars_init(Game* g){
    for(int i=0;i<128;++i){
        g->stars[i].x = rand()%g->width;
        g->stars[i].y = rand()%g->height;
        g->stars[i].speed = 1 + rand()%3;
        g->stars[i].brightness = (uint8_t)(80 + rand()%176);
    }
}

static void stars_update(Game* g){
    for(int i=0;i<128;++i){
        g->stars[i].y -= g->stars[i].speed;
        if(g->stars[i].y < 0){
            g->stars[i].y = g->height;
            g->stars[i].x = rand()%g->width;
        }
    }
}

static void stars_render(const Game* g, Buffer* buf){
    for(int i=0;i<128;++i){
        int x=g->stars[i].x + g->shake_x;
        int y=g->stars[i].y + g->shake_y;
        if(x<0||y<0||x>=(int)buf->width||y>=(int)buf->height) continue;
        uint8_t b=g->stars[i].brightness;
        buf->data[y*buf->width+x]=rgb_to_uint32(b,b,b);
    }
}

// ---------------------------------------------------------------------------
void game_init(Game* g, int width, int height){
    memset(g, 0, sizeof(*g));
    srand((unsigned)time(nullptr));
    g->width  = width;
    g->height = height;
    g->phase  = PHASE_TITLE;
    g->difficulty = DIFF_NORMAL;
    g->stage  = 1;
    g->menu_diff_sel = 1;
    stars_init(g);
}

static void start_stage(Game* g){
    player_init(&g->player, g->width, g->height);
    enemy_manager_init(&g->enemies);
    bullet_manager_init(&g->bullets);
    item_manager_init(&g->items);
    stage_init(&g->stage_mgr, g->stage, g->difficulty);
    g->combo       = 0;
    g->combo_timer = 0;
    g->stage_timer = 0;
    g->shake_timer = 0;
    g->shake_x     = 0;
    g->shake_y     = 0;
    g->shots_fired = 0;
    g->shots_hit   = 0;
    g->phase       = PHASE_PLAYING;
}

// ---------------------------------------------------------------------------
void game_update(Game* g, InputState* input){
    ++g->frame_count;
    stars_update(g);

    // 屏幕震动
    if(g->shake_timer>0){
        --g->shake_timer;
        g->shake_x = (rand()%SHAKE_MAX_PIXELS*2)-SHAKE_MAX_PIXELS;
        g->shake_y = (rand()%SHAKE_MAX_PIXELS*2)-SHAKE_MAX_PIXELS;
    } else { g->shake_x=0; g->shake_y=0; }

    // --- PHASE_TITLE ---
    if(g->phase==PHASE_TITLE){
        if(input){
            if(input->menu_right){ g->menu_diff_sel=(g->menu_diff_sel+1)%3; input->menu_right=false; }
            if(input->menu_left) { g->menu_diff_sel=(g->menu_diff_sel+2)%3; input->menu_left=false;  }
            input->move_x=0;
            if(input->fire_hold||input->fire_pressed){
                g->difficulty=(Difficulty)g->menu_diff_sel;
                g->score=0; g->kills=0; g->stage=1;
                start_stage(g);
                // 清零所有输入状态，防止残留按键影响游戏
                input->fire_pressed=false;
                input->fire_hold=false;
                input->move_x=0;
                input->move_y=0;
                input->move_dir=0;
                input->slow_hold=false;
            }
        }
        return;
    }

    // --- PHASE_PAUSE ---
    if(g->phase==PHASE_PAUSE){
        if(input&&input->pause_pressed){
            g->phase=PHASE_PLAYING;
            input->pause_pressed=false;
        }
        return;
    }

    // --- PHASE_CLEAR ---
    if(g->phase==PHASE_CLEAR){
        if(g->clear_stats.time_frames==0){
            g->clear_stats.score       = g->score;
            g->clear_stats.kills       = g->kills;
            g->clear_stats.hp_remaining= g->player.hp;
            g->clear_stats.time_frames = g->stage_timer;
            g->clear_stats.shots_fired = g->shots_fired;
            g->clear_stats.shots_hit   = g->shots_hit;
            g->clear_stats.grade       = ui_calc_grade(&g->clear_stats);
        }
        // 180帧后自动推进
        ++g->stage_mgr.clear_timer;
        if(input&&(input->fire_hold||input->fire_pressed)||
           g->stage_mgr.clear_timer>180){
            if(input) input->fire_pressed=false;
            if(g->stage<NUM_STAGES){
                ++g->stage;
                start_stage(g);
            } else {
                g->phase=PHASE_WIN;
            }
        }
        return;
    }

    // --- PHASE_WIN / PHASE_LOSE ---
    if(g->phase==PHASE_WIN||g->phase==PHASE_LOSE){
        // 空格键重新开始：回到主菜单
        if(input&&(input->fire_hold||input->fire_pressed)){
            input->fire_pressed=false;
            input->fire_hold=false;
            input->move_x=0; input->move_y=0;
            input->move_dir=0; input->slow_hold=false;
            // 重置游戏状态回主菜单
            g->phase=PHASE_TITLE;
            g->score=0; g->kills=0; g->stage=1;
            g->frame_count=0;
        }
        return;
    }

    // --- PHASE_PLAYING ---
    ++g->stage_timer;

    // 暂停
    if(input&&input->pause_pressed){
        g->phase=PHASE_PAUSE;
        input->pause_pressed=false;
        return;
    }

    // 连杀超时
    if(g->combo>0){
        if(--g->combo_timer<=0){ g->combo=0; g->combo_timer=0; }
    }

    // 玩家更新
    bool should_fire = player_update(&g->player, input, g->width, g->height);
    if(should_fire){
        int pw=player_sprite_width(), ph=player_sprite_height();
        bullet_fire_player(&g->bullets,
            g->player.x, g->player.y+ph,
            g->player.weapon_level, g->player.wingmen);
        ++g->shots_fired;
    }
    // 僚机射击冷却推进
    for(int i=0;i<2;++i)
        if(g->player.wingmen[i].active&&g->player.wingmen[i].fire_timer>0)
            --g->player.wingmen[i].fire_timer;

    // 关卡更新（生成敌机）
    stage_update(&g->stage_mgr, &g->enemies, g->width, g->height);

    // 关卡通关检测
    if(g->stage_mgr.phase==STAGE_CLEAR){
        g->phase=PHASE_CLEAR;
        memset(&g->clear_stats,0,sizeof(g->clear_stats));
        return;
    }

    // 获取敌机 AABB
    static int ex[MAX_ENEMIES],ey[MAX_ENEMIES],ew2[MAX_ENEMIES],eh2[MAX_ENEMIES];
    static bool ealive[MAX_ENEMIES];
    int ecount=0;
    enemy_get_aabbs(&g->enemies,ex,ey,ew2,eh2,ealive,(int)MAX_ENEMIES,&ecount);

    // 子弹更新 + 碰撞
    static HitRecord hits[512];
    int pw=player_sprite_width(), ph2=player_sprite_height();
    size_t nhits = bullet_update(
        &g->bullets,
        g->player.x, g->player.y, pw, ph2,
        g->player.invincible_timer>0,
        ex,ey,ew2,eh2, ealive, ecount,
        g->width, g->height,
        hits, 512);

    // 处理命中结果
    for(size_t hi=0;hi<nhits;++hi){
        if(hits[hi].target_idx==-1){
            // 玩家受击
            bool dead = player_take_damage(&g->player, hits[hi].damage*20);
            g->shake_timer = SHAKE_DURATION;
            if(dead){ g->phase=PHASE_LOSE; return; }
        } else {
            int idx=hits[hi].target_idx;
            bool killed = enemy_deal_damage(&g->enemies,idx,hits[hi].damage);
            if(killed){
                ++g->shots_hit;
                // 不在这里加分，等死亡事件
            }
        }
    }

    // 敌机更新（AI + 射击 + 死亡事件）
    static EnemyDeathEvent deaths[MAX_ENEMIES];
    size_t ndeaths = enemy_update(&g->enemies, &g->bullets, &g->player,
                                  g->width, g->height, deaths, MAX_ENEMIES);
    for(size_t di=0;di<ndeaths;++di){
        g->score += deaths[di].score;
        ++g->kills;
        // 连杀
        ++g->combo;
        g->combo_timer = COMBO_TIMEOUT;
        if(g->combo>0 && g->combo%COMBO_BONUS_STEP==0)
            g->score += COMBO_BONUS;
        // 道具掉落
        item_try_spawn(&g->items, deaths[di].x, deaths[di].y);
        // 爆炸震动
        if(deaths[di].type==ENEMY_BOSS||deaths[di].type==ENEMY_ELITE)
            g->shake_timer=SHAKE_DURATION*2;
        else
            g->shake_timer=SHAKE_DURATION;
    }

    // 道具更新
    int picked = item_update(&g->items,
        g->player.x, g->player.y,
        player_sprite_width(), player_sprite_height());
    if(picked>=0){
        switch((ItemType)picked){
        case ITEM_WEAPON_UP: player_weapon_up(&g->player);             break;
        case ITEM_SHIELD:    player_add_shield(&g->player,ITEM_SHIELD_AMOUNT); break;
        case ITEM_HEAL:      player_add_hp(&g->player,ITEM_HEAL_AMOUNT);       break;
        case ITEM_WINGMAN:   player_add_wingman(&g->player);           break;
        }
    }

    // 玩家死亡检测
    if(g->player.hp<=0){ g->phase=PHASE_LOSE; return; }

    if(input) input->pause_pressed=false;
}

// ---------------------------------------------------------------------------
void game_render(const Game* g, Buffer* buf){
    // 星空背景
    stars_render(g, buf);

    if(g->phase==PHASE_TITLE){
        ui_render_title(buf, g->menu_diff_sel, g->frame_count);
        return;
    }
    if(g->phase==PHASE_WIN){
        ui_render_result(buf, true, g->score);
        return;
    }
    if(g->phase==PHASE_LOSE){
        ui_render_result(buf, false, g->score);
        return;
    }
    if(g->phase==PHASE_CLEAR){
        ui_render_clear(buf, &g->clear_stats);
        return;
    }

    // 渲染游戏对象（加震动偏移）
    // 注意：震动偏移已经在 stars_render 里体现
    // 游戏对象通过 shake_x/y 偏移绘制
    // 简化：直接渲染（shake 仅作用于星空层）

    enemy_render(&g->enemies, buf);
    bullet_render(&g->bullets, buf);
    item_render(&g->items, buf, g->frame_count);
    player_render(&g->player, buf, g->frame_count);

    // HUD
    ui_render_hud(buf, g->score, g->combo, &g->player,
                  g->stage, g->player.weapon_level, g->frame_count);

    // 暂停遮罩
    if(g->phase==PHASE_PAUSE)
        ui_render_pause(buf);
}

void game_shutdown(Game* /*g*/){
    // 所有子系统使用静态/栈内存，无需额外释放
}
