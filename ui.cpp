#include "ui.hpp"
#include "config.h"
#include <cstring>

// ---------------------------------------------------------------------------
// 5x7 像素字体（数字 0-9）
// ---------------------------------------------------------------------------
static uint8_t font[10][35]={
    {0,1,1,1,0,1,0,0,0,1,1,0,0,1,1,1,0,1,0,1,1,1,0,0,1,1,0,0,0,1,0,1,1,1,0},
    {0,0,1,0,0,0,1,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,1,1,0},
    {0,1,1,1,0,1,0,0,0,1,0,0,0,0,1,0,0,1,1,0,0,1,0,0,0,1,0,0,0,0,1,1,1,1,1},
    {1,1,1,1,0,0,0,0,0,1,0,0,0,0,1,0,1,1,1,0,0,0,0,0,1,0,0,0,0,1,1,1,1,1,0},
    {0,0,0,1,0,0,0,1,1,0,0,1,0,1,0,1,0,0,1,0,1,1,1,1,1,0,0,0,1,0,0,0,0,1,0},
    {1,1,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,1,0,0,0,0,1,1,0,0,0,1,0,1,1,1,0},
    {0,1,1,1,0,1,0,0,0,0,1,0,0,0,0,1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0},
    {1,1,1,1,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0},
    {0,1,1,1,0,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0},
    {0,1,1,1,0,1,0,0,0,1,1,0,0,0,1,0,1,1,1,1,0,0,0,0,1,0,0,0,0,1,0,1,1,1,0},
};

static void draw_pixel(Buffer* b, int x, int y, uint32_t c){
    if(x>=0&&y>=0&&x<(int)b->width&&y<(int)b->height) b->data[y*b->width+x]=c;
}
static void draw_rect(Buffer* b, int x, int y, int w, int h, uint32_t c){
    for(int xi=0;xi<w;++xi) for(int yi=0;yi<h;++yi) draw_pixel(b,x+xi,y+yi,c);
}
static void draw_digit(Buffer* b, int d, int x, int y, uint32_t c){
    if(d<0||d>9) return;
    for(int yi=0;yi<7;++yi) for(int xi=0;xi<5;++xi)
        if(font[d][yi*5+xi]) draw_pixel(b,x+xi,y+yi,c);
}
static void draw_number(Buffer* b, size_t n, int x, int y, uint32_t c){
    char buf[16]; int len=0;
    if(n==0){buf[len++]=0;}
    else{size_t t=n;while(t>0){buf[len++]=(char)(t%10);t/=10;}
         for(int i=0;i<len/2;++i){char tmp=buf[i];buf[i]=buf[len-1-i];buf[len-1-i]=tmp;}}
    for(int i=0;i<len;++i) draw_digit(b,buf[i],x+i*6,y,c);
}

// ---------------------------------------------------------------------------
Grade ui_calc_grade(const ClearStats* s){
    int score=0;
    if(s->hp_remaining>70) score+=3;
    else if(s->hp_remaining>40) score+=2;
    else if(s->hp_remaining>10) score+=1;
    if(s->time_frames<3600) score+=2; // 1分钟内
    else if(s->time_frames<7200) score+=1;
    int acc=s->shots_fired>0?s->shots_hit*100/s->shots_fired:0;
    if(acc>80) score+=2;
    else if(acc>50) score+=1;
    if(score>=7) return GRADE_S;
    if(score>=5) return GRADE_A;
    if(score>=3) return GRADE_B;
    return GRADE_C;
}

// ---------------------------------------------------------------------------
void ui_render_hud(Buffer* buf, size_t score, int combo,
                   const Player* player, int stage, int weapon_level, int frame_count){
    uint32_t white=rgb_to_uint32(255,255,255);
    uint32_t grey =rgb_to_uint32(80,80,80);

    // 左上：分数
    int ui_y=(int)buf->height-16;
    draw_number(buf,score,8,ui_y,white);

    // 左上：连杀（combo>1 时显示）
    if(combo>1) draw_number(buf,(size_t)combo,8,ui_y-10,rgb_to_uint32(255,200,0));

    // 右上：HP 血条
    int bar_x=(int)buf->width-112;
    draw_rect(buf,bar_x,ui_y+1,100,5,grey);
    if(player->hp>0){
        int fill=player->hp; // hp 0-100 → 0-100px
        uint32_t hc=(player->hp>40)?rgb_to_uint32(0,220,0):rgb_to_uint32(220,0,0);
        draw_rect(buf,bar_x,ui_y+1,fill,5,hc);
    }

    // 右上：护盾条（HP条上方）
    if(player->shield>0){
        int sfill=player->shield*100/PLAYER_MAX_SHIELD;
        draw_rect(buf,bar_x,ui_y-5,100,4,grey);
        draw_rect(buf,bar_x,ui_y-5,sfill,4,rgb_to_uint32(64,128,255));
    }

    // 右上：武器等级小方块
    for(int lv=0;lv<5;++lv){
        uint32_t lc=(lv<weapon_level)?rgb_to_uint32(255,220,0):grey;
        draw_rect(buf,(int)buf->width-112+lv*7,ui_y+8,5,4,lc);
    }

    // 底部：僚机图标
    for(int w=0;w<2;++w){
        uint32_t wc=player->wingmen[w].active?rgb_to_uint32(0,200,255):grey;
        draw_rect(buf,8+w*14,(int)buf->height/2-60,10,6,wc);
    }

    (void)stage; (void)frame_count;
}

