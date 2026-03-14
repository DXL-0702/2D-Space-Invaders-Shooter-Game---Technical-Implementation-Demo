#include "input.hpp"
#include <GLFW/glfw3.h>

static InputState g_input = { true, 0, false, 0, 0, false, false, false, false, false }; // game_running,move_dir,fire_pressed,move_x,move_y,slow_hold,fire_hold,pause_pressed,menu_left,menu_right

static void key_callback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/) {
    switch (key) {
    // --- 退出 ---
    case GLFW_KEY_ESCAPE:
        if (action == GLFW_PRESS) g_input.game_running = false;
        break;

    // --- 左右移动（兼容旧字段 move_dir + 新字段 move_x）---
    case GLFW_KEY_RIGHT:
        if (action == GLFW_PRESS)        { g_input.move_dir += 1; g_input.move_x += 1; g_input.menu_right = true; }
        else if (action == GLFW_RELEASE) { g_input.move_dir -= 1; g_input.move_x -= 1; }
        break;
    case GLFW_KEY_LEFT:
        if (action == GLFW_PRESS)        { g_input.move_dir -= 1; g_input.move_x -= 1; g_input.menu_left = true; }
        else if (action == GLFW_RELEASE) { g_input.move_dir += 1; g_input.move_x += 1; }
        break;

    // --- 上下移动（新增）---
    case GLFW_KEY_UP:
        if (action == GLFW_PRESS)        g_input.move_y += 1;
        else if (action == GLFW_RELEASE) g_input.move_y -= 1;
        break;
    case GLFW_KEY_DOWN:
        if (action == GLFW_PRESS)        g_input.move_y -= 1;
        else if (action == GLFW_RELEASE) g_input.move_y += 1;
        break;

    // --- 射击（持续按住）---
    case GLFW_KEY_SPACE:
        if (action == GLFW_PRESS)        { g_input.fire_hold = true; }
        else if (action == GLFW_RELEASE) { g_input.fire_hold = false;
                                           g_input.fire_pressed = true; } // 兼容旧逻辑
        break;

    // --- 慢速模式（左 Shift）---
    case GLFW_KEY_LEFT_SHIFT:
    case GLFW_KEY_RIGHT_SHIFT:
        if (action == GLFW_PRESS)        g_input.slow_hold = true;
        else if (action == GLFW_RELEASE) g_input.slow_hold = false;
        break;

    // --- 暂停（P 键）---
    case GLFW_KEY_P:
        if (action == GLFW_PRESS) g_input.pause_pressed = true;
        break;

    default:
        break;
    }
}

InputState* input_setup(GLFWwindow* window) {
    g_input.game_running=true; g_input.move_dir=0; g_input.fire_pressed=false;
    g_input.move_x=0; g_input.move_y=0; g_input.slow_hold=false;
    g_input.fire_hold=false; g_input.pause_pressed=false;
    g_input.menu_left=false; g_input.menu_right=false;
    glfwSetKeyCallback(window, key_callback);
    return &g_input;
}
