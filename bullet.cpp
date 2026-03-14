#include "bullet.hpp"
#include "player.hpp"
#include "config.h"
#include <cmath>
#include <cstring>

void bullet_manager_init(BulletManager* bm) {
    memset(bm, 0, sizeof(*bm));
}

// ---------------------------------------------------------------------------
// 内部辅助：添加玩家子弹
// ---------------------------------------------------------------------------
static void add_player_bullet(BulletManager* bm, float x, float y, float vx, float vy) {
    if (bm->num_player_bullets >= MAX_PLAYER_BULLETS) return;
    Bullet& b = bm->player_bullets[bm->num_player_bullets++];
    b.x = x; b.y = y; b.vx = vx; b.vy = vy;
    b.active = true; b.damage = 1;
    b.color  = rgb_to_uint32(255, 255, 0); // 黄色
    b.w = 2; b.h = 6;
}

// ---------------------------------------------------------------------------
// 玩家发射（5 级武器）
// ---------------------------------------------------------------------------
void bullet_fire_player(BulletManager* bm, int px, int py,
                         int weapon_level, const Wingman* wingmen) {
    float cx = (float)px + player_sprite_width() * 0.5f;
    float top = (float)(py + player_sprite_height());
    float spd = (float)PLAYER_BULLET_SPEED;

    switch (weapon_level) {
    case 1: // 单发
        add_player_bullet(bm, cx - 1, top, 0, spd);
        break;
    case 2: // 双排
        add_player_bullet(bm, cx - 4, top, 0, spd);
        add_player_bullet(bm, cx + 3, top, 0, spd);
        break;
    case 3: // 扇形 3 发
        add_player_bullet(bm, cx - 1, top,  0,     spd);
        add_player_bullet(bm, cx - 1, top, -spd*0.25f, spd*0.97f);
        add_player_bullet(bm, cx - 1, top,  spd*0.25f, spd*0.97f);
        break;
    case 4: { // 跟踪导弹
        if (bm->num_missiles < MAX_MISSILES) {
            Missile& m = bm->missiles[bm->num_missiles++];
            m.x = cx; m.y = top;
            m.vx = 0; m.vy = spd;
            m.angle = (float)M_PI / 2.0f; // 向上
            m.target_idx = -1;
            m.active = true;
            m.lifetime = MISSILE_LIFETIME;
        }
        // 同时还发一颗普通子弹
        add_player_bullet(bm, cx - 1, top, 0, spd);
        break;
    }
    case 5: { // 激光束
        bullet_fire_laser(bm, (int)cx, py + player_sprite_height(), 480,
                          rgb_to_uint32(255, 255, 100));
        // 同时双排子弹
        add_player_bullet(bm, cx - 4, top, 0, spd);
        add_player_bullet(bm, cx + 3, top, 0, spd);
        break;
    }
    default:
        add_player_bullet(bm, cx - 1, top, 0, spd);
        break;
    }

    // 僚机发射基础子弹
    if (wingmen) {
        int cxi = (int)cx;
        for (int i = 0; i < 2; ++i) {
            if (!wingmen[i].active) continue;
            if (wingmen[i].fire_timer > 0) continue;
            float wx = (float)(cxi + wingmen[i].offset_x);
            add_player_bullet(bm, wx, top, 0, spd);
        }
    }
}

// ---------------------------------------------------------------------------
// 敌机弹幕接口
// ---------------------------------------------------------------------------
void bullet_fire_straight(BulletManager* bm, float x, float y,
                           float vx, float vy, int dmg, uint32_t color) {
    if (bm->num_enemy_bullets >= MAX_ENEMY_BULLETS) return;
    Bullet& b = bm->enemy_bullets[bm->num_enemy_bullets++];
    b.x = x; b.y = y; b.vx = vx; b.vy = vy;
    b.active = true; b.damage = dmg; b.color = color;
    b.w = 3; b.h = 6;
}

void bullet_fire_fan(BulletManager* bm, float x, float y,
                      int count, float center_angle_rad, float spread_rad,
                      float speed, int dmg, uint32_t color) {
    for (int i = 0; i < count; ++i) {
        float a = center_angle_rad + spread_rad * (i - (count-1)*0.5f);
        float vx =  speed * cosf(a);
        float vy =  speed * sinf(a);
        bullet_fire_straight(bm, x, y, vx, vy, dmg, color);
    }
}

void bullet_fire_ring(BulletManager* bm, float x, float y,
                       int count, float speed, int dmg, uint32_t color) {
    float step = 2.0f * (float)M_PI / (float)count;
    for (int i = 0; i < count; ++i) {
        float a  = step * i;
        float vx = speed * cosf(a);
        float vy = speed * sinf(a);
        bullet_fire_straight(bm, x, y, vx, vy, dmg, color);
    }
}

