#include "enemy.hpp"
#include "bullet.hpp"
#include "player.hpp"
#include "config.h"
#include <cmath>
#include <cstring>
#include <cstdlib>

static uint8_t sp_s0[64]={0,0,0,1,1,0,0,0,0,0,1,1,1,1,0,0,0,1,1,0,0,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,0,1,0,0,1,0,0,1,0,0,0,1,0,1,1,0,1,0,1,0,0,0,0,0,0,1};
static uint8_t sp_s1[64]={0,0,0,1,1,0,0,0,0,0,1,1,1,1,0,0,0,1,1,0,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,0,1,0,1,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,0,0,1,0};
static uint8_t sp_m0[120]={0,0,0,0,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,0,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,1,0,1,0,1,0,0,0,0,0,0,0,0,1,0};
static uint8_t sp_m1[120]={0,0,0,0,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,0,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,0,1,1,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,1,0,0,1,1,0,0,1,0,1,0,1,0,1,0,0,0,0,1,0,1,0,1,0,1,0,0,0,1,0,0,1,0,0};
static uint8_t sp_e0[168]={
    0,0,0,0,0,1,1,1,1,0,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,0,0,0,
    0,0,1,1,1,1,1,1,1,1,1,1,0,0,
    0,1,1,0,0,1,1,1,1,0,0,1,1,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,0,1,1,0,1,1,1,1,1,
    1,1,0,1,0,1,0,0,1,0,1,0,1,1,
    0,1,1,0,1,0,1,1,0,1,0,1,1,0,
    0,0,1,1,0,1,0,0,1,0,1,1,0,0,
    0,0,0,1,1,0,1,1,0,1,1,0,0,0,
    0,0,0,0,1,1,0,0,1,1,0,0,0,0,
    0,0,0,0,0,1,1,1,1,0,0,0,0,0,
};
static uint8_t sp_e1[168]={
    0,0,0,0,0,1,1,1,1,0,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,0,0,0,
    0,0,1,1,1,1,1,1,1,1,1,1,0,0,
    0,1,1,0,0,1,1,1,1,0,0,1,1,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,1,1,0,1,1,1,1,0,1,1,0,1,
    0,1,0,0,1,0,1,1,0,1,0,0,1,0,
    1,0,1,1,0,1,0,0,1,0,1,1,0,1,
    0,1,0,0,1,0,1,1,0,1,0,0,1,0,
    0,0,1,1,0,0,0,0,0,0,1,1,0,0,
    0,0,0,1,1,0,0,0,0,1,1,0,0,0,
    0,0,0,0,1,1,1,1,1,1,0,0,0,0,
};
static uint8_t sp_b0[1536];
static uint8_t sp_b1[1536];
static Sprite spr[5][2];
static bool g_inited=false;

static void init_sprites(){
    if(g_inited)return; g_inited=true;
    spr[ENEMY_SMALL][0]={8,8,sp_s0};   spr[ENEMY_SMALL][1]={8,8,sp_s1};
    spr[ENEMY_MEDIUM][0]={12,10,sp_m0};spr[ENEMY_MEDIUM][1]={12,10,sp_m1};
    spr[ENEMY_ELITE][0]={14,12,sp_e0}; spr[ENEMY_ELITE][1]={14,12,sp_e1};
    for(int i=0;i<1536;++i){
        int r=i/48,c=i%48,dx=c-23,dy=r-15;
        sp_b0[i]=(dx*dx/16+dy*dy/9<=64)?1:0;
        sp_b1[i]=sp_b0[i]; if(r>=12&&r<=18&&c>=20&&c<=27)sp_b1[i]^=1;
    }
    spr[ENEMY_BOSS][0]={48,32,sp_b0}; spr[ENEMY_BOSS][1]={48,32,sp_b1};
}

static int ew(EnemyType t){switch(t){case ENEMY_SMALL:return 8;case ENEMY_MEDIUM:return 12;case ENEMY_ELITE:return 14;case ENEMY_BOSS:return 48;default:return 8;}}
static int eh(EnemyType t){switch(t){case ENEMY_SMALL:return 8;case ENEMY_MEDIUM:return 10;case ENEMY_ELITE:return 12;case ENEMY_BOSS:return 32;default:return 8;}}

void enemy_manager_init(EnemyManager* em){memset(em,0,sizeof(*em));init_sprites();}

