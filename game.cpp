#include "game.hpp"
#include "input.hpp"
#include <cstdlib>
#include <ctime>

// --- 子弹精灵 ---
static uint8_t bullet_data[4] = { 1, 1, 1, 1 };
static Sprite bullet_sprite;
static Sprite enemy_bullet_sprite;

// --- 外星人精灵 (Part 4: 3种类型 x 2帧) ---
static uint8_t alien_a0[64] = {
    0,0,0,1,1,0,0,0,
    0,0,1,1,1,1,0,0,
    0,1,1,1,1,1,1,0,
    1,1,0,1,1,0,1,1,
    1,1,1,1,1,1,1,1,
    0,1,0,1,1,0,1,0,
    1,0,0,0,0,0,0,1,
    0,1,0,0,0,0,1,0
};
static uint8_t alien_a1[64] = {
    0,0,0,1,1,0,0,0,
    0,0,1,1,1,1,0,0,
    0,1,1,1,1,1,1,0,
    1,1,0,1,1,0,1,1,
    1,1,1,1,1,1,1,1,
    0,0,1,0,0,1,0,0,
    0,1,0,1,1,0,1,0,
    1,0,1,0,0,1,0,1
};
static uint8_t alien_b0[88] = {
    0,0,1,0,0,0,0,0,1,0,0,
    0,0,0,1,0,0,0,1,0,0,0,
    0,0,1,1,1,1,1,1,1,0,0,
    0,1,1,0,1,1,1,0,1,1,0,
    1,1,1,1,1,1,1,1,1,1,1,
    1,0,1,1,1,1,1,1,1,0,1,
    1,0,1,0,0,0,0,0,1,0,1,
    0,0,0,1,1,0,1,1,0,0,0
};
static uint8_t alien_b1[88] = {
    0,0,1,0,0,0,0,0,1,0,0,
    1,0,0,1,0,0,0,1,0,0,1,
    1,0,1,1,1,1,1,1,1,0,1,
    1,1,1,0,1,1,1,0,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,
    0,1,1,1,1,1,1,1,1,1,0,
    0,0,1,0,0,0,0,0,1,0,0,
    0,1,0,0,0,0,0,0,0,1,0
};
static uint8_t alien_c0[96] = {
    0,0,0,0,1,1,1,1,0,0,0,0,
    0,1,1,1,1,1,1,1,1,1,1,0,
    1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,0,0,1,1,0,0,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,
    0,0,0,1,1,0,0,1,1,0,0,0,
    0,0,1,1,0,1,1,0,1,1,0,0,
    1,1,0,0,0,0,0,0,0,0,1,1
};
static uint8_t alien_c1[96] = {
    0,0,0,0,1,1,1,1,0,0,0,0,
    0,1,1,1,1,1,1,1,1,1,1,0,
    1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,0,0,1,1,0,0,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,
    0,0,1,1,1,0,0,1,1,1,0,0,
    0,1,1,0,0,1,1,0,0,1,1,0,
    0,0,1,1,0,0,0,0,1,1,0,0
};
static uint8_t alien_death_data[91] = {
    0,1,0,0,1,0,0,0,1,0,0,1,0,
    0,0,1,0,0,1,0,1,0,0,1,0,0,
    0,0,0,1,0,0,0,0,0,1,0,0,0,
    1,1,0,0,0,0,0,0,0,0,0,1,1,
    0,0,0,1,0,0,0,0,0,1,0,0,0,
    0,0,1,0,0,1,0,1,0,0,1,0,0,
    0,1,0,0,1,0,0,0,1,0,0,1,0
};

static Sprite alien_sprites[6];
static Sprite alien_death_sprite;
static Sprite player_sprite;
static uint8_t player_data[77] = {
    0,0,0,0,0,1,0,0,0,0,0,
    0,0,0,0,1,1,1,0,0,0,0,
    0,0,0,0,1,1,1,0,0,0,0,
    0,1,1,1,1,1,1,1,1,1,0,
    1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,
};

static SpriteAnimation alien_animations[3];

static size_t alien_sprite_width(uint8_t type) {
    if (type == ALIEN_TYPE_A) return 8;
    if (type == ALIEN_TYPE_B) return 11;
    if (type == ALIEN_TYPE_C) return 12;
    return 8;
}

