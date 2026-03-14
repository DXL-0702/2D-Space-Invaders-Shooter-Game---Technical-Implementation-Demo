#pragma once

struct GLFWwindow;

struct InputState {
    // 原有字段（保持现有代码兼容）
    bool game_running;
    int  move_dir;        // 左右 -1/0/+1（兼容旧代码）
    bool fire_pressed;    // 旧版单次触发（兼容旧代码）

    // 新增：全向移动
    int  move_x;          // 左右 -1/0/+1
    int  move_y;          // 上下 -1/0/+1

    // 新增：功能键
    bool slow_hold;       // Shift 持续按住 = 慢速模式
    bool fire_hold;       // 空格持续按住 = 持续射击
    bool pause_pressed;   // P 单帧触发（game_update 末尾清零）

    // 菜单专用：单帧触发，读取后立即清零
    bool menu_left;       // 左键单次按下
    bool menu_right;      // 右键单次按下
};

InputState* input_setup(GLFWwindow* window);