void bullet_fire_laser(BulletManager* bm, int x, int y_bottom,
                        int screen_h, uint32_t color) {
    if (bm->num_lasers >= MAX_LASERS) return;
    Laser& l = bm->lasers[bm->num_lasers++];
    l.x = x; l.y_bottom = y_bottom; l.y_top = screen_h;
    l.duration = LASER_DURATION;
    l.active = true; l.color = color;
}

void bullet_fire_missile(BulletManager* bm, float x, float y) {
    if (bm->num_missiles >= MAX_MISSILES) return;
    Missile& m = bm->missiles[bm->num_missiles++];
    m.x = x; m.y = y;
    m.vx = 0; m.vy = -(float)MISSILE_SPEED; // 敌方导弹向下
    m.angle = -(float)M_PI / 2.0f;
    m.target_idx = -1; // 锁定玩家（由 update 处理）
    m.active = true;
    m.lifetime = MISSILE_LIFETIME;
}

// ---------------------------------------------------------------------------
// 每帧更新
// ---------------------------------------------------------------------------
size_t bullet_update(
    BulletManager* bm,
    int player_x, int player_y, int player_w, int player_h,
    bool player_invincible,
    int* ex, int* ey, int* ew, int* eh,
    bool* ealive, int enemy_count,
    int screen_w, int screen_h,
    HitRecord* out_hits, size_t max_hits)
{
    size_t hit_count = 0;

    // --- 玩家子弹移动 + 命中敌机 ---
    for (size_t i = 0; i < bm->num_player_bullets;) {
        Bullet& b = bm->player_bullets[i];
        b.x += b.vx; b.y += b.vy;
        // 超出屏幕
        if (b.y >= (float)screen_h || b.y < 0 ||
            b.x < 0 || b.x >= (float)screen_w) {
            b = bm->player_bullets[--bm->num_player_bullets];
            continue;
        }
        // 命中敌机
        bool hit = false;
        for (int ei = 0; ei < enemy_count && !hit; ++ei) {
            if (!ealive[ei]) continue;
            int bxi = (int)b.x, byi = (int)b.y;
            if (bxi < ex[ei] + ew[ei] && bxi + b.w > ex[ei] &&
                byi < ey[ei] + eh[ei] && byi + b.h > ey[ei]) {
                if (hit_count < max_hits)
                    out_hits[hit_count++] = { ei, b.damage };
                b = bm->player_bullets[--bm->num_player_bullets];
                hit = true;
            }
        }
        if (!hit) ++i;
    }

    // --- 敌机子弹移动 + 命中玩家 ---
    for (size_t i = 0; i < bm->num_enemy_bullets;) {
        Bullet& b = bm->enemy_bullets[i];
        b.x += b.vx; b.y += b.vy;
        if (b.y >= (float)screen_h || b.y < 0 ||
            b.x < 0 || b.x >= (float)screen_w) {
            b = bm->enemy_bullets[--bm->num_enemy_bullets];
            continue;
        }
        // 命中玩家
        bool hit = false;
        if (!player_invincible) {
            int bxi = (int)b.x, byi = (int)b.y;
            if (bxi < player_x + player_w && bxi + b.w > player_x &&
                byi < player_y + player_h && byi + b.h > player_y) {
                if (hit_count < max_hits)
                    out_hits[hit_count++] = { -1, b.damage };
                b = bm->enemy_bullets[--bm->num_enemy_bullets];
                hit = true;
            }
        }
        if (!hit) ++i;
    }

    // --- 跟踪导弹 ---
    for (size_t i = 0; i < bm->num_missiles;) {
        Missile& m = bm->missiles[i];
        m.lifetime--;
        if (m.lifetime <= 0) {
            m = bm->missiles[--bm->num_missiles];
            continue;
        }

        // 寻找锁定目标（仅玩家发射的导弹锁敌机）
        if (m.vy > 0 && enemy_count > 0) { // 向上飞=玩家导弹
            float best_dist = 1e9f;
            int best = -1;
            for (int ei = 0; ei < enemy_count; ++ei) {
                if (!ealive[ei]) continue;
                float dx = ex[ei] + ew[ei]*0.5f - m.x;
                float dy = ey[ei] + eh[ei]*0.5f - m.y;
                float d  = dx*dx + dy*dy;
                if (d < best_dist) { best_dist = d; best = ei; }
            }
            if (best >= 0) {
                float tx = ex[best] + ew[best]*0.5f;
                float ty = ey[best] + eh[best]*0.5f;
                float da = atan2f(ty - m.y, tx - m.x) - m.angle;
                // 归一化到 [-π, π]
                while (da >  (float)M_PI) da -= 2*(float)M_PI;
                while (da < -(float)M_PI) da += 2*(float)M_PI;
                float turn = MISSILE_TURN_RAD;
                if (da >  turn) da =  turn;
                if (da < -turn) da = -turn;
                m.angle += da;
            }
        }

        m.vx = (float)MISSILE_SPEED * cosf(m.angle);
        m.vy = (float)MISSILE_SPEED * sinf(m.angle);
        m.x += m.vx; m.y += m.vy;

        if (m.x < 0 || m.x >= (float)screen_w ||
            m.y < 0 || m.y >= (float)screen_h) {
            m = bm->missiles[--bm->num_missiles];
            continue;
        }

        // 碰撞
        bool hit = false;
        int mx = (int)m.x, my = (int)m.y;
        if (m.vy > 0) { // 玩家导弹 → 击敌
            for (int ei = 0; ei < enemy_count && !hit; ++ei) {
                if (!ealive[ei]) continue;
                if (mx < ex[ei]+ew[ei] && mx+4 > ex[ei] &&
                    my < ey[ei]+eh[ei] && my+4 > ey[ei]) {
                    if (hit_count < max_hits)
                        out_hits[hit_count++] = { ei, 2 };
                    m = bm->missiles[--bm->num_missiles];
                    hit = true;
                }
            }
        } else if (!player_invincible) { // 敌方导弹 → 击玩家
            if (mx < player_x+player_w && mx+4 > player_x &&
                my < player_y+player_h && my+4 > player_y) {
                if (hit_count < max_hits)
                    out_hits[hit_count++] = { -1, 2 };
                m = bm->missiles[--bm->num_missiles];
                hit = true;
            }
        }
        if (!hit) ++i;
    }

    // --- 激光 ---
    for (size_t i = 0; i < bm->num_lasers;) {
        Laser& l = bm->lasers[i];
        if (--l.duration <= 0) {
            l = bm->lasers[--bm->num_lasers];
            continue;
        }
        // 激光命中敌机（X 轴重叠即命中）
        if (l.color == rgb_to_uint32(255,255,100)) { // 玩家激光
            for (int ei = 0; ei < enemy_count; ++ei) {
                if (!ealive[ei]) continue;
                if (l.x >= ex[ei] && l.x < ex[ei]+ew[ei] &&
                    ey[ei]+eh[ei] >= l.y_bottom) {
                    if (hit_count < max_hits)
                        out_hits[hit_count++] = { ei, 1 }; // 每帧持续1伤害
                }
            }
        }
        ++i;
    }

    return hit_count;
}