void game_init(Game* game, size_t width, size_t height) {
    srand((unsigned)time(nullptr));
    game->width = width;
    game->height = height;
    game->num_aliens = 55;
    game->num_player_bullets = 0;
    game->num_enemy_bullets  = 0;
    game->score = 0;
    game->phase = PHASE_PLAYING;
    game->aliens = new Alien[game->num_aliens];
    game->death_counters = new uint8_t[game->num_aliens];

    for (size_t i = 0; i < game->num_aliens; ++i)
        game->death_counters[i] = 10;

    game->player.x  = (width - 11) / 2;
    game->player.y  = 32;
    game->player.hp = 100;

    // 外星人 5 行 x 11 列，按行分配类型：row0=C, row1-2=B, row3-4=A
    const size_t formation_w = 12 * 11;
    const size_t start_x = (width - formation_w) / 2;
    const size_t start_y = height - 80;
    for (size_t yi = 0; yi < 5; ++yi) {
        uint8_t t = (yi == 0) ? ALIEN_TYPE_C : (yi <= 2) ? ALIEN_TYPE_B : ALIEN_TYPE_A;
        for (size_t xi = 0; xi < 11; ++xi) {
            size_t idx = yi * 11 + xi;
            game->aliens[idx].x = start_x + 12 * xi;
            game->aliens[idx].y = start_y - 17 * yi;
            game->aliens[idx].type = t;
        }
    }
    game->alien_dir         = 1;
    game->alien_move_timer  = 0;
    game->alien_shoot_timer = 0;

    // 子弹精灵（1x4，玩家与敌机共用数据）
    bullet_sprite.width = 1;
    bullet_sprite.height = 4;
    bullet_sprite.data = bullet_data;
    enemy_bullet_sprite.width = 2;
    enemy_bullet_sprite.height = 4;
    enemy_bullet_sprite.data = bullet_data;

    // 外星人精灵
    alien_sprites[0] = { 8, 8, alien_a0 };
    alien_sprites[1] = { 8, 8, alien_a1 };
    alien_sprites[2] = { 11, 8, alien_b0 };
    alien_sprites[3] = { 11, 8, alien_b1 };
    alien_sprites[4] = { 12, 8, alien_c0 };
    alien_sprites[5] = { 12, 8, alien_c1 };
    alien_death_sprite = { 13, 7, alien_death_data };

    player_sprite = { 11, 7, player_data };

    // 三种外星人动画
    for (int t = 0; t < 3; ++t) {
        alien_animations[t].loop = true;
        alien_animations[t].num_frames = 2;
        alien_animations[t].frame_duration = 10;
        alien_animations[t].time = 0;
        alien_animations[t].frames = new const Sprite*[2];
        alien_animations[t].frames[0] = &alien_sprites[t * 2];
        alien_animations[t].frames[1] = &alien_sprites[t * 2 + 1];
    }
}

