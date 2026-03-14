#pragma once
#include "buffer.hpp"
#include "config.h"
#include <cstdint>
#include <cstddef>

enum ItemType : uint8_t {
    ITEM_WEAPON_UP = 0, // 火力升级（黄色）
    ITEM_SHIELD    = 1, // 护盾回复（蓝色）
    ITEM_HEAL      = 2, // 回血（绿色）
    ITEM_WINGMAN   = 3, // 僚机（青色）
};

struct Item {
    int      x, y;
    ItemType type;
    bool     active;
    int      lifetime;    // 剩余帧数
};

struct ItemManager {
    Item   items[MAX_ITEMS];
    size_t num_items;
};

void item_manager_init(ItemManager* im);
// 30% 概率在 (x,y) 生成随机道具
void item_try_spawn(ItemManager* im, int x, int y);
// 强制生成指定类型
void item_spawn(ItemManager* im, int x, int y, ItemType type);
// 每帧更新：下落 + 计时消失；返回被拾取的道具类型（-1=无）
int  item_update(ItemManager* im, int px, int py, int pw, int ph);
void item_render(const ItemManager* im, Buffer* buf, int frame_count);
