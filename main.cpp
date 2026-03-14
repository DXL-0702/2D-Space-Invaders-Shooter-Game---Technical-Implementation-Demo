#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <cstdlib>
#include <thread>
#include <chrono>

#include "buffer.hpp"
#include "game.hpp"
#include "input.hpp"
#include "stage.hpp"

void error_callback(int error, const char* description) 
{
    fprintf(stderr, "GLEW Error %d: %s\n", error, description);
}

// 检查着色器编译错误
void validate_shader(GLuint shader, const char* file = 0) {
    static const unsigned int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];
    GLsizei length = 0;
    glGetShaderInfoLog(shader, BUFFER_SIZE, &length, buffer);
    if (length > 0) {
        printf("Shader %d(%s) compile error: %s\n", shader, (file ? file : ""), buffer);
    }
}

//检查程序连接错误
bool validate_program(GLuint program) {
    static const GLsizei BUFFER_SIZE = 512;
    GLchar buffer[BUFFER_SIZE];
    GLsizei length = 0;
    glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer);
    if (length > 0) {
        printf("Program %d link error: %s\n", program, buffer);
        return false;
    }
    return true;
}

int main() {

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) 
    {
        return -1;
    }

    // macOS: window hints 必须在创建窗口之前设置
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Space Invaders", NULL, NULL);
    if (!window) 
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Part 3: V-sync 固定帧率

    int glVersion[2] = {-1, 1};
    glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
    glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);
    printf("OpenGL version: %d.%d\n", glVersion[0], glVersion[1]);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f); //R=1, G=0, B=0, A=1 ->red

    // 定义屏幕尺寸
    const size_t buffer_width = 640;
    const size_t buffer_height = 480;

    // 定义清除颜色 （绿色）
    uint32_t clear_color = rgb_to_uint32(0, 128, 0);

    // 创建缓冲区object
    Buffer buffer;
    buffer.width = buffer_width;
    buffer.height = buffer_height;

    // 分配内存
    buffer.data = new uint32_t[buffer.width * buffer.height];

    // 初始化为绿色
    buffer_clear(&buffer, clear_color);

    // Part 3: 初始化游戏和输入
    Game game;
    game_init(&game, (int)buffer_width, (int)buffer_height);
    InputState* input = input_setup(window);

    const char* vertex_shader =
    "#version 330\n"
    "\n"
    "noperspective out vec2 TexCoord;\n"
    "\n"
    "void main(void){\n"
    "    // 根据顶点ID生成纹理坐标 (0,0), (2,0), (0,2) 覆盖屏幕\n"
    "    TexCoord.x = (gl_VertexID == 2)? 2.0: 0.0;\n"
    "    TexCoord.y = (gl_VertexID == 1)? 2.0: 0.0;\n"
    "\n"
    "    // 将纹理坐标转换为裁剪空间坐标 (-1 到 1)\n"
    "    gl_Position = vec4(2.0 * TexCoord - 1.0, 0.0, 1.0);\n"
    "}\n";

    const char* fragment_shader = 
    "#version 330\n"
    "\n"
    "uniform sampler2D buffer;      // 我们的缓冲区纹理\n"
    "noperspective in vec2 TexCoord; // 从顶点着色器接收的坐标\n"
    "\n"
    "out vec3 outColor;             // 最终输出的颜色\n"
    "\n"
    "void main(void){\n"
    "    // 采样纹理并输出 RGB 分量\n"
    "    outColor = texture(buffer, TexCoord).rgb;\n"
    "}\n";

    // 1. 创建程序对象
    GLuint shader_id = glCreateProgram();

    // 2. 创建并编译顶点着色器
    {
        GLuint shader_vp = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(shader_vp, 1, &vertex_shader, 0);
        glCompileShader(shader_vp);
        {
            GLint success;
            glGetShaderiv(shader_vp, GL_COMPILE_STATUS, &success);
            if (!success) {
                char infoLog[512];
                glGetShaderInfoLog(shader_vp, 512, NULL, infoLog);
                printf("❌ VERTEX SHADER COMPILATION FAILED:\n%s\n", infoLog);
                exit(1); // 或者直接返回
            }           
        }
        validate_shader(shader_vp, "Vertex Shader");// 检查错误
        glAttachShader(shader_id, shader_vp);
        glDeleteShader(shader_vp);// 附加后可以删除着色器对象
    }

    // 3. 创建并编译片段着色器
    {
        GLuint shader_fp = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(shader_fp, 1, &fragment_shader, 0);
        glCompileShader(shader_fp);
        validate_shader(shader_fp, "Fragment Shader");// 检查错误
        glAttachShader(shader_id, shader_fp);
        glDeleteShader(shader_fp);// 附加后可以删除着色器对象
    }

    // 4. 链接程序
    glLinkProgram(shader_id);
    if (!validate_program(shader_id)) {
        fprintf(stderr, "Error while validating shader.\n");
        delete[] buffer.data;
        glfwDestroyWindow(window);
        glfwTerminate();
        // 这里应该添加清理代码并退出
        return -1;
    };

    // 创建 VAO
    GLuint fullscreen_triangle_vao;
    glGenVertexArrays(1, &fullscreen_triangle_vao);
    glBindVertexArray(fullscreen_triangle_vao);

    // 创建纹理
    GLuint buffer_texture;
    glGenTextures(1, &buffer_texture);// 生成纹理 ID
    glBindTexture(GL_TEXTURE_2D, buffer_texture);// 绑定纹理对象

    //上传数据并指定格式
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, buffer.width, buffer.height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buffer.data);

    // 设置纹理参数
    // 1. 过滤方式：GL_NEAREST 表示不进行平滑滤波（保留像素风）
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // 2. 环绕方式：GL_CLAMP_TO_EDGE 防止越界
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glUseProgram(shader_id);
    // 获取 uniform 变量的位置
    GLint location = glGetUniformLocation(shader_id, "buffer");
    // 将纹理单元 0 赋值给变量
    glUniform1i(location, 0);

    glDisable(GL_DEPTH_TEST); // 2D 游戏不需要深度测试
    glBindVertexArray(fullscreen_triangle_vao); // 绑定 VAO
    glUseProgram(shader_id); // 激活着色器程序
    glActiveTexture(GL_TEXTURE0); // 激活纹理单元 0
    glBindTexture(GL_TEXTURE_2D, buffer_texture); // 绑定我们的缓冲区纹理

    const double target_fps = 60.0;
    const double frame_time  = 1.0 / target_fps;
    double last_time = glfwGetTime();

    while (!glfwWindowShouldClose(window) && input->game_running) {
        double now     = glfwGetTime();
        double elapsed = now - last_time;
        if (elapsed < frame_time) {
            std::this_thread::sleep_for(
                std::chrono::duration<double>(frame_time - elapsed));
        }
        last_time = glfwGetTime();

        // 1. CPU 端：按关卡背景色清空缓冲区
        uint32_t bg = stage_bg_color(game.stage);
        buffer_clear(&buffer, bg);

        // 2. 更新游戏逻辑（只在 PLAYING 阶段更新）
        game_update(&game, input);

        // 3. 渲染游戏画面（始终渲染）
        game_render(&game, &buffer);

        // 4. GPU 端：更新纹理数据
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, buffer.width, buffer.height,
                        GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buffer.data);

        // 5. GPU 端：绘制全屏三角形
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // 6. 交换缓冲区并处理事件
        glfwSwapBuffers(window);
        glfwPollEvents();

        // 7. 游戏结束：继续刷新画面，等待用户按 ESC 退出
        // （game_update 在非 PLAYING 阶段会直接返回，画面保持最后状态）
    }

    // 清理资源
    game_shutdown(&game);
    delete[] buffer.data;
    glDeleteTextures(1, &buffer_texture);
    glDeleteProgram(shader_id);
    glDeleteVertexArrays(1, &fullscreen_triangle_vao);
    
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;

}