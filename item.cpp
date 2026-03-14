#include "item.hpp"
#include <cstring>
#include <cstdlib>

void item_manager_init(ItemManager* im){ memset(im,0,sizeof(*im)); }

void item_try_spawn(ItemManager* im, int x, int y){
    if(rand()%100 >= ITEM_DROP_CHANCE) return;
    ItemType t=(ItemType)(rand()%4);
    item_spawn(im,x,y,t);
}

void item_spawn(ItemManager* im, int x, int y, ItemType type){
    if(im->num_items>=MAX_ITEMS) return;
    Item& it=im->items[im->num_items++];
    it.x=x; it.y=y; it.type=type;
    it.active=true; it.lifetime=ITEM_LIFETIME;
}

int item_update(ItemManager* im, int px, int py, int pw, int ph){
    int picked=-1;
    for(size_t i=0;i<im->num_items;){
        Item& it=im->items[i];
        // 下落
        it.y-=ITEM_FALL_SPEED;
        if(--it.lifetime<=0||it.y<0){
            it=im->items[--im->num_items]; continue;
        }
        // 拾取检测（8x8 道具 AABB）
        if(picked<0){
            if(it.x<px+pw&&it.x+8>px&&it.y<py+ph&&it.y+8>py){
                picked=(int)it.type;
                it=im->items[--im->num_items]; continue;
            }
        }
        ++i;
    }
    return picked;
}

void item_render(const ItemManager* im, Buffer* buf, int frame_count){
    for(size_t i=0;i<im->num_items;++i){
        const Item& it=im->items[i];
        // 最后 ITEM_BLINK_START 帧闪烁
        if(it.lifetime<=ITEM_BLINK_START&&(frame_count/4)%2==1) continue;
        uint32_t col;
        switch(it.type){
        case ITEM_WEAPON_UP: col=rgb_to_uint32(255,220,0);   break; // 黄
        case ITEM_SHIELD:    col=rgb_to_uint32(64,128,255);  break; // 蓝
        case ITEM_HEAL:      col=rgb_to_uint32(0,220,80);    break; // 绿
        case ITEM_WINGMAN:   col=rgb_to_uint32(0,220,220);   break; // 青
        default:             col=rgb_to_uint32(200,200,200); break;
        }
        // 8x8 实心方块
        for(int xi=0;xi<8;++xi)
            for(int yi=0;yi<8;++yi){
                int sx=it.x+xi,sy=it.y+yi;
                if(sx>=0&&sy>=0&&sx<(int)buf->width&&sy<(int)buf->height)
                    buf->data[sy*buf->width+sx]=col;
            }
        // 中心十字标记区分类型
        uint32_t mk=rgb_to_uint32(0,0,0);
        int cx=it.x+3,cy=it.y+3;
        for(int k=0;k<3;++k){
            if(cx+k<(int)buf->width&&cy+1>=0&&cy+1<(int)buf->height)
                buf->data[(cy+1)*buf->width+cx+k]=mk;
            if(cx+1>=0&&cx+1<(int)buf->width&&cy+k>=0&&cy+k<(int)buf->height)
                buf->data[(cy+k)*buf->width+cx+1]=mk;
        }
    }
}