void game_update(Game* game, InputState* input) {
    if (game->phase != PHASE_PLAYING) return;

    // 推进所有外星人动画
    for (int t = 0; t < 3; ++t) {
        ++alien_animations[t].time;
        if (alien_animations[t].time >= alien_animations[t].num_frames * alien_animations[t].frame_duration)
            alien_animations[t].time = 0;
    }

    // 死亡计数器
    for (size_t ai = 0; ai < game->num_aliens; ++ai) {
        if (game->aliens[ai].type == ALIEN_DEAD && game->death_counters[ai]) {
            --game->death_counters[ai];
        }
    }

    // -----------------------------------------------------------------------
    // 玩家子弹：碰撞检测 + 移动 + 销毁
    // -----------------------------------------------------------------------
    for (size_t bi = 0; bi < game->num_player_bullets;) {
        bool hit = false;
        for (size_t ai = 0; ai < game->num_aliens && !hit; ++ai) {
            if (game->aliens[ai].type == ALIEN_DEAD) continue;
            const SpriteAnimation& anim = alien_animations[game->aliens[ai].type - 1];
            const Sprite& asp = *anim.frames[anim.time / anim.frame_duration];

            if (sprite_overlap_check(bullet_sprite,
                    game->player_bullets[bi].x, game->player_bullets[bi].y,
                    asp, game->aliens[ai].x, game->aliens[ai].y)) {
                uint8_t otype = game->aliens[ai].type;
                if (otype == ALIEN_TYPE_A)      game->score += 30;
                else if (otype == ALIEN_TYPE_B) game->score += 20;
                else                            game->score += 10;
                game->aliens[ai].type = ALIEN_DEAD;
                int dx = (int)(alien_death_sprite.width - asp.width) / 2;
                int nx = (int)game->aliens[ai].x - dx;
                game->aliens[ai].x = (size_t)(nx < 0 ? 0 : nx);
                game->player_bullets[bi] = game->player_bullets[--game->num_player_bullets];
                hit = true;
            }
        }
        if (hit) continue;
        game->player_bullets[bi].y += (size_t)game->player_bullets[bi].dir;
        if (game->player_bullets[bi].y + bullet_sprite.height >= game->height) {
            game->player_bullets[bi] = game->player_bullets[--game->num_player_bullets];
            continue;
        }
        ++bi;
    }

    // -----------------------------------------------------------------------
    // 敌机子弹：碰撞玩家 + 移动 + 销毁
    // -----------------------------------------------------------------------
    for (size_t bi = 0; bi < game->num_enemy_bullets;) {
        int new_y = (int)game->enemy_bullets[bi].y + game->enemy_bullets[bi].dir;
        if (new_y < 0) {
            game->enemy_bullets[bi] = game->enemy_bullets[--game->num_enemy_bullets];
            continue;
        }
        game->enemy_bullets[bi].y = (size_t)new_y;

        if (sprite_overlap_check(enemy_bullet_sprite,
                game->enemy_bullets[bi].x, game->enemy_bullets[bi].y,
                player_sprite, game->player.x, game->player.y)) {
            game->player.hp -= 20;
            if (game->player.hp < 0) game->player.hp = 0;
            game->enemy_bullets[bi] = game->enemy_bullets[--game->num_enemy_bullets];
            continue;
        }
        ++bi;
    }

    // -----------------------------------------------------------------------
    // 玩家发射（空格）
    // -----------------------------------------------------------------------
    if (input && input->fire_pressed && game->num_player_bullets < GAME_MAX_BULLETS) {
        size_t bi = game->num_player_bullets++;
        game->player_bullets[bi].x   = game->player.x + player_sprite.width / 2;
        game->player_bullets[bi].y   = game->player.y + player_sprite.height;
        game->player_bullets[bi].dir = 3;
    }
    if (input) input->fire_pressed = false;

    // -----------------------------------------------------------------------
    // 外星人移动：碰边整体下移并反向
    // -----------------------------------------------------------------------
    ++game->alien_move_timer;
    if (game->alien_move_timer >= 10) {
        game->alien_move_timer = 0;
        size_t leftmost = game->width, rightmost = 0;
        bool any_alive = false;
        for (size_t i = 0; i < game->num_aliens; ++i) {
            if (game->aliens[i].type == ALIEN_DEAD) continue;
            any_alive = true;
            size_t w = alien_sprite_width(game->aliens[i].type);
            if (game->aliens[i].x < leftmost)      leftmost  = game->aliens[i].x;
            if (game->aliens[i].x + w > rightmost) rightmost = game->aliens[i].x + w;
        }
        if (any_alive) {
            int dx = game->alien_dir * 4;
            bool hit_edge = (dx > 0 && rightmost + 4 >= game->width) ||
                            (dx < 0 && (int)leftmost - 4 <= 0);
            if (hit_edge) {
                game->alien_dir = -game->alien_dir;
                for (size_t i = 0; i < game->num_aliens; ++i)
                    if (game->aliens[i].y >= 8)
                        game->aliens[i].y -= 8;
            } else {
                for (size_t i = 0; i < game->num_aliens; ++i)
                    game->aliens[i].x = (size_t)((int)game->aliens[i].x + dx);
            }
        }
    }

    // -----------------------------------------------------------------------
    // 敌机随机射击（约每60帧一次）
    // -----------------------------------------------------------------------
    ++game->alien_shoot_timer;
    if (game->alien_shoot_timer >= 30) {
        game->alien_shoot_timer = 0;
        size_t alive[55];
        size_t num_alive = 0;
        for (size_t i = 0; i < game->num_aliens; ++i)
            if (game->aliens[i].type != ALIEN_DEAD)
                alive[num_alive++] = i;
        // 每次同时随机发 2 颗，增加压迫感
        size_t shots = (num_alive > 5) ? 2 : 1;
        for (size_t s = 0; s < shots; ++s) {
            if (num_alive > 0 && game->num_enemy_bullets < GAME_MAX_BULLETS) {
                size_t pick = alive[(size_t)rand() % num_alive];
                size_t w    = alien_sprite_width(game->aliens[pick].type);
                size_t bi   = game->num_enemy_bullets++;
                game->enemy_bullets[bi].x   = game->aliens[pick].x + w / 2;
                game->enemy_bullets[bi].y   = (game->aliens[pick].y > 4) ? game->aliens[pick].y - 4 : 0;
                game->enemy_bullets[bi].dir = -3;
            }
        }
    }

    // -----------------------------------------------------------------------
    // 玩家移动
    // -----------------------------------------------------------------------
    int pmov = 4 * (input ? input->move_dir : 0);
    if (pmov != 0) {
        if ((int)game->player.x + (int)player_sprite.width + pmov >= (int)game->width)
            game->player.x = game->width - player_sprite.width;
        else if ((int)game->player.x + pmov <= 0)
            game->player.x = 0;
        else
            game->player.x = (size_t)((int)game->player.x + pmov);
    }

    // -----------------------------------------------------------------------
    // 游戏胜负判断
    // -----------------------------------------------------------------------
    if (game->player.hp <= 0) {
        game->phase = PHASE_LOSE;
        return;
    }
    bool all_dead = true;
    for (size_t i = 0; i < game->num_aliens; ++i)
        if (game->aliens[i].type != ALIEN_DEAD) { all_dead = false; break; }
    if (all_dead) game->phase = PHASE_WIN;
}