void enemy_spawn(EnemyManager* em,EnemyType type,int x,int y,float dm){
    if(em->num_enemies>=MAX_ENEMIES)return;
    Enemy& e=em->enemies[em->num_enemies++]; memset(&e,0,sizeof(e));
    e.type=type;e.x=x;e.y=y;
    e.move_dir=(rand()%2)?1:-1; e.move_phase=(float)(rand()%628)/100.f;
    switch(type){
    case ENEMY_SMALL:  e.hp=e.max_hp=(int)(SMALL_HP *dm+.5f); e.score_value=SMALL_SCORE;  break;
    case ENEMY_MEDIUM: e.hp=e.max_hp=(int)(MEDIUM_HP*dm+.5f); e.score_value=MEDIUM_SCORE; break;
    case ENEMY_ELITE:  e.hp=e.max_hp=(int)(ELITE_HP *dm+.5f); e.score_value=ELITE_SCORE;  break;
    case ENEMY_BOSS:   e.hp=e.max_hp=(int)(BOSS_HP_BASE*dm+.5f); e.score_value=BOSS_SCORE;
                       e.boss_phase=BOSS_ENTER; e.boss_target_x=x; break;
    default:break;
    }
    e.shoot_timer=rand()%60;
}

bool enemy_deal_damage(EnemyManager* em,int idx,int dmg){
    if(idx<0||idx>=(int)em->num_enemies)return false;
    Enemy& e=em->enemies[idx];
    if(e.death_timer>0||e.type==ENEMY_NONE)return false;
    e.hp-=dmg;
    if(e.hp<=0){e.hp=0;e.death_timer=EXPLOSION_FRAMES*EXPLOSION_FRAME_DUR;return true;}
    return false;
}

void enemy_get_aabbs(const EnemyManager* em,int* ox,int* oy,int* ow2,int* oh2,bool* alive,int max,int* cnt){
    int n=0;
    for(size_t i=0;i<em->num_enemies&&n<max;++i,++n){
        const Enemy& e=em->enemies[i];
        ox[n]=e.x; oy[n]=e.y; ow2[n]=ew(e.type); oh2[n]=eh(e.type);
        alive[n]=(e.death_timer==0&&e.type!=ENEMY_NONE);
    }
    *cnt=n;
}

