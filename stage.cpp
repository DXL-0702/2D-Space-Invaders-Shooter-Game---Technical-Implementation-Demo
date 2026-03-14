#include "stage.hpp"
#include "buffer.hpp"
#include <cstdlib>

void stage_init(StageManager* sm, int stage, Difficulty diff){
    sm->stage        = stage;
    sm->diff         = diff;
    sm->phase        = STAGE_WAVE;
    sm->wave         = 0;
    sm->spawn_timer  = 0;
    sm->clear_timer  = 0;
    sm->boss_spawned = false;
    sm->diff_hp_mult  = DIFF_HP_MULT[diff];
    sm->diff_spd_mult = DIFF_SPEED_MULT[diff];
}

// 每关波次配置：{小型数, 中型数, 精英数}
static const int wave_cfg[3][4][3]={
    // 关卡 1
    {{6,0,0},{8,2,0},{10,3,1},{0,0,0}},
    // 关卡 2
    {{8,2,0},{10,4,1},{12,4,2},{0,0,0}},
    // 关卡 3
    {{10,4,1},{12,6,2},{14,6,3},{0,0,0}},
};
static const int waves_per_stage = 3;

void stage_update(StageManager* sm, EnemyManager* em, int sw, int sh){
    if(sm->phase==STAGE_CLEAR){
        ++sm->clear_timer;
        return;
    }
    if(sm->phase==STAGE_BOSS){
        // BOSS 存活期间不生成新敌机
        // 当 BOSS 全部死亡时切换为 CLEAR
        bool boss_alive=false;
        for(size_t i=0;i<em->num_enemies;++i)
            if(em->enemies[i].type==ENEMY_BOSS&&em->enemies[i].death_timer==0)
                {boss_alive=true;break;}
        if(!boss_alive&&sm->boss_spawned){
            sm->phase=STAGE_CLEAR;
            sm->clear_timer=0;
        }
        return;
    }

    // STAGE_WAVE：计时生成敌机
    ++sm->spawn_timer;
    // 每 90 帧生成一批
    int interval = 90 - sm->stage*10 - (int)sm->diff*5;
    if(interval<30) interval=30;

    if(sm->spawn_timer < interval) return;
    sm->spawn_timer=0;

    // 检查当前波次是否还有敌机需要生成
    if(sm->wave >= waves_per_stage){
        // 所有波次完成且场上无普通敌机，生成 BOSS
        bool any_normal=false;
        for(size_t i=0;i<em->num_enemies;++i)
            if(em->enemies[i].type!=ENEMY_BOSS&&em->enemies[i].death_timer==0)
                {any_normal=true;break;}
        if(!any_normal&&!sm->boss_spawned){
            sm->phase=STAGE_BOSS;
            enemy_spawn(em,ENEMY_BOSS,sw/2-24,sh,sm->diff_hp_mult);
            sm->boss_spawned=true;
        }
        return;
    }

    int si=sm->stage-1; if(si<0)si=0; if(si>2)si=2;
    int wi=sm->wave;    if(wi>2)wi=2;
    int n_small  = wave_cfg[si][wi][0];
    int n_medium = wave_cfg[si][wi][1];
    int n_elite  = wave_cfg[si][wi][2];

    // 随机 X 位置生成
    for(int k=0;k<n_small;++k){
        int x=20+rand()%(sw-40);
        enemy_spawn(em,ENEMY_SMALL,x,sh-10,sm->diff_hp_mult);
    }
    for(int k=0;k<n_medium;++k){
        int x=30+rand()%(sw-60);
        enemy_spawn(em,ENEMY_MEDIUM,x,sh+20,sm->diff_hp_mult);
    }
    for(int k=0;k<n_elite;++k){
        int x=50+rand()%(sw-100);
        enemy_spawn(em,ENEMY_ELITE,x,sh+40,sm->diff_hp_mult);
    }
    ++sm->wave;
}

uint32_t stage_bg_color(int stage){
    switch(stage){
    case 1: return rgb_to_uint32(0,10,30);   // 深蓝星空
    case 2: return rgb_to_uint32(20,0,30);   // 深紫星云
    case 3: return rgb_to_uint32(30,10,0);   // 深红熔岩
    default:return rgb_to_uint32(0,0,0);
    }
}