void game_render(const Game* game, Buffer* buffer) {
    const uint32_t alien_color  = rgb_to_uint32(0, 0, 0);       // 外星人：黑色
    const uint32_t player_color = rgb_to_uint32(0, 128, 255);   // 玩家：蓝色
    const uint32_t pbullet_color = rgb_to_uint32(255, 255, 0);  // 玩家子弹：黄色
    const uint32_t ebullet_color = rgb_to_uint32(180, 0, 255);  // 敌机子弹：紫色

    // 绘制外星人组
    for (size_t ai = 0; ai < game->num_aliens; ++ai) {
        if (!game->death_counters[ai]) continue;
        const Alien& alien = game->aliens[ai];
        if (alien.type == ALIEN_DEAD) {
            buffer_sprite_draw(buffer, alien_death_sprite, alien.x, alien.y, alien_color);
        } else {
            const SpriteAnimation& anim = alien_animations[alien.type - 1];
            size_t cf = anim.time / anim.frame_duration;
            buffer_sprite_draw(buffer, *anim.frames[cf], alien.x, alien.y, alien_color);
        }
    }

    // 绘制玩家子弹组
    for (size_t bi = 0; bi < game->num_player_bullets; ++bi)
        buffer_sprite_draw(buffer, bullet_sprite,
            game->player_bullets[bi].x, game->player_bullets[bi].y, pbullet_color);

    // 绘制敌机子弹组
    for (size_t bi = 0; bi < game->num_enemy_bullets; ++bi)
        buffer_sprite_draw(buffer, enemy_bullet_sprite,
            game->enemy_bullets[bi].x, game->enemy_bullets[bi].y, ebullet_color);

    // 绘制玩家组
    buffer_sprite_draw(buffer, player_sprite, game->player.x, game->player.y, player_color);
}

void game_shutdown(Game* game) {
    delete[] game->aliens;
    game->aliens = nullptr;
    delete[] game->death_counters;
    game->death_counters = nullptr;
    for (int t = 0; t < 3; ++t) {
        delete[] alien_animations[t].frames;
        alien_animations[t].frames = nullptr;
    }
}