// ---------------------------------------------------------------------------
// 渲染
// ---------------------------------------------------------------------------
void bullet_render(const BulletManager* bm, Buffer* buf) {
    // 玩家子弹（黄色）
    for (size_t i = 0; i < bm->num_player_bullets; ++i) {
        const Bullet& b = bm->player_bullets[i];
        int bx = (int)b.x, by = (int)b.y;
        for (int xi = 0; xi < b.w; ++xi)
            for (int yi = 0; yi < b.h; ++yi) {
                int sx = bx+xi, sy = by+yi;
                if (sx>=0 && sy>=0 && sx<(int)buf->width && sy<(int)buf->height)
                    buf->data[sy*buf->width+sx] = b.color;
            }
    }
    // 敌机子弹（各自颜色）
    for (size_t i = 0; i < bm->num_enemy_bullets; ++i) {
        const Bullet& b = bm->enemy_bullets[i];
        int bx = (int)b.x, by = (int)b.y;
        for (int xi = 0; xi < b.w; ++xi)
            for (int yi = 0; yi < b.h; ++yi) {
                int sx = bx+xi, sy = by+yi;
                if (sx>=0 && sy>=0 && sx<(int)buf->width && sy<(int)buf->height)
                    buf->data[sy*buf->width+sx] = b.color;
            }
    }
    // 导弹（橙色小方块 4x4）
    uint32_t mc = rgb_to_uint32(255, 128, 0);
    for (size_t i = 0; i < bm->num_missiles; ++i) {
        const Missile& m = bm->missiles[i];
        int mx = (int)m.x, my = (int)m.y;
        for (int xi = 0; xi < 4; ++xi)
            for (int yi = 0; yi < 4; ++yi) {
                int sx = mx+xi, sy = my+yi;
                if (sx>=0 && sy>=0 && sx<(int)buf->width && sy<(int)buf->height)
                    buf->data[sy*buf->width+sx] = mc;
            }
    }
    // 激光（竖线）
    for (size_t i = 0; i < bm->num_lasers; ++i) {
        const Laser& l = bm->lasers[i];
        // 闪烁效果：激光宽 2-3px，奇偶帧切换
        int lw = (l.duration % 2 == 0) ? 3 : 2;
        for (int xi = 0; xi < lw; ++xi) {
            int sx = l.x + xi - lw/2;
            if (sx < 0 || sx >= (int)buf->width) continue;
            for (int sy = l.y_bottom; sy < l.y_top && sy < (int)buf->height; ++sy)
                buf->data[sy*buf->width+sx] = l.color;
        }
    }
}
