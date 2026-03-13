#include "input.hpp"
#include <GLFW/glfw3.h>

static InputState g_input = { true, 0, false };

static void key_callback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/) {
    switch (key) {
    case GLFW_KEY_ESCAPE:
        if (action == GLFW_PRESS)
            g_input.game_running = false;
        break;
    case GLFW_KEY_RIGHT:
        if (action == GLFW_PRESS)   g_input.move_dir += 1;
        else if (action == GLFW_RELEASE) g_input.move_dir -= 1;
        break;
    case GLFW_KEY_LEFT:
        if (action == GLFW_PRESS)   g_input.move_dir -= 1;
        else if (action == GLFW_RELEASE) g_input.move_dir += 1;
        break;
    case GLFW_KEY_SPACE:
        if (action == GLFW_RELEASE)
            g_input.fire_pressed = true;
        break;
    default:
        break;
    }
}

InputState* input_setup(GLFWwindow* window) {
    g_input.game_running = true;
    g_input.move_dir = 0;
    g_input.fire_pressed = false;
    glfwSetKeyCallback(window, key_callback);
    return &g_input;
}