size_t enemy_update(EnemyManager* em,BulletManager* bm,const Player* player,
                    int sw,int sh,EnemyDeathEvent* deaths,size_t maxd){
    size_t dc=0;
    uint32_t ec=rgb_to_uint32(255,64,255);
    float px=player?(float)(player->x+player_sprite_width()/2):(float)(sw/2);
    float py=player?(float)(player->y+player_sprite_height()/2):60.f;

    for(size_t i=0;i<em->num_enemies;){
        Enemy& e=em->enemies[i];
        if(e.death_timer>0){
            if(--e.death_timer==0){
                if(dc<maxd) deaths[dc++]={e.x+ew(e.type)/2,e.y+eh(e.type)/2,e.type,e.score_value};
                e=em->enemies[--em->num_enemies]; continue;
            }
            ++i; continue;
        }
        if(++e.anim_timer>=12){e.anim_timer=0;e.anim_frame^=1;}

        switch(e.type){
        case ENEMY_SMALL:{
            e.y-=SMALL_MOVE_SPEED;
            if(++e.move_timer>=20){e.move_timer=0;e.x+=e.move_dir*5;
                if(e.x<=0||e.x+8>=sw)e.move_dir=-e.move_dir;}
            if(e.y+8<0){e=em->enemies[--em->num_enemies];continue;}
            if(++e.shoot_timer>=SMALL_FIRE_RATE){e.shoot_timer=0;
                bullet_fire_straight(bm,(float)(e.x+4),(float)e.y,0.f,-(float)ENEMY_BULLET_SPEED,1,ec);}
            break;
        }
        case ENEMY_MEDIUM:{
            if(++e.move_timer>=3){e.move_timer=0;
                float ddx=px-(float)(e.x+6),ddy=py-(float)(e.y+5);
                float len=sqrtf(ddx*ddx+ddy*ddy);
                if(len>.1f){e.x+=(int)(ddx/len*MEDIUM_MOVE_SPEED);e.y+=(int)(ddy/len*MEDIUM_MOVE_SPEED);}
                if(e.x<0)e.x=0; if(e.x+12>sw)e.x=sw-12;}
            if(e.y+10<0||e.y>sh){e=em->enemies[--em->num_enemies];continue;}
            if(++e.shoot_timer>=MEDIUM_FIRE_RATE){e.shoot_timer=0;
                float ang=atan2f(py-(float)e.y,px-(float)(e.x+6));
                bullet_fire_fan(bm,(float)(e.x+6),(float)e.y,3,ang,.4f,(float)ENEMY_BULLET_SPEED,1,ec);}
            break;
        }
        case ENEMY_ELITE:{
            e.move_phase+=.05f;
            e.x=(int)((float)(sw/2)+(float)(sw/2-14)*sinf(e.move_phase)*.7f);
            e.y-=1;
            if(e.y+12<0){e=em->enemies[--em->num_enemies];continue;}
            if(++e.shoot_timer>=ELITE_FIRE_RATE){e.shoot_timer=0;
                bullet_fire_ring(bm,(float)(e.x+7),(float)e.y,8,(float)ENEMY_BULLET_SPEED,1,ec);
                bullet_fire_missile(bm,(float)(e.x+7),(float)e.y);}
            break;
        }
        case ENEMY_BOSS:{
            int hp_pct=e.max_hp>0?e.hp*100/e.max_hp:0;
            if(e.boss_phase==BOSS_ENTER&&e.y<=sh-32-40) e.boss_phase=BOSS_P1;
            else if(e.boss_phase==BOSS_P1&&hp_pct<60)   e.boss_phase=BOSS_P2;
            else if(e.boss_phase==BOSS_P2&&hp_pct<30)   e.boss_phase=BOSS_P3;
            ++e.boss_timer;
            if(e.boss_phase==BOSS_ENTER){ e.y-=1; }
            else{
                int ddx2=e.boss_target_x-(e.x+24);
                if(abs(ddx2)<BOSS_MOVE_SPEED+1) e.boss_target_x=40+rand()%(sw-80);
                e.x+=(ddx2>0?1:-1)*BOSS_MOVE_SPEED;
                if(e.x<0)e.x=0; if(e.x+48>sw)e.x=sw-48;
            }
            int fr=(e.boss_phase==BOSS_P3)?BOSS_FIRE_RATE/2:BOSS_FIRE_RATE;
            if(e.boss_phase!=BOSS_ENTER&&++e.shoot_timer>=fr){
                e.shoot_timer=0;
                float bx=(float)(e.x+24),by=(float)e.y;
                switch(e.boss_phase){
                case BOSS_P1:
                    for(int k=-2;k<=2;++k)
                        bullet_fire_straight(bm,bx+k*8,by,0.f,-(float)ENEMY_BULLET_SPEED,2,ec);
                    break;
                case BOSS_P2:
                    bullet_fire_fan(bm,bx,by,5,-(float)M_PI/2,1.2f,(float)ENEMY_BULLET_SPEED,2,ec);
                    bullet_fire_ring(bm,bx,by,12,(float)ENEMY_BULLET_SPEED*.7f,1,rgb_to_uint32(255,100,100));
                    break;
                case BOSS_P3:
                    bullet_fire_laser(bm,(int)bx,(int)by,sh,rgb_to_uint32(255,50,50));
                    bullet_fire_missile(bm,bx-10,by);
                    bullet_fire_missile(bm,bx+10,by);
                    for(int k=-3;k<=3;++k)
                        bullet_fire_straight(bm,bx+k*6,by,(float)k*.3f,-(float)ENEMY_BULLET_SPEED,2,ec);
                    break;
                default:break;
                }
            }
            break;
        }
        default:break;
        }
        ++i;
    }
    return dc;
}

void enemy_render(const EnemyManager* em,Buffer* buf){
    for(size_t i=0;i<em->num_enemies;++i){
        const Enemy& e=em->enemies[i];
        if(e.type==ENEMY_NONE) continue;
        uint32_t col;
        switch(e.type){
        case ENEMY_SMALL:  col=rgb_to_uint32(220,60,60);  break;
        case ENEMY_MEDIUM: col=rgb_to_uint32(60,200,60);  break;
        case ENEMY_ELITE:  col=rgb_to_uint32(80,80,255);  break;
        case ENEMY_BOSS:   col=rgb_to_uint32(255,120,0);  break;
        default:           col=rgb_to_uint32(200,200,200);break;
        }
        // 死亡闪烁
        if(e.death_timer>0){
            if((e.death_timer/4)%2==0) col=rgb_to_uint32(255,255,255);
            else continue;
        }
        int frame=e.anim_frame&1;
        if(e.type>=ENEMY_SMALL&&e.type<=ENEMY_BOSS)
            buffer_sprite_draw(buf,spr[e.type][frame],(size_t)e.x,(size_t)e.y,col);
        // BOSS 血条
        if(e.type==ENEMY_BOSS&&e.death_timer==0&&e.max_hp>0){
            int bw=48, fill=bw*e.hp/e.max_hp;
            uint32_t bg=rgb_to_uint32(60,0,0), fg=rgb_to_uint32(255,30,30);
            for(int xi=0;xi<bw;++xi){
                size_t sx=(size_t)(e.x+xi);
                size_t sy=(size_t)(e.y+33);
                if(sx<buf->width&&sy<buf->height)
                    buf->data[sy*buf->width+sx]=(xi<fill)?fg:bg;
            }
        }
    }
}