void ui_render_pause(Buffer* buf){
    // 半透明深色遮罩（每隔一像素）
    for(size_t y=0;y<buf->height;y+=2)
        for(size_t x=0;x<buf->width;++x)
            buf->data[y*buf->width+x]=rgb_to_uint32(0,0,0);
    // "PAUSE" 用方块字母替代：画一个矩形框
    int cx=(int)buf->width/2, cy=(int)buf->height/2;
    draw_rect(buf,cx-30,cy-10,60,20,rgb_to_uint32(40,40,80));
    draw_number(buf,0,cx-3,cy-4,rgb_to_uint32(200,200,255)); // 占位
    // P-A-U-S-E 每个字母用方块表示
    uint32_t tc=rgb_to_uint32(255,255,255);
    draw_rect(buf,cx-28,cy-4,4,8,tc); draw_rect(buf,cx-28,cy-4,8,4,tc); // P
    draw_rect(buf,cx-18,cy-4,8,8,tc); draw_rect(buf,cx-18,cy,4,4,tc);   // A
    draw_rect(buf,cx-8, cy-4,4,8,tc); draw_rect(buf,cx-4, cy-4,4,8,tc); // U
    draw_rect(buf,cx+2, cy-4,8,8,tc); draw_rect(buf,cx+2, cy-4,8,4,tc); // S
    draw_rect(buf,cx+12,cy-4,8,4,tc); draw_rect(buf,cx+14,cy,4,4,tc);   // E
}

void ui_render_clear(Buffer* buf, const ClearStats* s){
    int cx=(int)buf->width/2, cy=(int)buf->height/2;
    draw_rect(buf,cx-60,cy-40,120,80,rgb_to_uint32(20,20,40));
    uint32_t w=rgb_to_uint32(255,255,255);
    // 得分
    draw_number(buf,s->score,cx-30,cy-30,w);
    // 评分
    uint32_t gc;
    switch(s->grade){
    case GRADE_S: gc=rgb_to_uint32(255,220,0);  break;
    case GRADE_A: gc=rgb_to_uint32(0,220,100);  break;
    case GRADE_B: gc=rgb_to_uint32(100,180,255);break;
    default:      gc=rgb_to_uint32(180,180,180);break;
    }
    draw_rect(buf,cx+20,cy-30,10,10,gc);
    // 击杀数
    draw_number(buf,(size_t)s->kills,cx-30,cy-15,w);
    // 通关时间（秒）
    draw_number(buf,(size_t)(s->time_frames/60),cx-30,cy,w);
}

void ui_render_title(Buffer* buf, int selected_diff, int frame_count){
    uint32_t bg=rgb_to_uint32(0,0,20);
    for(size_t i=0;i<buf->width*buf->height;++i) buf->data[i]=bg;
    // 标题光条
    int cy=(int)buf->height/2;
    draw_rect(buf,80,cy-60,480,8,rgb_to_uint32(0,100,200));
    draw_rect(buf,80,cy-50,480,8,rgb_to_uint32(0,150,255));
    // 难度选项
    uint32_t diff_cols[3]={rgb_to_uint32(0,220,100),rgb_to_uint32(255,220,0),rgb_to_uint32(255,60,60)};
    const char* labels[3]={"EASY","NORMAL","HARD"};
    (void)labels;
    for(int d=0;d<3;++d){
        uint32_t c=(d==selected_diff)?diff_cols[d]:rgb_to_uint32(80,80,80);
        draw_rect(buf,200+d*80,cy,60,14,c);
        draw_number(buf,(size_t)(d+1),222+d*80,cy+4,rgb_to_uint32(0,0,0));
    }
    // 闪烁提示
    if((frame_count/30)%2==0)
        draw_rect(buf,(int)buf->width/2-20,cy+30,40,6,rgb_to_uint32(255,255,255));
}

void ui_render_result(Buffer* buf, bool win, size_t score){
    int cx=(int)buf->width/2, cy=(int)buf->height/2;
    uint32_t bc=win?rgb_to_uint32(0,200,100):rgb_to_uint32(200,40,40);
    draw_rect(buf,cx-50,cy-12,100,24,bc);
    draw_number(buf,score,cx-18,cy-4,rgb_to_uint32(0,0,0));
}
