#ifndef SHADER_H
#define SHADER_H

#include <OpenGL/gl3.h>

// 着色器验证函数
void validate_shader(GLuint shader, const char* file = 0);
bool validate_program(GLuint program);

#endif // SHADER_H