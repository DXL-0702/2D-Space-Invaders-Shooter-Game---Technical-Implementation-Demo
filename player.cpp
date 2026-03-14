#include "player.hpp"
#include "input.hpp"
#include "config.h"

// ---------------------------------------------------------------------------
// 玩家飞船精灵（11x7）
// ---------------------------------------------------------------------------
static uint8_t s_player_data[77] = {
    0,0,0,0,0,1,0,0,0,0,0,
    0,0,0,0,1,1,1,0,0,0,0,
    0,0,0,0,1,1,1,0,0,0,0,
    0,1,1,1,1,1,1,1,1,1,0,
    1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,
};
static Sprite s_player_sprite = { 11, 7, s_player_data };

// 僚机精灵（7x5）
static uint8_t s_wingman_data[35] = {
    0,0,1,0,0,0,0,
    0,1,1,1,0,0,0,
    1,1,1,1,1,0,0,
    1,1,1,1,1,1,0,
    1,1,1,1,1,1,1,
};
static Sprite s_wingman_sprite = { 7, 5, s_wingman_data };

void player_init(Player* p, int screen_w, int screen_h) {
    p->x = (screen_w - (int)s_player_sprite.width) / 2;
    p->y = 32;
    p->hp               = PLAYER_MAX_HP;
    p->shield           = 0;
    p->invincible_timer = 0;
    p->weapon_level     = 1;
    p->fire_timer       = 0;
    p->slow_mode        = false;
    for (int i = 0; i < 2; ++i) {
        p->wingmen[i].active     = false;
        p->wingmen[i].fire_timer = 0;
        p->wingmen[i].offset_x   = (i == 0) ? -WINGMAN_OFFSET_X : WINGMAN_OFFSET_X;
    }
    (void)screen_h;
}

bool player_update(Player* p, const InputState* input, int screen_w, int screen_h) {
    if (!input) return false;

    // 慢速模式
    p->slow_mode = input->slow_hold;
    int spd = p->slow_mode ? PLAYER_SPEED_SLOW : PLAYER_SPEED_NORMAL;

    // 水平移动
    int dx = input->move_x * spd;
    p->x += dx;
    if (p->x < 0) p->x = 0;
    if (p->x + (int)s_player_sprite.width > screen_w)
        p->x = screen_w - (int)s_player_sprite.width;

    // 垂直移动（限制在屏幕下半部分，y 轴向上）
    int dy = input->move_y * spd;
    p->y += dy;
    if (p->y < 8)  p->y = 8;
    // 最高不超过屏幕 40% 高度，保留战斗区域
    int max_y = (int)(screen_h * 40 / 100);
    if (p->y + (int)s_player_sprite.height > max_y)
        p->y = max_y - (int)s_player_sprite.height;

    // 无敌帧倒计时
    if (p->invincible_timer > 0) --p->invincible_timer;

    // 射击冷却
    bool should_fire = false;
    if (p->fire_timer > 0) --p->fire_timer;
    int rate = PLAYER_FIRE_RATE[p->weapon_level - 1];
    if (input->fire_hold && p->fire_timer == 0) {
        should_fire   = true;
        p->fire_timer = rate;
    }

    // 僚机射击冷却
    for (int i = 0; i < 2; ++i) {
        if (!p->wingmen[i].active) continue;
        if (p->wingmen[i].fire_timer > 0) --p->wingmen[i].fire_timer;
    }

    return should_fire;
}

void player_render(const Player* p, Buffer* buf, int frame_count) {
    // 无敌帧时每 4 帧闪烁一次（奇数帧不绘制）
    bool blink = (p->invincible_timer > 0) && ((frame_count / 4) % 2 == 1);
    if (!blink) {
        uint32_t col = rgb_to_uint32(0, 128, 255);
        buffer_sprite_draw(buf, s_player_sprite, (size_t)p->x, (size_t)p->y, col);
    }

    // 绘制僚机
    uint32_t wm_col = rgb_to_uint32(0, 200, 255);
    int cx = p->x + (int)s_player_sprite.width / 2;
    for (int i = 0; i < 2; ++i) {
        if (!p->wingmen[i].active) continue;
        int wx = cx + p->wingmen[i].offset_x - (int)s_wingman_sprite.width / 2;
        int wy = p->y;
        if (wx < 0) wx = 0;
        if (!blink)
            buffer_sprite_draw(buf, s_wingman_sprite, (size_t)wx, (size_t)wy, wm_col);
    }

    // 护盾条（玩家正上方，蓝色）
    if (p->shield > 0) {
        int bw = (int)s_player_sprite.width * p->shield / PLAYER_MAX_SHIELD;
        if (bw < 1) bw = 1;
        uint32_t sc = rgb_to_uint32(64, 128, 255);
        for (int xi = 0; xi < bw; ++xi) {
            size_t sx = (size_t)(p->x + xi);
            size_t sy = (size_t)(p->y + (int)s_player_sprite.height + 1);
            if (sx < buf->width && sy < buf->height)
                buf->data[sy * buf->width + sx] = sc;
        }
    }
}

bool player_take_damage(Player* p, int dmg) {
    if (p->invincible_timer > 0) return false; // 无敌帧免疫
    if (p->shield > 0) {
        // 护盾吸收一次伤害
        p->shield -= dmg;
        if (p->shield < 0) p->shield = 0;
    } else {
        p->hp -= dmg;
        if (p->hp < 0) p->hp = 0;
    }
    p->invincible_timer = PLAYER_INVINCIBLE_F;
    return (p->hp <= 0);
}

void player_weapon_up(Player* p) {
    if (p->weapon_level < 5) ++p->weapon_level;
}

void player_add_shield(Player* p, int amount) {
    p->shield += amount;
    if (p->shield > PLAYER_MAX_SHIELD) p->shield = PLAYER_MAX_SHIELD;
}

void player_add_hp(Player* p, int amount) {
    p->hp += amount;
    if (p->hp > PLAYER_MAX_HP) p->hp = PLAYER_MAX_HP;
}

void player_add_wingman(Player* p) {
    for (int i = 0; i < 2; ++i) {
        if (!p->wingmen[i].active) {
            p->wingmen[i].active     = true;
            p->wingmen[i].fire_timer = 0;
            return;
        }
    }
}

int player_sprite_width()  { return (int)s_player_sprite.width; }
int player_sprite_height() { return (int)s_player_sprite.height; }
