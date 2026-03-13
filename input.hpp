#pragma once

struct GLFWwindow;

// Part 4 会用到的输入状态（Part 3 先搭好结构）
struct InputState {
    bool game_running;   // Esc 退出
    int move_dir;       // 左 -1，右 +1
    bool fire_pressed;   // 空格发射
};

// 设置键盘回调，返回当前输入状态指针（供主循环读取）
InputState* input_setup(GLFWwindow* window);
